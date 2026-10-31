# Outils système Kyronext à développer — priorités

État de référence : audit du dépôt le 31 octobre 2026. Cette liste complète la
feuille de route sans prétendre que ces outils sont déjà disponibles dans l'ISO.
Les composants KDE/Debian existants restent les solutions de secours tant que
les remplacements Kyronext n'ont pas été testés et empaquetés.

## Les dix priorités

| Rang | Outil ou capacité | Première version utile | Pourquoi / garde-fous |
|---:|---|---|---|
| 1 | **Kyronext Installer** | Installateur Calamares séparé, d'abord dans une VM avec disques virtuels jetables ; détection explicite de la cible, résumé avant écriture et annulation sûre. | Le Live 0.1 ne s'installe pas sur disque. C'est le blocage principal pour un usage quotidien. Ne jamais tester sur un disque physique et ne publier qu'après essais UEFI/BIOS et redémarrage. |
| 2 | **Kyronext Update** | Première interface Qt ajoutée : simulation locale `apt-get -s upgrade`, résumé des paquets proposés et sortie APT consultable. | Cette version est strictement en lecture seule : elle n'actualise pas les index, ne télécharge rien et n'installe rien. CTest vérifie le résumé. L'installation confirmée et l'origine des index restent à intégrer via les mécanismes Debian signés. |
| 3 | **Kyronext Backup** | MVP de sauvegarde archivée : dossiers explicitement choisis, destination nouvelle, annulation, empreinte SHA-256. Restauration, chiffrement, planification et vérification après copie restent à faire. | Le code utilise GNU tar par argv, sans root ni shell ; les tests isolés ont passé 5 cas sous Debian 13. Les archives sont en clair et l'application n'est pas encore intégrée à l'ISO. |
| 4 | **Kyronext Recovery** | Module source ajouté : rapport local JSON/texte, lecture seule, sans root, états de services et jusqu'à 20 erreurs accessibles, avec masquage de données courantes. | Tests fixtures et export privé passés sur Debian ; pas d'action de réparation. Le démarrage léger et la restauration restent hors périmètre jusqu'à intégration et tests dédiés. |
| 5 | **Kyronext Network** | Monitor affiche l'état de liaison, les adresses locales actives, la route par défaut et les DNS configurés lorsque Linux les expose. | Les parseurs route/DNS et les liens sont testés sur fixtures. Aucune requête réseau ni test d'accès Internet n'est exécuté ; aucun scan ni collecte distante. Les profils NetworkManager restent à intégrer. |
| 6 | **Kyronext Devices** | Compléter Files/Monitor par l'état du support, le système de fichiers et une action d'éjection sûre via Solid/KIO. | Files propose déjà les volumes montés et Monitor affiche espace libre/capacité. L'ajout utile est l'éjection et l'état, pas une seconde liste de volumes ; aucun formatage automatique. |
| 7 | **Kyronext Disk Health** | Nouvelle interface source en lecture seule : énumération prudente via `/sys/block` et lecture `smartctl -j -H` si `smartmontools` est déjà installé. | Aucun root, écriture, numéro de série exposé ou installation automatique. L'état inconnu est explicite ; compilation/test de la suite intégrée et contrôle sur disque physique restent à valider. `smartmontools` n'est pas imposé. |
| 8 | **Kyronext Power** | Monitor détecte en lecture seule les sources secteur/USB et estime l'autonomie si le noyau expose des mesures valides. Étape suivante : profils d'énergie via les services KDE/systemd réellement disponibles. | Détection et calcul ont des tests sysfs simulés. Les profils restent à réaliser. Estimation plafonnée et présentée comme indicative ; aucune écriture matérielle. |
| 9 | **Kyronext Print & Scan** | Assistant de découverte locale des imprimantes/scanners, ajout explicite et page de test, avec CUPS et SANE. | Fonction souvent attendue sur un bureau grand public. Les pilotes et firmwares dépendent des modèles ; pas de pilote téléchargé ou activé sans source vérifiée. |
| 10 | **Kyronext Accessibility** | Compléter Settings par les options de clavier à l'écran, lecteur d'écran et accès direct aux réglages d'accessibilité KDE. | Settings propose déjà la taille du texte et, depuis cette révision, le contraste renforcé ; le framework a des états de focus visibles. Ne pas dupliquer ces réglages. Réutiliser Plasma/Qt et valider au clavier. |

## Ordre d'exécution proposé

1. Valider le démarrage Live et la compatibilité réelle sur le HP avant toute
   installation ; préparer en parallèle un scénario d'installation jetable.
2. Livrer d'abord Installateur expérimental et mise à jour contrôlée, après
   tests reproductibles en VM et vérification de la conservation des données.
3. Ajouter sauvegarde et diagnostic de récupération avant d'élargir les actions
   de maintenance.
4. Intégrer réseau, supports amovibles, énergie et santé disque en réutilisant
   les interfaces système prises en charge par Debian 13.
5. Ajouter impression/scanner et panneau d'accessibilité après validation des
   dépendances, licences, traductions et parcours clavier.

## État vérifié et limites

Le dépôt contient déjà Kyronext Files, Monitor, Settings, Capture, Gallery,
Video, Calendar, Clock, Archive, Guardian Hardware Passport, Welcome, Notes,
Calculator, Paint, AMP et Navigator dans des états de développement variés.
Leur présence dans les sources ne prouve pas leur présence dans l'ISO ni leur
validation sur matériel physique. Firefox ESR et les outils KDE standards sont
conservés comme solutions de secours. Le diagnostic Guardian est local et en
lecture seule ; il ne configure pas les pilotes.

Monitor expose déjà les volumes, les températures et les informations batterie
que le noyau fournit ; il indique désormais aussi les sources secteur/USB
disponibles et une estimation si les mesures existent. Files propose des accès
aux volumes montés. Les priorités 6 et 8 sont des extensions ciblées (éjection
sûre et profils d'énergie),
pas de nouvelles listes de données déjà affichées.

Les noms ci-dessus sont des propositions de produit, non des binaires existants.
Chaque capacité doit recevoir ses propres tests, documentation et statut de
compatibilité ; aucun bouton ne doit annoncer une fonction avant son intégration.
