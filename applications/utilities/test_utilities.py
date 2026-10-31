"""Tests des calculs et sauvegardes ; aucune interface Qt nécessaire."""
from pathlib import Path
import runpy
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
BIN = ROOT / 'iso/config/includes.chroot/usr/local/bin'
calculator = runpy.run_path(str(BIN / 'kyronext-calculator'))
notes = runpy.run_path(str(BIN / 'kyronext-notes'))


class CalculatorTests(unittest.TestCase):
    def test_precedence_decimals_and_parentheses(self):
        for expression, expected in [('2+3*4', '14'), ('(2+3)*4', '20'),
                                     ('0,1+0,2', '0.3'), ('-(2+3)/2', '-2.5'),
                                     ('17%5', '2'), ('5×4÷2', '10'), ('-0', '0')]:
            with self.subTest(expression=expression):
                self.assertEqual(calculator['calculate'](expression), expected)

    def test_rejects_code_invalid_and_zero_division(self):
        for expression in ['__import__("os")', 'open("test")', '1/0', '1%0',
                           '2(3)', '(1+2', '2**3', '1e5', '', '-' * 40 + '1']:
            with self.subTest(expression=expression):
                with self.assertRaises(calculator['CalculationError']):
                    calculator['calculate'](expression)

    def test_bounded_input(self):
        with self.assertRaises(calculator['CalculationError']):
            calculator['calculate']('1' * 513)


class NotesTests(unittest.TestCase):
    def setUp(self):
        build = ROOT / 'build'
        build.mkdir(exist_ok=True)
        self.scratch = tempfile.TemporaryDirectory(prefix='notes-test-', dir=build)
        self.addCleanup(self.scratch.cleanup)
        self.path = Path(self.scratch.name) / 'notes.txt'

    def test_unicode_round_trip_and_replace(self):
        notes['save_text'](self.path, 'Été — Kyronext 🚗\nDeuxième ligne\n')
        self.assertEqual(self.path.read_text(encoding='utf-8'), 'Été — Kyronext 🚗\nDeuxième ligne\n')
        first_hash = notes['fingerprint'](self.path)
        notes['save_text'](self.path, 'Version suivante')
        self.assertNotEqual(notes['fingerprint'](self.path), first_hash)
        self.assertEqual(self.path.read_bytes(), b'Version suivante')
        self.assertEqual(list(self.path.parent.iterdir()), [self.path])

    def test_failed_replace_keeps_original_and_cleans_own_temp(self):
        self.path.write_bytes(b'Original')
        with patch.object(notes['os'], 'replace', side_effect=OSError('Échec simulé')):
            with self.assertRaises(OSError):
                notes['save_text'](self.path, 'Nouvelle version')
        self.assertEqual(self.path.read_bytes(), b'Original')
        self.assertEqual(list(self.path.parent.iterdir()), [self.path])


if __name__ == '__main__':
    unittest.main()
