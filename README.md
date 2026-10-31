# Kyronext OS — 0.1 Developer Preview

**Kyronext Systems — By Manix**
## Candidate Live actuelle — 31 octobre 2026

`releases/20261031T1854-suite-search-preview/Kyronext-OS-0.1-amd64.iso` — 2 454 208 512 octets, SHA-256 `631637ee963f8ed2e3660aa9e2cdef39ad53c397e6a12b02e37c5c31123704b4`. Cette candidate inclut le paquet Kyronext Suite Preview et Launchpad. QEMU/WHPX a affiché le bureau sous BIOS et UEFI ; Monitor a été ouvert dans la session UEFI. Aucun disque virtuel n'était attaché. C'est une ISO Live Developer Preview, pas un installateur. Voir `BUILD_REPORT.md` et `TEST_REPORT.md`.

La candidate précédente `releases/20261031T175731-clean-live/` est conservée intacte. La candidate actuelle n'intègre ni identifiants Live personnels ni mot de passe sudo par défaut.

L'ancienne candidate `releases/20261031T1542-legacy-fbdev/` reste privée : elle contient des accès temporaires et ne doit pas être distribuée.

Véritable distribution Live Debian 13 AMD64 : noyau Debian officiel, KDE Plasma/SDDM et X11, systemd, PipeWire et NetworkManager. ISO hybride UEFI/BIOS fabriquée avec Debian live-build.

L'image initiale a été produite et son SHA-256 vérifié. Le bureau a été observé sous QEMU Windows en BIOS et UEFI avec VGA standard, ainsi que dans VirtualBox UEFI. Konsole, Dolphin, clavier/souris, réseau NAT, arrêt et redémarrage ont fait l'objet de contrôles réels. La personnalisation du bureau, de l'accueil et des menus de démarrage a ensuite été observée dans l'image `20261030T143045Z-branding-final`. Voir TEST_REPORT.md pour le périmètre précis et les preuves.

La révision `20261030T151047Z-branding-806` inclut la Veille Kyronext et le contraste des icônes. L'ISO et son SHA-256 ont été vérifiés sous Windows, ainsi que 298 sommes internes et les structures hybrides BIOS/UEFI. Voir [la validation visuelle](docs/VALIDATION_VISUELLE.md) et [la personnalisation](docs/PERSONNALISATION.md).

**Live uniquement : aucun installateur actif, aucun modèle IA.** Aucun ordinateur physique déclaré compatible. QEMU WHPX BIOS et UEFI atteignent le bureau. Sur cette configuration Windows, QEMU TCG a produit un affichage brouillé ou noir et n'a pas atteint le bureau ; le script Windows utilise maintenant WHPX par défaut. Le HP EliteBook 2540p reste à tester physiquement.

- Candidate Live actuelle et empreinte : `releases/20261031T1854-suite-search-preview/`. L'ancienne candidate privée `20261031T1542-legacy-fbdev` contient des accès de test et ne doit pas être distribuée.
- BUILD.md : reconstruire dans Debian 13 isolé.
- `scripts/build-suite-package.sh` : reconstruire et tester la suite Qt optionnelle ; le paquet preview 2 est intégré à la candidate courante.
- TESTING.md : essayer l'ISO ; scripts Windows disponibles.
- TEST_REPORT.md et KNOWN_ISSUES.md : résultats et limites observés.
- HARDWARE_COMPATIBILITY.md : matrice, sans compatibilité physique supposée.
- docs/GUARDIAN.md : Hardware Passport indépendant et fonctions futures.
- docs/ROADMAP_0.2.md : étapes de la version suivante.
- LICENSES/ : licences et inventaire des firmwares.

Les sources sont séparées des travaux, journaux et artefacts. build/, logs/ et releases/ sont exclus de Git. Ne jamais publier les clés privées de construction.
