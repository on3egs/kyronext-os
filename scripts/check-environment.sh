#!/bin/bash
set -euo pipefail
source /etc/os-release
[[ "$ID" == debian && "$VERSION_ID" == 13 ]] || { echo "Debian 13 requis"; exit 1; }
[[ $(dpkg --print-architecture) == amd64 ]]
for tool in lb debootstrap xorriso mksquashfs sha256sum python3; do command -v "$tool" >/dev/null || { echo "Manquant: $tool"; exit 1; }; done
[[ $(df -Pk . | awk 'NR==2 {print $4}') -gt 41943040 ]] || { echo "40 Gio libres requis"; exit 1; }
echo "Environnement de construction vérifié"
