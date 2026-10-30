# Validation

Dans Debian, depuis la racine des sources :
```bash
for f in scripts/*.sh tests/*.sh iso/auto/config iso/config/hooks/live/*; do bash -n "$f" || exit; done
python3 tests/check_sources.py
bash tests/inspect-iso.sh releases/IDENTIFIANT/Kyronext-OS-0.1-amd64.iso
bash scripts/test-iso.sh releases/IDENTIFIANT/Kyronext-OS-0.1-amd64.iso
```
Remplacer IDENTIFIANT par le sous-dossier réellement produit. QEMU utilise OVMF avec variables temporaires, 3 Gio RAM, réseau NAT et aucun disque hôte. TCG ne nécessite pas de virtualisation imbriquée, mais est lent. Pour un test BIOS séparé, retirer les deux options pflash de la commande du script.

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
powershell.exe -NoProfile -ExecutionPolicy Bypass -File G:\kyronext-os\scripts\test-iso-windows.ps1 -IsoPath G:\kyronext-os\releases\20261030T121627Z-1881\Kyronext-OS-0.1-amd64.iso -Mode uefi -Video std
```
Pour BIOS, remplacer uefi par bios. Au menu, sélectionner « Kyronext OS 0.1 Live (amd64) » puis Entrée. Tester un seul mode à la fois sur un hôte limité ; TCG est lent et une longue attente graphique a été observée. Voir les résultats et défauts dans TEST_REPORT.md.

La VM VirtualBox de démonstration, sans disque physique ni disque d'installation, est également enregistrée sous le nom Kyronext-Live-UEFI. Sa session graphique peut être reprise depuis VirtualBox. La fenêtre Python de consultation QEMU est en lecture seule ; la fenêtre VirtualBox est interactive.
