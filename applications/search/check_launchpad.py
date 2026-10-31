"""Smoke tests Qt du Launchpad sans réseau ni accès aux données utilisateur."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / 'iso/config/includes.chroot/usr/local/bin/kyronext-launchpad'


def main():
    env = os.environ.copy()
    env['QT_QPA_PLATFORM'] = 'offscreen'
    result = subprocess.run([sys.executable, str(SCRIPT), '--smoke-test'],
                            env=env, capture_output=True, text=True, timeout=15)
    if result.returncode != 0:
        raise SystemExit(result.stderr or f'Launchpad exited {result.returncode}')
    print('Launchpad Qt startup OK (offscreen; links not opened; no directories indexed).')

    with tempfile.TemporaryDirectory(prefix='kyronext-search-ui-') as temporary:
        first = Path(temporary) / 'first'
        second = Path(temporary) / 'second'
        first.mkdir()
        second.mkdir()
        (first / 'kyronext-ui-check.txt').write_text('', encoding='utf-8')
        (second / 'unrelated.txt').write_text('', encoding='utf-8')
        result = subprocess.run(
            [sys.executable, str(SCRIPT), '--smoke-test-files', str(first), str(second)],
            env=env, capture_output=True, text=True, timeout=12)
    if result.returncode != 0:
        raise SystemExit(result.stderr or f'Launchpad file-search UI exited {result.returncode}')
    print(result.stdout.strip())


if __name__ == '__main__':
    main()
