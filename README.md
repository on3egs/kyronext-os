# Kyronext OS 0.1 Developer Preview

**Kyronext Systems - By Manix**

Distribution Linux Live Debian 13 AMD64 avec KDE Plasma, SDDM, X11,
PipeWire et NetworkManager. Version de travail, sans installateur sur disque,
sans persistance et sans IA locale installee.

## Telecharger

Les fichiers sont disponibles dans la release **v0.1-developer-preview** :
ISO, SHA256SUMS, manifeste des paquets et rapport de validation.
L'ISO publique utilise exclusivement les ressources visuelles originales
Kyronext ; aucune illustration de voiture de television n'est incluse.

Image publique : revision 20261030T143045Z-branding-final.
Taille : 1 634 861 056 octets.
SHA-256 : `91d8238d4eb33686b9674981dabf828fafb97f8e9d4d2319b019486e59508c85`.

## Etat reel

BIOS QEMU et UEFI VirtualBox observes jusqu'au bureau personnalise.
Accueil PyQt6, panneau KDE et ouverture de Konsole depuis l'accueil verifies.
Sommes internes et structures ISO hybrides verifiees.
Le test USB rapporte par le createur concerne une autre revision de travail ;
il ne constitue pas une validation materielle de cette image publique.
Secure Boot, installation sur disque, chiffrement et materiel physique non valides.

## Essayer et reconstruire

Consulter BUILD.md, TESTING.md et TEST_REPORT.md. Rufus ou un autre outil
adapte aux ISO hybrides peut ecrire l'image sur une cle USB : ses donnees
seront effacees. Selectionner soigneusement la cle cible.
Ces sources de configuration correspondent a la base publique personnalisee ;
aucune reproductibilite binaire n'est promise. Les mises a jour Debian evoluent.

## Licences

Les logiciels et firmwares Debian conservent leurs licences et fichiers
`/usr/share/doc/*/copyright` dans le Live. Le manifeste packages.txt accompagne
la release. Les sources Debian sont disponibles via les depots officiels
Debian et https://sources.debian.org/ ; les scripts de configuration de ce
depot n'apportent aucun correctif au noyau ou aux binaires Debian.
Les creations Kyronext restent reservees a Kyronext Systems - By Manix,
dans l'attente d'un choix de licence. La publication ne revendique aucun
partenariat avec une serie televisee ni aucun droit sur ses marques.
