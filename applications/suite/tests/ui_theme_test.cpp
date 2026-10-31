#include <kyronext/ui.h>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QWidget>
#include <QtTest>
#include <memory>

class KyronextThemeTests final : public QObject {
    Q_OBJECT
    QTemporaryDir settingsRoot;
    bool settingsIsolated = false;
private slots:
    void initTestCase() {
        QVERIFY(settingsRoot.isValid());
        const QString systemConfig = QDir(settingsRoot.path()).filePath(QStringLiteral("system"));
        QVERIFY(QDir().mkpath(systemConfig));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsRoot.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, systemConfig);
        settingsIsolated = true;
    }

    void cleanup() {
        if (!settingsIsolated) return;
        QSettings user;
        user.clear();
        user.sync();
        QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
        shared.clear();
        shared.sync();
    }

    void commonControlsHaveVisibleStates() {
        QApplication &app = *qApp;
        Kyronext::applyTheme(app, Kyronext::Palette::Red);
        const auto redStyle = app.styleSheet();
        QVERIFY(redStyle.contains("QPushButton:focus"));
        QVERIFY(redStyle.contains("QCheckBox::indicator:checked"));
        QVERIFY(redStyle.contains("QProgressBar::chunk"));
        QVERIFY(redStyle.contains("QScrollBar::handle:vertical:hover"));
        QVERIFY(redStyle.contains("selection-background-color:#ef3c60"));

        Kyronext::applyTheme(app, Kyronext::Palette::Amber);
        const auto amberStyle = app.styleSheet();
        QVERIFY(amberStyle.contains("selection-background-color:#e9ad52"));
        QVERIFY(amberStyle.contains("QPushButton:focus"));

        Kyronext::applyTheme(app, Kyronext::Palette::HighContrast);
        QCOMPARE(app.palette().color(QPalette::Window), QColor("#000000"));
        QCOMPARE(app.palette().color(QPalette::WindowText), QColor("#ffffff"));
        QVERIFY(app.styleSheet().contains("selection-background-color:#ff625f"));
    }
    void densityPreferencePersistsAndKeepsTargetsAccessible() {
        QSettings settings;
        QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
        settings.remove(QStringLiteral("appearance/density"));
        shared.setValue(QStringLiteral("appearance/density"), QStringLiteral("compact"));
        shared.sync();
        QVERIFY(Kyronext::densityFromSettings() == Kyronext::Density::Compact);

        Kyronext::setPreferredDensity(Kyronext::Density::Compact);
        QVERIFY(Kyronext::densityFromSettings() == Kyronext::Density::Compact);
        Kyronext::applyTheme(*qApp, Kyronext::Palette::Red);
        const auto compactStyle = qApp->styleSheet();
        QVERIFY(compactStyle.contains("QPushButton{min-height:36px"));
        QVERIFY(compactStyle.contains("QToolButton,QComboBox{min-height:36px"));
        QVERIFY(compactStyle.contains("QMenu::item{min-height:32px"));

        Kyronext::setPreferredDensity(Kyronext::Density::Comfortable);
        QVERIFY(Kyronext::densityFromSettings() == Kyronext::Density::Comfortable);
        Kyronext::applyTheme(*qApp, Kyronext::Palette::Red);
        QVERIFY(qApp->styleSheet().contains("QPushButton{min-height:44px"));
        QVERIFY(qApp->styleSheet().contains("QToolButton,QComboBox{min-height:44px"));
    }
    void emptyStateExplainsTheNextUserAction() {
        std::unique_ptr<QWidget> state(Kyronext::emptyState(
            QStringLiteral("Playlist vide"), QStringLiteral("Ajoutez des fichiers audio.")));
        QVERIFY(state);
        const auto *heading = state->findChild<QLabel *>(QStringLiteral("kyronextEmptyStateTitle"));
        const auto *description = state->findChild<QLabel *>(QStringLiteral("kyronextEmptyStateDescription"));
        QVERIFY(heading);
        QVERIFY(description);
        QCOMPARE(heading->text(), QStringLiteral("Playlist vide"));
        QCOMPARE(description->accessibleName(), QStringLiteral("Playlist vide. Ajoutez des fichiers audio."));
    }
    void reducedMotionPreferenceUsesSharedDefaultAndAllowsUserOverride() {
        QSettings user;
        QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
        user.remove(QStringLiteral("appearance/reducedMotion"));
        shared.setValue(QStringLiteral("appearance/reducedMotion"), true);
        shared.sync();
        QVERIFY(Kyronext::reducedMotionFromSettings());
        Kyronext::setReducedMotionEnabled(false);
        QVERIFY(!Kyronext::reducedMotionFromSettings());
    }
    void lightModeForcesCompactLowMotionButPreservesManualChoices() {
        QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
        shared.setValue(QStringLiteral("appearance/lightMode"), true);
        shared.setValue(QStringLiteral("appearance/density"), QStringLiteral("comfortable"));
        shared.setValue(QStringLiteral("appearance/reducedMotion"), false);
        shared.sync();

        QVERIFY(Kyronext::lightModeFromSettings());
        QVERIFY(Kyronext::densityFromSettings() == Kyronext::Density::Compact);
        QVERIFY(Kyronext::reducedMotionFromSettings());

        Kyronext::setPreferredDensity(Kyronext::Density::Comfortable);
        Kyronext::setReducedMotionEnabled(false);
        QVERIFY(Kyronext::densityFromSettings() == Kyronext::Density::Compact);
        QVERIFY(Kyronext::reducedMotionFromSettings());

        Kyronext::setLightModeEnabled(false);
        QSettings user;
        user.remove(QStringLiteral("appearance/lightMode"));
        shared.setValue(QStringLiteral("appearance/lightMode"), false);
        user.sync();
        shared.sync();
        QVERIFY(!Kyronext::lightModeFromSettings());
        QVERIFY(Kyronext::densityFromSettings() == Kyronext::Density::Comfortable);
        QVERIFY(!Kyronext::reducedMotionFromSettings());
    }
    void forcedReducedMotionControlShowsEffectiveStateAndRestoresManualChoice() {
        QCheckBox control;
        Kyronext::setManualReducedMotionChoice(&control, false);

        Kyronext::displayReducedMotionChoice(&control, true);
        QVERIFY(control.isChecked());
        QVERIFY(!control.isEnabled());
        QVERIFY(!Kyronext::manualReducedMotionChoice(&control));

        Kyronext::displayReducedMotionChoice(&control, false);
        QVERIFY(control.isEnabled());
        QVERIFY(!control.isChecked());
        QVERIFY(!Kyronext::manualReducedMotionChoice(&control));

        Kyronext::setManualReducedMotionChoice(&control, true);
        Kyronext::displayReducedMotionChoice(&control, true);
        QVERIFY(control.isChecked());
        QVERIFY(!control.isEnabled());
        QVERIFY(Kyronext::manualReducedMotionChoice(&control));
        Kyronext::displayReducedMotionChoice(&control, false);
        QVERIFY(control.isEnabled());
        QVERIFY(control.isChecked());
        QVERIFY(Kyronext::manualReducedMotionChoice(&control));
    }
};

QTEST_MAIN(KyronextThemeTests)
#include "ui_theme_test.moc"
