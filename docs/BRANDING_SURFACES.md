# Identité des écrans système

## Révision préparée après la première publication

Cette révision est présente dans les sources. Elle ne change pas rétroactivement
les ISO déjà produites : il faut reconstruire, puis contrôler les écrans en VM.

| Surface | Personnalisation | Validation actuelle |
|---|---|---|
| À propos de ce système, KInfoCenter | `etc/xdg/kcm-about-distrorc` impose le nom, la version et le logo original Kyronext | Champs vérifiés dans le code amont KDE Plasma 6.3 ; affichage de cette révision non testé |
| Applications lisant os-release | Ajout `LOGO=kyronext-logo` et `HOME_URL` au hook de construction | Syntaxe du hook contrôlée ; le remaster doit aussi appliquer ces deux champs |
| Icônes de distribution et menu générique | Alias originaux `distributor-logo.svg` et `start-here.svg` dans le thème Kyronext | SVG analysés ; pas de remplacement des fichiers du paquet Debian |
| Connexion SDDM | `debian-breeze` conservé avec `theme.conf.user` : logo et fond Kyronext | Noms des options vérifiés dans Breeze 6.3 ; écran de connexion à contrôler |
| Chargement de la session Plasma | Thème isolé `org.kyronext.desktop`, logo, textes et trait rouge statiques | JSON et chemins vérifiés ; rendu QML et démarrage de session à contrôler |
| Console locale | `etc/issue` présente Kyronext OS | Fichier statique ; autologin console toujours désactivé |
| Menu BIOS/UEFI et bureau | Personnalisations déjà réalisées lors de la révision précédente | Voir `VALIDATION_VISUELLE.md` |
| Plymouth | Aucun changement | Aucun paquet Plymouth dans le manifeste actuel ; aucun écran Plymouth ajouté |

Le splash ne comporte aucune minuterie, animation, accès réseau ni pourcentage
de chargement inventé. Le bureau conserve Breeze comme thème global et ses
composants standards ; seul le splash sélectionné dans `ksplashrc` est changé.
L'authentification, les sessions et l'autologin SDDM ne sont pas modifiés.

La base Debian, les noms techniques des paquets, les dépôts APT et les crédits
des composants sont conservés. Les noms KDE figurant dans les crédits des
applications ne sont pas remplacés par une attribution Kyronext.

## Intégration dans une nouvelle ISO

Une reconstruction live-build exécute le hook `0900-branding.hook.chroot`.
Le chemin rapide `remaster-branding.sh` copie les fichiers `includes.chroot`,
mais n'exécute pas ce hook : ses champs `LOGO` et `HOME_URL` doivent être
appliqués explicitement dans la racine de travail du remaster. Ne jamais
exécuter le hook chroot sur l'hôte pour cette opération : il contient aussi
la préparation de sécurité du compte Live.

Tests manuels attendus : démarrage jusqu'au bureau, À propos de ce système,
passage du splash, puis contrôle de l'écran SDDM dans une VM de test dédiée.
La session Live possède un mot de passe verrouillé ; préserver la session
interactive de l'utilisateur et ne pas la fermer pour tester SDDM.

## Références amont consultées

- [KInfoCenter 6.3, lecture de LogoPath et des informations système](https://github.com/KDE/kinfocenter/blob/Plasma/6.3/kcms/about-distro/src/main.cpp)
- [Breeze SDDM 6.3, variables du thème](https://github.com/KDE/plasma-desktop/blob/Plasma/6.3/sddm-theme/theme.conf.cmake)
- [Breeze SDDM 6.3, utilisation des variables](https://github.com/KDE/plasma-desktop/blob/Plasma/6.3/sddm-theme/Main.qml)
- [Structure du thème et splash Plasma 6.3](https://github.com/KDE/plasma-workspace/tree/Plasma/6.3/lookandfeel/org.kde.breeze)
