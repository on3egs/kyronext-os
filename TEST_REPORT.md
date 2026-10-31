# Validation de la candidate Live intégrant la suite — 20261031T1854-suite-search-preview

Date : 31 octobre 2026. Candidate Debian 13 Trixie AMD64 avec noyau Debian `6.12.113+deb13-amd64`.

Artefact : `releases/20261031T1854-suite-search-preview/Kyronext-OS-0.1-amd64.iso`
Taille : **2 454 208 512 octets** (environ 2,45 Go, plafond 16 Go).
SHA-256 : `631637ee963f8ed2e3660aa9e2cdef39ad53c397e6a12b02e37c5c31123704b4`.

| Vérification | Résultat observé |
|---|---|
| live-build | Phase binaire terminée. Les journaux de build/chroot, la liste des paquets et le rapport sont dans la release. Le wrapper a ensuite cherché l'ISO dans le mauvais sous-dossier ; le fichier réel à la racine de build existe et ses sommes locale et distante correspondent. |
| Format | Image hybride avec BIOS ISOLINUX, UEFI GRUB, MBR isohybrid, GPT et APM, confirmés par xorriso. |
| Contenu | Debian 13, Plasma, Firefox ESR Debian, Kyronext Welcome, Launchpad et paquet `kyronext-suite-preview 0.1.0-0~preview2`. Le manifeste de paquets se trouve dans la release. |
| QEMU UEFI — WHPX | **Réussi visuellement** : menu Kyronext, démarrage Live, bureau KDE et Welcome. Kyronext Monitor a été lancé ; ses mesures et graphiques sont visibles. Capture : `releases/20261031T1854-suite-search-preview/validation/qemu-whpx-uefi-monitor.png`. |
| QEMU BIOS — WHPX | **Réussi visuellement** : menu Kyronext, écran d'accueil puis bureau KDE/Welcome. Capture : `releases/20261031T1854-suite-search-preview/validation/qemu-whpx-bios-desktop.png`. |
| Marque dans la confirmation d'arrêt | **Vérifiée dans le Live BIOS** : la confirmation KDE affiche l'identité Kyronext OS ; aucun logo Debian n'est visible. Capture : `releases/20261031T1854-suite-search-preview/validation/qemu-whpx-bios-shutdown.png`. |
| Tests des applications | Sur Debian 13, CTest 16/16 ; tests Python 45 réussis et 2 ignorés pour les limites de plateforme Windows. Le paquet a été installé puis retiré dans une VM isolée. Ces résultats ne remplacent pas l'essai interactif de chaque application. |
| Launchpad | Smoke test UI et recherche sur des dossiers temporaires réussi dans le chroot Debian ; l'application est incluse dans l'image. Son usage interactif complet et l'accessibilité restent à valider. |
| Sécurité de l'image | Aucun hash de mot de passe Live ni aucune clé SSH personnelle n'est intégré à cette candidate. Aucun mot de passe sudo par défaut n'est défini. |
| Matériel réel | HP EliteBook 2540p, Wi-Fi, audio, températures et périphériques physiques non testés. Aucune compatibilité physique n'est déclarée. |
| Installation | Calamares, persistance et installation sur disque ne sont pas inclus/validés dans cette image. QEMU n'avait aucun disque virtuel ; aucune clé USB ni aucun disque physique n'a été touché. |

Une première tentative BIOS faite pendant que la VM UEFI réservait encore sa mémoire a échoué à l'allocation WHPX ; elle a été relancée seule et a démarré correctement. Les avertissements CPU WHPX n'ont pas empêché le démarrage. La capture et le journal décrivent le test réussi.

Cette candidate est une **Live Developer Preview**. Elle ne termine pas toutes les applications prévues dans la feuille de route ; Navigator reste expérimental et Firefox ESR demeure recommandé. Elle n'est pas déclarée stable ni prête à installer sur un ordinateur réel.

---

# Validation antérieure — candidate Live propre 20261031T175731-clean-live

## Validation complémentaire de la suite Qt — 31 octobre 2026

- Build complet des applications C++/Qt sous Debian 13 AMD64, compilateur GCC 14.2 et CMake/Ninja : réussi.
- CTest hors écran dans une session D-Bus : **16/16 groupes réussis**.
- Tests Python locaux après l'ajout de la recherche : **45 réussis, 2 ignorés** sur Windows (cas Linux/sysfs et création de symlink indisponible) ; les 9 tests du backend de recherche passent sur Debian 13, symlink inclus.
- Le smoke test PyQt6 de Launchpad passe en Windows/offscreen sans lancer d'indexation ; les dialogues et l'ouverture interactive de fichiers dans KDE restent à valider.
- Paquet `kyronext-suite-preview 0.1.0-0~preview2` construit (439 704 octets) ; simulation APT hors ligne, installation `dpkg`, vérification des exécutables Monitor/AMP/Navigator, puis retrait du paquet : réussis dans la VM isolée.
- SHA-256 du paquet : `9ff4282b131185a60a62e9fec6bedf97450d78ded16a48408bcbb9865d877e2d`.
- Le paquet est disponible dans `releases/apps-preview-20261031/`. Il n'est pas intégré à l'ISO propre mentionnée ci-dessous. Navigator reste expérimental pour la raison de maintenance de sécurité documentée dans son README.
- La suite n'a pas été testée sur le HP EliteBook 2540p ; un build/test Qt hors écran ne remplace pas une validation graphique interactive.
- Vérification HTTP(S) des liens depuis Windows : `www.kitt-franco-belge.be` redirige en HTTPS vers `kitt-franco-belge.be` (200, certificat validé par curl) ; `kyronext.eu` répond en HTTPS (200, certificat validé). `kironext.be` ne résout pas en DNS lors du contrôle : aucun remplacement par `kyronext.be` n'a été fait. Les clics depuis Welcome/Navigator restent à tester.

Date : 31 octobre 2026. Build Debian 13 Trixie AMD64 avec live-build ; noyau embarqué `6.12.113+deb13-amd64`.

Artefact : `releases/20261031T175731-clean-live/Kyronext-OS-0.1-amd64.iso`

Taille : **2 451 800 064 octets** (plafond : 16 000 000 000).

SHA-256 : `ad67f47ede2170f5a9fcc2b029b0d139cca5b8d8282da380f942c913927ccadd`.

| Vérification | Résultat observé |
|---|---|
| Build live-build | Phase binaire terminée ; journal et liste des paquets conservés dans le répertoire de release. Le fichier ISO a été transféré depuis la VM Debian et le SHA-256 local correspond au SHA-256 distant. |
| Format hybride | `xorriso` confirme l'entrée BIOS ISOLINUX, l'entrée UEFI GRUB, MBR isohybrid, GPT et APM. |
| Accès privés | Dans le squashfs, `/etc/kyronext/live-password.hash` et `/etc/kyronext/ssh/authorized_keys` sont absents. Le service SSH est conditionné à la présence de la clé autorisée. Aucun mot de passe sudo n'est défini ; le compte Live est verrouillé pour le mot de passe. |
| Tests de sources | 36 tests Python : 35 réussis et 1 ignoré car spécifique aux chemins sysfs Linux ; `tests/check_sources.py` réussi ; syntaxe PowerShell du test QEMU validée. |
| QEMU UEFI — WHPX | **Réussi visuellement** : menu, puis bureau KDE et accueil Kyronext visibles (QEMU 11, q35, VGA standard, 3 Gio, aucun disque). Capture `validation/qemu-whpx-uefi-clean-desktop.png`. |
| QEMU BIOS — WHPX | **Réussi visuellement** : menu, puis le même bureau KDE et accueil Kyronext visibles, sans disque virtuel. Capture `validation/qemu-whpx-bios-desktop.png`. |
| QEMU UEFI — TCG, VGA standard | Menu visible ; l'image devient brouillée après sélection et aucun bureau n'a été validé. |
| QEMU UEFI — TCG, VirtIO et entrée Legacy | Console de démarrage lisible après deux minutes, puis écran noir à trois minutes ; aucun bureau validé. La lenteur et le rendu TCG restent un défaut de cette configuration de test. |
| Vérification systemd par SSH | Non exécutée sur cette candidate : SSH a été volontairement désactivé faute de clé privée. L'accueil visuel prouve l'ouverture de session graphique, pas l'absence d'unités systemd en échec. |
| HP EliteBook 2540p, Wi-Fi, audio et périphériques réels | Non testés physiquement. Aucune compatibilité matérielle n'est déclarée. |
| Installation sur disque, persistance, Calamares | Non testés et non inclus. L'image est une ISO Live, pas un installateur. Aucun disque physique ou USB n'a été touché. |

Les captures BIOS/UEFI WHPX sont des preuves visuelles de démarrage de bureau dans QEMU seulement ; elles ne certifient pas un PC réel. Les captures de tous les essais et le journal de compilation sont dans `releases/20261031T175731-clean-live/validation/` et le dossier de release.

L'ancienne candidate `20261031T1542-legacy-fbdev` reste une image privée avec accès personnels. De plus, l'historique Git local contient un ancien hash de mot de passe ; ne publie pas cet historique avant nettoyage, même si la nouvelle ISO est sans accès privé.

---

# Validation antérieure — candidate privée 20261031T1542-legacy-fbdev

ISO : `releases/20261031T1542-legacy-fbdev/Kyronext-OS-0.1-amd64.iso`
Taille : **2 451 800 064 octets** (sous le plafond décimal de 16 Go).
SHA-256 : `c15fbd4e7f1e81b339f53ec1c4edba5bcb165aacc3d15361edfc82132a3d4e38`. La somme Windows correspond à celle calculée côté constructeur Debian. Paquet intégré : `kyronext-suite-preview 0.1.0-0~preview2`, SHA-256 `80a558d1704722d1e2441c005501038a70d37d1618960c19253b0525a7e0011f`.

| Contrôle | Résultat observé |
|---|---|
| ISO hybride | Présence des entrées BIOS ISOLINUX et UEFI GRUB, MBR isohybrid, GPT et APM vérifiés par xorriso. |
| UEFI standard | Menu, session Live KDE Plasma Wayland et Kyronext Welcome visibles dans VirtualBox. Noyau Debian `6.12.113+deb13-amd64`; aucune unité systemd en échec au contrôle. |
| UEFI « Legacy Compatibility » | Démarrage visible en X11. `nomodeset kyronext.legacy=1` confirmé. `/dev/fb0` existe et la configuration Xorg `fbdev` est générée automatiquement par le hook ; `kwin_x11` et `plasmashell` sont actifs. |
| BIOS standard | Menu et session Live KDE Plasma Wayland visibles dans VirtualBox ; aucune unité systemd en échec au contrôle. |
| BIOS « Legacy Compatibility » | Bureau visible en X11 ; `nomodeset kyronext.legacy=1` confirmé ; `kwin_x11` et `plasmashell` actifs. Ce chemin n'avait pas `/dev/fb0` et a utilisé le repli Xorg par défaut. |
| Kyronext Monitor | Démarré dans la session UEFI standard. CPU, RAM et réseau affichent des mesures réelles de la VM ; GPU et température indiqués comme non mesurés. |
| Tests de sources | Tests Python locaux antérieurs : 36 cas, 35 réussis et 1 ignoré (cas Linux/sysfs non exécutable sous Windows). `tests/check_sources.py` et `git diff --check` réussis avant la dernière mise à jour documentaire. Tests Qt précédents compilés sous Debian : 16/16 CTest. Ces résultats ne valident pas chaque application dans l'ISO. |
| QEMU UEFI/OVMF — WHPX | Réussi sous Windows avec q35, VGA standard et 3 Gio : Kyronext Welcome visible ; SDDM, `kwin_wayland` et `plasmashell` actifs ; aucune unité systemd en échec. Le transfert SSH de diagnostic était lié uniquement à `127.0.0.1`. |
| QEMU UEFI/OVMF — TCG | TCG single-thread très lent : le menu et la console sont visibles, puis la capture demeure noire. À un point de contrôle SSH, SDDM et KWin existaient, mais Plasma n’avait pas fini de démarrer. Aucun bureau TCG validé. QEMU BIOS non testé sur cette candidate. |
| HP EliteBook 2540p et périphériques physiques | Non testés. Le démarrage en VM ne prouve pas le fonctionnement du Wi-Fi, de l'audio, du GPU ou des autres périphériques du HP. |
| Installation sur disque, Calamares et USB | Non testés ; aucune écriture sur support physique. Calamares n'est pas intégré ni activé. |

Comparaison QEMU Windows/OVMF : avec WHPX, la même ISO affiche le bureau complet en environ une minute, et SSH confirme `sddm`, `kwin_wayland`, `plasmashell` et zéro unité échouée. En TCG single-thread, le bureau n’a pas été visible dans le délai observé ; SSH a confirmé que le lancement de Plasma restait incomplet lors du contrôle. La cause exacte du rendu TCG n’est pas isolée. QEMU BIOS reste à tester. Captures et journaux dans `releases/20261031T1542-legacy-fbdev/validation/`.

Captures : `releases/20261031T1542-legacy-fbdev/validation/`. Les profils BIOS/UEFI et normal/Legacy sont tous visibles en VM ; la compatibilité physique reste inconnue. La candidate demeure **privée** : elle contient la clé publique SSH du propriétaire et un accès temporaire d'administration Live. Ne pas publier ni partager. Il faut retirer ces éléments et reconstruire avant distribution publique.

Avertissement mineur de xorriso : l'étiquette Joliet `KYRONEXT_0_1_AMD64` dépasse la recommandation de 16 caractères. Le volume ISO reste lisible ; aucun échec n'a été observé.

---

## Candidate privée 20261031T145700Z-legacy-compat — validation VirtualBox, 31 octobre 2026

Artefact : `releases/20261031T145700Z-legacy-compat/Kyronext-OS-0.1-amd64.iso`

Taille : **2 451 800 064 octets**. SHA-256 : `fa1179d315c8f8da4faba5755e855a9d572e5b45914aa7fb51b047a842e0a55d` ; l'empreinte calculée sous Debian et Windows correspond. La compilation complète live-build a réussi sous Debian 13 Trixie AMD64. L'image possède les entrées de démarrage BIOS et UEFI et contient le paquet `kyronext-suite-preview` (`0.1.0-0~preview2`, SHA-256 `80a558d1704722d1e2441c005501038a70d37d1618960c19253b0525a7e0011f`). Le plafond ISO de 16 000 000 000 octets et la réserve pilotes/firmwares de moins de 1 Go passent.

| Vérification | Résultat observé |
|---|---|
| Démarrage UEFI standard | Réussi dans la VM VirtualBox dédiée : Live Debian 13, KDE Plasma Wayland, compte utilisateur Kyronext et Welcome visibles. |
| Démarrage BIOS standard | Réussi dans la même VM, BIOS Legacy : bureau KDE et Welcome visibles ; `kwin_wayland` et `plasmashell` actifs. |
| Kyronext Monitor | Le paquet présent dans l'ISO démarre dans le bureau Live BIOS et UEFI ; CPU, mémoire et réseau mesurés, métriques GPU explicitement indiquées comme indisponibles. Captures dans `validation/`. |
| Profil « Legacy Compatibility » | L'entrée s'affiche et transmet `nomodeset kyronext.legacy=1`. Sous VirtualBox VMSVGA, SDDM choisit bien Plasma X11, mais le pilote virtuel `vmware` ne peut mapper son framebuffer et Xorg plante ; le bureau de ce profil n'est donc **pas validé** dans cette VM. |
| Contrôleur VBoxVGA de diagnostic | L'écran reste noir ; ce deuxième adaptateur virtuel ne fournit pas une validation graphique exploitable. La VM a ensuite été remise en EFI64/VMSVGA. |
| Services en BIOS | Cible systemd graphique atteinte, SSH/NM/SDDM actifs, aucune unité échouée au moment du contrôle. |
| Disque attaché à la VM | Aucun disque virtuel ; seulement l'ISO. Aucun disque ou support physique modifié. |
| QEMU/KVM pour cette candidate | Non exécuté. Les anciens résultats QEMU ne sont pas attribués à cette image. |
| HP EliteBook 2540p | Non testé physiquement ; aucune compatibilité matérielle réelle déclarée. Audio/Wi-Fi/GPU physiques, installation sur disque et Calamares restent non validés. |

Cette image est destinée à un essai privé et ne doit pas être publiée : elle inclut la clé publique SSH du propriétaire et un accès sudo temporaire pour la session Live. Avant publication, retirer ces accès et reconstruire l'image. Elle n'a pas été écrite sur une clé USB.

Tests complémentaires de sources : 35 tests Python réussis, 1 test Linux/sysfs ignoré sous Windows ; budgets 7/7, pilotes 17/17, liens 8/8, catalogue 4/4, veille 4/4 ; contrôle statique des sources réussi. Suite Qt construite sous Debian (166/166 étapes, CTest 16/16). Ces tests ne valident pas toutes les applications dans le Live.

Captures et traces : `releases/20261031T145700Z-legacy-compat/validation/`, ainsi que `logs/`. Rapport détaillé et limites dans `TEST_REPORT.md`, `TEST_REPORT_SUITE.md` et `KNOWN_ISSUES.md`.

---

﻿# Validation Kyronext OS 0.1 — 31 octobre 2026

## Release de test validée — 20261031T0334-live-account-fix

ISO : `releases/20261031T0334-live-account-fix/Kyronext-OS-0.1-amd64.iso`
Taille : 1 762 754 560 octets.
SHA-256 Windows : `1f3165e754d9afe3efebd96e1e3cdb4d911b22e83e12efe7f1e919ff0d4ce90c`. L'empreinte calculée dans Debian correspond exactement à celle de la copie Windows.

| Test | Résultat observé |
|---|---|
| Construction live-build | Réussie ; ISO hybride produite par xorriso, logs du constructeur archivés sous `logs/20261031-sudo-fix2-rebuild.log` |
| Menu UEFI VirtualBox | Réussi ; menu Kyronext avec modes normal et fail-safe visible |
| Bureau KDE en UEFI | Réussi ; session Live Kyronext et Welcome atteints |
| Compte et groupes Live | Réussi ; `groups` affiche le groupe `sudo` |
| Authentification sudo | Réussie ; `sudo id` avec le secret temporaire configuré pour cette image privée retourne `uid=0(root)` ; root reste verrouillé |
| Démarrage BIOS Legacy VirtualBox | Réussi ; menu Kyronext puis bureau KDE Live visibles |
| Empreinte du transfert | Réussie ; SHA-256 Debian et Windows identiques |
| Écriture USB physique | Non exécutée : Windows a annulé UAC avant le lancement du writer. Le journal est absent et le volume F: reste monté ; aucune donnée de clé n'a été modifiée. |
| Tests branding bureau | 6 tests réussis (`test_desktop_branding.py`) |
| Tests source du compte/capture | 3 tests réussis (`test_live_accessibility.py`) |
| Vérification statique des sources | Réussie (`tests/check_sources.py`) |
| Syntaxe shell du hook de compte | `dash -n` réussie dans le chroot |
| Tests CTest de Kyronext Suite Preview | 11 groupes précédemment exécutés avec succès dans le constructeur Debian ; la suite n'a pas été recompilée pendant le correctif du hook |
| Installation disque / Calamares | Non exécutée ; installateur désactivé |
| Matériel physique HP EliteBook 2540p | Pas encore testé ; le démarrage VM ne prouve pas la compatibilité de ses périphériques |
| Capture d'écran sur le HP | Non testée ; `kde-spectacle` et le repli Kyronext Capture sont inclus/testés au niveau source, pas au clavier physique |
| Audio, Wi-Fi, GPU physique, imprimante, chiffrement, Secure Boot, IA locale | Non testés ou absents |

Captures dans `logs/` : `live-account-fix-uefi-menu.png`, `live-account-fix-desktop.png`, `live-account-fix-groups2.png`, `live-sudo-success.png`, `legacy-bios-menu.png`, `legacy-bios-desktop.png`.

L'image précédente `20261031T031250Z-sudo-fix` a été conservée comme trace de diagnostic, mais n'est pas validée : son script se trouvait dans un composant live-config inactif. Ne pas la copier sur une clé.

## Historique des validations antérieures
# Validation actuelle — release 20261031T0334-live-account-fix

ISO : `releases/20261031T0334-live-account-fix/Kyronext-OS-0.1-amd64.iso`
Taille : 1 762 754 560 octets. SHA-256 : `1f3165e754d9afe3efebd96e1e3cdb4d911b22e83e12efe7f1e919ff0d4ce90c`. L'empreinte calculée dans Debian correspond exactement à celle de la copie Windows.

| Test | Résultat observé |
|---|---|
| Construction Debian 13 live-build | Réussie ; ISO hybride produite par xorriso |
| Menu et session Live UEFI VirtualBox | Réussis ; menu Kyronext puis bureau KDE/Welcome |
| Groupes du compte Live | Réussi ; `groups` affiche `sudo` |
| Authentification sudo | Réussie ; `sudo id` avec le secret temporaire configuré pour cette image privée retourne `uid=0(root)` ; root reste verrouillé |
| Démarrage Legacy BIOS VirtualBox | Réussi ; menu Kyronext et bureau KDE Live visibles |
| Transfert ISO Debian vers Windows | Réussi ; SHA-256 identique |
| Tests branding bureau | 6 réussis (`test_desktop_branding.py`) |
| Tests source compte/capture | 3 réussis (`test_live_accessibility.py`) |
| Vérifications statiques des sources | Réussies (`tests/check_sources.py`) |
| Syntaxe shell du hook dans le chroot | `dash -n` réussi |
| Tests CTest Kyronext Suite Preview | 11 groupes déjà exécutés avec succès dans le constructeur avant ce correctif ; la suite n'a pas été recompilée durant cette correction |
| Installation sur disque / Calamares | Non exécutée ; installateur non activé |
| HP EliteBook 2540p et pilotes physiques | Non testés ; aucune compatibilité matérielle réelle revendiquée |
| Capture Print Screen sur HP | Non testée physiquement ; `kde-spectacle` et le fallback Kyronext Capture présents, test source uniquement |
| Audio, Wi-Fi, GPU physique, imprimantes, Secure Boot, chiffrement, IA locale | Non testés ou absents |

Captures disponibles dans `logs/` : `live-account-fix-uefi-menu.png`, `live-account-fix-desktop.png`, `live-account-fix-groups2.png`, `live-sudo-success.png`, `legacy-bios-menu.png`, `legacy-bios-desktop.png`.

L'image précédente `20261031T031250Z-sudo-fix` est conservée comme trace, mais n'est pas validée : son script était placé dans un composant live-config inactif. Ne pas l'utiliser.

---
# Validation Kyronext OS 0.1 — 30 octobre 2026

## Contrôle visuel du branding — 20261030T225146Z-155866 (31 octobre)

ISO : [Kyronext-OS-0.1-amd64.iso](releases/20261030T225146Z-155866/Kyronext-OS-0.1-amd64.iso). Le transfert depuis le constructeur a été vérifié : 1 582 940 160 octets et SHA-256 `855ba54b168edc6f95aca49366b3ac0f4b5a004f0b7d5925fc47fa138e81bfd3` identique au manifeste distant.

| Contrôle | Résultat observé |
|---|---|
| Construction Debian 13 AMD64, BIOS/UEFI hybride | Réussie ; xorriso a écrit 772 920 secteurs et live-build a terminé sans erreur |
| Démarrage UEFI dans VirtualBox | Réussi ; menu Kyronext, puis session Live et accueil Kyronext visibles |
| Accueil Kyronext | Réussi ; les liens KITT Franco-Belge, Kyronext officiel et boutique sont affichés |
| Confirmation d'arrêt Plasma | Réussie visuellement ; logo Kyronext et nom Kyronext OS visibles |
| Identité du profil dans Kickoff | Échec observé ; « Debian Live user » et l'avatar Debian demeurent. Le GECOS réellement lu dans la VM était encore « Debian Live user ». Ce défaut a déclenché un correctif source post-live-config et un nom complet explicite sur la ligne de démarrage ; ils restent à inclure dans une autre ISO et à retester. |
| Test automatisé de configuration de branding | 6 tests réussis après le correctif source |
| Tests statiques du dépôt | Réussis après le correctif source |

Le résultat de cet essai est donc mixte : la boîte d'arrêt ciblée est corrigée, mais le menu n'est pas encore corrigé dans l'ISO transférée. Les captures `releases/20261030T225146Z-155866/vm-boot.png`, `vm-menu.png` et `vm-shutdown.png` conservent les observations.

## Nouvelle construction Firefox — 20261030T202137Z-16799

Artefacts : [ISO Kyronext OS 0.1](releases/20261030T202137Z-16799/Kyronext-OS-0.1-amd64.iso), [paquet Kyronext Suite Preview](releases/20261030T202137Z-16799/kyronext-suite-preview_0.1.0-0~preview1_amd64.deb), manifeste de paquets et sommes dans le même dossier.

Taille ISO : **1 582 940 160 octets**. SHA-256 : `57f9b1a9d498863700b73353c14e888ce29b95734cbc65e84ae6a09a4b0b6c9b`. Le manifeste de sommes a été vérifié sous Debian puis recalculé sous Windows après transfert.

| Test | Résultat observé |
|---|---|
| Construction live-build | Réussie, Debian 13 Trixie AMD64, noyau 6.12.113+deb13-amd64, 989 paquets configurés dans le chroot |
| Contenu du manifeste | `live-task-kde`, `firefox-esr` 153.6.0esr, `kyronext-suite-preview` 0.1.0-0~preview1 ; aucun Calamares |
| Structure hybride | `xorriso` confirme les entrées El Torito BIOS et UEFI, MBR isohybrid et partition EFI |
| Démarrage BIOS Legacy | Réussi dans VirtualBox ; menu Kyronext puis session KDE Live et Kyronext Welcome visibles. Capture : `build/vms/Kyronext-Firefox-BIOS-Test/bios-desktop2.png` |
| Démarrage UEFI | Réussi dans VirtualBox ; menu Kyronext, délai d'initialisation graphique, puis session KDE Live et Kyronext Welcome visibles. Capture : `build/vms/Kyronext-Firefox-UEFI/desktop-second.png` |
| Firefox Kyronext | L'entrée « Firefox — thème Kyronext » ouvre Firefox ESR avec chrome sombre et accent rouge. Capture : `build/vms/Kyronext-Firefox-UEFI/firefox-start.png` |
| Page de démarrage Firefox hors ligne | **Échec visuel** : un onglet « New Tab » vide est apparu au lieu de Kyronext Start ; à corriger avant de considérer cette intégration terminée |
| Suite C++/Qt et navigateur Navigator | Les 11 groupes CTest et 7 tests du lanceur Firefox avaient réussi dans le constructeur Debian avant cette compilation. Navigator est inclus mais son lancement graphique dans cette ISO n'a pas été vérifié |

## Nouvelle construction avec tentative de correction Firefox — 20261030T213008Z-85730

| Vérification | Résultat |
|---|---|
| Construction live-build | Réussie dans Debian 13 ; ISO hybride de 1 582 940 160 octets |
| SHA-256 | `b4bfdda565b9487875229519e5f0ae13175af7a34f9d7c26d820220d9b358a7d` ; identique entre le manifeste Debian et le fichier rapatrié sous Windows |
| Tests unitaires Firefox | 9 réussis dans Debian |
| Démarrage UEFI dans VirtualBox | Réussi jusqu'au menu Kyronext, puis Welcome et bureau KDE Live |
| Firefox ESR | Le lanceur ouvre bien l'URL locale `file:///home/kyronext/.local/share/kyronext/firefox/start/index.html`, mais la zone de contenu reste vide. **Échec visuel confirmé** ; la tentative de correction par `--new-window` n'a pas résolu le défaut |
| Firefox en mode headless sur la VM de construction | Non concluant : le processus est resté bloqué lors de l'initialisation du compositeur sans sortie graphique ; processus arrêté ensuite |
| BIOS Legacy sur cette nouvelle ISO | Non exécuté ; l'image précédente avait démarré en BIOS |

La nouvelle ISO est conservée comme artefact de diagnostic, mais l'intégration Kyronext Start dans Firefox n'est pas validée. L'ISO précédente reste inchangée. Capture du démarrage UEFI : `build/vms/Kyronext-Firefox-UEFI/firefox-fix-desktop.png`. Capture Firefox : `build/vms/Kyronext-Firefox-UEFI/firefox-start-fix-later.png`.

Le menu et le bureau ont donc été réellement observés dans les deux modes firmware. Ces essais VirtualBox ne valident pas tous les ordinateurs physiques, le réseau matériel, l'audio réel ni une installation sur disque.

## Révisions de personnalisation

Le détail des contrôles des révisions visuelles se trouve dans
[docs/VALIDATION_VISUELLE.md](docs/VALIDATION_VISUELLE.md).
La section ci-dessous conserve les résultats de la première base live-build ;
ils ne doivent pas être attribués automatiquement aux images suivantes.

## Image produite

Fichier : releases/20261030T121627Z-1881/Kyronext-OS-0.1-amd64.iso

Taille : **1 484 423 168 octets**.

SHA-256 : `b73af49221f3dd1871390ae6e8041d7db65de834951d5a0687afcf92b3f76ac2`

Fabrication live-build terminée dans une VM Debian 13.7 indépendante. Empreinte recalculée après copie sous Windows : concordance. Cette image est un **Live sans installateur actif**.

## Tests réussis

| Test | Observation et preuve |
|---|---|
| Syntaxes Bash, Python, PowerShell et SVG | Contrôles exécutés sur les sources ; aucune réécriture de code par un formateur |
| Configuration Live et paquets | user-setup ajouté explicitement ; manifeste releases/.../packages.txt |
| Fabrication ISO et SHA-256 | logs/build-20261030T121627Z-1881.log et SHA256SUMS |
| Inspection SquashFS, identité et applications | logs/iso-inspection.txt : os-release, Plasma, Konsole, Dolphin, accueil et composants Live présents |
| Catalogue UEFI et BIOS | xorriso a confirmé deux entrées El Torito et les chargeurs correspondants |
| Bureau BIOS QEMU Windows | VGA standard, qemu64, 1 vCPU, 3 Gio, TCG : logs/resume-bios.png |
| Bureau UEFI QEMU Windows/OVMF Debian | Même profil VGA standard : logs/uefi-std-08.png |
| Bureau UEFI VirtualBox | VMSVGA, 1 vCPU, 3 Gio : logs/resume-vbox.png ; contrôles détaillés non réalisés dans cette VM |
| Utilisateur non root et session X11 | UID 1000 kyronext, groupes sans sudo et XDG_SESSION_TYPE=x11 dans les deux QEMU ; logs/bios-session-info.png et logs/uefi-session-checks.png |
| Identité et noyau | os-release Kyronext OS 0.1 Developer Preview ; noyau 6.12.113+deb13-amd64 ; logs/bios-system-checks.png |
| Terminal et clavier français | Commandes réellement saisies et exécutées dans Konsole |
| Réseau NAT BIOS | Interface connectée, DNS deb.debian.org résolu, ping passerelle 10.0.2.2 : 1 paquet reçu ; logs/bios-network-services.png |
| Services systemd | systemctl --failed : 0 unités au moment des contrôles BIOS et UEFI |
| Dolphin BIOS | Dossier personnel et média ISO visibles ; logs/bios-dolphin.png |
| Souris BIOS | Déplacement et ouverture du menu contextuel Dolphin ; logs/bios-native-mouse.png |
| Rapport matériel local | Fichier de rapport automatique observé sous ~/.local/state/kyronext/hardware/ |
| Arrêt BIOS | Après systemctl poweroff depuis le compte Live, QEMU PID3220 s'est terminé sans arrêt forcé de cette instance ; logs/bios-poweroff-result.json |
| Redémarrage UEFI | Après systemctl reboot depuis le compte Live, retour au menu GRUB dans le même processus QEMU PID4560 ; logs/uefi-reboot-check.png |
| Legacy Driver Manager indépendant | 2 tests sous Debian, collecte réelle, permissions 0600 et refus d'écrasement ; logs/legacy-review-validation.md |
| Guardian Hardware Passport indépendant | 3 tests sous Debian, collecte réelle, permissions 0600, refus écrasement et lien symbolique ; logs/guardian-review-validation.md |

## Échecs, incidents et limites observés

- Deux QEMU simultanés dans le constructeur VirtualBox ont provoqué des softlockups et une perte de SSH. Ce test imbriqué a échoué. Le constructeur a été récupéré, puis arrêté proprement après copie des artefacts. Trace : logs/builder-softlockup-during-parallel-boot.txt.
- Les premiers profils QEMU Windows avec carte VirtIO graphique ont présenté un écran noir persistant. Ils ne sont pas validés. Le profil VGA standard a ensuite affiché le bureau en BIOS et UEFI. Aucune cause unique n'est affirmée sans journaux suffisants, ni incompatibilité généralisée de VirtIO.
- Le démarrage UEFI QEMU VGA standard a présenté un affichage brouillé temporaire avant l'initialisation de bochs-drm. Le bureau et Konsole ont ensuite été observés. L'artefact initial reste à corriger.
- L'émulation TCG est lente sur cet hôte. Trois VM ouvertes ont réduit la RAM Windows disponible sous 1 Gio ; la VM de comparaison a été sauvegardée et la mémoire libérée. Ne pas prendre ces temps de démarrage comme mesure sur matériel réel ou KVM.
- Le fond du menu de démarrage conserve la présentation Debian/live-build. Les libellés du menu, l'identité système, le fond du bureau et l'accueil sont personnalisés Kyronext.
- loginctl show-session self a échoué depuis l'application lancée par le gestionnaire utilisateur ; printenv a confirmé x11 et loginctl list-sessions a bien listé la session utilisateur. Ce contrôle négatif est conservé dans logs/bios-session-info.png.

## Tests non exécutés et périmètre

Aucun ordinateur physique déclaré compatible. Audio réel, Wi-Fi physique, GPU physique, imprimantes/scanners, écriture USB, installation sur disque, Secure Boot, chiffrement, récupération et IA locale non testés ou absents. KVM et WHPX non testés : les essais QEMU réussis utilisent TCG directement sous Windows. Le second bureau après reboot n'a pas été attendu : le retour GRUB prouve ici le redémarrage.

Les fonctions Guardian/Legacy avancées restent des modules indépendants des sources ; le Live contient le diagnostic texte local. Aucun statut QMP running ni simple lancement d'émulateur n'a été retenu comme preuve de bureau.

## ISO `20261031T000112Z-1816` — vérification post-construction

| Vérification | Résultat |
|---|---|
| live-build Debian 13 | Réussi |
| Taille ISO | 1 754 726 400 octets |
| SHA-256 constructeur et copie Windows | Identique : `86b0fbbaeab6b3b62157b47d299ef5f9b05a702d319ac9a32975d51a75562bee` |
| Démarrage UEFI VirtualBox | KDE Live atteint ; écran Welcome Kyronext visible |
| Menu Kickoff | Nom Kyronext et avatar K visibles ; « Debian Live user » n'apparaît plus |
| `live-config.service` | Échec signalé pendant le démarrage ; la session graphique démarre malgré tout. Cause à diagnostiquer |
| sudo/password | Non disponible dans cet ISO ; configuration corrective ajoutée aux sources pour une prochaine reconstruction, pas encore validée |
| Capture d'écran | Capture par VirtualBox vérifiée ; Print Screen du HP non testé. La prochaine construction ajoutera kde-spectacle et un repli pour Kyronext Capture |
| Applications complètes | Non validées comme suite complète ; consulter `docs/OUTILS_MANQUANTS.md` |

Captures locales : `build/kyronext-new-iso-menu.png` et `build/kyronext-new-iso-desktop.png`. Cette ISO est une préversion Live; ne pas la considérer comme incluant le correctif sudo ajouté après sa compilation.

## Tentative d'écriture sur clé USB — 31 octobre 2026

La clé USB identifiée par le numéro de série `5297401173270427525` a été
passée à la VM Debian ; la source transférée a conservé le SHA-256
`1f3165e754d9afe3efebd96e1e3cdb4d911b22e83e12efe7f1e919ff0d4ce90c`.
L'écriture brute de 1 762 754 560 octets s'est terminée et `fsync` a rendu la
main. La comparaison de relecture a cependant donné
`96aa97741987c9ffcb525eef87e5decf5b26355f56ab68a4d0c1ab80c90ea986`, qui ne
correspond pas à la source. Le support n'est donc **pas validé** et ne doit pas
être considéré comme prêt à démarrer. À la demande de l'utilisateur, toute
opération sur la clé a été arrêtée et le périphérique détaché de la VM ; il
sera réécrit manuellement avec Rufus.
