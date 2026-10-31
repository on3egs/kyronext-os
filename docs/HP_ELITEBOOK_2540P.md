# Profil matériel cible — HP EliteBook 2540p

État : **compatible selon la documentation, non testé sur la machine physique**.
Le créateur indique que Kyronext OS doit être essayé sur un EliteBook 2540p ;
le modèle de la carte Wi-Fi, la mémoire installée et les résultats de démarrage
de cet exemplaire ne sont pas encore relevés.

## Matériel annoncé par HP

Les configurations comprennent des processeurs Intel Core i5/i7 de première
génération, l'Intel HD Graphics intégrée, un contrôleur Ethernet Gigabit, une
baie SATA, deux emplacements mémoire DDR3 pouvant accueillir jusqu'à 8 Gio,
ainsi que selon les options un module Wi-Fi Intel Centrino Advanced-N 6200 ou
Broadcom. L'écran 12,1 pouces est WXGA. Les options de carte Wi-Fi varient selon
la référence régionale ; vérifier l'identifiant PCI de l'unité réelle.

Debian amd64 prend en charge les processeurs Intel 64. L'Intel HD Graphics
ancienne génération utilise le pilote noyau `i915` et le pilote graphique
libre Mesa lorsqu'ils sont disponibles. L'inventaire de l'ISO de travail
confirme `firmware-iwlwifi`, `firmware-intel-graphics` et `intel-microcode`.
Le Wi-Fi Intel peut utiliser `iwlwifi`. Broadcom ne reçoit aucun pilote
propriétaire ou firmware téléchargé automatiquement ; le module exact doit
d'abord être identifié.

Sources : [HP EliteBook 2540p QuickSpecs](https://cdn.cnetcontent.com/c5/fb/c5fb014f-7955-43a8-ac95-0d698db8de79.pdf), [architecture amd64 Debian](https://www.debian.org/ports/amd64/), [guide d'installation Debian 13](https://www.debian.org/releases/stable/amd64/).

## Validation requise sur cet exemplaire

1. Démarrer sur la clé en mode BIOS/Legacy puis, si le firmware le propose,
   répéter séparément en UEFI. Ne lancer aucun installateur.
2. Vérifier l'affichage interne, le clavier, le pavé tactile et le TrackPoint.
3. Vérifier Ethernet, Wi-Fi, audio, webcam éventuelle, Bluetooth et veille.
4. Consulter le passeport matériel et relever modèle, RAM, identifiant PCI Wi-Fi,
   pilote lié et messages de firmware manquant.
5. Vérifier l'arrêt et le redémarrage depuis le Live.

N'installer sur le disque interne qu'après une validation indépendante et une
sauvegarde. L'essai Live n'écrit pas sur ce disque.

## Ergonomie conseillée

- Profil **Legacy Compatibility** : animations désactivées, session X11
  disponible et résolution native de l'écran détectée sans exiger d'accélération
  3D moderne.
- Un SSD SATA et 8 Gio de mémoire améliorent probablement la réactivité, mais
  ce sont des conseils matériels, pas des prérequis Kyronext vérifiés.
- Les applications IA locales, le navigateur experimental et le plein écran
  multimédia ne sont pas recommandés comme fonctions principales sur ce modèle.
