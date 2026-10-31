// Compile the actual application window into the smoke test, without replacing
// the audio backend or painting simulated widgets.
#define main ampProgramMain
#include "../main.cpp"
#undef main
#include <QtTest>
#include <QTemporaryDir>
#include <QScrollBar>

class AmpUiTests final : public QObject {
    Q_OBJECT
private slots:
    void controlsAndRealCapture() {
        QTemporaryDir settings;QVERIFY(settings.isValid());
        QCoreApplication::setOrganizationName("KyronextTests");QCoreApplication::setApplicationName("AmpUi");
        QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settings.path());
        AmpWindow window;window.resize(900,800);window.show();QTest::qWait(100);
        QCheckBox *dsp=nullptr;
        for(auto *box:window.findChildren<QCheckBox*>())if(box->text().startsWith("Lecture DSP"))dsp=box;
        QVERIFY(dsp);QVERIFY(!dsp->isChecked());
        dsp->setChecked(true);QTest::qWait(100);
        auto *group=window.findChild<QGroupBox*>();QVERIFY(group);QVERIFY(group->isEnabled());QVERIFY(group->isVisible());
        int bands=0;for(auto *slider:group->findChildren<QSlider*>())if(slider->orientation()==Qt::Vertical){++bands;QCOMPARE(slider->minimum(),-12);QCOMPARE(slider->maximum(),12);slider->setValue(3);}
        QCOMPARE(bands,10);
        QVERIFY(window.grab().save("amp-dsp-preview.png"));
        window.resize(800,600);QTest::qWait(50);QCOMPARE(window.size(),QSize(800,600));
        auto *scroll=window.findChild<QScrollArea*>();QVERIFY(scroll);QVERIFY(scroll->verticalScrollBar()->maximum()>0);
        scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());QTest::qWait(50);
        QVERIFY(window.grab().save("amp-dsp-small-screen.png"));
        QTest::keyClick(&window,Qt::Key_F11);QTest::qWait(50);QVERIFY(window.isFullScreen());
        QTest::keyClick(&window,Qt::Key_F11);QTest::qWait(50);QVERIFY(!window.isFullScreen());
        dsp->setChecked(false);QVERIFY(!group->isVisible());
        window.close();
    }
};
QTEST_MAIN(AmpUiTests)
#include "test_ui.moc"
