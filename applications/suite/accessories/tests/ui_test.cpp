#define main GalleryCheckEntry
#include "../gallery.cpp"
#undef main
#define main VideoCheckEntry
#include "../video.cpp"
#undef main
#define main CaptureCheckEntry
#include "../capture.cpp"
#undef main
#include <QTemporaryDir>
#include <QtTest>

class AccessoryUiTests : public QObject {
    Q_OBJECT
private slots:
    void galleryLoadsAndRotatesViaRealAction() {
        QTemporaryDir directory; QImage source(60, 30, QImage::Format_RGB32); source.fill(Qt::red);
        QString file = directory.filePath("red.png"); QVERIFY(source.save(file));
        Gallery gallery; gallery.show(); QVERIFY(gallery.open(file)); QCoreApplication::processEvents();
        QLabel *picture = nullptr;
        for (auto *label : gallery.findChildren<QLabel *>()) if (label->accessibleName() == "Image courante") picture = label;
        QVERIFY(picture); QVERIFY(!picture->pixmap().isNull()); QVERIFY(picture->pixmap().width() > picture->pixmap().height());
        QAction *rotate = nullptr;
        for (auto *action : gallery.findChildren<QAction *>()) if (action->text() == "Tourner à droite") rotate = action;
        QVERIFY(rotate); rotate->trigger(); QCoreApplication::processEvents();
        QVERIFY(picture->pixmap().height() > picture->pixmap().width());
        QVERIFY(gallery.statusBar()->currentMessage().contains("rotation non enregistrée"));
        QAction *help = nullptr;
        for (auto *action : gallery.findChildren<QAction *>()) if (action->text() == "Mode d'emploi hors ligne") help = action;
        QVERIFY(help); QCOMPARE(help->shortcut(), QKeySequence(QKeySequence::HelpContents)); bool shown = false;
        QTimer::singleShot(50, [&]{ auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if (dialog) { shown = dialog->text().contains("16 millions"); dialog->accept(); } });
        help->trigger(); QVERIFY(shown);
    }
    void videoStartsWithoutAutoSource() {
        Video video; video.show(); QCoreApplication::processEvents();
        QVERIFY(video.windowTitle().contains("Kyronext Video"));
        auto *widget = video.findChild<QVideoWidget *>(); QVERIFY(widget);
        auto combos = video.findChildren<QComboBox *>(); QCOMPARE(combos.size(), 2);
        for (auto *combo : combos) QVERIFY(!combo->isEnabled());
        QVERIFY(video.statusBar()->currentMessage().contains("Aucun téléchargement"));
    }
    void captureFallsBackWhenNativeCaptureIsUnavailable() {
        Capture capture; capture.show(); QCoreApplication::processEvents();
        QPushButton *take = nullptr, *save = nullptr;
        for (auto *button : capture.findChildren<QPushButton *>()) {
            if (button->text().startsWith("Capturer")) take = button;
            if (button->text().startsWith("Enregistrer")) save = button;
        }
        QVERIFY(take); QVERIFY(save); QVERIFY(!save->isEnabled()); bool explained = false;
        QTimer::singleShot(50, [&]{
            auto *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (message) { explained = message->text().contains("Spectacle"); message->accept(); }
            else explained = capture.statusBar()->currentMessage().contains("Spectacle");
        });
        take->click(); QVERIFY(explained); QVERIFY(!save->isEnabled());
    }
};
QTEST_MAIN(AccessoryUiTests)
#include "ui_test.moc"
