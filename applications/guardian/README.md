# Kyronext Guardian — Hardware Passport 0.1

Module avancé indépendant fourni dans les sources. Il ne constitue pas une
interface graphique Guardian et n'est pas embarqué dans l'ISO actuellement
en construction. L'ISO 0.1 contient déjà le diagnostic texte
`kyronext-hardware-report`, lancé localement à l'ouverture de la session.

Depuis la racine des sources, sous Linux, sans sudo :

```bash
python3 applications/guardian/hardware_passport.py --output "$HOME/hardware-passport.json"
```

Le module réutilise `hardware/legacy-driver-manager/diagnose.py` et Python
standard. Les outils Debian pciutils, usbutils et kmod enrichissent son
inventaire ; leur absence ou les droits restreints sont conservés dans le
rapport. Aucun accès Internet, téléchargement, installation de pilote,
changement système ou action d'administration n'est déclenché.

Le JSON privé est créé avec permissions 0600 et refuse d'écraser un fichier
ou un lien déjà présent. Le répertoire de destination doit déjà exister.
Fabricant/modèle, CPU/RAM, noyau et mode de démarrage sont accompagnés des
identifiants PCI/USB, pilotes liés, modules candidats et messages accessibles.
Numéro de série DMI et identifiant unique de machine ne sont pas collectés
directement ; les journaux doivent néanmoins être relus avant partage.

Tous les états de compatibilité restent **non vérifiés**. Un inventaire
effectué sous Linux n'est pas une validation du démarrage, de l'affichage,
du réseau ou des périphériques essentiels. Le passeport ne rejoint pas
automatiquement la base des matériels compatibles.

Voir `docs/GUARDIAN.md` pour l'architecture future et les limites exactes.
