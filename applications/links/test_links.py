"""Tests locaux des liens : aucune connexion Internet ou ouverture de navigateur."""
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
path = ROOT / 'iso/config/includes.chroot/usr/lib/kyronext/kyronext_links.py'
spec = importlib.util.spec_from_file_location('kyronext_links', path)
links = importlib.util.module_from_spec(spec)
spec.loader.exec_module(links)


class LinksTests(unittest.TestCase):
    def setUp(self):
        (ROOT / 'build').mkdir(exist_ok=True)
        self.scratch = tempfile.TemporaryDirectory(prefix='links-test-', dir=ROOT / 'build')
        self.addCleanup(self.scratch.cleanup)
        self.user = Path(self.scratch.name) / 'links.json'

    def override(self, entries):
        self.user.write_text(json.dumps({'schema_version': 1, 'links': entries}), encoding='utf-8')

    def test_exact_requested_defaults_and_no_network(self):
        configured, warnings = links.load_links(user_path=self.user)
        self.assertFalse(warnings)
        self.assertEqual([item['url'] for item in configured], ['https://www.kitt-franco-belge.be', 'https://kironext.be', 'https://kyronext.eu'])

    def test_rejects_unsafe_schemes_credentials_controls_and_ambiguous_hosts(self):
        for url in ['file:///etc/passwd', 'javascript:alert(1)', 'http://example.com',
                    'https://user:secret@example.com', 'https://@example.com',
                    'https://example.com/%0aBAD', 'https://example.com\n',
                    'https://example.com\\@other.test', 'https://bad host.test',
                    'https://example.com:0', 'https://-invalid.test', 'https://a..test',
                    'https://example.com/<img>', 'https://example.com/"quoted"']:
            with self.subTest(url=url):
                with self.assertRaises(links.LinkConfigError):
                    links.validate_url(url)

    def test_invalid_json_preserves_system_defaults(self):
        self.user.write_text('{not json', encoding='utf-8')
        configured, warnings = links.load_links(user_path=self.user)
        self.assertEqual(len(configured), 3)
        self.assertTrue(warnings)

    def test_malformed_override_cannot_crash_or_add_id(self):
        self.override([{'id': []}, {'id': 'unknown', 'url': 'https://example.test'},
                       {'id': 'kyronext-eu', 'visible': 'false'}])
        configured, warnings = links.load_links(user_path=self.user)
        self.assertEqual(len(configured), 3)
        self.assertTrue(configured[2]['visible'])
        self.assertEqual(len(warnings), 3)

    def test_override_hides_and_custom_url_is_marked_unverified(self):
        self.override([{'id': 'kyronext-eu', 'visible': False, 'url': 'https://example.test/custom'}])
        configured, warnings = links.load_links(user_path=self.user)
        self.assertFalse(warnings)
        self.assertFalse(configured[2]['visible'])
        self.assertEqual(configured[2]['verification']['status'], 'to-confirm')

    def test_xdg_path_is_user_scoped(self):
        with patch.dict(os.environ, {'XDG_CONFIG_HOME': self.scratch.name}):
            self.assertEqual(links.user_config_path(), Path(self.scratch.name) / 'kyronext/links.json')

    def test_missing_browser_does_not_dispatch(self):
        dispatched, notices = [], []
        self.assertFalse(links.open_link(None, {'url': 'https://kyronext.eu'},
            opener=lambda url: dispatched.append(url), browser_probe=lambda: None,
            notify=lambda title, text: notices.append(title)))
        self.assertEqual(dispatched, [])
        self.assertEqual(notices, ['Navigateur indisponible'])

    def test_mock_dispatch_uses_url_as_data_not_a_shell_command(self):
        dispatched = []
        url = 'https://example.test/?q=$(not-a-command)'
        self.assertTrue(links.open_link(None, {'url': url},
            opener=lambda address: dispatched.append(address) or True,
            browser_probe=lambda: '/mock/firefox-esr', notify=lambda *unused: self.fail()))
        self.assertEqual(dispatched, [url])


if __name__ == '__main__':
    unittest.main()
