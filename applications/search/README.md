# Recherche locale Kyronext — premier périmètre

Kyronext Launchpad propose déjà la recherche et les favoris des applications
installées. Cette extension ajoute une portée de recherche de fichiers facultative
dans des dossiers choisis par l'utilisateur.
Par défaut, aucun dossier personnel n'est indexé et l'ouverture du Launchpad ne
déclenche aucun parcours. L'indexation commence lorsque l'utilisateur choisit
la portée « Fichiers sélectionnés », ajoute un dossier ou demande une
réindexation. Le premier index est limité à 20 000 fichiers, 10 000 dossiers,
12 niveaux de sous-dossiers et 8 secondes par demande ; la recherche affiche au
plus 100 résultats. L'utilisateur peut interrompre l'indexation. Les entrées
cachées, liens symboliques et volumes montés sous un dossier choisi ne sont
pas parcourus. Seuls les noms sont gardés temporairement en mémoire ; le
contenu des fichiers n'est pas lu et aucun index n'est envoyé ou persisté.

Les résultats fichiers s'ouvrent avec l'application associée par le bureau via
une URL locale, sans commande shell. Cette version ne cherche pas encore dans le
contenu des documents, les réglages ou la documentation, et n'annonce donc pas
la recherche universelle complète.

Validation : les 9 tests du backend passent sous Windows et Debian 13, dont les
limites, la durée, l'annulation, la progression et les liens symboliques sous
Linux. `python3 applications/search/check_launchpad.py` contrôle le lancement
Qt hors écran ; il ne simule pas les dialogues ni l'ouverture interactive d'un fichier.
