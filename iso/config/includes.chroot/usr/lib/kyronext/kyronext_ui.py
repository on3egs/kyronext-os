"""Adaptateur PyQt6 commun des applications Kyronext existantes.

Ce module ne fournit pas de binding au framework C++ Qt : il partage seulement
la présentation et les métadonnées des dialogues À propos des outils Python.
Les imports Qt restent locaux pour permettre les contrôles sans interface.
"""
from pathlib import Path


APP_VERSION = '0.1.0'
PUBLISHER = 'Kyronext Systems'
SIGNATURE = 'By Manix'
OFFICIAL_URL = 'https://kyronext.eu/kyronext-os/'
PROJECT_LICENSE = 'Créations originales Kyronext : droits réservés.'
COMPONENT_LICENSES = (
    'PyQt6 : GPL v3 ; Qt : LGPL/GPL selon les composants. '
    'Les composants tiers conservent leurs licences et conditions propres.'
)
APPLICATIONS = {
    'launchpad': ('Kyronext Launchpad', 'Catalogue local des applications installées, favoris, raccourcis et liens configurables.'),
    'help': ('Centre d’aide Kyronext', 'Documentation locale de Kyronext OS, disponible hors ligne.'),
    'calculator': (
        'Calculatrice Kyronext',
        'Calculatrice décimale native avec priorités, parenthèses et palettes '
        'graphite/rouge ou graphite/ambre. Aucun code saisi n’est exécuté.',
    ),
    'notes': (
        'Bloc-notes Kyronext',
        'Éditeur de textes UTF-8 locaux avec recherche, sauvegarde atomique '
        'et confirmation des modifications non enregistrées.',
    ),
}


def configure_application(app, application_id):
    from PyQt6.QtCore import QSettings
    shared = QSettings('Kyronext Systems', 'Appearance')
    try:
        points = int(shared.value('appearance/fontSize', app.font().pointSize()))
        if 8 <= points <= 24:
            font = app.font()
            font.setPointSize(points)
            app.setFont(font)
    except (TypeError, ValueError):
        pass
    name, _ = APPLICATIONS[application_id]
    app.setApplicationName(name)
    app.setApplicationVersion(APP_VERSION)
    app.setOrganizationName(PUBLISHER)
    app.setOrganizationDomain('kyronext.eu')


def create_about_dialog(parent, application_id):
    from PyQt6.QtCore import Qt
    from PyQt6.QtGui import QFont, QIcon
    from PyQt6.QtWidgets import QDialog, QDialogButtonBox, QHBoxLayout, QLabel, QVBoxLayout

    name, description = APPLICATIONS[application_id]
    dialog = QDialog(parent)
    dialog.setObjectName('kyronextAboutDialog')
    dialog.setWindowTitle('À propos de ' + name)
    dialog.setAccessibleName('À propos de ' + name)
    dialog.setMinimumWidth(340)
    dialog.resize(480, 420)
    layout = QVBoxLayout(dialog)
    layout.setContentsMargins(22, 22, 22, 22)
    layout.setSpacing(14)
    header = QHBoxLayout()
    logo = QLabel()
    logo.setAccessibleName('Logo Kyronext')
    icon_path = Path(__file__).resolve().parents[2] / 'share/icons/Kyronext/scalable/apps/kyronext-logo.svg'
    if icon_path.is_file():
        icon = QIcon(str(icon_path))
        logo.setPixmap(icon.pixmap(64, 64))
        dialog.setWindowIcon(icon)
    else:
        logo.setText('K')
    logo.setFixedSize(68, 68)
    logo.setAlignment(Qt.AlignmentFlag.AlignCenter)
    header.addWidget(logo)
    identity = QVBoxLayout()
    title = QLabel(name)
    title.setWordWrap(True)
    font = QFont(title.font())
    font.setPointSize(16)
    font.setBold(True)
    title.setFont(font)
    identity.addWidget(title)
    identity.addWidget(QLabel('Version ' + APP_VERSION))
    header.addLayout(identity, 1)
    layout.addLayout(header)
    for text in (PUBLISHER + ' — ' + SIGNATURE, description, PROJECT_LICENSE, COMPONENT_LICENSES):
        label = QLabel(text)
        label.setWordWrap(True)
        label.setTextInteractionFlags(Qt.TextInteractionFlag.TextSelectableByMouse)
        layout.addWidget(label)
    link = QLabel('<a style="color:#f1f2f5" href="' + OFFICIAL_URL + '">Site officiel Kyronext OS</a>')
    link.setOpenExternalLinks(True)
    link.setAccessibleName('Ouvrir le site officiel Kyronext OS')
    link.setTextInteractionFlags(Qt.TextInteractionFlag.LinksAccessibleByMouse | Qt.TextInteractionFlag.LinksAccessibleByKeyboard)
    layout.addWidget(link)
    buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
    buttons.button(QDialogButtonBox.StandardButton.Close).setText('Fermer')
    buttons.rejected.connect(dialog.reject)
    layout.addWidget(buttons)
    return dialog


def add_help_menu(window, application_id):
    from PyQt6.QtGui import QAction

    name, _ = APPLICATIONS[application_id]
    menu = window.menuBar().addMenu('&Aide')
    if application_id != 'help':
        from PyQt6.QtCore import QProcess
        from PyQt6.QtWidgets import QMessageBox
        help_action = QAction('Documentation locale', window)
        help_action.setShortcut('F1')
        def open_help():
            started, _ = QProcess.startDetached('/usr/bin/python3', [str(Path(__file__).resolve().parents[2] / 'local/bin/kyronext-help')])
            if not started:
                QMessageBox.information(window, 'Aide Kyronext', 'Le centre d’aide n’a pas pu être lancé.')
        help_action.triggered.connect(open_help)
        menu.addAction(help_action)
    action = QAction('À propos de ' + name + '…', window)
    action.setObjectName('kyronextAboutAction')

    def show_about():
        dialog = create_about_dialog(window, application_id)
        dialog.exec()
        dialog.deleteLater()

    action.triggered.connect(show_about)
    menu.addAction(action)
    return action
