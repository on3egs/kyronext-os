# Outils natifs Kyronext

La nouvelle révision est décrite dans [EXTENDED_FEATURES.md](EXTENDED_FEATURES.md) :
onglets, remplacement, lignes et récupération facultative pour Notes ; modes
scientifique/programmeur, conversions et historique pour Calculator.
Les ISO antérieures conservent leurs fonctionnalités d'origine.

Deux applications Python/PyQt6, sans service, réseau, animation ou privilège
administrateur. Elles utilisent les dépendances déjà présentes dans l'ISO.
Les palettes graphite/rouge et graphite/ambre sont sélectionnables ; le choix
est enregistré par utilisateur avec QSettings.

## Calculatrice Kyronext

```bash
python3 /usr/local/bin/kyronext-calculator
python3 /usr/local/bin/kyronext-calculator --theme amber
```

Clavier et boutons, priorités usuelles, parenthèses, signes, virgule ou point
décimal, multiplication et division. `%` calcule le **reste de division**,
pas un pourcentage. Entrée calcule, Échap efface, Ctrl+Maj+C copie le résultat.
Les erreurs n'exécutent aucun code : analyseur arithmétique dédié, expression
limitée à 512 caractères et imbrication à 32 niveaux. Les opérations utilisent
Decimal avec une précision de 28 chiffres significatifs ; les divisions non
finies sont arrondies. Pas de calcul scientifique ou d'historique persistant.

## Bloc-notes Kyronext

```bash
python3 /usr/local/bin/kyronext-notes
python3 /usr/local/bin/kyronext-notes --theme amber mon-texte.txt
```

Nouveau, ouvrir, enregistrer, enregistrer sous ; annuler/rétablir et presse-papiers,
recherche avec retour au début, police, retour à la ligne, position du curseur.
Raccourcis standards Ctrl+N/O/S, Ctrl+Maj+S, Ctrl+F et F3.
La fermeture, l'ouverture et Nouveau demandent quoi faire des modifications.
Annuler un dialogue de sauvegarde conserve le document et interrompt l'action.
Un fichier modifié sur disque depuis l'ouverture demande une confirmation avant
remplacement. Pas d'autosauvegarde ni de synchronisation distante.

Entrée UTF-8 avec ou sans BOM, ouverture limitée à 16 Mio. La sauvegarde produit
du texte UTF-8 sans BOM avec fins de ligne LF. Les autres encodages doivent être
convertis au préalable ; cette version ne prétend pas les détecter.
L'écriture utilise un fichier temporaire dans le même dossier et un remplacement
atomique. Une erreur de permission ou d'écriture est affichée sans vider le texte
en mémoire. Ce mécanisme n'est pas une sauvegarde ni une protection complète
contre les écritures simultanées d'autres programmes.

## Intégration et validation

Lanceurs `.desktop` dans `usr/share/applications`, icônes SVG originales dans
le thème Kyronext, exécutables dans `usr/local/bin`. Les sources sont sous
`iso/config/includes.chroot/`. L'intégration effective dépend de la prochaine
construction d'image ; ces outils ne sont pas rétroactivement présents dans les
ISO déjà livrées.

```bash
python3 applications/utilities/test_utilities.py
python3 applications/utilities/check_ui.py calculator
python3 applications/utilities/check_ui.py notes
```

Ces tests vérifient les calculs, le rejet d'expressions invalides, l'UTF-8 et la
préservation du fichier original lors d'un échec de remplacement. Ils ne prouvent
pas à eux seuls le fonctionnement des dialogues et raccourcis dans le Live.

Les contrôles Qt hors écran ont été exécutés sous Windows : calcul et erreur
affichés, changement de palette, sauvegarde UTF-8, annulation d'Enregistrer sous
et de fermeture, protection face à un changement externe et recherche.
Ils n'ont pas validé les sélecteurs de fichiers interactifs, les raccourcis
clavier ni le lancement depuis une nouvelle ISO. Captures et résultats JSON
dans `logs/kyronext-{calculator,notes}-preview.png` et `*-ui-check.json`.
Le contrôle hors écran lit Segoe UI depuis Windows si nécessaire pour son
rendu ; cette police n'est ni copiée ni distribuée dans Kyronext.

## Version et À propos communs

Les deux applications portent la version **0.1.0**, accessible aussi avec
`--version`. Leur menu **Aide → À propos** ouvre le même dialogue accessible :
logo original, nom de l'application, version, Kyronext Systems — By Manix,
description, statut des créations originales, licences des composants et lien
vers le site officiel.

Les métadonnées et le dialogue sont partagés dans
`iso/config/includes.chroot/usr/lib/kyronext/kyronext_ui.py`, installé dans
`/usr/lib/kyronext/`. Les scripts trouvent ce module à partir de leur propre
arborescence `usr/`, aussi bien depuis les sources qu'après installation.
Ils ne modifient pas `sys.path` et ne dépendent pas d'un chemin utilisateur.

Cet adaptateur **Python/PyQt6 est distinct du framework C++/Qt** de la suite
future. Il ne fournit pas de binding C++ et ne prétend pas mutualiser les
binaires C++. Les conditions GPL/LGPL des composants tiers restent applicables ;
la mention de droits réservés décrit les créations originales et n'accorde
aucune dérogation aux licences de ces composants.

Le contrôle Qt déclenche réellement chaque action À propos, vérifie ses
métadonnées puis ferme le dialogue. Captures supplémentaires :
`logs/kyronext-calculator-about.png` et `logs/kyronext-notes-about.png`.
Ces ajouts concernent la prochaine image ; les ISO précédemment compilées
restent inchangées et ne valident pas cette nouvelle fonction.
