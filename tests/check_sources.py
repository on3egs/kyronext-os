from pathlib import Path
p=Path(__file__).resolve().parents[1]
for name in ['iso/auto/config','scripts/build-iso.sh','scripts/test-iso.sh']:
 assert (p/name).is_file(), name
pkgs=(p/'iso/config/package-lists/desktop.list.chroot').read_text().splitlines()
assert len(pkgs)==len(set(pkgs))
assert 'live-task-kde' in pkgs and 'live-boot' in pkgs
assert not any(x.startswith('calamares') for x in pkgs)
live_pkgs=(p/'iso/config/package-lists/live-essential.list.chroot_live').read_text().splitlines()
assert 'user-setup' in live_pkgs, 'Création du compte Live requise sans recommandations APT'
assert 'keyboard-configuration' in live_pkgs
print('Contrôles statiques sources réussis')
