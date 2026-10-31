# Liens Kyronext et accueil existant

Les trois adresses fournies sont conservées exactement, sans correction ni
substitution par les éventuelles redirections :

- `https://www.kitt-franco-belge.be`
- `https://kironext.be`
- `https://kyronext.eu`

Le fichier central est `/etc/kyronext/links.json` et sa version source se trouve
dans `iso/config/includes.chroot/etc/kyronext/links.json`.
Les résultats datés du contrôle explicite sont dans [DOMAIN_REPORT.md](DOMAIN_REPORT.md)
et `logs/links-domain-validation.json`. Un échec depuis ce poste ne prouve pas
qu'un site est inaccessible partout ; les adresses non confirmées restent visibles
avec leur statut daté dans l'infobulle. Aucune connexion n'est testée au lancement.

## Accueil et raccourcis

L'accueil existant conserve ses quatre actions locales : Fichiers, Terminal,
Paramètres et Diagnostic matériel. Trois boutons affichent le nom et le domaine
de destination ; l'infobulle indique l'adresse complète et le contrôle daté.
Le navigateur s'ouvre seulement après un clic. **Afficher les liens utiles** et
**Choisir les liens** permettent de les masquer ; les préférences sont enregistrées
par utilisateur dans QSettings (`Kyronext`, `Welcome`).

Trois lanceurs du menu utilisent un identifiant fixe, jamais une commande extraite
d'un fichier de configuration :

```bash
python3 /usr/local/bin/kyronext-open-link --list
python3 /usr/local/bin/kyronext-open-link kitt-franco-belge
python3 /usr/local/bin/kyronext-open-link kironext-be
python3 /usr/local/bin/kyronext-open-link kyronext-eu
```

Les adresses sont ouvertes avec QDesktopServices/QUrl, sans shell. Seules les URL
HTTPS complètes sont acceptées, sans identifiants, caractères de contrôle ni
syntaxe d'hôte ambiguë. Aucun scan de réseau, téléchargement de pilote ou modèle,
installation automatique ni changement de navigateur par défaut.

Sans navigateur reconnu installé, un dialogue explique la situation et affiche
l'adresse ; les outils locaux et le bureau restent utilisables. Le navigateur
installé doit également être correctement défini comme navigateur par défaut.
La détection actuelle reconnaît Firefox ESR/Firefox, Chromium, Falkon, Epiphany,
Google Chrome et Brave par leur exécutable ; un navigateur personnalisé sous un
autre nom peut nécessiter une extension de cette détection.

Les ISO précédemment livrées ne contiennent pas de navigateur. Ces boutons ne
doivent donc pas être présentés comme testés sur Internet dans ces images.
Firefox ESR depuis les paquets officiels Debian est prévu pour une construction
ultérieure ; cette livraison de sources ne modifie aucune ISO existante.

## Réglages utilisateur XDG

Des surcharges facultatives sont lues dans
`$XDG_CONFIG_HOME/kyronext/links.json`, sinon `~/.config/kyronext/links.json`.
`XDG_CONFIG_HOME` doit être absolu. Exemple pour masquer un lien :

```json
{
  "schema_version": 1,
  "links": [
    {"id": "kironext-be", "visible": false}
  ]
}
```

Les champs modifiables sont `label`, `url` et `visible`, uniquement pour les
identifiants déjà définis par le système. Une adresse personnalisée est marquée
« À confirmer » et n'hérite pas du statut vérifié de l'adresse d'origine.
Un masquage dans ce fichier reste prioritaire sur la préférence QSettings.
Un JSON invalide ou une surcharge dangereuse est ignoré avec avertissement :
les valeurs système valides sont conservées. Aucun réglage utilisateur n'est
écrit automatiquement dans `/etc`.

## Contrôles

```bash
python3 applications/links/test_links.py
python3 applications/links/check_welcome_ui.py
```

Les tests sont locaux : URI dangereuses, JSON invalide, surcharge XDG et masquage,
absence de navigateur, transmission simulée d'une URL comme donnée. Le contrôle
Qt ouvre l'accueil réel hors écran, conserve les quatre cartes locales et teste
les masquages QSettings et les clics avec QDesktopServices simulé. Il n'installe
et ne lance pas de navigateur. Résultats : `logs/welcome-links-ui-check.json` ;
capture : `logs/welcome-links-preview.png`.
Le test vérifie aussi l'absence de débordement horizontal à 680 × 460.
Sous Windows, le contrôle hors écran lit une police système existante pour ses
captures ; cette police n'est ni copiée ni distribuée dans l'OS.

Pour répéter volontairement le contrôle Internet avec validation TLS normale :

```bash
python3 applications/links/audit_domains.py
# Facultatif : reporter ce nouveau contrôle daté dans la configuration source.
python3 applications/links/audit_domains.py --update-config
```

Ce script de développement utilise trois vérifications parallèles, des délais
de huit secondes par requête et la validation standard des certificats. Il ne
fait pas partie d'un démarrage automatique de session.
