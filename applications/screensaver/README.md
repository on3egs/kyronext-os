# Veille Kyronext — Stardrive

Application PyQt6/QPainter indépendante, lancée manuellement depuis le menu
**Veille Kyronext**. Elle ne verrouille pas la session et ne configure aucun
service de veille ou déclenchement automatique.

Elle utilise exclusivement les PNG transparents `kitt.png` et `karr.png`
fournis dans `/usr/share/kyronext/stardrive/`. Les représentations graphiques
originales sont fournies séparément ; `IMAGE_PROMPTS.json` en conserve la
description. Si une image nécessaire manque ou est illisible, une erreur
claire est affichée. Aucune silhouette générique ne la remplace.

```bash
python3 /usr/local/bin/kyronext-stardrive
python3 /usr/local/bin/kyronext-stardrive --mode red
python3 /usr/local/bin/kyronext-stardrive --mode amber --reduced-motion
python3 /usr/local/bin/kyronext-stardrive --windowed --assets-dir /chemin/vers/images
```

Le mode `mixed` est le défaut. Douze voitures sont affichées par défaut,
avec un maximum strict de vingt-quatre via `--count`. Le mouvement en
perspective est limité à quinze images par seconde et les tailles de
sprites sont mises en cache une seule fois. `--reduced-motion` affiche une
composition immobile sans minuterie d'animation. Le réglage partagé
« Réduire les animations et visualisations dans AMP » désactive aussi le
mouvement de cette veille pour l'utilisateur courant ; `--reduced-motion`
permet de le forcer depuis la ligne de commande.

Échap ferme immédiatement l'application. Une touche, un clic, la molette
ou un déplacement de souris dépassant dix-huit pixels la ferment après
une courte période de grâce de 0,85 seconde. Cela évite que les événements
du lancement depuis le menu ferment aussitôt la fenêtre. Le curseur est
masqué uniquement dans cette application.

Aucun accès administrateur, téléchargement, indicateur matériel inventé ou
modification de pilote. Le composant ne demande aucune dépendance nouvelle
par rapport à PyQt6 déjà installé. La validation du rendu réel et du coût
CPU revient aux captures dans la VM ; les tests de trajectoire ne prouvent
pas seuls le résultat graphique.
