# État de validation — Kyronext Recovery 0.1

Date : 31 octobre 2026.

## Réussis

- Tests unitaires sur Windows/Python 3.13 : 3 réussis, 1 test de lien
  symbolique ignoré car Windows n'accorde pas le privilège de créer ce lien
  dans cet environnement.
- Tests unitaires dans la VM Debian : 4 réussis, dont le refus d'écraser un
  fichier ou de suivre un lien symbolique.
- Syntaxe Python (`py_compile`) : réussie.
- Configuration CMake, installation dans un répertoire temporaire et lancement
  `--help` : réussis dans la VM Debian.

## Non exécutés / limites

- Collecte réelle du journal du constructeur : non exécutée afin de ne pas
  enregistrer ni exposer de messages de diagnostic privés.
- Interface graphique, entrée du menu, paquet Debian et intégration ISO : non
  réalisés.
- Les états de services et la collecte journald sont validés avec réponses
  simulées ; aucun service réel n'est déclaré sain ou défaillant par ce rapport.

Le composant est donc une **version source fonctionnelle et testée**, mais il
n'est pas encore intégré à l'ISO Kyronext et ne constitue pas un mécanisme de
réparation ou de restauration.
