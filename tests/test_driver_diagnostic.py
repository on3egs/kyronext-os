"""Contrats du diagnostic : identifiants USB et absence de promesse implicite."""
import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

path = Path(__file__).resolve().parents[1] / 'hardware/legacy-driver-manager/diagnose.py'
spec = importlib.util.spec_from_file_location('diagnose', path)
diagnose = importlib.util.module_from_spec(spec)
spec.loader.exec_module(diagnose)


class DiagnosticTests(unittest.TestCase):
    @unittest.skipUnless(sys.platform.startswith('linux'), 'Les chemins sysfs avec deux-points sont propres à Linux.')
    def test_usb_interface_retains_parent_identifiers(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            actual = root / 'physical/1-1'
            interface = actual / '1-1:1.0'
            interface.mkdir(parents=True)
            (actual / 'idVendor').write_text('1234')
            (actual / 'idProduct').write_text('5678')
            (interface / 'modalias').write_text('usb:example')
            link = root / 'bus-interface'
            link.symlink_to(interface, target_is_directory=True)
            with patch.object(diagnose, 'command', return_value={'status': 'ok', 'output': 'example'}), patch.object(diagnose, 'firmware', return_value={}):
                result = diagnose.device(link, 'usb')
            self.assertEqual((result['vendor_id'], result['device_id']), ('1234', '5678'))
            self.assertEqual(result['candidate_modules'], ['example'])
            self.assertEqual(result['compatibility_status'], 'statut inconnu')

    def test_missing_information_does_not_crash_or_invent_driver(self):
        with tempfile.TemporaryDirectory() as temporary:
            result = diagnose.device(Path(temporary), 'pci')
            self.assertIsNone(result['bound_driver'])
            self.assertEqual(result['candidate_modules'], [])
            self.assertEqual(result['compatibility_status'], 'statut inconnu')


if __name__ == '__main__':
    unittest.main()
