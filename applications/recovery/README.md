# Kyronext Recovery — diagnostic initial

Outil local hors ligne, en lecture seule et sans privilèges. Il rassemble la
distribution, le noyau, l'architecture, l'espace libre de `/`, quelques états
systemd et jusqu'à 20 erreurs du journal du démarrage courant accessibles à
l'utilisateur. Les champs indisponibles sont signalés, pas devinés.

## Utilisation

Afficher un résumé dans le terminal, sans créer de fichier :

```bash
python3 applications/recovery/kyronext_recovery.py
```

Exporter un fichier JSON ou texte vers un nouveau chemin choisi par l'utilisateur :

```bash
python3 applications/recovery/kyronext_recovery.py --format json --output "$HOME/kyronext-recovery.json"
python3 applications/recovery/kyronext_recovery.py --format text --output "$HOME/kyronext-recovery.txt"
```

Le parent doit déjà exister. Le rapport est créé en mode privé `0600`; un fichier
existant ou un lien symbolique n'est jamais écrasé. L'outil refuse de s'exécuter
en root. La sortie ne contient pas de nom d'hôte, de processus, d'adresse
réseau ni de numéro de série matériel. Les messages du journal sont limités et
leurs adresses IP, MAC, courriels, chemins utilisateur, URL et paramètres
secrets courants sont masqués. Relisez le rapport avant de le partager.

Si journald ou les services systemd ne sont pas accessibles, la sortie le dit
explicitement. Une session Live peut présenter des services inactifs qui sont
normaux pour cette session. Ce module ne répare rien, ne modifie aucun service,
ne collecte pas de données en arrière-plan, n'envoie rien sur le réseau et ne
garantit pas l'intégrité du système.

## Compilation et tests

L'installation CMake copie le script exécutable dans le répertoire `bin` choisi
par `CMAKE_INSTALL_PREFIX` ; Python 3 et sa bibliothèque standard sont les seuls
prérequis à l'exécution.

```bash
cmake -S applications/recovery -B build/recovery -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build/recovery
cmake --install build/recovery --prefix "$HOME/.local"
python3 -m unittest discover -s applications/recovery -v
```

Les tests utilisent des fixtures temporaires pour les journaux, états de
services et exports ; ils ne lancent aucune réparation et ne touchent pas les
disques physiques.
