# Audit de l'hôte et constructeur

Audit du 30 octobre 2026 : Windows 11 Pro AMD64, Intel i7-4710HQ,
15,9 Gio RAM. C: environ 7,3 Gio libres, G: environ 115,6 Gio libres.
Le dépôt indépendant est sur G:. Les sources Kyronext existantes ont été
consultées en lecture seule ; leur intégration est différée.

Après autorisation explicite du propriétaire, VirtualBox 7.2.22 a été
installé via winget depuis Oracle. Aucun Extension Pack ajouté.
L'image Debian generic AMD64 officielle a été téléchargée sur HTTPS et
comparée à SHA512SUMS du serveur Debian (empreinte concordante).
La VM utilise un disque virtuel dédié ; aucun accès brut aux disques hôtes.
Les clés de développement et images restent dans build/, exclu de Git.
Le WSL NVIDIA reste intact. Le réseau VM est en NAT, avec accès SSH prévu
uniquement sur 127.0.0.1:2222 ; aucun service invité exposé sur le LAN.

La prise en charge matérielle est détectée par VirtualBox ; un démarrage
réussi reste nécessaire pour valider son fonctionnement effectif.

Mise à jour de validation : le constructeur a produit l'ISO puis a été arrêté proprement. Après incident des tests QEMU imbriqués, sa configuration a été ramenée à 1 vCPU avec paravirtualisation désactivée. QEMU Windows 11.1.0 est extrait en mode portable dans build/tools/qemu depuis le téléchargement winget vérifié ; l'installation avec élévation n'a pas abouti. Les fichiers OVMF proviennent du paquet Debian installé dans le constructeur. Les bureaux Live ont été observés en BIOS/UEFI QEMU VGA standard et dans VirtualBox UEFI VMSVGA. La VM de démonstration VirtualBox est sauvegardée dans build/vbox-live/.
