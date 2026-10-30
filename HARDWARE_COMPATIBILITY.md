# Compatibilité matérielle Kyronext OS

Aucun ordinateur physique n'a été testé ou déclaré compatible. Le démarrage du constructeur Debian n'est pas un test de l'ISO Kyronext. Les observations ci-dessous concernent exclusivement des périphériques émulés.

Statuts : **compatible et testé**, **compatible selon la documentation, non testé**, **compatibilité partielle**, **non pris en charge**, **statut inconnu**. Une correspondance de nom ou de modalias ne suffit pas à valider le fonctionnement. Pour une machine complète, exiger démarrage, affichage, réseau et périphériques essentiels réellement testés.

## Matrice observée — 30 octobre 2026

| Fabricant/modèle | PCI/USB | Pilote utilisé | Noyau | Résultat | Limites et preuves |
|---|---|---|---|---|---|
| QEMU VGA standard émulée, BIOS | PCI1234:1111 | bochs-drm ; module bochs | 6.12.113+deb13-amd64 | Compatible et testé dans cette VM | KDE1280x800, Konsole et Dolphin ; logs/bios-hardware-verified.png |
| QEMU Ethernet VirtIO émulé, BIOS | PCI1af4:1000 | Liaison PCI virtio-pci observée | 6.12.113+deb13-amd64 | Compatible et testé dans cette VM | DNS et ping passerelle NAT ; pas de contrôleur physique testé |
| QEMU clavier/souris émulés, BIOS | Identifiants non relevés | Pilotes non relevés | 6.12.113+deb13-amd64 | Fonctionnement testé dans cette VM | Saisie française, pointeur et menu contextuel ; logs/bios-native-mouse.png |
| QEMU VGA standard, UEFI/OVMF | Identifiants non relevés dans cette session | bochs-drm observé au démarrage | 6.12.113+deb13-amd64 | Compatibilité partielle | Bureau et terminal valides ; artefact initial avant DRM |
| VirtualBox VMSVGA, UEFI | Identifiants non relevés | Pilote non relevé | Non relevé dans cette session | Compatibilité partielle | Bureau et accueil observés ; réseau/périphériques non testés ici |
| QEMU VirtIO graphique, premiers essais | Identifiants non relevés | Pilote non relevé | 6.12.113+deb13-amd64 | Statut inconnu | Écran noir du profil essayé, cause non établie ; aucune conclusion sur un GPU physique |

Profil QEMU de référence : QEMU 11.1.0 sous Windows 11 Pro, machine q35, CPU qemu64, 1 vCPU, RAM 3072 Mio, TCG thread=single, carte VGA std, réseau NAT. ISO SHA-256 b73af49221f3dd1871390ae6e8041d7db65de834951d5a0687afcf92b3f76ac2.

## Campagne physique à réaliser

Intel GMA/HD/UHD ; NVIDIA GeForce/Quadro/Tesla avec Nouveau lorsque pertinent ; AMD/ATI avec radeon/amdgpu ; Intel/Realtek/Broadcom/Qualcomm Atheros/Marvell pour le réseau ; IDE/PATA, SATA/AHCI, NVMe, RAID et USB pour le stockage ; audio Intel/Realtek/Creative ; webcams, Bluetooth, PS/2, USB1/2/3, pavés/écrans tactiles et lecteurs de cartes ; imprimantes et scanners.

Le statut de chaque modèle physique reste **inconnu** tant qu'il n'a pas été identifié et testé, ou relié à une documentation précise et alors marqué « documentation, non testé ». Ces noms définissent la campagne, pas une promesse de prise en charge. Voir hardware/README.md et hardware/legacy-driver-manager/README.md.
