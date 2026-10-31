## État d'avancement — 31 octobre 2026

Candidate Live propre construite : `releases/20261031T175731-clean-live/` — 2 451 800 064 octets, SHA-256 `ad67f47ede2170f5a9fcc2b029b0d139cca5b8d8282da380f942c913927ccadd`. Les fichiers de clé SSH et de hash du mot de passe sont absents du système Live. QEMU WHPX a atteint l'accueil Kyronext en BIOS et UEFI. QEMU TCG a affiché un rendu brouillé ou noir et n'a pas atteint le bureau. Ancienne candidate privée `20261031T1542-legacy-fbdev` conservée séparément. Voir `TEST_REPORT.md` et le rapport dans le répertoire de release.

À faire avant toute annonce matérielle : essais physiques sur HP EliteBook 2540p, réseau, audio, GPU et capture d'écran. Aucun test matériel réel n'est déduit des VM. Calamares n'est pas activé et l'ISO n'est pas un installateur. Les applications non intégrées restent des prototypes.

---

# Architecture et feuille de route

Kyronext OS 0.1 Developer Preview — Kyronext Systems — By Manix.

Base : Debian 13 Trixie AMD64, noyau officiel, KDE/SDDM et X11, systemd, PipeWire, NetworkManager. Sources live-build dans iso/, travaux dans build/, preuves dans logs/, artefacts dans releases/.

## État réel

A : audit Windows et constructeur Debian isolé réalisés. B : Debian Live KDE construit. C : identité Kyronext et Welcome intégrés. D : candidate propre `20261031T175731-clean-live` construite et vérifiée par empreinte. E : bureau atteint sous QEMU WHPX en BIOS et UEFI. La candidate privée antérieure a aussi atteint le bureau sous VirtualBox dans les profils BIOS/UEFI et standard/Legacy ; Monitor a été ouvert en UEFI. TCG reste défaillant sur cet hôte. Détails dans `TEST_REPORT.md`.

La candidate propre reste une ISO Live de démonstration, pas un installateur ; son compte verrouillé n'a pas de sudo. L'historique Git local contient une ancienne empreinte de mot de passe et ne doit pas être publié sans nettoyage. QEMU TCG, Calamares et le HP réel restent à valider.
## Modules indépendants

Le Legacy Driver Manager collecte les identifiants, pilotes et modules candidats sans modifier la machine. Guardian Hardware Passport produit un rapport JSON hors ligne non privilégié avec statuts non vérifiés explicites. Dans l'ISO, le diagnostic texte local est sauvegardé automatiquement. Voir docs/ARCHITECTURE.md et docs/GUARDIAN.md.

Adaptive OS proposera des profils explicables et un choix manuel. Phoenix Recovery commencera par un environnement de dépannage avant la restauration des mises à jour. AI Doctor restera local, facultatif, sans rôle administrateur permanent et avec confirmation explicite des actions sensibles. Ces fonctions ne sont pas annoncées opérationnelles en 0.1.

Les sources comprennent désormais un premier Kyronext Recovery CLI, limité à un
rapport local en lecture seule, ainsi qu'un Kyronext Update qui ne fait qu'une
simulation APT. Aucun des deux n'est empaqueté dans l'ISO 0.1 ; voir les limites
dans `applications/recovery/VALIDATION.md` et `applications/suite/update/README.md`.

## Version suivante

Les extensions suite logicielle et portail sont intégrées dans
[APPLICATION_SUITE.md](docs/APPLICATION_SUITE.md). Priorités : préserver l'ISO,
centraliser les liens officiels, faire évoluer Welcome, valider le framework
commun et les premiers outils natifs, notamment AMP et Paint. Les autres
applications sont planifiées progressivement, sans les présenter comme livrées.

Les capacités système complémentaires à prioriser sont classées dans
[OUTILS_PRIORITAIRES.md](docs/OUTILS_PRIORITAIRES.md). Les améliorations de
présentation communes aux applications natives, avec leurs statuts de
validation, sont suivies dans [AMELIORATIONS_GRAPHIQUES.md](docs/AMELIORATIONS_GRAPHIQUES.md).

Les sources de la suite contiennent maintenant des premières versions de
Kyronext Backup (archive TAR locale, en clair, sans restauration) et Kyronext
Disk Health (lecture SMART facultative, en lecture seule). Leur présence dans
le CMake de développement ne signifie pas qu'elles sont présentes dans l'ISO
0.1 ou validées sur le HP. Les rapports donnent les tests réellement exécutés.
Le framework expose aussi les panneaux d'état vide et une préférence globale de
réduction des effets visuels, reprise par AMP. Ces ajouts restent à compiler,
tester et intégrer dans un paquet avant d'être annoncés comme livrés.

Voir docs/ROADMAP_0.2.md. Priorité : défauts graphiques observés, cohérence du démarrage et du panneau, profils de compatibilité testés puis intégration empaquetée du passeport. Calamares ne sera expérimenté qu'après validation de la base et exclusivement sur disques virtuels jetables.

## Consolidation demandée — 31 octobre 2026

- Compte Live `kyronext` avec sudo qui demande le secret temporaire configuré pour cette image privée; le compte root reste verrouillé.
- Capture : Spectacle est explicitement installé et Kyronext Capture l'ouvre si la capture Qt/X11 n'est pas autorisée.
- Assistant : catalogue d'actions locales autorisées, interface française, modèle CPU facultatif après mesure et validation sur HP 2540p.
- Inspiration openKylin : ouverture d'applications et réglages, recherche locale, actions inter-applications avec autorisations, et parcours vocal. Voir `docs/ASSISTANT_LOCAL.md`; aucun code ou média openKylin n'est repris.
- Jeux AGX : raccourcis uniquement à partir d'un inventaire en lecture seule, après appairage explicite. Aucun scan réseau automatique.
