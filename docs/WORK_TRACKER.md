# État réel de la suite et des extensions Kyronext

Mis à jour le 31 octobre 2026. « Présent dans le dépôt » ne signifie pas
« intégré à l'ISO ». Les tests listés doivent rester liés à leur véritable
environnement d'exécution.

| Domaine | État vérifié | À faire avant annonce stable |
|---|---|---|
| ISO Live KDE | Candidate la plus récente `20261031T1854-suite-search-preview` ; KDE/Welcome vus sous QEMU WHPX en BIOS et UEFI ; suite preview et Launchpad intégrés ; aucun accès SSH/mot de passe intégré | Tester le HP EliteBook 2540p et ses périphériques ; terminer la matrice matérielle et valider l'usage interactif des applications |
| Installateur | Profil source séparé prêt : Calamares graphique avec diaporama, Debian Installer en mode texte au démarrage, lanceur Live limité à Calamares | Compilation, démarrage BIOS/UEFI, installation, redémarrage et retrait des droits à valider seulement avec des disques virtuels jetables |
| Identité bureau | Menu, bureau/Welcome et boîte d'arrêt affichant Kyronext OS observés dans la candidate courante BIOS/UEFI QEMU ; aucun logo Debian dans la confirmation d'arrêt | Vérifier le matériel physique et revalider les changements futurs |
| Qt Framework | Bibliothèque C++/Qt 6, thème global, préférences, À propos, aide F1 et journaux compilés | Accessibilité complète, traductions, notifications bureau et QML |
| Notes | Éditeur UTF-8 avec recherche/remplacement, onglets, lignes, récupération et tests GUI | Essais interactifs dans l'ISO, encodages atypiques et usage prolongé |
| Calculator | Standard, scientifique, programmeur, conversions, historique, thèmes, tests de calcul | Relecture des unités et essai Live multi-clavier |
| Paint | Pinceau, gomme, formes, remplissage, pipette, texte, sélection, recadrage, transformations, undo/redo, PNG/JPEG/BMP | Essais nouveaux outils sur image réelle dans la nouvelle ISO ; fonctions avancées restent planifiées |
| AMP | Lecture, playlists, métadonnées, égaliseur DSP 10 bandes testé sur signaux synthétiques, visualisations audio réelles | Écoute physique et validation de plusieurs codecs/sorties audio |
| Monitor | Refonte responsive dans le paquet preview ; rendu et graphiques testés hors écran ; lancé visuellement dans la candidate la plus récente sous QEMU UEFI, avec métriques CPU/RAM/réseau/graphique et courbes visibles | Vérifier capteurs GPU/température disponibles et tester sur matériel réel ; la capture QEMU ne certifie pas les capteurs physiques |
| Files | KIO, opérations locales et volumes montés avec tests | Support Wayland/X11, périphériques variés et usage Live |
| Disk Health | Inclus dans le paquet preview et l’ISO candidate ; interface Qt en lecture seule, tests intégrés passent | Lancement interactif ISO et test uniquement sur disques virtuels jetables ; `smartmontools` reste optionnel |
| Gallery / Video / Capture | Applications Qt compilées ; manipulations image testées ; capture dépend des permissions du compositeur | Lecture média et capture réelle dans KDE Live, limites X11/Wayland |
| Archive / Calendar / Clock | Moteur KArchive, événements locaux JSON, minuteur/chronomètre ; tests Qt passés | Essai interactif et limites de formats/documentation |
| Settings | Thème, profils graphique léger/confortable, choix manuel des animations conservé, export/restauration JSON ; compilé et inclus dans le paquet preview | Test interactif dans KDE et vérification dans une nouvelle ISO |
| Firefox ESR | Paquet Debian officiel, profil facultatif séparé, palette rouge/noire, icône et lanceur Kyronext ; 9 tests passent sous Debian ; démarrage UEFI et ouverture de l'URL locale observés | Kyronext Start demeure vide dans Firefox ; diagnostic du rendu local requis avant toute validation ; suivi ESR |
| Navigator | Qt WebEngine, protections de permissions/TLS/profil privé ; tests de politique et moteur local passés ; livré en option dans le paquet preview ; n'est pas par défaut | Valider son lancement graphique dans le Live ; moteur sans suivi de sécurité Debian, donc pas pour une diffusion stable ou sensible |
| Welcome / liens | Configuration centralisée, liens contrôlés/masquables, accueil hors ligne ; KITT Franco-Belge redirige en HTTPS vers le domaine apex (HTTP 200), kyronext.eu répond en HTTPS (HTTP 200), curl valide les certificats | kironext.be ne résout pas en DNS ; domaine laissé inchangé. Tester les clics depuis Welcome/Navigator ; aucune substitution automatique |
| Launchpad | Catalogue `.desktop`, catégories, favoris, liens et épinglage ; recherche facultative bornée, annulable et limitée aux dossiers choisis ; intégré à la candidate la plus récente, smoke UI/recherche réussi dans le chroot Debian | Tester dialogues, ouverture MIME et accessibilité par interaction dans le Live |
| Help | Centre local français, F1 disponible dans les apps natives | Étendre la documentation et confirmer tous les raccourcis dans l'image |
| Hardware Passport / Connect | Diagnostic matériel en lecture seule et inventaire réseau local sans scan | Vérifier rapport complet dans Live et tester plusieurs équipements ; aucune compatibilité générale affirmée |
| Guardian / Adaptive / Phoenix / AI Doctor | Plan et primitives de diagnostic en lecture seule | Profils adaptatifs, récupération et assistant IA non implémentés |
| IA locale | Architecture prévue, sans modèle lourd ni privilège admin permanent | Moteur local facultatif, modèles, voix, permissions et tests restent à développer |
| Launchpad Search / Universal Search | Recherche de noms de fichiers opt-in, sans parcours au démarrage, limitée en durée, profondeur, fichiers et dossiers ; annulation, progression et volumes imbriqués gérés ; backend testé sous Debian 13 et Windows, smoke en chroot ; présente dans la candidate récente | Recherche de paramètres/contenu et index persistant local facultatif restent à développer ; valider l'expérience interactive complète |
| Media Center / Smart Open / Connect | Pas encore livrés comme expériences complètes ; KDE conserve les associations MIME | Catalogue média choisi, ouverture avec application préférée et appairage réseau à développer |
| Backup | Inclus dans le paquet preview et l’ISO candidate ; tests intégrés passent | Validation interactive dans le Live ; chiffrement et restauration restent à développer |
| Terminal / Software / First Run / Personalization | KDE et Debian restent disponibles ; interfaces Kyronext prévues | Applications propres et flux sûrs restent à implémenter |
| Navigator sécurité | Pas navigateur par défaut ; aucune désactivation TLS/sandbox | Audits et mises à jour continue nécessaires ; tests locaux seuls ne suffisent pas |

Les tâches ouvertes ne sont pas annoncées comme terminées. Aucun installateur
Calamares, chiffrement, Secure Boot, restauration Phoenix ou IA embarquée n'est
présenté comme opérationnel dans la version 0.1.

## Validation de la candidate 20261031T145700Z-legacy-compat

1. `live-build` a produit l’ISO Debian 13 AMD64 hybride ; la copie Windows a la même empreinte SHA-256. La suite preview (Backup, Disk Health, Monitor et autres applications) est installée dans cette candidate.
2. Bureau Kyronext/Welcome observé en UEFI et BIOS VirtualBox. Monitor a été lancé depuis la session Live dans les deux modes. Les captures se trouvent dans `releases/20261031T145700Z-legacy-compat/validation/`.
3. L’entrée Legacy transmet `nomodeset` et sélectionne Plasma X11, mais Xorg VMware échoue sur l’adaptateur virtuel VMSVGA ; cette option reste à corriger. Aucun résultat VirtualBox ne certifie le HP EliteBook 2540p.
4. La VM était sans disque virtuel. Aucun disque hôte/physique ni clé USB n’a été écrit. Après essai BIOS, la VM de test est éteinte et revenue à EFI64/VMSVGA.
5. La candidate reste privée, car elle contient une clé publique SSH du propriétaire et un accès Live temporaire. QEMU, HP physique, réseau/audio matériels, Calamares et installation disque restent à valider.

## Créations encore au programme

L'installateur Calamares séparé reste la priorité produit après validation du
Live et ne sera testé que sur des disques virtuels jetables. Restent également
les profils d'énergie, l'éjection sûre des supports, Print & Scan, la restauration
et le chiffrement de Backup, Phoenix Recovery, Adaptive OS, l'IA locale et ses
permissions/voix, First Run, Universal Search, Media Center et les parcours
complets de Settings/Personalization. Le Navigator demeure expérimental ; Firefox
est conservé comme solution de secours. Ces chantiers ne sont pas oubliés et ne
sont pas annoncés comme terminés.

## Candidate propre ISO 20261031T175731-clean-live — 31 octobre 2026

- [x] Construire l'image Debian Live KDE AMD64 dans le constructeur Debian isolé.
- [x] Vérifier BIOS/UEFI hybrides avec xorriso et contrôler la taille maximale.
- [x] Vérifier le transfert par SHA-256 ; empreinte `ad67f47ede2170f5a9fcc2b029b0d139cca5b8d8282da380f942c913927ccadd`.
- [x] Confirmer l'absence du hash de mot de passe Live et de la clé SSH propriétaire dans le squashfs.
- [x] Observer le bureau Kyronext dans QEMU WHPX en UEFI et BIOS ; conserver les captures.
- [x] Reproduire le défaut TCG : écran brouillé en VGA standard et noir avec VirtIO/Legacy après démarrage lent.
- [ ] Tester le HP EliteBook 2540p, son Wi-Fi, l'audio, la capture et les autres périphériques physiques.
- [ ] Tester une installation seulement après ajout et validation d'un installateur sur un disque virtuel jetable.

Rapport et captures : `TEST_REPORT.md` et `releases/20261031T175731-clean-live/`.

## Candidate privée antérieure 20261031T1542-legacy-fbdev — 31 octobre 2026

- [x] Reconstruire la couche binaire live après ajout conditionnel de Xorg fbdev dans le répertoire de construction temporaire Debian.
- [x] Vérifier l'empreinte du transfert Debian/Windows et la limite de taille de 16 Go.
- [x] Observer les bureaux Live VirtualBox en BIOS et UEFI, standard et Legacy.
- [x] Lancer Kyronext Monitor en UEFI standard ; lectures CPU/RAM/réseau de la VM, GPU/températures non mesurés.
- [x] Valider le bureau UEFI QEMU sous WHPX et inspecter SDDM/Plasma via un port SSH local.
- [x] Reproduire TCG single-thread ; la session graphique reste incomplète ou invisible dans le délai testé.
- [ ] Tester QEMU BIOS.
- [ ] Produire et valider une ISO sans clé SSH ni hash de mot de passe.
- [ ] Valider le matériel réel, dont HP EliteBook 2540p, Wi-Fi, audio, GPU et capture d'écran.
- [ ] Retirer la clé SSH et l'accès Live privés, reconstruire et retester avant toute distribution.
- [ ] Continuer la validation séparée des applications nouvelles ; ne pas présenter comme intégrées à l'ISO celles qui n'y sont pas.

Résultats détaillés et captures : `TEST_REPORT.md` et `releases/20261031T1542-legacy-fbdev/validation/`.
