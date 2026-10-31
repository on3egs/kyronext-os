# Kyronext Navigator — prototype expérimental

Kyronext Systems — By Manix. Version 0.1.0. Application native C++17/Qt 6.8,
avec Qt WebEngine/Chromium et le framework commun `KyronextUi`.

## Limitation de sécurité déterminante

Les notes de Debian 13 indiquent que les applications utilisant `qt6-webengine`
ne bénéficient pas du support de sécurité de Debian. Elles recommandent Firefox
ou Chromium pour le Web général. Le paquet officiel disponible lors du contrôle
était `qt6-webengine-dev 6.8.2+dfsg-4` dans Trixie/main. Utiliser un paquet Debian
officiel ne suffit donc pas à déclarer ce navigateur adapté au Web quotidien.

**Navigator doit rester optionnel, désactivé dans les profils publics et ne pas
être défini comme navigateur par défaut. Firefox ESR est la solution de secours.**
L'interface rappelle cette limite. Toute navigation Web exige une autorisation
explicite pour la fenêtre/session. Le lancement normal affiche une page locale.
Une diffusion principale exige un moteur bénéficiant de mises à jour de sécurité
rapides, une politique de maintenance effective et une nouvelle validation.

## Fonctions réellement implémentées

- Barre d'adresse ou recherche explicite DuckDuckGo, précédent/suivant/recharger.
- Onglets, Ctrl+L, Ctrl+T, Ctrl+W ; zoom et plein écran F11.
- Page **Kyronext Start** locale servie par `kyronext://start/`, sans script
  distant ni requête réseau de vérification au lancement.
- Liens lus dans `/etc/kyronext/links.json`, surcharge XDG utilisateur commune.
  `kironext.be` est conservé tel quel ; aucun domaine n'est corrigé ou remplacé.
- Favoris ajoutés et consultables ; historique persistant facultatif, désactivé
  par défaut, limité à 500 visites et effaçable. Les URL d'historique sont stockées
  sans requête ni fragment pour limiter l'enregistrement de données sensibles.
- Téléchargements : destination choisie, progression, erreur, annulation et
  confirmation avant fermeture pendant un transfert. Aucun fichier exécuté.
- Session privée séparée, cookies en mémoire, suppression par site et blocage
  des cookies tiers. Le blocage par domaine est limité à la session.
- Géolocalisation, microphone et caméra demandés avec le domaine et refus
  par défaut. Les autres permissions, le partage d'écran et les accès directs
  au système de fichiers Web sont refusés dans cette première version.
- Erreurs HTTPS refusées, sans option d'exception. Ouverture volontaire d'une
  page HTML locale ; les autres fichiers locaux restent bloqués.
- À propos commun ; commande de secours pour ouvrir une adresse dans Firefox
  ESR s'il est installé. Aucun changement des associations MIME par défaut.

## Confidentialité et protections

Les profils WebEngine, y compris les fenêtres normales, sont en mémoire : pas
de cookies persistants ni de cache HTTP disque. Les fenêtres privées n'ajoutent
aucune entrée à l'historique persistant. Un favori explicitement enregistré ou
un téléchargement choisi par l'utilisateur reste toutefois sur disque.
L'application ne propose pas de gestionnaire de mots de passe et ne les écrit
pas dans ses préférences. L'authentification HTTP n'est proposée que sous HTTPS,
avec un champ de mot de passe masqué et sans mémorisation propre à Navigator.
Les journaux applicatifs persistants sont désactivés pour ce prototype.

Le sandbox reste actif. L'application refuse de démarrer comme administrateur,
ou avec des variables externes désactivant le sandbox, injectant des options
Chromium, changeant son exécutable ou activant le débogage distant. Aucun
certificat supplémentaire ni contournement HTTPS n'est installé. Les données
HTML générées sont échappées et la page Start impose une CSP sans scripts.

## Compilation ciblée

Dans Debian 13 isolé, dépendances officielles : `qt6-webengine-dev`, Qt 6
Widgets/Test, CMake, Ninja et un compilateur C++17. Leur installation est
réservée à la VM de construction ; aucune installation Windows n'est nécessaire.

Depuis le projet, lorsque le CMake racine déclare cette option :

```bash
cmake -S applications/suite -B build/navigator -G Ninja \
  -DKYRONEXT_BUILD_NAVIGATOR=ON -DBUILD_TESTING=ON
cmake --build build/navigator --target kyronext-navigator \
  test-navigator-policy test-navigator-engine --parallel 1
ctest --test-dir build/navigator -R navigator --output-on-failure
```

Le module n'est pas un programme HTML simulant un navigateur : le rendu, les
cookies, les certificats et les téléchargements passent par Qt WebEngine.

## Validation exécutée et limites

Compilation réussie dans `/home/builder/navigator-work/build`, Debian 13 AMD64,
Qt 6.8.2. Deux groupes CTest passent : politique URL/stockage et moteur réel.
Le second charge du HTML local, la page Start, contrôle un profil off-record,
refuse une permission native, refuse le certificat auto-signé d'un serveur TLS
local temporaire, ouvre la fenêtre complète et crée/ferme un onglet au clavier.
Le rendu sombre est contrôlé après chargement ; une capture blanche ne suffit
pas à faire réussir ce test.

Tests avec sandbox actif, utilisateur ordinaire, rendu logiciel hors écran.
Aucun drapeau de désactivation des protections n'a été utilisé. Les clés TLS
du test sont générées dans un dossier temporaire puis supprimées par le test ;
aucune clé de production n'est fournie dans les sources.

Preuves : `logs/navigator-tests.log`, `logs/navigator-start-preview.png`,
`logs/navigator-window-preview.png`.

Restent à vérifier : compatibilité des sites publics, ouverture réelle des trois
liens dans ce navigateur, transferts HTTP complets, périphériques caméra/micro,
accessibilité avec lecteur d'écran, intégration dans un Live et tests physiques.
Les favoris ne disposent pas encore d'un gestionnaire d'édition complet.
Le mode lecture, les résumés IA, le blocage avancé des traqueurs et la gestion de
mots de passe ne sont pas implémentés. Ces limites s'ajoutent à celle du support
de sécurité du moteur : ce prototype n'est pas annoncé comme stable ou sûr
pour des usages sensibles.

## Licences et références primaires

Créations Kyronext : droits réservés, licence de diffusion à définir. Qt 6,
Chromium et leurs bibliothèques tierces conservent leurs licences ; consulter
notamment `/usr/share/doc/libqt6webenginecore6/copyright` sur Debian.

- [Debian 13 : moteurs de navigateurs et sécurité](https://www.debian.org/releases/trixie/release-notes/issues.en.html#security-status-of-web-browsers-and-their-rendering-engines)
- [Qt 6.8 : QWebEnginePage](https://doc.qt.io/qt-6.8/qwebenginepage.html)
- [Qt 6.8 : profils et mode off-record](https://doc.qt.io/qt-6.8/qwebengineprofile.html)
- [Qt 6.8 : permissions](https://doc.qt.io/qt-6.8/qwebenginepermission.html)
- [Qt : considérations de sécurité](https://doc.qt.io/qt-6/qtwebengine-security.html)
