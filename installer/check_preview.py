"""Smoke test graphique : ouvre puis ferme uniquement l'aperçu."""
import tkinter as tk
import preview

original = tk.Tk.mainloop
def smoke(window):
    window.update()
    assert window.title() == 'Kyronext OS — aperçu de l’installateur'
    window.after(300, window.destroy)
    original(window)
tk.Tk.mainloop = smoke
preview.graphical_preview(preview.load_slides())
print('Aperçu graphique chargé et fermé correctement.')
