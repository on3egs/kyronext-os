# Compiler dans une VM Debian 13 AMD64

Prévoir 6 Gio RAM, 4 vCPU et un disque virtuel ext4 de 70 Gio sur G:. Aucun disque physique attaché. Ne pas utiliser le WSL JetPack.

Dans la VM uniquement :
```bash
sudo apt update
sudo apt install git live-build debootstrap xorriso squashfs-tools isolinux syslinux-common grub-efi-amd64-bin mtools dosfstools qemu-system-x86 ovmf python3
```
Copier les sources sur ext4 puis, depuis leur racine :
```bash
bash scripts/check-environment.sh
python3 tests/check_sources.py
sudo bash scripts/build-iso.sh
```
Les répertoires de construction sont uniques ; aucun nettoyage destructif automatique. Les recommandations APT sont désactivées pour éviter Calamares. Les signatures APT restent activées. Chaque succès produit dans releases/IDENTIFIANT/ une ISO, SHA256SUMS, packages.txt, build-tools.txt et BUILD_REPORT.md. Le journal est dans logs/.

Les versions APT évoluent : aucune reproductibilité octet pour octet promise. Le succès de compilation ne valide pas le bureau. Suivre TESTING.md.

Le constructeur créé pendant cette session est accessible depuis Windows avec scripts/connect-builder.ps1. Les sources Linux sont dans /home/builder/kyronext-os. Le disque virtuel et la clé SSH restent dans build/builder/. L'arrêt propre s'effectue avec scripts/connect-builder.ps1 -Command 'sudo poweroff'.

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
