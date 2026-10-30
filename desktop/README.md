# Bureau Kyronext — identité graphique

Le bureau reste KDE Plasma 6 avec fenêtres et composants standards. Le profil visuel fourni dans iso/config/includes.chroot comprend :

- Palette complète KyronextGraphite : graphite, fonds de lecture profonds, sélection bordeaux, accent rouge et texte clair.
- Icônes vectorielles originales Kyronext pour le menu, Dolphin, Konsole, paramètres et dossiers. Le thème hérite de Breeze pour les icônes non redessinées ; aucune application KDE n'est renommée artificiellement.
- Panneau inférieur sur toute la largeur, menu portant le nom et le logo Kyronext, tâches avec raccourcis explicites vers Dolphin, Konsole et les paramètres, zone de notification et horloge/date. Les applications absentes ne sont pas épinglées.
- Hauteur adaptée à la résolution initiale (42 à 56 pixels), aucune longueur fixe qui dépasse d'un petit écran. KDE conserve ses outils de configuration et l'adaptation à la largeur disponible.
- Animations et composition X11 désactivées par défaut pour cette préversion, sans modifier le noyau ou les pilotes. Elles restent réactivables dans les paramètres KDE. Le verrouillage automatique demeure désactivé pour le compte Live verrouillé.

Le script de migration kyronext-visual-02.js est distinct de l'ancien script de fond. Il réutilise le panneau principal, ne touche pas aux autres panneaux et ne s'applique qu'une fois, grâce à LayoutVersion dans kyronext-desktoprc. Dans la session Live neuve, ce profil remplace les raccourcis Debian génériques du panneau. Réexécuter la même version ne crée pas de panneau ou widget en double.

## Validation réellement effectuée

- node --check : syntaxe JavaScript correcte.
- Exécution simulée de l'API Plasma avec Node : second panneau préservé, aucune duplication après deux exécutions, raccourci absent omis et hauteur de panneau correcte sur une géométrie800x600.
- Contrôle ConfigParser : groupes de palette présents et composantes RGB dans0..255.
- Parse XML de neuf ressources SVG : réussi.

Ces vérifications ne constituent pas une validation visuelle dans KDE. Le rendu et les interactions du menu/panneau doivent encore être observés dans la nouvelle ISO par la validation principale.

## Références de l'API

- https://develop.kde.org/docs/plasma/scripting/
- https://develop.kde.org/docs/plasma/scripting/api/
- Sources Plasma6.3 : shell/scripting/panel.h et panel.cpp, configuration Kickoff et horloge numérique.
- Noms .desktop vérifiés dans les listes de fichiers Debian Trixie : org.kde.konsole.desktop et systemsettings.desktop.
