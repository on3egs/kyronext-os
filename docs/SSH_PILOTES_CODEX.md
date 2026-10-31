# SSH, pilotes et Codex — 31 octobre 2026

**État vérifié le 31 octobre 2026 :** l’ISO privée candidate `20261031T145700Z-legacy-compat` a été construite et testée sous VirtualBox. Aucune clé USB ni aucun disque physique n’a été modifié. Le constructeur Debian isolé a servi à produire cette image.

## SSH

Serveur et client OpenSSH inclus, service activé au boot, port 22, utilisateur
`kyronext`, authentification par clé uniquement. Root et le mot de passe Live
partagé ne permettent pas l'accès distant. Sans clé autorisée, le serveur répond
mais refuse l'authentification.

Depuis le terminal Live : `sudo kyronext-ssh authorize /chemin/manix.pub`, puis
`kyronext-ssh status`. Connexion distante : `ssh kyronext@ADRESSE_IP`.

Une image privée peut autoriser explicitement une clé publique au build :
`sudo env KYRONEXT_SSH_PUBLIC_KEY=/chemin/manix.pub bash scripts/build-iso.sh`.
Ne pas publier cette image comme image générique : le détenteur de la clé pourra
administrer les machines utilisant cette image. Aucune clé du PC n'est copiée
automatiquement. Les clés privées ne sont jamais incluses.

La clé hôte est générée dans `/run` à chaque démarrage Live ; son empreinte change
donc entre deux boots. Vérifier localement avec
`sudo ssh-keygen -lf /run/kyronext-ssh/ssh_host_ed25519_key.pub` avant d'actualiser
known_hosts. Ne pas désactiver la vérification de clé hôte globalement.
Une autorisation ajoutée au Live non persistant disparaît après redémarrage.

## Matériel

Noyau Debian AMD64 et ses modules existants, firmwares Intel, Qualcomm/Atheros,
Realtek, Broadcom brcm80211, Marvell/Libertas, MediaTek, TI, anciennes clés USB,
AMD/Intel/NVIDIA, Intel audio/SOF/Cirrus et Ethernet courants. Mesa/Xorg assurent
le graphique. Aucun pilote propriétaire NVIDIA ou Broadcom STA DKMS forcé.
Tous les matériels existants ne sont pas garantis compatibles : certaines cartes
nécessitent un pilote spécifique, absent du noyau, ou un firmware non redistribuable.

Gobi 2000 est un modem WWAN 3G, pas une carte Wi-Fi. ModemManager, QMI,
usb-modeswitch et PPP sont prévus. Le paquet Trixie gobi-loader 0.6 étant trop
ancien, le constructeur recompile les sources Debian 0.7 vérifiées par SHA-256,
sans dépôt Sid/Testing. Les modules qcserial/qmi_wwan restent autodétectés.
Les fichiers constructeur compatibles `amss.mbn`, `apps.mbn`, `UQCN.mbn` dans
`/lib/firmware/gobi/` restent nécessaires et **ne sont pas fournis**. Pas de flash
ni de téléchargement OEM automatique. SIM/APN/réseau opérateur à vérifier.
Diagnostic : `lsusb`, `mmcli -L`, `nmcli device`, `rfkill list`.

## Plafonds

ISO strictement inférieure à **16 000 000 000 octets** (16 Go décimaux). Somme Installed-Size
des paquets de `hardware-connectivity.list.chroot` plus les archives de la réserve hors ligne strictement inférieure à
**1 000 000 000 octets**. Ce dernier périmètre couvre les ajouts sélectionnés,
pas le noyau déjà présent ni toutes leurs dépendances transitives.
La construction doit échouer si un plafond est dépassé ou un paquet absent ;
`size-budget.json` consigne la mesure. Un remaster visuel ne suffit pas pour
installer ces nouveaux paquets : reconstruire avec live-build.

## Codex CLI

Hook de préinstallation du paquet officiel npm `@openai/codex@0.166.0`, avec
Node.js, npm, Git, certificats et ripgrep. Le build vérifie `codex --version`.
Lancer `codex` en utilisateur normal et se connecter avec son propre compte.
Aucun jeton/API key/compte de l'hôte incorporé. Codex nécessite un accès réseau
et une authentification ; ce n'est pas un modèle local préchargé. Le Live sans
persistance ne conserve pas l'authentification entre les boots.
Documentation officielle : https://learn.chatgpt.com/docs/codex/cli

## Résultats de cette candidate et suites nécessaires

L’ISO `20261031T145700Z-legacy-compat` a été produite avec le paquet de suite preview et SHA-256 vérifié. La réserve pilotes/firmwares et l’image respectent leurs plafonds. Le bureau Live standard a démarré jusqu’à KDE sous UEFI et BIOS VirtualBox. Le profil Legacy démarre la cible graphique mais Xorg échoue avec le pilote VMSVGA de cette VM ; ne pas annoncer ce profil comme validé. QEMU, le HP EliteBook 2540p et ses cartes physiques, la validation des fonctions Codex dans l’image et le démarrage sur matériel réel restent à tester. Ne pas écrire l’image sur USB depuis ce script.

Sources : https://packages.debian.org/trixie/firmware-atheros ;
https://sources.debian.org/src/gobi-loader/0.7%2Bgit20171116.143deb4%2Bdfsg-1/debian/README.Debian

## Vérifications réellement effectuées

Les paquets matériels sélectionnés depuis Debian 13 et le paquet local Gobi 0.7 sont recensés dans `size-budget.json` de la candidate. Le contrôle de la construction mesure 623 169 536 octets de paquets installés, 265 286 712 octets de réserve hors ligne, soit 888 456 248 octets au total.
Archive source Debian Gobi SHA-256 vérifiée et option -2000 confirmée dans le code.
Syntaxe de 7 scripts shell vérifiée avec bash -n. Tests sources existants : OK.
7 tests de refus/acceptation des budgets et de non-écrasement du rapport : OK.
3 tests existants d'accessibilité Live : OK.
La règle NAT temporaire 2224 testée pour diagnostiquer le constructeur a été
retirée ; la règle existante 2222 est conservée.
La candidate a été construite et démarrée. Cela ne valide pas la compatibilité des cartes physiques ni l’usage de Codex sur Internet depuis l’image Live.
