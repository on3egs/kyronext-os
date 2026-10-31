"""Vue légère du framebuffer QEMU existant ; aucune nouvelle VM n'est lancée."""
import json
import argparse
from pathlib import Path
import socket
import threading
import tkinter as tk

root_path = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', type=int, choices=(4444, 4445), default=4444)
args = parser.parse_args()
mode = 'UEFI' if args.port == 4444 else 'BIOS'
frame_path = root_path / 'logs/qemu-live-view.png'
window = tk.Tk()
window.title(f'Kyronext OS — QEMU {mode} en direct (lecture seule)')
window.configure(background='#15161b')
label = tk.Label(window, text='Connexion à QEMU…', background='#15161b', foreground='white')
label.pack()
normal_status = f'{mode} · actualisation toutes les 5 secondes · fermer cette fenêtre ne ferme pas QEMU'
status = tk.Label(window, text=normal_status,
                  background='#15161b', foreground='#dddddd', padx=10, pady=8)
status.pack()
busy = False
alive = True
result = None


def capture():
    global result
    try:
        with socket.create_connection(('127.0.0.1', args.port), timeout=2) as sock:
            with sock.makefile('rwb', buffering=0) as stream:
                json.loads(stream.readline())
                for command in ({'execute': 'qmp_capabilities'},
                                {'execute': 'screendump', 'arguments': {
                                    'filename': str(frame_path), 'format': 'png'}}):
                    stream.write((json.dumps(command) + '\n').encode())
                    while True:
                        response = json.loads(stream.readline())
                        if 'error' in response:
                            raise RuntimeError(response['error'])
                        if 'return' in response:
                            break
        result = ('ok', None)
    except Exception as error:
        result = ('error', str(error))


def refresh():
    global busy, result
    if not alive:
        return
    if busy and result is not None:
        state, detail = result
        result = None
        busy = False
        if state == 'ok':
            image = tk.PhotoImage(file=str(frame_path))
            if image.width() > 1400 or image.height() > 900:
                image = image.subsample(2, 2)
            label.configure(image=image, text='')
            label.image = image
            status.configure(text=normal_status)
        else:
            status.configure(text='QEMU indisponible momentanément ; nouvelle tentative dans 5 secondes')
        window.after(5000, start_capture)
    window.after(200, refresh)


def start_capture():
    global busy
    if alive and not busy:
        busy = True
        threading.Thread(target=capture, daemon=True).start()


def close():
    global alive
    alive = False
    window.destroy()


window.protocol('WM_DELETE_WINDOW', close)
start_capture()
refresh()
window.mainloop()
