#include "../imageio.h"
#include <QTemporaryDir>
#include <QFile>
#include <QTransform>
#include <QtTest>

class ImageTests : public QObject {
    Q_OBJECT
private slots:
    void pngRoundTripAndRotation() {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        QImage source(3, 2, QImage::Format_ARGB32); source.fill(Qt::transparent); source.setPixelColor(0, 0, Qt::red);
        QString error; auto path = directory.filePath("image.png"); QVERIFY(AccessoryImages::save(source, path, error));
        auto loaded = AccessoryImages::load(path, error); QCOMPARE(loaded.size(), source.size()); QCOMPARE(loaded.pixelColor(0, 0), QColor(Qt::red));
        QCOMPARE(loaded.pixelColor(2, 1).alpha(), 0);
        auto rotated = loaded.transformed(QTransform().rotate(90)); QCOMPARE(rotated.size(), QSize(2, 3));
        QCOMPARE(rotated.pixelColor(1, 0), QColor(Qt::red));
    }
    void jpegFlattensTransparencyAndBmpWorks() {
        QTemporaryDir directory; QImage source(8, 8, QImage::Format_ARGB32); source.fill(Qt::transparent);
        QString error;
        QVERIFY2(AccessoryImages::save(source, directory.filePath("photo.jpg"), error), qPrintable(error));
        auto jpeg = AccessoryImages::load(directory.filePath("photo.jpg"), error); QVERIFY(!jpeg.isNull()); QVERIFY(jpeg.pixelColor(4, 4).red() > 245);
        QVERIFY(AccessoryImages::save(source, directory.filePath("photo.bmp"), error));
        QVERIFY(!AccessoryImages::load(directory.filePath("photo.bmp"), error).isNull());
    }
    void invalidFileDoesNotChangeOriginal() {
        QTemporaryDir directory; auto path = directory.filePath("keep.txt"); QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); file.write("original"); file.close();
        QString error; QImage source(8, 8, QImage::Format_RGB32); source.fill(Qt::blue);
        QVERIFY(!AccessoryImages::save(source, path, error)); QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), QByteArray("original")); file.close();
        QVERIFY(AccessoryImages::load(path, error).isNull()); QVERIFY(!error.isEmpty());
        QVERIFY(!AccessoryImages::save(source, directory.filePath("missing/image.png"), error));
    }
};
QTEST_MAIN(ImageTests)
#include "image_test.moc"
