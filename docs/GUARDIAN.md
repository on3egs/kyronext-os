# Kyronext Guardian : architecture et état réel

Guardian regroupera des fonctions de diagnostic et d'assistance, sans
privilèges administrateur permanents ni télémétrie imposée. Le bureau doit
continuer de fonctionner sans Guardian et sans IA locale.

## Hardware Passport — prototype 0.1 fourni

`applications/guardian/hardware_passport.py` produit un inventaire JSON
hors ligne, en réutilisant le Legacy Driver Manager indépendant. Il relève
fabricant/modèle lorsqu'accessibles, CPU/RAM, noyau, mode de démarrage,
identifiants PCI/USB, pilotes liés, modules candidats par modalias,
métadonnées firmware et messages noyau accessibles. Les accès restreints
ne déclenchent pas une élévation de privilèges automatique.

Le rapport reste local, créé sans écrasement avec permissions 0600.
Tous les tests fonctionnels sont marqués **non vérifiés**. Une simple
détection ne peut alimenter la catégorie « compatible et testé » de
HARDWARE_COMPATIBILITY.md. Avant de partager, relire les journaux qui peuvent
contenir des identifiants ou des informations personnelles.

Ce module avancé est fourni séparément avec les sources. **L'ISO actuelle
n'intègre pas une GUI Guardian.** Elle inclut le diagnostic texte
`kyronext-hardware-report` et sa sauvegarde locale automatique dans
`~/.local/state/kyronext/hardware/`. Ces fonctions ne sont pas un gestionnaire
automatique de réparation ou de pilotes.

## Adaptive OS — recommandations manuelles futures

Prévoir une interface lisant le passeport et affichant des recommandations
explicables, liées aux identifiants exacts et à des essais documentés.
Le choix d'un profil léger, X11, animations réduites ou édition Xfce doit
rester manuel. Aucun réglage, blacklist, installation de pilote ou changement
de noyau automatique n'est implémenté. Une compatibilité supposée ne suffit
jamais à proposer une action comme sûre ou opérationnelle.

## Phoenix Recovery — fonction future

Concevoir ultérieurement des points de restauration et procédures de retour
arrière, avec cible explicite, état initial conservé et tests uniquement sur
disques virtuels jetables avant toute disponibilité publique. Cette version
ne crée ni instantané, ni sauvegarde, ni partition de récupération et ne peut
promettre une restauration du système. Aucune écriture sur disque physique.

## AI Doctor — fonction future, facultative

Une future IA locale pourra expliquer un rapport choisi par l'utilisateur.
Elle devra distinguer faits, hypothèses et actions proposées, fonctionner
sans rôle administrateur et n'exécuter aucune action système directement.
Toute opération sensible devra passer par une couche de permissions dédiée,
une autorisation explicite et une procédure réversible réellement testée.
Aucun moteur IA, modèle ou accès distant n'est fourni ici.

## Validation du module

Sous Debian depuis la racine des sources :

```bash
python3 applications/guardian/test_hardware_passport.py
python3 applications/guardian/hardware_passport.py --output "$HOME/hardware-passport.json"
stat -c '%a' "$HOME/hardware-passport.json"
```

La dernière commande doit indiquer 600. Réutiliser le même chemin doit être
refusé ; choisir un nouveau nom pour une autre observation. Les résultats
réels des essais restent consignés dans logs/, séparément des sources.
