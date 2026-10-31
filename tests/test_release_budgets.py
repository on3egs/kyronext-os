"""Vérifie le refus effectif des plafonds et des paquets non installés."""
import contextlib
import io
import runpy
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/check-release-budgets.py'


class ReleaseBudgetsTests(unittest.TestCase):
    def invoke(self, argv):
        with patch('sys.argv', [str(SCRIPT), *argv]), contextlib.redirect_stdout(io.StringIO()):
            runpy.run_path(str(SCRIPT), run_name='__main__')

    def test_nonempty_image_and_report(self):
        with tempfile.TemporaryDirectory() as name:
            iso = Path(name) / 'test.iso'
            iso.write_bytes(b'test-image')
            report = Path(name) / 'report.json'
            self.invoke(['--iso', str(iso), '--report', str(report)])
            self.assertIn('iso_bytes', report.read_text())

    def test_empty_image_refused(self):
        with tempfile.TemporaryDirectory() as name:
            iso = Path(name) / 'empty.iso'
            iso.touch()
            with self.assertRaises(SystemExit):
                self.invoke(['--iso', str(iso)])

    def test_sixteen_gb_image_refused_without_allocating_disk(self):
        original = Path.stat
        def stat(path, *args, **kwargs):
            if str(path) == 'oversize.iso':
                return SimpleNamespace(st_size=16_000_000_000)
            return original(path, *args, **kwargs)
        with patch.object(Path, 'stat', stat), self.assertRaises(SystemExit):
            self.invoke(['--iso', 'oversize.iso'])

    def test_image_just_below_sixteen_gb_is_accepted_without_allocating_disk(self):
        original = Path.stat
        def stat(path, *args, **kwargs):
            if str(path) == 'large-but-valid.iso':
                return SimpleNamespace(st_size=15_999_999_999)
            return original(path, *args, **kwargs)
        with patch.object(Path, 'stat', stat):
            self.invoke(['--iso', 'large-but-valid.iso'])

    def test_uninstalled_package_refused(self):
        response = SimpleNamespace(stdout='firmware-atheros\t100\tconfig-files\n')
        with patch('subprocess.run', return_value=response), self.assertRaises(SystemExit):
            self.invoke(['--root', 'unused-test-root'])

    def test_hardware_over_one_gb_refused(self):
        response = SimpleNamespace(stdout='firmware-atheros\t1000000\tinstalled\n')
        with patch('subprocess.run', return_value=response), self.assertRaises(SystemExit):
            self.invoke(['--root', 'unused-test-root'])

    def test_no_overwrite_of_existing_report(self):
        with tempfile.TemporaryDirectory() as name:
            iso = Path(name) / 'test.iso'
            iso.write_bytes(b'test-image')
            report = Path(name) / 'report.json'
            report.write_text('preserve-me')
            with self.assertRaises(FileExistsError):
                self.invoke(['--iso', str(iso), '--report', str(report)])
            self.assertEqual('preserve-me', report.read_text())


if __name__ == '__main__':
    unittest.main()
