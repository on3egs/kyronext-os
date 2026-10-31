"""Contrôles du catalogue local et de la création non destructive de raccourcis."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('desktop_catalog', ROOT / 'iso/config/includes.chroot/usr/lib/kyronext/desktop_catalog.py')
catalog = importlib.util.module_from_spec(spec)
spec.loader.exec_module(catalog)


class CatalogTests(unittest.TestCase):
    def setUp(self):
        (ROOT / 'build').mkdir(exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(dir=ROOT / 'build')
        self.root = Path(self.temporary.name)

    def tearDown(self):
        self.temporary.cleanup()

    def entry(self, folder, name, extra=''):
        folder.mkdir(parents=True, exist_ok=True)
        path = folder / name
        path.write_text('[Desktop Entry]\nType=Application\nName=Test\nDBusActivatable=true\n' + extra, encoding='utf-8')
        return path

    def test_user_hidden_override_suppresses_system_application(self):
        user, system = self.root / 'user', self.root / 'system'
        self.entry(user, 'app.desktop', 'Hidden=true\n')
        self.entry(system, 'app.desktop')
        self.assertEqual(catalog.scan_applications([user, system]), [])

    def test_missing_executable_is_not_advertised(self):
        self.entry(self.root, 'missing.desktop', 'TryExec=kyronext-nonexistent-test-application\n')
        self.assertEqual(catalog.scan_applications([self.root]), [])

    def test_malformed_entry_does_not_hide_valid_other_application(self):
        (self.root / 'broken.desktop').write_text('invalid', encoding='utf-8')
        self.entry(self.root, 'valid.desktop')
        self.assertEqual([app['id'] for app in catalog.scan_applications([self.root])], ['valid.desktop'])

    def test_pin_never_overwrites_an_existing_shortcut(self):
        path = self.entry(self.root / 'source', 'app.desktop')
        desktop = self.root / 'desktop'
        target = catalog.pin_application(path, desktop)
        target.write_text('precious content', encoding='utf-8')
        with self.assertRaises(FileExistsError):
            catalog.pin_application(path, desktop)
        self.assertEqual(target.read_text(encoding='utf-8'), 'precious content')


if __name__ == '__main__':
    unittest.main()
