#define KYRONEXT_MONITOR_NO_MAIN
#include "../monitor.cpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QTabWidget>
#include <QTimer>

namespace {
bool metricsCollected(const MonitorWindow &window) {
    for (const auto *label : window.findChildren<QLabel *>()) {
        if (label->text() == QStringLiteral("Collecte…"))
            return false;
    }
    return true;
}

bool captureOverview(MonitorWindow &window, const QString &path, const QString &chartPath) {
    auto *tabs = window.findChild<QTabWidget *>();
    if (!tabs || tabs->count() == 0)
        return false;

    auto *scroll = qobject_cast<QScrollArea *>(tabs->widget(0));
    if (!scroll || !scroll->widget())
        return false;

    tabs->setCurrentIndex(0);
    window.resize(1120, 520);
    window.layout()->activate();
    scroll->widget()->layout()->activate();
    QApplication::processEvents();

    auto *bar = scroll->verticalScrollBar();
    if (bar->maximum() <= 0)
        return false;

    bar->setValue(0);
    QApplication::processEvents();
    const QImage overview = window.grab().toImage();
    if (overview.isNull() || overview.width() < 640 || overview.height() < 480
        || !overview.save(path))
        return false;

    const auto chartPanels = scroll->findChildren<QFrame *>(QStringLiteral("monitorChartPanel"));
    if (chartPanels.size() < 2)
        return false;

    window.resize(720, 520);
    window.layout()->activate();
    scroll->widget()->layout()->activate();
    QApplication::processEvents();
    const int firstChartY = chartPanels[0]->mapTo(scroll->widget(), QPoint(0, 0)).y();
    const int secondChartY = chartPanels[1]->mapTo(scroll->widget(), QPoint(0, 0)).y();
    if (secondChartY <= firstChartY)
        return false;

    window.resize(1120, 520);
    window.layout()->activate();
    scroll->widget()->layout()->activate();
    QApplication::processEvents();
    const int chartTop = chartPanels.first()->mapTo(scroll->widget(), QPoint(0, 0)).y();
    bar->setValue(qBound(0, chartTop, bar->maximum()));
    QApplication::processEvents();

    const QRect viewportRect = scroll->viewport()->rect();
    for (const auto *panel : chartPanels) {
        const QRect panelRect(panel->mapTo(scroll->viewport(), QPoint(0, 0)), panel->size());
        if (!panelRect.intersects(viewportRect) || panelRect.top() < 0 || panelRect.top() > 12)
            return false;
    }

    const QImage lowerCharts = window.grab().toImage();
    return !lowerCharts.isNull() && lowerCharts.size() == overview.size()
        && lowerCharts != overview && lowerCharts.save(chartPath);
}
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    QApplication app(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext Monitor preview"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());

    MonitorWindow window;
    window.resize(1120, 520);
    window.show();

    const QString path = QString::fromLocal8Bit(argv[1]);
    const QFileInfo output(path);
    const QString chartPath = output.dir().filePath(output.completeBaseName() + QStringLiteral("-graphs.png"));

    auto *poll = new QTimer(&app);
    poll->setInterval(100);
    auto *deadline = new QElapsedTimer;
    deadline->start();
    QObject::connect(poll, &QTimer::timeout, &app, [&app, &window, poll, deadline, path, chartPath] {
        if (!metricsCollected(window) || deadline->elapsed() < 4500) {
            if (deadline->elapsed() >= 12000)
                app.exit(1);
            return;
        }

        poll->stop();
        QTimer::singleShot(100, &app, [&app, &window, path, chartPath] {
            app.exit(captureOverview(window, path, chartPath) ? 0 : 1);
        });
    });
    poll->start();
    return app.exec();
}
