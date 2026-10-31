#include "metrics.h"
#include <kyronext/ui.h>

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSet>
#include <QStorageInfo>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QSysInfo>

#include <cmath>
#include <algorithm>
#include <limits>

namespace {
const QColor cpuColor("#ef3c60");
const QColor memoryColor("#4aa8ff");
const QColor networkInColor("#43d49b");
const QColor networkOutColor("#e9ad52");

QString formatBytes(quint64 bytes) {
    const quint64 limit = quint64(std::numeric_limits<qint64>::max());
    return QLocale().formattedDataSize(qint64(qMin(bytes, limit)));
}

QString formatRate(double bytesPerSecond) {
    if (bytesPerSecond < 0.0 || !std::isfinite(bytesPerSecond))
        return QStringLiteral("Mesure en cours");
    const double limit = double(std::numeric_limits<qint64>::max());
    return formatBytes(quint64(qMin(bytesPerSecond, limit))) + QStringLiteral("/s");
}

QString formatPercent(double value) {
    return QLocale().toString(value, 'f', 1) + QStringLiteral(" %");
}

QString formatDuration(qint64 minutes) {
    if (minutes < 0) return QStringLiteral("indisponible");
    const qint64 hours = minutes / 60;
    const qint64 rest = minutes % 60;
    if (hours == 0) return QStringLiteral("%1 min").arg(rest);
    return QStringLiteral("%1 h %2 min").arg(hours).arg(rest);
}

QFrame *makeInfoPanel(const QString &title, QLabel **valueOut) {
    auto *panel = new QFrame;
    panel->setObjectName(QStringLiteral("monitorInfoPanel"));
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setStyleSheet(QStringLiteral(
        "QFrame#monitorInfoPanel{background:#151a22;border:1px solid #303a49;border-radius:12px;}"));
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(18, 16, 18, 17);
    layout->setSpacing(10);
    auto *heading = new QLabel(title, panel);
    heading->setStyleSheet(QStringLiteral("font-size:15px;font-weight:700;color:#f0f2f6;border:0;background:transparent;"));
    layout->addWidget(heading);
    auto *value = new QLabel(panel);
    value->setWordWrap(true);
    value->setTextInteractionFlags(Qt::TextSelectableByMouse);
    value->setStyleSheet(QStringLiteral("font-size:13px;color:#c4ccd7;line-height:1.35;border:0;background:transparent;"));
    value->setMinimumHeight(64);
    layout->addWidget(value);
    if (valueOut) *valueOut = value;
    return panel;
}

QWidget *makeScrollPage(QTabWidget *tabs, const QString &title, QVBoxLayout **layoutOut) {
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(4, 10, 4, 18);
    layout->setSpacing(14);
    scroll->setWidget(content);
    tabs->addTab(scroll, title);
    if (layoutOut) *layoutOut = layout;
    return content;
}
}

class NumericItem final : public QTableWidgetItem {
public:
    double number = -1.0;
    bool operator<(const QTableWidgetItem &other) const override {
        const auto *numeric = dynamic_cast<const NumericItem *>(&other);
        return numeric ? number < numeric->number : QTableWidgetItem::operator<(other);
    }
};

class MetricCard final : public QFrame {
public:
    MetricCard(const QString &title, const QColor &accent, QWidget *parent = nullptr)
        : QFrame(parent) {
        setObjectName(QStringLiteral("monitorMetricCard"));
        setFrameShape(QFrame::StyledPanel);
        setMinimumHeight(145);
        setStyleSheet(QStringLiteral(
            "QFrame#monitorMetricCard{background:#151a22;border:1px solid #303a49;border-left:3px solid %1;border-radius:12px;}"
            "QProgressBar{height:7px;min-height:7px;max-height:7px;border:0;border-radius:4px;background:#272e39;}"
            "QProgressBar::chunk{border-radius:4px;background:%1;}"
        ).arg(accent.name()));
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(17, 14, 17, 15);
        layout->setSpacing(6);
        title_ = new QLabel(title, this);
        title_->setStyleSheet(QStringLiteral("font-size:11px;font-weight:700;letter-spacing:1px;color:#9ca8b8;border:0;background:transparent;"));
        value_ = new QLabel(QStringLiteral("—"), this);
        value_->setMinimumHeight(37);
        value_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        value_->setStyleSheet(QStringLiteral("font-size:26px;font-weight:700;color:#f5f6f8;border:0;background:transparent;"));
        detail_ = new QLabel(QStringLiteral("Collecte des mesures"), this);
        detail_->setWordWrap(true);
        detail_->setStyleSheet(QStringLiteral("font-size:12px;color:#b3bdca;border:0;background:transparent;"));
        progress_ = new QProgressBar(this);
        progress_->setRange(0, 100);
        progress_->setTextVisible(false);
        progress_->hide();
        layout->addWidget(title_);
        layout->addWidget(value_);
        layout->addWidget(detail_, 1);
        layout->addWidget(progress_);
    }

    void setMetric(const QString &value, const QString &detail, int percent = -1) {
        value_->setText(value);
        detail_->setText(detail);
        progress_->setVisible(percent >= 0);
        if (percent >= 0) progress_->setValue(qBound(0, percent, 100));
        setAccessibleName(title_->text() + QStringLiteral(" : ") + value);
        setAccessibleDescription(detail);
    }

private:
    QLabel *title_ = nullptr;
    QLabel *value_ = nullptr;
    QLabel *detail_ = nullptr;
    QProgressBar *progress_ = nullptr;
};

class HistoryChart final : public QWidget {
public:
    enum class Scale { Percent, BytesPerSecond };
    struct Sample { double first = -1.0; double second = -1.0; };

    explicit HistoryChart(Scale scale, const QString &description,
                          const QString &firstSeries, const QString &secondSeries,
                          QWidget *parent = nullptr)
        : QWidget(parent), scale_(scale), firstSeries_(firstSeries), secondSeries_(secondSeries) {
        setMinimumHeight(205);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAccessibleName(description);
        setAccessibleDescription(QStringLiteral("Courbes locales, sans interpolation entre mesures indisponibles."));
    }

    void append(double first, double second) {
        samples_.append({first, second});
        while (samples_.size() > 120) samples_.removeFirst();
        const auto describe = [this](double value) {
            if (value < 0.0 || !std::isfinite(value)) return QStringLiteral("indisponible");
            return scale_ == Scale::Percent
                ? QLocale().toString(value, 'f', 1) + QStringLiteral(" %")
                : formatRate(value);
        };
        setAccessibleDescription(QStringLiteral("Dernière mesure · ") + firstSeries_ + QStringLiteral(" : ")
            + describe(first) + QStringLiteral(" · ") + secondSeries_ + QStringLiteral(" : ")
            + describe(second) + QStringLiteral(". Les données restent locales."));
        setToolTip(firstSeries_ + QStringLiteral(" : ") + describe(first) + QStringLiteral("\n")
            + secondSeries_ + QStringLiteral(" : ") + describe(second));
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor(QStringLiteral("#11161e")));

        const qreal valueGutter = scale_ == Scale::BytesPerSecond ? 94.0 : 48.0;
        const QRectF plot = QRectF(rect()).adjusted(valueGutter, 12, -12, -30);
        if (plot.width() < 40 || plot.height() < 40) return;

        double maximum = 0.0;
        if (scale_ == Scale::BytesPerSecond) {
            for (const auto &sample : samples_) {
                if (sample.first >= 0.0 && std::isfinite(sample.first)) maximum = qMax(maximum, sample.first);
                if (sample.second >= 0.0 && std::isfinite(sample.second)) maximum = qMax(maximum, sample.second);
            }
            maximum = qMax(1024.0, maximum * 1.18);
        } else {
            maximum = 100.0;
        }

        QFont axisFont = painter.font();
        axisFont.setPointSizeF(qMax(8.0, axisFont.pointSizeF() - 1.0));
        painter.setFont(axisFont);
        for (int line = 0; line <= 4; ++line) {
            const qreal ratio = qreal(line) / 4.0;
            const qreal y = plot.top() + ratio * plot.height();
            painter.setPen(QColor(QStringLiteral("#35404e")));
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
            QString label;
            if (scale_ == Scale::Percent) {
                label = QStringLiteral("%1%").arg(100 - line * 25);
            } else {
                label = formatRate(maximum * (1.0 - ratio));
            }
            if (!label.isEmpty()) {
                painter.setPen(QColor(QStringLiteral("#b0bac8")));
                painter.drawText(QRectF(0, y - 8, valueGutter - 6.0, 16),
                                 Qt::AlignRight | Qt::AlignVCenter, label);
            }
        }

        painter.setPen(QColor(QStringLiteral("#aab4c2")));
        painter.drawText(QRectF(plot.left(), plot.bottom() + 7, 100, 18), Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Plus ancien"));
        painter.drawText(QRectF(plot.right() - 100, plot.bottom() + 7, 100, 18), Qt::AlignRight | Qt::AlignVCenter,
                         QStringLiteral("Maintenant"));

        const bool hasPoint = std::any_of(samples_.cbegin(), samples_.cend(), [](const Sample &sample) {
            return (sample.first >= 0.0 && std::isfinite(sample.first))
                || (sample.second >= 0.0 && std::isfinite(sample.second));
        });
        if (!hasPoint) {
            painter.setPen(QColor(QStringLiteral("#9ca8b8")));
            painter.drawText(plot, Qt::AlignCenter, QStringLiteral("Aucune mesure exploitable pour ce graphique."));
            return;
        }

        drawSeries(painter, plot, maximum, 0, scale_ == Scale::Percent ? cpuColor : networkInColor);
        drawSeries(painter, plot, maximum, 1, scale_ == Scale::Percent ? memoryColor : networkOutColor);
    }

private:
    Scale scale_;
    QString firstSeries_;
    QString secondSeries_;
    QVector<Sample> samples_;

    void drawSeries(QPainter &painter, const QRectF &plot, double maximum,
                    int series, const QColor &color) const {
        QPainterPath path;
        bool segmentOpen = false;
        int validPoints = 0;
        QPointF latestPoint;
        const int count = samples_.size();
        for (int i = 0; i < count; ++i) {
            const double value = series == 0 ? samples_[i].first : samples_[i].second;
            if (value < 0.0 || !std::isfinite(value)) {
                segmentOpen = false;
                continue;
            }
            const double bounded = qBound(0.0, value, maximum);
            const qreal x = plot.left() + (count <= 1 ? 1.0 : qreal(i) / qreal(count - 1)) * plot.width();
            const qreal y = plot.bottom() - qreal(bounded / maximum) * plot.height();
            if (segmentOpen) path.lineTo(x, y);
            else path.moveTo(x, y);
            segmentOpen = true;
            latestPoint = QPointF(x, y);
            ++validPoints;
        }
        if (validPoints == 0) return;
        painter.setPen(QPen(color, 2.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(path);
        painter.setPen(QPen(QColor(QStringLiteral("#11161e")), 1.5));
        painter.setBrush(color);
        painter.drawEllipse(latestPoint, 4.0, 4.0);
    }
};

namespace {
QFrame *makeChartPanel(const QString &title, const QString &subtitle,
                       const QString &firstLegend, const QColor &firstColor,
                       const QString &secondLegend, const QColor &secondColor,
                       HistoryChart::Scale scale, HistoryChart **chartOut) {
    auto *panel = new QFrame;
    panel->setObjectName(QStringLiteral("monitorChartPanel"));
    panel->setStyleSheet(QStringLiteral(
        "QFrame#monitorChartPanel{background:#151a22;border:1px solid #303a49;border-radius:12px;}"));
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 15, 16, 14);
    layout->setSpacing(6);
    auto *headingRow = new QHBoxLayout;
    auto *heading = new QLabel(title, panel);
    heading->setStyleSheet(QStringLiteral("font-size:15px;font-weight:700;color:#f0f2f6;border:0;background:transparent;"));
    headingRow->addWidget(heading);
    headingRow->addStretch();
    auto *first = new QLabel(QStringLiteral("● %1").arg(firstLegend), panel);
    first->setStyleSheet(QStringLiteral("font-size:11px;font-weight:600;color:%1;border:0;background:transparent;").arg(firstColor.name()));
    headingRow->addWidget(first);
    auto *second = new QLabel(QStringLiteral("● %1").arg(secondLegend), panel);
    second->setStyleSheet(QStringLiteral("font-size:11px;font-weight:600;color:%1;border:0;background:transparent;").arg(secondColor.name()));
    headingRow->addWidget(second);
    layout->addLayout(headingRow);
    auto *caption = new QLabel(subtitle, panel);
    caption->setStyleSheet(QStringLiteral("font-size:11px;color:#909bad;border:0;background:transparent;"));
    layout->addWidget(caption);
    auto *chart = new HistoryChart(scale, title, firstLegend, secondLegend, panel);
    layout->addWidget(chart, 1);
    if (chartOut) *chartOut = chart;
    return panel;
}
}

class MonitorWindow final : public QMainWindow {
public:
    MonitorWindow() {
        setWindowTitle(QStringLiteral("Kyronext Monitor"));
        setMinimumSize(640, 480);
        resize(1120, 780);

        auto *central = new QWidget(this);
        auto *root = new QVBoxLayout(central);
        root->setContentsMargins(20, 14, 20, 14);
        root->setSpacing(12);
        setCentralWidget(central);

        auto *header = new QFrame(central);
        auto *headerLayout = new QVBoxLayout(header);
        headerLayout->setContentsMargins(0, 2, 0, 8);
        headerLayout->setSpacing(8);
        auto *brandRow = new QHBoxLayout;
        auto *brandColumn = new QVBoxLayout;
        brandColumn->setSpacing(2);
        auto *brand = new QLabel(QStringLiteral("KYRONEXT  /  MONITOR"), header);
        brand->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;letter-spacing:1px;color:#f1f3f6;"));
        auto *tagline = new QLabel(QStringLiteral("Vue claire de votre système, en temps réel"), header);
        tagline->setStyleSheet(QStringLiteral("font-size:12px;color:#aab4c2;"));
        brandColumn->addWidget(brand);
        brandColumn->addWidget(tagline);
        brandRow->addLayout(brandColumn);
        brandRow->addStretch();

        auto *live = new QLabel(QStringLiteral("●  EN DIRECT"), header);
        live->setAccessibleName(QStringLiteral("Actualisation locale active"));
        live->setStyleSheet(QStringLiteral(
            "QLabel{background:#2b1821;color:#ff7891;border:1px solid #643043;border-radius:10px;"
            "padding:7px 11px;font-size:11px;font-weight:700;letter-spacing:1px;}"));
        brandRow->addWidget(live);
        headerLayout->addLayout(brandRow);

        auto *controlRow = new QHBoxLayout;
        controlRow->addStretch();
        updated_ = new QLabel(QStringLiteral("En attente de la première mesure"), header);
        updated_->setMinimumWidth(160);
        updated_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        updated_->setStyleSheet(QStringLiteral("font-size:11px;color:#aab4c2;"));
        controlRow->addWidget(updated_);

        refreshButton_ = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload),
                                         QStringLiteral("Actualiser"), header);
        refreshButton_->setAccessibleName(QStringLiteral("Actualiser les mesures maintenant"));
        controlRow->addWidget(refreshButton_);
        compactView_ = new QCheckBox(QStringLiteral("Vue légère"), header);
        compactView_->setToolTip(QStringLiteral("Masquer les graphiques pour réduire l'activité visuelle."));
        compactView_->setAccessibleName(QStringLiteral("Masquer ou afficher les graphiques"));
        controlRow->addWidget(compactView_);
        headerLayout->addLayout(controlRow);
        root->addWidget(header);

        tabs_ = new QTabWidget(central);
        tabs_->setDocumentMode(true);
        root->addWidget(tabs_, 1);
        buildOverviewPage();
        buildHardwarePage();
        buildProcessesPage();
        storageClock_.start();
        connect(tabs_, &QTabWidget::currentChanged, this, [this](int index) {
            if (index == 1) {
                storagePageViewed_ = true;
                refreshStorage();
                storageClock_.restart();
            } else if (index == 2) {
                sample();
            }
        });

        auto *appearance = menuBar()->addMenu(QStringLiteral("Apparence"));
        connect(appearance->addAction(QStringLiteral("Graphite et rouge")), &QAction::triggered,
                this, [] { Kyronext::applyTheme(*qApp, Kyronext::Palette::Red); });
        connect(appearance->addAction(QStringLiteral("Graphite et ambre")), &QAction::triggered,
                this, [] { Kyronext::applyTheme(*qApp, Kyronext::Palette::Amber); });
        connect(appearance->addAction(QStringLiteral("Contraste renforcé")), &QAction::triggered,
                this, [] { Kyronext::applyTheme(*qApp, Kyronext::Palette::HighContrast); });
        Kyronext::addAboutMenu(this, QStringLiteral("Kyronext Monitor"),
            QStringLiteral("Tableau de bord local en lecture seule ; les actions processus restent limitées à votre session."),
            QStringLiteral("Qt 6 Widgets ; /proc, /sys, QStorageInfo et interfaces réseau Linux"));

        connect(refreshButton_, &QPushButton::clicked, this, [this] { sample(); });
        connect(compactView_, &QCheckBox::toggled, this, [this](bool compact) {
            if (resourcePanel_) resourcePanel_->setVisible(!compact);
            if (networkPanel_) networkPanel_->setVisible(!compact);
        });
        auto *refreshAction = new QAction(this);
        refreshAction->setShortcut(QKeySequence::Refresh);
        addAction(refreshAction);
        connect(refreshAction, &QAction::triggered, this, [this] { sample(); });

        timer_ = new QTimer(this);
        timer_->setInterval(2000);
        connect(timer_, &QTimer::timeout, this, [this] { sample(); });
        clock_.start();
        sample();
        timer_->start();
        statusBar()->showMessage(QStringLiteral("Mesures locales uniquement · aucune télémétrie"));
    }

private:
    QTabWidget *tabs_ = nullptr;
    QLabel *updated_ = nullptr;
    QPushButton *refreshButton_ = nullptr;
    QCheckBox *compactView_ = nullptr;
    MetricCard *cpuCard_ = nullptr;
    MetricCard *memoryCard_ = nullptr;
    MetricCard *networkCard_ = nullptr;
    MetricCard *gpuCard_ = nullptr;
    QFrame *resourcePanel_ = nullptr;
    QFrame *networkPanel_ = nullptr;
    QGridLayout *chartsLayout_ = nullptr;
    int chartColumns_ = 2;
    HistoryChart *resourceChart_ = nullptr;
    HistoryChart *networkChart_ = nullptr;
    QLabel *batteryInfo_ = nullptr;
    QLabel *powerInfo_ = nullptr;
    QLabel *thermalInfo_ = nullptr;
    QLabel *graphicsInfo_ = nullptr;
    QLabel *systemInfo_ = nullptr;
    QLabel *networkInfo_ = nullptr;
    QLabel *storageStatus_ = nullptr;
    QTableWidget *storageTable_ = nullptr;
    QTableWidget *processTable_ = nullptr;
    QLineEdit *processFilter_ = nullptr;
    QPushButton *terminateButton_ = nullptr;
    QTimer *timer_ = nullptr;
    QElapsedTimer clock_;
    QElapsedTimer storageClock_;
    SystemSample previous_;
    QMap<qint64, ProcessInfo> currentProcesses_;
    bool havePrevious_ = false;
    bool storagePageViewed_ = false;

    void resizeEvent(QResizeEvent *event) override {
        QMainWindow::resizeEvent(event);
        arrangeCharts();
    }

    void arrangeCharts() {
        if (!chartsLayout_ || !resourcePanel_ || !networkPanel_)
            return;
        const int columns = width() < 900 ? 1 : 2;
        if (columns == chartColumns_)
            return;

        chartsLayout_->removeWidget(resourcePanel_);
        chartsLayout_->removeWidget(networkPanel_);
        chartsLayout_->addWidget(resourcePanel_, 0, 0);
        chartsLayout_->addWidget(networkPanel_, columns == 1 ? 1 : 0, columns == 1 ? 0 : 1);
        chartsLayout_->setColumnStretch(0, 1);
        chartsLayout_->setColumnStretch(1, columns == 1 ? 0 : 1);
        chartsLayout_->invalidate();
        chartColumns_ = columns;
    }

    void buildOverviewPage() {
        QVBoxLayout *pageLayout = nullptr;
        makeScrollPage(tabs_, QStringLiteral("Vue d'ensemble"), &pageLayout);
        auto *intro = new QLabel(QStringLiteral("PERFORMANCES SYSTÈME"));
        intro->setStyleSheet(QStringLiteral("font-size:12px;font-weight:700;letter-spacing:1px;color:#9ca8b8;"));
        pageLayout->addWidget(intro);

        auto *metrics = new QGridLayout;
        metrics->setHorizontalSpacing(12);
        metrics->setVerticalSpacing(12);
        cpuCard_ = new MetricCard(QStringLiteral("PROCESSEUR"), cpuColor);
        memoryCard_ = new MetricCard(QStringLiteral("MÉMOIRE VIVE"), memoryColor);
        gpuCard_ = new MetricCard(QStringLiteral("GRAPHIQUES"), QColor(QStringLiteral("#e9ad52")));
        networkCard_ = new MetricCard(QStringLiteral("RÉSEAU"), networkInColor);
        metrics->addWidget(cpuCard_, 0, 0);
        metrics->addWidget(memoryCard_, 0, 1);
        metrics->addWidget(networkCard_, 1, 0);
        metrics->addWidget(gpuCard_, 1, 1);
        metrics->setColumnStretch(0, 1);
        metrics->setColumnStretch(1, 1);
        pageLayout->addLayout(metrics);

        chartsLayout_ = new QGridLayout;
        chartsLayout_->setHorizontalSpacing(12);
        resourcePanel_ = makeChartPanel(QStringLiteral("Charge des ressources"),
            QStringLiteral("Pourcentage total de la machine · 120 dernières mesures"),
            QStringLiteral("CPU"), cpuColor, QStringLiteral("RAM"), memoryColor,
            HistoryChart::Scale::Percent, &resourceChart_);
        networkPanel_ = makeChartPanel(QStringLiteral("Débit réseau"),
            QStringLiteral("Octets reçus et envoyés · boucle locale exclue"),
            QStringLiteral("Reçu"), networkInColor, QStringLiteral("Envoyé"), networkOutColor,
            HistoryChart::Scale::BytesPerSecond, &networkChart_);
        chartsLayout_->addWidget(resourcePanel_, 0, 0);
        chartsLayout_->addWidget(networkPanel_, 0, 1);
        chartsLayout_->setColumnStretch(0, 1);
        chartsLayout_->setColumnStretch(1, 1);
        pageLayout->addLayout(chartsLayout_);
        pageLayout->addStretch();
    }

    void buildHardwarePage() {
        QVBoxLayout *pageLayout = nullptr;
        makeScrollPage(tabs_, QStringLiteral("Matériel et réseau"), &pageLayout);
        auto *grid = new QGridLayout;
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(12);
        grid->addWidget(makeInfoPanel(QStringLiteral("Batterie"), &batteryInfo_), 0, 0);
        grid->addWidget(makeInfoPanel(QStringLiteral("Alimentation"), &powerInfo_), 0, 1);
        grid->addWidget(makeInfoPanel(QStringLiteral("Températures"), &thermalInfo_), 1, 0);
        grid->addWidget(makeInfoPanel(QStringLiteral("Graphiques"), &graphicsInfo_), 1, 1);
        grid->addWidget(makeInfoPanel(QStringLiteral("Réseau local"), &networkInfo_), 2, 0, 1, 2);
        grid->addWidget(makeInfoPanel(QStringLiteral("Système et mémoire d'échange"), &systemInfo_), 3, 0, 1, 2);
        grid->setColumnStretch(0, 1);
        grid->setColumnStretch(1, 1);
        pageLayout->addLayout(grid);

        auto *storagePanel = new QFrame;
        storagePanel->setObjectName(QStringLiteral("monitorInfoPanel"));
        storagePanel->setStyleSheet(QStringLiteral(
            "QFrame#monitorInfoPanel{background:#151a22;border:1px solid #303a49;border-radius:12px;}"));
        auto *storageLayout = new QVBoxLayout(storagePanel);
        storageLayout->setContentsMargins(18, 16, 18, 17);
        auto *storageHeading = new QLabel(QStringLiteral("Stockage monté"), storagePanel);
        storageHeading->setStyleSheet(QStringLiteral("font-size:15px;font-weight:700;color:#f0f2f6;border:0;background:transparent;"));
        storageLayout->addWidget(storageHeading);
        storageTable_ = new QTableWidget(0, 4, storagePanel);
        storageTable_->setHorizontalHeaderLabels({QStringLiteral("Point de montage"), QStringLiteral("Périphérique"),
                                                   QStringLiteral("Libre pour la session"), QStringLiteral("Capacité")});
        storageTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        storageTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        storageTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        storageTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        storageTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        storageTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        storageTable_->setMinimumHeight(150);
        storageLayout->addWidget(storageTable_);
        storageStatus_ = new QLabel(QStringLiteral("Les volumes seront lus à l'ouverture de cette vue."), storagePanel);
        storageStatus_->setWordWrap(true);
        storageStatus_->setStyleSheet(QStringLiteral("font-size:11px;color:#909bad;border:0;background:transparent;"));
        storageLayout->addWidget(storageStatus_);
        auto *storageNote = new QLabel(QStringLiteral("Lecture seule · aucun montage, démontage ou formatage n'est effectué."), storagePanel);
        storageNote->setStyleSheet(QStringLiteral("font-size:11px;color:#909bad;border:0;background:transparent;"));
        storageLayout->addWidget(storageNote);
        pageLayout->addWidget(storagePanel);
        pageLayout->addStretch();
    }

    void buildProcessesPage() {
        QVBoxLayout *pageLayout = nullptr;
        makeScrollPage(tabs_, QStringLiteral("Processus"), &pageLayout);
        auto *intro = new QLabel(QStringLiteral(
            "Activité des processus visibles pour votre session. Le pourcentage CPU est une part de toute la machine."));
        intro->setWordWrap(true);
        intro->setStyleSheet(QStringLiteral("font-size:12px;color:#aab4c2;"));
        pageLayout->addWidget(intro);
        auto *controls = new QHBoxLayout;
        processFilter_ = new QLineEdit;
        processFilter_->setPlaceholderText(QStringLiteral("Filtrer par nom ou PID"));
        processFilter_->setAccessibleName(QStringLiteral("Filtrer les processus par nom ou identifiant"));
        controls->addWidget(processFilter_, 1);
        terminateButton_ = new QPushButton(QStringLiteral("Demander l'arrêt du processus"));
        terminateButton_->setEnabled(false);
        controls->addWidget(terminateButton_);
        pageLayout->addLayout(controls);

        processTable_ = new QTableWidget(0, 5);
        processTable_->setHorizontalHeaderLabels({QStringLiteral("PID"), QStringLiteral("Processus"),
            QStringLiteral("CPU · % machine"), QStringLiteral("Mémoire"), QStringLiteral("UID")});
        processTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        processTable_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        processTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
        processTable_->setSelectionMode(QAbstractItemView::SingleSelection);
        processTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        processTable_->setAlternatingRowColors(true);
        processTable_->setShowGrid(false);
        processTable_->verticalHeader()->setVisible(false);
        processTable_->setSortingEnabled(true);
        processTable_->setMinimumHeight(390);
        processTable_->setAccessibleName(QStringLiteral("Liste des processus locaux"));
        pageLayout->addWidget(processTable_);
        auto *warning = new QLabel(QStringLiteral(
            "L'arrêt envoie uniquement SIGTERM à un processus de votre compte. Une application peut perdre des modifications non enregistrées."));
        warning->setWordWrap(true);
        warning->setStyleSheet(QStringLiteral("font-size:11px;color:#d0a7ad;"));
        pageLayout->addWidget(warning);
        pageLayout->addStretch();

        connect(processFilter_, &QLineEdit::textChanged, this, [this] { filterProcesses(); });
        connect(processTable_, &QTableWidget::itemSelectionChanged, this, [this] {
            if (terminateButton_) terminateButton_->setEnabled(processTable_->currentRow() >= 0);
        });
        connect(terminateButton_, &QPushButton::clicked, this, [this] { terminateSelected(); });
    }

    QString formatMemory(quint64 total, quint64 available) const {
        if (total == 0) return QStringLiteral("Indisponible");
        const quint64 used = total - qMin(total, available);
        return QStringLiteral("%1 sur %2").arg(formatBytes(used), formatBytes(total));
    }

    int usedPercent(quint64 total, quint64 available) const {
        if (total == 0) return -1;
        const quint64 used = total - qMin(total, available);
        return qBound(0, qRound(100.0 * double(used) / double(total)), 100);
    }

    void updateHardware(const SystemSample &sample) {
        QStringList batteries;
        for (const auto &battery : sample.batteries) {
            QStringList parts;
            parts.append(battery.capacityPercent < 0
                ? QStringLiteral("niveau indisponible")
                : QStringLiteral("%1 %").arg(battery.capacityPercent));
            if (!battery.status.isEmpty()) parts.append(battery.status);
            if (battery.healthPercent >= 0.0)
                parts.append(QStringLiteral("santé estimée %1 %").arg(QLocale().toString(battery.healthPercent, 'f', 0)));
            if (battery.remainingMinutes >= 0)
                parts.append(QStringLiteral("autonomie indicative %1").arg(formatDuration(battery.remainingMinutes)));
            if (battery.cyclesAvailable)
                parts.append(QStringLiteral("%1 cycles exposés par le noyau").arg(battery.cycles));
            batteries.append(battery.name + QStringLiteral(" · ") + parts.join(QStringLiteral(" · ")));
        }
        batteryInfo_->setText(batteries.isEmpty()
            ? QStringLiteral("Aucune batterie détectée par les interfaces Linux de cette machine.")
            : batteries.join(QLatin1Char('\n')));

        QStringList sources;
        for (const auto &source : sample.powerSources) {
            const QString label = source.type == QStringLiteral("Mains") ? QStringLiteral("Secteur")
                : source.type == QStringLiteral("USB_C") ? QStringLiteral("USB-C") : QStringLiteral("USB");
            sources.append(source.name + QStringLiteral(" · ") + label + QStringLiteral(" · ")
                + (source.online ? QStringLiteral("connecté") : QStringLiteral("déconnecté")));
        }
        powerInfo_->setText(sources.isEmpty()
            ? QStringLiteral("État secteur/USB non exposé par le noyau.")
            : sources.join(QLatin1Char('\n')));

        thermalInfo_->setText(sample.temperatures.isEmpty()
            ? QStringLiteral("Aucun capteur thermique accessible. Cela ne signifie pas que la machine n'a pas de capteur.")
            : sample.temperatures.join(QLatin1Char('\n')));
        QStringList graphics;
        for(const auto &device:sample.graphicsDevices) {
            QString line=QStringLiteral("PCI %1 · vendeur %2 · périphérique %3")
                .arg(device.pciAddress,device.vendorId.isEmpty()?QStringLiteral("inconnu"):device.vendorId,
                     device.deviceId.isEmpty()?QStringLiteral("inconnu"):device.deviceId);
            line+=QStringLiteral(" · pilote %1").arg(device.driver.isEmpty()?QStringLiteral("non lié ou inaccessible"):device.driver);
            graphics.append(line);
        }
        if(graphics.isEmpty()) {
            graphicsInfo_->setText(QStringLiteral(
                "Aucun contrôleur PCI de classe graphique exposé. Cela ne prouve pas l'absence de matériel graphique."));
        } else {
            graphicsInfo_->setText(graphics.join(QLatin1Char('\n'))+
                QStringLiteral("\nUtilisation et températures GPU non mesurées."));
        }

        QStringList networkLines;
        for (const auto &link : sample.networkLinks) {
            QString state = link.carrierAvailable
                ? (link.carrier ? QStringLiteral("lien actif") : QStringLiteral("aucun lien"))
                : (link.state.isEmpty() ? QStringLiteral("état indisponible") : link.state);
            if (link.speedMbps > 0) state += QStringLiteral(" · %1 Mbit/s").arg(link.speedMbps);
            networkLines.append(link.name + QStringLiteral(" · ") + state);
        }
        for (auto it = sample.networkAddresses.cbegin(); it != sample.networkAddresses.cend(); ++it)
            networkLines.append(it.key() + QStringLiteral(" · ") + it.value().join(QStringLiteral(", ")));
        const auto config = sample.networkConfiguration;
        networkLines.append(QStringLiteral("Passerelle IPv4 · ") + (config.defaultGateway.isEmpty()
            ? QStringLiteral("non exposée") : config.defaultGateway));
        networkLines.append(QStringLiteral("DNS configurés · ") + (config.dnsServers.isEmpty()
            ? QStringLiteral("non exposés") : config.dnsServers.join(QStringLiteral(", "))));
        networkLines.append(QStringLiteral("Ces informations locales ne testent pas l'accès à Internet."));
        networkInfo_->setText(networkLines.join(QLatin1Char('\n')));

        QStringList systemLines;
        systemLines.append(QStringLiteral("Architecture · ") + QSysInfo::currentCpuArchitecture());
        systemLines.append(QStringLiteral("Noyau · ") + QSysInfo::kernelVersion());
        systemLines.append(QStringLiteral("RAM · ") + formatMemory(sample.memoryTotal, sample.memoryAvailable));
        systemLines.append(QStringLiteral("Swap · ") + formatMemory(sample.swapTotal, sample.swapFree));
        systemInfo_->setText(systemLines.join(QLatin1Char('\n')));

    }

    void refreshStorage() {
        QSet<QString> seenMounts;
        QList<QStorageInfo> volumes;
        for (const auto &volume : QStorageInfo::mountedVolumes()) {
            const QString path = volume.rootPath();
            if (!volume.isValid() || !volume.isReady() || volume.bytesTotal() <= 0 || seenMounts.contains(path)) continue;
            if (path == QStringLiteral("/proc") || path.startsWith(QStringLiteral("/proc/"))
                || path == QStringLiteral("/sys") || path.startsWith(QStringLiteral("/sys/"))
                || path == QStringLiteral("/dev") || path.startsWith(QStringLiteral("/dev/"))
                || path == QStringLiteral("/run") || path.startsWith(QStringLiteral("/run/"))) continue;
            seenMounts.insert(path);
            volumes.append(volume);
        }
        storageTable_->setRowCount(volumes.size());
        storageStatus_->setVisible(volumes.isEmpty());
        if (volumes.isEmpty())
            storageStatus_->setText(QStringLiteral("Aucun volume monté avec une capacité accessible n'a été signalé."));
        int row = 0;
        for (const auto &volume : volumes) {
            const QStringList cells = {volume.rootPath(), volume.device(),
                formatBytes(quint64(qMax<qint64>(0, volume.bytesAvailable()))),
                formatBytes(quint64(qMax<qint64>(0, volume.bytesTotal())))};
            for (int column = 0; column < cells.size(); ++column) {
                auto *item = new QTableWidgetItem(cells[column]);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                storageTable_->setItem(row, column, item);
            }
            ++row;
        }
    }

    void sample() {
        const double seconds = qMax<qint64>(250, clock_.restart()) / 1000.0;
        const bool processViewActive = tabs_->currentIndex() == 2;
        const SystemSample next = readSystem(QStringLiteral("/proc"), QStringLiteral("/sys"), processViewActive);
        const double cpu = cpuPercent(previous_.cpu, next.cpu);
        const int memoryUse = usedPercent(next.memoryTotal, next.memoryAvailable);
        const quint64 memoryUsed = next.memoryTotal - qMin(next.memoryTotal, next.memoryAvailable);

        const int graphicsCount=next.graphicsDevices.size();
        gpuCard_->setMetric(graphicsCount==0 ? QStringLiteral("Non identifié")
                                             : QStringLiteral("%1 contrôleur%2").arg(graphicsCount)
                                                 .arg(graphicsCount==1 ? QString() : QStringLiteral("s")),
            graphicsCount==0
                ? QStringLiteral("Aucun contrôleur PCI graphique exposé ; vérifiez la vue Matériel.")
                : QStringLiteral("Identité disponible · charge et température non mesurées"));

        cpuCard_->setMetric(cpu < 0.0 ? QStringLiteral("Collecte…") : formatPercent(cpu),
            QStringLiteral("Charge totale du processeur · pas un cœur individuel"), cpu < 0.0 ? -1 : qRound(cpu));
        memoryCard_->setMetric(memoryUse < 0 ? QStringLiteral("Indisponible") : formatBytes(memoryUsed),
            memoryUse < 0 ? QStringLiteral("Mesure mémoire non exposée")
                          : QStringLiteral("%1 · %2 % utilisés").arg(formatBytes(next.memoryTotal), QString::number(memoryUse)),
            memoryUse);
        double receiveRate = -1.0;
        double sendRate = -1.0;
        bool hasNonLoopbackInterface = false;
        if (havePrevious_ && !next.network.isEmpty()) {
            quint64 received = 0;
            quint64 sent = 0;
            bool found = false;
            for (auto it = next.network.cbegin(); it != next.network.cend(); ++it) {
                if (it.key() == QStringLiteral("lo")) continue;
                hasNonLoopbackInterface = true;
                if (!previous_.network.contains(it.key())) continue;
                const auto old = previous_.network.value(it.key());
                if (it.value().first >= old.first && it.value().second >= old.second) {
                    received += it.value().first - old.first;
                    sent += it.value().second - old.second;
                    found = true;
                }
            }
            if (found) {
                receiveRate = double(received) / seconds;
                sendRate = double(sent) / seconds;
            }
        } else {
            for (auto it = next.network.cbegin(); it != next.network.cend(); ++it)
                if (it.key() != QStringLiteral("lo")) hasNonLoopbackInterface = true;
        }
        if (!hasNonLoopbackInterface) {
            networkCard_->setMetric(QStringLiteral("Indisponible"),
                QStringLiteral("Aucune interface réseau mesurable n'est exposée."));
        } else {
            networkCard_->setMetric(QStringLiteral("↓ %1").arg(formatRate(receiveRate)),
                QStringLiteral("↑ %1 · hors boucle locale").arg(formatRate(sendRate)));
        }

        resourceChart_->append(cpu, memoryUse < 0 ? -1.0 : double(memoryUse));
        networkChart_->append(receiveRate, sendRate);
        updateHardware(next);
        if (processViewActive) updateProcesses(next);
        if (storagePageViewed_ && storageClock_.elapsed() >= 15000) {
            refreshStorage();
            storageClock_.restart();
        }
        previous_ = next;
        havePrevious_ = true;
        updated_->setText(QStringLiteral("Mesuré à %1").arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
        const QString status=processViewActive
            ? QStringLiteral("%1 processus accessibles · actualisation toutes les 2 secondes · lecture locale").arg(next.processes.size())
            : QStringLiteral("Mesures locales actualisées · scan des processus suspendu hors de leur vue");
        statusBar()->showMessage(status);
    }

    void updateProcesses(const SystemSample &next) {
        qint64 selectedPid = -1;
        if (processTable_->currentRow() >= 0 && processTable_->item(processTable_->currentRow(), 0))
            selectedPid = processTable_->item(processTable_->currentRow(), 0)->data(Qt::UserRole).toLongLong();
        const int sortColumn = processTable_->horizontalHeader()->sortIndicatorSection();
        const Qt::SortOrder sortOrder = processTable_->horizontalHeader()->sortIndicatorOrder();

        QMap<qint64, ProcessInfo> old;
        for (const auto &process : previous_.processes) old.insert(process.pid, process);
        currentProcesses_.clear();
        processTable_->setSortingEnabled(false);
        processTable_->clearContents();
        processTable_->setRowCount(next.processes.size());
        int row = 0;
        for (const auto &process : next.processes) {
            currentProcesses_.insert(process.pid, process);
            double cpuUse = -1.0;
            if (havePrevious_ && old.contains(process.pid) && old[process.pid].start == process.start
                && process.ticks >= old[process.pid].ticks && next.cpu.total > previous_.cpu.total) {
                cpuUse = 100.0 * double(process.ticks - old[process.pid].ticks)
                    / double(next.cpu.total - previous_.cpu.total);
            }
            const QString cpuText = cpuUse < 0.0 ? QStringLiteral("—") : QLocale().toString(cpuUse, 'f', 1);
            const QStringList texts = {QString::number(process.pid), process.name, cpuText,
                                       formatBytes(process.rss), QString::number(process.uid)};
            for (int column = 0; column < texts.size(); ++column) {
                QTableWidgetItem *item = column == 1
                    ? static_cast<QTableWidgetItem *>(new QTableWidgetItem)
                    : static_cast<QTableWidgetItem *>(new NumericItem);
                item->setText(texts[column]);
                if (auto *numeric = dynamic_cast<NumericItem *>(item)) {
                    if (column == 0) numeric->number = double(process.pid);
                    else if (column == 2) numeric->number = cpuUse;
                    else if (column == 3) numeric->number = double(process.rss);
                    else if (column == 4) numeric->number = double(process.uid);
                }
                item->setData(Qt::UserRole, process.pid);
                processTable_->setItem(row, column, item);
            }
            ++row;
        }
        processTable_->setSortingEnabled(true);
        processTable_->sortItems(sortColumn, sortOrder);
        if (selectedPid > 0) {
            for (int index = 0; index < processTable_->rowCount(); ++index) {
                if (processTable_->item(index, 0)->data(Qt::UserRole).toLongLong() == selectedPid) {
                    processTable_->selectRow(index);
                    break;
                }
            }
        }
        filterProcesses();
    }

    void filterProcesses() {
        if (!processFilter_ || !processTable_) return;
        const QString query = processFilter_->text().trimmed();
        for (int row = 0; row < processTable_->rowCount(); ++row) {
            const auto *pid = processTable_->item(row, 0);
            const auto *name = processTable_->item(row, 1);
            const bool matches = query.isEmpty() || (pid && pid->text().contains(query, Qt::CaseInsensitive))
                || (name && name->text().contains(query, Qt::CaseInsensitive));
            processTable_->setRowHidden(row, !matches);
        }
    }

    void terminateSelected() {
        if (!processTable_ || processTable_->currentRow() < 0) return;
        const auto *item = processTable_->item(processTable_->currentRow(), 0);
        if (!item) return;
        const qint64 pid = item->data(Qt::UserRole).toLongLong();
        if (!currentProcesses_.contains(pid)) return;
        const ProcessInfo expected = currentProcesses_.value(pid);

        QMessageBox question(QMessageBox::Warning, QStringLiteral("Arrêter un processus"), {},
                             QMessageBox::Yes | QMessageBox::Cancel, this);
        question.setTextFormat(Qt::PlainText);
        question.setDefaultButton(QMessageBox::Cancel);
        question.setText(QStringLiteral("Envoyer SIGTERM à « %1 » (PID %2) ?\n\nLes documents non enregistrés de cette application peuvent être perdus.")
            .arg(expected.name).arg(pid));
        if (question.exec() != QMessageBox::Yes) return;

        QString error;
        if (!terminateProcess(expected, &error)) {
            Kyronext::showError(this, error);
        } else {
            statusBar()->showMessage(QStringLiteral("Demande SIGTERM envoyée ; l'application peut mettre du temps à se fermer."), 6000);
        }
    }
};

#ifndef KYRONEXT_MONITOR_NO_MAIN
int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext Monitor"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    MonitorWindow window;
    window.show();
    return app.exec();
}
#endif
