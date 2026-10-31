#include <kyronext/ui.h>
#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDir>
#include <QFrame>
#include <QFile>
#include <QMainWindow>
#include <QMenuBar>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QSettings>
#include <QStandardPaths>
#include <QTranslator>
#include <QLocale>
#include <QStyle>
#include <QSysInfo>
#include <QAction>
#include <QProcess>
#include <QKeySequence>
#include <QMutex>
#include <QMutexLocker>
#include <QVBoxLayout>
#include <cstdio>
#include <cstdlib>

namespace {
QMutex logMutex;
QString logPath;
QtMessageHandler previousHandler = nullptr;
bool handlerInstalled = false;
constexpr auto manualMotionChoiceProperty = "kyronextManualReducedMotion";
void writeLog(QtMsgType type, const QMessageLogContext &context, const QString &message) {
    QMutexLocker lock(&logMutex);
    if (!logPath.isEmpty()) {
        QFile output(logPath);
        if (output.size() > 1024 * 1024) {
            QFile::remove(logPath + ".previous");
            output.rename(logPath + ".previous");
            output.setFileName(logPath);
        }
        if (output.open(QIODevice::WriteOnly | QIODevice::Append)) {
            output.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
            output.write((QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + " " +
                          QString::number(type) + " " + message + "\n").toUtf8());
        }
    }
    if (previousHandler) previousHandler(type, context, message);
    else std::fprintf(stderr, "%s\n", message.toLocal8Bit().constData());
    if (type == QtFatalMsg) std::abort();
}
QIcon logo() {
    QPixmap image(64, 64); image.fill(Qt::transparent);
    QPainter painter(&image); painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#ef3c60"), 7, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(18, 12, 18, 52); painter.drawLine(19, 33, 48, 12);
    painter.drawLine(19, 33, 49, 52);
    return QIcon(image);
}
}
namespace Kyronext {
void configureApplication(const QString &appName, const QString &version) {
    QCoreApplication::setOrganizationName("Kyronext Systems");
    QCoreApplication::setOrganizationDomain("kyronext.eu");
    QCoreApplication::setApplicationName(appName);
    QCoreApplication::setApplicationVersion(version);
    QApplication::setWindowIcon(logo());
    QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
    const auto size = shared.value(QStringLiteral("appearance/fontSize"), QApplication::font().pointSize()).toInt();
    if (size >= 8 && size <= 24) {
        auto font = QApplication::font(); font.setPointSize(size); QApplication::setFont(font);
    }
    auto directory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (QDir().mkpath(directory)) logPath = directory + "/application.log";
    if (!handlerInstalled && QSettings().value("logging/enabled", true).toBool()) {
        previousHandler = qInstallMessageHandler(writeLog);
        handlerInstalled = true;
    }
    auto *translation = new QTranslator(qApp);
    const auto translations = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                      "translations", QStandardPaths::LocateDirectory);
    if (!translations.isEmpty() && translation->load(QLocale(), "kyronext", "_", translations))
        qApp->installTranslator(translation);
}
Palette paletteFromSettings() {
    QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
    const auto value = QSettings().value("appearance/palette", shared.value("appearance/palette", "red")).toString();
    if (value == "amber") return Palette::Amber;
    if (value == "high-contrast") return Palette::HighContrast;
    return Palette::Red;
}
void setPreferredPalette(Palette palette) {
    const auto value = palette == Palette::Amber ? "amber" :
        palette == Palette::HighContrast ? "high-contrast" : "red";
    QSettings().setValue("appearance/palette", value);
}
bool lightModeFromSettings() {
    QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
    return QSettings().value(QStringLiteral("appearance/lightMode"),
        shared.value(QStringLiteral("appearance/lightMode"), false)).toBool();
}
void setLightModeEnabled(bool enabled) {
    QSettings().setValue(QStringLiteral("appearance/lightMode"), enabled);
}
Density densityFromSettings() {
    if (lightModeFromSettings()) return Density::Compact;
    QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
    const auto value = QSettings().value("appearance/density", shared.value("appearance/density", "comfortable")).toString();
    return value == "compact" ? Density::Compact : Density::Comfortable;
}
void setPreferredDensity(Density density) {
    QSettings().setValue("appearance/density", density == Density::Compact ? "compact" : "comfortable");
}
bool reducedMotionFromSettings() {
    if (lightModeFromSettings()) return true;
    QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
    return QSettings().value(QStringLiteral("appearance/reducedMotion"),
        shared.value(QStringLiteral("appearance/reducedMotion"), false)).toBool();
}
void setReducedMotionEnabled(bool enabled) {
    QSettings().setValue(QStringLiteral("appearance/reducedMotion"), enabled);
}
bool manualReducedMotionChoice(const QCheckBox *control) {
    if (!control) return false;
    const auto choice = control->property(manualMotionChoiceProperty);
    return choice.isValid() ? choice.toBool() : control->isChecked();
}
void setManualReducedMotionChoice(QCheckBox *control, bool enabled) {
    if (control) control->setProperty(manualMotionChoiceProperty, enabled);
}
void displayReducedMotionChoice(QCheckBox *control, bool forced) {
    if (!control) return;
    if (forced) {
        control->setChecked(true);
        control->setEnabled(false);
    } else {
        control->setEnabled(true);
        control->setChecked(manualReducedMotionChoice(control));
    }
}
void applyTheme(QApplication &app, Palette palette) {
    const bool compact = densityFromSettings() == Density::Compact;
    const int controlHeight = compact ? 36 : 44;
    const int verticalPadding = compact ? 5 : 8;
    const int horizontalPadding = compact ? 10 : 14;
    const int menuItemHeight = compact ? 32 : 40;
    const bool highContrast = palette == Palette::HighContrast;
    const QColor accent(palette == Palette::Amber ? "#e9ad52" :
                        highContrast ? "#ff625f" : "#ef3c60");
    QPalette colors;
    colors.setColor(QPalette::Window, QColor(highContrast ? "#000000" : "#101318"));
    colors.setColor(QPalette::WindowText, QColor(highContrast ? "#ffffff" : "#f0f2f6"));
    colors.setColor(QPalette::Base, QColor(highContrast ? "#000000" : "#090c10"));
    colors.setColor(QPalette::AlternateBase, QColor(highContrast ? "#171717" : "#1b2028"));
    colors.setColor(QPalette::Text, QColor(highContrast ? "#ffffff" : "#f0f2f6"));
    colors.setColor(QPalette::Button, QColor(highContrast ? "#171717" : "#202630"));
    colors.setColor(QPalette::ButtonText, QColor(highContrast ? "#ffffff" : "#f0f2f6"));
    colors.setColor(QPalette::Highlight, accent);
    colors.setColor(QPalette::HighlightedText, QColor("#090c10"));
    colors.setColor(QPalette::ToolTipBase, QColor("#202630"));
    colors.setColor(QPalette::ToolTipText, QColor("#f0f2f6"));
    colors.setColor(QPalette::Disabled, QPalette::Text, QColor(highContrast ? "#c0c0c0" : "#929aa7"));
    colors.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(highContrast ? "#c0c0c0" : "#929aa7"));
    app.setStyle("Fusion"); app.setPalette(colors);
    // Consistent, low-motion styling for shared Kyronext Widgets applications.
    // Keep controls keyboard-visible and readable at compact laptop resolutions.
    app.setStyleSheet(QString(
        "QPushButton{min-height:%2px;border:1px solid #424956;border-radius:7px;padding:%3px %4px;background:#20242d;color:#f4f5f7;}"
        "QPushButton:hover{border-color:%1;background:#2a2028;} QPushButton:pressed{background:#35222c;}"
        "QPushButton:disabled{color:#a5abb5;background:#171b22;border-color:#303743;}"
        "QPushButton:focus,QToolButton:focus,QCheckBox:focus,QRadioButton:focus,QSlider:focus{border:2px solid %1;}"
        "QLineEdit,QSpinBox,QDoubleSpinBox,QListWidget,QTreeWidget,QTableWidget{min-height:%2px;background:#0d1016;border:1px solid #343b48;border-radius:6px;padding:%3px;selection-background-color:%1;selection-color:#090c10;}"
        "QLineEdit:focus,QComboBox:focus{border:2px solid %1;}"
        "QMenuBar{padding:4px;} QMenuBar::item{min-height:%5px;padding:6px 9px;border-radius:4px;}"
        "QMenuBar::item:selected,QMenu::item:selected{background:%1;color:#090c10;}"
        "QMenu{background:#141922;color:#eef2f7;border:1px solid #394252;padding:5px;} QMenu::item{min-height:%5px;padding:7px 24px;}"
        "QStatusBar{padding:4px;color:#c1c8d2;} QToolBar{spacing:6px;padding:5px;border-bottom:1px solid #383e48;}"
        "QToolButton,QComboBox{min-height:%2px;padding:%3px %4px;border:1px solid #394252;border-radius:5px;background:#1b222d;color:#f0f2f6;}"
        "QComboBox QAbstractItemView{background:#131923;color:#eef2f7;selection-background-color:%1;}"
        "QCheckBox,QRadioButton{min-height:%2px;spacing:8px;color:#edf0f5;} QCheckBox::indicator,QRadioButton::indicator{width:16px;height:16px;border:1px solid #687386;background:#111720;}"
        "QCheckBox::indicator{border-radius:4px;} QRadioButton::indicator{border-radius:9px;}"
        "QCheckBox::indicator:checked,QRadioButton::indicator:checked{background:%1;border:2px solid %1;}"
        "QSlider::groove:horizontal{height:4px;background:#303b4b;border-radius:2px;}"
        "QSlider::sub-page:horizontal{background:%1;border-radius:2px;} QSlider::handle:horizontal{background:%1;width:12px;margin:-5px 0;border-radius:4px;}"
        "QSlider::groove:vertical{width:4px;background:#303b4b;border-radius:2px;} QSlider::handle:vertical{background:%1;height:10px;margin:0 -7px;border-radius:3px;}"
        "QTabBar::tab{background:#171e28;color:#c6cfdb;padding:9px 16px;border-bottom:2px solid #303949;}"
        "QTabBar::tab:selected{background:#222b39;color:#ffffff;border-bottom:2px solid %1;}"
        "QHeaderView::section{background:#1c2430;color:#dce4ef;border:0;border-bottom:1px solid #3a4556;padding:7px;}"
        "QGroupBox{border:1px solid #344052;border-radius:8px;margin-top:14px;padding:14px;} QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 6px;color:%1;}"
        "QProgressBar{min-height:12px;border:1px solid #394252;border-radius:5px;background:#0d1118;color:#f0f2f6;text-align:center;}"
        "QProgressBar::chunk{background:%1;border-radius:4px;}"
        "QScrollBar:vertical{width:10px;background:#111720;margin:2px;border-radius:5px;} QScrollBar::handle:vertical{min-height:24px;background:#485365;border-radius:5px;}"
        "QScrollBar::handle:vertical:hover{background:%1;} QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
        "QToolTip{background:#202630;color:#f4f5f7;border:1px solid #687386;padding:5px;}"
    )
        .arg(accent.name())
        .arg(controlHeight)
        .arg(verticalPadding)
        .arg(horizontalPadding)
        .arg(menuItemHeight));
}
void addAboutMenu(QMainWindow *window, const QString &appName,
                  const QString &description, const QString &dependencies) {
    auto *menu = window->menuBar()->addMenu(QObject::tr("Aid&e"));
    auto *help = menu->addAction(QObject::tr("Documentation locale"));
    help->setShortcut(QKeySequence::HelpContents);
    QObject::connect(help, &QAction::triggered, window, [window] {
        if (!QProcess::startDetached(QStringLiteral("kyronext-help"), {}))
            Kyronext::showError(window, QObject::tr("Le centre d’aide n’est pas installé ou ne peut pas démarrer."));
    });
    auto *action = menu->addAction(QObject::tr("À propos de %1").arg(appName));
    action->setMenuRole(QAction::AboutRole);
    QObject::connect(action, &QAction::triggered, window, [=] {
        QMessageBox dialog(window);
        dialog.setWindowTitle(QObject::tr("À propos de %1").arg(appName));
        dialog.setIconPixmap(logo().pixmap(64, 64));
        dialog.setTextFormat(Qt::PlainText);
        dialog.setText(appName + "\n" + QCoreApplication::applicationVersion() +
            "\nKyronext Systems — By Manix\n\n" + description);
        dialog.setInformativeText(QObject::tr(
            "Créations Kyronext : tous droits réservés, licence publique à définir.\n"
            "Qt 6 : LGPLv3/GPLv3 ou licence commerciale selon les composants.\n"
            "Bibliothèques : %1\n\nQt %2 — %3\nhttps://kyronext.eu/")
            .arg(dependencies, QString::fromLatin1(qVersion()), QSysInfo::prettyProductName()));
        dialog.setStandardButtons(QMessageBox::Ok); dialog.exec();
    });
}
void showError(QWidget *parent, const QString &message) {
    qWarning("Operation failed in Kyronext application");
    QMessageBox::warning(parent, QObject::tr("Kyronext — erreur"), message);
}
void notify(QWidget *parent, const QString &message) {
    QMessageBox::information(parent, QObject::tr("Kyronext"), message);
}
QWidget *emptyState(const QString &title, const QString &description, QWidget *parent) {
    auto *panel = new QFrame(parent);
    panel->setObjectName(QStringLiteral("kyronextEmptyState"));
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setStyleSheet(QStringLiteral(
        "QFrame#kyronextEmptyState{background:#121720;border:1px solid #394252;border-radius:12px;}"));
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(24, 22, 24, 22);
    layout->setSpacing(8);
    auto *mark = new QLabel(QStringLiteral("K"), panel);
    mark->setObjectName(QStringLiteral("kyronextEmptyStateMark"));
    mark->setAlignment(Qt::AlignCenter);
    mark->setStyleSheet(QStringLiteral("font-size:28px;font-weight:800;color:%1;border:0;background:transparent;")
        .arg(QApplication::palette().color(QPalette::Highlight).name()));
    layout->addWidget(mark);
    auto *heading = new QLabel(title, panel);
    heading->setObjectName(QStringLiteral("kyronextEmptyStateTitle"));
    heading->setAlignment(Qt::AlignCenter);
    heading->setWordWrap(true);
    heading->setStyleSheet(QStringLiteral("font-size:17px;font-weight:700;color:#f0f2f6;border:0;background:transparent;"));
    layout->addWidget(heading);
    auto *details = new QLabel(description, panel);
    details->setObjectName(QStringLiteral("kyronextEmptyStateDescription"));
    details->setAccessibleName(title + QStringLiteral(". ") + description);
    details->setAlignment(Qt::AlignCenter);
    details->setWordWrap(true);
    details->setStyleSheet(QStringLiteral("font-size:14px;color:#c1c8d2;border:0;background:transparent;"));
    layout->addWidget(details);
    return panel;
}
}
