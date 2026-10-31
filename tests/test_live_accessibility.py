import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
INCLUDES = ROOT / "iso/config/includes.chroot"


class LiveAccessibilityTests(unittest.TestCase):
    def test_live_image_includes_sudo_and_kde_capture_tool(self):
        packages = (ROOT / "iso/config/package-lists/desktop.list.chroot").read_text(encoding="utf-8")
        self.assertIn("\nsudo\n", "\n" + packages)
        self.assertIn("\nkde-spectacle\n", "\n" + packages)

    def test_live_account_password_is_optional_and_sudo_requires_authentication(self):
        script = (INCLUDES / "usr/local/lib/kyronext/apply-live-account-branding").read_text(encoding="utf-8")
        self.assertIn("getent group sudo >/dev/null || groupadd --system sudo", script)
        self.assertIn("--append --groups sudo kyronext", script)
        self.assertIn("password_hash_file=/etc/kyronext/live-password.hash", script)
        self.assertIn("--password=\"$live_password_hash\"", script)
        self.assertIn("usermod --comment='Kyronext OS' --lock --append --groups sudo kyronext", script)
        self.assertNotIn("--password='$6$", script)
        build_script = (ROOT / "scripts/build-iso.sh").read_text(encoding="utf-8")
        self.assertIn("KYRONEXT_LIVE_PASSWORD_HASH_FILE", build_script)
        self.assertIn("mode 0600", build_script)
        ssh_gate = (INCLUDES / "etc/systemd/system/ssh.service.d/10-kyronext-access.conf").read_text(encoding="utf-8")
        self.assertIn("ConditionPathExists=/etc/kyronext/ssh/authorized_keys", ssh_gate)
        hash_tool = (ROOT / "scripts/create-live-password-hash.sh").read_text(encoding="utf-8")
        self.assertIn("openssl passwd -6 -stdin", hash_tool)
        self.assertIn("set -o noclobber", hash_tool)
        self.assertIn("unset password confirmation", hash_tool)
        self.assertNotIn("passwd -l kyronext", script)
        self.assertIn("kyronext ALL=(ALL:ALL) ALL", script)
        self.assertIn("visudo -cf /etc/sudoers.d/kyronext", script)
        unit = (INCLUDES / "etc/systemd/system/live-config.service.d/20-kyronext-branding.conf").read_text(encoding="utf-8")
        self.assertIn("ExecStartPost=/usr/local/lib/kyronext/apply-live-account-branding", unit)
        self.assertFalse((INCLUDES / "usr/lib/live/config/9990-kyronext-security").exists())

    def test_legacy_boot_selects_x11_and_disables_compositing(self):
        packages = (ROOT / "iso/config/package-lists/desktop.list.chroot").read_text(encoding="utf-8")
        self.assertIn("\nxserver-xorg-video-fbdev\n", "\n" + packages)
        hook = (ROOT / "iso/config/hooks/live/0950-boot-branding.hook.binary").read_text(encoding="utf-8")
        self.assertIn("(Legacy Compatibility)", hook)
        self.assertIn('" nomodeset "', hook)
        self.assertIn("kyronext.legacy=1", hook)

        script = (INCLUDES / "usr/local/lib/kyronext/apply-live-account-branding").read_text(encoding="utf-8")
        self.assertIn("grep -Fqw -- 'kyronext.legacy=1' /proc/cmdline", script)
        self.assertIn("DisplayServer=x11", script)
        self.assertIn("Session=plasmax11.desktop", script)
        self.assertIn("if [ -c /dev/fb0 ]", script)
        self.assertIn('Driver "fbdev"', script)
        self.assertIn("90-kyronext-legacy-framebuffer.conf", script)
        self.assertIn("kwriteconfig6 --file", script)
        self.assertIn("--group Compositing --key Enabled false", script)

    def test_capture_uses_spectacle_when_native_capture_is_unavailable(self):
        source = (ROOT / "applications/suite/accessories/capture.cpp").read_text(encoding="utf-8")
        self.assertIn('QStringLiteral("spectacle")', source)
        self.assertIn('QStringLiteral("--region")', source)
        self.assertIn("if (!supported()) { startSpectacle(region); return; }", source)


if __name__ == "__main__":
    unittest.main()
