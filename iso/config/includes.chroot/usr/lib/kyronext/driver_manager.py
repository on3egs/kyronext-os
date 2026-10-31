"""Diagnostic et réparation limitée du Wi-Fi, sans arrêt du réseau ou de SSH."""
import argparse
import datetime as dt
import hashlib
import glob
import json
import os
from pathlib import Path
import platform
import re
import shutil
import stat
import subprocess
import sys
import uuid

BUNDLE = Path('/usr/share/kyronext/drivers/offline')
STATE = Path('/var/lib/kyronext/driver-manager')
FAMILIES = {
    'iwl': 'firmware-iwlwifi', 'ath': 'firmware-atheros',
    'b43': 'firmware-b43-installer',
    'rtw': 'firmware-realtek', 'rtl': 'firmware-realtek',
    'brcm': 'firmware-brcm80211', 'mt7': 'firmware-mediatek',
    'amdgpu': 'firmware-amd-graphics', 'i915': 'firmware-intel-graphics',
    'xe': 'firmware-intel-graphics', 'nouveau': 'firmware-nvidia-graphics',
    'snd_sof': 'firmware-sof-signed',
}
SPECIFIC_FIRMWARE = {
    'ath9k_htc': 'firmware-ath9k-htc',
    'carl9170': 'firmware-carl9170',
    'zd1211rw': 'firmware-zd1211',
}


def firmware_packages_for_modules(modules):
    packages = set()
    for module in modules:
        specific = next((package for prefix, package in SPECIFIC_FIRMWARE.items()
                         if module.startswith(prefix)), None)
        if specific:
            packages.add(specific)
            continue
        packages.update(package for prefix, package in FAMILIES.items()
                        if module.startswith(prefix))
    return sorted(packages)


def run(*args, check=False, timeout=35):
    executable = shutil.which(args[0])
    if executable is None:
        executable = next((str(Path(folder) / args[0]) for folder in ('/usr/sbin', '/sbin')
                           if (Path(folder) / args[0]).is_file()), args[0])
    try:
        p = subprocess.run((executable, *args[1:]), capture_output=True, text=True, timeout=timeout,
                           env={**os.environ, 'LC_ALL': 'C'}, check=False)
    except (OSError, subprocess.TimeoutExpired) as exc:
        if check:
            raise RuntimeError(str(exc)) from exc
        return {'ok': False, 'output': '', 'error': str(exc)}
    if check and p.returncode:
        raise RuntimeError(p.stderr.strip() or p.stdout.strip() or f'Échec : {args[0]}')
    return {'ok': p.returncode == 0, 'output': p.stdout.strip(), 'error': p.stderr.strip()}


def read(path):
    try:
        return Path(path).read_text(errors='replace').strip()
    except OSError:
        return ''


def redact(text):
    text = re.sub(r'(?i)\b(?:[0-9a-f]{2}:){5}[0-9a-f]{2}\b', '[MAC masquée]', text)
    return re.sub(r'\b(?:\d{1,3}\.){3}\d{1,3}\b', '[IP masquée]', text)


def inventory():
    devices = []
    for bus in ('pci', 'usb'):
        for path in sorted((Path('/sys/bus') / bus / 'devices').glob('*')):
            alias = read(path / 'modalias')
            if not alias:
                continue
            identity = path if (path / 'idVendor').exists() or bus == 'pci' else path.resolve().parent
            vendor = read(identity / ('vendor' if bus == 'pci' else 'idVendor'))
            product = read(identity / ('device' if bus == 'pci' else 'idProduct'))
            link = path / 'driver/module'
            bound = link.resolve().name if link.is_symlink() else ''
            candidates = run('modprobe', '--resolve-alias', alias)
            modules = sorted(set(candidates['output'].splitlines()))
            packages = firmware_packages_for_modules(modules)
            firmware_files = []
            for module in ([bound] if bound else modules):
                for name in run('modinfo', '-F', 'firmware', module)['output'].splitlines():
                    if name.startswith('/') or '..' in Path(name).parts:
                        continue
                    found = any(glob.glob(str(root / (name + suffix)))
                                for root in (Path('/lib/firmware'), Path('/usr/lib/firmware'))
                                for suffix in ('', '.xz', '.zst', '.gz'))
                    firmware_files.append({'name': name, 'present_locally': found,
                                           'required_for_this_device': 'inconnu'})
            devices.append({'bus': bus, 'address': path.name, 'vendor': vendor,
                            'product': product, 'driver': bound, 'candidates': modules,
                            'firmware_packages_to_check': packages,
                            'firmware_files': firmware_files,
                            'state': 'Pilote lié' if bound else 'À examiner — pas une preuve de panne'})
    journal = run('journalctl', '-k', '-b', '--no-pager', '-n', '800')
    messages = [redact(line) for line in journal['output'].splitlines()
                if any(word in line.lower() for word in ('firmware', 'failed', 'unknown symbol'))]
    return {'schema': 1, 'timestamp_utc': dt.datetime.now(dt.timezone.utc).isoformat(),
            'kernel': platform.release(), 'architecture': platform.machine(),
            'devices': devices, 'network': run('nmcli', '-f', 'DEVICE,TYPE,STATE', 'device'),
            'wifi': wifi_state(strict=False), 'audio': run('aplay', '-l'),
            'graphics': run('lspci', '-nnk'), 'kernel_messages': messages,
            'kernel_log_accessible': journal['ok'],
            'notice': 'Correspondances indicatives : un firmware annoncé par modinfo peut être optionnel pour ce périphérique. Aucun pilote propriétaire ni module forcé. Relire le rapport avant partage.'}


def wifi_state(strict=True):
    radio = run('nmcli', 'radio', 'wifi', check=strict)
    rf = run('rfkill', '--json', '--output', 'ID,TYPE,SOFT,HARD', check=strict)
    try:
        blocks = json.loads(rf['output']).get('rfkilldevices', []) if rf['ok'] else []
    except (ValueError, AttributeError):
        if strict:
            raise RuntimeError('État rfkill illisible ; réparation annulée')
        blocks = []
    return {'radio': radio['output'], 'boot_id': read('/proc/sys/kernel/random/boot_id'), 'radio_query_ok': radio['ok'],
            'rfkill_query_ok': rf['ok'], 'rfkill': blocks}


def require_admin():
    if os.geteuid() != 0:
        raise RuntimeError('Cette action nécessite une authentification administrateur.')


def save_private(path, data):
    fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    with os.fdopen(fd, 'w', encoding='utf-8') as out:
        json.dump(data, out, ensure_ascii=False, indent=2)


def read_rollback_snapshot(identifier):
    """Open a root-owned rollback file without following a snapshot symlink."""
    if STATE.is_symlink() or not STATE.is_dir():
        raise RuntimeError('Dossier de sauvegarde non sûr')
    if os.name == 'posix':
        directory = STATE.stat(follow_symlinks=False)
        if directory.st_uid != 0 or directory.st_mode & 0o077:
            raise RuntimeError('Dossier de sauvegarde non sûr')

    path = STATE / (identifier + '.json')
    if path.is_symlink():
        raise RuntimeError('Sauvegarde non sûre')
    flags = os.O_RDONLY | getattr(os, 'O_NOFOLLOW', 0) | getattr(os, 'O_CLOEXEC', 0)
    try:
        fd = os.open(path, flags)
    except OSError as exc:
        raise RuntimeError('Sauvegarde absente ou non sûre') from exc
    try:
        info = os.fstat(fd)
        if not stat.S_ISREG(info.st_mode):
            raise RuntimeError('Sauvegarde non sûre')
        if os.name == 'posix' and (info.st_uid != 0 or info.st_mode & 0o077):
            raise RuntimeError('Sauvegarde non sûre')
        with os.fdopen(fd, 'r', encoding='utf-8') as source:
            fd = -1
            snapshot = json.load(source)
    except (OSError, ValueError) as exc:
        raise RuntimeError('Sauvegarde illisible ou invalide') from exc
    finally:
        if fd >= 0:
            os.close(fd)
    if not isinstance(snapshot, dict):
        raise RuntimeError('Sauvegarde invalide')
    return snapshot


def repair_wifi():
    require_admin()
    before = wifi_state()
    if before['radio'] not in ('enabled', 'disabled'):
        raise RuntimeError('État radio inconnu ; aucune modification')
    STATE.mkdir(parents=True, exist_ok=True, mode=0o700)
    if STATE.is_symlink() or STATE.stat().st_uid != 0 or STATE.stat().st_mode & 0o022:
        raise RuntimeError('Dossier de sauvegarde non sûr')
    identifier = uuid.uuid4().hex
    save_private(STATE / (identifier + '.json'), before)
    try:
        run('rfkill', 'unblock', 'wifi', check=True)
        run('nmcli', 'radio', 'wifi', 'on', check=True)
    except Exception as exc:
        try:
            restore_wifi(identifier)
        except Exception as rollback:
            raise RuntimeError(f'Échec : {exc}. Retour arrière incomplet : {rollback}. Référence : {identifier}') from exc
        raise RuntimeError(f'Échec corrigé par retour arrière : {exc}') from exc
    return {'message': 'Radio Wi-Fi activée et blocages logiciels levés. Aucun profil ni service réseau modifié. Un blocage matériel exige le bouton du PC.',
            'rollback_id': identifier}


def restore_wifi(identifier):
    require_admin()
    if not re.fullmatch(r'[0-9a-f]{32}', identifier):
        raise RuntimeError('Référence de retour arrière invalide')
    before = read_rollback_snapshot(identifier)
    if before.get('boot_id') != read('/proc/sys/kernel/random/boot_id'):
        raise RuntimeError('Sauvegarde d’un autre démarrage : retour radio refusé.')
    if before.get('radio') not in ('enabled', 'disabled'):
        raise RuntimeError('Sauvegarde radio invalide')
    if not isinstance(before.get('rfkill'), list):
        raise RuntimeError('Sauvegarde rfkill invalide')
    for device in before['rfkill']:
        if not isinstance(device, dict):
            raise RuntimeError('Sauvegarde rfkill invalide')
        if device.get('type') in ('wlan', 'wifi') and (
                not str(device.get('id', '')).isdigit()
                or device.get('soft') not in ('blocked', 'unblocked')):
            raise RuntimeError('Entrée rfkill non reconnue dans la sauvegarde')
    # Rétablir uniquement les états radio ; ne jamais supprimer un profil.
    run('nmcli', 'radio', 'wifi', 'on' if before['radio'] == 'enabled' else 'off', check=True)
    for device in before['rfkill']:
        if device.get('type') not in ('wlan', 'wifi'):
            continue
        identifier = str(device.get('id', ''))
        if not identifier.isdigit() or device.get('soft') not in ('blocked', 'unblocked'):
            raise RuntimeError('Entrée rfkill non reconnue dans la sauvegarde')
        run('rfkill', 'block' if device['soft'] == 'blocked' else 'unblock', identifier, check=True)
    return {'message': 'État précédent de la radio Wi-Fi rétabli.'}


def verify_bundle():
    manifest = json.loads((BUNDLE / 'manifest.json').read_text())
    for item in manifest['packages']:
        name = item['file']
        if Path(name).name != name or not name.endswith('.deb'):
            raise RuntimeError('Nom de paquet invalide dans la réserve')
        path = BUNDLE / name
        if path.is_symlink() or not path.is_file():
            raise RuntimeError(f'Paquet absent ou lien refusé : {name}')
        with path.open('rb') as stream:
            digest = hashlib.file_digest(stream, 'sha256').hexdigest()
        if digest != item['sha256']:
            raise RuntimeError(f'Paquet altéré : {name}')
    return manifest


def install_firmware(package):
    require_admin()
    # Réserve système uniquement ; jamais de paquet fourni par une URL utilisateur.
    if BUNDLE.is_symlink() or BUNDLE.stat().st_uid != 0 or BUNDLE.stat().st_mode & 0o022:
        raise RuntimeError('Réserve non sûre')
    manifest = verify_bundle()
    entries = {item['package']: item for item in manifest['packages']}
    if package not in manifest['firmware_allowlist'] or package not in entries:
        raise RuntimeError('Seuls les firmwares sélectionnés dans cette image sont installables.')
    item = entries[package]
    current = run('dpkg-query', '-W', '-f=${Version}', package)
    if current['ok'] and current['output'] != item['version']:
        raise RuntimeError('Version différente déjà installée : aucune rétrogradation automatique.')
    # Une simulation précède l'installation ; jamais de suppression ni téléchargement.
    target = str(BUNDLE / item['file'])
    common = ['apt-get', '-o', f'Dir::Cache::archives={BUNDLE}', '--no-download', '--no-remove', '--no-upgrade', '--no-install-recommends', '--reinstall']
    run(*common, '--simulate', 'install', target, check=True)
    result = run(*common, '--assume-yes', 'install', target, check=True, timeout=None)
    return {'message': result['output'] + '\nFirmware réinstallé. Rebrancher le périphérique ou redémarrer plus tard si nécessaire. Aucun module déchargé automatiquement.'}


def install_b43_firmware():
    """Install Broadcom's non-redistributable b43 firmware only on request."""
    require_admin()
    devices = inventory()['devices']
    supported = any(device.get('bus') == 'pci'
                    and ('b43' in device.get('candidates', [])
                         or 'b43legacy' in device.get('candidates', []))
                    for device in devices)
    if not supported:
        raise RuntimeError('Aucun périphérique PCI associé au pilote b43/b43legacy n’a été détecté.')
    # Debian's contrib package fetches the vendor firmware during installation.
    # It is intentionally absent from the freely redistributable ISO.
    run('apt-get', 'update', check=True, timeout=None)
    run('apt-get', '--no-remove', '--no-upgrade', '--no-install-recommends',
        '--assume-yes', '--reinstall', 'install', 'firmware-b43-installer',
        check=True, timeout=None)
    return {'message': 'Le paquet Debian a terminé l’installation du firmware Broadcom b43. Redémarrez pour que le pilote le charge. Aucun module n’a été déchargé automatiquement.'}


def main():
    parser = argparse.ArgumentParser(description='Kyronext — pilotes et connexion')
    sub = parser.add_subparsers(dest='action', required=True)
    sub.add_parser('diagnose')
    sub.add_parser('repair-wifi')
    sub.add_parser('verify-offline')
    sub.add_parser('install-b43-firmware')
    restore = sub.add_parser('restore-wifi'); restore.add_argument('identifier')
    install = sub.add_parser('install-firmware'); install.add_argument('package')
    args = parser.parse_args()
    try:
        if args.action == 'diagnose': result = inventory()
        elif args.action == 'repair-wifi': result = repair_wifi()
        elif args.action == 'restore-wifi': result = restore_wifi(args.identifier)
        elif args.action == 'install-firmware': result = install_firmware(args.package)
        elif args.action == 'install-b43-firmware': result = install_b43_firmware()
        else: result = verify_bundle()
        print(json.dumps(result, ensure_ascii=False, indent=2))
    except Exception as exc:
        print(json.dumps({'error': str(exc)}, ensure_ascii=False), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
