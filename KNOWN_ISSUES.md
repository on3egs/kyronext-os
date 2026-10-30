# Limites connues — 0.1 Developer Preview

- ISO Live uniquement : aucun installateur sur disque ni persistance intégrée. Les changements du Live sont temporaires.
- Le profil QEMU validé utilise VGA standard (`-vga std`), 1 vCPU qemu64 et 3 Gio RAM. VirtIO graphique n'est pas validé : les premiers essais ont présenté un écran noir persistant. Ne pas généraliser ce résultat à tous les systèmes ou versions QEMU.
- Le démarrage UEFI QEMU VGA standard a montré un affichage brouillé avant l'initialisation du pilote bochs-drm ; le bureau est ensuite devenu lisible et fonctionnel. Ce défaut de démarrage reste à corriger.
- L'émulation TCG sur cet hôte est lente, avec des pauses importantes lorsque plusieurs VM et la vérification SHA-256 se disputent les ressources. Les essais QEMU imbriqués dans VirtualBox ont provoqué un softlockup du constructeur. Préférer QEMU direct et tester les modes séparément.
- Les fonds de démarrage BIOS/UEFI et les raccourcis du panneau ont été corrigés et observés dans l'image personnalisée `20261030T143045Z-branding-final`. Le dernier ajustement de contraste des icônes reste à vérifier dans l'image suivante. Aucun navigateur Web n'a été explicitement intégré à cette version minimale.
- La Veille Kyronext est une application lancée manuellement ; elle ne verrouille pas la session et n'active aucune veille automatique. Son rendu Qt hors écran a été testé ; le lancement depuis le bureau de la dernière image reste à vérifier.
- Le compte Live est verrouillé pour l'authentification par mot de passe ; l'autologin graphique est prévu. Ne pas verrouiller manuellement la session. Le verrouillage automatique est désactivé ; une fermeture de session relance la session Live.
- Le journal noyau peut être restreint pour le diagnostic non privilégié. Une absence de message accessible n'est pas une preuve d'absence de défaut.
- Aucun ordinateur physique déclaré compatible. Audio réel, Wi-Fi physique, GPU physique, imprimantes, scanners, Secure Boot, chiffrement et installation sur disque ne sont pas validés.
- Cockpit, édition Xfce, mode Legacy dédié, Phoenix Recovery, Adaptive OS et AI Doctor ne sont pas opérationnels. Les prototypes Guardian/Legacy JSON sont fournis dans les sources, séparément de l'ISO qui contient le diagnostic texte local.
