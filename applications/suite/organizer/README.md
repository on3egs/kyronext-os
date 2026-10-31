# Outils Organizer — version 0.1.0

Trois applications C++/Qt 6 indépendantes utilisent KyronextUi et ses fenêtres
À propos. Aucun daemon, compte en ligne ou accès administrateur.

## Kyronext Archive

Ouvrir et lister un ZIP ou un TAR non compressé ; créer une archive à partir
d'un dossier explicitement choisi ; extraire dans un **nouveau** dossier sous
le dossier parent choisi. Un fichier ou dossier existant n'est jamais remplacé.
La création garde les sources intactes et refuse une destination située dedans.

Le moteur est KDE KArchive maintenu. Une vérification préalable des métadonnées
refuse chemins absolus, segments `..`, chemins Windows, liens symboliques/liens
TAR et fichiers spéciaux. ZIP64, archives multi-volumes et extensions TAR GNU/PAX
ne sont pas pris en charge dans cette première version. Certains TAR valides
seront donc refusés : cette limite est affichée dans l'interface.

Limites : 64 Mio par archive, 64 Mio par fichier interne, 256 Mio décompressés,
10 000 entrées, 64 niveaux. La lecture décompressée vérifie aussi les octets
réellement produits. Les données sont écrites dans un dossier temporaire privé,
avec droits utilisateur restreints ; un renommage Linux atomique sans écrasement
publie le résultat seulement après succès. Aucune fonction n'utilise l'extraction
aveugle de chemins fournis par l'archive. Les sources de création liées et noms
de plus de 100 octets sont refusés pour conserver un TAR simple et vérifiable.

L'extraction et la création sont synchrones et peuvent momentanément occuper
l'interface sur des fichiers proches des limites. Extraction en arrière-plan,
archives chiffrées, compression TAR et édition du contenu restent à développer.
Cet outil n'est pas une garantie absolue contre tout défaut du moteur tiers.

## Kyronext Calendar

Calendrier mensuel, événements d'un jour, création/modification/suppression avec
date, heure locale, titre et notes. Les jours contenant des événements sont mis
en évidence. Les données restent dans le répertoire utilisateur Qt/XDG, fichier
`events.json`, avec écriture atomique et permissions de fichier réservées au
propriétaire. Le format est versionné ; maximum 1000 événements et 1 Mio.

Une modification externe du fichier est refusée à la sauvegarde ; le bouton
Recharger permet de relire les données. Un JSON invalide bloque les modifications
pour préserver le contenu existant. Il n'y a pas de synchronisation cloud.
Les rappels sont visuels et fonctionnent seulement lorsque l'application reste
ouverte. Au lancement, les événements survenus pendant la dernière minute peuvent
être rappelés ; les événements passés plus anciens ne le sont pas. Récurrence,
partage, pièces jointes et récupération après une fermeture restent à développer.

## Kyronext Clock

Heure et date locales, chronomètre démarrer/pause/reprendre/remise à zéro,
minuteur en secondes de 1 seconde à 24 heures, et jusqu'à 16 alarmes de session.
Les durées utilisent QElapsedTimer, indépendamment des ajustements de l'horloge
civile. Une alarme dont l'heure est déjà passée est programmée pour le lendemain.
Les alertes sont des fenêtres visuelles ; aucun son personnalisé n'est livré.
Les alarmes ne sont pas persistées et cessent à la fermeture de l'application.
Aucun réveil d'ordinateur ni service permanent n'est configuré.

## Construction et validation

Paquet officiel Debian 13 : `libkf6archive-dev` pour construire, en plus de Qt 6
Widgets/Test. La dépendance d'exécution KF6Archive doit figurer dans le paquet
final. Les licences de Qt et de KArchive restent celles de ces composants.

```bash
cmake -S applications/suite/organizer -B build/organizer-suite -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build build/organizer-suite --parallel 1
QT_QPA_PLATFORM=offscreen ctest --test-dir build/organizer-suite --output-on-failure
```

Tests : ZIP/TAR temporaires, sous-dossiers, refus de traversée/liens/tailles et
écrasement, calendrier UTF-8 et changement externe. Les contrôles Qt de Clock
actionnent vraiment le chronomètre et déclenchent la fenêtre de fin d'un minuteur.
Les tests ne valident pas la réactivité sur vieux PC, les dialogues de sélection
interactifs ni la notification sur tous les bureaux. Essais Live à réaliser
avant d'intégrer ces applications à une version stable.
