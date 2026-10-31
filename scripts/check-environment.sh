#!/bin/bash
set -euo pipefail
source /etc/os-release
[[ "$ID" == debian && "$VERSION_ID" == 13 ]] || { echo "Debian 13 requis"; exit 1; }
[[ $(dpkg --print-architecture) == amd64 ]]
for tool in lb debootstrap xorriso mksquashfs sha256sum python3; do command -v "$tool" >/dev/null || { echo "Manquant: $tool"; exit 1; }; done
minimum_free_kib=${KYRONEXT_MIN_FREE_KIB:-31457280}
[[ $minimum_free_kib =~ ^[0-9]+$ ]] || { echo "KYRONEXT_MIN_FREE_KIB doit être un entier en Kio"; exit 1; }
available_kib=$(df -Pk . | awk 'NR==2 {print $4}')
[[ $available_kib -gt $minimum_free_kib ]] || { echo "$(((minimum_free_kib + 1048575) / 1048576)) Gio libres requis"; exit 1; }
echo "Environnement de construction vérifié"
