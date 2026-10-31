## Candidate suite intégrée 20261031T1854-suite-search-preview — 31 octobre 2026

- Ajoute le paquet Qt Kyronext Suite Preview 0.1.0-0~preview2 et Kyronext Launchpad à l'ISO Live Debian 13.
- Produit une ISO hybride de 2 454 208 512 octets ; empreinte conservée dans la release.
- Valide visuellement l'accueil et KDE sous QEMU/WHPX en BIOS et UEFI ; ouvre Monitor et voit ses métriques dans le Live.
- Vérifie que la confirmation d'arrêt affiche Kyronext OS sans logo Debian.
- Ne revendique pas d'installation sur disque ni de validation matérielle du HP EliteBook 2540p.

## Suite Qt preview — 31 octobre 2026

- Compilation complète sous Debian 13 AMD64 et CTest : 16/16 groupes réussis.
- Construit `kyronext-suite-preview 0.1.0-0~preview2` ; installation et désinstallation validées en VM isolée, puis intégration à la candidate `20261031T1854-suite-search-preview`.
- Kyronext Launchpad ajoute une recherche de noms de fichiers facultative, bornée, annulable et limitée aux dossiers choisis ; aucun parcours au démarrage.
- Recherche locale : 9/9 tests sous Debian 13 et Windows. L'application est présente dans la candidate récente ; son usage interactif complet dans KDE reste à tester.

## Candidate Live propre 20261031T175731-clean-live — 31 octobre 2026

- Reconstruit une candidate sans clé SSH privée ni hash de mot de passe Live ; conserve l'ancienne image privée séparément.
- Vérifie l'ISO hybride et son SHA-256 après transfert depuis le constructeur Debian.
- Valide visuellement l'accueil KDE Kyronext en QEMU WHPX, BIOS et UEFI, sans disque virtuel.
- Reproduit les limites de QEMU TCG (rendu brouillé ou noir) et règle le script Windows pour utiliser WHPX par défaut sur ce poste.
- Aucun test sur le HP EliteBook 2540p ou autre matériel physique n'est revendiqué.

## 0.1 Developer Preview — candidate privée 20261031T1542-legacy-fbdev

- Corrige le profil Legacy X11 lorsque `/dev/fb0` et le pilote Xorg fbdev sont disponibles ; conserve le repli Xorg standard sur les autres chemins BIOS.
- Ajoute explicitement `xserver-xorg-video-fbdev` à la liste des paquets.
- Valide visuellement dans VirtualBox les démarrages Live BIOS/UEFI en modes standard et Legacy ; rapport et captures dans `TEST_REPORT.md` et `releases/20261031T1542-legacy-fbdev/validation/`.
- Teste Kyronext Monitor dans le Live UEFI standard ; GPU et températures restent non mesurés en VM.
- Image privée de 2 451 800 064 octets, sous le plafond de 16 Go. Ne pas distribuer : clé SSH publique et accès Live privés présents.

## Durcissement de la construction et diagnostics QEMU — 31 octobre 2026

- Le hook de compte Live ne contient plus de hash codé en dur : le compte est verrouillé par défaut. Un hash privé peut être fourni depuis un fichier en mode 0600 ; un outil le génère avec saisie masquée.
- La clé SSH reste facultative et les identifiants de test ne sont jamais inclus implicitement. L’ancienne ISO privée n’est pas modifiée.
- Le test Windows accepte `-Acceleration whpx` et un transfert SSH diagnostique limité à `127.0.0.1`.
- QEMU UEFI avec WHPX affiche Kyronext Welcome ; SDDM, Wayland et Plasma sont actifs. TCG est lent et reste non validé visuellement ; QEMU BIOS n’a pas été testé.

---

# Journal des modifications

## Candidate privée 20261031T145700Z-legacy-compat — validation — 31 octobre 2026

- Construction Debian 13 live-build AMD64 terminée ; ISO de 2 451 800 064 octets, SHA-256 Debian/Windows identique ; limites de 16 Go ISO et moins de 1 Go pour pilotes/firmwares respectées.
- Suite Qt preview intégrée ; Monitor testé visuellement dans le Live BIOS et UEFI. Bureau/Welcome observés dans les deux firmwares VirtualBox.
- L’entrée Legacy et `nomodeset` sont confirmés, mais Xorg échoue sous VMSVGA ; ce mode reste ouvert. Aucun test sur le HP 2540p ni clé USB n’a été effectué.
- Candidate gardée privée (clé publique SSH du propriétaire et accès Live temporaire) ; QEMU et Calamares ne sont pas validés.

## Sécurité et validation locale — 31 octobre 2026

- Le retour arrière Wi-Fi ouvre ses snapshots sans suivre de lien symbolique et refuse les fichiers non ordinaires ou accessibles au groupe/autres ; tests du gestionnaire : 17/17.
- Les contrôles de budget passent 6/6. Sous Windows, 34 tests dépôt, 37 tests Firefox/Guardian/liens/veille/utilitaires et 4 tests Recovery passent ; six cas Linux/POSIX ou privilèges sont explicitement ignorés.
- Les préférences du test de thème Qt sont isolées dans un répertoire temporaire. Le cas sysfs utilisant un nom Linux est maintenant ignoré sur Windows.
- CTest est configuré pour signaler comme ignorés les tests Backup sautés sous root ; le comportement et les modifications C++/CMake attendent une recompilation Debian.
- Les dépendances explicites CPack suivent maintenant les modules activés au lieu d'imposer toutes les dépendances facultatives ; la génération du paquet reste à valider sur Debian.
- Le constructeur VirtualBox n'a pas été redémarré. Aucune nouvelle suite intégrée ni ISO n'a été produite ; aucune clé USB n'a été modifiée.

## Consolidation des applications — 31 octobre 2026

- Kyronext Settings et le framework Qt ajoutent contraste renforcé et densités compacte/confortable, avec import rétrocompatible des préférences.
- Kyronext Monitor indique les liens réseau et sources secteur/USB disponibles ; il estime l'autonomie seulement à partir de mesures noyau cohérentes.
- Refonte source de Monitor : vues performances, matériel/réseau et processus ; courbes CPU/RAM/réseau ; inventaire PCI graphique avec distinction explicite entre matériel détecté et métriques GPU non disponibles.
- Le scan des processus se suspend hors de sa vue et l'arrêt utilise pidfd pour éviter de signaler un PID réattribué. La compilation Debian de Monitor et ses tests système/rendu hors écran réussissent 2/2 ; les captures sont dans `logs/`. Le test en session KDE et l'intégration à une ISO restent à faire.
- Première interface Kyronext Update : simulation APT strictement en lecture seule, raccourci et icône SVG originaux.
- Kyronext Recovery produit sur demande un rapport local texte/JSON, sans root ni réparation, avec création exclusive en mode privé.
- Suite C++ compilée intégralement dans Debian ; CTest 13/13 ; installation de prévisualisation validée dans un `DESTDIR` temporaire.
- Rebuild intégré actuel : 166 étapes de compilation et CTest 16/16. Backup et Disk Health sont inclus ; le paquet `.deb` récent est construit avec la dépendance `tar`. La reconstruction et le démarrage de l'ISO qui contiendra ce paquet restent à faire.
- Ces changements restent dans les sources et ne sont pas intégrés à l'ISO 0.1.

## Nouveaux outils en cours de validation — 31 octobre 2026

- Kyronext Backup : archive TAR de dossiers choisis, publication sans écrasement, annulation, empreinte SHA-256 et avertissement explicite sur l'absence de chiffrement. Les tests autonomes Debian de l'agent passent 5 cas ; la compilation intégrée à toute la suite n'a pas pu être confirmée après l'indisponibilité de la VM.
- Kyronext Disk Health : diagnostic SMART/NVMe local en lecture seule, sans root ; contrôleurs ou permissions non accessibles restent marqués inconnus.
- Réduction des effets visuels : préférence commune consommée par AMP et par la veille Kyronext ; son test de trajectoire passe 4/4.
- Les deux outils sont dans les sources seulement. Ils ne sont pas ajoutés à l'ISO 0.1 et aucun disque physique n'a été testé.

## Révision outils et identité — en cours de validation

- Calculatrice et bloc-notes natifs PyQt6, palettes rouge et ambre.
- Tests du calcul, des sauvegardes UTF-8 et des confirmations réussis.
- Identité Kyronext pour À propos, SDDM et splash Plasma statique.
- Intégration des nouveaux champs os-release pendant la révision d'ISO.
- Publication de la première ISO publique préparée, distincte de cette révision.

## 0.1 — révision visuelle du 30 octobre 2026

- Fonds et libellés Kyronext pour les menus de démarrage BIOS et UEFI.
- Palette graphite/rouge, icônes originales et panneau KDE complet avec raccourcis vers les applications installées.
- Accueil PyQt6 avec accès aux fichiers, terminal, paramètres et diagnostic matériel.
- Veille Kyronext native : illustrations originales de KITT et KARR en perspective, modes rouge/ambre/mixte et option sans animation ; lancement manuel uniquement.
- Script de révision d'une ISO existante, conservant les paquets Debian et contrôlant la structure hybride.

L'image personnalisée `143045` a atteint le bureau en BIOS/UEFI. La révision suivante `151047`, contenant la veille et le dernier ajustement du contraste, est produite ; ses vérifications finales restent en cours. Les droits des illustrations de voitures sont décrits dans `LICENSES/VEILLE.md`.

## 0.1 Developer Preview — préparation

- Dépôt indépendant et constructeur Debian 13 isolé sur G:.
- Configuration live-build hybride UEFI/BIOS avec KDE, SDDM et X11.
- Identité originale, palette sombre/rouge et accueil PyQt6.
- Diagnostic matériel local, sans installation automatique de pilote.
- Prototype indépendant Kyronext Legacy Driver Manager et matrice de compatibilité.
- Scripts de construction, inspection ISO et tests QEMU/OVMF.

La disponibilité de chaque fonction est documentée dans TEST_REPORT.md ; une entrée dans ce journal ne constitue pas une validation du démarrage.

## Résultat de la préversion

ISO effectivement produite, empreinte vérifiée et bureau atteint en BIOS/UEFI dans le profil QEMU VGA standard. Les résultats détaillés, les profils non validés et les limites sont consignés dans TEST_REPORT.md. Guardian Hardware Passport indépendant validé sous Debian ; aucune interface Guardian complète ni installateur ajouté à l'image.

## Préparation du prochain média — 2026-10-31

- Compte Live Kyronext avec le secret temporaire configuré pour cette image privée; sudo le demande et root reste verrouillé.
- Ajout explicite de `kde-spectacle`; Kyronext Capture ouvre Spectacle si la capture native n'est pas autorisée.
- Ajout des tests de configuration Live et de repli de capture, ainsi que de l'architecture d'assistant local inspirée des fonctions publiques d'openKylin.
- Aucun modèle IA ni lanceur distant de jeu n'est annoncé prêt : le modèle attend mesure/licence, et les jeux attendent l'appairage à l'AGX.
