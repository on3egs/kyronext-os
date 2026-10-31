#pragma once
#include <QList>
#include <QString>
#include <QUrl>

namespace Amp {
bool isReadableLocalAudio(const QUrl &url);
QList<QUrl> loadM3u(const QString &fileName, QString *error = nullptr);
bool saveM3u(const QString &fileName, const QList<QUrl> &tracks, QString *error = nullptr);
// sample is an injected random value, keeping the selection testable.
int nextIndex(int current, int count, bool repeat, bool shuffle, unsigned sample = 0);
QString displayTime(qint64 milliseconds);
}
