#pragma once
#include <kyronext/ui.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QLabel>

inline void addOrganizerAppearance(QMainWindow *window) {
    auto *menu=window->menuBar()->addMenu(QObject::tr("Apparence"));auto *group=new QActionGroup(window);group->setExclusive(true);
    auto apply=[window](Kyronext::Palette palette){Kyronext::setPreferredPalette(palette);Kyronext::applyTheme(*qApp,palette);for(auto *label:window->findChildren<QLabel*>("KyronextAppBrand"))label->setStyleSheet(QString("font-size:20px;font-weight:700;color:%1").arg(palette==Kyronext::Palette::Amber?"#e9ad52":"#ef3c60"));};
    for(auto choice:{qMakePair(QString("Graphite / rouge"),Kyronext::Palette::Red),qMakePair(QString("Graphite / ambre"),Kyronext::Palette::Amber)}) {
        auto *action=menu->addAction(choice.first);action->setCheckable(true);action->setChecked(Kyronext::paletteFromSettings()==choice.second);group->addAction(action);QObject::connect(action,&QAction::triggered,window,[apply,palette=choice.second]{apply(palette);});
    }
    apply(Kyronext::paletteFromSettings());
}
