#!/usr/bin/env python3
"""Kyronext Guardian : Hardware Passport 0.1, inventaire local non privilégié.

Ce programme réutilise le diagnostic indépendant Legacy Driver Manager.
Il n'installe aucun pilote et ne vérifie pas la compatibilité d'un ordinateur.
"""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import platform
import sys


def load_diagnostic():
    root = Path(__file__).resolve().parents[2]
    source = root / 'hardware/legacy-driver-manager/diagnose.py'
    specification = importlib.util.spec_from_file_location('kyronext_legacy_diagnostic', source)
    if specification is None or specification.loader is None:
        raise RuntimeError('Module Legacy Driver Manager introuvable dans les sources.')
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def first_value(text, key):
    if text is None:
        return None
    for line in text.splitlines():
        field, separator, value = line.partition(':')
        if separator and field.strip() == key:
            return value.strip()
    return None


def create_passport(diagnostic):
    inventory = diagnostic.collect()
    cpu = first_value(diagnostic.read('/proc/cpuinfo'), 'model name')
    memory = first_value(diagnostic.read('/proc/meminfo'), 'MemTotal')
    return {
        'component': 'Kyronext Guardian — Hardware Passport',
        'version': '0.1',
        'schema_version': 1,
        'created_at_utc': inventory['timestamp_utc'],
        'validation': {
            'status': 'non vérifié',
            'meaning': 'Inventaire observé uniquement ; aucun test de compatibilité réalisé.',
            'boot': 'non vérifié',
            'display': 'non vérifié',
            'network': 'non vérifié',
            'essential_peripherals': 'non vérifié',
        },
        'machine': {
            'manufacturer': diagnostic.read('/sys/class/dmi/id/sys_vendor'),
            'model': diagnostic.read('/sys/class/dmi/id/product_name'),
            'architecture': inventory['architecture'],
            'running_kernel': inventory['kernel'],
            'current_boot_mode': inventory['boot_mode'],
            'cpu_model': cpu,
            'memory_total': memory,
        },
        'hardware_inventory': inventory,
        'capabilities': {
            'offline_inventory': True,
            'automatic_changes': False,
            'driver_installation': False,
            'adaptive_os': False,
            'phoenix_recovery': False,
            'ai_doctor': False,
            'guardian_gui': False,
        },
        'privacy': {
            'stored_locally_only': True,
            'telemetry': False,
            'note': 'Relire les journaux avant partage : des identifiants peuvent y apparaître.',
        },
        'limitations': [
            'Un module candidat par modalias ne prouve pas le fonctionnement du matériel.',
            'Un firmware absent localement peut être optionnel pour cette variante.',
            'Des informations peuvent manquer sans accès au journal noyau.',
            'Aucun profil, pilote, service, partition ou réglage n’est modifié.',
        ],
    }


def write_passport(path, report):
    # O_EXCL refuse aussi un lien symbolique existant. Permissions privées dès la création.
    descriptor = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    with os.fdopen(descriptor, 'w', encoding='utf-8') as stream:
        json.dump(report, stream, ensure_ascii=False, indent=2)
        stream.write('\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True,
                        help='Nouveau rapport JSON ; répertoire parent existant ; aucun écrasement')
    args = parser.parse_args()
    if platform.system() != 'Linux':
        parser.error('Exécuter sous Linux ; aucun inventaire Windows pris en charge.')
    try:
        diagnostic = load_diagnostic()
        report = create_passport(diagnostic)
        write_passport(args.output, report)
    except (OSError, RuntimeError) as error:
        parser.exit(1, f'Hardware Passport non créé : {error}\n')
    print(f'Hardware Passport enregistré : {args.output}\nStatut : non vérifié.')


if __name__ == '__main__':
    main()
