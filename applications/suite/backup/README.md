# Kyronext Backup — MVP

Kyronext Backup crée une archive locale `tar.gz` à partir des dossiers choisis explicitement par l’utilisateur. L’interface utilise le thème partagé Kyronext et affiche l’empreinte SHA-256 calculée après création.

## Sécurité et limites

- Les opérations sont exécutées avec les droits de la session courante ; l’application refuse de fonctionner si elle est lancée en root.
- Le programme `tar` du système est invoqué avec des arguments séparés par `QProcess`. Aucun shell ni commande construite depuis une chaîne utilisateur n’est utilisé.
- La destination doit être un nouveau fichier dans un dossier déjà existant. L’application refuse toute destination existante et publie l’archive par lien atomique sans remplacement. Le fichier intermédiaire privé est supprimé si `tar` échoue ou si l’utilisateur annule.
- Une destination située dans un dossier sauvegardé est refusée pour éviter que l’archive s’inclue elle-même.
- **Les archives ne sont pas chiffrées.** Le texte d’avertissement est affiché avant la sauvegarde. Stockez l’archive sur un emplacement de confiance et utilisez un outil de chiffrement distinct si nécessaire.
- L’empreinte SHA-256 est montrée à l’écran et peut être copiée. Le MVP ne crée pas de fichier manifeste ou de somme séparé.
- Le MVP n’offre pas encore de restauration, planification, sauvegarde incrémentale, sélection de fichiers individuels ou estimation de taille/progression.
- GNU `tar` doit être disponible (`/usr/bin/tar` ou `/bin/tar`). Les erreurs d’accès à un fichier conduisent à l’échec de l’archive ; aucune archive finale incomplète n’est publiée.
- Les permissions et métadonnées enregistrées sont celles accessibles au compte courant. La restauration exacte des propriétaires privilégiés n’est pas garantie.

## Compiler et lancer sur Debian

Prérequis : Debian avec Qt 6 (Widgets, Test), CMake, Ninja ou Make, un compilateur C++17, GNU `tar`, et le dépôt de sources Kyronext.

Depuis ce dossier, en session utilisateur ordinaire :

```bash
cmake -S . -B /tmp/kyronext-backup-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build /tmp/kyronext-backup-build
ctest --test-dir /tmp/kyronext-backup-build --output-on-failure
/tmp/kyronext-backup-build/kyronext-backup
```

L’installation (si souhaitée) est séparée et explicite :

```bash
cmake --install /tmp/kyronext-backup-build --prefix "$HOME/.local"
```

Cette compilation autonome construit aussi le framework UI partagé depuis `../ui`. Le CMake parent inclut le module lorsque `KYRONEXT_BUILD_BACKUP=ON` ; cela ne l’ajoute pas à l’ISO existante. La reconstruction intégrée de la suite et la validation d'une nouvelle ISO restent nécessaires.

## Tests couverts

Les tests utilisent des dossiers temporaires et le vrai `tar` : création et lecture de l’archive avec empreinte, préservation d’une destination existante, refus d’une destination située dans la source, nettoyage après échec de l’outil et annulation avant démarrage. Ils doivent être lancés sans root. Ils n’accèdent à aucun disque amovible.

## Licences

Le code de ce module est distribué sous la licence globale du dépôt Kyronext. Il dépend de Qt 6 et du `tar` fourni par Debian ; consulter les notices des paquets de la distribution pour leurs licences respectives.
