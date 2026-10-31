# Preuves de validation QEMU

Cette procédure prépare des preuves. Elle ne déclare aucun démarrage réussi.
À exécuter dans le constructeur Debian, depuis la racine des sources,
uniquement après fabrication et vérification de l'ISO.

## Deux démarrages distincts

Remplacer IDENTIFIANT par le sous-dossier réellement produit :

```bash
iso=$(realpath releases/IDENTIFIANT/Kyronext-OS-0.1-amd64.iso)
bash tests/inspect-iso.sh "$iso"
bash scripts/test-iso.sh "$iso" uefi
```

Le script laisse QEMU au premier plan. Utiliser un second terminal pour les
captures. Le mode UEFI utilise OVMF avec une copie de ses variables dans le
répertoire de traces. Après arrêt propre de ce test, recommencer séparément :

```bash
bash scripts/test-iso.sh "$iso" bios
```

Le mode BIOS utilise le firmware BIOS fourni par QEMU, sans les lecteurs
pflash OVMF. Le résultat d'un mode ne valide pas l'autre.

Les premiers essais imbriqués utilisaient q35, CPU émulé qemu64, deux vCPU,
TCG, graphique VirtIO et réseau utilisateur/NAT VirtIO. Le script actuel
emploie un vCPU, TCG à thread unique (ou KVM si disponible) et une carte
VGA standard par défaut ; `KYRONEXT_TEST_VIDEO=virtio` permet un comparatif.
La RAM est de
3 Gio par défaut, modifiable avec KYRONEXT_TEST_RAM exprimée en Mio.
Aucun disque de l'hôte n'est attaché. Sans affichage graphique, QEMU tourne
sans fenêtre et reste contrôlable par le socket QMP local.

## Captures et commandes QMP

Dans un second terminal, sélectionner le répertoire affiché par le script.
Exemple pour le dernier test UEFI :

```bash
run=$(find logs -maxdepth 1 -type d -name 'test-uefi-*' | sort | tail -n 1)
run=$(realpath "$run")
python3 tests/qmp.py "$run/qmp.sock" status
python3 tests/qmp.py "$run/qmp.sock" screenshot "$run/01-ecran-observe.png"
```

Le chemin du PNG doit être absolu : il est interprété par le processus QEMU.
Le fichier est créé dans le constructeur, pas directement dans Windows.
Pour un test BIOS, employer `test-bios-*` à la place de `test-uefi-*`.

Le moniteur peut injecter une touche pour tester une interaction :

```bash
python3 tests/qmp.py "$run/qmp.sock" monitor 'sendkey ret'
```

Ne pas supposer que les codes de touches produisent le même texte sur
clavier français et clavier américain. Observer les résultats avant de
continuer. Ne pas envoyer de commandes d'installation ou de gestion de disque.

Pour demander un arrêt ACPI :

```bash
python3 tests/qmp.py "$run/qmp.sock" shutdown
```

Une demande d'arrêt acceptée par QMP n'est pas une preuve d'arrêt du système.
Observer que QEMU termine proprement. Tester également l'arrêt depuis KDE.

## Ce qu'il faut conserver

- SHA-256 de l'ISO et manifeste de paquets de la fabrication.
- Mode UEFI/BIOS, version QEMU, version OVMF le cas échéant, RAM et configuration.
- Captures du menu de démarrage, du bureau, de Konsole et de Dolphin.
- Résultats visibles de `id`, `cat /etc/os-release`, `uname -r` et `systemctl --failed`.
- Observations du clavier, de la souris, du réseau et des arrêts/redémarrages.
- `serial.log` lorsqu'il contient des informations ; il peut rester vide si
  la session n'a pas de console série configurée.

Une capture du bureau peut prouver son affichage à un instant donné ; elle
ne valide pas à elle seule le réseau, l'audio, le clavier ou le redémarrage.
QMP `status: running` prouve seulement que l'émulateur exécute la machine.
Le profil qemu64 et les périphériques virtuels ne valident aucun ordinateur
physique ancien. Les résultats matériels physiques restent dans leur propre
matrice avec identifiants PCI/USB et preuves.

## Incident observé le 30 octobre 2026

La construction de l'ISO a réussi. Les premiers tests UEFI et BIOS ont ensuite
été lancés simultanément dans le constructeur VirtualBox de 6 Gio, avec QEMU
TCG, deux vCPU et 2 Gio de RAM par test.

Le menu BIOS a été réellement affiché et capturé dans
`logs/bios-01-initial.png`. Les libellés des entrées affichent Kyronext OS ;
le fond stock contient encore une mention Debian et constitue une limite
visuelle distincte. Une touche Entrée a été envoyée pour démarrer le Live.

Le constructeur est ensuite devenu inaccessible par SSH/QMP. Son journal série
contient des avertissements `soft lockup` concernant un processus QEMU et un
long intervalle du contrôle d'horloge. Une seconde capture BIOS n'a pas abouti.
La trace a été préservée dans
`logs/builder-softlockup-during-parallel-boot.txt`.

Une tentative d'arrêt temporaire des seuls processus QEMU par SSH n'a pas pu
être exécutée, faute d'accès au constructeur. Le constructeur virtuel a été
redémarré de force pour récupération ; aucune nouvelle instance de test BIOS
n'a été relancée automatiquement. Aucun disque physique n'était attaché aux
machines de test.

Cet incident est un **échec du test simultané dans cet environnement de
virtualisation**, pas un échec de fabrication de l'ISO. Il ne permet pas
d'attribuer une panne au bureau Live ou à un pilote particulier.
Le bureau, le réseau et les périphériques du Live BIOS ne sont pas validés.
Les essais suivants doivent être séquentiels, avec un vCPU et TCG à thread
unique, puis leurs résultats devront être consignés séparément.

## Tester directement sous Windows

Pour éviter l'émulation imbriquée dans le constructeur, le script
`scripts/test-iso-windows.ps1` peut lancer QEMU installé sous Windows ou une
copie portable officielle extraite dans `build/tools/qemu/`.
Cette voie ne construit pas l'ISO : elle teste uniquement le fichier déjà
fabriqué et son empreinte. Aucun périphérique physique n'est accepté.
Les chemins doivent désigner des fichiers locaux absolus, sans virgule ;
les chemins de périphériques, UNC et liens de fichiers sont refusés.

Préparer `SHA256SUMS` dans le dossier de l'ISO. Pour UEFI, placer les fichiers
Debian OVMF `OVMF_CODE_4M.fd` et `OVMF_VARS_4M.fd` dans `build/builder/`, ou
fournir leurs chemins avec `-OvmfCodePath` et `-OvmfVarsPath`.
Le fichier code est ouvert en lecture seule ; les variables sont copiées
dans un répertoire de traces neuf pour chaque test.

Dans PowerShell, remplacer IDENTIFIANT par le sous-dossier réellement livré :

```powershell
Set-Location G:\kyronext-os
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\test-iso-windows.ps1 -IsoPath 'G:\kyronext-os\releases\IDENTIFIANT\Kyronext-OS-0.1-amd64.iso' -Mode uefi
python .\tests\qmp.py tcp:127.0.0.1:4444 status
```

L'option ExecutionPolicy de cette commande concerne uniquement ce processus
PowerShell, pour le script local audité ; elle ne modifie pas la politique
Windows persistante. Le script cherche d'abord l'exécutable dans
`C:\Program Files\qemu\`, puis dans `build/tools/qemu/`. Le paramètre
`-QemuPath` permet de préciser un autre fichier exécutable local.

Le processus est détaché, sans fenêtre, avec journal, PID et `launch.json`
dans `logs/test-windows-uefi-DATE/`. Le profil est q35, qemu64, 3 Gio RAM,
un vCPU, TCG à thread unique, affichage VGA standard par défaut et réseau
utilisateur/NAT VirtIO. `-Video virtio` sélectionne un comparatif VirtIO ;
la valeur choisie est consignée dans `launch.json`.
Le succès du lancement ne valide ni le démarrage ni le bureau.

Capturer dans le dossier de traces affiché par le script :

```powershell
python .\tests\qmp.py tcp:127.0.0.1:4444 screenshot 'G:\kyronext-os\logs\test-windows-uefi-DATE\01-observation.png'
python .\tests\qmp.py tcp:127.0.0.1:4444 monitor 'sendkey ret'
python .\tests\qmp.py tcp:127.0.0.1:4444 shutdown
```

Les chemins des PNG sont interprétés par le QEMU Windows : ils doivent être
des chemins Windows absolus. Le port QMP est lié seulement à 127.0.0.1.
Ne pas exposer ce port sur le réseau. Le client QMP conserve également la
prise en charge des sockets Unix pour les tests dans Debian.

Après arrêt observé de la première machine, tester séparément le BIOS :

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\test-iso-windows.ps1 -IsoPath 'G:\kyronext-os\releases\IDENTIFIANT\Kyronext-OS-0.1-amd64.iso' -Mode bios
python .\tests\qmp.py tcp:127.0.0.1:4445 status
```

Le BIOS n'utilise pas OVMF. Le port 4445 et le dossier de traces BIOS sont
distincts ; cette séparation ne justifie pas de lancer deux machines en
parallèle sur un hôte déjà chargé. Ces commandes décrivent la procédure,
pas des résultats déjà obtenus.
