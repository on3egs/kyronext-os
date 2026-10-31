# Suite Kyronext — intégration progressive

Kyronext Systems — By Manix. Version des applications : 0.1.0.

## Continuité et périmètre

Les sources de l'ISO fonctionnelle sont conservées dans Git. Le point
`83cdb2a` précède l'extension de la suite. Les ISO déjà livrées restent intactes.
L'image publique `143045` est une Developer Preview Live, sans installateur.
L'image de travail `162639` ajoute Calculator, Notes et les écrans système.
Le framework C++, Paint, AMP et les nouveaux liens sont développés séparément :
leur présence dans les sources ne signifie pas leur présence dans une ISO.

## Architecture retenue

`applications/suite/` utilise C++17, Qt 6 et CMake. Le premier socle emploie
Qt Widgets : composants maintenus, navigation clavier et dialogues standards,
sans dépendance supplémentaire à Qt Quick. Des composants QML pourront compléter
ce socle après validation ; ce choix ne remplace pas le bureau KDE.

La bibliothèque `KyronextUi` partage thème rouge/ambre, préférences utilisateur,
gestion des erreurs, journal local et fenêtre À propos. Les fenêtres restent
redimensionnables et n'imposent aucune animation. Les traductions passent par
les mécanismes Qt ; les traductions complètes restent à produire.

Notes et Calculator existent déjà en Python/PyQt6 : ils sont conservés.
Un adaptateur Python commun fournit leur À propos avec les mêmes métadonnées.
Cet adaptateur n'est pas un binding du code C++. Les bibliothèques tierces et
leurs licences restent identifiées séparément des créations Kyronext.

## Applications prioritaires

| Composant | Première version | Suite prévue |
|---|---|---|
| Notes | UTF-8, sauvegarde atomique, recherche, confirmation des modifications | Remplacement, onglets, lignes, récupération puis Markdown |
| Calculator | Analyseur décimal borné, opérations standards, clavier | Historique, scientifique, programmation, unités |
| Paint | QImage/QPainter, pinceau, gomme, couleurs, onglets, undo, PNG/JPEG/BMP | Sélections, formes, texte, recadrage, transformations puis calques |
| AMP | QMediaPlayer/QAudioOutput, lecture locale, playlists, volume, métadonnées | Égaliseur DSP réel et visualisations sur échantillons réels |
| Firefox ESR | Intégré depuis Debian, profil Kyronext facultatif avec page locale et CSS | Vérifier le rendu après mises à jour Firefox ; Firefox standard reste disponible |
| Navigator | C++/Qt WebEngine, profil privé, liens centralisés, cookies mémoire, refus TLS et permissions | Expérimental ; tests moteur puis validation Live avant navigateur par défaut |

AMP n'affiche pas de faux égaliseur : les dix bandes devront modifier le signal
et être testées par mesures avant livraison. Les formats audio dépendent des
codecs effectivement présents. Aucun skin, logo ou code propriétaire Winamp
n'est repris. Paint doit modifier réellement les pixels ; ses limites mémoire
et son historique borné font partie de la validation.

## Portail et liens officiels

La source système est `/etc/kyronext/links.json`, avec préférences XDG utilisateur.
Les références restent exactement `https://www.kitt-franco-belge.be`,
`https://kironext.be` et `https://kyronext.eu`. Les erreurs de vérification sont
conservées avec leur date ; aucun domaine n'est substitué automatiquement.
L'ouverture se fait sur demande, par le navigateur configuré, sans shell.
L'accueil et l'aide locale doivent fonctionner sans Internet. Les raccourcis
peuvent être masqués. Un navigateur Debian maintenu reste nécessaire avant
validation de la prochaine ISO avec accès Web ; Navigator n'est pas imposé.

## Ordre de réalisation des extensions

1. Préserver les livraisons et poursuivre les tests de l'ISO.
2. Vérifier les liens, configurer les raccourcis et enrichir l'accueil existant.
3. Compiler et tester le framework, Paint et AMP dans Debian isolé.
4. Produire des paquets expérimentaux ; tester installation, mise à jour et
   suppression dans la VM avant toute sélection dans une ISO.
5. Faire évoluer Notes et Calculator, puis Monitor, Files, Gallery et Capture.
6. Introduire progressivement les modules du portail décrits ci-dessous.
7. Tester hors ligne, au clavier et sur matériel modeste ; publier les limites.

## Modules prévus, non annoncés opérationnels

- Launchpad : catalogue issu des véritables fichiers `.desktop`, catégories,
  favoris, recherche ; aucune entrée « installée » pour un logiciel absent.
- Kyronext Start : accueil local du futur navigateur, raccourcis configurables,
  historique facultatif et choix d'une autre page d'accueil.
- Connect : inventaire local en lecture seule comme première étape. Découverte
  réseau désactivée par défaut, consentement, appairage et authentification
  nécessaires avant les commandes distantes ; aucun changement Jetson.
- Media Center : recherche limitée aux dossiers choisis ; AMP, Video, Gallery
  et Paint restent des applications indépendantes.
- Universal Search : index local facultatif et reconstructible, permissions
  Linux respectées, aucun envoi cloud et aucun parcours privé imposé.
- Smart Open : associations MIME et choix de l'utilisateur conservés.
- Command Center, Smart Actions et Assistant : API internes documentées et
  autorisations explicites ; aucun rôle administrateur permanent.
- First Run : étapes facultatives, aucun compte obligatoire ni modèle IA
  téléchargé automatiquement ; dépend de l'installation future sur disque.
- Personalization : réglages réversibles, profils lisibles et mode ancien PC.
- Help Center : documentation locale et F1 ; les fonctions non disponibles,
  notamment l'installation sur disque, doivent être clairement signalées.
- Safe Recovery/Phoenix : diagnostic avant restauration ; aucune réparation
  risquée automatique et aucune restauration revendiquée avant ses tests.

## Qualité et livraison

Les solutions KDE de secours restent installées. Chaque application est
autonome, non privilégiée, sans télémétrie imposée. Les paquets Debian officiels
fournissent Qt et les moteurs. Une compilation ou un bouton visible ne constitue
pas une preuve de fonctionnement : les rapports distinguent tests unitaires,
tests graphiques, traitement audio réel, essais en Live et essais physiques.
