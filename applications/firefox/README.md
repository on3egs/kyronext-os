# Firefox — thème Kyronext

Kyronext OS installe Firefox ESR officiel Debian **en plus de Kyronext Navigator**.
Firefox garde son moteur, son bac à sable, ses avertissements HTTPS et ses mises
à jour Debian. Il n'est pas remplacé ni modifié par un fork.

Le lanceur « Firefox — thème Kyronext » prépare une seule fois un profil dédié
dans `~/.local/share/kyronext/firefox/`. Il ajoute une page locale Kyronext Start,
la palette sombre/rouge, et les liens du fichier centralisé. La page fonctionne
hors ligne ; les sites et les recherches ne s'ouvrent qu'après une action de
l'utilisateur. Les liens de l'utilisateur restent modifiables dans le fichier
XDG `~/.config/kyronext/links.json`.

Le profil reste un profil Firefox normal, séparé de tout profil existant.
Firefox standard reste disponible depuis son propre lanceur et via
`kyronext-firefox --plain`. Le profil dédié n'est pas réécrit après sa première
création ; l'utilisateur peut choisir une autre page d'accueil et remplacer ou
désactiver le CSS. `userChrome.css` est une personnalisation facultative de
l'interface Firefox et peut devoir être ajustée après une mise à jour majeure.
La page Start est indépendante de ce CSS.

Le lanceur refuse root, les schémas non HTTP(S), les identifiants dans une URL,
les arguments ambigus et les options shell. Il ne supprime aucun verrou Firefox,
n'enregistre aucun mot de passe et n'ajoute aucune option de désactivation du
sandbox. Le mode `--plain` lance Firefox sans toucher au profil Kyronext.

## Tests

```bash
python3 -m unittest applications.firefox.test_firefox_profile -v
```

Les tests valident l'échappement HTML, les liens, les URL, l'absence
d'injection d'options, la préservation des réglages Firefox existants et le
refus de supprimer des verrous actifs. Les deux tests de verrouillage POSIX sont
ignorés sur Windows et s'exécutent dans la VM Debian. L'apparence doit aussi être
vérifiée dans la session graphique de l'ISO ; un test de génération HTML ne
prouve pas le rendu Firefox.

Firefox est distribué et maintenu par Debian. L'identité Kyronext ne signifie
pas que Mozilla a conçu ou approuvé ce thème.
