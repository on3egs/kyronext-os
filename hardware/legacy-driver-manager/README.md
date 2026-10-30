# Kyronext Legacy Driver Manager

Prototype indépendant, non intégré comme gestionnaire de pilotes dans l'ISO 0.1.
Il fonctionne hors ligne, sans privilèges, avec Python standard et les outils
Debian pciutils, usbutils et kmod. Le journal noyau peut être inaccessible.

```bash
python3 hardware/legacy-driver-manager/diagnose.py --output rapport-materiel.json
```

Le rapport conserve les identifiants PCI/USB, pilote lié, modalias, modules
correspondant aux alias du noyau installé, métadonnées firmware et erreurs
accessibles. Les rapports ne sont pas envoyés sur Internet. Relire avant partage.
Les modules disponibles sont des candidats, jamais une preuve de compatibilité.
Une liste modinfo contient aussi des firmwares optionnels ou destinés à
d'autres variantes : « absent localement » n'est pas « nécessaire et manquant ».

La détection et le chargement normal restent assurés par le noyau et udev.
Le prototype n'appelle ni modprobe pour charger un module, ni APT, ni DKMS.
Il ne télécharge aucun pilote et ne propose aucune correction non validée.

## Versions suivantes

Ajouter une interface lisant le rapport et une base de problèmes sourcée par
identifiants exacts et versions de noyau. Toute alternative devra être testée
sur la machine cible, avec capture de l'état initial, autorisation explicite,
modification réversible et retour au profil d'origine. Pas de blacklist,
changement de noyau ou conflit provoqué automatiquement.

NVIDIA 304, 340 et 390 : installation automatique interdite. Nouveau reste le
candidat libre lorsqu'il fonctionne. Aucun pilote Windows .sys, dépôt tiers
ou mélange de versions Debian. La présence de firmwares binaires officiels
doit être documentée par le manifeste et les licences des paquets.

## Mode Legacy Compatibility prévu

Préparer après validation du Live standard une entrée de démarrage séparée
« Kyronext OS — Legacy Compatibility », X11, animations et composition
désactivées, rendu Mesa logiciel et journaux conservés. La tester séparément
en BIOS et UEFI avant activation dans une ISO publiée.

Une entrée de diagnostic nomodeset sera distincte et facultative : elle
peut supprimer l'affichage sur certaines cartes et ne sera pas la valeur
par défaut. Xfce est prévu pour une édition légère ultérieure. Aucun de ces
modes supplémentaires n'est actuellement annoncé comme disponible ou testé.
