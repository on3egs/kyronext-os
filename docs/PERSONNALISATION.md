# Personnalisation Kyronext OS

Ce document décrit le nouveau profil préparé dans les sources de la préversion
0.1. L'intégration et la validation visuelle de la nouvelle ISO sont en cours
au moment de sa rédaction. La présence des fichiers ne prouve pas encore leur
rendu dans KDE : les captures et le rapport de validation de la nouvelle image
doivent confirmer le résultat. Une ancienne session Live conserve son ancien
profil jusqu'au démarrage de l'image mise à jour.

## Le bureau

KDE Plasma conserve ses fenêtres, son menu et ses réglages standards. Le profil
Kyronext ajoute une palette complète **KyronextGraphite**, avec fonds graphite,
texte clair, sélection bordeaux et accent rouge. Les icônes vectorielles du
menu, des fichiers, du terminal et des paramètres sont des créations originales.
Le thème hérite de Breeze pour les autres icônes ; aucune application existante
n'est présentée artificiellement comme une nouvelle application Kyronext.

Le panneau inférieur occupe la largeur disponible. Il réunit le menu Kyronext,
les tâches et leurs raccourcis vers Dolphin, Konsole et les paramètres KDE,
la zone de notification et l'horloge avec la date. Les raccourcis d'applications
absentes sont omis. Sa hauteur dépend de la résolution initiale. Le script
versionné réutilise le panneau principal et prévoit de ne pas créer de doublon
lors d'une seconde exécution.

Les animations et la composition X11 sont désactivées par défaut dans ce profil
pour privilégier l'affichage sur les machines modestes. KDE conserve ses
mécanismes de personnalisation ; leur réactivation reste un choix de
l'utilisateur selon son matériel. Rechercher « animations » ou « effets » dans
les paramètres KDE pour les options disponibles. Aucun réglage du noyau ou
pilote propriétaire n'est ajouté pour obtenir ces effets visuels.

## L'accueil

L'**Accueil Kyronext** est une application PyQt6 locale avec une identité
graphite, bordeaux et rouge, la signature **Kyronext Systems — By Manix** et
quatre actions correspondant à des outils réellement inclus :

| Action | Outil ouvert | Effet |
|---|---|---|
| Fichiers | Dolphin | Consulter les dossiers |
| Terminal | Konsole | Ouvrir un terminal utilisateur |
| Paramètres | Paramètres système KDE | Accéder aux réglages du bureau et des périphériques |
| Diagnostic matériel | Rapport local dans Konsole | Lire les informations matérielles sans installer ni modifier de pilote |

Le bouton « Découvrir le bureau » ferme l'accueil. Celui-ci apparaît une fois
automatiquement dans la session Live. Pour le rouvrir, chercher **Accueil
Kyronext** dans le menu des applications ; le lancement manuel reste possible
après sa fermeture. La commande correspondante est `kyronext-welcome`.

La fenêtre s'adapte à la place disponible. Une zone défilante garde les actions
accessibles sur 800×600 et sur un affichage agrandi. Les boutons restent
accessibles au clavier avec la navigation habituelle de Qt. L'accueil ne
contient pas d'animation imposée, de faux indicateur CPU/GPU ou de téléchargement.
Son rendu aux différentes résolutions et les actions doivent être vérifiés
dans la VM avant validation finale.

## Le démarrage Live

Les fonds BIOS et UEFI sont préparés à partir d'une création vectorielle
originale Kyronext. Le hook de fabrication remplace la présentation graphique
Debian/live-build du menu, tout en conservant les chemins du noyau et de
l'initramfs et les entrées Live et de secours. Le fond BIOS et le fond UEFI ont
des dimensions adaptées à leurs menus respectifs ; leur validation est séparée.

Au menu de démarrage, choisir l'entrée **Kyronext OS 0.1 Live** et appuyer sur
Entrée. Ce menu démarre un environnement Live ; il ne lance aucun installateur.
Le mode de secours garde ses paramètres de diagnostic et ne constitue pas une
garantie de compatibilité avec tout matériel ancien.

## Sources de l'intégration

- `desktop/README.md` décrit la palette, les icônes et la migration du panneau.
- Les réglages utilisateur et ressources installées sont dans
  `iso/config/includes.chroot/`.
- `applications/welcome/README.md` décrit le lanceur de l'accueil, ses actions
  et la distinction entre autostart et ouverture manuelle.
- `branding/boot/README.md` décrit les fonds et leur intégration dans l'arbre ISO.

Les ressources graphiques sont originales. La veille utilise des illustrations
évocatrices des voitures KITT et KARR de la série télévisée ; aucun photogramme,
logo officiel, son ou voix de la série n'est copié. Le nom public reste
**Kyronext OS**. Ces créations ne constituent pas une autorisation d'exploitation
des personnages ou des modèles de véhicules ; voir `LICENSES/VEILLE.md`.

## Veille Kyronext

Chercher **Veille Kyronext** dans le menu lance une animation native PyQt6 :
les voitures avancent en perspective sur fond noir, à la manière d'un champ
d'étoiles. Les modes rouge, ambre et mixte sont disponibles. Échap ferme la
fenêtre ; `kyronext-stardrive --reduced-motion` désactive le mouvement.
Le lancement reste manuel : cette application ne verrouille pas la session et
ne configure pas la veille automatique de KDE.

Les deux images sont installées dans `/usr/share/kyronext/stardrive/` depuis
`iso/config/includes.chroot/usr/share/kyronext/stardrive/`. Les consignes de
génération sont conservées dans `applications/screensaver/IMAGE_PROMPTS.json`.
Deux captures Qt hors écran ont confirmé des images différentes pendant
l'animation, puis la fermeture par Échap avec un code de sortie nul. Ce test
ne mesure pas les performances sur un ordinateur physique. Le lancement depuis
KRunner dans le bureau de la dernière ISO, l'animation plein écran et le retour
par Échap ont ensuite été vérifiés dans VirtualBox UEFI. Voir
`docs/VALIDATION_VISUELLE.md` pour les preuves et le périmètre exact.

## Limites de cette étape

La personnalisation du bureau ne remplace pas KDE par un Cockpit. Le scanner
animé, les instruments du Cockpit et l'IA locale restent des fonctions futures.
Cette ISO demeure un Live sans installateur sur disque ; les changements de la
session sont temporaires. Le système n'affiche pas ces fonctions futures comme
si elles étaient déjà disponibles.
