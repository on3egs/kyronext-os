#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
[[ $EUID != 0 ]] || { echo 'Compiler avec un utilisateur ordinaire dans la VM Debian.'; exit 1; }
[[ -d build/native-stage/usr ]] || { echo 'Lancer scripts/build-applications.sh avant le packaging.'; exit 1; }
command -v dpkg-deb >/dev/null
run="$(date -u +%Y%m%dT%H%M%SZ)-$$"
root="$PWD/build/suite-package-$run"
[[ ! -e "$root" && ! -L "$root" ]] || { echo 'Répertoire de paquet déjà existant.'; exit 1; }
install -d "$root/DEBIAN"
cp -a build/native-stage/. "$root/"
install -m 0644 applications/suite/debian/control "$root/DEBIAN/control"
version=$(sed -n 's/^Version:[[:space:]]*//p' applications/suite/debian/control | head -n 1)
[[ "$version" =~ ^[0-9A-Za-z.+:~_-]+$ ]] || { echo 'Version Debian invalide.'; exit 1; }
deb="$PWD/build/kyronext-suite-preview_${version}_amd64.deb"
dpkg-deb --build --root-owner-group "$root" "$deb"
[[ $(dpkg-deb -f "$deb" Package) == kyronext-suite-preview ]]
[[ $(dpkg-deb -f "$deb" Version) == "$version" ]]
[[ $(dpkg-deb -f "$deb" Architecture) == amd64 ]]
dpkg-deb --contents "$deb" >/dev/null
echo "Paquet créé : $deb"
