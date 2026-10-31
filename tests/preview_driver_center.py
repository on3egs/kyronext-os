"""Captures Qt hors écran avec données SIMULÉES, jamais un diagnostic matériel."""
import importlib.machinery
import importlib.util
import os
from pathlib import Path
from unittest.mock import patch

os.environ['QT_QPA_PLATFORM'] = 'offscreen'
from PyQt6.QtWidgets import QApplication
from PyQt6.QtGui import QFont, QFontDatabase

root = Path(__file__).resolve().parents[1]
source = root / 'iso/config/includes.chroot/usr/local/bin/kyronext-driver-center'
loader = importlib.machinery.SourceFileLoader('driver_center_preview', str(source))
spec = importlib.util.spec_from_loader(loader.name, loader)
module = importlib.util.module_from_spec(spec); loader.exec_module(module)
data = {'devices': [
    {'bus':'pci', 'address':'0000:00:02.0', 'vendor':'8086', 'product':'0046', 'driver':'i915', 'state':'Pilote lié', 'candidates':['i915'], 'firmware_packages_to_check':['firmware-intel-graphics']},
    {'bus':'pci', 'address':'0000:02:00.0', 'vendor':'8086', 'product':'422c', 'driver':'iwlwifi', 'state':'Pilote lié', 'candidates':['iwlwifi'], 'firmware_packages_to_check':['firmware-iwlwifi']},
    {'bus':'pci', 'address':'0000:00:1b.0', 'vendor':'8086', 'product':'3b56', 'driver':'snd_hda_intel', 'state':'Pilote lié', 'candidates':['snd_hda_intel'], 'firmware_packages_to_check':[]},
    {'bus':'usb', 'address':'1-2:1.0', 'vendor':'03f0', 'product':'241d', 'driver':'', 'state':'À examiner — pas une preuve de panne', 'candidates':['qcserial'], 'firmware_packages_to_check':[]},
], 'wifi':{'radio':'enabled'}, 'notice':'APERÇU AVEC DONNÉES SIMULÉES — cette capture ne valide aucun matériel.'}
app = QApplication([])
if os.name == 'nt':
    assert QFontDatabase.addApplicationFont('C:/Windows/Fonts/segoeui.ttf') >= 0
    app.setFont(QFont('Segoe UI', 10))
with patch.object(module.DriverCenter, 'launch'), patch.object(module.DriverCenter, 'load_firmware'):
    window = module.DriverCenter()
window.display_report(data)
window.firmware.addItems(['firmware-atheros', 'firmware-iwlwifi', 'firmware-realtek', 'firmware-amd-graphics'])
window.statusBar().showMessage('APERÇU SIMULÉ · aucune action matérielle exécutée')
out = root / 'logs/driver-center-preview'; out.mkdir(parents=True, exist_ok=True)
for width, height, tab, name in [(1080,740,0,'materiel'), (1080,740,1,'connexion'), (800,600,1,'compact')]:
    window.resize(width,height); window.tabs.setCurrentIndex(tab); window.show(); app.processEvents()
    assert window.grab().save(str(out / (name+'.png')))
    assert window.scan.isVisible(), 'Bouton analyse masqué'
    print(name, window.width(), window.height())
window.close()
