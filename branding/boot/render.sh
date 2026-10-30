#!/bin/sh
# Rendre les créations originales une seule fois ; le hook utilise les PNG livrés.
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
command -v rsvg-convert >/dev/null 2>&1 || {
    echo 'Outil Debian requis : rsvg-convert (paquet librsvg2-bin).' >&2
    exit 1
}
out="$root/iso/config/includes.binary/boot/kyronext"
mkdir -p "$out"
rsvg-convert --width 800 --height 600 "$root/branding/boot/kyronext-boot.svg" --output "$out/boot-grub.png"
rsvg-convert --width 640 --height 480 "$root/branding/boot/kyronext-boot.svg" --output "$out/boot-bios.png"
printf '%s\n' 'Fonds GRUB 800x600 et BIOS 640x480 générés.'
