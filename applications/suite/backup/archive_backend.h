#pragma once

#include <QString>
#include <QStringList>
#include <atomic>

struct BackupResult {
    bool success = false;
    bool cancelled = false;
    QString error;
    QString sha256;
    quint64 bytes = 0;
};

BackupResult createBackupArchive(const QStringList &sourceDirectories,
                                 const QString &destinationFile,
                                 const std::atomic_bool *cancelRequested = nullptr,
                                 const QString &tarExecutable = {});
