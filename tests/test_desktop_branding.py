import configparser
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
INCLUDES = ROOT / "iso/config/includes.chroot"


class DesktopBrandingTests(unittest.TestCase):
    def test_default_session_selects_kyronext_look_and_feel(self):
        config = configparser.ConfigParser()
        config.read(INCLUDES / "etc/skel/.config/kdeglobals", encoding="utf-8")
        self.assertEqual(config["KDE"]["LookAndFeelPackage"], "org.kyronext.desktop")
        self.assertEqual(config["Icons"]["Theme"], "Kyronext")

    def test_brand_icons_cover_kickoff_and_distribution_icon_names(self):
        root = INCLUDES / "usr/share/icons/Kyronext/scalable/places"
        logo = (INCLUDES / "usr/share/icons/Kyronext/scalable/apps/kyronext-logo.svg").read_bytes()
        for name in (
            "start-here-kde.svg",
            "start-here-kde-plasma.svg",
            "start-here-kde-symbolic.svg",
            "start-here-kde-plasma-symbolic.svg",
            "distributor-logo.svg",
        ):
            with self.subTest(icon=name):
                self.assertEqual((root / name).read_bytes(), logo)

        for context in ("actions", "applets"):
            for name in ("user-identity.svg", "user-identity-symbolic.svg"):
                with self.subTest(context=context, icon=name):
                    self.assertEqual((root.parent / context / name).read_bytes(), logo)

    def test_shutdown_dialog_uses_kyronext_identity_and_standard_power_actions(self):
        theme = INCLUDES / "usr/share/plasma/look-and-feel/org.kyronext.desktop"
        metadata = json.loads((theme / "metadata.json").read_text(encoding="utf-8"))
        self.assertEqual(metadata["KPlugin"]["Id"], "org.kyronext.desktop")
        qml = (theme / "contents/logout/Logout.qml").read_text(encoding="utf-8")
        self.assertIn("/var/lib/AccountsService/icons/kyronext.png", qml)
        self.assertIn('name: "Kyronext OS"', qml)
        self.assertIn('icon.name: "system-shutdown"', qml)
        self.assertIn("haltRequested()", qml)

    def test_live_account_avatar_is_present_and_matches_qml_path(self):
        account = INCLUDES / "var/lib/AccountsService/users/kyronext"
        self.assertIn("Icon=/var/lib/AccountsService/icons/kyronext.png", account.read_text(encoding="utf-8"))
        self.assertTrue((INCLUDES / "var/lib/AccountsService/icons/kyronext.png").is_file())
        self.assertEqual(
            (INCLUDES / "etc/skel/.face").read_bytes(),
            (INCLUDES / "var/lib/AccountsService/icons/kyronext.png").read_bytes(),
        )

    def test_live_boot_sets_a_kyronext_account_display_name(self):
        hook = (ROOT / "iso/config/hooks/live/0950-boot-branding.hook.binary").read_text(encoding="utf-8")
        self.assertIn("user-fullname=Kyronext", hook)
        config = (ROOT / "iso/auto/config").read_text(encoding="utf-8")
        self.assertIn("$common user-fullname=Kyronext", config)

        systemd_override = INCLUDES / "etc/systemd/system/live-config.service.d/20-kyronext-branding.conf"
        self.assertIn("ExecStartPost=/usr/local/lib/kyronext/apply-live-account-branding", systemd_override.read_text(encoding="utf-8"))
        ordering = INCLUDES / "etc/systemd/system/display-manager.service.d/20-kyronext-branding-order.conf"
        self.assertIn("After=live-config.service", ordering.read_text(encoding="utf-8"))
        branding_script = INCLUDES / "usr/local/lib/kyronext/apply-live-account-branding"
        branding = branding_script.read_text(encoding="utf-8")
        self.assertIn("--comment='Kyronext OS'", branding)
        self.assertIn("--append --groups sudo kyronext", branding)

    def test_login_screen_selects_upstream_breeze_with_kyronext_branding(self):
        config = (INCLUDES / "etc/sddm.conf.d/50-kyronext-theme.conf").read_text(encoding="utf-8")
        theme = INCLUDES / "usr/share/sddm/themes/breeze/theme.conf.user"
        self.assertIn("Current=breeze", config)
        self.assertIn("logo=/usr/share/icons/hicolor/scalable/apps/kyronext-logo.svg", theme.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
