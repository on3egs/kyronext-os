"""Catalogue XDG local. L'exécution est confiée à Gio, jamais à un shell."""
import configparser
import os
from pathlib import Path
import shlex
import shutil


def application_directories():
    home = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share'))
    return [home / 'applications'] + [Path(p) / 'applications' for p in
            os.environ.get('XDG_DATA_DIRS', '/usr/local/share:/usr/share').split(':') if p]


def scan_applications(directories=None):
    applications, seen = [], set()
    desktop = set(os.environ.get('XDG_CURRENT_DESKTOP', 'KDE').split(':'))
    for directory in directories or application_directories():
        directory = Path(directory)
        if not directory.is_dir():
            continue
        for path in sorted(directory.rglob('*.desktop')):
            identifier = str(path.relative_to(directory)).replace(os.sep, '-')
            if identifier in seen:
                continue
            seen.add(identifier)
            try:
                if path.stat().st_size > 262144:
                    continue
                parser = configparser.ConfigParser(interpolation=None, strict=False)
                parser.optionxform = str
                parser.read_string(path.read_text(encoding='utf-8'))
                entry = parser['Desktop Entry']
                if entry.get('Type') != 'Application' or entry.getboolean('Hidden', fallback=False) or entry.getboolean('NoDisplay', fallback=False):
                    continue
                only = set(filter(None, entry.get('OnlyShowIn', '').split(';')))
                excluded = set(filter(None, entry.get('NotShowIn', '').split(';')))
                if (only and not desktop.intersection(only)) or desktop.intersection(excluded):
                    continue
                command = entry.get('TryExec', '')
                if not command and not entry.getboolean('DBusActivatable', fallback=False):
                    tokens = shlex.split(entry.get('Exec', ''))
                    if not tokens:
                        continue
                    command = tokens[0]
                if command and not shutil.which(command):
                    continue
                name = entry.get('Name[fr_FR]', entry.get('Name[fr]', entry.get('Name', '')))
                if not name.strip():
                    continue
                applications.append({'id': identifier, 'name': name, 'path': str(path),
                                     'icon': entry.get('Icon', 'application-x-executable'),
                                     'categories': entry.get('Categories', ''),
                                     'comment': entry.get('Comment[fr]', entry.get('Comment', ''))})
            except (OSError, UnicodeError, ValueError, configparser.Error, KeyError):
                continue
    return sorted(applications, key=lambda app: app['name'].casefold())


def launch_application(path):
    from gi.repository import Gio
    info = Gio.DesktopAppInfo.new_from_filename(str(path))
    if info is None:
        raise RuntimeError('Le lanceur n’est plus disponible.')
    if not info.launch([], None):
        raise RuntimeError('L’application n’a pas pu démarrer.')


def pin_application(source, desktop_directory):
    source, directory = Path(source), Path(desktop_directory)
    if source.suffix != '.desktop' or not source.is_file():
        raise ValueError('Fichier de lancement invalide.')
    directory.mkdir(parents=True, exist_ok=True)
    destination = directory / source.name
    # Création exclusive : aucun raccourci existant n'est écrasé.
    with destination.open('xb') as stream:
        stream.write(source.read_bytes())
    destination.chmod(0o700)
    return destination
