"""Saisie de commandes de test dans le clavier français du Live QEMU local."""
import argparse
from pathlib import Path
import runpy
import sys
import time

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('endpoint')
parser.add_argument('text')
parser.add_argument('--enter', action='store_true')
args = parser.parse_args()
mapping = {'a': 'q', 'q': 'a', 'z': 'w', 'w': 'z', 'm': 'semicolon',
           ' ': 'spc', '-': '6', '_': '8', '/': 'shift-dot', '.': 'shift-comma',
           ':': 'dot', ';': 'comma', '=': 'equal', '+': 'shift-equal',
           "'": '4', '"': '3', '(': '5', ')': 'minus', '|': 'alt_r-6'}
keys = []
for char in args.text:
    if char in mapping:
        keys.append(mapping[char])
    elif char.isascii() and char.islower() and char.isalpha():
        keys.append(char)
    elif char.isascii() and char.isupper() and char.isalpha():
        keys.append('shift-' + mapping.get(char.lower(), char.lower()))
    elif char in '0123456789':
        keys.append('shift-' + char)
    else:
        parser.error(f'Caractère non pris en charge : {char!r}')
sys.argv = ['qmp.py', args.endpoint, 'status']
client = runpy.run_path(str(Path(__file__).with_name('qmp.py')))
command = client['command']
for key in keys + (['ret'] if args.enter else []):
    command('human-monitor-command', {'command-line': 'sendkey ' + key + ' 30'})
    time.sleep(0.07)
