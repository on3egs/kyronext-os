#include "archive_backend.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
QString findTar() {
    for (const auto &path : {QStringLiteral("/usr/bin/tar"), QStringLiteral("/bin/tar")}) {
        const QFileInfo info(path);
        if (info.isFile() && info.isExecutable()) return path;
    }
    return {};
}

bool isInside(const QString &child, const QString &parent) {
    const QString prefix = parent.endsWith(QDir::separator())
        ? parent : parent + QDir::separator();
    return child == parent || child.startsWith(prefix);
}

bool hashFile(const QString &path, QString *digest, quint64 *size, QString *error) {
    QFile input(path);
    if (!input.open(QIODevice::ReadOnly)) {
        *error = QStringLiteral("Impossible de vérifier l'archive temporaire.");
        return false;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    quint64 total = 0;
    while (!input.atEnd()) {
        const QByteArray chunk = input.read(1024 * 1024);
        if (chunk.isEmpty() && input.error() != QFileDevice::NoError) {
            *error = QStringLiteral("Erreur pendant la vérification de l'archive.");
            return false;
        }
        hash.addData(chunk);
        total += quint64(chunk.size());
    }
    *digest = QString::fromLatin1(hash.result().toHex());
    *size = total;
    return true;
}
}

BackupResult createBackupArchive(const QStringList &sourceDirectories,
                                 const QString &destinationFile,
                                 const std::atomic_bool *cancelRequested,
                                 const QString &requestedTar) {
    BackupResult result;
    const auto cancelled = [&] { return cancelRequested && cancelRequested->load(); };
    if (geteuid() == 0) {
        result.error = QStringLiteral("Kyronext Backup doit être lancé depuis votre session, sans privilèges administrateur.");
        return result;
    }
    if (cancelled()) {
        result.cancelled = true;
        result.error = QStringLiteral("Sauvegarde annulée.");
        return result;
    }
    if (sourceDirectories.isEmpty()) {
        result.error = QStringLiteral("Sélectionnez au moins un dossier source.");
        return result;
    }
    const QFileInfo requestedDestination(destinationFile);
    const QString parentPath = requestedDestination.absolutePath();
    const QFileInfo parentInfo(parentPath);
    const QString canonicalParent = parentInfo.canonicalFilePath();
    if (canonicalParent.isEmpty() || !parentInfo.isDir() || requestedDestination.fileName().isEmpty()) {
        result.error = QStringLiteral("Le dossier de destination doit déjà exister.");
        return result;
    }
    const QString finalPath = QDir(canonicalParent).filePath(requestedDestination.fileName());
    struct stat existing {};
    if (::lstat(QFile::encodeName(finalPath).constData(), &existing) == 0) {
        result.error = QStringLiteral("Ce fichier existe déjà ; aucune donnée ne sera remplacée.");
        return result;
    }
    if (errno != ENOENT) {
        result.error = QStringLiteral("Impossible de vérifier le chemin de destination.");
        return result;
    }

    QStringList canonicalSources;
    for (const QString &path : sourceDirectories) {
        const QFileInfo sourceInfo(path);
        const QString canonical = sourceInfo.canonicalFilePath();
        if (!sourceInfo.isDir() || canonical.isEmpty()) {
            result.error = QStringLiteral("Un dossier source est absent ou inaccessible.");
            return result;
        }
        if (canonical == QDir::rootPath()) {
            result.error = QStringLiteral("La sauvegarde de la racine du système est refusée.");
            return result;
        }
        if (isInside(finalPath, canonical)) {
            result.error = QStringLiteral("Choisissez une destination située hors des dossiers sauvegardés.");
            return result;
        }
        if (!canonicalSources.contains(canonical)) canonicalSources.append(canonical);
    }

    const QString tar = requestedTar.isEmpty() ? findTar() : requestedTar;
    if (tar.isEmpty() || !QFileInfo(tar).isExecutable()) {
        result.error = QStringLiteral("Le programme tar n'est pas disponible.");
        return result;
    }

    QByteArray temporaryTemplate = QFile::encodeName(
        QDir(canonicalParent).filePath(QStringLiteral(".kyronext-backup-XXXXXX")));
    temporaryTemplate.append('\0');
    const int descriptor = ::mkstemp(temporaryTemplate.data());
    if (descriptor < 0) {
        result.error = QStringLiteral("Impossible de créer un fichier temporaire privé : %1")
            .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return result;
    }
    const QString temporaryPath = QFile::decodeName(temporaryTemplate.constData());
    bool keepTemporary = false;
    const auto cleanup = [&] {
        if (!keepTemporary) ::unlink(QFile::encodeName(temporaryPath).constData());
    };
    const int permissionStatus = ::fchmod(descriptor, 0600);
    const int closeStatus = ::close(descriptor);
    if (permissionStatus != 0 || closeStatus != 0) {
        result.error = QStringLiteral("Impossible de protéger le fichier temporaire.");
        cleanup();
        return result;
    }

    QStringList arguments{QStringLiteral("-czf"), temporaryPath, QStringLiteral("--")};
    arguments.append(canonicalSources);
    QProcess process;
    process.setProgram(tar);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();
    if (!process.waitForStarted(3000)) {
        result.error = QStringLiteral("Impossible de démarrer tar.");
        cleanup();
        return result;
    }
    while (!process.waitForFinished(100)) {
        if (cancelled()) {
            process.terminate();
            if (!process.waitForFinished(1200)) {
                process.kill();
                process.waitForFinished(3000);
            }
            result.cancelled = true;
            result.error = QStringLiteral("Sauvegarde annulée ; le fichier temporaire a été supprimé.");
            cleanup();
            return result;
        }
    }
    if (cancelled()) {
        result.cancelled = true;
        result.error = QStringLiteral("Sauvegarde annulée ; le fichier temporaire a été supprimé.");
        cleanup();
        return result;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        result.error = QStringLiteral("tar n'a pas terminé correctement ; aucune archive finale n'a été créée.");
        cleanup();
        return result;
    }

    const int syncDescriptor = ::open(QFile::encodeName(temporaryPath).constData(), O_RDONLY | O_CLOEXEC);
    if (syncDescriptor < 0 || ::fsync(syncDescriptor) != 0) {
        if (syncDescriptor >= 0) ::close(syncDescriptor);
        result.error = QStringLiteral("Impossible de synchroniser l'archive temporaire.");
        cleanup();
        return result;
    }
    ::close(syncDescriptor);
    if (!hashFile(temporaryPath, &result.sha256, &result.bytes, &result.error)) {
        cleanup();
        return result;
    }
    if (cancelled()) {
        result.cancelled = true;
        result.error = QStringLiteral("Sauvegarde annulée avant publication.");
        cleanup();
        return result;
    }
    if (::link(QFile::encodeName(temporaryPath).constData(),
               QFile::encodeName(finalPath).constData()) != 0) {
        result.error = errno == EEXIST
            ? QStringLiteral("Un fichier de destination vient d'apparaître ; il n'a pas été remplacé.")
            : QStringLiteral("Impossible de publier l'archive : %1").arg(QString::fromLocal8Bit(std::strerror(errno)));
        cleanup();
        return result;
    }
    keepTemporary = false;
    ::unlink(QFile::encodeName(temporaryPath).constData());
    const int directoryDescriptor = ::open(QFile::encodeName(canonicalParent).constData(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (directoryDescriptor >= 0) {
        ::fsync(directoryDescriptor);
        ::close(directoryDescriptor);
    }
    result.success = true;
    return result;
}
