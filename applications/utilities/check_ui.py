"""Contrôle Qt hors écran ; aucun disque physique ni configuration hôte touché."""
import argparse
import json
import os
from pathlib import Path
import runpy
import sys
import traceback

os.environ['QT_QPA_PLATFORM'] = 'offscreen'
from PyQt6.QtCore import QSettings, QTimer
from PyQt6.QtGui import QFont, QFontDatabase
from PyQt6.QtWidgets import QApplication, QFileDialog, QLabel, QMessageBox, QInputDialog

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('application', choices=('calculator', 'notes'))
args = parser.parse_args()
scratch = ROOT / 'build' / 'native-utilities-check' / args.application
scratch.mkdir(parents=True, exist_ok=True)
output = ROOT / 'logs'
output.mkdir(exist_ok=True)
QSettings.setDefaultFormat(QSettings.Format.IniFormat)
QSettings.setPath(QSettings.Format.IniFormat, QSettings.Scope.UserScope, str(scratch))
original_exec = QApplication.exec
results = {'application': args.application, 'checks': [], 'success': False}


def check(app):
    try:
        window = next(widget for widget in app.topLevelWidgets() if hasattr(widget, 'settings'))
        if args.application == 'calculator':
            window.expression.setText('0,1 + 0,2')
            window.evaluate()
            assert window.result.text() == '0.3'
            results['checks'].append('Calcul décimal via la fenêtre réelle')
            window.theme.setCurrentIndex(1)
            assert '#e9ad52' in window.styleSheet()
            results['checks'].append('Palette ambre')
            window.expression.setText('1/0')
            window.evaluate()
            assert not window.result.text() and 'zéro' in window.message.text()
            results['checks'].append('Erreur de division affichée')
            window.mode.setCurrentIndex(1)
            window.expression.setText('sqrt(144) + 0.1 + 0.2'); window.evaluate()
            assert window.result.text() == '12.3' and window.functions.isVisible()
            window.mode.setCurrentIndex(2)
            window.expression.setText('(0xff & 15) << 2'); window.evaluate()
            assert window.result.text() == '60' and '0x3c' in window.message.text()
            window.mode.setCurrentIndex(3); window.category.setCurrentText('Température')
            window.source_unit.setCurrentText('°C'); window.target_unit.setCurrentText('°F')
            window.expression.setText('100'); window.evaluate()
            assert window.result.text() == '212'
            item = window.history.item(window.history.count() - 1)
            window.category.setCurrentText('Longueur'); window.restore_history(item)
            assert window.category.currentText() == 'Température' and window.target_unit.currentText() == '°F'
            assert 0 < window.history.count() <= 100
            results['checks'].append('Modes scientifique/programmeur/conversion et restauration des unités depuis historique')
            window.mode.setCurrentIndex(0)
            window.expression.setText('(12,5 + 3) × 2')
            window.evaluate()
        else:
            window.editor.setPlainText('Été — Kyronext\nDeuxième ligne')
            window.editor.document().setModified(True)
            QFileDialog.getSaveFileName = staticmethod(lambda *a, **k: ('', ''))
            QMessageBox.warning = staticmethod(lambda *a, **k: QMessageBox.StandardButton.Save)
            assert not window.confirm_leave()
            assert window.editor.document().isModified()
            assert 'Été' in window.editor.toPlainText()
            results['checks'].append('Annuler Enregistrer sous conserve le document et bloque la fermeture')
            path = scratch / 'document.txt'
            assert window.write(path)
            assert path.read_text(encoding='utf-8').startswith('Été')
            assert not window.editor.document().isModified()
            results['checks'].append('Enregistrement UTF-8 depuis la fenêtre réelle')
            window.editor.insertPlainText('Modification ')
            path.write_text('Version externe', encoding='utf-8')
            window.check_external_changes()
            assert window.editor.external_changed
            QMessageBox.warning = staticmethod(lambda *a, **k: QMessageBox.StandardButton.Cancel)
            assert not window.save()
            assert path.read_text(encoding='utf-8') == 'Version externe'
            assert window.editor.document().isModified()
            assert not window.close()
            assert window.isVisible()
            results['checks'].append('Changement externe et fermeture annulés sans perte')
            window.find_text = 'Kyronext'
            window.find_next()
            assert window.editor.textCursor().selectedText() == 'Kyronext'
            results['checks'].append('Recherche dans le document')
            first = window.editor; original = first.toPlainText()
            window.new(); second = window.editor
            assert first is not second and first.toPlainText() == original
            second.setPlainText('alpha alpha\nbeta'); second.document().setModified(True)
            assert second.numbers is not None and second.blockCount() == 2
            answers = iter([('alpha', True), ('omega', True)])
            QInputDialog.getText = staticmethod(lambda *a, **k: next(answers))
            QMessageBox.question = staticmethod(lambda *a, **k: QMessageBox.StandardButton.Yes)
            window.replace()
            assert second.toPlainText() == 'omega omega\nbeta'
            second.undo(); assert second.toPlainText() == 'alpha alpha\nbeta'
            second.redo(); assert second.toPlainText() == 'omega omega\nbeta'
            count = window.tabs.count()
            assert not window.close_tab(window.tabs.currentIndex()) and window.tabs.count() == count
            results['checks'].append('Onglets indépendants, lignes, remplacer tout annulable et fermeture annulée')
            window.recovery_directory = lambda: scratch / 'recovery'
            window.recovery_action.setChecked(True); window.save_recovery()
            draft = scratch / 'recovery' / (second.recovery_id + '.json')
            assert draft.is_file()
            index = window.tabs.indexOf(second); window.tabs.removeTab(index); second.deleteLater()
            window.restore_recovery()
            assert window.editor.toPlainText() == 'omega omega\nbeta'
            assert window.path is None and window.editor.document().isModified()
            window.recovery_action.setChecked(False)
            assert not draft.exists()
            results['checks'].append('Récupération facultative : écriture locale et restauration en document non enregistré')
        about_check = {}

        def inspect_about():
            dialog = app.activeModalWidget()
            try:
                assert dialog is not None and dialog.objectName() == 'kyronextAboutDialog'
                text = '\n'.join(label.text() for label in dialog.findChildren(QLabel))
                for expected in ('Version 0.1.0', 'Kyronext Systems', 'By Manix',
                                 'droits réservés', 'PyQt6', 'LGPL/GPL', 'https://kyronext.eu/kyronext-os/'):
                    assert expected in text, expected
                dialog.grab().save(str(output / ('kyronext-' + args.application + '-about.png')))
                about_check['success'] = True
            except Exception:
                about_check['error'] = traceback.format_exc()
            finally:
                if dialog is not None:
                    dialog.accept()

        QTimer.singleShot(50, inspect_about)
        window.about_action.trigger()
        assert about_check.get('success'), about_check.get('error', 'Dialogue À propos absent')
        assert app.applicationVersion() == '0.1.0'
        results['checks'].append('Menu Aide et dialogue À propos commun avec version, crédits, licences et URL')
        window.grab().save(str(output / ('kyronext-' + args.application + '-preview.png')))
        results['success'] = True
        app.exit(0)
    except Exception:
        results['error'] = traceback.format_exc()
        app.exit(1)
    finally:
        (output / ('kyronext-' + args.application + '-ui-check.json')).write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding='utf-8')
        print(json.dumps(results, ensure_ascii=False))


def exec_with_check(app):
    # Le plugin offscreen Windows ne découvre pas toujours les polices système.
    # Lecture d'une police existante pour ce contrôle uniquement, sans copie.
    font = Path('C:/Windows/Fonts/segoeui.ttf')
    if sys.platform == 'win32' and font.is_file():
        QFontDatabase.addApplicationFont(str(font))
        QApplication.setFont(QFont('Segoe UI', 10))
        for window in app.topLevelWidgets():
            if hasattr(window, 'editor'):
                window.editor.setFont(QFont('Segoe UI', 11))
    QTimer.singleShot(150, lambda: check(app))
    return original_exec()


QApplication.exec = exec_with_check
module = runpy.run_path(str(ROOT / 'iso/config/includes.chroot/usr/local/bin' / ('kyronext-' + args.application)))
raise SystemExit(module['main']([]))
