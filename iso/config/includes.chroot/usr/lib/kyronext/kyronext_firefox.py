"""Profil Firefox Kyronext isolé, sans extension ni modification de sécurité."""
from contextlib import contextmanager
import html
import importlib.util
import json
import os
import re
from pathlib import Path
import shutil
import tempfile
import unicodedata
from urllib.parse import unquote, urlsplit


ASSETS = Path(__file__).resolve().parents[2] / 'share/kyronext/firefox'


def data_directory():
    configured = os.environ.get('XDG_DATA_HOME', '')
    root = Path(configured) if configured and Path(configured).is_absolute() else Path.home() / '.local/share'
    return root / 'kyronext/firefox'


def validate_argument_url(value):
    if not isinstance(value, str) or not value or len(value) > 4096:
        raise ValueError('Adresse Web absente ou trop longue.')
    if any(c.isspace() or unicodedata.category(c).startswith('C') for c in value):
        raise ValueError('Adresse contenant des caractères de contrôle ou des espaces.')
    if any(unicodedata.category(c).startswith('C') for c in unquote(value)) or any(c in value for c in '\\<>"`'):
        raise ValueError('Adresse ambiguë ou contenant des caractères interdits.')
    try:
        parts = urlsplit(value)
        if parts.scheme not in ('http', 'https') or not parts.hostname or parts.username is not None or parts.password is not None:
            raise ValueError('Seules les adresses HTTP/HTTPS sans identifiants sont acceptées.')
        if parts.port is not None and not 1 <= parts.port <= 65535:
            raise ValueError('Port invalide.')
        host = parts.hostname.encode('idna').decode('ascii')
        if '%' in host or any(c not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-:' for c in host):
            raise ValueError('Nom de serveur invalide.')
        if ':' not in host and any(not re.fullmatch(r'[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?', part) for part in host.rstrip('.').split('.')):
            raise ValueError('Nom de serveur invalide.')
    except (UnicodeError, ValueError) as error:
        raise ValueError('Adresse Web invalide : ' + str(error)) from error
    return value


def load_links():
    path = Path(__file__).with_name('kyronext_links.py')
    spec = importlib.util.spec_from_file_location('kyronext_firefox_links', path)
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    return module.load_links()


def start_html(links, warnings):
    cards = []
    for entry in links:
        if not entry.get('visible', True): continue
        # Valider à nouveau même si un appelant de test fournit sa propre liste.
        try:
            url = validate_argument_url(entry.get('url'))
            if not url.startswith('https://'): continue
        except ValueError:
            continue
        label = html.escape(str(entry.get('label', 'Lien Kyronext')))
        address = html.escape(url, quote=True)
        verification = entry.get('verification')
        confirmed = isinstance(verification, dict) and verification.get('status') == 'verified'
        state = 'Adresse vérifiée lors de la préparation' if confirmed else 'Adresse à confirmer'
        cards.append(f'<a class="card" href="{address}" rel="noreferrer"><strong>{label}</strong><span>{address}</span><small>{state}</small></a>')
    notices = ''.join('<p>' + html.escape(str(message)) + '</p>' for message in warnings)
    return '''<!doctype html>
<html lang="fr"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'self'; form-action https://duckduckgo.com; base-uri 'none'; object-src 'none'">
<title>Kyronext Start — Firefox</title><link rel="stylesheet" href="start.css"></head>
<body><main><header><span class="mark" aria-hidden="true">K</span><p>KYRONEXT SYSTEMS — BY MANIX</p><small>PAGE LOCALE · FIREFOX ESR</small></header>
<h1>Bienvenue dans votre espace.</h1><p class="intro">Vos liens, votre navigateur, votre liberté.</p>
<form action="https://duckduckgo.com/" method="get"><label for="search">Rechercher avec DuckDuckGo</label><div class="search"><input id="search" name="q" type="search" placeholder="Que souhaitez-vous découvrir ?" maxlength="2048" required><button type="submit">Rechercher</button></div></form>
<section aria-labelledby="links"><h2 id="links">KYRONEXT NETWORK</h2><div class="cards">''' + ''.join(cards) + '''</div></section>
<section class="note"><h2>Vous gardez le choix.</h2><p>Cette page reste disponible hors ligne. Les recherches et sites s'ouvrent uniquement sur votre demande et nécessitent Internet.</p><p>La barre d'adresse de Firefox reste disponible. Favoris, historique et téléchargements se trouvent dans ses menus habituels.</p><p>Modifiez la page d'accueil dans les paramètres de Firefox. Pour consulter Firefox sans ce profil, utilisez l'action « Firefox normal » du lanceur Kyronext.</p></section>
<aside role="status">''' + notices + '''</aside><footer>Firefox reste Firefox. Le thème Kyronext est une personnalisation locale indépendante.</footer></main></body></html>'''


def atomic_write(path, content, mode=0o600):
    path = Path(path)
    if path.is_symlink(): raise ValueError('Un fichier de personnalisation symbolique est refusé.')
    path.parent.mkdir(parents=True, exist_ok=True, mode=0o700)
    fd, temporary = tempfile.mkstemp(prefix='.' + path.name + '.', dir=path.parent)
    try:
        with os.fdopen(fd, 'w', encoding='utf-8', newline='\n') as stream:
            stream.write(content); stream.flush(); os.fsync(stream.fileno())
        os.chmod(temporary, mode); os.replace(temporary, path)
    finally:
        if os.path.exists(temporary): os.unlink(temporary)


def refresh_start(root, links, warnings, assets=ASSETS):
    page = Path(root) / 'start/index.html'
    atomic_write(page, start_html(links, warnings))
    stylesheet = page.with_name('start.css')
    if not stylesheet.exists(): atomic_write(stylesheet, (Path(assets) / 'start.css').read_text(encoding='utf-8'))
    return page


def prepare_profile(root, homepage, assets=ASSETS):
    """Ne modifie jamais prefs.js, user.js ou le CSS d'un profil déjà présent."""
    root = Path(root); root.mkdir(parents=True, exist_ok=True, mode=0o700)
    profile = root / 'profile'
    if profile.exists():
        if profile.is_symlink() or not profile.is_dir(): raise ValueError('Le profil doit être un répertoire ordinaire.')
        return profile, False
    staging = Path(tempfile.mkdtemp(prefix='.profile-new-', dir=root))
    try:
        (staging / 'chrome').mkdir(mode=0o700)
        prefs = {
            'browser.startup.page': 1,
            'browser.startup.homepage': Path(homepage).resolve().as_uri(),
            'toolkit.legacyUserProfileCustomizations.stylesheets': True,
            'extensions.activeThemeID': 'firefox-compact-dark@mozilla.org',
            'browser.shell.checkDefaultBrowser': False,
        }
        content = '// Initialisation Kyronext unique. Modifiable ensuite dans Firefox.\n'
        content += ''.join('user_pref(' + json.dumps(key) + ', ' + json.dumps(value) + ');\n' for key, value in prefs.items())
        atomic_write(staging / 'prefs.js', content)
        atomic_write(staging / 'chrome/userChrome.css', (Path(assets) / 'userChrome.css').read_text(encoding='utf-8'))
        atomic_write(staging / '.kyronext-profile.json', json.dumps({'schema_version': 1, 'created_by': 'Kyronext Systems — By Manix'}))
        os.rename(staging, profile)
    finally:
        if staging.exists(): shutil.rmtree(staging)
    return profile, True


@contextmanager
def launcher_lock(root):
    """Un seul lanceur Kyronext ; Firefox conserve aussi ses propres verrous."""
    import fcntl
    root = Path(root); root.mkdir(parents=True, exist_ok=True, mode=0o700)
    path = root / '.launcher.lock'
    if path.is_symlink(): raise ValueError('Verrou de profil symbolique refusé.')
    with path.open('a+b') as stream:
        try: fcntl.flock(stream.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError as error: raise RuntimeError('Le profil Firefox Kyronext est déjà ouvert. Utilisez sa fenêtre existante ou Firefox normal.') from error
        try:
            lock = root / 'profile/.parentlock'
            if lock.is_symlink(): raise RuntimeError('Le verrou Firefox nécessite une vérification manuelle ; aucun verrou n’a été supprimé.')
            if lock.is_file():
                with lock.open('r+b') as firefox_lock:
                    try: fcntl.lockf(firefox_lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    except BlockingIOError as error: raise RuntimeError('Ce profil est utilisé par Firefox. Utilisez la fenêtre existante ; aucun verrou n’a été supprimé.') from error
                    else: fcntl.lockf(firefox_lock.fileno(), fcntl.LOCK_UN)
            yield
        finally:
            fcntl.flock(stream.fileno(), fcntl.LOCK_UN)


def command(binary, profile, urls, plain=False, homepage=None):
    checked = [validate_argument_url(url) for url in urls]
    args = [str(binary), *([] if plain else ['--no-remote', '--profile', str(profile)])]
    # Firefox ESR peut afficher un onglet vide lors du premier démarrage même
    # si browser.startup.homepage vient d'être écrit dans prefs.js. Passer
    # explicitement le fichier local garantit Kyronext Start hors ligne.
    if not plain and not checked and homepage is not None:
        page = Path(homepage)
        if page.is_symlink() or not page.is_file():
            raise ValueError('La page de démarrage locale est absente ou invalide.')
        args.extend(['--new-window', page.resolve().as_uri()])
    args.extend(checked)
    return args
