#include "document.h"
#include <QtTest>
#include <QTemporaryDir>

class DocumentTest : public QObject {
    Q_OBJECT
private slots:
    void fillRespectsBoundaryAndUndo() {
        PaintDocument d; QVERIFY(d.create(32,32));
        d.shape({5,5},{25,25},Qt::black,2,1);
        d.fill({15,15},Qt::blue);
        QCOMPARE(d.image().pixelColor(15,15),QColor(Qt::blue));
        QCOMPARE(d.image().pixelColor(1,1),QColor(Qt::white));
        QVERIFY(d.undo()); QCOMPARE(d.image().pixelColor(15,15),QColor(Qt::white));
        QVERIFY(d.redo()); QCOMPARE(d.image().pixelColor(15,15),QColor(Qt::blue));
    }
    void cropResizeRotatePreserveHistory() {
        PaintDocument d; QVERIFY(d.create(30,20));
        d.beginStroke(); d.line({4,5},{10,5},Qt::red,3);
        QVERIFY(d.crop({2,2,15,10})); QCOMPARE(d.image().size(),QSize(15,10));
        QCOMPARE(d.image().pixelColor(3,3),QColor(Qt::red));
        d.rotateClockwise(); QCOMPARE(d.image().size(),QSize(10,15));
        QVERIFY(d.undo()); QCOMPARE(d.image().size(),QSize(15,10));
        QVERIFY(d.resizeImage({60,40})); QCOMPARE(d.image().size(),QSize(60,40));
        QVERIFY(d.undo()); QCOMPARE(d.image().size(),QSize(15,10));
        const QImage previous=d.image(); QString error;
        QVERIFY(!d.crop({100,100,2,2},&error)); QCOMPARE(d.image(),previous);
        QVERIFY(!d.resizeImage({100000,100000},&error)); QCOMPARE(d.image(),previous);
    }
    void shapesAndTextProducePixels() {
        PaintDocument d; QVERIFY(d.create(80,80));
        d.shape({5,5},{40,5},Qt::red,4,0);
        QCOMPARE(d.image().pixelColor(20,5),QColor(Qt::red));
        d.shape({10,15},{40,45},Qt::blue,4,2);
        QVERIFY(d.image().pixelColor(25,15)!=QColor(Qt::white));
        const QImage before=d.image(); d.text({4,70},"Kyronext",Qt::black,QFont("Sans Serif",12));
        QVERIFY(d.image()!=before); QVERIFY(d.undo()); QCOMPARE(d.image(),before);
    }

    void paintUndoRedo() {
        PaintDocument doc;
        QVERIFY(doc.create(64, 64));
        doc.beginStroke(); doc.line({5, 10}, {50, 10}, Qt::red, 6);
        QCOMPARE(doc.image().pixelColor(20, 10), QColor(Qt::red));
        QVERIFY(doc.modified()); QVERIFY(doc.undo());
        QCOMPARE(doc.image().pixelColor(20, 10), QColor(Qt::white));
        QVERIFY(doc.redo()); QCOMPARE(doc.image().pixelColor(20, 10), QColor(Qt::red));
        doc.beginStroke(); doc.line({5, 10}, {50, 10}, Qt::red, 6, true);
        QCOMPARE(doc.image().pixelColor(20, 10).alpha(), 0);
    }
    void saveAndReload() {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        PaintDocument doc; doc.create(30, 20); doc.beginStroke(); doc.line({2, 5}, {20, 5}, Qt::blue, 4);
        QString error; const QString file = directory.filePath("dessin.png");
        QVERIFY2(doc.save(file, "png", &error), qPrintable(error)); QVERIFY(!doc.modified());
        PaintDocument loaded; QVERIFY2(loaded.load(file, &error), qPrintable(error));
        QCOMPARE(loaded.image(), doc.image());
    }
    void invalidOperationsPreserveImage() {
        PaintDocument doc; doc.create(16, 16); const QImage original = doc.image(); QString error;
        QVERIFY(!doc.create(100000, 100000, &error)); QCOMPARE(doc.image(), original);
        QVERIFY(!doc.load("/file-that-does-not-exist", &error)); QCOMPARE(doc.image(), original);
        QVERIFY(!doc.save("/file-that-does-not-exist/out.png", "png", &error)); QCOMPARE(doc.image(), original);
        QVERIFY(!doc.save("invalid.xyz", "xyz", &error)); QCOMPARE(doc.image(), original);
    }
    void redoInvalidatedByNewStroke() {
        PaintDocument doc; doc.create(16, 16); doc.beginStroke(); doc.line({1, 1}, {8, 8}, Qt::red, 2);
        QVERIFY(doc.undo()); doc.beginStroke(); doc.line({2, 2}, {5, 5}, Qt::blue, 2);
        QVERIFY(!doc.redo());
    }
};
QTEST_MAIN(DocumentTest)
#include "paint_document_test.moc"
