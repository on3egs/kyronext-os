# Kyronext AMP — première version native

Application C++17 / Qt 6 Widgets et Multimedia. Elle utilise le framework commun
Kyronext pour son thème et sa fenêtre À propos. Aucune copie de skin, logo,
musique ou code propriétaire d'un autre lecteur.

## Fonctions présentes

- Lecture/pause, stop, piste précédente/suivante, double-clic d'une piste.
- Position, durée, temps restant, volume à progression cubique et mute.
- Ajout/retrait de fichiers locaux, filtre de playlist, aléatoire et répétition.
- Import M3U/M3U8 UTF-8, chemins relatifs, export atomique et confirmation de
  conservation d'une playlist modifiée. L'import ne démarre pas la lecture.
- Titre, artiste, album et pochette quand le backend fournit les métadonnées.
- Menus, raccourcis Ctrl+O/Ctrl+S et espace, palette rouge ou ambre, À propos.

Les boutons retirent uniquement des entrées de playlist : ils ne suppriment
jamais les fichiers musicaux. La playlist courante n'est pas restaurée
automatiquement ; l'export M3U permet sa conservation. Une entrée absente ou
distante est ignorée avec un message. Les flux Internet restent désactivés.
Limites : 1 Mio par playlist importée et 10 000 entrées ; pas de médiathèque
persistante ni d'édition de métadonnées.

La liste de formats du dialogue est un filtre de fichiers, pas une garantie
de décodage. MP3/FLAC/OGG/WAV et autres formats dépendent des codecs du backend
Qt Multimedia fourni par Debian. Une erreur réelle est affichée si un format
ou un périphérique audio manque. Les notifications du backend ne prouvent
pas que des haut-parleurs physiques fonctionnent.

## Compilation et tests

Depuis `applications/suite/`, utiliser le CMake commun avec
`-DKYRONEXT_BUILD_AMP=ON -DBUILD_TESTING=ON`, puis `cmake --build` et `ctest`.
Les tests vérifient les playlists UTF-8, les rejets distants, un export invalide
préservant la destination, la sélection suivante et le décodage réel d'un WAV
synthétique de 1 seconde. Aucun son propriétaire n'est utilisé.

L'installation CMake ajoute le binaire, un lanceur `.desktop` et une icône SVG
originale. Elle ne change pas les associations par défaut de l'OS. Cette suite
reste indépendante de l'ISO tant que sa validation n'est pas terminée.

## Egaliseur et visualisations DSP

Le mode Lecture DSP est facultatif et recharge le morceau courant. Il decode
le fichier local avec QAudioDecoder, traite le PCM par dix filtres biquad par
canal, puis envoie le PCM traite a QAudioSink. La voie standard QMediaPlayer
reste disponible sans egaliseur. Frequences : 31, 63, 125, 250, 500 Hz puis
1, 2, 4, 8, 16 kHz ; chaque bande de -12 a +12 dB. Les bandes au-dessus du
Nyquist du fichier sont neutres.

Presets Neutre, Rock, Pop, Jazz, Classique, Vocal, Personnalise ; bypass et
preampli -24 a +6 dB. Le garde-fou numerique borne la sortie entre -1 et +1
et signale les echantillons limites. Il peut creer de la distorsion : reduire
le preampli lorsqu'il intervient. Ce n'est pas un limiteur transparent de mastering.

Spectre DFT fenetre, oscilloscope, vu-metre et scanner reactif utilisent le
vrai PCM apres egalisation, avant volume du peripherique. Aucun hasard.
Le scanner reagit a l'amplitude. Les visualisations sont desactivables et leur
minuterie s'arrete quand elles sont masquees. F11 affiche le lecteur en plein ecran.
Le spectre est un apercu lineaire borne a 256 echantillons, pas un instrument calibre.

Le fichier est decode avant lecture, avec une limite PCM de 128 Mio, mono a
8 canaux et sortie flottante au taux natif. Un codec, une limite memoire ou
un peripherique incompatible provoque un message et le retour a la lecture
standard sans egaliseur. Ce moteur ne fait pas encore de streaming DSP.

Tests : neutralite et bypass, gain de 6 dB sur sinus, preampli, isolation des
canaux, impulsion, limitation et lecture du vrai QIODevice traite. Le decodage
WAV synthetique reste teste separement. L'ecoute physique et les peripheriques
restent a valider. Mediatheque, MPRIS, radios, normalisation et fondu restent futurs.

## Dépendances et licences

Qt 6 Widgets/Multimedia : conditions de la version Debian utilisée, notamment
LGPL/GPL selon les composants. Les codecs et backends conservent leurs licences
propres ; relever les paquets effectifs et leurs `copyright` avant publication.
Les créations Kyronext restent réservées à Kyronext Systems — By Manix en
attendant un choix de licence du propriétaire.

Références techniques primaires :

- https://doc.qt.io/qt-6/qmediaplayer.html
- https://doc.qt.io/qt-6/qaudiooutput.html
- https://doc.qt.io/qt-6/qmediametadata.html
- https://doc.qt.io/qt-6/qaudiobufferoutput.html
