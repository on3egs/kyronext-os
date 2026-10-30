#!/usr/bin/env python3
"""Kyronext Legacy Driver Manager : prototype de diagnostic Linux en lecture seule.

Les alias proviennent du noyau installé. Une correspondance n'est jamais
assimilée à une validation matérielle. Aucun module n'est chargé par cet outil.
"""
import argparse
import datetime
import glob
import json
import os
import platform
from pathlib import Path
import shutil
import subprocess


def read(path):
    try:
        return Path(path).read_text(errors='replace').strip()
    except OSError:
        return None


def command(*args):
    executable = shutil.which(args[0])
    if executable is None:
        for directory in ('/usr/sbin', '/sbin'):
            candidate = Path(directory) / args[0]
            if candidate.is_file():
                executable = str(candidate)
                break
    if executable is None:
        return {'status': 'outil absent', 'output': ''}
    try:
        result = subprocess.run((executable, *args[1:]), capture_output=True, text=True,
                                errors='replace', timeout=15, check=False)
        return {'status': 'ok' if result.returncode == 0 else 'indisponible',
                'output': result.stdout.strip(), 'error': result.stderr.strip()}
    except (OSError, subprocess.TimeoutExpired) as exc:
        return {'status': 'indisponible', 'output': '', 'error': str(exc)}


def driver(path):
    link = path / 'driver'
    return link.resolve().name if link.is_symlink() else None


def firmware(module):
    result = command('modinfo', '-F', 'firmware', module)
    items = []
    for name in sorted(set(result['output'].splitlines())):
        # Ce champ peut contenir des motifs/versionnements, pas un chemin arbitraire.
        if name.startswith('/') or '..' in Path(name).parts:
            continue
        roots = [base / sub for base in (Path('/lib/firmware'), Path('/usr/lib/firmware'))
                 for sub in ('', platform.release(), 'updates', 'updates/' + platform.release())]
        found = any(glob.glob(str(root / (name + suffix)))
                    for root in roots
                    for suffix in ('', '.xz', '.zst', '.gz'))
        items.append({'name': name, 'present_locally': found,
                      'required_for_this_device': 'inconnu'})
    return {'query_status': result['status'], 'items': items}


def device(path, bus):
    alias = read(path / 'modalias')
    candidates = command('modprobe', '--resolve-alias', alias) if alias else {
        'status': 'aucun modalias', 'output': ''}
    modules = sorted(set(candidates['output'].splitlines()))
    bound = driver(path)
    module_link = path / 'driver/module'
    bound_module = module_link.resolve().name if module_link.is_symlink() else None
    identity = path
    if bus == 'usb' and not (path / 'idVendor').exists():
        identity = path.resolve().parent
    vendor = read(identity / ('vendor' if bus == 'pci' else 'idVendor'))
    product = read(identity / ('device' if bus == 'pci' else 'idProduct'))
    class_code = read(path / 'class') if bus == 'pci' else (
        read(path / 'bInterfaceClass') or read(identity / 'bDeviceClass'))
    return {'bus': bus, 'address': path.name, 'vendor_id': vendor,
            'device_id': product, 'class': class_code,
            'modalias': alias, 'bound_driver': bound,
            'bound_module': bound_module,
            'module_alias_query_status': candidates['status'],
            'candidate_modules': modules,
            'firmware_by_module': {m: firmware(m) for m in modules},
            'compatibility_status': 'statut inconnu',
            'note': 'Alias compatible selon le noyau installé ; fonctionnement non validé.'}


def collect():
    devices = []
    for bus in ('pci', 'usb'):
        root = Path('/sys/bus') / bus / 'devices'
        for path in sorted(root.glob('*')):
            if ((path / 'modalias').exists() or (path / 'vendor').exists()
                    or (path / 'idVendor').exists()):
                devices.append(device(path, bus))
    journal = command('journalctl', '-k', '-b', '--no-pager', '-n', '1500')
    journal['output'] = '\n'.join(line for line in journal['output'].splitlines()
                                  if any(term in line.lower() for term in
                                         ('firmware', 'failed', 'error', 'unknown symbol')))
    return {'component': 'Kyronext Legacy Driver Manager', 'schema_version': 1,
            'timestamp_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'kernel': platform.release(), 'architecture': platform.machine(),
            'boot_mode': 'UEFI' if Path('/sys/firmware/efi').exists() else 'BIOS',
            'devices': devices,
            'lspci': command('lspci', '-nnk'), 'lsusb': command('lsusb'),
            'kernel_messages': journal,
            'limits': ['Aucun pilote installé, chargé, déchargé ou modifié.',
                       'Un firmware absent de modinfo peut être optionnel pour ce matériel.',
                       'Un journal restreint ne permet pas de conclure à l’absence d’erreurs.',
                       'Les problèmes connus sont documentés séparément ; aucun diagnostic par ressemblance de nom.']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, help='Rapport JSON local, créé sans écrasement')
    args = parser.parse_args()
    if platform.system() != 'Linux':
        parser.error('Exécuter dans Linux ; aucun diagnostic Windows pris en charge.')
    report = json.dumps(collect(), ensure_ascii=False, indent=2) + '\n'
    if args.output:
        descriptor = os.open(args.output, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        with os.fdopen(descriptor, 'w', encoding='utf-8') as stream:
            stream.write(report)
    else:
        print(report, end='')


if __name__ == '__main__':
    main()
