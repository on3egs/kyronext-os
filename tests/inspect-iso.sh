#!/bin/bash
# Vérifier le contenu de l'image sans démarrer le système ni monter de disque.
set -euo pipefail
iso=$(realpath "${1:?Chemin ISO requis}")
root=$(cd -- "$(dirname -- "$0")/.." && pwd -P)
[[ ! -L "$root/build" ]] || { echo 'Répertoire build symbolique refusé'; exit 1; }
mkdir -p "$root/build"
work=$(mktemp -d "$root/build/inspect.XXXXXX")
trap 'rm -rf -- "$work"' EXIT
xorriso -osirrox on -indev "$iso" -extract /live/filesystem.squashfs "$work/root.squashfs"
unsquashfs -cat "$work/root.squashfs" etc/os-release | tee "$work/os-release"
grep -q 'ID=kyronext' "$work/os-release"
unsquashfs -ll "$work/root.squashfs" > "$work/files.txt"
for file in usr/bin/dolphin usr/bin/konsole usr/bin/plasmashell usr/local/bin/kyronext-welcome usr/local/bin/kyronext-hardware-report usr/lib/user-setup/user-setup-apply usr/share/xsessions/plasmax11.desktop; do
    grep -q "$file" "$work/files.txt"
done
! grep -q 'usr/bin/calamares' "$work/files.txt"
echo 'Inspection statique de l’ISO réussie ; le démarrage reste à tester.'
