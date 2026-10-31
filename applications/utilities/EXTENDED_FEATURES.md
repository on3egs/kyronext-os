# Notes et Calculator — révision étendue

Kyronext Systems — By Manix. Applications natives PyQt6, version 0.1.0.
Les changements de cette révision doivent être intégrés dans une nouvelle ISO
avant d'être considérés disponibles dans le Live distribué.

## Notes

- Documents indépendants en onglets, limite de 32 ; Ctrl+N crée un onglet et
  Ctrl+W le ferme après confirmation des modifications. La fermeture de la
  fenêtre vérifie tous les documents et peut être annulée sans les supprimer.
- Numéros de lignes, recherche et remplacement de toutes les occurrences
  dans l'onglet actif avec confirmation. Un remplacement peut être annulé.
- Détection légère des changements sur disque toutes les 2,5 secondes, sans
  relire continuellement les fichiers. Le texte en mémoire est conservé ; la
  sauvegarde compare toujours l'empreinte complète avant de remplacer le fichier.
- Récupération locale facultative dans Affichage : brouillons toutes les
  30 secondes, dans le répertoire de données utilisateur Qt, sous `recovery/`.
  Le menu Fichier permet de les récupérer en nouveaux documents non enregistrés.
  Aucun remplacement automatique du fichier original et aucun envoi réseau.
- Enregistrer ou fermer volontairement un document supprime son brouillon.
  Désactiver la récupération arrête les copies et retire les brouillons des
  onglets ouverts ; des brouillons d'une ancienne interruption peuvent rester.
  La récupération est une aide, pas une garantie contre toute perte de données.
- Fichiers UTF-8 jusqu'à 16 Mio ; sauvegarde atomique avec fins de ligne LF.
  Le Live sans persistance perd aussi ses brouillons lors du redémarrage.

Markdown, coloration syntaxique, comparaison de fichiers et assistant IA restent
à développer. Aucun bouton ne les présente comme disponibles.

## Calculator

- Standard : calculs décimaux à 28 chiffres significatifs, parenthèses et
  priorités. `%` est le reste de division, pas un pourcentage commercial.
- Scientifique : `sqrt`, `ln`, `log10`, `exp`, `abs`, fonctions trigonométriques
  et inverses en radians, constantes `pi` et `e`, puissance `**` entière bornée.
  Les fonctions trigonométriques et constantes utilisent la précision de `math`
  Python (environ 15 chiffres), pas 28 chiffres exacts.
- Programmation : entiers décimaux, hexadécimaux `0xff`, binaires `0b1010`,
  opérations `& | ^ ~ << >>`, division entière `//`. Résultats bornés à 4096 bits,
  décalages à 1024 et exposants à 1000. Les signes suivent les entiers Python ;
  aucun débordement 8/16/32/64 bits implicite. Hexadécimal et binaire sont affichés.
- Conversions : longueur, masse, durée, octets SI/IEC et températures. Le zéro
  absolu est contrôlé ; les résultats sont arrondis à 28 chiffres significatifs.
- Historique local des 100 derniers calculs, effaçable ; double-clic pour
  reprendre le calcul, son mode et les unités de conversion.

L'analyse syntaxique autorise uniquement les nombres, opérations et fonctions
documentés. Il n'y a aucun `eval`, import dynamique fourni par l'expression,
accès à un attribut ou exécution de commande. Les limites sont appliquées à
la taille, l'imbrication et aux résultats.

## Vérifications exécutables

```bash
python3 applications/utilities/test_utilities.py
python3 applications/utilities/test_extended.py
python3 applications/utilities/check_ui.py calculator
python3 applications/utilities/check_ui.py notes
```

Tests Qt hors écran : modes, historique des unités, onglets, remplacement et
annulation, détection des changements externes, récupération de brouillon,
confirmations et À propos. Les dialogues de fichiers interactifs et les
raccourcis doivent également être vérifiés dans la prochaine ISO.
