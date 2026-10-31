#!/usr/bin/env python3
"""Aperçu graphique ou TTY du futur installateur ; ne modifie aucun disque."""
import argparse
import json
import sys
import time
from pathlib import Path

BASE = Path(__file__).resolve().parent

def load_slides():
    with (BASE / 'slides-fr.json').open(encoding='utf-8-sig') as stream:
        return json.load(stream)['slides']

def tty_preview(slides, delay):
    for index, slide in enumerate(slides):
        if sys.stdout.isatty():
            print('\033[2J\033[H', end='')
        progress = round((index + 1) * 100 / len(slides))
        filled = progress // 5
        print('\n  KYRONEXT OS  /  APERÇU DE L’INSTALLATEUR')
        print('  Démonstration : aucune installation ni modification de disque.\n')
        print('  ' + slide['title'].upper() + '\n')
        import textwrap
        print('\n'.join('  ' + line for line in textwrap.wrap(slide['body'], 72)))
        print('\n  [' + '#' * filled + '-' * (20-filled) + f'] {progress} %')
        print('  Progression de la présentation uniquement.\n', flush=True)
        if delay:
            time.sleep(delay)

def graphical_preview(slides):
    import tkinter as tk
    from tkinter import ttk
    root = tk.Tk()
    root.title('Kyronext OS — aperçu de l’installateur')
    root.geometry('1020x700')
    root.minsize(740, 580)
    root.configure(bg='#101621')
    top = tk.Frame(root, bg='#151e2c', padx=28, pady=20)
    top.pack(fill='x')
    tk.Label(top, text='KYRONEXT  /  OS', font=('Segoe UI', 20, 'bold'), bg='#151e2c', fg='#f0f4fa').pack(side='left')
    tk.Label(top, text='BY MANIX', font=('Segoe UI', 10, 'bold'), bg='#151e2c', fg='#ff625f').pack(side='right')
    main = tk.Frame(root, bg='#101621', padx=40, pady=20)
    main.pack(fill='both', expand=True)
    tk.Label(main, text='APERÇU · AUCUNE MODIFICATION DU DISQUE', bg='#101621', fg='#f0bb67', font=('Segoe UI', 10, 'bold')).pack(anchor='w')
    title = tk.Label(main, text='', bg='#101621', fg='#f2f5fa', font=('Segoe UI', 23, 'bold'), anchor='w')
    title.pack(fill='x', pady=(12, 4))
    body = tk.Label(main, text='', bg='#101621', fg='#c6cfdb', font=('Segoe UI', 12), wraplength=820, justify='left', anchor='w')
    body.pack(fill='x', pady=(4, 12))
    picture = tk.Label(main, bg='#202a39', text='KYRONEXT\n\nUNE PASSION. UN PROJET. UN VOYAGE.', fg='#e8edf5', font=('Segoe UI', 18, 'bold'))
    picture.pack(fill='both', expand=True, pady=8)
    image_refs = {}
    for name in ('desktop', 'applications'):
        path = BASE / 'assets' / (name + '.png')
        if path.exists():
            image = tk.PhotoImage(file=str(path))
            ratio = max(1, (image.width() + 699) // 700, (image.height() + 299) // 300)
            image_refs[name] = image.subsample(ratio, ratio)
    logo_path = BASE / 'assets' / 'kyronext.png'
    if logo_path.exists():
        image_refs['story'] = tk.PhotoImage(file=str(logo_path))
        image_refs['thanks'] = image_refs['story']
    style = ttk.Style(root)
    style.theme_use('clam')
    style.configure('Kyronext.Horizontal.TProgressbar', troughcolor='#263244', background='#e64b49', borderwidth=0, thickness=12)
    progress = ttk.Progressbar(main, maximum=100, style='Kyronext.Horizontal.TProgressbar')
    progress.pack(fill='x', pady=(18, 4))
    status = tk.Label(main, bg='#101621', fg='#aab8ca', font=('Segoe UI', 10), anchor='w')
    status.pack(fill='x')
    buttons = tk.Frame(main, bg='#101621')
    buttons.pack(fill='x', pady=(12, 0))
    current = [0]
    def render():
        slide = slides[current[0]]
        title.config(text=slide['title'])
        body.config(text=slide['body'])
        selected = image_refs.get(slide.get('image'))
        picture.config(image=selected or '', text='' if selected else 'KYRONEXT\n\nUNE PASSION. UN PROJET. UN VOYAGE.')
        value = round((current[0] + 1) * 100 / len(slides))
        progress['value'] = value
        status.config(text=f'Présentation {current[0]+1} / {len(slides)} · {value} % de la démonstration')
    def move(delta):
        current[0] = (current[0] + delta) % len(slides)
        render()
    def button(text, command, color):
        tk.Button(buttons, text=text, command=command, bg=color, fg='white', relief='flat', padx=18, pady=9, font=('Segoe UI', 10, 'bold')).pack(side='left', padx=(0, 10))
    button('← Précédent', lambda: move(-1), '#344256')
    button('Suivant →', lambda: move(1), '#c33739')
    button('Fermer', root.destroy, '#344256')
    render()
    root.mainloop()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tty', action='store_true', help='Afficher la présentation dans le terminal')
    parser.add_argument('--delay', type=float, default=3, help='Secondes par page en mode TTY')
    args = parser.parse_args()
    if args.delay < 0 or args.delay > 60:
        parser.error('--delay doit être compris entre 0 et 60')
    if args.tty:
        tty_preview(load_slides(), args.delay)
    else:
        graphical_preview(load_slides())

if __name__ == '__main__':
    main()
