"""Contrôle réel de l'accueil Qt hors écran, navigateur simulé et réseau interdit."""
import json
import os
from pathlib import Path
import runpy
import shutil
import sys
import traceback
import urllib.request

os.environ['QT_QPA_PLATFORM'] = 'offscreen'
from PyQt6.QtCore import QSettings, QTimer
from PyQt6.QtGui import QAction, QDesktopServices, QFont, QFontDatabase
from PyQt6.QtWidgets import QApplication, QCheckBox, QMainWindow, QMessageBox, QPushButton, QScrollArea
import PyQt6.QtWidgets as QtWidgets

ROOT = Path(__file__).resolve().parents[2]
scratch = ROOT / 'build/links-welcome-check'
scratch.mkdir(parents=True, exist_ok=True)
os.environ['XDG_CONFIG_HOME'] = str(scratch / 'xdg')
QSettings.setDefaultFormat(QSettings.Format.IniFormat)
QSettings.setPath(QSettings.Format.IniFormat, QSettings.Scope.UserScope, str(scratch))
preferences = QSettings('Kyronext', 'Welcome')
preferences.clear()
original_exec = QApplication.exec
original_which = shutil.which
dispatched, notices = [], []
shutil.which = lambda command: '/mock/firefox-esr' if command == 'firefox-esr' else None
QDesktopServices.openUrl = staticmethod(lambda url: dispatched.append(url.toString()) or True)
QMessageBox.warning = staticmethod(lambda parent, title, text: notices.append((title, text)))
urllib.request.urlopen = lambda *a, **k: (_ for _ in ()).throw(AssertionError('Accès réseau interdit pendant le test UI'))
result = {'success': False, 'checks': []}


class CheckApplication(QApplication):
    def __init__(self, *args):
        super().__init__(*args)
        # Le plugin offscreen Windows ne charge pas toujours ses polices.
        # Charger avant la création des widgets évite des indices de taille
        # calculés avec les métriques de caractères manquants.
        font = Path('C:/Windows/Fonts/segoeui.ttf')
        if sys.platform == 'win32' and font.is_file():
            QFontDatabase.addApplicationFont(str(font))
            QFont.insertSubstitution('Noto Sans', 'Segoe UI')
            QApplication.setFont(QFont('Segoe UI', 10))


def check(app):
    try:
        window = next(widget for widget in app.topLevelWidgets() if isinstance(widget, QMainWindow))
        cards = [button for button in window.findChildren(QPushButton) if button.objectName() == 'ActionCard']
        assert len(cards) == 4
        buttons = {button.property('kyronextLinkId'): button for button in window.findChildren(QPushButton) if button.objectName() == 'WebLink'}
        assert len(buttons) == 3 and not dispatched
        assert all(button.isVisible() for button in buttons.values())
        result['checks'].append('Quatre cartes locales conservées et aucun navigateur ouvert au lancement')
        original_size = window.size()
        window.resize(680, 460)
        app.processEvents()
        scroll = window.findChild(QScrollArea)
        assert scroll.widget().width() <= scroll.viewport().width(), (scroll.widget().width(), scroll.viewport().width())
        window.resize(original_size)
        app.processEvents()
        result['checks'].append('Disposition sans débordement horizontal à 680 × 460')
        show = window.findChild(QCheckBox, 'ShowUsefulLinks')
        show.setChecked(False)
        assert not any(button.isVisible() for button in buttons.values())
        assert not preferences.value('links/show', True, type=bool)
        show.setChecked(True)
        visibility = window.findChild(QAction, 'ShowLink-kironext-be')
        visibility.setChecked(False)
        assert not buttons['kironext-be'].isVisible()
        assert buttons['kyronext-eu'].isVisible()
        visibility.setChecked(True)
        result['checks'].append('Masquage global et individuel persisté dans QSettings')
        buttons['kitt-franco-belge'].click()
        assert dispatched == ['https://www.kitt-franco-belge.be']
        result['checks'].append('Clic explicite transmis à QDesktopServices simulé avec URL exacte')
        shutil.which = lambda command: None
        buttons['kyronext-eu'].click()
        assert len(dispatched) == 1
        assert notices and notices[-1][0] == 'Navigateur indisponible'
        result['checks'].append('Absence de navigateur signalée sans tentative réseau ni blocage')
        scroll.verticalScrollBar().setValue(scroll.verticalScrollBar().maximum())
        (ROOT / 'logs').mkdir(exist_ok=True)
        window.grab().save(str(ROOT / 'logs/welcome-links-preview.png'))
        result['success'] = True
        app.exit(0)
    except Exception:
        result['error'] = traceback.format_exc()
        app.exit(1)
    finally:
        (ROOT / 'logs/welcome-links-ui-check.json').write_text(json.dumps(result, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
        print(json.dumps(result, ensure_ascii=False))


def exec_with_check(app):
    QTimer.singleShot(200, lambda: check(app))
    return original_exec()


QApplication.exec = exec_with_check
QtWidgets.QApplication = CheckApplication
module = runpy.run_path(str(ROOT / 'iso/config/includes.chroot/usr/local/bin/kyronext-welcome'))
module['main'].__globals__['ASSETS'] = ROOT / 'iso/config/includes.chroot/usr/share/kyronext/welcome'
raise SystemExit(module['main']([]))
