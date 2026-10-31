#include "clock_window.h"
#include <QtTest>
class ClockTest:public QObject {
    Q_OBJECT
private slots:
    void stopwatchStartPauseReset() {
        ClockWindow window;window.show();auto *toggle=window.findChild<QPushButton*>("stopwatchToggle");auto *reset=window.findChild<QPushButton*>("stopwatchReset");auto *display=window.findChild<QLabel*>("stopwatchDisplay");QVERIFY(toggle && reset && display);
        toggle->click();QTest::qWait(250);toggle->click();QVERIFY(display->text()!="00:00:00.0");const auto paused=display->text();QTest::qWait(150);QCOMPARE(display->text(),paused);reset->click();QCOMPARE(display->text(),QString("00:00:00.0"));
    }
    void countdownShowsRealNotification() {
        ClockWindow window;window.show();auto *seconds=window.findChild<QSpinBox*>("countdownSeconds");auto *start=window.findChild<QPushButton*>("countdownStart");QVERIFY(seconds && start);seconds->setValue(1);bool observed=false;
        QTimer watcher;watcher.setInterval(50);connect(&watcher,&QTimer::timeout,this,[&]{for(auto *widget:QApplication::topLevelWidgets())if(auto *message=qobject_cast<QMessageBox*>(widget)){if(message->text().contains("Minuteur termine")){observed=true;message->accept();}}});watcher.start();start->click();QTRY_VERIFY_WITH_TIMEOUT(observed,4500);
    }
};
QTEST_MAIN(ClockTest)
#include "clock_test.moc"
