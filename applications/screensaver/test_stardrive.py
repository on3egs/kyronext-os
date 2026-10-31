"""Contrôles des trajectoires et des modes sans initialiser Qt."""
from pathlib import Path
import random
import runpy
import unittest

ROOT = Path(__file__).resolve().parents[2]
MODULE = runpy.run_path(str(ROOT / 'iso/config/includes.chroot/usr/local/bin/kyronext-stardrive'))
Flight = MODULE['Flight']
should_reduce_motion = MODULE['should_reduce_motion']


class FlightTests(unittest.TestCase):
    def test_projection_grows_towards_viewer(self):
        flight = Flight(1, 'red', random.Random(1))
        car = flight.cars[0]
        car.x, car.y, car.z = 0.1, 0.1, 1.0
        before = flight.project(car, 1000, 650)
        flight.advance(0.1)
        after = flight.project(car, 1000, 650)
        self.assertLess(car.z, 1.0)
        self.assertGreater(after[0], before[0])
        self.assertGreaterEqual(after[2], before[2])

    def test_recycling_keeps_depth_positive(self):
        flight = Flight(24, 'mixed', random.Random(2))
        for _ in range(10000):
            flight.advance(0.1)
        self.assertEqual(len(flight.cars), 24)
        self.assertTrue(all(0.1 <= car.z <= 1.15 for car in flight.cars))

    def test_modes_never_substitute_generic_car(self):
        self.assertEqual({car.asset for car in Flight(12, 'red').cars}, {'kitt.png'})
        self.assertEqual({car.asset for car in Flight(12, 'amber').cars}, {'karr.png'})
        self.assertEqual({car.asset for car in Flight(12, 'mixed').cars}, {'kitt.png', 'karr.png'})
        with self.assertRaises(ValueError):
            Flight(25)

    def test_user_preference_or_command_line_disables_motion(self):
        self.assertFalse(should_reduce_motion(False, False))
        self.assertTrue(should_reduce_motion(False, True))
        self.assertTrue(should_reduce_motion(True, False))
        self.assertFalse(should_reduce_motion(False, 'false'))
        self.assertTrue(should_reduce_motion(False, 'true'))


if __name__ == '__main__':
    unittest.main()
