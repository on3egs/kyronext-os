#!/usr/bin/env python3
"""Plafonds décimaux : pilotes/firmwares AJOUTÉS <1 Go ; ISO finale <16 Go."""
import argparse
import json
import subprocess
from pathlib import Path

MAX_HARDWARE_BYTES = 1_000_000_000
MAX_ISO_BYTES = 16_000_000_000

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--root', type=Path)
parser.add_argument('--iso', type=Path)
parser.add_argument('--report', type=Path)
args = parser.parse_args()
if not args.root and not args.iso:
    parser.error('--root et/ou --iso requis')
report = {
    'hardware_added_limit_bytes': MAX_HARDWARE_BYTES,
    'iso_size_limit_bytes': MAX_ISO_BYTES,
}
if args.root:
    manifest = Path(__file__).resolve().parents[1] / 'iso/config/package-lists/hardware-connectivity.list.chroot'
    packages = [line.strip() for line in manifest.read_text().splitlines() if line.strip() and not line.lstrip().startswith('#')]
    packages.append('gobi-loader')  # paquet local 0.7, hors résolution APT Trixie
    result = subprocess.run(['chroot', str(args.root), 'dpkg-query', '-W', '-f=${Package}\t${Installed-Size}\t${db:Status-Status}\n', *packages], text=True, capture_output=True, check=True)
    entries = {}
    for line in result.stdout.splitlines():
        name, size, status = line.split('\t')
        if status != 'installed':
            raise SystemExit(f'Paquet non installé : {name}')
        entries[name] = int(size) * 1024
    report['hardware_selected_packages_installed_bytes'] = sum(entries.values())
    report['packages'] = entries
    reserve = args.root / 'usr/share/kyronext/drivers/offline'
    report['offline_reserve_bytes'] = sum(p.stat().st_size for p in reserve.glob('*.deb'))
    report['hardware_and_offline_bytes'] = sum(entries.values()) + report['offline_reserve_bytes']
    if report['hardware_and_offline_bytes'] >= MAX_HARDWARE_BYTES:
        raise SystemExit('Plafond pilotes/firmwares ajoutés de 1 Go dépassé')
if args.iso:
    report['iso_bytes'] = args.iso.stat().st_size
    if not 0 < report['iso_bytes'] < MAX_ISO_BYTES:
        raise SystemExit('ISO vide ou plafond de 16 Go dépassé')
if args.report:
    with args.report.open('x', encoding='utf-8') as out:
        json.dump(report, out, ensure_ascii=False, indent=2)
print(json.dumps(report, ensure_ascii=False, indent=2))
