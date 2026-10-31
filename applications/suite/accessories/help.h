#pragma once
#include <QAction>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
namespace AccessoryHelp {
inline void install(QMainWindow *window, const QString &title, const QString &text) {
    auto actions = window->menuBar()->actions();
    auto *menu = actions.isEmpty() ? window->menuBar()->addMenu("Aide") : actions.last()->menu();
    if (!menu) menu = window->menuBar()->addMenu("Aide");
    auto *help = new QAction("Mode d'emploi hors ligne", window);
    help->setShortcut(QKeySequence::HelpContents);
    QObject::connect(help, &QAction::triggered, window, [window, title, text]{
        QMessageBox dialog(window); dialog.setWindowTitle(title + " — Aide");
        dialog.setTextFormat(Qt::PlainText); dialog.setText(text); dialog.setStandardButtons(QMessageBox::Ok); dialog.exec();
    });
    menu->addSeparator(); menu->addAction(help);
}
}
