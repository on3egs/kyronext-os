import importlib.util
from pathlib import Path
import tempfile
from threading import Event
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'iso/config/includes.chroot/usr/lib/kyronext/file_search.py'
SPEC = importlib.util.spec_from_file_location('kyronext_file_search', SOURCE)
file_search = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(file_search)


class FileSearchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def test_only_indexes_selected_visible_file_names(self):
        (self.root / 'Music').mkdir()
        (self.root / 'Music' / 'Daft Punk - One More Time.mp3').write_bytes(b'private audio')
        (self.root / '.hidden').write_text('hidden', encoding='utf-8')
        (self.root / 'secret.txt').write_text('private needle in file contents', encoding='utf-8')
        result = file_search.index_paths([self.root / 'Music'])
        self.assertEqual([Path(path).name for _, path in result['files']],
                         ['Daft Punk - One More Time.mp3'])
        self.assertEqual(file_search.search_paths(result, 'one more'),
                         [str(self.root / 'Music' / 'Daft Punk - One More Time.mp3')])
        self.assertEqual(file_search.search_paths(result, 'private'), [])

    def test_deduplicates_roots_and_returns_case_insensitive_order(self):
        (self.root / 'zeta.txt').touch()
        (self.root / 'Alpha.txt').touch()
        result = file_search.index_paths([self.root, self.root])
        self.assertEqual([Path(path).name for _, path in result['files']],
                         ['Alpha.txt', 'zeta.txt'])
        self.assertEqual(len(file_search.search_paths(result, 'TXT')), 2)

    def test_file_cap_and_result_cap_are_reported(self):
        for number in range(4):
            (self.root / f'item-{number}.txt').touch()
        result = file_search.index_paths([self.root], max_files=2)
        self.assertTrue(result['truncated'])
        self.assertEqual(len(result['files']), 2)
        self.assertEqual(len(file_search.search_paths({'files': [('x', str(i)) for i in range(8)]},
                                                       'x', limit=2)), 2)

    def test_directory_limit_bounds_empty_directory_trees(self):
        for number in range(4):
            (self.root / f'dir-{number}').mkdir()
        result = file_search.index_paths([self.root], max_directories=1)
        self.assertTrue(result['directory_limited'])
        self.assertEqual(result['directories'], 1)

    def test_cancellation_is_checked_before_walking(self):
        (self.root / 'visible.txt').touch()
        cancel = Event()
        cancel.set()
        result = file_search.index_paths([self.root], cancel_event=cancel)
        self.assertTrue(result['cancelled'])
        self.assertEqual(result['files'], [])

    def test_deadline_is_reported(self):
        (self.root / 'visible.txt').touch()
        with patch.object(file_search.time, 'monotonic', side_effect=[0.0, 0.2]):
            result = file_search.index_paths([self.root], deadline_seconds=0.1)
        self.assertTrue(result['timed_out'])
        self.assertEqual(result['files'], [])

    def test_progress_callback_reports_work_without_file_contents(self):
        for number in range(64):
            (self.root / f'dir-{number}').mkdir()
        progress = []
        result = file_search.index_paths([self.root], progress=progress.append)
        self.assertGreaterEqual(result['directories'], 64)
        self.assertTrue(progress)
        self.assertIn('directories', progress[0])

    def test_symlink_root_is_not_followed(self):
        outside = self.root / 'outside'
        outside.mkdir()
        (outside / 'private.txt').touch()
        link = self.root / 'linked'
        try:
            link.symlink_to(outside, target_is_directory=True)
        except (OSError, NotImplementedError):
            self.skipTest('Création de liens symboliques indisponible')
        self.assertEqual(file_search.index_paths([link])['files'], [])

    def test_rejects_unbounded_limits(self):
        with self.assertRaises(ValueError):
            file_search.index_paths([self.root], max_files=1000000)
        with self.assertRaises(ValueError):
            file_search.search_paths({'files': []}, 'x', limit=1001)


if __name__ == '__main__':
    unittest.main()
