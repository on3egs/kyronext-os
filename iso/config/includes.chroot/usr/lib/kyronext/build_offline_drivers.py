"""Exécuté uniquement au build : réserve APT vérifiée et dépendances installées."""
import hashlib
import json
from pathlib import Path
import re
import subprocess

DEST = Path('/usr/share/kyronext/drivers/offline')
LIST = Path('/usr/share/kyronext/drivers/hardware-packages.txt')


def resolve_closure(selected, installed):
    providers = {}
    for name, info in sorted(installed.items()):
        for expression in info.get('provides', '').split(','):
            virtual = re.split(r'[\s(:]', expression.strip())[0]
            if virtual:
                providers.setdefault(virtual, name)
    todo = list(selected)
    closure = set()
    while todo:
        name = todo.pop()
        if name in closure:
            continue
        if name not in installed:
            raise RuntimeError(f'Paquet sélectionné non installé : {name}')
        closure.add(name)
        for expression in installed[name]['depends'].split(','):
            if not expression.strip():
                continue
            candidates = [re.split(r'[\s(:]', alt.strip())[0] for alt in expression.split('|')]
            choice = next((candidate if candidate in installed else providers[candidate]
                           for candidate in candidates if candidate in installed or candidate in providers), None)
            if choice:
                todo.append(choice)
            else:
                # Certaines dépendances sont virtuelles : vérifier via APT au build.
                # Ne jamais prétendre avoir une réserve complète dans ce cas.
                raise RuntimeError(f'Dépendance virtuelle/non résolue à traiter : {expression}')
    return closure


def main():
    selected = [line.strip() for line in LIST.read_text().splitlines() if line.strip() and not line.startswith('#')]
    output = subprocess.check_output(['dpkg-query', '-W', '-f=${Package}\t${Version}\t${Depends}, ${Pre-Depends}\t${Provides}\t${db:Status-Status}\n'], text=True)
    installed = {}
    for line in output.splitlines():
        name, version, depends, provides, state = line.split('\t')
        if state == 'installed':
            installed[name] = {'version': version, 'depends': depends, 'provides': provides}
    closure = resolve_closure(selected, installed)
    DEST.mkdir(parents=True, exist_ok=True)
    packages = []
    for package in sorted(closure):
        version = installed[package]['version']
        subprocess.run(['apt-get', 'download', f'{package}={version}'], cwd=DEST, check=True)
    size = sum(path.stat().st_size for path in DEST.glob('*.deb'))
    if size >= 1_000_000_000:
        raise RuntimeError(f'Réserve hors ligne trop volumineuse : {size} octets')
    for path in sorted(DEST.glob('*.deb')):
        package = subprocess.check_output(['dpkg-deb', '-f', str(path), 'Package'], text=True).strip()
        version = subprocess.check_output(['dpkg-deb', '-f', str(path), 'Version'], text=True).strip()
        with path.open('rb') as stream:
            digest = hashlib.file_digest(stream, 'sha256').hexdigest()
        packages.append({'package': package, 'version': version, 'file': path.name, 'sha256': digest})
    manifest = {'schema': 1, 'bytes': size, 'dependency_closure_complete': True,
                'firmware_allowlist': [p for p in selected if p.startswith('firmware-')], 'packages': packages}
    (DEST / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')


if __name__ == '__main__':
    main()
