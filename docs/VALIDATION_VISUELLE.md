# Validation de la personnalisation — 30 octobre 2026

## Première révision personnalisée

Image : `releases/20261030T143045Z-branding-final/Kyronext-OS-0.1-amd64.iso`.
SHA-256 : `91d8238d4eb33686b9674981dabf828fafb97f8e9d4d2319b019486e59508c85`.

Tests réussis : démarrage BIOS QEMU et UEFI VirtualBox jusqu'au bureau KDE,
menus de démarrage Kyronext originaux, panneau complet et icônes Kyronext,
accueil PyQt6 et ouverture réelle de Konsole depuis sa carte Terminal.
Les sommes internes et les structures hybrides BIOS/UEFI ont été vérifiées.

Preuves dans `logs/` : `branding-bios-boot-01.png`,
`branding-vbox-boot-01.png`, `branding-bios-desktop.png`,
`branding-vbox-02.png`, `branding-welcome-terminal-action.png`,
`branding-integrity.txt`.

## Veille Stardrive et révision suivante

Image finale : `releases/20261030T151047Z-branding-806/Kyronext-OS-0.1-amd64.iso`.
Taille : 1 638 072 320 octets. SHA-256 recalculé sous Windows et concordant :
`1bdac4c7af5981819564818b7bc3b8be9a295b5a5ab3af1f0a4eb2136d04d0cd`.
Les 298 contrôles SHA-256 internes ont réussi. Les structures BIOS, UEFI,
MBR hybride et Joliet sont présentes. Le menu UEFI personnalisé est observé
sur cette image (`logs/stardrive-uefi-boot.png`).

Illustrations originales KITT et KARR intégrées au lanceur manuel
**Veille Kyronext**. Tests de trajectoire : 3 réussis. Contrôles des sources
et du hook de démarrage réussis. Rendu PyQt6 réel hors écran dans Debian :
deux images différentes à 400 et 1400 ms, puis Échap ; sortie 0 et fenêtre
fermée. Preuves : `logs/stardrive-runtime-validation.json` et
`logs/stardrive-preview-0.png`, `logs/stardrive-preview-1.png`.

Validation finale dans VirtualBox UEFI : bureau KDE et accueil observés,
icônes de notification lisibles, lancement de `kyronext-stardrive` depuis
KRunner, rendu plein écran des deux voitures et deux captures différentes
à deux secondes d'intervalle. Échap ramène à l'accueil et au bureau.
Preuves : `logs/stardrive-uefi-desktop.png`, `logs/stardrive-launch.png`,
`logs/stardrive-live-01.png`, `logs/stardrive-live-02.png`,
`logs/stardrive-live-escape.png`. Le lancement par le menu des applications
n'a pas été testé séparément. Le BIOS de cette dernière révision n'a pas été
redémarré : le test réel BIOS ci-dessus concerne la révision précédente.

Aucune mesure de fluidité sur ordinateur physique, aucun déclenchement de
veille automatique ni verrouillage de session ne sont fournis.

Les résultats historiques de la première base live-build restent dans
`TEST_REPORT.md` et dans sa livraison d'origine. Chaque image doit conserver
son empreinte et son périmètre de validation propres.
