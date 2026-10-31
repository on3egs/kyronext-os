#include "calendar_store.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QSet>
#include <QLockFile>

namespace {
bool fail(QString *error,const QString &message){if(error)*error=message;return false;}
bool validate(const QVector<CalendarEvent> &events,QString *error) {
    if(events.size()>1000)return fail(error,"Limite de 1000 evenements.");QSet<QString> ids;
    for(const auto &e:events){if(e.id.isEmpty() || e.id.size()>80 || ids.contains(e.id) || e.title.trimmed().isEmpty() || e.title.size()>256 || e.notes.size()>4000 || !e.date.isValid() || !e.time.isValid())return fail(error,"Evenement invalide ou identifiant duplique.");ids.insert(e.id);}return true;
}
}
QByteArray calendarFingerprint(const QString &path) {QFile file(path);if(!file.open(QIODevice::ReadOnly))return {};return QCryptographicHash::hash(file.read(1024*1024+1),QCryptographicHash::Sha256);}
bool loadCalendar(const QString &path,QVector<CalendarEvent> *events,QString *error) {
    events->clear();const QFileInfo info(path);if(!info.exists())return true;if(info.isSymLink())return fail(error,"Lien symbolique refuse pour les donnees du calendrier.");
    QFile file(path);if(!file.open(QIODevice::ReadOnly))return fail(error,file.errorString());if(file.size()>1024*1024)return fail(error,"Fichier du calendrier trop grand.");
    QJsonParseError parse;const auto document=QJsonDocument::fromJson(file.readAll(),&parse);if(parse.error!=QJsonParseError::NoError || !document.isObject())return fail(error,"JSON du calendrier invalide.");
    const auto root=document.object();if(root.value("version").toInt()!=1 || !root.value("events").isArray())return fail(error,"Version du calendrier inconnue.");
    QVector<CalendarEvent> loaded;for(const auto &value:root.value("events").toArray()) {
        if(!value.isObject())return fail(error,"Evenement invalide.");const auto o=value.toObject();
        loaded.append({o.value("id").toString(),o.value("title").toString(),o.value("notes").toString(),QDate::fromString(o.value("date").toString(),Qt::ISODate),QTime::fromString(o.value("time").toString(),"HH:mm")});
    }
    if(!validate(loaded,error))return false;*events=loaded;return true;
}
bool saveCalendar(const QString &path,const QVector<CalendarEvent> &events,const QByteArray &expected,QString *error) {
    if(!validate(events,error))return false;const QFileInfo info(path);if(info.isSymLink())return fail(error,"Lien symbolique refuse.");
    if(!QDir().mkpath(info.absolutePath()))return fail(error,"Dossier utilisateur inaccessible.");
    QLockFile lock(path+".lock");if(!lock.tryLock(100))return fail(error,"Le calendrier est utilise par une autre instance.");
    if(calendarFingerprint(path)!=expected)return fail(error,"Le calendrier a change sur disque. Rechargez les donnees avant d'enregistrer.");
    QJsonArray array;for(const auto &e:events)array.append(QJsonObject{{"id",e.id},{"title",e.title},{"notes",e.notes},{"date",e.date.toString(Qt::ISODate)},{"time",e.time.toString("HH:mm")}});
    const auto data=QJsonDocument(QJsonObject{{"version",1},{"events",array}}).toJson();if(data.size()>1024*1024)return fail(error,"Donnees du calendrier trop volumineuses.");
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return fail(error,file.errorString());file.setPermissions(QFile::ReadOwner|QFile::WriteOwner);
    if(file.write(data)!=data.size()){file.cancelWriting();return fail(error,file.errorString());}if(calendarFingerprint(path)!=expected){file.cancelWriting();return fail(error,"Modification externe detectee avant enregistrement.");}if(!file.commit())return fail(error,file.errorString());return true;
}
