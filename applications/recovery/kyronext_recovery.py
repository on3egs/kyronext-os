#!/usr/bin/env python3
"""Kyronext Recovery: local read-only diagnostic report for Debian/Linux."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone


SERVICES = ("systemd-logind.service", "NetworkManager.service", "sddm.service")
MAX_JOURNAL_ENTRIES = 20
MAX_MESSAGE_LENGTH = 500
_REDACTIONS = (
    (re.compile(r"\b[\w.+-]+@[\w.-]+\.[A-Za-z]{2,}\b"), "[courriel masqué]"),
    (re.compile(r"\b(?:\d{1,3}\.){3}\d{1,3}\b"), "[adresse IP masquée]"),
    (re.compile(r"(?<![0-9A-Fa-f:])(?:[0-9A-Fa-f]{0,4}:){2,7}[0-9A-Fa-f]{0,4}(?![0-9A-Fa-f:])"), "[adresse IP masquée]"),
    (re.compile(r"\b(?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}\b"), "[adresse MAC masquée]"),
    (re.compile(r"(?i)(password|passwd|token|secret)=\S+"), r"\1=[masqué]"),
    (re.compile(r"/(?:home|root)/[^\s,;:]+"), "[chemin privé masqué]"),
    (re.compile(r"https?://[^\s,;]+", re.IGNORECASE), "[URL masquée]"),
)


def _read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace").strip()
    except OSError:
        return ""


def read_os_release(path: Path = Path("/etc/os-release")) -> dict[str, str]:
    values: dict[str, str] = {}
    for line in _read(path).splitlines():
        key, separator, value = line.partition("=")
        if separator and key in {"PRETTY_NAME", "ID", "VERSION_ID"}:
            values[key] = value.strip().strip('"').strip("'")[:160]
    return values


def run_command(argv: list[str], timeout: float = 4.0) -> subprocess.CompletedProcess[str]:
    """Run a fixed argv without a shell; never elevate or access the network."""
    try:
        return subprocess.run(argv, check=False, capture_output=True, text=True,
                              timeout=timeout, shell=False, env={**os.environ, "LC_ALL": "C"})
    except (OSError, subprocess.TimeoutExpired) as exc:
        return subprocess.CompletedProcess(argv, 127, "", str(exc))


def _system_binary(name: str) -> str | None:
    """Only use root-owned standard Debian locations; don't resolve via PATH."""
    for directory in (Path("/usr/bin"), Path("/bin")):
        candidate = directory / name
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return str(candidate)
    return None


def _service_status(executable: str | None, service: str, runner) -> dict[str, str]:
    if not executable:
        return {"service": service, "status": "indisponible"}
    result = runner([executable, "show", "--no-pager",
                     "--property=LoadState,ActiveState,SubState", service])
    if result.returncode != 0:
        return {"service": service, "status": "indisponible"}
    fields = dict(line.split("=", 1) for line in result.stdout.splitlines() if "=" in line)
    if fields.get("LoadState") == "not-found":
        return {"service": service, "status": "absent"}
    return {"service": service,
            "load_state": fields.get("LoadState", "inconnu"),
            "active_state": fields.get("ActiveState", "inconnu"),
            "sub_state": fields.get("SubState", "inconnu")}


def _clean_message(message: str) -> str:
    text = " ".join(message.replace("\x00", " ").split())
    for pattern, replacement in _REDACTIONS:
        text = pattern.sub(replacement, text)
    return text[:MAX_MESSAGE_LENGTH]


def _journal_errors(executable: str | None, runner) -> dict:
    if not executable:
        return {"status": "indisponible", "entries": []}
    result = runner([executable, "--no-pager", "--quiet", "--boot=0",
                     "--priority=err..emerg", f"--lines={MAX_JOURNAL_ENTRIES}",
                     "--output=json"], timeout=6.0)
    if result.returncode != 0:
        return {"status": "indisponible ou accès refusé", "entries": []}
    entries = []
    for line in result.stdout.splitlines()[-MAX_JOURNAL_ENTRIES:]:
        try:
            record = json.loads(line)
        except (json.JSONDecodeError, TypeError):
            continue
        entries.append({
            "timestamp_us": str(record.get("__REALTIME_TIMESTAMP", ""))[:24],
            "priority": str(record.get("PRIORITY", ""))[:2],
            "unit": str(record.get("_SYSTEMD_UNIT", ""))[:120],
            "message": _clean_message(str(record.get("MESSAGE", ""))),
        })
    return {"status": "disponible", "entries": entries}


def collect_report(*, os_release_path: Path = Path("/etc/os-release"), root_path: Path = Path("/"),
                   runner=run_command, systemctl: str | None = None,
                   journalctl: str | None = None, now: datetime | None = None) -> dict:
    try:
        usage = shutil.disk_usage(root_path)
        disk = {"mount": str(root_path), "total_bytes": usage.total,
                "free_bytes": usage.free,
                "used_percent": round(100 * (usage.total - usage.free) / usage.total, 1)
                if usage.total else None}
    except OSError:
        disk = {"mount": str(root_path), "status": "indisponible"}
    systemctl = systemctl if systemctl is not None else _system_binary("systemctl")
    journalctl = journalctl if journalctl is not None else _system_binary("journalctl")
    timestamp = now or datetime.now(timezone.utc)
    return {
        "component": "Kyronext Recovery — diagnostic local",
        "schema_version": 1,
        "created_at_utc": timestamp.astimezone(timezone.utc).isoformat(),
        "system": {"distribution": read_os_release(os_release_path),
                   "kernel": platform.release(), "architecture": platform.machine()},
        "root_filesystem": disk,
        "services": [_service_status(systemctl, name, runner) for name in SERVICES],
        "recent_errors": _journal_errors(journalctl, runner),
        "privacy": {
            "network_access": False, "privilege_escalation": False,
            "data_collection": "local et à la demande uniquement",
            "note": "Relire le rapport avant partage. Les journaux peuvent contenir des informations privées.",
        },
        "limitations": [
            "Le diagnostic ne répare pas le système et ne garantit pas son intégrité.",
            "Les services absents ou inactifs peuvent être normaux selon la session.",
            "Les journaux sont limités aux erreurs accessibles à l'utilisateur courant.",
        ],
    }


def render_text(report: dict) -> str:
    system = report["system"]
    dist = system["distribution"].get("PRETTY_NAME", "Distribution inconnue")
    disk = report["root_filesystem"]
    lines = ["KYRONEXT RECOVERY — diagnostic local en lecture seule",
             f"Date UTC : {report['created_at_utc']}",
             f"Système : {dist}", f"Noyau : {system['kernel']} ({system['architecture']})"]
    if "free_bytes" in disk:
        lines.append(f"Espace libre sur {disk['mount']} : {disk['free_bytes']} octets ({disk['used_percent']} % utilisés)")
    else:
        lines.append("Espace disque : indisponible")
    lines.append("Services :")
    for item in report["services"]:
        status = item.get("active_state") or item.get("status", "indisponible")
        lines.append("  " + item["service"] + " : " + status)
    errors = report["recent_errors"]
    lines.append(f"Erreurs récentes accessibles : {len(errors['entries'])} ({errors['status']})")
    for entry in errors["entries"]:
        lines.append(f"  [{entry['priority'] or '?'}] {entry['unit']}: {entry['message']}")
    lines.append(report["privacy"]["note"])
    return "\n".join(lines) + "\n"


def write_new_file(path: Path, content: str) -> None:
    """Create a private file exclusively; never follow/overwrite an existing path."""
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    if hasattr(os, "O_NOFOLLOW"):
        flags |= os.O_NOFOLLOW
    descriptor = os.open(path, flags, 0o600)
    with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
        stream.write(content)
        stream.flush()
        os.fsync(stream.fileno())


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--format", choices=("text", "json"), default="text")
    parser.add_argument("--output", type=Path, help="fichier neuf choisi par vous ; sans cette option, affiche dans le terminal")
    args = parser.parse_args(argv)
    if not sys.platform.startswith("linux"):
        parser.error("Kyronext Recovery s'exécute sous Linux uniquement.")
    if os.geteuid() == 0:
        parser.error("Lancez cet outil depuis votre session utilisateur, sans sudo.")
    report = collect_report()
    output = json.dumps(report, ensure_ascii=False, indent=2) + "\n" if args.format == "json" else render_text(report)
    if args.output:
        try:
            write_new_file(args.output, output)
        except OSError as exc:
            parser.exit(1, f"Rapport non enregistré ({exc}). Aucun fichier existant n'a été remplacé.\n")
        print(f"Rapport Kyronext Recovery créé : {args.output}")
    else:
        sys.stdout.write(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
