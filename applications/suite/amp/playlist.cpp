#include "playlist.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextStream>

namespace Amp {
bool isReadableLocalAudio(const QUrl &url) {
    if (!url.isLocalFile() || (!url.host().isEmpty() && url.host() != QStringLiteral("localhost"))) return false;
    const QFileInfo file(url.toLocalFile());
    return file.isFile() && file.isReadable();
}
QList<QUrl> loadM3u(const QString &fileName, QString *error) {
    if (error) error->clear();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024) {
        if (error) *error = QStringLiteral("Playlist illisible ou supérieure à 1 Mio.");
        return {};
    }
    QTextStream input(&file);
    input.setEncoding(QStringConverter::Utf8);
    QList<QUrl> tracks;
    const QDir directory = QFileInfo(fileName).absoluteDir();
    int ignored = 0;
    while (!input.atEnd()) {
        const QString line = input.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        QUrl url;
        if (QFileInfo(line).isAbsolute()) url = QUrl::fromLocalFile(line);
        else if (line.contains("://")) url = QUrl(line);
        else url = QUrl::fromLocalFile(directory.absoluteFilePath(line));
        if (isReadableLocalAudio(url)) tracks.append(url);
        else ++ignored;
        if (tracks.size() >= 10000) break;
    }
    if (error && ignored) *error = QStringLiteral("%1 entrée(s) distante(s), absente(s) ou illisible(s) ignorée(s).").arg(ignored);
    return tracks;
}
bool saveM3u(const QString &fileName, const QList<QUrl> &tracks, QString *error) {
    if (error) error->clear();
    QByteArray content("#EXTM3U\n");
    for (const QUrl &url : tracks) {
        const QString path = url.toLocalFile();
        if (!url.isLocalFile() || (!url.host().isEmpty() && url.host() != QStringLiteral("localhost")) || path.contains('\n') || path.contains('\r')) {
            if (error) *error = QStringLiteral("La playlist contient un chemin non local ou invalide.");
            return false;
        }
        content += QFileInfo(path).absoluteFilePath().toUtf8() + '\n';
    }
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size() || !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}
int nextIndex(int current, int count, bool repeat, bool shuffle, unsigned sample) {
    if (count <= 0) return -1;
    if (current < 0 || current >= count) return 0;
    if (shuffle && count > 1) {
        int candidate = int(sample % unsigned(count - 1));
        return candidate >= current ? candidate + 1 : candidate;
    }
    if (current + 1 < count) return current + 1;
    return repeat ? 0 : -1;
}
QString displayTime(qint64 value) {
    const qint64 seconds = qMax<qint64>(0, value / 1000);
    return QStringLiteral("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}
}
