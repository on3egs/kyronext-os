#include "archive_backend.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>
#include <sys/types.h>
#include <unistd.h>

class BackupTest final : public QObject {
    Q_OBJECT
private slots:
    void createsReadableArchiveWithHash() {
        if (geteuid() == 0) QSKIP("Les tests fonctionnels doivent être lancés sans privilèges root.");
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString source = temporary.path() + QStringLiteral("/documents");
        QVERIFY(QDir().mkpath(source));
        QFile input(source + QStringLiteral("/note.txt"));
        QVERIFY(input.open(QIODevice::WriteOnly));
        QCOMPARE(input.write("Kyronext backup fixture\n"), qint64(24));
        input.close();
        const QString output = temporary.path() + QStringLiteral("/archive.tar.gz");

        const BackupResult result = createBackupArchive({source}, output);
        QVERIFY2(result.success, qPrintable(result.error));
        QCOMPARE(result.sha256.size(), 64);
        QCOMPARE(result.bytes, QFileInfo(output).size());
        QVERIFY(QFileInfo(output).isFile());

        QProcess list;
        const QString tar = QStandardPaths::findExecutable(QStringLiteral("tar"));
        QVERIFY2(!tar.isEmpty(), "GNU tar is required by this test");
        list.setProgram(tar);
        list.setArguments({QStringLiteral("-tzf"), output});
        list.start();
        QVERIFY(list.waitForStarted());
        QVERIFY(list.waitForFinished());
        QCOMPARE(list.exitCode(), 0);
        QVERIFY(list.readAllStandardOutput().contains("/note.txt"));
    }

    void refusesExistingDestinationWithoutChangingIt() {
        if (geteuid() == 0) QSKIP("Les tests fonctionnels doivent être lancés sans privilèges root.");
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString source = temporary.path() + QStringLiteral("/source");
        QVERIFY(QDir().mkpath(source));
        const QString output = temporary.path() + QStringLiteral("/existing.tar.gz");
        QFile existing(output);
        QVERIFY(existing.open(QIODevice::WriteOnly));
        QCOMPARE(existing.write("keep me"), qint64(7));
        existing.close();

        const BackupResult result = createBackupArchive({source}, output);
        QVERIFY(!result.success);
        QVERIFY(result.error.contains(QStringLiteral("existe"), Qt::CaseInsensitive));
        QVERIFY(existing.open(QIODevice::ReadOnly));
        QCOMPARE(existing.readAll(), QByteArray("keep me"));
    }

    void rejectsDestinationInsideSource() {
        if (geteuid() == 0) QSKIP("Les tests fonctionnels doivent être lancés sans privilèges root.");
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString source = temporary.path() + QStringLiteral("/source");
        QVERIFY(QDir().mkpath(source));
        const QString output = source + QStringLiteral("/backup.tar.gz");
        const BackupResult result = createBackupArchive({source}, output);
        QVERIFY(!result.success);
        QVERIFY(result.error.contains(QStringLiteral("destination"), Qt::CaseInsensitive));
        QVERIFY(!QFileInfo::exists(output));
    }

    void toolFailureLeavesNoFinalOrTemporaryFile() {
        if (geteuid() == 0) QSKIP("Les tests fonctionnels doivent être lancés sans privilèges root.");
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString source = temporary.path() + QStringLiteral("/source");
        QVERIFY(QDir().mkpath(source));
        const QString output = temporary.path() + QStringLiteral("/failed.tar.gz");
        const BackupResult result = createBackupArchive({source}, output, nullptr,
                                                        QStringLiteral("/bin/false"));
        QVERIFY(!result.success);
        QVERIFY(!QFileInfo::exists(output));
        const QStringList temporaryFiles = QDir(temporary.path()).entryList(
            {QStringLiteral(".kyronext-backup-*")}, QDir::Files | QDir::Hidden);
        QVERIFY(temporaryFiles.isEmpty());
    }

    void cancellationBeforeStartCreatesNothing() {
        if (geteuid() == 0) QSKIP("Les tests fonctionnels doivent être lancés sans privilèges root.");
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        const QString source = temporary.path() + QStringLiteral("/source");
        QVERIFY(QDir().mkpath(source));
        std::atomic_bool cancel{true};
        const QString output = temporary.path() + QStringLiteral("/cancelled.tar.gz");
        const BackupResult result = createBackupArchive({source}, output, &cancel);
        QVERIFY(result.cancelled);
        QVERIFY(!QFileInfo::exists(output));
    }
};

QTEST_GUILESS_MAIN(BackupTest)
#include "backup_test.moc"
