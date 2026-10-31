## Validation de l'ISO candidate 20261031T1542-legacy-fbdev — 31 octobre 2026

La suite `kyronext-suite-preview 0.1.0-0~preview2` est présente dans l'image (SHA-256 du .deb : `80a558d1704722d1e2441c005501038a70d37d1618960c19253b0525a7e0011f`). Kyronext Monitor a été lancé depuis le bureau Live UEFI standard : mesures CPU, mémoire et réseau visibles ; GPU/températures signalés comme indisponibles dans la VM.

L'ISO inclut les corrections graphiques testées en VirtualBox : démarrage standard BIOS et UEFI sous Plasma Wayland ; démarrage « Legacy Compatibility » BIOS et UEFI sous Plasma X11. En UEFI, le hook sélectionne `fbdev` uniquement lorsque `/dev/fb0` et son module Xorg existent ; le test a observé le bureau et les processus X11. En BIOS, X11 a fonctionné avec le repli Xorg par défaut. Captures dans `releases/20261031T1542-legacy-fbdev/validation/`.

Les tests unitaires complets de chaque application de la suite n'ont pas été refaits dans cette validation ISO. Les résultats historiques ci-dessous ne doivent pas être interprétés comme des tests de chaque application dans cette candidate. Le HP 2540p, les périphériques physiques, l’audio et l’installation sur disque restent non validés. QEMU UEFI/OVMF avec WHPX affiche le bureau Kyronext et confirme Plasma actif ; le profil TCG reste très lent et n’a pas donné de bureau visible pendant le test. QEMU BIOS reste non testé.

---

# Validation de la suite Kyronext - Developer Preview

ISO : releases/20261030T175828Z-branding-5824/Kyronext-OS-0.1-amd64.iso
SHA-256 : `5858cefbaab2b4c5ea2a80b7bdc2be7bc94ef04faa0665bc86c70f8d32dc58ce`.

Reussis : compilation Debian 13 Qt 6 ; tests de calcul et de sauvegarde ; tests Qt Paint (pixels, gomme, annulation et PNG), playlists AMP et decodage WAV ; 8 tests de configuration des liens ; copie ISO et somme SHA-256 ; controle des fichiers inclus.

VM VirtualBox UEFI : bureau et accueil observes, avec liens officiels ; ouverture reelle de Kyronext AMP et de Kyronext Paint. Le pinceau a produit des traits visibles. Captures : logs/suite-live.png, logs/suite-amp-open.png, logs/suite-paint-open.png.

Non verifies : ecoute audio physique, autres formats audio, equipements physiques sur cette revision, installateur et Secure Boot. L'egaliseur et les visualisations audio restent a developper. Le navigateur n'est pas inclus dans cette image ; les liens exigent un navigateur installe. `kironext.be` reste a confirmer, sans substitution.

La premiere ISO publique est une revision differente. Cette image est une version de travail, non publiee comme stable.

## Couche graphique du framework — 31 octobre 2026

Compilation C++ réelle sur le constructeur Debian avec Qt 6.8.2 : le framework
partagé et Kyronext Settings compilent. Le test automatisé `kyronext-theme`
réussit (1/1) avec le backend Qt offscreen ; il vérifie les états communs et
les palettes Rouge, Ambre et Contraste renforcé.

La palette Contraste renforcé est enregistrable dans Kyronext Settings et
incluse dans l'export/restauration des préférences. Cette couche n'a pas été
reconstruite dans l'ISO 0.1 : elle reste une évolution des sources et nécessite
une intégration/validation de paquet séparée avant d'être présentée comme
disponible dans l'image.

## Densité graphique — 31 octobre 2026

Kyronext Settings et le framework Qt ont été recompilés sur Debian 13 avec Qt
6.8.2. CTest `kyronext-theme` réussit (1/1) : il vérifie la préférence stockée
par utilisateur, le repli sur la configuration partagée, les hauteurs minimales
36/44 px et les menus compacts. L'export des préférences passe au schéma 2 ; le
code d'import du schéma 1 assigne une densité confortable. L'interface n'a pas été
testée visuellement sur le HP 2540p et cette version n'est pas intégrée à l'ISO.

## Kyronext Monitor — alimentation — 31 octobre 2026

Les sources mises à jour de Monitor, Files et du framework Qt ont été compilées
sur Debian 13/Qt 6.8.2. CTest réussit (1/1) avec un cas simulé vérifiant les
états `Mains` connecté et `USB_C` déconnecté, plus des estimations de 120/300
minutes par énergie/puissance et charge/courant synthétiques. Un état Charging
ne reçoit pas d'estimation de décharge. Des fixtures confirment le lien Ethernet
à 1 000 Mbit/s, le Wi-Fi sans lien, la passerelle IPv4 et les DNS IPv4/IPv6.
La collecte reste en lecture seule et n'émet aucune requête réseau.
Ni la batterie ni la prise secteur du HP EliteBook 2540p n'ont été mesurées :
cette fonction n'est pas encore intégrée à l'ISO 0.1.

## Consolidation des sources — 31 octobre 2026

Construction complète de `applications/suite` sur Debian 13/Qt 6.8.2 : les
137 étapes Ninja se terminent avec succès. CTest réussit **13/13**, y compris
Kyronext Update, les thèmes/densités, Monitor et les tests existants d'AMP,
Paint, Files, accessoires, organisateur et Navigator. Des avertissements de
dépréciation Qt concernant `QMenu::addAction` restent dans AMP et quelques
accessoires ; aucune erreur de compilation ne les accompagne.

`cmake --install` vers un `DESTDIR` temporaire installe le binaire, le raccourci
`.desktop` et l'icône SVG de Kyronext Update ; aucun paquet n'a été installé sur
l'hôte. Une simulation réelle `apt-get -s -o Debug::NoLocking=1 upgrade` dans la
VM a signalé une mise à niveau de `tzdata` sans modifier de paquet.

Kyronext Recovery a aussi passé ses quatre tests Debian (rapport synthétique,
masquage de valeurs courantes, création privée sans écrasement et refus des
liens symboliques), ainsi que la vérification d'installation CMake. La collecte
réelle de journaux n'a pas été exécutée. Ces composants restent des sources et
ne sont pas intégrés à l'ISO 0.1 ; aucun test matériel n'a été effectué sur le HP.

## Nouveaux outils — 31 octobre 2026

Kyronext Backup : l'agent a compilé et installé le module dans un répertoire
temporaire sur Debian 13/Qt 6.8.2 et exécuté ses cinq tests sans root : création
d'archive lisible avec empreinte, non-écrasement, refus d'une destination dans
la source, nettoyage après échec et annulation avant départ. Le CMake parent
ajoute maintenant le module par défaut via `KYRONEXT_BUILD_BACKUP=ON` ; la
reconstruction intégrée et l'installation de la suite complète restent à faire.
Aucun test de la suite intégrée n'est revendiqué ici. Aucune donnée utilisateur
ni clé USB n'a été utilisée.

Kyronext Disk Health : le CMake parent propose aussi le module par défaut via
`KYRONEXT_BUILD_DISK_HEALTH=ON`. Les tests de parsing SMART et de découverte
sysfs existent, mais n'ont pas de résultat validé. Une compilation complète de
la suite a été lancée sur la VM, mais celle-ci a cessé de répondre avant la fin
et la session SSH a expiré ; aucune réussite de compilation ou de test n'est
revendiquée pour Disk Health. Le code a depuis reçu des corrections qui
n'étaient pas dans cette tentative. `smartmontools` est optionnel, aucune
élévation de privilèges n'est demandée, et aucun disque physique n'a été
interrogé. Ces outils ne sont pas inclus dans l'ISO 0.1.

Le framework Qt propose aussi un panneau d'état vide accessible, utilisé dans
AMP et Gallery ; un test vérifie son texte et son nom accessible. Settings
exporte désormais un réglage de réduction des visualisations d'AMP au schéma 3,
avec maintien des imports schémas 1 et 2. La préférence désactive aussi la
minuterie de la veille Kyronext ; ses tests Python réussissent 4/4.
Le test Python de préférence et trajectoire est passé 4/4. La VM est restée
indisponible avant la recompilation des changements Qt ; les tests du panneau
vide et des préférences Qt n'ont donc pas de résultat. Les animations Plasma
restent séparées ; la veille Kyronext est bien contrôlée par cette préférence.


## Gestionnaire de pilotes — contrôles locaux, non intégrés à l'ISO

Le 31 octobre 2026, les tests Python du gestionnaire réussissent 17/17
(`py -3 -m unittest tests.test_driver_manager -v`) et ceux des budgets 6/6
(`py -3 -m unittest tests.test_release_budgets -v`). Les contrôles statiques
(`py -3 tests/check_sources.py`) réussissent. Ils couvrent l'intégrité SHA-256
de fixtures, les refus de chemins non sûrs, les snapshots qui ne sont pas des
fichiers ordinaires ou des liens symboliques, les scénarios Wi-Fi simulés, la fermeture des dépendances
et les associations de firmware de quelques anciennes clés USB. Ils
n'installent aucun paquet, n'utilisent ni APT ni périphérique physique, et ne
prouvent aucune compatibilité matérielle.

Les aperçus Qt de `logs/driver-center-preview/` utilisent des données simulées, en 1080×740 et 800×600. L'accès au constructeur Debian est rétabli. Le gestionnaire, sa réserve de paquets et les changements récents de la suite restent absents de l'ISO validée ci-dessus ; leur intégration et une nouvelle construction live-build restent à valider. Aucun test de la nouvelle suite complète, de Polkit, du Wi-Fi physique, du BIOS/UEFI ou du HP EliteBook 2540p n'est revendiqué ici.

## Tests Python locaux — 31 octobre 2026

Sous Windows, `py -3 -m unittest discover -s tests -v` réussit 34 cas avec un
test Linux/sysfs explicitement ignoré. Les modules Firefox, Guardian, liens,
veille et utilitaires réussissent 37 cas, dont quatre tests dépendant des
permissions Unix ou de verrous POSIX sont ignorés sur Windows. La découverte
documentée de Recovery (`py -3 -m unittest discover -s applications/recovery
-v`) réussit quatre cas ; son essai de lien symbolique est ignoré car Windows
n'a pas accordé le privilège requis. Aucun échec dans ces trois commandes.

Le test de chemin USB utilise le nom Linux `1-1:1.0`; il est marqué Linux uniquement. Le test Qt du thème redirige ses préférences utilisateur et système vers un répertoire temporaire, y compris lors du nettoyage après chaque cas. Ce test partagé a été compilé et validé dans le rebuild complet de la suite. CTest configure également les cas Backup sautés sous root comme ignorés ; ce comportement particulier n'a pas été exécuté sous root dans cette session.


## Suite Qt intégrée — Debian 13 / 31 octobre 2026

Les sources courantes de `applications/suite` ont été compilées dans un staging isolé : CMake/Ninja a terminé **166/166 étapes**. CTest réussit **16/16** (19,97 s) : Update, Backup, Disk Health, AMP (playlist/décodage, DSP, interface), Monitor (métriques/fichiers et capture hors écran), Gallery/Video/Capture, Archive/Calendar/Clock, Navigator (politique et moteur local), thème et Paint. Le test de thème s'exécute avec un répertoire QSettings temporaire.

`cmake --install` a écrit dans un `DESTDIR` sous `build/`, sans installation sur l'hôte. Le paquet `kyronext-suite-preview_0.1.0-0~preview2_amd64.deb` a été construit et inspecté ; son SHA-256 est `80a558d1704722d1e2441c005501038a70d37d1618960c19253b0525a7e0011f`. La dépendance système `tar`, utilisée par Backup, est déclarée. Une simulation `apt-get -s install ./paquet.deb` ne prévoit que l'installation de ce paquet et ne modifie rien.

Le paquet n'est pas encore dans l'ISO privée existante. Le prochain contrôle est une nouvelle construction live-build, puis des démarrages QEMU séparés UEFI et BIOS. Les avertissements de dépréciation `QMenu::addAction` de Qt dans AMP et des accessoires restent à migrer ; ils n'ont pas bloqué la compilation.

## Refonte Kyronext Monitor — validation Qt isolée — 31 octobre 2026

Les sources actuelles de Monitor, des métriques, du framework Qt et du test de capture ont été synchronisées dans un staging Debian ; leurs empreintes ont été comparées avant le dernier passage. Configuration et compilation réussies sous Debian 13 / Qt 6.8.2. CTest réussit **2/2** : métriques/fichiers et rendu Monitor hors écran (5,52 s, dont 5,01 s pour la fenêtre). Le test attend les mesures réelles, capture la vue d'ensemble et aligne les deux panneaux de courbes pour produire une seconde image distincte.

Aperçus : `logs/kyronext-monitor-preview.png` et `logs/kyronext-monitor-preview-graphs.png`. Le premier affiche l'état courant ; le second montre les courbes CPU/RAM et réseau. Ce rendu Qt hors écran ne valide pas encore le lancement en session KDE Live ni la consommation sur un PC ancien. La refonte n'est pas intégrée à l'ISO privée déjà produite. L'utilisation et la température GPU restent indisponibles et ne sont pas simulées.

La suite C++ complète avec Backup, Disk Health, les préférences et Navigator doit encore être recompilée et testée ensemble avant de produire un nouveau paquet ou une nouvelle ISO.


## ISO privée 20261031T145700Z-legacy-compat — validation réelle du bureau — 31 octobre 2026

La suite Qt complète est intégrée au paquet preview présent dans cette image. Le paquet a été construit sous Debian 13 en 166 étapes, CTest 16/16, puis lancé depuis le Live sans installation sur disque. Kyronext Monitor a été ouvert graphiquement en sessions UEFI et BIOS ; les captures conservent CPU/RAM/réseau en direct et le statut GPU non mesuré : `releases/20261031T145700Z-legacy-compat/validation/monitor-uefi.png` et `monitor-bios.png`.

Le Live standard a atteint le bureau Welcome en UEFI et BIOS VirtualBox. Le profil Legacy est distinct : `nomodeset kyronext.legacy=1` et le choix Plasma X11 sont confirmés, mais Xorg échoue sous l'adaptateur VMSVGA fourni par VirtualBox (pilote `vmware_drv`, mappage framebuffer impossible, segfault). Ce résultat est consigné comme échec du profil virtuel et ne prouve ni compatibilité ni incompatibilité du HP physique. VBoxVGA a aussi produit un affichage noir. La VM a été arrêtée et restaurée à EFI64/VMSVGA.

SHA-256 ISO : `fa1179d315c8f8da4faba5755e855a9d572e5b45914aa7fb51b047a842e0a55d` ; 2 451 800 064 octets. Une VM sans disque virtuel a été utilisée. QEMU, installation/Calamares, HP 2540p, Wi-Fi/audio/GPU physiques et USB n'ont pas été testés sur cette candidate. Elle reste privée à cause de la clé publique SSH et de l'accès Live temporaire.
