# Kyronext Monitor et Files — 0.1.0

Applications originales C++17/Qt 6 utilisant KyronextUi, sans privilèges permanents.
Les outils KDE existants restent disponibles. Ces exécutables ne changent ni les
associations MIME ni le gestionnaire de fichiers par défaut.

## Monitor

Les valeurs viennent de `/proc/stat`, `/proc/meminfo`, `/proc/net/dev`, des fichiers
`/proc/PID/stat`, de `/sys/class/thermal` et de QStorageInfo. Le premier relevé CPU
et réseau affiche « en attente » ; les suivants utilisent les différences réelles
et le temps écoulé. Le réseau exclut la boucle locale. Les compteurs qui ont
reculé ne produisent pas de débit négatif inventé. Les processus non lisibles ou
disparus sont ignorés. Les températures absentes sont signalées.

La liste est filtrable, triable et actualisée toutes les deux secondes lorsque
l'onglet Processus est ouvert ; le scan des processus est suspendu dans les
autres vues pour réduire le travail en arrière-plan sur les anciennes machines.
Le CPU d'un processus est exprimé en pourcentage de **toute la machine**, sans
multiplier par le nombre de cœurs. La vue légère masque les graphiques. Les
courbes CPU/RAM et réseau conservent chacune jusqu'à 120 relevés. Les volumes
montés affichent capacité et espace disponible ;
le débit d'E/S disque et l'utilisation GPU ne sont pas implémentés.

L'arrêt nécessite une sélection et une confirmation. SIGTERM est envoyé via un
descripteur pidfd, afin qu'une réutilisation de PID entre la vérification et le
signal ne cible pas un autre processus. Si le noyau ne fournit pas pidfd, l'action
est refusée. Seul SIGTERM est envoyé,
aux processus appartenant à l'utilisateur courant. PID 1, Monitor lui-même,
identités réutilisées et exécution en root sont refusés. L'application ne demande
aucune élévation et ne fournit pas de bouton SIGKILL. Un processus peut ignorer
SIGTERM ou avoir des documents non enregistrés ; la confirmation le rappelle.

### Refonte du tableau de bord — validation Qt isolée

La fenêtre est organisée en trois vues :

- **Vue d'ensemble** : cartes CPU, mémoire vive, réseau et état GPU explicite,
  courbe CPU/RAM et courbe de débit reçu/envoyé. Les courbes retiennent au plus
  120 relevés, se mettent à jour sans animation et affichent les données manquantes
  comme indisponibles au lieu de créer des valeurs.
- **Matériel et réseau** : batterie et autonomie indicative, alimentation,
  capteurs thermiques accessibles, interfaces/adresses, passerelle/DNS exposés,
  architecture/noyau, mémoire swap et volumes montés.
- **Processus** : filtre nom/PID, tri conservé pendant les relevés, CPU rapporté
  à toute la machine, mémoire et demande SIGTERM confirmée.

La vue légère masque les graphes. Le tableau de bord reste local et en lecture
seule, sauf la demande d'arrêt de processus déjà protégée et limitée à la session.
L'identité PCI des contrôleurs graphiques et le pilote lié sont affichés si sysfs
les expose. L'absence de contrôleur PCI signalé n'est pas une preuve d'absence
de matériel graphique. L'utilisation et la température GPU, le débit d'E/S disque
et les profils d'énergie ne sont pas encore mesurés.
La refonte actuelle a été compilée sous Debian 13 avec Qt 6.8.2. CTest réussit
2/2 cas : métriques/fichiers et rendu Monitor hors écran. La capture graphique
attend plusieurs relevés, puis vérifie que les deux panneaux et courbes sont
visibles dans une seconde capture distincte. Aperçus :
`logs/kyronext-monitor-preview.png` et
`logs/kyronext-monitor-preview-graphs.png`. Le rendu Qt hors écran ne remplace
pas un essai dans la session KDE Live. Cette refonte n'est pas intégrée à l'ISO
actuelle ; aucune mesure d'utilisation GPU ou de charge sur matériel ancien n'est
revendiquée.

## Files

Onglets, adresse locale, dossier parent/accueil, vues liste/grille/détails, filtre
des noms du dossier, favoris persistants, fichiers cachés et volumes **déjà montés**.
Le double-clic sur un fichier utilise l'application choisie par le système.
Copier, couper, coller, copier/déplacer vers, renommer et corbeille utilisent KDE
KIO de manière asynchrone. Ctrl+C/X/V et Ctrl+T/W sont disponibles.

Cette première version opère sur **un seul élément sélectionné**. Une destination
existante est refusée : aucun écrasement ou fusion automatique. Copier un dossier
dans lui-même est refusé. La corbeille exige confirmation et ne bascule jamais
vers une suppression définitive lorsque KIO ne peut pas mettre l'élément à la
corbeille. La création d'un dossier respecte les permissions ordinaires.
Les propriétés affichent taille, propriétaire et droits effectifs en lecture seule.

Glisser-déposer, opérations groupées, aperçus intégrés, retour historique,
modification des permissions et montage de volumes restent à développer.
Le filtre n'est pas une recherche récursive ni une indexation des dossiers privés.

## Dépendances et compilation

Paquets officiels Debian 13 vérifiés : `libkf6kio-dev` pour construire et `kio6`
pour les workers locaux à l'exécution, en plus de Qt 6 Widgets. Aucun besoin direct
de Solid pour cette version. Les bibliothèques gardent leurs licences propres.
KIO conserve ses protections et son traitement des erreurs ; aucun moteur de
copie ou de corbeille propriétaire n'est réinventé.

```bash
cmake -S applications/suite/system -B build/system-suite -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build build/system-suite --parallel 1
QT_QPA_PLATFORM=offscreen dbus-run-session -- \
  ctest --test-dir build/system-suite --output-on-failure
```

Les tests vérifient les compteurs CPU (dont le double comptage guest évité),
RAM/swap/réseau, noms de processus avec parenthèses, PID réutilisé et SIGTERM
sur un enfant de test, ainsi que de vraies copies et déplacements KIO dans un
répertoire temporaire. L'écrasement et la copie récursive dans soi-même sont
refusés. Les tests n'utilisent aucun disque physique et ne modifient aucun
fichier personnel. Le fonctionnement graphique et la corbeille restent à vérifier
dans la session Live ; aucune compatibilité physique n'est revendiquée ici.
# Kyronext Monitor

Le panneau de détails lit les batteries présentes dans `/sys/class/power_supply`.
Il affiche l'état, le niveau, et lorsque le noyau expose les valeurs correspondantes,
une estimation de santé (`energy_full` ou `charge_full` comparée à la valeur
design) et le nombre de cycles. Les champs absents restent absents ; l'estimation
ne remplace pas un test de capacité ni un diagnostic constructeur. Cette fonction
est en lecture seule et n'écrit pas dans les réglages d'alimentation.

Monitor signale également les sources `Mains`, `USB` et `USB_C` lorsque
`/sys/class/power_supply` fournit un état `online` valide. Une absence de donnée
est indiquée comme indisponible ; le programme ne déduit pas la présence du
secteur à partir du niveau de batterie et ne modifie aucun réglage matériel.

Pendant une décharge, Monitor peut calculer une durée indicative à partir de
`energy_now/power_now` ou, à défaut, `charge_now/current_now`. L'estimation est
affichée seulement si les deux mesures sont valides et cohérentes (au plus
7 jours) ; elle varie avec la charge et ne remplace pas l'estimation du firmware.

Monitor lit aussi l'état de lien exposé dans `/sys/class/net`, les adresses
locales des interfaces actives, la route IPv4 par défaut et les serveurs DNS
déclarés localement lorsque disponibles. Ces données ne prouvent pas l'accès à
Internet : l'application n'envoie aucun paquet de test et ne scanne aucun réseau.
