"""Tests de génération locale, préférences préservées et URL sans shell."""
from pathlib import Path
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'iso/config/includes.chroot'
path = SOURCE / 'usr/lib/kyronext/kyronext_firefox.py'
spec = importlib.util.spec_from_file_location('firefox_test_helper', path)
helper = importlib.util.module_from_spec(spec); spec.loader.exec_module(helper)


class ProfileTests(unittest.TestCase):
    def setUp(self):
        (ROOT / 'build').mkdir(exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(prefix='firefox-test-', dir=ROOT / 'build')
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def test_bootstrap_only_once_preserves_user_homepage_and_css(self):
        homepage = helper.refresh_start(self.root, [], [])
        profile, created = helper.prepare_profile(self.root, homepage)
        self.assertTrue(created)
        prefs = (profile / 'prefs.js').read_text(encoding='utf-8')
        self.assertIn('toolkit.legacyUserProfileCustomizations.stylesheets", true', prefs)
        self.assertIn('firefox-compact-dark@mozilla.org', prefs)
        self.assertIn(homepage.resolve().as_uri(), prefs)
        self.assertNotIn('security.sandbox', prefs)
        self.assertNotIn('disable', prefs)
        (profile / 'prefs.js').write_text('user_pref("browser.startup.homepage", "https://example.org/");', encoding='utf-8')
        (profile / 'chrome/userChrome.css').write_text('/* Choix utilisateur */', encoding='utf-8')
        original = (profile / 'prefs.js').read_bytes()
        again, created = helper.prepare_profile(self.root, homepage)
        self.assertFalse(created); self.assertEqual(again, profile)
        self.assertEqual((profile / 'prefs.js').read_bytes(), original)
        self.assertEqual((profile / 'chrome/userChrome.css').read_text(encoding='utf-8'), '/* Choix utilisateur */')
        self.assertFalse((profile / 'user.js').exists())

    def test_start_escapes_labels_and_honors_visibility(self):
        links = [
            {'url': 'https://example.org/?a=1&b=2', 'label': '<img src=x onerror=bad()>', 'visible': True},
            {'url': 'https://hidden.example/', 'label': 'Masqué', 'visible': False},
            {'url': 'javascript:alert(1)', 'label': 'Invalide'},
        ]
        page = helper.start_html(links, ['<script>bad()</script>'])
        self.assertIn('&lt;img src=x onerror=bad()&gt;', page)
        self.assertNotIn('<script>', page)
        self.assertNotIn('hidden.example', page)
        self.assertNotIn('javascript:', page)
        self.assertIn('default-src', page)
        self.assertIn('https://example.org/?a=1&amp;b=2', page)
        self.assertNotIn('<img ', page)

    def test_urls_and_plain_command_cannot_inject_firefox_options(self):
        for value in ['--no-sandbox', 'javascript:alert(1)', 'file:///etc/passwd', 'https://user:pass@example.org/', 'https://example.org/%0a--profile', 'https://bad..host/', 'https://example.org/ a', 'https://example.org:99999/', 'https://example.org/`cmd`']:
            with self.subTest(value=value), self.assertRaises(ValueError): helper.validate_argument_url(value)
        urls = ['https://example.org/?q=a&b=2', 'http://127.0.0.1:8000/']
        command = helper.command('/usr/bin/firefox-esr', '/tmp/profile with spaces', urls)
        self.assertEqual(command, ['/usr/bin/firefox-esr', '--no-remote', '--profile', '/tmp/profile with spaces', *urls])
        self.assertEqual(helper.command('/usr/bin/firefox-esr', None, urls, plain=True), ['/usr/bin/firefox-esr', *urls])

    def test_first_launch_explicitly_opens_generated_local_start_page(self):
        homepage = self.root / 'start/index.html'
        homepage.parent.mkdir()
        homepage.write_text('<title>Kyronext Start</title>', encoding='utf-8')
        command = helper.command('/usr/bin/firefox-esr', self.root / 'profile', [], homepage=homepage)
        self.assertEqual(command, [
            '/usr/bin/firefox-esr', '--no-remote', '--profile', str(self.root / 'profile'),
            '--new-window', homepage.resolve().as_uri(),
        ])

    def test_missing_local_start_page_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'absente ou invalide'):
            helper.command('/usr/bin/firefox-esr', self.root / 'profile', [], homepage=self.root / 'missing.html')

    def test_existing_unknown_profile_is_not_rewritten(self):
        profile = self.root / 'profile'; profile.mkdir()
        (profile / 'prefs.js').write_text('user preferences', encoding='utf-8')
        _, created = helper.prepare_profile(self.root, self.root / 'index.html')
        self.assertFalse(created)
        self.assertFalse((profile / 'chrome').exists())
        self.assertEqual((profile / 'prefs.js').read_text(), 'user preferences')

    def test_xdg_link_overrides_are_validated_then_escaped(self):
        config = self.root / 'config/kyronext/links.json'; config.parent.mkdir(parents=True)
        config.write_text(json.dumps({'schema_version': 1, 'links': [
            {'id': 'kitt-franco-belge', 'label': 'Mon <portail>', 'url': 'https://example.org/local'},
            {'id': 'kironext-be', 'visible': False},
        ]}), encoding='utf-8')
        with patch.dict(os.environ, {'XDG_CONFIG_HOME': str(self.root / 'config')}):
            links, warnings = helper.load_links()
        self.assertFalse(warnings)
        page = helper.start_html(links, warnings)
        self.assertIn('Mon &lt;portail&gt;', page)
        self.assertIn('https://example.org/local', page)
        self.assertNotIn('https://kironext.be', page)
        self.assertIn('https://kyronext.eu', page)

    @unittest.skipUnless(sys.platform == 'linux', 'Verrou POSIX vérifié dans la VM Debian')
    def test_second_launcher_refuses_busy_profile_without_deleting_lock(self):
        code = 'import importlib.util,sys; s=importlib.util.spec_from_file_location("h",sys.argv[1]); h=importlib.util.module_from_spec(s); s.loader.exec_module(h);\nwith h.launcher_lock(sys.argv[2]): print("unexpected")'
        with helper.launcher_lock(self.root):
            result = subprocess.run([sys.executable, '-c', code, str(path), str(self.root)], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('déjà ouvert', result.stderr.decode('utf-8'))
        self.assertTrue((self.root / '.launcher.lock').exists())

    @unittest.skipUnless(sys.platform == 'linux', 'Verrou Firefox POSIX vérifié dans la VM Debian')
    def test_firefox_parentlock_is_never_removed(self):
        profile = self.root / 'profile'; profile.mkdir()
        lock = profile / '.parentlock'
        code = 'import fcntl,sys; f=open(sys.argv[1],"a+b"); fcntl.lockf(f.fileno(),fcntl.LOCK_EX); print("ready",flush=True); sys.stdin.readline()'
        process = subprocess.Popen([sys.executable, '-c', code, str(lock)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        try:
            self.assertEqual(process.stdout.readline().strip(), 'ready')
            with self.assertRaisesRegex(RuntimeError, 'utilisé par Firefox'):
                with helper.launcher_lock(self.root): pass
            self.assertTrue(lock.exists())
        finally:
            process.communicate('stop\n', timeout=5)


if __name__ == '__main__': unittest.main()
