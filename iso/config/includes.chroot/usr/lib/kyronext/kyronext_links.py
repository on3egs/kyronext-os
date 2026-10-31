"""Liens Kyronext validés localement ; aucun accès réseau lors du chargement."""
import json
import os
from pathlib import Path
import re
import shutil
from urllib.parse import unquote, urlsplit
import unicodedata


SYSTEM_CONFIG = Path(__file__).resolve().parents[3] / 'etc/kyronext/links.json'
MAX_CONFIG_BYTES = 128 * 1024
ID_PATTERN = re.compile(r'[a-z][a-z0-9-]{0,47}')


class LinkConfigError(ValueError):
    pass


def has_controls(value):
    return any(unicodedata.category(char).startswith('C') for char in value)


def validate_url(url):
    """Conserve l'adresse fournie ; refuse URL ambiguë, identifiants et contrôles."""
    if not isinstance(url, str) or not url or len(url) > 2048:
        raise LinkConfigError('Adresse HTTPS absente ou trop longue.')
    if any(char.isspace() for char in url) or has_controls(url) or has_controls(unquote(url)) or any(char in url for char in '\\<>"`'):
        raise LinkConfigError('Adresse contenant des espaces ou des caractères interdits.')
    try:
        parts = urlsplit(url)
        if parts.scheme != 'https' or not parts.hostname:
            raise LinkConfigError('Une adresse HTTPS complète est nécessaire.')
        if parts.username is not None or parts.password is not None:
            raise LinkConfigError('Les identifiants dans une adresse sont interdits.')
        if parts.port is not None and not 1 <= parts.port <= 65535:
            raise LinkConfigError('Port HTTPS invalide.')
        host = parts.hostname.encode('idna').decode('ascii')
        if '%' in host or not re.fullmatch(r'[A-Za-z0-9.:-]+', host):
            raise LinkConfigError('Nom de serveur invalide.')
        if ':' not in host and any(not re.fullmatch(r'[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?', label) for label in host.rstrip('.').split('.')):
            raise LinkConfigError('Nom de serveur invalide.')
    except (ValueError, UnicodeError) as error:
        if isinstance(error, LinkConfigError):
            raise
        raise LinkConfigError('Adresse HTTPS invalide.') from error
    return url


def user_config_path():
    configured = os.environ.get('XDG_CONFIG_HOME')
    root = Path(configured) if configured and Path(configured).is_absolute() else Path.home() / '.config'
    return root / 'kyronext/links.json'


def _read_json(path):
    with Path(path).open('rb') as stream:
        data = stream.read(MAX_CONFIG_BYTES + 1)
    if len(data) > MAX_CONFIG_BYTES:
        raise LinkConfigError('Configuration de liens trop volumineuse.')
    try:
        document = json.loads(data.decode('utf-8'))
    except (UnicodeError, json.JSONDecodeError) as error:
        raise LinkConfigError('Configuration de liens UTF-8/JSON invalide.') from error
    if not isinstance(document, dict) or document.get('schema_version') != 1 or not isinstance(document.get('links'), list):
        raise LinkConfigError('Format de configuration de liens inconnu.')
    return document


def _label(value):
    if not isinstance(value, str) or not value.strip() or len(value) > 100 or has_controls(value):
        raise LinkConfigError('Libellé du lien invalide.')
    return value.strip()


def load_links(system_path=None, user_path=None):
    """Retourne (liens, avertissements), en conservant les défauts en cas d'erreur utilisateur."""
    warnings = []
    try:
        document = _read_json(system_path or SYSTEM_CONFIG)
    except (OSError, LinkConfigError) as error:
        return [], ['Configuration système des liens indisponible : ' + str(error)]
    links = []
    seen = set()
    for entry in document['links']:
        try:
            if not isinstance(entry, dict) or not isinstance(entry.get('id'), str) or not ID_PATTERN.fullmatch(entry['id']) or entry['id'] in seen:
                raise LinkConfigError('Identifiant de lien invalide ou dupliqué.')
            link = dict(entry)
            link['url'] = validate_url(entry.get('url'))
            link['label'] = _label(entry.get('label'))
            if not isinstance(entry.get('visible', True), bool):
                raise LinkConfigError('Visibilité du lien invalide.')
            link['visible'] = entry.get('visible', True)
            seen.add(link['id'])
            links.append(link)
        except LinkConfigError as error:
            warnings.append(str(error))
    path = Path(user_path) if user_path is not None else user_config_path()
    if not path.exists():
        return links, warnings
    try:
        overrides = _read_json(path)
    except (OSError, LinkConfigError) as error:
        return links, warnings + ['Réglages utilisateur ignorés : ' + str(error)]
    index = {link['id']: link for link in links}
    overridden = set()
    for entry in overrides['links']:
        try:
            if not isinstance(entry, dict) or not isinstance(entry.get('id'), str) or entry['id'] not in index or entry['id'] in overridden:
                raise LinkConfigError('Identifiant utilisateur inconnu ou dupliqué.')
            original = index[entry['id']]
            replacement = dict(original)
            if 'url' in entry:
                replacement['url'] = validate_url(entry['url'])
                if replacement['url'] != original['url']:
                    replacement['verification'] = {'status': 'to-confirm', 'checked_at': None, 'detail': 'Adresse personnalisée, non contrôlée.'}
            if 'label' in entry:
                replacement['label'] = _label(entry['label'])
            if 'visible' in entry:
                if not isinstance(entry['visible'], bool):
                    raise LinkConfigError('Visibilité utilisateur invalide.')
                replacement['visible'] = entry['visible']
            overridden.add(entry['id'])
            original.clear()
            original.update(replacement)
        except LinkConfigError as error:
            warnings.append('Réglage utilisateur ignoré : ' + str(error))
    return links, warnings


def icon_path(link_id):
    if not isinstance(link_id, str) or not ID_PATTERN.fullmatch(link_id):
        raise LinkConfigError('Identifiant de lien invalide.')
    return Path(__file__).resolve().parents[2] / 'share/icons/Kyronext/scalable/apps' / ('kyronext-link-' + link_id + '.svg')


def find_browser():
    for name in ('firefox-esr', 'firefox', 'chromium', 'falkon', 'epiphany', 'google-chrome', 'brave-browser'):
        found = shutil.which(name)
        if found:
            return found
    return None


def open_link(parent, link, opener=None, browser_probe=None, notify=None):
    """Ouvre uniquement sur demande ; pas de shell, téléchargement ou test réseau."""
    if notify is None:
        from PyQt6.QtWidgets import QMessageBox
        notify = lambda title, message: QMessageBox.warning(parent, title, message)
    try:
        url = validate_url(link.get('url'))
    except (AttributeError, LinkConfigError) as error:
        notify('Lien invalide', str(error))
        return False
    if not (browser_probe or find_browser)():
        notify('Navigateur indisponible',
               'Aucun navigateur pris en charge n’est installé dans cette session. '
               'Les outils locaux restent utilisables. Un navigateur maintenu et '
               'configuré par défaut est nécessaire pour ouvrir ce lien.\n\nAdresse : ' + url)
        return False
    if opener is None:
        from PyQt6.QtCore import QUrl
        from PyQt6.QtGui import QDesktopServices
        opener = lambda address: QDesktopServices.openUrl(QUrl(address, QUrl.ParsingMode.StrictMode))
    try:
        if opener(url):
            return True
    except (OSError, RuntimeError):
        pass
    notify('Ouverture impossible', 'Le navigateur n’a pas pu ouvrir cette adresse. '
           'Vérifiez le navigateur par défaut et votre connexion.\n\nAdresse : ' + url)
    return False
