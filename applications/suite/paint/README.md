# Kyronext Paint — outils de dessin

Application native C++/Qt 6, indépendante, utilisant le thème et le menu À propos
de KyronextUi. Aucun composant Web ni accès administrateur.

Le sélecteur de la barre d'outils propose pinceau, gomme transparente, ligne,
rectangle, ellipse, remplissage, pipette, texte et sélection rectangulaire.
Les formes s'affichent en pointillés pendant le déplacement puis sont appliquées
aux pixels au relâchement. La pipette choisit la couleur du pixel cliqué.
Le texte est dessiné au point choisi ; le bouton Police règle sa police.

Pour recadrer, choisir Sélection, tracer la zone puis utiliser
**Image → Recadrer la sélection**. Le menu Image permet également le
redimensionnement et la rotation de 90 degrés. Ces opérations sont annulables.
Le redimensionnement utilise des pixels absolus et peut modifier les proportions.

Chaque document conserve ses propres pixels et son historique. Les documents
s'ouvrent dans des onglets. Ctrl+N/O/S, Ctrl+Maj+S, Ctrl+Z et Ctrl+Maj+Z suivent
les conventions Qt. La fermeture demande confirmation des modifications.
Enregistrer conserve les pixels en PNG ; JPEG et BMP sont aplatis sur blanc
après confirmation. Les écritures utilisent QSaveFile pour préserver le fichier
précédent si l'enregistrement échoue.

La limite est de 16 mégapixels par image et 8 documents. L'historique est borné,
avec au moins un état complet par document ; une très grande image reste coûteuse
en mémoire. Le remplissage utilise des segments contigus plutôt qu'une file de
chaque pixel. Les fichiers invalides et transformations hors limites sont refusés.

Calques, filtres, effets et IA restent à développer. Les formes sont des contours,
le remplissage peut ensuite colorer leur intérieur. Les outils ne constituent
pas encore un éditeur vectoriel ou une gestion de sélections multiples.

Validation automatique : `ctest --test-dir build/native-suite -R paint
--output-on-failure` après compilation. Les tests contrôlent les frontières du
remplissage, les pixels de formes et de texte, les transformations, l'historique,
les erreurs et la sauvegarde PNG. Ils ne remplacent pas le contrôle des dialogues
et des périphériques d'entrée dans la session Live.
