# Installateur expérimental Kyronext

Le profil `calamares-live` configure une ISO distincte de la préversion Live 0.1. La même clé USB est prévue pour proposer l’installateur Debian en mode texte au démarrage et Calamares depuis un bouton sur l’écran d’accueil ou le menu des applications KDE. Le diaporama Calamares est configuré avec de vraies captures du bureau et d’AMP et les textes de Manix pendant les opérations d’installation. Le mode texte reste un parcours Debian Installer, sans photos.

Les réglages Calamares et le diaporama Kyronext sont copiés par un hook après l’installation des paquets Debian dans la session Live. Les lanceurs sont des surcharges locales dans `/usr/local/share/applications`, ce qui évite de remplacer des fichiers détenus par les paquets Debian.

Pour produire cette image dans le constructeur Debian 13, il faut au moins 30 Gio libres dans le répertoire de compilation :

```sh
sudo env KYRONEXT_INSTALLER_PROFILE=calamares-live bash scripts/build-iso.sh
```

Le lanceur graphique n’accorde à l’utilisateur Live un droit `sudo` sans mot de passe que pour le binaire Calamares, sans argument. Le seul paquet local temporaire est le lanceur Kyronext ; les réglages et les illustrations restent dans la session Live. La page de résumé demande une confirmation avant les écritures.

Cette configuration est préparée, mais l’ISO complète n’a pas encore été construite ni installée sur disque virtuel. Le constructeur VirtualBox dédié reste sous le seuil de 30 Gio. Une compilation isolée dans un chroot Debian 13 sur ext4 est en cours de préparation ; elle ne modifie pas l’image du constructeur ni les builds déjà archivés.

Le profil complet inclut la suite Kyronext native, Steam et ses bibliothèques 32/64 bits, ainsi que les pilotes matériels Debian retenus. Le paquet Steam est présent hors ligne, mais son client Valve se télécharge au premier lancement. Le microcode Broadcom b43 reste à télécharger après connexion réseau depuis Debian `contrib`.

## Aperçu graphique et TTY

`preview.py` reste une maquette visuelle séparée ; sa barre mesure la progression de la présentation et ne modifie aucun disque :

```sh
python3 installer/preview.py
python3 installer/preview.py --tty
```

Les images sont dans `assets/`, le texte de la maquette est dans `slides-fr.json`, et le diaporama réel de Calamares se trouve dans `calamares/branding/kyronext/show.qml`. Aucun lien de don n’est affiché : il sera ajouté lorsqu’il existera.

Avant toute distribution, il reste à construire l’image, l’ouvrir en VM UEFI et BIOS avec des disques virtuels jetables, effectuer une installation complète, vérifier le démarrage installé et confirmer que les outils d’installation et les droits temporaires ont disparu du système cible.
