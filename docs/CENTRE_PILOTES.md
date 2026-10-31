# Pilotes et connexion Kyronext

**Code développé et contrôlé ; pas encore livré dans une nouvelle ISO.**
Le constructeur Debian isolé ne répond pas en SSH. Aucun redémarrage du PC hôte
ou de l'AGX, aucune modification de leur réseau. La clé USB n'est pas mise à jour.

## Interface

Lancer **Pilotes et connexion Kyronext** dans le menu ou depuis l'Accueil.
Le diagnostic local démarre dans un processus séparé, sans droits administrateur.
Palette sombre et rouge, cartes d'état, focus clavier, onglets et vues défilantes.

- Matériel : identifiants PCI/USB, module lié, candidats du noyau, familles de
  firmwares à examiner dans les infobulles. Les clés USB ath9k-htc, carl9170 et
  zd1211rw ont des associations de paquets spécifiques. Une correspondance n'est pas une
  recommandation d'installation automatique.
- Connexion et réserve : activation Wi-Fi, annulation radio, réinstallation d'un
  firmware sélectionné et vérification SHA-256 des archives.
- Rapport : réseau, graphique, audio, messages noyau, fichiers de firmware
  annoncés par modinfo et présence locale. Un fichier absent peut être optionnel
  ou destiné à un autre modèle : il ne prouve pas une panne.
- Export JSON privé, sans écrasement. MAC et IPv4 des messages filtrés sont
  masquées. Relire les autres informations techniques avant partage.

## Réparation et retour arrière

Confirmation et authentification Polkit avant modification. La réparation
sauvegarde l'état radio, active le Wi-Fi et lève ses blocages logiciels. Elle ne
supprime aucun profil, ne change aucun mot de passe et ne redémarre aucun service.
Un blocage matériel demande le bouton du PC. Une activation réussie ne garantit
ni association Wi-Fi ni accès Internet.

Snapshots root dans `/var/lib/kyronext/driver-manager/`. L'identifiant de la
dernière réparation reste dans les préférences de la fenêtre après fermeture.
Le retour est refusé après un autre boot : les identifiants rfkill peuvent changer.
Il demande confirmation car l'état précédent peut être Wi-Fi coupé. Une réparation
qui échoue tente immédiatement sa restauration et signale un retour incomplet.

CLI : `kyronext-drivers diagnose`, `kyronext-drivers verify-offline`,
`sudo kyronext-drivers repair-wifi`,
`sudo kyronext-drivers restore-wifi IDENTIFIANT`.

## Réserve et limites

Le build télécharge via APT les versions exactes des paquets sélectionnés et de
leurs dépendances installées, avec alternatives et fournisseurs virtuels. Une
dépendance non résolue arrête le build. Manifeste : nom, version, fichier, SHA-256.
Les empreintes sont vérifiées à la construction puis avant réinstallation.
Les noms des paquets firmware ont été contrôlés dans l'index officiel Debian
Trixie ([catalogue firmware](https://packages.debian.org/source/trixie/all/firmware-nonfree)).
Les conditions de redistribution dépendent du paquet et de chaque fichier ; voir
`LICENSES/FIRMWARE.md` et les notices `copyright` installées avec les paquets.

La réinstallation est limitée aux firmwares sélectionnés, avec simulation APT,
confirmation et authentification. Aucun téléchargement, suppression ou mise à
niveau, aucun module déchargé, aucun reboot automatique. Une version différente
déjà installée provoque un refus, sans rétrogradation. Le retour arrière radio
n'est pas une restauration générale de paquets. Les pilotes propriétaires,
le noyau, le BIOS et les firmwares flashables ne sont pas modifiés par cet outil.

Budget : tailles installées des paquets matériels sélectionnés **plus archives
de la réserve < 1 000 000 000 octets**. Le noyau déjà présent et les dépendances
système déjà installées ne sont pas comptés comme nouveaux pilotes. Les dépendances
dans la réserve sont comptées dans ses archives. ISO entière < 16 000 000 000 octets.
Tout dépassement bloque la livraison ; tailles finales encore non mesurées.

## Vérification effectuée

17 tests automatisés passent : intégrité des archives, chemins sortants refusés,
récupération invalide ou autre boot refusée, état radio inconnu sans modification,
réparation et retour simulés, refus des sauvegardes qui ne sont pas des fichiers
ordinaires ou qui sont des liens symboliques,
récupération après échec, dépendances alternatives,
fournisseurs virtuels, cycles, dépendances manquantes et associations précises
pour les firmwares USB ath9k-htc, carl9170 et zd1211. 6 tests de budgets passent.
Commandes reproductibles : `py -3 -m unittest tests.test_driver_manager -v`,
`py -3 -m unittest tests.test_release_budgets -v`, `py -3 tests/check_sources.py`.
Réseau/APT simulés : aucun essai de réparation réelle sur le PC hôte.

Captures Qt hors écran avec données **simulées**, présentation contrôlée en
1080×740 et 800×600 : `logs/driver-center-preview/`. Ce n'est pas une validation
du Live Linux ni du matériel. Restent la construction Linux, la réserve réelle,
les mesures finales, le démarrage BIOS/UEFI, Polkit, SSH et des essais physiques.
