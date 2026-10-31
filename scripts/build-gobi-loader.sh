#!/bin/bash
# Reconstruire les sources Debian 0.7 sur Debian 13, sans dépôt Sid/Testing.
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
[[ $(dpkg --print-architecture) == amd64 ]] || { echo 'Constructeur AMD64 requis'; exit 1; }
dest=$(realpath -m "${1:?Dossier packages.chroot requis}")
mkdir -p "$dest" build
work=$(mktemp -d "$PWD/build/gobi-source.XXXXXX")
base=https://deb.debian.org/debian/pool/contrib/g/gobi-loader
version=0.7+git20171116.143deb4+dfsg
curl --fail --location --proto '=https' --tlsv1.2 "$base/gobi-loader_${version}.orig.tar.xz" -o "$work/source.tar.xz"
printf '%s  %s\n' 7a7ffe1e32693065824e4cfe271dabe64350ee0d32df594e5297e9e11e2643f6 "$work/source.tar.xz" | sha256sum -c -
curl --fail --location --proto '=https' --tlsv1.2 "$base/gobi-loader_${version}-1.debian.tar.xz" -o "$work/debian.tar.xz"
printf '%s  %s\n' 0b348ca90234af55d921fde525944c6e41b438751b0a1c89f3fecb4fcfd89f64 "$work/debian.tar.xz" | sha256sum -c -
mkdir "$work/src"
tar -xJf "$work/source.tar.xz" --strip-components=1 -C "$work/src"
tar -xJf "$work/debian.tar.xz" -C "$work/src"
(cd "$work/src"; dpkg-buildpackage -b -us -uc)
package="$work/gobi-loader_${version}-1_amd64.deb"
[[ -s "$package" ]] || { echo 'Paquet Gobi absent'; exit 1; }
dpkg-deb -f "$package" Package Version Architecture
cp "$package" "$dest/"
