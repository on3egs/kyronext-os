# Compiler dans une VM Debian 13 AMD64

Le constructeur Debian isolé de cette session a 4 Gio RAM, 3 vCPU et un disque virtuel ext4 dynamique de 69 Gio sur G:. La VM dispose de 2 Gio de swap. Le plafond cgroup `MemoryMax=16G` du service de compilation ne change pas la RAM réellement allouée à la VM. Prévoir 30 Gio libres avant une reconstruction complète ; vérifier l'espace réel avant chaque lancement. Aucun disque physique n'est attaché ; ne pas utiliser le WSL JetPack.

Dans la VM uniquement :
```bash
sudo apt update
sudo apt install git live-build debootstrap xorriso squashfs-tools isolinux syslinux-common grub-efi-amd64-bin mtools dosfstools qemu-system-x86 ovmf python3
sudo apt install build-essential debhelper pkgconf systemd-dev curl openssh-client
```
Copier les sources sur ext4 puis, depuis leur racine :
Les ajouts SSH, pilotes, Gobi 2000, Codex CLI et les plafonds sont décrits dans
[SSH_PILOTES_CODEX.md](docs/SSH_PILOTES_CODEX.md). Ils nécessitent une nouvelle
construction complète ; un remaster visuel n'installe pas ces paquets.
```bash
bash scripts/check-environment.sh
python3 tests/check_sources.py
sudo bash scripts/build-iso.sh
```
La construction de la suite native nécessite aussi `g++ cmake ninja-build dpkg-dev qt6-base-dev qt6-multimedia-dev qt6-webengine-dev libkf6kio-dev libkf6archive-dev`. Construire puis empaqueter les applications ainsi :

```bash
bash scripts/build-applications.sh
bash scripts/package-applications.sh
sudo env KYRONEXT_SUITE_DEB="$PWD/build/kyronext-suite-preview_0.1.0-0~preview3_amd64.deb" bash scripts/build-iso.sh
```

Pour une préversion autonome avec compilation, CTest sous D-Bus, paquet Debian
et SHA-256 dans un nouveau sous-dossier `releases/`, utiliser à la place :

```bash
bash scripts/build-suite-package.sh
```

Le chemin du paquet est affiché à la fin. Cette commande ne modifie pas l'ISO
existante. Le paquet contient Kyronext Navigator ; Debian ne fournit pas le
suivi de sécurité de `qt6-webengine`, donc Navigator reste optionnel et ne doit
pas devenir navigateur par défaut ni être présenté comme prêt pour les usages
sensibles. Ne l'intègre à une candidate publique qu'après décision et validation
de cette limitation.

## Image installable expérimentale

La configuration `installer/` ajoute un profil distinct à la préversion Live :
`KYRONEXT_INSTALLER_PROFILE=calamares-live`. Il active Debian Installer dans le
menu de démarrage, ajoute un bouton Calamares à l'écran d'accueil Live avec ses
photos/textes, et laisse l'ISO standard sans installateur. Le mode texte
n'affiche pas de photos.

Dans la VM Debian, depuis une copie à jour du dépôt sur ext4 et après avoir
libéré au moins 30 Gio :

```bash
sudo env KYRONEXT_INSTALLER_PROFILE=calamares-live bash scripts/build-iso.sh
```

La sortie portera le nom `Kyronext-OS-0.2-experimental-installer-amd64.iso`.
Cette image reste expérimentale. Elle n'a pas été reconstruite ni validée sur
disque virtuel. Avant tout usage réel, terminer les essais BIOS/UEFI, créer un
utilisateur, installer puis redémarrer, et vérifier la suppression des outils
de Live et des droits de lancement Calamares dans le système installé. Ne pas
installer cette image sur un disque physique avant ces validations.

## Identifiants du compte Live

Par défaut, le mot de passe du compte Live est verrouillé : l’autologin graphique reste prévu, mais `sudo` ne peut pas être utilisé. Le hook ne contient aucun hash de mot de passe. Une image destinée à être publique doit aussi être construite sans `KYRONEXT_SSH_PUBLIC_KEY`.

Pour une image privée, générer le hash à l’invite masquée et le garder dans `build/` (ce répertoire est ignoré par Git) :

```bash
./scripts/create-live-password-hash.sh
sudo env KYRONEXT_LIVE_PASSWORD_HASH_FILE="$PWD/build/live-password.hash" bash scripts/build-iso.sh
```

Le hash n’est accepté que depuis un fichier régulier en mode `0600`, puis l’image est marquée privée. Ne jamais enregistrer le mot de passe ou son hash dans les sources. L’historique Git local contient une ancienne version du hash de test : ne publie pas l’historique de ce dépôt avant son nettoyage. L’ISO privée déjà construite reste inchangée et contient ses anciens accès ; ne la distribue pas.

Pour une candidate sans accès privé, laisser les deux variables absentes :

```bash
sudo env -u KYRONEXT_SSH_PUBLIC_KEY -u KYRONEXT_LIVE_PASSWORD_HASH_FILE bash scripts/build-iso.sh
```

Cette ISO Live n’a pas d’installateur activé ; le compte verrouillé peut servir à vérifier l’interface, pas à administrer le système. Les réglages concernent uniquement le compte Live, pas un compte créé par un futur installateur.

Les répertoires de construction sont uniques ; aucun nettoyage destructif automatique. Les recommandations APT restent désactivées ; le profil installateur ajoute explicitement ses paquets. Les signatures APT restent activées. Chaque succès produit dans releases/IDENTIFIANT/ une ISO, SHA256SUMS, packages.txt, build-tools.txt et BUILD_REPORT.md. Le journal est dans logs/. Prévoir au moins 30 Gio libres ; une limite supérieure peut être fixée avec `KYRONEXT_MIN_FREE_KIB`.

Les versions APT évoluent : aucune reproductibilité octet pour octet promise. Le succès de compilation ne valide pas le bureau. Suivre TESTING.md. Sous Windows, le script utilise WHPX par défaut (`-Acceleration whpx`) ; les essais TCG sur cet hôte ont été fortement ralentis et n'ont pas atteint le bureau. On peut demander explicitement `-Acceleration tcg` pour diagnostiquer un autre environnement. `-HostSshPort 2249` ouvre un transfert SSH uniquement sur `127.0.0.1` pour l'inspection, si l'image possède une clé publique autorisée.

Le contrôleur de budget refuse une ISO vide ou d'au moins 16 Go décimaux (16 000 000 000 octets) et limite les firmwares/pilotes ajoutés à moins de 1 Go. Le plafond de 16 Go est une borne de sécurité de build, pas une taille cible.

Le constructeur créé pendant cette session est accessible depuis Windows avec scripts/connect-builder.ps1. Les sources Linux sont dans /home/builder/kyronext-suite-iso. Le disque virtuel et la clé SSH restent dans build/builder/. L'arrêt propre s'effectue avec scripts/connect-builder.ps1 -Command 'sudo poweroff'.

Si Windows bloque le helper PowerShell, utiliser une exception limitée au processus, sans modifier la politique persistante :
```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File G:\kyronext-os\scripts\connect-builder.ps1
```


## Révision visuelle depuis une ISO existante

Dans la VM Debian 13, depuis les sources à jour :

```bash
sudo bash scripts/remaster-branding.sh /home/builder/kyronext-os/releases/20261030T121627Z-1881/Kyronext-OS-0.1-amd64.iso
```

Le fichier SHA256SUMS et packages.txt doivent accompagner l'image source.
Ce chemin rapide conserve les paquets, le noyau et les initrd de la base ;
il remplace les ressources Kyronext et recompose le SquashFS en zstd.
Il conserve les entrées BIOS/UEFI, le MBR hybride et Joliet, puis les contrôle.
Il ne remplace pas une reconstruction live-build pour changer les paquets.
Les sorties sont dans un nouveau releases/*-branding-*/ et le journal dans logs/.
