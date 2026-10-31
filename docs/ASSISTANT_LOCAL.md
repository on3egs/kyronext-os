# Kyronext Intelligence — conception locale

## Référence étudiée

Les annonces officielles d'openKylin décrivent un assistant intégré au bureau,
capable d'appeler des applications et fonctions système, avec recherche
intelligente transversale et interfaces vocales. Kyronext reprend ces idées de
fonctionnement — commandes structurées, recherche locale et actions visibles —
sans copier le code, le thème, les marques ni les services d'openKylin.

- [openKylin 3.0, annonce officielle](https://www.openkylin.top/news/4099-en.html)
- [Assistant Marvis proposé dans le magasin openKylin](https://www.openkylin.top/news/4179-en.html)

## Architecture Kyronext

1. Un catalogue d'actions documentées expose uniquement des fonctions
   utilisateur : ouvrir une application installée, chercher dans les dossiers
   sélectionnés, lire l'état du matériel ou contrôler AMP.
2. Les actions sont validées par une liste autorisée et passent par des API
   utilisateur. L'assistant ne lance pas de commandes shell arbitraires et ne
   reçoit jamais de privilèges administrateur permanents.
3. La compréhension locale sera un moteur facultatif. Si le moteur ou le modèle
   manque, les fonctions système et les commandes directes resteront utilisables.
4. Les recherches restent locales et respectent les permissions Linux. Les
   dossiers indexés sont choisis par la personne.
5. La voix et le texte partagent les mêmes intentions, confirmations et règles
   d'autorisation. La voix est désactivable.

## Modèle CPU léger à évaluer

Qwen2.5-0.5B-Instruct-GGUF est une piste de prototype : son dépôt officiel
indique la licence Apache-2.0 et des fichiers quantifiés d'environ 429 à 491 Mo
en Q4. Ce poids ne comprend pas le moteur d'inférence ni sa mémoire de travail.
Le modèle n'est pas garanti excellent en français et ses performances sur le
Core i5 ancien du HP 2540p ne sont pas encore mesurées.

Debian 13 Trixie ne propose pas actuellement llama.cpp dans sa suite stable
AMD64 ; les résultats du catalogue Debian trouvés concernent testing/unstable.
Kyronext ne doit donc pas installer de paquet d'une autre suite. Avant d'inclure
un modèle, il faudra empaqueter un moteur maintenu depuis une révision épinglée,
vérifier licences et mises à jour, puis mesurer latence et mémoire sur le HP.

- [Modèle GGUF officiel Qwen](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF)
- [Recherche Debian pour llama.cpp](https://packages.debian.org/search?arch=amd64&keywords=llama.cpp)

La correction orthographique de textes pourra utiliser un correcteur français
local éprouvé en complément. Une petite LLM ne doit pas être présentée comme
garantissant une orthographe parfaite.

## Voix

Une voix synthétique Kyronext peut être fournie avec Piper après vérification de
la licence de la voix. Le profil « Manix » nécessite un enregistrement ou modèle
fourni/autorisé par Manix. Une voix de Guy Chapellier ou une imitation de cette
voix nécessite une licence et l'accord du titulaire concerné ; aucun clone ou
enregistrement protégé n'est intégré par défaut.

## Jeux sur le Jetson AGX

La rubrique « Jeux » ne doit contenir que les jeux réellement découverts sur un
Jetson explicitement appairé. Une simple présence sur le réseau ne rend pas une
machine fiable. La découverte, l'inventaire et le lancement distant seront
désactivés sans appairage ; l'inventaire initial sera en lecture seule. Le
lancement en plein écran ne sera ajouté qu'après choix d'un protocole compatible
avec le système réellement installé sur l'AGX et validation des permissions.

### Vérification de l'ISO openKylin fournie

L'image montée en lecture seule depuis `G:\openKylin-Desktop-V3.0-20260918-x86_64.iso` a un manifeste Live contenant `kylin-aiassistant`, `ukui-search`, trois paquets de modèles déclarés cloud (DeepSeek-R1, DeepSeek-V3, Qwen-Plus), et des paquets de modèles vocaux ASR/TTS sur l'appareil. Cette distinction confirme qu'une présence de composant IA ne prouve pas un LLM local hors ligne. Kyronext séparera clairement les deux modes et gardera toute connexion distante facultative.
