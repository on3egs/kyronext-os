# État réel des outils Kyronext

**La suite complète demandée n'est pas terminée.** Cette feuille distingue les
applications déjà créées de celles qui manquent encore pour un poste généraliste.

| État | Outils |
|---|---|
| Sources et intégration de préversion | Welcome, Notes, Calculator, Paint, AMP, Files, Monitor, Gallery, Video, Capture, Archive, Calendar, Clock, Settings, Guardian Hardware Passport, Firefox ESR thème Kyronext, Navigator expérimental, veille Stardrive |
| Composants KDE utilisés comme solutions éprouvées | Dolphin, Konsole, NetworkManager, Paramètres KDE, gestion d'énergie KDE, gestionnaire de paquets APT |
| À développer ou intégrer après validation | Bureautique (documents, tableur, présentation), gestionnaire de logiciels graphique, interface de mises à jour, sauvegarde/restauration, imprimantes et scanners, visionneuse PDF, client mail, gestionnaire de mots de passe, outils d'accessibilité avancés, installateur graphique |
| Fonctions encore au stade planifié | Kyronext Intelligence locale complète, Connect/appairage, Universal Search, Media Center, Phoenix Recovery et AI Doctor |

Les fichiers sources ou les boutons d'une application ne signifient pas que chaque
fonction est intégrée à l'ISO stable ou testée sur un PC physique. Firefox doit
encore être corrigé : sa page locale Kyronext Start s'ouvre mais son contenu reste
vide dans le test VirtualBox du 30 octobre 2026. Navigator reste experimental.

## Priorités pour le HP EliteBook 2540p

1. Identité Kyronext complète du menu et de la boîte d'arrêt.
2. Diagnostics de batterie, Wi-Fi, veille et affichage sur la machine réelle.
3. Environnement léger X11/Xfce en édition ultérieure, sans retirer KDE de
   l'édition standard.
4. Bureautique hors ligne et visionneuse PDF, en intégrant des moteurs libres
   maintenus plutôt qu'en réécrivant ces fonctions.
5. Sauvegarde et gestionnaire de logiciels avant toute installation publique.

Les projets d'installation, de sauvegarde et de récupération n'écrivent pas sur
un disque dans cette préversion.

## Extensions demandées ensuite

- Compte Live avec sudo protégé par mot de passe : le prochain média doit
  proposer le secret temporaire configuré pour cette image privée, documenté et modifiable.
- Capture : intégrer explicitement `kde-spectacle` et faire basculer Kyronext
  Capture vers Spectacle si l'API native n'est pas disponible, notamment sous
  Wayland. La capture reste soumise à l'autorisation du bureau.
- Assistant local : commencer par un catalogue d'actions Kyronext à permissions
  limitées. Le modèle CPU léger doit rester optionnel, fonctionner hors ligne et
  ne pas recevoir de droits administrateur. Aucun modèle n'est intégré tant que
  son français, ses performances sur l'EliteBook 2540p, sa licence et son moteur
  d'inférence ne sont pas validés.
- Jeux AGX : afficher une catégorie réservée aux lanceurs réellement découverts
  après appairage explicite avec le Jetson. Aucun scan réseau ni faux raccourci
  n'est activé par défaut.
