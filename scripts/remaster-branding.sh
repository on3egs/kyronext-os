#!/bin/bash
# Recréer l'identité depuis une ISO Kyronext vérifiée, dans Debian isolé.
# Tous les montages portent sur des fichiers en lecture seule, jamais un disque physique.
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
project=$(pwd -P)
[[ $EUID == 0 ]] || { echo 'Exécuter avec sudo dans la VM Debian dédiée'; exit 1; }
source /etc/os-release
[[ "$ID" == debian && "$VERSION_ID" == 13 ]] || { echo 'Debian 13 requis'; exit 1; }
base=$(realpath "${1:?Chemin ISO Kyronext de référence requis}")
[[ -f "$base" && "$base" == *.iso ]] || { echo 'Fichier ISO régulier requis'; exit 1; }
for tool in xorriso mksquashfs mount mountpoint umount sha256sum python3; do command -v "$tool" >/dev/null; done
manifest="$(dirname "$base")/SHA256SUMS"
expected=$(awk -v file="${base##*/}" 'substr($0,67)==file {print substr($0,1,64)}' "$manifest")
[[ "$expected" =~ ^[a-fA-F0-9]{64}$ ]] || { echo 'Entrée SHA-256 manquante ou ambiguë'; exit 1; }
actual=$(sha256sum "$base"); actual=${actual%% *}
[[ "$actual" == "$expected" ]] || { echo 'ISO de référence altérée'; exit 1; }
for directory in build logs releases; do
    [[ ! -L "$directory" ]] || { echo "Lien de sortie refusé : $directory"; exit 1; }
done
mkdir -p build logs releases
run="$(date -u +%Y%m%dT%H%M%SZ)-branding-$$"
work=$(mktemp -d "$project/build/remaster-$run.XXXXXX")
out="$project/releases/$run"
[[ ! -e "$out" ]] || { echo 'Sortie déjà existante'; exit 1; }
mkdir "$out"
if [[ ${SUDO_UID:-} =~ ^[0-9]+$ && ${SUDO_GID:-} =~ ^[0-9]+$ ]]; then
    chown "$SUDO_UID:$SUDO_GID" "$project/build" "$project/logs" "$project/releases"
fi
exec > >(tee "$project/logs/remaster-$run.log") 2>&1
cleanup() {
    result=$?
    for target in "$work/merged" "$work/lower" "$work/base"; do
        if mountpoint -q "$target"; then umount "$target" || result=1; fi
    done
    if (( result != 0 )); then echo "Échec ; travaux et traces conservés dans $work" >&2; fi
    trap - EXIT
    exit "$result"
}
trap cleanup EXIT
mkdir -p "$work"/{base,lower,upper,overlay-work,merged,iso-patch}
mount -t iso9660 -o loop,ro,nodev,nosuid,noexec "$base" "$work/base"
[[ -f "$work/base/live/filesystem.squashfs" ]]
mount -t squashfs -o loop,ro,nodev,nosuid,noexec "$work/base/live/filesystem.squashfs" "$work/lower"
grep -q '^ID=kyronext' "$work/lower/etc/os-release"
cp -a "$project/iso/config/includes.chroot/." "$work/upper/"
if [[ ${KYRONEXT_INCLUDE_PREVIEW_APPS:-0} == 1 ]]; then
    for app in kyronext-paint kyronext-amp; do
        [[ -x "$project/build/native-stage/usr/bin/$app" ]] || { echo "Application compilée absente : $app"; exit 1; }
    done
    cp -a "$project/build/native-stage/." "$work/upper/"
fi
# Une archive copiée depuis Windows ne doit pas donner l'UID1000 ou le mode0666 aux fichiers système.
chown -hR 0:0 "$work/upper"
find "$work/upper" -type d -exec chmod 0755 {} +
find "$work/upper" -type f -exec chmod 0644 {} +
for executable in "$work/upper"/usr/local/bin/kyronext-* "$work/upper"/usr/lib/live/config/*; do
    [[ -f "$executable" && ! -L "$executable" ]] && chmod 0755 "$executable"
done
for executable in "$work/upper"/usr/bin/kyronext-*; do
    [[ -f "$executable" && ! -L "$executable" ]] && chmod 0755 "$executable"
done
mount -t overlay overlay -o "lowerdir=$work/lower,upperdir=$work/upper,workdir=$work/overlay-work" "$work/merged"
python3 - "$work/merged/usr/lib/os-release" <<'PY'
import re,sys
from pathlib import Path
path=Path(sys.argv[1])
content=path.read_text()
for key,value in [('LOGO','kyronext-logo'),('HOME_URL','https://kyronext.eu/kyronext-os/')]:
    content=re.sub(r'^'+key+r'=.*\n?', '', content, flags=re.M)
    content=content.rstrip()+'\n'+key+'="'+value+'"\n'
path.write_text(content)
PY
python3 - "$work/merged/usr/local/bin/kyronext-welcome" <<'PY'
import ast,sys
from pathlib import Path
ast.parse(Path(sys.argv[1]).read_text())
PY
# Zstd est géré par le noyau Debian ; il réduit fortement le coût de cette révision visuelle.
grep -q '^CONFIG_SQUASHFS_ZSTD=y' "/boot/config-$(uname -r)"
mksquashfs "$work/merged" "$work/filesystem.squashfs" -comp zstd -Xcompression-level 8 -noappend -processors 1 -no-progress
du -sx --block-size=1 "$work/merged" | awk '{print $1}' > "$work/filesystem.size"
cp -a "$work/base/boot" "$work/iso-patch/boot"
cp -a "$work/base/isolinux" "$work/iso-patch/isolinux"
if [[ -d "$work/base/.disk" ]]; then cp -a "$work/base/.disk" "$work/iso-patch/.disk"; fi
# Les fichiers extraits d'ISO sont en lecture seule : autoriser uniquement cette copie de travail.
chmod -R u+w "$work/iso-patch"
cp -a "$project/iso/config/includes.binary/." "$work/iso-patch/"
chown -hR 0:0 "$work/iso-patch"
(cd "$work/iso-patch"; bash "$project/iso/config/hooks/live/0950-boot-branding.hook.binary")
# Recalculer les empreintes internes, en excluant les secteurs que xorriso réécrit à l'assemblage.
python3 - "$work" <<'PY'
import hashlib,sys
from pathlib import Path
root=Path(sys.argv[1]); base=root/'base'; patch=root/'iso-patch'
files={str(p.relative_to(base)):p for p in base.rglob('*') if p.is_file() and not p.is_symlink()}
files.update({str(p.relative_to(patch)):p for p in patch.rglob('*') if p.is_file() and not p.is_symlink()})
files['live/filesystem.squashfs']=root/'filesystem.squashfs'
files['live/filesystem.size']=root/'filesystem.size'
with (root/'sha256sum.txt').open('w') as stream:
    for name,path in sorted(files.items()):
        if name in {'sha256sum.txt','md5sum.txt','isolinux/isolinux.bin','isolinux/boot.cat','boot.catalog'}:
            continue
        with path.open('rb') as item:
            digest=hashlib.file_digest(item,'sha256').hexdigest()
        stream.write(f'{digest}  ./{name}\n')
PY
iso="$out/Kyronext-OS-0.1-amd64.iso"
maps=()
while IFS= read -r -d '' entry; do
    relative=${entry#"$work/iso-patch/"}
    # Garder les objets boot originaux : les remplacer par une copie de
    # répertoire détacherait leurs références El Torito (images cachées).
    case "$relative" in isolinux/isolinux.bin|isolinux/boot.cat|boot/grub/efi.img) continue ;; esac
    if [[ -f "$work/base/$relative" ]] && cmp -s "$entry" "$work/base/$relative"; then continue; fi
    maps+=(-map "$entry" "/$relative")
done < <(find "$work/iso-patch" -type f -print0)
xorriso -indev "$base" -outdev "$iso" -assess_indev_features replay -boot_image any replay \
    -map "$work/filesystem.squashfs" /live/filesystem.squashfs \
    -map "$work/filesystem.size" /live/filesystem.size \
    "${maps[@]}" \
    -map "$work/sha256sum.txt" /sha256sum.txt -commit -end
[[ -s "$iso" ]]
python3 "$project/scripts/check-release-budgets.py" --iso "$iso" --report "$out/size-budget.json"
(cd "$out"; sha256sum Kyronext-OS-0.1-amd64.iso > SHA256SUMS)
cp "$(dirname "$base")/packages.txt" "$out/packages.txt"
printf '%s\n' "$expected  ${base##*/}" > "$out/BASE_SHA256.txt"
dpkg-query -W xorriso squashfs-tools > "$out/remaster-tools.txt"
xorriso -indev "$iso" -report_el_torito plain > "$out/BOOT_REPORT.txt" 2>&1
xorriso -indev "$iso" -report_system_area plain >> "$out/BOOT_REPORT.txt" 2>&1
xorriso -indev "$iso" -assess_indev_features plain > "$out/ISO_FEATURES.txt" 2>&1
grep -q ' BIOS ' "$out/BOOT_REPORT.txt"
grep -q ' UEFI ' "$out/BOOT_REPORT.txt"
grep -q 'MBR isohybrid' "$out/BOOT_REPORT.txt"
grep -q 'joliet=1' "$out/ISO_FEATURES.txt"
printf 'Révision visuelle créée depuis une ISO vérifiée. Aucun paquet, noyau ou initrd remplacé. Bureau et actions à valider en VM.\n' > "$out/BUILD_REPORT.md"
echo "ISO révisée : $iso"
