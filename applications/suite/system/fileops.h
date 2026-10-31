#pragma once
#include <QUrl>
#include <QString>
class KJob;
bool validateDestination(const QUrl &source,const QUrl &destination,QString *error);
KJob *copyLocal(const QUrl &source,const QUrl &destination,QString *error);
KJob *moveLocal(const QUrl &source,const QUrl &destination,QString *error);
KJob *trashLocal(const QUrl &source,QString *error);
