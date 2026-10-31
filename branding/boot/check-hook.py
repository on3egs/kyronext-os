#!/usr/bin/env python3
"""Vérifier le hook boot sur des menus BIOS/UEFI sans démarrer une machine."""
import argparse
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--bash', default='C:/Program Files/Git/bin/bash.exe' if os.name == 'nt' else 'bash')
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
hook = root / 'iso/config/hooks/live/0950-boot-branding.hook.binary'
assets = root / 'iso/config/includes.binary/boot/kyronext'
required = ('username=kyronext hostname=kyronext locales=fr_FR.UTF-8 '
            'keyboard-layouts=fr live-config.nottyautologin '
            'live-config.nocomponents=sudo').split()

for file, size in [('boot-grub.png', (800, 600)), ('boot-bios.png', (640, 480))]:
    data = (assets / file).read_bytes()
    assert data[:8] == b'\x89PNG\r\n\x1a\n', file
    assert struct.unpack('>II', data[16:24]) == size, file

fixtures = {
    'boot/grub/grub.cfg': '''source /boot/grub/config.cfg
menuentry "Live system (amd64)" {
 linux /live/vmlinuz-TEST boot=live components username=kyronext findiso=${iso_path}
 initrd /live/initrd.img-TEST
}
menuentry "Live system (amd64 fail-safe mode)" {
 linux /live/vmlinuz-TEST boot=live components memtest noapic noapm nodma nomce nosmp nosplash vga=788
 initrd /live/initrd.img-TEST
}
# linux /example/not-a-command boot=live
submenu 'Utilities...' {}
''',
    'boot/grub/config.cfg': 'set default=0\nsource /boot/grub/theme.cfg\n',
    'boot/grub/loopback.cfg': 'source /boot/grub/grub.cfg\n',
    'isolinux/menu.cfg': '''menu hshift 0
menu width 82
menu title Boot menu
include stdmenu.cfg
include live.cfg
menu begin utilities
 menu label ^Utilities
 menu title Utilities
 menu label ^Back..
menu end
''',
    'isolinux/live.cfg': '''label live-amd64
 menu label ^Live system (amd64)
 linux /live/vmlinuz
 initrd /live/initrd.img
 append boot=live components username=kyronext
label live-amd64-failsafe
 menu label Live system (amd64 fail-safe mode)
 linux /live/vmlinuz
 initrd /live/initrd.img
 append boot=live components memtest noapic noapm nodma nomce nosmp nosplash vga=788
''',
}

def command_lines(text):
    return [line for line in text.splitlines()
            if line.lstrip().startswith(('linux ', 'linuxefi ', 'append ', 'initrd '))]

(root / 'build').mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='boot-branding-check-', dir=root / 'build') as temporary:
    tree = Path(temporary)
    for name, content in fixtures.items():
        path = tree / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding='utf-8', newline='\n')
    shutil.copytree(assets, tree / 'boot/kyronext')
    subprocess.run([args.bash, hook.as_posix()], cwd=tree, check=True)
    for name, original in fixtures.items():
        before = command_lines(original)
        after = command_lines((tree / name).read_text(encoding='utf-8'))
        assert len(before) == len(after), name
        for old, new in zip(before, after):
            expected = old
            if 'boot=live' in old.split():
                expected += ''.join(' ' + item for item in required if item not in old.split())
                for item in required:
                    assert new.split().count(item) == 1, (name, item)
            assert new == expected, (name, old, new)
    snapshot = {path.relative_to(tree): path.read_bytes() for path in tree.rglob('*') if path.is_file()}
    subprocess.run([args.bash, hook.as_posix()], cwd=tree, check=True)
    assert snapshot == {path.relative_to(tree): path.read_bytes() for path in tree.rglob('*') if path.is_file()}, 'Hook non idempotent'
    assert (tree / 'boot/grub/splash.png').read_bytes() == (assets / 'boot-grub.png').read_bytes()
    assert (tree / 'isolinux/splash.png').read_bytes() == (assets / 'boot-bios.png').read_bytes()
print('Boot branding : PNG, options Live, chemins et idempotence OK')
