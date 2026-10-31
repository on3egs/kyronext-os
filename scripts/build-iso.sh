#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
project=$PWD
installer_profile=${KYRONEXT_INSTALLER_PROFILE:-none}
case "$installer_profile" in
    none)
        [[ ${KYRONEXT_DEBIAN_INSTALLER:-none} == none ]] || { echo 'Le profil Live standard ne peut pas activer Debian Installer'; exit 2; }
        export KYRONEXT_DEBIAN_INSTALLER=none
        export KYRONEXT_ISO_APPLICATION='Kyronext OS 0.1 Developer Preview'
        export KYRONEXT_ISO_VOLUME='KYRONEXT_0_1_AMD64'
        ;;
    calamares-live)
        [[ ${KYRONEXT_DEBIAN_INSTALLER:-live} == live ]] || { echo 'Le profil Calamares exige Debian Installer live'; exit 2; }
        export KYRONEXT_DEBIAN_INSTALLER=live
        export KYRONEXT_ISO_APPLICATION='Kyronext OS 0.2 Experimental Installer'
        export KYRONEXT_ISO_VOLUME='KYRONEXT_0_2_INSTALLER'
        ;;
    *) echo "Profil de compilation invalide : $installer_profile"; exit 2 ;;
esac
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
if [[ "$installer_profile" == calamares-live ]]; then
    installer_root="$project/installer"
    cp "$installer_root/profile/calamares-live.packages" "$work/config/package-lists/installer-base.list.chroot"
    cp "$installer_root/profile/installer-live.packages" "$work/config/package-lists/installer-live.list.chroot_live"
    install -Dm0755 "$installer_root/profile/installer-profile.hook.chroot" "$work/config/hooks/live/0940-installer-profile.hook.chroot"
    branding_source="$work/config/includes.chroot/usr/share/kyronext/installer/calamares"
    install -Dm0644 "$installer_root/calamares/settings.conf" "$branding_source/settings.conf"
    install -Dm0644 "$installer_root/calamares/modules/packages.conf" "$branding_source/modules/packages.conf"
    install -Dm0644 "$installer_root/calamares/modules/users.conf" "$branding_source/modules/users.conf"
    install -Dm0644 "$installer_root/calamares/modules/unpackfs.conf" "$branding_source/modules/unpackfs.conf"
    install -Dm0644 "$installer_root/calamares/modules/bootloader.conf" "$branding_source/modules/bootloader.conf"
    install -Dm0644 "$installer_root/calamares/branding/kyronext/branding.desc" "$branding_source/branding/kyronext/branding.desc"
    install -Dm0644 "$installer_root/calamares/branding/kyronext/show.qml" "$branding_source/branding/kyronext/show.qml"
    install -Dm0644 "$installer_root/assets/desktop.png" "$branding_source/branding/kyronext/desktop.png"
    install -Dm0644 "$installer_root/assets/applications.png" "$branding_source/branding/kyronext/applications.png"
    install -Dm0644 "$installer_root/assets/kyronext-logo.svg" "$branding_source/branding/kyronext/kyronext-logo.svg"
    install -Dm0644 "$installer_root/profile/launcher-calamares.desktop" "$work/config/includes.chroot/usr/local/share/applications/calamares.desktop"
    install -Dm0644 "$installer_root/profile/launcher-debian-installer.desktop" "$work/config/includes.chroot/usr/local/share/applications/debian-installer-launcher.desktop"
    install -Dm0644 "$installer_root/assets/kyronext-logo.svg" "$work/config/includes.chroot/usr/local/share/icons/hicolor/scalable/apps/kyronext-logo.svg"
    printf '%s\n' 'Kyronext OS 0.2 — installateur expérimental (Debian 13 Trixie)' > "$work/config/includes.binary/.disk/info"
    package_root="$work/installer-launcher-package"
    install -Dm0644 "$installer_root/profile/launcher.control" "$package_root/DEBIAN/control"
    install -Dm0755 "$installer_root/profile/launcher.kyronext-installer" "$package_root/usr/local/bin/kyronext-installer"
    install -Dm0440 "$installer_root/profile/launcher.sudoers" "$package_root/etc/sudoers.d/kyronext-installer"
    install -Dm0644 "$installer_root/profile/launcher.kyronext-installer.desktop" "$package_root/usr/share/applications/kyronext-installer.desktop"
    visudo -cf "$package_root/etc/sudoers.d/kyronext-installer"
    mkdir -p "$work/config/packages.chroot"
    dpkg-deb --root-owner-group --build "$package_root" "$work/config/packages.chroot/kyronext-installer-launcher_0.2.0_all.deb"
fi
install -Dm0644 iso/config/package-lists/hardware-connectivity.list.chroot "$work/config/includes.chroot/usr/share/kyronext/drivers/hardware-packages.txt"
# Construire localement Gobi 0.7 (le paquet Trixie 0.6 ne gère pas Gobi 2000).
bash scripts/build-gobi-loader.sh "$work/config/packages.chroot"
# Clé PUBLIQUE facultative pour une image privée, jamais copiée implicitement.
if [[ -n ${KYRONEXT_SSH_PUBLIC_KEY:-} ]]; then
    key=$(realpath "$KYRONEXT_SSH_PUBLIC_KEY")
    [[ -f "$key" ]] || { echo 'Clé publique absente'; exit 1; }
    ssh-keygen -l -f "$key" >/dev/null
    [[ $(grep -cve '^[[:space:]]*$' "$key") == 1 ]] || { echo 'Une seule clé publique requise'; exit 1; }
    case "$(awk '{print $1}' "$key")" in ssh-ed25519|ssh-rsa|ecdsa-sha2-*|sk-ssh-ed25519@openssh.com|sk-ecdsa-sha2-nistp256@openssh.com) ;; *) echo 'Clé publique invalide'; exit 1;; esac
    install -Dm0644 "$key" "$work/config/includes.chroot/etc/kyronext/ssh/authorized_keys"
fi
# Un hash Live facultatif permet une image privée avec sudo authentifié. Le
# fichier doit rester hors Git et n'est jamais demandé dans un build public.
if [[ -n ${KYRONEXT_LIVE_PASSWORD_HASH_FILE:-} ]]; then
    [[ -f "$KYRONEXT_LIVE_PASSWORD_HASH_FILE" && ! -L "$KYRONEXT_LIVE_PASSWORD_HASH_FILE" ]] || { echo 'Fichier de hash Live invalide'; exit 1; }
    password_hash_file=$(realpath -- "$KYRONEXT_LIVE_PASSWORD_HASH_FILE")
    [[ $(stat -c '%a' "$password_hash_file") == 600 ]] || { echo 'Le fichier de hash Live doit avoir le mode 0600'; exit 1; }
    live_password_hash=$(cat -- "$password_hash_file")
    [[ $live_password_hash =~ ^\$6\$(rounds=[0-9]+\$)?[A-Za-z0-9./]{1,16}\$[A-Za-z0-9./]{86}$ ]] || { echo 'Hash Live SHA-512 crypt invalide'; exit 1; }
    install -Dm0600 "$password_hash_file" "$work/config/includes.chroot/etc/kyronext/live-password.hash"
fi
if [[ -n ${KYRONEXT_SUITE_DEB:-} ]]; then
    suite_deb=$(realpath "$KYRONEXT_SUITE_DEB")
    [[ -f "$suite_deb" && "$suite_deb" == *.deb ]] || { echo 'Paquet de la suite invalide'; exit 1; }
    [[ $(dpkg-deb -f "$suite_deb" Package) == kyronext-suite-preview ]] || { echo 'Nom du paquet inattendu'; exit 1; }
    [[ $(dpkg-deb -f "$suite_deb" Architecture) == amd64 ]] || { echo 'Paquet AMD64 requis'; exit 1; }
    mkdir -p "$work/config/packages.chroot"
    cp "$suite_deb" "$work/config/packages.chroot/"
fi
# Les sources copiées depuis Windows ou extraites par builder ne doivent pas
# laisser les outils privilégiés et configurations modifiables par le compte Live.
chown -hR 0:0 "$work/config/includes.chroot"
chmod +x "$work/auto/config" "$work"/config/hooks/live/*
cd "$work"
while read -r package; do
    if [[ "$package" == kyronext-installer-launcher && "$installer_profile" == calamares-live ]]; then
        [[ $(dpkg-deb -f "config/packages.chroot/${package}_0.2.0_all.deb" Package) == "$package" ]] || { echo "Paquet local inattendu : $package"; exit 1; }
    else
        apt_package=${package%:i386}
        apt-cache show "$apt_package" >/dev/null || { echo "Paquet indisponible sur le constructeur : $package (activer main, contrib et non-free-firmware Debian 13)"; exit 1; }
    fi
done < <(awk '!/^#/ && NF {print $1}' config/package-lists/*.list.chroot* | sort -u)
lb config
lb build
[[ -s live-image-amd64.hybrid.iso ]]
python3 "$project/scripts/check-release-budgets.py" --root "$PWD/chroot" --iso "$PWD/live-image-amd64.hybrid.iso" --report "$PWD/size-budget.json"
if grep -Eq '^calamares([[:space:]]|-)' live-image-amd64.packages; then
    [[ "$installer_profile" == calamares-live ]] || { echo 'Calamares détecté hors du profil installateur'; exit 1; }
    for package in calamares calamares-settings-debian debian-installer-launcher kyronext-installer-launcher; do
        grep -qx "$package" live-image-amd64.packages || { echo "Paquet installateur absent : $package"; exit 1; }
    done
else
    [[ "$installer_profile" == none ]] || { echo 'Calamares absent du profil installateur'; exit 1; }
fi
for package in steam-installer steam-devices; do
    grep -qx "$package" live-image-amd64.packages || { echo "Paquet Steam absent : $package"; exit 1; }
done
out="../../releases/$run"
mkdir -p "$out"
if [[ "$installer_profile" == calamares-live ]]; then
    iso_name=Kyronext-OS-0.2-experimental-installer-amd64.iso
else
    iso_name=Kyronext-OS-0.1-amd64.iso
fi
cp live-image-amd64.hybrid.iso "$out/$iso_name"
cp live-image-amd64.packages "$out/packages.txt"
cp size-budget.json "$out/size-budget.json"
if [[ -n ${suite_deb:-} ]]; then
    cp "$suite_deb" "$out/"
    package_file=${suite_deb##*/}
fi
dpkg-query -W live-build debootstrap xorriso squashfs-tools > "$out/build-tools.txt"
cd "$out"
if [[ -n ${package_file:-} ]]; then
    sha256sum "$iso_name" "$package_file" > SHA256SUMS
else
    sha256sum "$iso_name" > SHA256SUMS
fi
if [[ -n ${KYRONEXT_SSH_PUBLIC_KEY:-} || -n ${KYRONEXT_LIVE_PASSWORD_HASH_FILE:-} ]]; then
    access_status='Image de test privée : accès SSH ou sudo configuré par l’opérateur.'
else
    access_status='Aucune clé SSH propriétaire ni aucun mot de passe sudo Live injecté.'
fi
if [[ "$installer_profile" == calamares-live ]]; then
    printf 'Compilation terminée. Installateur expérimental : Calamares graphique depuis le bureau et Debian Installer depuis le menu de démarrage. Installation sur disque virtuel et redémarrage non encore validés.\n%s\n' "$access_status" > BUILD_REPORT.md
else
    printf 'Compilation terminée. Démarrage non encore validé.\n%s\n' "$access_status" > BUILD_REPORT.md
fi
