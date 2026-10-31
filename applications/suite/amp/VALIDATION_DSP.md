# Validation AMP DSP

Sources C++17, Qt 6.8.2, Debian 13 AMD64, GCC 14.2 dans la VM de construction.
La compilation se fait dans `/home/builder/amp-dsp-work/build`, séparément
de l'ISO et des autres travaux. Aucun composant installé sur Windows.

## Contrôles exécutés

- Playlists et décodage réel d'un WAV synthétique : réussis.
- DSP : neutralité et bypass, gain de 6 dB à 1 kHz, préamplification,
  isolation des canaux, impulsion et limitation des valeurs de sortie.
- Le `QIODevice` de sortie renvoie effectivement les échantillons traités.
- Fenêtre AMP réelle hors écran : activation du DSP, dix bandes accessibles,
  changement des gains, masquage du panneau et plein écran F11.
- Capture produite par Qt, sans image simulée : `logs/amp-dsp-preview.png`.
- Redimensionnement réel en 800×600, défilement jusqu'aux commandes du bas et
  plein écran : réussis ; `logs/amp-dsp-small-screen.png`.
- Résultats complets : `logs/amp-dsp-test-final.log`.

## Ce que ces résultats ne prouvent pas

La sortie vers des haut-parleurs physiques, toutes les cartes son et tous les
formats ne sont pas validés. Les visualisations sont alimentées par le PCM
post-DSP, mais aucune mesure de latence ou calibration professionnelle du
spectre n'est revendiquée. Les tests hors écran ne prouvent pas l'intégration
dans une ISO ou le comportement de tous les gestionnaires de fenêtres.

Le mode standard QMediaPlayer reste sans égaliseur. L'activation volontaire du
mode DSP recharge le morceau et décode au maximum 128 Mio de PCM en mémoire.
Une sortie incompatible, une erreur de décodage ou cette limite déclenchent
un message et le retour à la lecture standard. Le limiteur numérique est une
protection contre les valeurs hors échelle ; diminuer le préampli lorsqu'il
intervient pour éviter la distorsion.
