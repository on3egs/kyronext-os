#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
bash scripts/check-environment.sh
[[ $EUID == 0 ]] || { echo "Exécuter avec sudo dans la VM dédiée"; exit 1; }
run="$(date -u +%Y%m%dT%H%M%SZ)-$$"
work="$PWD/build/$run"
for directory in build logs releases; do
    [[ ! -L "$directory" ]] || { echo "Répertoire de sortie symbolique refusé : $directory"; exit 1; }
done
[[ ! -e "$work" ]] || { echo 'Répertoire de construction déjà existant'; exit 1; }
mkdir -p "$work" logs releases
if [[ ${SUDO_UID:-} =~ ^[0-9]+$ && ${SUDO_GID:-} =~ ^[0-9]+$ ]]; then
    chown "$SUDO_UID:$SUDO_GID" build logs releases
fi
exec > >(tee "logs/build-$run.log") 2>&1
trap 'status=$?; if (( status != 0 )); then printf "Échec de compilation (code %s). Voir logs/build-%s.log\n" "$status" "$run" >&2; fi' EXIT
cp -a iso/. "$work/"
chmod +x "$work/auto/config" "$work"/config/hooks/live/*
cd "$work"
while read -r package; do
    apt-cache show "$package" >/dev/null || exit 1
done < <(awk '!/^#/ && NF {print $1}' config/package-lists/*.list.chroot* | sort -u)
lb config
lb build
[[ -s live-image-amd64.hybrid.iso ]]
if grep -Eq '^calamares([[:space:]]|-)' live-image-amd64.packages; then echo "Installateur interdit"; exit 1; fi
out="../../releases/$run"
mkdir -p "$out"
cp live-image-amd64.hybrid.iso "$out/Kyronext-OS-0.1-amd64.iso"
cp live-image-amd64.packages "$out/packages.txt"
dpkg-query -W live-build debootstrap xorriso squashfs-tools > "$out/build-tools.txt"
cd "$out"
sha256sum Kyronext-OS-0.1-amd64.iso > SHA256SUMS
printf 'Compilation terminée. Démarrage non encore validé.\n' > BUILD_REPORT.md
