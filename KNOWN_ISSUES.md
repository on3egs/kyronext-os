# État actuel — candidate propre 20261031T175731-clean-live (31 octobre 2026)

- La candidate Live de 2 451 800 064 octets n'embarque ni `authorized_keys` propriétaire ni hash de mot de passe Live. L'accueil démarre avec autologin ; aucun sudo n'est disponible par défaut.
- QEMU WHPX atteint l'accueil Kyronext avec VGA standard en UEFI et BIOS. Captures dans `releases/20261031T175731-clean-live/validation/`.
- QEMU TCG n'est pas validé sur cet hôte : VGA standard devient brouillé après le menu ; VirtIO garde une console lisible pendant le démarrage Legacy, puis l'écran devient noir sans atteindre le bureau après trois minutes. Le script Windows prend désormais WHPX par défaut.
- L'HP EliteBook 2540p n'a pas été testé physiquement. Wi-Fi, audio, GPU, capture, imprimantes et autres périphériques matériels restent inconnus.
- L'ISO est Live seulement. Calamares, installation disque, Secure Boot, chiffrement, IA locale et restauration Phoenix ne sont pas revendiqués comme opérationnels.
- L'historique Git local contient un ancien hash de mot de passe de test ; ne pas pousser/publicer l'historique avant nettoyage.

---

# Candidate privée antérieure — 20261031T1542-legacy-fbdev

- Les quatre chemins testés en VirtualBox (BIOS/UEFI, standard/Legacy) atteignent un bureau visible. Cela ne prouve pas la compatibilité d'un ordinateur physique ni du HP EliteBook 2540p.
- La candidate contient une clé SSH publique personnelle et un accès d'administration Live temporaire ; elle reste privée et doit être nettoyée puis reconstruite avant publication.
- QEMU UEFI/OVMF sous WHPX affiche le bureau et confirme SDDM/Plasma actifs. TCG single-thread est fortement ralenti et n’a pas montré le bureau durant la fenêtre d’observation ; la cause du rendu TCG n’est pas isolée. QEMU BIOS et les périphériques réels restent à tester. VirtualBox passe les quatre profils.
- xorriso signale que l'identifiant Joliet dépasse la recommandation de 16 caractères ; aucun problème de démarrage n'a été observé.
- Les défauts Legacy X11 observés sur la candidate précédente sont corrigés pour cette image et testés sous VirtualBox. La compatibilité avec les anciens GPU physiques reste à vérifier.
- Phoenix Recovery, Adaptive OS et AI Doctor restent des fondations et éléments de feuille de route, pas des systèmes de restauration ou de diagnostic assisté complets.

---

# Limites de la candidate 20261031T145700Z-legacy-compat — 31 octobre 2026

- Le profil « Legacy Compatibility » échoue sous VirtualBox VMSVGA : le pilote Xorg VMware livré avec l'hyperviseur ne mappe pas le framebuffer avec `nomodeset`, puis plante. Le démarrage X11 et l'affichage de ce profil restent à corriger et à tester sur matériel réel. Le mode standard UEFI et BIOS, lui, atteint KDE.
- Le changement vers VBoxVGA laisse un écran noir dans cette VM ; VMSVGA/EFI64 ont été restaurés après les essais.
- Le HP EliteBook 2540p n'a pas été testé : ne pas inférer sa compatibilité depuis VirtualBox. Wi-Fi, audio, GPU, capture d'écran et périphériques réels restent à vérifier.
- La candidate privée inclut une clé publique SSH du propriétaire et un accès sudo temporaire Live. Ne pas publier ni partager ; retirer ces accès avant une construction publique.
- QEMU/KVM, Calamares, installation sur disque, Secure Boot, chiffrement, IA locale et restauration Phoenix ne sont pas validés ou ne sont pas inclus.
- Le domaine `kironext.be` reste « À confirmer » faute de résolution DNS dans l'audit ; son orthographe n'a pas été remplacée.

Les validations positives de cette candidate sont détaillées dans `TEST_REPORT.md` et `TEST_REPORT_SUITE.md`.

---

﻿# Actualisation des limites — 31 octobre 2026

- `20261031T0334-live-account-fix` démarre jusqu'au bureau en VirtualBox UEFI et BIOS Legacy. Le groupe `sudo` et l'accès `sudo id` avec le secret temporaire configuré pour cette image privée sont validés en Live.
- À la résolution virtuelle 1024×768, le fond d'écran est cadré de façon imparfaite et le grand texte Kyronext est partiellement hors écran. À ajuster pour les résolutions 4:3 et 16:10 ; le bureau reste utilisable.
- Le HP EliteBook 2540p n'a pas encore été testé sur matériel réel. Ne pas lui attribuer un statut de compatibilité validé avant vérification de l'affichage, du réseau, de l'audio et du stockage.
- L'ISO est une préversion Live, pas une image d'installation vérifiée. Calamares, chiffrement de disque, Secure Boot, recovery et restauration ne sont pas validés.
- La capture par Print Screen sur le HP n'a pas été testée physiquement. L'image contient `kde-spectacle` et le code de repli Kyronext Capture.
- Les anciennes entrées ci-dessous sont des observations d'ISO antérieures ; elles ne décrivent pas la release actuelle lorsqu'un correctif est indiqué plus haut.
# Limites actuelles — 31 octobre 2026

- `20261031T0334-live-account-fix` démarre jusqu'au bureau en VirtualBox UEFI et BIOS Legacy. Le groupe `sudo` et la commande `sudo id` avec le secret temporaire configuré pour cette image privée sont validés en Live.
- À la résolution virtuelle 1024×768, le fond d'écran est cadré imparfaitement et le grand texte Kyronext est partiellement hors écran. À corriger pour les résolutions 4:3 et 16:10 ; le bureau reste utilisable.
- Le HP EliteBook 2540p n'a pas été testé sur matériel réel. Ne pas lui attribuer de statut de compatibilité avant vérification de l'affichage, du réseau, de l'audio et du stockage.
- L'ISO est une préversion Live, pas une image d'installation testée. Calamares, le chiffrement de disque, Secure Boot, Phoenix Recovery et l'IA locale ne sont pas validés.
- Print Screen n'a pas été testé sur le HP. Le paquet `kde-spectacle` et le repli de Kyronext Capture sont inclus dans la configuration ; le test physique reste à faire.
- Les entrées plus anciennes ci-dessous documentent des images antérieures et ne doivent pas être interprétées comme l'état de la release actuelle lorsque le correctif est indiqué ci-dessus.
# Limites connues — 0.1 Developer Preview

- Dans les ISO `20261030T202137Z-16799` et `20261030T213008Z-85730`, Firefox ESR et son entrée Kyronext démarrent. Le chrome du navigateur est sombre avec un accent rouge. Le second essai ouvre explicitement l'URL locale Kyronext Start, mais son contenu reste visuellement vide. La page de démarrage n'est donc pas fonctionnelle et doit être corrigée avant de déclarer Firefox terminé.
- L'ISO `20261030T202137Z-16799` a démarré jusqu'au bureau KDE Live sous UEFI et BIOS Legacy dans VirtualBox. Le premier écran UEFI est resté noir pendant l'initialisation graphique avant l'affichage du splash KDE puis du bureau ; les temps mesurés en VM ne représentent pas du matériel réel.
- Le contrôle visuel de l'ISO `20261030T225146Z-155866` a confirmé l'accueil Kyronext et l'écran de confirmation d'arrêt Kyronext OS. Le menu Kickoff affichait encore « Debian Live user » et son avatar Debian. Le correctif actuel fournit `user-fullname=Kyronext`, un avatar dans `/etc/skel` et une commande post-`live-config`; il n'est pas inclus dans cette ISO publiée et doit être reconstruit puis revérifié.
- Kyronext Navigator est inclus comme application expérimentale et non définie par défaut. Les tests C++/Qt et du moteur local réussissent dans le constructeur Debian, mais le lancement graphique de Navigator dans cette image n'a pas encore été validé.
- Les sites externes n'ont pas été ouverts pendant le test Live. Les raccourcis visibles dans Welcome ne prouvent ni la disponibilité actuelle des domaines ni la connectivité réseau de la VM.
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

## ISO `20261031T000112Z-1816`

Le bureau KDE Live démarre en UEFI et Kickoff affiche désormais Kyronext avec l'avatar K. `live-config.service` a néanmoins signalé un échec au démarrage. Le compte sudo n'a pas de mot de passe utilisable dans cette image. Les corrections sudo et capture sont présentes dans les sources actuelles, mais attendent une reconstruction et une validation visuelle.
