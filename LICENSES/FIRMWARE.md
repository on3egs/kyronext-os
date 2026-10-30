# Firmwares et microcodes de la première image

Inventaire observé le 30 octobre 2026 pendant la fabrication 0.1. Il provient du manifeste `chroot.packages.live` et des métadonnées des paquets Debian téléchargés. Le manifeste de fichiers `chroot.files` confirme la présence d’un fichier `copyright` pour chacun des paquets ci-dessous. L’inspection de l’ISO finale reste une validation distincte.

## Nature des composants

Les pilotes du noyau Debian (par exemple i915, radeon, amdgpu, nouveau, ahci et les pilotes réseau) et les firmwares ne sont pas interchangeables. Les firmwares sont des programmes chargés dans les périphériques ; les microcodes concernent le processeur. Le fait qu’un paquet soit inclus ne prouve ni son chargement ni la compatibilité d’un ordinateur.

L’inventaire contient **38 paquets de firmwares/microcodes**, dont **35 classés par Debian dans non-free-firmware** et **3 dans main**. Certains sont des métapaquets sans micrologiciel propre. Cette première sélection large provient du mécanisme standard de live-build ; elle comprend aussi du matériel spécialisé ou serveur. Une sélection plus ciblée pourra être évaluée après les tests, sans retirer arbitrairement un firmware utile.

La section Debian est une information de classification, pas une licence unique. Les conditions exactes restent celles de chaque fichier et paquet. Cette liste ne constitue pas un audit juridique exhaustif ni une autorisation supplémentaire de redistribution. Les paquets proviennent des dépôts officiels Debian utilisés par la construction ; les signatures APT sont conservées.

## Inventaire observé

| Paquet | Version | Section Debian |
|---|---|---|
| `amd64-microcode` | `3.20250311.1` | `non-free-firmware/admin` |
| `atmel-firmware` | `1.3-7.1` | `non-free-firmware/kernel` |
| `bluez-firmware` | `1.2-13` | `non-free-firmware/kernel` |
| `dahdi-firmware-nonfree` | `2.11.1.0.20170917-3` | `non-free-firmware/comm` |
| `firmware-amd-graphics` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-ast` | `20140808-10` | `non-free-firmware/kernel` |
| `firmware-ath9k-htc` | `1.4.0-110-ge888634+dfsg1-0.1` | `kernel` |
| `firmware-atheros` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-bnx2` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-bnx2x` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-brcm80211` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-carl9170` | `1.9.9-450-gad1c721+dfsg-0.1` | `kernel` |
| `firmware-cavium` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-cirrus` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-intel-graphics` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-intel-misc` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-intel-sound` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-ipw2x00` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-ivtv` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-iwlwifi` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-libertas` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-linux` | `20250410-2` | `non-free-firmware/metapackages` |
| `firmware-linux-free` | `20241210-2` | `kernel` |
| `firmware-linux-nonfree` | `20250410-2` | `non-free-firmware/metapackages` |
| `firmware-marvell-prestera` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-mediatek` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-misc-nonfree` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-myricom` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-netronome` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-netxen` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-nvidia-graphics` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-nvidia-tesla-535-gsp` | `535.274.02-1~deb13u1` | `non-free-firmware/kernel` |
| `firmware-qlogic` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-realtek` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-siano` | `20250410-2` | `non-free-firmware/kernel` |
| `firmware-sof-signed` | `2025.01-1` | `non-free-firmware/kernel` |
| `firmware-zd1211` | `1:1.5-13` | `non-free-firmware/kernel` |
| `intel-microcode` | `3.20251111.1~deb13u1` | `non-free-firmware/admin` |

Les textes de licence et attributions sont disponibles dans le Live sous `/usr/share/doc/PAQUET/copyright`, en remplaçant PAQUET par le nom exact ci-dessus. Le manifeste final `packages.txt` accompagne chaque ISO produite ; les preuves de cet inventaire sont dans `logs/firmware-inventory.json`.

## NVIDIA et pilotes historiques

Les seuls noms de paquets contenant `nvidia` dans le manifeste observé sont `firmware-nvidia-graphics` et `firmware-nvidia-tesla-535-gsp`. Ce dernier est un **paquet de firmware GSP**, et non l’installation d’un pilote historique NVIDIA 304, 340 ou 390. Le paquet source `nvidia-graphics-drivers-tesla-535` indiqué dans ses métadonnées ne change pas la nature du paquet binaire inclus.

Aucun paquet binaire portant les noms `nvidia-driver`, `nvidia-kernel-dkms` ou les familles `nvidia-legacy-304*`, `nvidia-legacy-340*`, `nvidia-legacy-390*` n’est présent dans le manifeste observé. Cette constatation repose sur les noms exacts de paquets, pas sur la ressemblance entre un nom de carte et un nom de pilote. Aucun pilote Windows `.sys` n’est ajouté par la configuration Kyronext.

Le gestionnaire de diagnostic ne télécharge, ne charge, ne remplace et ne supprime aucun pilote. La compatibilité matérielle reste documentée séparément dans `HARDWARE_COMPATIBILITY.md`.
