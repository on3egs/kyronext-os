# Validation

Dans Debian, depuis la racine des sources :
```bash
for f in scripts/*.sh tests/*.sh iso/auto/config iso/config/hooks/live/*; do bash -n "$f" || exit; done
python3 tests/check_sources.py
bash tests/inspect-iso.sh releases/IDENTIFIANT/Kyronext-OS-0.1-amd64.iso
bash scripts/test-iso.sh releases/IDENTIFIANT/Kyronext-OS-0.1-amd64.iso
```
Remplacer IDENTIFIANT par le sous-dossier réellement produit. QEMU utilise OVMF avec variables temporaires, 3 Gio RAM, réseau NAT et aucun disque hôte. Sous Windows, WHPX est le choix par défaut et doit être disponible dans QEMU/Windows. TCG est un mode logiciel de diagnostic ; sur le PC Windows de validation il ne finit pas d'ouvrir le bureau. Pour un test BIOS séparé, utiliser `-Mode bios` dans le script Windows ; il n'attache aucun disque.

Conserver captures, configuration VM, journaux et observations dans logs/ :
1. Menu UEFI et session Plasma avec panneau et menu fonctionnels.
2. Accueil Kyronext et fond original.
3. Clavier et souris utilisables, ouverture de Dolphin et Konsole.
4. Vérifier `cat /etc/os-release`, `id`, `uname -r`, `systemctl --failed`.
5. Session non root, aucun sudo sans mot de passe ; réseau NetworkManager et audio PipeWire lorsque disponible.
6. Désactiver les effets dans les paramètres KDE et vérifier le bureau.
7. Arrêter, redémarrer, puis tester un démarrage à froid.

Un lancement QEMU ou une inspection SquashFS ne valide pas le bureau. Marquer les vérifications non observées « non exécutées ». Secure Boot non validé.

Les modes sont désormais sélectionnables avec un second argument : uefi (défaut) ou bios. Sans affichage graphique, le script démarre QEMU sans fenêtre et expose un socket QMP local dans logs/test-MODE-DATE/. Le client tests/qmp.py permet une capture PNG. Les tests utilisent explicitement le processeur émulé qemu64 ; ce test virtuel ne valide pas un ordinateur physique ancien.

## Essai direct depuis Windows

Le profil validé emploie VGA standard. QEMU portable et OVMF sont déjà disponibles dans build/ sur cette machine.
```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File G:\kyronext-os\scripts\test-iso-windows.ps1 -IsoPath G:\kyronext-os\releases\20261031T175731-clean-live\Kyronext-OS-0.1-amd64.iso -Mode uefi -Video std
```
Pour BIOS, remplacer `-Mode uefi` par `-Mode bios`. Au menu, sélectionner « Kyronext OS 0.1 Live (amd64) » puis Entrée via QMP. Sur la candidate actuelle, WHPX a affiché le bureau et l'accueil en BIOS et UEFI ; TCG a affiché un écran brouillé ou noir et n'a pas atteint le bureau durant les observations. Le mode `-HostSshPort 2249` est réservé à une image privée avec clé autorisée et reste lié à `127.0.0.1`. Voir TEST_REPORT.md.

La VM VirtualBox de démonstration, sans disque physique ni disque d'installation, est également enregistrée sous le nom Kyronext-Live-UEFI. Sa session graphique peut être reprise depuis VirtualBox. La fenêtre Python de consultation QEMU est en lecture seule ; la fenêtre VirtualBox est interactive.
