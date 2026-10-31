"""Recherche locale bornée pour Kyronext Launchpad.

Les racines sont choisies explicitement par l'utilisateur. Seuls les noms de
fichiers sont indexés en mémoire ; aucun contenu n'est lu ni envoyé ailleurs.
"""
import os
from pathlib import Path
import stat
import time

MAX_ROOTS = 20
MAX_FILES = 20000
MAX_DEPTH = 12
MAX_DIRECTORIES = 10000
MAX_SCAN_SECONDS = 8
MAX_RESULTS = 100


def index_paths(roots, max_files=MAX_FILES, max_depth=MAX_DEPTH,
                max_directories=MAX_DIRECTORIES, deadline_seconds=MAX_SCAN_SECONDS,
                cancel_event=None, progress=None):
    """Indexe les fichiers réguliers des dossiers choisis, sans suivre les liens.

    Le résultat est volontairement limité et ne contient que les chemins en
    mémoire du processus appelant. `truncated` avertit si la limite est atteinte.
    """
    if not 1 <= int(max_files) <= 100000:
        raise ValueError('La limite de fichiers doit être comprise entre 1 et 100000.')
    if not 0 <= int(max_depth) <= 32:
        raise ValueError('La profondeur doit être comprise entre 0 et 32.')
    if not 1 <= int(max_directories) <= 100000:
        raise ValueError('La limite de dossiers doit être comprise entre 1 et 100000.')
    if not 0.1 <= float(deadline_seconds) <= 60:
        raise ValueError('La durée doit être comprise entre 0,1 et 60 secondes.')

    selected = []
    seen_roots = set()
    errors = 0
    for raw in list(roots)[:MAX_ROOTS]:
        try:
            root = Path(raw).expanduser()
            if root.is_symlink() or not root.is_dir():
                errors += 1
                continue
            root = root.resolve(strict=True)
            key = os.path.normcase(str(root))
            if key not in seen_roots:
                seen_roots.add(key)
                selected.append(root)
        except (OSError, RuntimeError, TypeError, ValueError):
            errors += 1

    indexed = []
    truncated = False
    directory_limited = False
    depth_limited = False
    timed_out = False
    cancelled = False
    mounts_skipped = 0
    directories_scanned = 0
    seen_files = set()
    seen_directories = set()
    started = time.monotonic()

    def is_cancelled():
        return bool(cancel_event is not None and cancel_event.is_set())

    for root in selected:
        try:
            root_stat = os.stat(root, follow_symlinks=False)
            if not stat.S_ISDIR(root_stat.st_mode):
                errors += 1
                continue
        except OSError:
            errors += 1
            continue

        # Mountpoints explicitly selected as roots are allowed. Nested
        # mountpoints and filesystem changes are skipped unless selected alone.
        stack = [(root, 0, root_stat.st_dev, (root_stat.st_dev, root_stat.st_ino))]
        while stack:
            if is_cancelled():
                cancelled = True
                break
            if time.monotonic() - started >= float(deadline_seconds):
                timed_out = True
                break
            if directories_scanned >= int(max_directories):
                directory_limited = True
                break

            path, depth, root_device, expected_identity = stack.pop()
            try:
                current_stat = os.stat(path, follow_symlinks=False)
                if not stat.S_ISDIR(current_stat.st_mode) or \
                        (current_stat.st_dev, current_stat.st_ino) != expected_identity:
                    errors += 1
                    continue
                if path != root:
                    resolved = path.resolve(strict=True)
                    if not resolved.is_relative_to(root):
                        errors += 1
                        continue
                    if os.name == 'posix' and (os.path.ismount(path) or
                                               current_stat.st_dev != root_device):
                        mounts_skipped += 1
                        continue
                identity = (current_stat.st_dev, current_stat.st_ino)
                if identity in seen_directories:
                    continue
                seen_directories.add(identity)
                directories_scanned += 1
                children = []
                with os.scandir(path) as entries:
                    for entry in entries:
                        if is_cancelled():
                            cancelled = True
                            break
                        if time.monotonic() - started >= float(deadline_seconds):
                            timed_out = True
                            break
                        name = entry.name
                        if name.startswith('.'):
                            continue
                        try:
                            if entry.is_symlink():
                                continue
                            if entry.is_dir(follow_symlinks=False):
                                if depth >= int(max_depth):
                                    depth_limited = True
                                    continue
                                # DirEntry.stat can expose zero inode/device on
                                # Windows; os.stat supplies the identity used
                                # to recheck the path when it is opened later.
                                child_stat = os.stat(entry.path, follow_symlinks=False)
                                child = Path(entry.path)
                                if os.name == 'posix' and (os.path.ismount(child) or
                                                           child_stat.st_dev != root_device):
                                    mounts_skipped += 1
                                    continue
                                if directories_scanned + len(children) + len(stack) >= int(max_directories):
                                    directory_limited = True
                                    break
                                children.append((child, depth + 1, root_device,
                                                 (child_stat.st_dev, child_stat.st_ino)))
                            elif entry.is_file(follow_symlinks=False):
                                identity = os.path.normcase(str(Path(entry.path)))
                                if identity in seen_files:
                                    continue
                                if len(indexed) >= int(max_files):
                                    truncated = True
                                    break
                                seen_files.add(identity)
                                indexed.append((name.casefold(), str(Path(entry.path))))
                        except OSError:
                            errors += 1
                stack.extend(reversed(children))
                if progress and directories_scanned % 64 == 0:
                    progress({'files': len(indexed), 'directories': directories_scanned})
            except (OSError, RuntimeError, ValueError):
                errors += 1
            if truncated or directory_limited or timed_out or cancelled:
                break
        if truncated or directory_limited or timed_out or cancelled:
            break

    indexed.sort(key=lambda item: (item[0], item[1].casefold()))
    return {'files': indexed, 'errors': errors, 'truncated': truncated,
            'directory_limited': directory_limited, 'depth_limited': depth_limited,
            'timed_out': timed_out, 'cancelled': cancelled,
            'mounts_skipped': mounts_skipped, 'directories': directories_scanned}


def search_paths(index, query, limit=MAX_RESULTS):
    """Cherche dans le nom du fichier uniquement et renvoie au plus `limit`."""
    if not 1 <= int(limit) <= 1000:
        raise ValueError('La limite de résultats doit être comprise entre 1 et 1000.')
    term = str(query).strip().casefold()
    if not term:
        return []
    return [path for name, path in index.get('files', ()) if term in name][:int(limit)]
