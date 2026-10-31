# Kyronext Application Suite — socle C++/Qt 6

Premiers composants natifs, distincts de l'ISO déjà validée. Leur présence dans
les sources ne signifie pas qu'ils sont compilés, empaquetés ou livrés dans une
image. Aucun outil KDE de secours n'est retiré.

## Construction dans Debian 13 isolé

Dépendances officielles à vérifier avec APT : `g++`, `cmake`, `ninja-build`,
`dpkg-dev`, `qt6-base-dev`, `qt6-multimedia-dev`, `libkf6kio-dev`,
`libkf6archive-dev` et `qt6-webengine-dev`. Ces paquets sont les outils de
construction Debian 13 pour toute la suite, Navigator compris. Aucun paquet
n'est installé automatiquement par CMake. Depuis la racine du dépôt :

```bash
cmake -S applications/suite -B build/suite -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/suite --parallel 1
QT_QPA_PLATFORM=offscreen ctest --test-dir build/suite --output-on-failure
./build/suite/paint/kyronext-paint
./build/suite/amp/kyronext-amp
```

On peut construire Paint sans AMP avec `-DKYRONEXT_BUILD_AMP=OFF`.
Kyronext Backup et Disk Health se construisent également dans la suite ; Backup
nécessite le paquet système `tar` à l'exécution, tandis que `smartmontools` est
facultatif pour Disk Health. Aucun des deux ne s'exécute en root. Le premier
crée uniquement des archives non chiffrées de dossiers explicitement choisis ;
le second ne lit que les états SMART présentés par le contrôleur.
Pour préparer un répertoire de paquet sans modifier le système :

```bash
DESTDIR="$PWD/build/suite-staging" cmake --install build/suite --prefix /usr
```

Le packaging Debian, les essais d'installation/désinstallation et l'intégration
dans l'ISO restent une étape séparée. Ne pas lancer un install global sur l'hôte
Windows ou sur le système Linux principal.

## Deux navigateurs

Firefox ESR officiel Debian est livré avec un profil Kyronext facultatif et
isolé : page locale Start, palette sombre/rouge et raccourcis officiels.
L'application Firefox normale reste disponible. Le profil et les paramètres
existants de Firefox ne sont pas réécrits.

Kyronext Navigator reste lui aussi dans la suite. Il utilise Qt WebEngine et
reste explicitement expérimental : il n'est pas le navigateur par défaut. La
construction teste TLS non fiable, refus de permissions, profil privé, liens
locaux et protections de schémas. Firefox ESR demeure le navigateur recommandé
tant que Navigator n'a pas été validé en session Live et suivi à travers les
mises à jour de sécurité de Debian.

## Kyronext UI Framework 0.1

Bibliothèque statique C++17 `KyronextUi`, construite sur Qt Widgets. Ce premier
socle privilégie les composants natifs accessibles, les menus et raccourcis
standards, sans animation ni décoration de fenêtre fragile. Un adaptateur
Qt Quick/QML pourra réutiliser les mêmes conventions lorsque nécessaire.

API publique dans `ui/include/kyronext/ui.h` :

- `configureApplication` : identité, version, icône originale et traduction.
- `applyTheme`, `paletteFromSettings`, `setPreferredPalette` : rouge, ambre ou contraste renforcé.
- `densityFromSettings`, `setPreferredDensity` : commandes compactes ou confortables ; au moins 36/44 px.
- `reducedMotionFromSettings`, `setReducedMotionEnabled` : préférence qui désactive les visualisations animées dans AMP ; les effets du bureau Plasma ont leur propre réglage.
- `addAboutMenu` : dialogue commun avec logo, éditeur, signature, version,
  bibliothèques, informations système et site officiel.
- `showError`, `notify` : messages locaux compréhensibles et accessibles.
- `emptyState` : panneau de marque accessible pour guider l'utilisateur lorsque
  AMP n'a pas de playlist ou Gallery n'a pas encore ouvert d'image.

Les préférences utilisent QSettings, par application et utilisateur, sans
privilège administrateur. Les couleurs et la densité sont enregistrées sous
`appearance/palette` et `appearance/density`. Kyronext Settings peut exporter
ces préférences au schéma JSON 2 et continue d'importer les exports schéma 1.
L'export des préférences utilise le schéma JSON 3 et conserve l'import des
schémas 1 et 2 ; les anciens fichiers restaurent les effets visuels à leur valeur
par défaut.
Le chargement des traductions recherche des fichiers
`kyronext_LANGUE.qm` dans le sous-dossier `translations` de l'emplacement
QStandardPaths AppDataLocation. Aucun catalogue traduit n'est encore livré.

La journalisation Qt est locale, sous QStandardPaths AppLocalDataLocation,
dans `application.log`, avec une copie précédente et rotation à environ 1 Mio.
Le fichier est limité au propriétaire sous Linux. `logging/enabled=false`
dans les préférences désactive l'installation du journal applicatif. Les
erreurs de documents ne sont pas recopiées dans le journal par `showError`.
Il n'existe aucun envoi réseau ni télémétrie.

Les notifications de ce socle sont des dialogues Qt locaux ; les notifications
de bureau D-Bus ne sont pas encore intégrées. La conformité complète avec les
lecteurs d'écran n'a pas été testée. Les menus et contrôles standards restent
pilotables au clavier ; le canevas de Paint exige actuellement un pointeur.

Les créations Kyronext restent réservées à Kyronext Systems — By Manix dans
l'attente d'un choix de licence. Qt et ses plugins gardent leurs propres
licences ; consulter les copyrights des paquets Debian effectivement installés.
Le dialogue À propos n'accorde aucune nouvelle licence à ces composants.

## Kyronext Paint — première version utilisable

Créer une image, ouvrir PNG/JPEG/BMP, dessiner au pinceau, effacer avec
transparence, choisir la couleur et l'épaisseur, zoomer, annuler/rétablir puis
enregistrer ou enregistrer sous. Plusieurs documents sont ouverts en onglets.
Le menu Aide contient la fenêtre À propos commune.

Raccourcis : Ctrl+N, Ctrl+O, Ctrl+S, Ctrl+Maj+S, Ctrl+Z, Ctrl+Maj+Z et Ctrl+W.
Un astérisque signale les modifications. Fermer un document ou quitter demande
de sauvegarder, abandonner ou annuler. Annuler un choix de fichier bloque la
fermeture. JPEG et BMP sont aplatis sur fond blanc après confirmation.

L'écriture passe par QSaveFile : une erreur ne doit pas vider le fichier cible.
Cela ne constitue pas une sauvegarde de secours ni une protection contre les
modifications simultanées d'autres programmes.

Limites : 8 documents, 16 mégapixels par image, allocation du décodeur limitée
à 128 Mio. L'historique conserve au plus 32 étapes avec un budget visé de
32 Mio par direction ; pour une grande image il conserve au moins une étape,
qui peut donc dépasser ce budget. Plusieurs grandes images restent coûteuses
en mémoire. Le format PNG conserve la transparence.

Sélection, recadrage, rotation, redimensionnement, texte, formes, remplissage,
calques, filtres et IA restent à développer. Aucun bouton de ces fonctions
n'est affiché comme opérationnel. Cette version n'est pas un remplacement
complet d'un éditeur d'images existant.

## Validation

Tests Qt Test inclus : modification réelle des pixels, gomme transparente,
undo/redo, invalidation du redo, aller-retour PNG et conservation de l'image
après erreur de chargement/enregistrement. Leurs résultats doivent être relevés
après compilation. Ils ne prouvent pas l'audio, les dialogues interactifs, le
comportement du gestionnaire de fenêtres ni le lancement depuis une ISO.
