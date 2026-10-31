#include "playlist.h"
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QDataStream>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
#include <cmath>

static QByteArray makeWave() {
    constexpr int sampleRate = 8000, frames = 8000;
    QByteArray wave;
    QDataStream stream(&wave, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4); stream << quint32(36 + frames * 2);
    stream.writeRawData("WAVEfmt ", 8); stream << quint32(16) << quint16(1) << quint16(1);
    stream << quint32(sampleRate) << quint32(sampleRate * 2) << quint16(2) << quint16(16);
    stream.writeRawData("data", 4); stream << quint32(frames * 2);
    for (int i = 0; i < frames; ++i)
        stream << qint16(12000 * std::sin(2.0 * 3.14159265358979323846 * 440.0 * i / sampleRate));
    return wave;
}

class AmpTest : public QObject {
    Q_OBJECT
private slots:
    void relativePlaylistAndRemoteRejection() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile track(dir.filePath(QStringLiteral("été.wav")));
        QVERIFY(track.open(QIODevice::WriteOnly)); track.write(makeWave()); track.close();
        QFile playlist(dir.filePath(QStringLiteral("music.m3u8")));
        QVERIFY(playlist.open(QIODevice::WriteOnly));
        playlist.write(QStringLiteral("#EXTM3U\nété.wav\nhttps://example.invalid/radio\nabsent.wav\n").toUtf8());
        playlist.close();
        QString message;
        const auto entries = Amp::loadM3u(playlist.fileName(), &message);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries.first(), QUrl::fromLocalFile(track.fileName()));
        QVERIFY(message.contains(QStringLiteral("2")));
        QVERIFY(!Amp::isReadableLocalAudio(QUrl(QStringLiteral("https://example.invalid/radio"))));
        QVERIFY(!Amp::isReadableLocalAudio(QUrl(QStringLiteral("file://example.invalid/music.wav"))));
    }
    void roundTripAndFailurePreserveDestination() {
        QTemporaryDir dir;
        QFile track(dir.filePath(QStringLiteral("with space #1.wav")));
        QVERIFY(track.open(QIODevice::WriteOnly)); track.write(makeWave()); track.close();
        const QString path = dir.filePath(QStringLiteral("export.m3u8"));
        const QList<QUrl> tracks{QUrl::fromLocalFile(track.fileName()), QUrl::fromLocalFile(track.fileName())};
        QString error;
        QVERIFY2(Amp::saveM3u(path, tracks, &error), qPrintable(error));
        QCOMPARE(Amp::loadM3u(path), tracks);
        QFile existing(path); QVERIFY(existing.open(QIODevice::ReadOnly));
        const QByteArray original = existing.readAll(); existing.close();
        QVERIFY(!Amp::saveM3u(path, {QUrl(QStringLiteral("https://example.invalid"))}, &error));
        QVERIFY(existing.open(QIODevice::ReadOnly));
        QCOMPARE(existing.readAll(), original);
    }
    void nextSelectionAndTime() {
        QCOMPARE(Amp::nextIndex(-1, 0, false, false), -1);
        QCOMPARE(Amp::nextIndex(-1, 3, false, false), 0);
        QCOMPARE(Amp::nextIndex(2, 3, false, false), -1);
        QCOMPARE(Amp::nextIndex(2, 3, true, false), 0);
        for (unsigned sample = 0; sample < 100; ++sample) {
            const int next = Amp::nextIndex(1, 4, false, true, sample);
            QVERIFY(next >= 0 && next < 4 && next != 1);
        }
        QCOMPARE(Amp::displayTime(-10), QStringLiteral("0:00"));
        QCOMPARE(Amp::displayTime(65000), QStringLiteral("1:05"));
    }
    void decodeRealGeneratedAudio() {
        QTemporaryDir dir;
        QFile audio(dir.filePath(QStringLiteral("tone.wav")));
        QVERIFY(audio.open(QIODevice::WriteOnly)); audio.write(makeWave()); audio.close();
        QAudioDecoder decoder;
        QSignalSpy finished(&decoder, &QAudioDecoder::finished);
        qint64 frames = 0;
        bool nonSilent = false;
        connect(&decoder, &QAudioDecoder::bufferReady, this, [&] {
            const QAudioBuffer buffer = decoder.read();
            if (!buffer.isValid()) return;
            frames += buffer.frameCount();
            const char *bytes = buffer.constData<char>();
            for (int i = 0; i < buffer.byteCount(); ++i) if (bytes[i]) nonSilent = true;
        });
        decoder.setSource(QUrl::fromLocalFile(audio.fileName()));
        decoder.start();
        QTRY_VERIFY_WITH_TIMEOUT(finished.count() > 0 || decoder.error() != QAudioDecoder::NoError, 10000);
        QVERIFY2(decoder.error() == QAudioDecoder::NoError, qPrintable(decoder.errorString()));
        QVERIFY(frames >= 7900 && frames <= 8100);
        QVERIFY(nonSilent);
    }
};
QTEST_GUILESS_MAIN(AmpTest)
#include "test_amp.moc"
