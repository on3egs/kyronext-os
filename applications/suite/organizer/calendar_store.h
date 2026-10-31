#pragma once
#include <QDate>
#include <QTime>
#include <QVector>
#include <QString>
#include <QByteArray>
struct CalendarEvent { QString id,title,notes;QDate date;QTime time; };
QByteArray calendarFingerprint(const QString &path);
bool loadCalendar(const QString &path,QVector<CalendarEvent> *events,QString *error);
bool saveCalendar(const QString &path,const QVector<CalendarEvent> &events,const QByteArray &expected,QString *error);
