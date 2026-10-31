"""Calculs avancés : exactitude, limites et absence d'exécution arbitraire."""
from pathlib import Path
import runpy
import unittest

ROOT = Path(__file__).resolve().parents[2]
module = runpy.run_path(str(ROOT / 'iso/config/includes.chroot/usr/local/bin/kyronext-calculator'))
calc, convert, Error = module['calculate_mode'], module['convert_unit'], module['CalculationError']


class AdvancedCalculationTests(unittest.TestCase):
    def test_scientific_decimal_and_domains(self):
        self.assertEqual(calc('sqrt(144) + 0.1 + 0.2', 'scientific'), '12.3')
        self.assertEqual(calc('ln(e)', 'scientific')[:14], '0.999999999999')
        self.assertEqual(calc('2**100', 'scientific'), '1267650600228229401496703205000')
        self.assertEqual(calc('sin(pi/2)', 'scientific'), '1')
        for expression in ('sqrt(-1)', 'ln(0)', 'exp(1001)', '2**1001', '1/0'):
            with self.assertRaises(Error): calc(expression, 'scientific')

    def test_programming_exact_big_integers(self):
        self.assertEqual(calc('(0xff & 15) << 2', 'programmer'), '60')
        self.assertEqual(calc('2**100', 'programmer'), str(2**100))
        self.assertEqual(calc('~0', 'programmer'), '-1')
        for expression in ('1 << -1', '1 << 1025', '2**(-1)', '4/2', 'True', '1.5', '2**1000 * 2**1000 * 2**1000 * 2**1000 * 2**1000'):
            with self.assertRaises(Error): calc(expression, 'programmer')

    def test_rejects_python_execution_and_ambiguous_syntax(self):
        for mode in ('scientific', 'programmer'):
            for expression in ('__import__("os").system("echo unsafe")', '(1).__class__', '[1][0]', 'lambda: 1', '1 if True else 2', 'sum([1])', 'sin(x=1)', 'open("file")'):
                with self.assertRaises(Error): calc(expression, mode)

    def test_unit_conversions_and_temperature(self):
        self.assertEqual(convert('1', 'Longueur', 'mi', 'm'), '1609.344')
        self.assertEqual(convert('1', 'Données', 'Mio', 'octet'), '1048576')
        self.assertEqual(convert('32', 'Température', '°F', '°C'), '0')
        self.assertEqual(convert('-273.15', 'Température', '°C', 'K'), '0')
        for value, category, source, target in [('nan', 'Masse', 'kg', 'g'), ('-1', 'Température', 'K', '°C'), ('1', 'Masse', 'kg', 'm')]:
            with self.assertRaises(Error): convert(value, category, source, target)


if __name__ == '__main__': unittest.main()
