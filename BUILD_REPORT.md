# Rapport de compilation — Kyronext OS 0.1

Date : 31 octobre 2026
Base : Debian 13 Trixie AMD64, noyau Debian `6.12.113+deb13-amd64`, KDE Plasma 6, live-build.
Constructeur : VM Debian isolée, disque virtuel uniquement.

## Artefact

- ISO : `releases/20261031T1854-suite-search-preview/Kyronext-OS-0.1-amd64.iso`
- Taille : **2 454 208 512 octets** (environ 2,45 Go), sous le plafond de 16 Go.
- SHA-256 : `631637ee963f8ed2e3660aa9e2cdef39ad53c397e6a12b02e37c5c31123704b4`
- Manifeste : `releases/20261031T1854-suite-search-preview/SHA256SUMS`
- Paquets installés : `releases/20261031T1854-suite-search-preview/packages.txt`

## Résultat de construction

La phase binaire live-build s'est achevée (`P: Binary stage completed`). L'image est hybride : xorriso a confirmé les entrées BIOS/ISOLINUX et UEFI/GRUB, le MBR isohybrid, GPT et APM. Les journaux de compilation et de reconstruction du chroot sont dans le dossier `logs/` de la release.

Le système comprend Plasma, Firefox ESR Debian, Kyronext Welcome, Launchpad et la suite native `kyronext-suite-preview 0.1.0-0~preview2`. Le paquet de prévisualisation a ses propres tests et a été installé puis retiré dans une VM Debian isolée. Navigator est présent comme option expérimentale ; Firefox ESR reste le choix recommandé. Les fonctions IA lourdes et l'installateur ne sont pas inclus.

Un contrôle manuel après la phase binaire a d'abord cherché l'ISO dans un sous-dossier `binary/`, alors que live-build la produit à la racine du répertoire de construction. La phase binaire s'était achevée normalement. Le script du dépôt et le transfert ont ensuite utilisé le bon chemin ; l'empreinte locale correspond à celle du constructeur.

## Vérifications réalisées

- UEFI avec QEMU/WHPX, VGA standard, 3 Gio : menu, démarrage Live, session KDE et Welcome Kyronext observés.
- Kyronext Monitor ouvert dans la session UEFI : cartes CPU, mémoire, réseau et graphiques ainsi que ses courbes visibles en direct. Capture : `validation/qemu-whpx-uefi-monitor.png`.
- BIOS avec QEMU/WHPX, VGA standard, 3 Gio : menu puis session KDE et Welcome observés. Capture : `validation/qemu-whpx-bios-desktop.png`.
- Confirmation d'arrêt KDE ouverte dans le Live BIOS : marque Kyronext OS au centre, sans logo Debian. Capture : `validation/qemu-whpx-bios-shutdown.png`.
- Les deux VM étaient dépourvues de disque virtuel. Aucun disque physique ni aucune clé USB n'a été utilisé.
- Sommes vérifiées lors du transfert entre le constructeur Debian et Windows ; elles correspondent à la ligne du manifeste ci-dessus.

Les captures et les journaux QEMU sont dans `releases/20261031T1854-suite-search-preview/validation/`.

## Limites

Il s'agit d'une candidate **Live Developer Preview**, et non d'une version stable ou d'un installateur. L'installation sur disque, Calamares, persistance, Secure Boot, chiffrement et restauration ne sont pas validés. Le HP EliteBook 2540p et tout autre matériel physique restent à tester. Le démarrage QEMU ne certifie ni le Wi-Fi, ni l'audio, ni les températures ou performances d'un ordinateur réel.
