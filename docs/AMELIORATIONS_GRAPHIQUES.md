# Améliorations graphiques transversales

Cette feuille de route concerne les applications natives de `applications/suite`.
Elle sépare les changements déjà codés des propositions qui restent à réaliser.
Les applications KDE standard de l'ISO ne sont pas remplacées par cette liste.

## Couche 1 — framework Qt partagé (compilée et testée)

Le thème de `KyronextUi::applyTheme()` fournit des états de focus au clavier
plus visibles, des sélections cohérentes avec les palettes Rouge/Ambre/Contraste
renforcé et une densité commune. Le mode compact garde les commandes à 36 px au
minimum ; le mode confortable les porte à 44 px. Les lignes de menu restent au
moins 32 px en mode compact. Ces préférences sont dans QSettings ; l'export
utilise désormais le schéma 3 et l'import conserve les exports schéma 1 et 2.
Les schémas plus anciens restaurent les visualisations à leur valeur par défaut.
Le thème partagé reste statique ; la préférence de réduction des effets pilote
les visualisations AMP et la veille Kyronext, sans agir sur les animations KDE.

## 10 améliorations à prioriser

| Priorité | Amélioration | Bénéfice et validation | État |
|---|---|---|---|
| 1 | États communs de focus, sélection, désactivation et composants de base | Navigation clavier plus claire et apparence cohérente ; test automatisé du framework | Validée : compilation Qt Debian et test automatisé réussi |
| 2 | Échelle d'interface compacte et confortable | Conserver les actions accessibles sur l'écran 1280×800 du HP 2540p ; captures comparatives aux deux échelles | Codée, compilée et testée ; captures sur le HP encore à réaliser |
| 3 | Contraste renforcé configurable | Distinguer texte, commandes et éléments sélectionnés ; vérification de contraste et essai sans couleur seule | Codée dans le framework et Kyronext Settings ; compilation et test automatisé réussis |
| 4 | Règles de disposition adaptatives | Empiler les panneaux au lieu de les rogner sur petite largeur ; tests à 800×600, 1024×768 et 1366×768 | Planifiée |
| 5 | Icônes vectorielles originales uniformes | Grille, tailles, épaisseur de trait et états actif/inactif communs ; contrôle SVG et aperçu | Planifiée |
| 6 | États vides et erreurs dessinés par Kyronext | Expliquer quoi faire sans écran vide ; tests d'accessibilité et de contenu | Composant commun codé et utilisé dans AMP/Gallery ; test ajouté, recompilation intégrée non confirmée |
| 7 | Motifs visuels d'accessibilité | Indicateurs de focus, raccourcis affichés, textes alternatifs et usage du clavier ; parcours sans souris | Planifiée |
| 8 | Densité visuelle réglable | Choisir espaces confortables ou compacts sans réduire les cibles clavier/souris ; tests de taille minimale | Espacement des commandes codé et testé ; densité des listes/cartes reste à développer |
| 9 | Réduction des effets visuels | Prévoir un rendu statique ou très léger sur anciens GPU ; profil sans animation et mesure comparative | La préférence désactive les visualisations AMP et le mouvement de la veille Kyronext. Settings exporte au schéma 3 ; les effets Plasma restent séparés. Tests écrits, compilation Qt en attente |
| 10 | Aperçus et captures de référence par application | Détecter les régressions de présentation ; captures automatisées dans l'environnement Qt offscreen | Planifiée |

## Protocole de progression

Les prochaines couches seront intégrées par petites étapes dans le framework
partagé, puis contrôlées sur les applications qui l'utilisent. Une proposition
ne sera marquée « codée » qu'après modification réelle ; « validée » exigera
une compilation et l'exécution du test associé. Cette feuille de route ne
signifie pas que les dix améliorations sont déjà présentes.
