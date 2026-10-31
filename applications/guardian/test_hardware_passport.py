"""Garanties du passeport : confidentialité, non-écrasement et statut non vérifié."""
import importlib.util
import json
import os
from pathlib import Path
import stat
import tempfile
import unittest

source = Path(__file__).with_name('hardware_passport.py')
specification = importlib.util.spec_from_file_location('hardware_passport', source)
passport = importlib.util.module_from_spec(specification)
specification.loader.exec_module(passport)


class FakeDiagnostic:
    @staticmethod
    def collect():
        return {'timestamp_utc': '2026-10-30T00:00:00Z',
                'architecture': 'x86_64', 'kernel': 'test-kernel',
                'boot_mode': 'UEFI', 'devices': []}

    @staticmethod
    def read(path):
        return {'/proc/cpuinfo': 'model name : Test CPU\n',
                '/proc/meminfo': 'MemTotal: 4096 kB\n'}.get(path)


class PassportTests(unittest.TestCase):
    def test_inventory_never_claims_validation(self):
        report = passport.create_passport(FakeDiagnostic())
        self.assertEqual(report['validation']['status'], 'non vérifié')
        for field in ('boot', 'display', 'network', 'essential_peripherals'):
            self.assertEqual(report['validation'][field], 'non vérifié')
        self.assertFalse(report['capabilities']['automatic_changes'])
        self.assertFalse(report['capabilities']['ai_doctor'])
        self.assertEqual(report['machine']['cpu_model'], 'Test CPU')

    @unittest.skipUnless(os.name == 'posix', 'Permissions Linux requises')
    def test_private_creation_and_refusal_to_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'passport.json'
            previous_umask = os.umask(0)
            try:
                passport.write_passport(path, {'status': 'non vérifié'})
            finally:
                os.umask(previous_umask)
            self.assertEqual(stat.S_IMODE(path.stat().st_mode), 0o600)
            original = path.read_bytes()
            with self.assertRaises(FileExistsError):
                passport.write_passport(path, {'status': 'invalide'})
            self.assertEqual(path.read_bytes(), original)
            self.assertEqual(json.loads(original)['status'], 'non vérifié')

    @unittest.skipUnless(os.name == 'posix', 'Liens Linux requis')
    def test_existing_symlink_is_not_followed(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory) / 'existing.txt'
            target.write_text('keep')
            link = Path(directory) / 'passport.json'
            link.symlink_to(target)
            with self.assertRaises(FileExistsError):
                passport.write_passport(link, {'unexpected': True})
            self.assertEqual(target.read_text(), 'keep')


if __name__ == '__main__':
    unittest.main()
