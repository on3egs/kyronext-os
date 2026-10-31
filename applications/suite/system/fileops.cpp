#include "fileops.h"
#include <KIO/CopyJob>
#include <KIO/Job>
#include <QFileInfo>
#include <QDir>

bool validateDestination(const QUrl &source,const QUrl &destination,QString *error) {
    auto fail=[&](QString m){if(error)*error=m;return false;};
    if(!source.isLocalFile() || !destination.isLocalFile() || !source.host().isEmpty() || !destination.host().isEmpty())return fail("Cette version accepte seulement les fichiers locaux.");
    const QFileInfo from(source.toLocalFile()),to(destination.toLocalFile());
    if(!from.exists() && !from.isSymLink())return fail("La source n'existe plus.");
    if(to.exists() || to.isSymLink())return fail("La destination existe deja : aucun fichier ne sera ecrase.");
    if(!QFileInfo(to.absolutePath()).isDir())return fail("Le dossier de destination n'existe pas.");
    const QString origin=from.canonicalFilePath();const QString parent=QFileInfo(to.absolutePath()).canonicalFilePath();
    if(from.isDir() && (parent==origin || parent.startsWith(origin+"/")))return fail("Un dossier ne peut pas etre copie dans lui-meme.");
    return true;
}
KJob *copyLocal(const QUrl &source,const QUrl &destination,QString *error) {
    if(!validateDestination(source,destination,error))return nullptr;
    auto *job=KIO::copyAs(source,destination,KIO::HideProgressInfo);job->setUiDelegate(nullptr);return job;
}
KJob *moveLocal(const QUrl &source,const QUrl &destination,QString *error) {
    if(!validateDestination(source,destination,error))return nullptr;
    auto *job=KIO::moveAs(source,destination,KIO::HideProgressInfo);job->setUiDelegate(nullptr);return job;
}
KJob *trashLocal(const QUrl &source,QString *error) {
    if(!source.isLocalFile() || !source.host().isEmpty() || (!QFileInfo::exists(source.toLocalFile()) && !QFileInfo(source.toLocalFile()).isSymLink())) {
        if(error)*error="La source locale n'existe plus.";return nullptr;
    }
    auto *job=KIO::trash(source,KIO::HideProgressInfo);job->setUiDelegate(nullptr);return job;
}
