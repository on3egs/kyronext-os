#include "calendar_store.h"
#include <kyronext/ui.h>
#include "appearance.h"
#include <QApplication>
#include <QMainWindow>
#include <QCalendarWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QSet>
#include <QStatusBar>
#include <QTextCharFormat>

class CalendarWindow final:public QMainWindow {
public:
    CalendarWindow(){
        setWindowTitle("Kyronext Calendar");resize(850,650);auto *body=new QWidget;auto *layout=new QVBoxLayout(body);layout->setContentsMargins(20,16,20,16);
        auto *brand=new QLabel("KYRONEXT / CALENDAR");brand->setObjectName("KyronextAppBrand");brand->setStyleSheet("font-size:20px;font-weight:700;color:#ef3c60");layout->addWidget(brand);
        calendar=new QCalendarWidget;calendar->setGridVisible(true);layout->addWidget(calendar);list=new QListWidget;list->setSortingEnabled(true);list->setAccessibleName("Evenements du jour selectionne");layout->addWidget(list);
        auto *row=new QHBoxLayout;auto *add=new QPushButton("Ajouter");auto *edit=new QPushButton("Modifier");auto *remove=new QPushButton("Supprimer");auto *reload=new QPushButton("Recharger le fichier");for(auto *button:{add,edit,remove,reload})row->addWidget(button);layout->addLayout(row);
        auto *notice=new QLabel("Donnees locales, sans synchronisation. Rappels affiches uniquement tant que cette application est ouverte.");notice->setWordWrap(true);layout->addWidget(notice);setCentralWidget(body);
        path=QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath("events.json");
        connect(calendar,&QCalendarWidget::selectionChanged,this,[this]{refresh();});connect(add,&QPushButton::clicked,this,[this]{editEvent(false);});connect(edit,&QPushButton::clicked,this,[this]{editEvent(true);});connect(remove,&QPushButton::clicked,this,[this]{removeEvent();});connect(reload,&QPushButton::clicked,this,[this]{load();});
        addOrganizerAppearance(this);Kyronext::addAboutMenu(this,"Kyronext Calendar","Evenements et rappels locaux, sans compte ni service distant.","Qt Core, Gui, Widgets");load();
        previous=QDateTime::currentDateTime().addSecs(-60);auto *timer=new QTimer(this);timer->setInterval(15000);connect(timer,&QTimer::timeout,this,[this]{reminders();});timer->start();
    }
private:
    QCalendarWidget *calendar;QListWidget *list;QString path;QByteArray fingerprint;QVector<CalendarEvent> events;bool valid=false;QSet<QString> notified;QDateTime previous;QSet<QDate> marked;
    void load(){QVector<CalendarEvent> loaded;QString error;valid=loadCalendar(path,&loaded,&error);if(!valid){Kyronext::showError(this,error);statusBar()->showMessage("Donnees non chargees : modifications bloquees pour preserver le fichier.");return;}events=loaded;fingerprint=calendarFingerprint(path);refresh();}
    void refresh(){list->clear();for(const auto &date:marked)calendar->setDateTextFormat(date,{});marked.clear();for(const auto &e:events){QTextCharFormat format;format.setForeground(QColor("#ef3c60"));format.setFontWeight(QFont::Bold);calendar->setDateTextFormat(e.date,format);marked.insert(e.date);if(e.date==calendar->selectedDate()){auto *item=new QListWidgetItem(e.time.toString("HH:mm")+"  "+e.title,list);item->setData(Qt::UserRole,e.id);item->setToolTip(e.notes);}}statusBar()->showMessage(QString("%1 evenements locaux. %2").arg(events.size()).arg(path));}
    bool persist(const QVector<CalendarEvent> &candidate){QString error;if(!valid || !saveCalendar(path,candidate,fingerprint,&error)){Kyronext::showError(this,valid?error:"Rechargez un fichier valide avant de modifier les evenements.");return false;}events=candidate;fingerprint=calendarFingerprint(path);refresh();return true;}
    int selected()const {if(!list->currentItem())return -1;const auto id=list->currentItem()->data(Qt::UserRole).toString();for(int i=0;i<events.size();++i)if(events[i].id==id)return i;return -1;}
    void editEvent(bool existing){if(!valid){Kyronext::showError(this,"Le fichier doit etre charge sans erreur.");return;}const int index=existing?selected():-1;if(existing && index<0)return;
        CalendarEvent item=existing?events[index]:CalendarEvent{QUuid::createUuid().toString(QUuid::WithoutBraces),{}, {},calendar->selectedDate(),QTime(9,0)};
        QDialog dialog(this);dialog.setWindowTitle(existing?"Modifier l'evenement":"Nouvel evenement");auto *layout=new QFormLayout(&dialog);
        QLineEdit title(item.title);title.setMaxLength(256);QDateEdit date(item.date);date.setCalendarPopup(true);QTimeEdit time(item.time);time.setDisplayFormat("HH:mm");QPlainTextEdit notes;notes.setPlainText(item.notes);
        layout->addRow("Titre",&title);layout->addRow("Date",&date);layout->addRow("Heure locale",&time);layout->addRow("Notes",&notes);QDialogButtonBox buttons(QDialogButtonBox::Save|QDialogButtonBox::Cancel);layout->addRow(&buttons);connect(&buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
        if(dialog.exec()!=QDialog::Accepted)return;item.title=title.text().trimmed();item.date=date.date();item.time=time.time();item.notes=notes.toPlainText();auto candidate=events;if(existing)candidate[index]=item;else candidate.append(item);persist(candidate);
    }
    void removeEvent(){const int index=selected();if(index<0)return;QMessageBox box(QMessageBox::Question,"Supprimer l'evenement",{},QMessageBox::Yes|QMessageBox::Cancel,this);box.setTextFormat(Qt::PlainText);box.setText("Supprimer "+events[index].title+" ?");box.setDefaultButton(QMessageBox::Cancel);if(box.exec()!=QMessageBox::Yes)return;auto candidate=events;candidate.removeAt(index);persist(candidate);}
    void reminders(){const auto now=QDateTime::currentDateTime();QStringList due;for(const auto &e:events){const QDateTime time(e.date,e.time);if(time>previous && time<=now && !notified.contains(e.id)){due.append(e.time.toString("HH:mm")+" - "+e.title);notified.insert(e.id);}}previous=now;if(!due.isEmpty()){QMessageBox box(QMessageBox::Information,"Rappels Kyronext",{},QMessageBox::Ok,this);box.setTextFormat(Qt::PlainText);box.setText(due.join('\n'));box.exec();}}
};
int main(int argc,char **argv){QApplication app(argc,argv);Kyronext::configureApplication("Kyronext Calendar","0.1.0");Kyronext::applyTheme(app,Kyronext::paletteFromSettings());CalendarWindow window;window.show();return app.exec();}
