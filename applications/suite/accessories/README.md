# Gallery, Video et Capture

Applications C++17/Qt 6 indépendantes, thème et À propos communs KyronextUi.
Chaque application possède aussi une aide locale accessible par F1.
Kyronext Systems — By Manix. Version 0.1.0. Aucun changement de navigateur ou
d'association MIME par défaut n'est effectué ; les `.desktop` proposent les
applications dans le menu et les choix d'ouverture habituels.

## Gallery

Ouvrir une image ou choisir explicitement un dossier. Le parcours concerne
seulement ce dossier, sans récursion, fichiers cachés ni indexation globale.
PNG, JPEG et BMP ; images bornées à 16 millions de pixels, allocation du lecteur
Qt limitée à 128 Mio. Le dossier est limité à 10 000 entrées d'images.

Précédente/suivante, rotation gauche/droite de 90° et diaporama facultatif à
4 secondes. La rotation transforme réellement les pixels en mémoire ; Enregistrer
propose un fichier et confirme l'écrasement par le dialogue standard. Changer
d'image ou fermer demande quoi faire d'une rotation non enregistrée. Le diaporama
n'avance pas pendant qu'une image est modifiée. La sauvegarde est atomique ; le
JPEG aplatit la transparence sur blanc. Le classement et l'édition des métadonnées
restent à développer.

## Video

Ouvrir un fichier local puis utiliser lecture/pause, stop, position et volume.
Espace bascule lecture/pause ; F11 bascule plein écran et Échap le quitte.
Les sélecteurs de pistes audio et de sous-titres intégrés sont alimentés par les
vraies pistes annoncées par Qt Multimedia. Les sous-titres reçus du QVideoSink
sont affichés en texte brut, sans interpréter du HTML fourni par le média.

Les formats et le décodage dépendent du backend/codecs Debian installé. Les
sous-titres externes et la lecture de flux réseau ne sont pas implémentés.
L'application ouvre seulement des fichiers locaux. Il faut notamment les
bibliothèques runtime `libqt6multimedia6` et `libqt6multimediawidgets6`.
La compilation ne prouve pas que chaque format, sortie sonore ou sous-titre est
correct sur chaque machine : ces essais doivent être réalisés dans le Live.

## Capture

Une capture démarre uniquement par un clic. Choisir l'écran, puis capturer tout
l'écran ou dessiner une zone ; Échap annule la sélection. La fenêtre se masque
350 ms pour éviter de se capturer elle-même. L'image reste en mémoire jusqu'au
choix explicite Enregistrer PNG. L'annulation ne crée aucun fichier.

Le premier backend prend en charge **X11 uniquement**, par QScreen. Sous Wayland,
offscreen et autres plateformes, une erreur explique la limite et invite à
utiliser un outil du bureau compatible avec son portail. Aucun faux succès ou
image noire substituée n'est fourni. Un écran est limité à 32 millions de pixels.
Le portail Wayland, les raccourcis globaux et l'historique sont futurs.

## Compilation et vérification

Le parent CMake doit ajouter `add_subdirectory(accessories)`. Dépendances :
Qt6 Widgets, Multimedia, MultimediaWidgets et Test pour les tests.
Les binaires s'installent avec CMake ; aucune dépendance n'est téléchargée par
ces applications et aucune n'a été ajoutée à l'ISO par ce travail.

Tests `accessory-image-files` : vrais fichiers PNG/JPEG/BMP, rotation, alpha,
refus de format invalide et conservation d'un fichier préexistant.
Tests `accessory-ui-and-permissions` : vrai chargement/rotation depuis l'action
Gallery, Video sans source automatique et refus explicite de capture hors X11.
Les tests hors écran ne valident pas une capture réelle, une vidéo affichée,
l'audio, les sous-titres ni les dialogues interactifs d'enregistrement.

Compilation exécutée dans le constructeur Debian 13, Qt 6.8.2, C++17 : les trois
exécutables ont été produits. Les deux groupes CTest ont réussi, dont l'ouverture
de l'aide F1 et le refus de capture hors X11. Journal :
`logs/accessories-build-debian.log`. Aucune intégration ISO effectuée ici.

Les créations Kyronext conservent la licence du projet. Qt et les codecs gardent
leurs licences propres, disponibles dans les paquets Debian correspondants.
