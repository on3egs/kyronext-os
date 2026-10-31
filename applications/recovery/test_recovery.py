import json
import os
from pathlib import Path
import stat
import tempfile
import unittest
from datetime import datetime, timezone

import kyronext_recovery as recovery


class FakeRunner:
    def __init__(self):
        self.calls = []

    def __call__(self, argv, timeout=4.0):
        self.calls.append(list(argv))
        if argv[0] == "/test/systemctl":
            if argv[-1] == "sddm.service":
                return recovery.subprocess.CompletedProcess(argv, 0,
                    "LoadState=loaded\nActiveState=inactive\nSubState=dead\n", "")
            return recovery.subprocess.CompletedProcess(argv, 0,
                "LoadState=loaded\nActiveState=active\nSubState=running\n", "")
        rows = [
            {"__REALTIME_TIMESTAMP": "1770000000000000", "PRIORITY": "3",
             "_SYSTEMD_UNIT": "kyronext-test.service",
             "MESSAGE": "password=secret contact alice@example.test ip 10.0.0.4 fe80::1 file /home/alice/private.txt"},
            {"__REALTIME_TIMESTAMP": "1770000001000000", "PRIORITY": "4",
             "_SYSTEMD_UNIT": "", "MESSAGE": "Second diagnostic"},
        ]
        return recovery.subprocess.CompletedProcess(argv, 0,
            "\n".join(json.dumps(row) for row in rows) + "\n", "")


class RecoveryTests(unittest.TestCase):
    def test_report_collects_limited_local_facts_and_redacts_common_secrets(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            release = root / "os-release"
            release.write_text('PRETTY_NAME="Kyronext Test"\nID=kyronext\nVERSION_ID=0.1\n', encoding="utf-8")
            runner = FakeRunner()
            report = recovery.collect_report(
                os_release_path=release, root_path=root, runner=runner,
                systemctl="/test/systemctl", journalctl="/test/journalctl",
                now=datetime(2026, 10, 31, tzinfo=timezone.utc))
        self.assertEqual(report["system"]["distribution"]["PRETTY_NAME"], "Kyronext Test")
        self.assertIn("free_bytes", report["root_filesystem"])
        self.assertEqual(report["services"][0]["active_state"], "active")
        self.assertEqual(report["services"][2]["active_state"], "inactive")
        self.assertEqual(len(report["recent_errors"]["entries"]), 2)
        message = report["recent_errors"]["entries"][0]["message"]
        self.assertIn("password=[masqué]", message)
        self.assertNotIn("alice@example.test", message)
        self.assertNotIn("10.0.0.4", message)
        self.assertNotIn("fe80::1", message)
        self.assertNotIn("/home/alice", message)
        self.assertNotIn("private.txt", message)
        self.assertFalse(report["privacy"]["network_access"])
        self.assertTrue(all("sudo" not in str(call) for call in runner.calls))

    def test_missing_system_services_and_journal_are_reported_not_guessed(self):
        with tempfile.TemporaryDirectory() as temporary:
            report = recovery.collect_report(root_path=Path(temporary), systemctl="",
                                             journalctl="", runner=lambda *_args, **_kwargs: self.fail("must not run"))
        self.assertTrue(all(item["status"] == "indisponible" for item in report["services"]))
        self.assertEqual(report["recent_errors"], {"status": "indisponible", "entries": []})

    def test_text_export_and_private_exclusive_creation(self):
        report = {"created_at_utc": "2026-10-31T00:00:00+00:00",
                  "system": {"distribution": {"PRETTY_NAME": "Kyronext"},
                             "kernel": "test-kernel", "architecture": "x86_64"},
                  "root_filesystem": {"mount": "/", "free_bytes": 100,
                                      "used_percent": 50.0},
                  "services": [{"service": "sddm.service", "active_state": "active"}],
                  "recent_errors": {"status": "disponible", "entries": []},
                  "privacy": {"note": "À relire"}}
        text = recovery.render_text(report)
        self.assertIn("KYRONEXT RECOVERY", text)
        self.assertIn("sddm.service : active", text)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "recovery.json"
            recovery.write_new_file(output, json.dumps(report))
            self.assertEqual(json.loads(output.read_text(encoding="utf-8"))["system"]["kernel"], "test-kernel")
            if os.name == "posix":
                self.assertEqual(stat.S_IMODE(output.stat().st_mode), 0o600)
            with self.assertRaises(FileExistsError):
                recovery.write_new_file(output, "replacement")
            self.assertNotEqual(output.read_text(encoding="utf-8"), "replacement")

    @unittest.skipUnless(hasattr(os, "symlink"), "symlinks unavailable")
    def test_existing_symlink_is_never_followed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            destination = root / "destination"
            link = root / "report.json"
            destination.write_text("keep", encoding="utf-8")
            try:
                link.symlink_to(destination)
            except OSError as exc:
                self.skipTest(f"symlink creation unavailable: {exc}")
            with self.assertRaises(FileExistsError):
                recovery.write_new_file(link, "overwrite")
            self.assertEqual(destination.read_text(encoding="utf-8"), "keep")


if __name__ == "__main__":
    unittest.main()
