import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
LIB = ROOT / 'iso/config/includes.chroot/usr/lib/kyronext'


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, LIB / filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


manager = load('driver_manager_test', 'driver_manager.py')
offline = load('offline_drivers_test', 'build_offline_drivers.py')


class DriverManagerTests(unittest.TestCase):
    def privileged_stat(self, path, *args, **kwargs):
        info = self.real_stat(path, *args, **kwargs)
        return type('Info', (), {'st_uid': 0, 'st_mode': info.st_mode & ~0o022})()

    def setUp(self):
        self.real_stat = Path.stat

    def test_specific_legacy_usb_firmware_mappings(self):
        self.assertEqual(
            ['firmware-ath9k-htc', 'firmware-carl9170', 'firmware-iwlwifi', 'firmware-zd1211'],
            manager.firmware_packages_for_modules(['ath9k_htc', 'carl9170', 'iwlwifi', 'zd1211rw']))

    def bundle(self, root, content=b'example-deb'):
        (root / 'firmware.deb').write_bytes(content)
        data = {'packages': [{'package': 'firmware-atheros', 'version': '1',
                'file': 'firmware.deb', 'sha256': hashlib.sha256(content).hexdigest()}],
                'firmware_allowlist': ['firmware-atheros']}
        (root / 'manifest.json').write_text(json.dumps(data))
        return data

    def test_valid_offline_bundle(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'BUNDLE', Path(tmp)):
            data = self.bundle(Path(tmp))
            self.assertEqual(data, manager.verify_bundle())

    def test_modified_package_refused(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'BUNDLE', Path(tmp)):
            self.bundle(Path(tmp)); (Path(tmp) / 'firmware.deb').write_bytes(b'tampered')
            with self.assertRaisesRegex(RuntimeError, 'altéré'): manager.verify_bundle()

    def test_manifest_path_escape_refused(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'BUNDLE', Path(tmp)):
            data = self.bundle(Path(tmp)); data['packages'][0]['file'] = '../firmware.deb'
            (Path(tmp) / 'manifest.json').write_text(json.dumps(data))
            with self.assertRaisesRegex(RuntimeError, 'Nom de paquet'): manager.verify_bundle()

    def test_unknown_radio_never_changes_network(self):
        with patch.object(manager, 'require_admin'), patch.object(manager, 'wifi_state', return_value={'radio': 'unknown'}), patch.object(manager, 'run') as command:
            with self.assertRaisesRegex(RuntimeError, 'État radio inconnu'): manager.repair_wifi()
            command.assert_not_called()

    def test_rollback_id_cannot_escape_state_directory(self):
        with patch.object(manager, 'require_admin'), patch.object(manager, 'run') as command:
            with self.assertRaisesRegex(RuntimeError, 'invalide'): manager.restore_wifi('../secret')
            command.assert_not_called()

    def test_rollback_snapshot_must_be_a_regular_file(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'STATE', Path(tmp)), patch.object(manager, 'require_admin'), patch.object(manager, 'run') as command:
            identifier = 'a' * 32
            (Path(tmp) / (identifier + '.json')).mkdir()
            with self.assertRaisesRegex(RuntimeError, 'Sauvegarde'):
                manager.restore_wifi(identifier)
            command.assert_not_called()

    def test_rollback_snapshot_symlink_is_refused_before_radio_change(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'STATE', Path(tmp)), patch.object(manager, 'require_admin'), patch.object(manager, 'run') as command:
            identifier = 'b' * 32
            path = Path(tmp) / (identifier + '.json')
            path.write_text('{}')
            with patch.object(Path, 'is_symlink', side_effect=[False, True]):
                with self.assertRaisesRegex(RuntimeError, 'Sauvegarde non sûre'):
                    manager.restore_wifi(identifier)
            command.assert_not_called()

    def test_unknown_radio_snapshot_never_changes_network(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'STATE', Path(tmp)), patch.object(manager, 'require_admin'), patch.object(manager, 'read', return_value='boot'), patch.object(manager, 'run') as command:
            path = Path(tmp) / ('a' * 32 + '.json')
            path.write_text(json.dumps({'radio': 'invalid', 'boot_id': 'boot'}))
            path.chmod(0o600)
            original = Path.stat
            def stat(p, *args, **kwargs):
                result = original(p, *args, **kwargs)
                return type('Info', (), {'st_uid': 0, 'st_mode': result.st_mode})()
            with patch.object(Path, 'stat', stat):
                with self.assertRaises(RuntimeError): manager.restore_wifi('a' * 32)
            command.assert_not_called()

    def test_old_boot_snapshot_refused(self):
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'STATE', Path(tmp)), patch.object(manager, 'require_admin'), patch.object(manager, 'read', return_value='new-boot'), patch.object(manager, 'run') as command:
            path = Path(tmp) / ('a' * 32 + '.json')
            path.write_text(json.dumps({'radio': 'enabled', 'boot_id': 'old-boot'}))
            path.chmod(0o600)
            # Root ownership is simulated; no real network command may execute.
            original = Path.stat
            def stat(p, *args, **kwargs):
                result = original(p, *args, **kwargs)
                if p == path:
                    return type('Info', (), {'st_uid': 0, 'st_mode': result.st_mode})()
                return result
            with patch.object(Path, 'stat', stat):
                with self.assertRaisesRegex(RuntimeError, 'autre démarrage'): manager.restore_wifi('a' * 32)
            command.assert_not_called()

    def test_report_redacts_ipv4_and_mac(self):
        result = manager.redact('firmware 192.168.1.4 aa:bb:cc:dd:ee:ff')
        self.assertNotIn('192.168.1.4', result); self.assertNotIn('aa:bb:cc:dd:ee:ff', result)

    def test_wifi_repair_has_usable_rollback(self):
        before = {'radio': 'disabled', 'boot_id': 'boot', 'rfkill': [{'id': 2, 'type': 'wlan', 'soft': 'blocked', 'hard': 'unblocked'}]}
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'STATE', Path(tmp)), patch.object(manager, 'require_admin'), patch.object(manager, 'wifi_state', return_value=before), patch.object(manager, 'read', return_value='boot'), patch.object(manager, 'run', return_value={'ok': True, 'output': ''}) as command, patch.object(Path, 'stat', lambda path, *a, **kw: self.privileged_stat(path, *a, **kw)):
            result = manager.repair_wifi()
            self.assertTrue((Path(tmp) / (result['rollback_id'] + '.json')).is_file())
            manager.restore_wifi(result['rollback_id'])
            calls = [call.args for call in command.call_args_list]
            self.assertEqual(calls, [('rfkill', 'unblock', 'wifi'), ('nmcli', 'radio', 'wifi', 'on'), ('nmcli', 'radio', 'wifi', 'off'), ('rfkill', 'block', '2')])

    def test_failed_wifi_activation_restores_previous_state(self):
        before = {'radio': 'disabled', 'boot_id': 'boot', 'rfkill': []}
        success = {'ok': True, 'output': ''}
        with tempfile.TemporaryDirectory() as tmp, patch.object(manager, 'STATE', Path(tmp)), patch.object(manager, 'require_admin'), patch.object(manager, 'wifi_state', return_value=before), patch.object(manager, 'read', return_value='boot'), patch.object(manager, 'run', side_effect=[success, RuntimeError('activation impossible'), success]) as command, patch.object(Path, 'stat', lambda path, *a, **kw: self.privileged_stat(path, *a, **kw)):
            with self.assertRaisesRegex(RuntimeError, 'retour arrière'):
                manager.repair_wifi()
            self.assertEqual(command.call_args_list[-1].args, ('nmcli', 'radio', 'wifi', 'off'))


class OfflineClosureTests(unittest.TestCase):
    def test_installed_alternative_and_virtual_provider(self):
        packages = {'firmware': {'depends': 'missing | libc, virtual (>= 1)'},
                    'libc': {'depends': ''}, 'provider': {'depends': '', 'provides': 'virtual (= 2)'}}
        self.assertEqual({'firmware', 'libc', 'provider'}, offline.resolve_closure(['firmware'], packages))

    def test_cycle_terminates(self):
        packages = {'a': {'depends': 'b'}, 'b': {'depends': 'a'}}
        self.assertEqual({'a', 'b'}, offline.resolve_closure(['a'], packages))

    def test_missing_dependency_is_not_silently_ignored(self):
        with self.assertRaisesRegex(RuntimeError, 'non résolue'):
            offline.resolve_closure(['firmware'], {'firmware': {'depends': 'missing'}})

    def test_missing_selected_package_refused(self):
        with self.assertRaisesRegex(RuntimeError, 'non installé'): offline.resolve_closure(['missing'], {})


if __name__ == '__main__': unittest.main()
