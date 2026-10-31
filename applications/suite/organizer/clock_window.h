#pragma once
#include <kyronext/ui.h>
#include "appearance.h"
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDateTime>
#include <QListWidget>
#include <QMessageBox>

namespace {QString clockDuration(qint64 ms){const auto seconds=qMax<qint64>(0,ms)/1000;return QString("%1:%2:%3").arg(seconds/3600,2,10,QChar('0')).arg(seconds/60%60,2,10,QChar('0')).arg(seconds%60,2,10,QChar('0'));}}
class ClockWindow final:public QMainWindow {
public:
    ClockWindow(){setWindowTitle("Kyronext Clock");resize(650,700);auto *body=new QWidget;auto *layout=new QVBoxLayout(body);layout->setContentsMargins(24,20,24,20);
        auto *brand=new QLabel("KYRONEXT / CLOCK");brand->setObjectName("KyronextAppBrand");brand->setStyleSheet("font-size:20px;font-weight:700;color:#ef3c60");layout->addWidget(brand);clockLabel=new QLabel;clockLabel->setAlignment(Qt::AlignCenter);clockLabel->setStyleSheet("font-size:40px;font-weight:600");layout->addWidget(clockLabel);
        auto *chrono=new QGroupBox("Chronometre");auto *chronoLayout=new QVBoxLayout(chrono);chronoLabel=new QLabel("00:00:00.0");chronoLabel->setObjectName("stopwatchDisplay");chronoLabel->setStyleSheet("font-size:28px");chronoLayout->addWidget(chronoLabel);auto *chronoRow=new QHBoxLayout;start=new QPushButton("Demarrer");start->setObjectName("stopwatchToggle");auto *reset=new QPushButton("Remettre a zero");reset->setObjectName("stopwatchReset");chronoRow->addWidget(start);chronoRow->addWidget(reset);chronoLayout->addLayout(chronoRow);layout->addWidget(chrono);
        connect(start,&QPushButton::clicked,this,[this]{if(running){accumulated+=stopwatch.elapsed();running=false;start->setText("Reprendre");}else{stopwatch.start();running=true;start->setText("Pause");}timer->setInterval(running?100:1000);tick();});connect(reset,&QPushButton::clicked,this,[this]{running=false;accumulated=0;start->setText("Demarrer");timer->setInterval(1000);tick();});
        auto *countdown=new QGroupBox("Minuteur");auto *countLayout=new QVBoxLayout(countdown);countLabel=new QLabel("Aucun minuteur actif");countLayout->addWidget(countLabel);auto *countRow=new QHBoxLayout;seconds=new QSpinBox;seconds->setObjectName("countdownSeconds");seconds->setRange(1,86400);seconds->setValue(300);seconds->setSuffix(" secondes");auto *countStart=new QPushButton("Lancer");countStart->setObjectName("countdownStart");auto *countStop=new QPushButton("Annuler");countRow->addWidget(seconds);countRow->addWidget(countStart);countRow->addWidget(countStop);countLayout->addLayout(countRow);layout->addWidget(countdown);
        connect(countStart,&QPushButton::clicked,this,[this]{countDuration=qint64(seconds->value())*1000;countTime.start();counting=true;tick();});connect(countStop,&QPushButton::clicked,this,[this]{counting=false;tick();});
        auto *alarmGroup=new QGroupBox("Alarmes locales");auto *alarmLayout=new QVBoxLayout(alarmGroup);auto *alarmRow=new QHBoxLayout;alarmTime=new QTimeEdit(QTime::currentTime());alarmTime->setDisplayFormat("HH:mm");auto *add=new QPushButton("Ajouter");auto *remove=new QPushButton("Retirer la selection");alarmRow->addWidget(alarmTime);alarmRow->addWidget(add);alarmRow->addWidget(remove);alarmLayout->addLayout(alarmRow);alarms=new QListWidget;alarmLayout->addWidget(alarms);layout->addWidget(alarmGroup);
        connect(add,&QPushButton::clicked,this,[this]{if(alarms->count()>=16){Kyronext::showError(this,"Limite de 16 alarmes.");return;}auto target=QDateTime(QDate::currentDate(),QTime(alarmTime->time().hour(),alarmTime->time().minute()));if(target<=QDateTime::currentDateTime())target=target.addDays(1);auto *item=new QListWidgetItem(target.toString("dd/MM/yyyy HH:mm"),alarms);item->setData(Qt::UserRole,target);});connect(remove,&QPushButton::clicked,this,[this]{delete alarms->takeItem(alarms->currentRow());});
        auto *notice=new QLabel("Les alarmes et minuteurs ne fonctionnent que tant que cette application reste ouverte. Aucun service permanent ni reveil du PC n'est configure.");notice->setWordWrap(true);layout->addWidget(notice);setCentralWidget(body);addOrganizerAppearance(this);Kyronext::addAboutMenu(this,"Kyronext Clock","Horloge, chronometre, minuteur et alarmes de session.","Qt Core, Gui, Widgets");
        timer=new QTimer(this);timer->setInterval(1000);connect(timer,&QTimer::timeout,this,[this]{tick();});timer->start();tick();}
private:
    QLabel *clockLabel,*chronoLabel,*countLabel;QPushButton *start;QSpinBox *seconds;QTimeEdit *alarmTime;QListWidget *alarms;QTimer *timer;QElapsedTimer stopwatch,countTime;bool running=false,counting=false; qint64 accumulated=0,countDuration=0;
    void tick(){const auto now=QDateTime::currentDateTime();clockLabel->setText(now.toString("HH:mm:ss\ndddd dd MMMM yyyy"));const auto chrono=accumulated+(running?stopwatch.elapsed():0);chronoLabel->setText(clockDuration(chrono)+"."+QString::number(chrono/100%10));QStringList due;
        if(counting){const auto remaining=countDuration-countTime.elapsed();countLabel->setText(clockDuration(remaining));if(remaining<=0){counting=false;due.append("Minuteur termine.");}}else countLabel->setText("Aucun minuteur actif");
        for(int i=alarms->count()-1;i>=0;--i){const auto target=alarms->item(i)->data(Qt::UserRole).toDateTime();if(target<=now){due.append("Alarme de "+target.toString("HH:mm"));delete alarms->takeItem(i);}}
        if(!due.isEmpty())QMessageBox::information(this,"Kyronext Clock",due.join('\n'));
    }
};
