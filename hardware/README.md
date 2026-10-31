# Compatibilité matérielle

La cible est Intel/AMD 64 bits pris en charge par le noyau officiel Debian.
La présence d'un pilote ne suffit pas à déclarer un ordinateur compatible.
UEFI et BIOS Legacy doivent être validés séparément, avec l'empreinte exacte
de l'image, le matériel, la date et les preuves dans COMPATIBILITY.csv.
Le registre ne contient actuellement aucune machine déclarée compatible.

Depuis le Live, lancer dans Konsole :

```bash
kyronext-hardware-report > ~/diagnostic-materiel.txt
```

L'outil inventorie CPU, PCI/pilotes, USB, stockage, réseau et audio sans
modifier le matériel. Les messages du noyau peuvent être restreints ; il
le signale. Relire les journaux avant partage : ils peuvent contenir des
identifiants matériels ou des informations personnelles.

Valider affichage, clavier/souris, stockage visible sans écriture, réseau
réellement utilisé, périphériques essentiels, arrêt et redémarrage. Tester
Wi-Fi et Ethernet séparément si présents ; noter l'audio et les limites.
Un résultat QEMU ne valide aucun ordinateur physique, même l'hôte Windows.

Les pilotes libres proviennent du noyau et de Mesa/Xorg Debian. Les firmwares
redistribués proviennent exclusivement de main/non-free-firmware officiels ;
leurs licences restent celles des paquets Debian. Un firmware binaire n'est
pas un installateur automatique de pilote propriétaire. Aucun dépôt tiers,
script constructeur ou téléchargement automatique de pilote n'est ajouté.

X11 est le choix initial KDE de la préversion, avec Mesa logiciel disponible.
Si l'affichage échoue, conserver les journaux avant toute proposition de
changement. Ne pas ajouter systématiquement nomodeset : il désactive des
capacités nécessaires à certaines cartes et ne constitue pas un remède universel.

Un profil Xfce léger séparé est prévu après validation du Live KDE, avec la
même base, les mêmes pilotes et son propre fichier ISO et sa propre matrice
de tests. Il n'est pas encore livré ni validé. L'IA est absente de l'image
0.1 ; l'OS et le futur installateur devront fonctionner sans elle.

Un rapport local est également enregistré automatiquement à l'ouverture du bureau dans ~/.local/state/kyronext/hardware/. Il n'est jamais transmis. Le gestionnaire avancé indépendant est dans hardware/legacy-driver-manager/.

## Acer Aspire 7004 WSMi

Les fiches disponibles pour ce modèle indiquent souvent une NVIDIA GeForce Go
6100 et, selon la révision, une Broadcom BCM94311MCG/BCM4311. Kyronext apporte
le pilote libre Nouveau avec le noyau et X.Org, ainsi que Mesa. Aucun ancien
pilote NVIDIA 304/340/390 n'est installé : ces branches ne sont pas ajoutées
automatiquement aux noyaux Debian actuels.

Pour la Broadcom, le pilote noyau `b43` est présent, mais la carte BCM4311 a
besoin d'un microcode Broadcom que Debian ne redistribue pas dans l'ISO. Le
centre « Pilotes & connexion » affiche une action confirmée si le noyau trouve
un module `b43`/`b43legacy`. Avec Ethernet ou un partage USB, cette action
installe le paquet Debian `firmware-b43-installer` depuis `contrib`; un
redémarrage peut être nécessaire. L'ISO n'embarque pas le binaire propriétaire.

Steam et ses bibliothèques i386 sont installés dans l'image, mais le premier
lancement télécharge le client de Valve et nécessite Internet. La Go 6100
précède les GPU pris en charge par Vulkan; les jeux récents qui exigent Vulkan
ou des fonctions OpenGL modernes ne sont donc pas garantis et peuvent ne pas
démarrer. Vérifier `lspci -nnk` sur la machine précise : les cartes varient
entre révisions, et le succès en VM ne valide pas l'Acer physique.
