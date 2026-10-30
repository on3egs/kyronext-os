# Architecture et feuille de route

Kyronext OS 0.1 Developer Preview — Kyronext Systems — By Manix.

Base : Debian 13 Trixie AMD64, noyau officiel, KDE/SDDM et X11, systemd, PipeWire, NetworkManager. Sources live-build dans iso/, travaux dans build/, preuves dans logs/, artefacts dans releases/.

## État réel

A : audit Windows et constructeur Debian isolé réalisés. B : système Live Debian KDE construit. C : identité, fond original et accueil intégrés et observés. D : ISO et SHA-256 produits, copie vérifiée sous Windows. E : bureau observé en BIOS et UEFI QEMU avec VGA standard, et en UEFI VirtualBox ; contrôles détaillés dans TEST_REPORT.md. F : documentation et livraison locale dans releases/.

Aucun installateur actif, aucun modèle IA, aucun pilote Windows ou ancien installateur NVIDIA. Le Live emploie le noyau, les pilotes et les firmwares Debian officiels. Les licences et les 38 paquets de firmware/microcode sont inventoriés dans LICENSES/FIRMWARE.md.

## Modules indépendants

Le Legacy Driver Manager collecte les identifiants, pilotes et modules candidats sans modifier la machine. Guardian Hardware Passport produit un rapport JSON hors ligne non privilégié avec statuts non vérifiés explicites. Dans l'ISO, le diagnostic texte local est sauvegardé automatiquement. Voir docs/ARCHITECTURE.md et docs/GUARDIAN.md.

Adaptive OS proposera des profils explicables et un choix manuel. Phoenix Recovery commencera par un environnement de dépannage avant la restauration des mises à jour. AI Doctor restera local, facultatif, sans rôle administrateur permanent et avec confirmation explicite des actions sensibles. Ces fonctions ne sont pas annoncées opérationnelles en 0.1.

## Version suivante

Voir docs/ROADMAP_0.2.md. Priorité : défauts graphiques observés, cohérence du démarrage et du panneau, profils de compatibilité testés puis intégration empaquetée du passeport. Calamares ne sera expérimenté qu'après validation de la base et exclusivement sur disques virtuels jetables.
