#!/bin/bash
set -euo pipefail
iso=$(realpath "${1:?Chemin ISO requis}")
[[ -f "$iso" ]]
manifest="$(dirname "$iso")/SHA256SUMS"
expected=$(awk -v file="${iso##*/}" 'substr($0,67)==file {print substr($0,1,64)}' "$manifest")
[[ "$expected" =~ ^[a-fA-F0-9]{64}$ ]] || { echo 'ISO absente du manifeste SHA-256 ou entrée ambiguë'; exit 1; }
(cd "$(dirname "$iso")"; sha256sum -c SHA256SUMS)
xorriso -indev "$iso" -report_el_torito plain
command -v qemu-system-x86_64 >/dev/null
code=/usr/share/OVMF/OVMF_CODE_4M.fd
vars=/usr/share/OVMF/OVMF_VARS_4M.fd
mode=${2:-uefi}
[[ "$mode" == uefi || "$mode" == bios ]] || { echo 'Mode attendu : uefi ou bios'; exit 1; }
root=$(cd -- "$(dirname -- "$0")/.." && pwd)
work="$root/logs/test-$mode-$(date -u +%Y%m%dT%H%M%SZ)"
mkdir -p "$work"
firmware=()
if [[ "$mode" == uefi ]]; then
    [[ -f "$code" && -f "$vars" ]]
    cp "$vars" "$work/vars.fd"
    firmware=(-drive "if=pflash,format=raw,readonly=on,file=$code" -drive "if=pflash,format=raw,file=$work/vars.fd")
fi
display=()
if [[ -z ${DISPLAY:-} && -z ${WAYLAND_DISPLAY:-} ]]; then
    display=(-display none)
fi
echo "Traces et socket QMP : $work"
memory=${KYRONEXT_TEST_RAM:-3072}
[[ "$memory" =~ ^[0-9]+$ ]] || { echo 'Mémoire de test invalide'; exit 1; }
video=${KYRONEXT_TEST_VIDEO:-std}
[[ "$video" == std || "$video" == virtio ]] || { echo 'Vidéo attendue : std ou virtio'; exit 1; }
accelerator=(-accel tcg,thread=single)
if [[ -r /dev/kvm && -w /dev/kvm && ${KYRONEXT_TEST_ACCEL:-auto} != tcg ]]; then
    accelerator=(-accel kvm)
fi
echo "Accélérateur : ${accelerator[*]} ; processeur qemu64 ; 1 vCPU ; RAM $memory Mio"
qemu-system-x86_64 -machine q35 -cpu qemu64 "${accelerator[@]}" -m "$memory" -smp 1 "${firmware[@]}" "${display[@]}" -qmp "unix:$work/qmp.sock,server=on,wait=off" -serial "file:$work/serial.log" -cdrom "$iso" -boot d -nic user,model=virtio-net-pci -vga "$video"
