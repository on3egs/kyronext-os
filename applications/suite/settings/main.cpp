#include <kyronext/ui.h>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QVBoxLayout>
#include <cmath>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext Settings"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    QMainWindow window;
    window.setWindowTitle(QStringLiteral("Kyronext Settings"));
    window.resize(720, 540);
    auto *body = new QWidget;
    auto *layout = new QVBoxLayout(body);
    layout->setContentsMargins(24, 24, 24, 24);
    auto *heading = new QLabel(QStringLiteral("Votre interface Kyronext"));
    heading->setStyleSheet(QStringLiteral("font-size:24px;font-weight:700;"));
    layout->addWidget(heading);
    auto *description = new QLabel(QStringLiteral("Choisissez les valeurs par défaut des applications. Les préférences propres à chaque application restent prioritaires. Les nouvelles valeurs sont utilisées à leur prochaine ouverture."));
    description->setWordWrap(true);
    layout->addWidget(description);
    QSettings shared(QStringLiteral("Kyronext Systems"), QStringLiteral("Appearance"));
    auto *form = new QFormLayout;
    auto *palette = new QComboBox;
    palette->addItem(QStringLiteral("Graphite et rouge"), QStringLiteral("red"));
    palette->addItem(QStringLiteral("Graphite et ambre"), QStringLiteral("amber"));
    palette->addItem(QStringLiteral("Contraste renforcé"), QStringLiteral("high-contrast"));
    const auto savedPalette = shared.value(QStringLiteral("appearance/palette"), QStringLiteral("red")).toString();
    palette->setCurrentIndex(qMax(0, palette->findData(savedPalette)));
    palette->setAccessibleName(QStringLiteral("Palette par défaut des applications"));
    auto *density = new QComboBox;
    density->addItem(QStringLiteral("Confortable (cibles 44 px)"), QStringLiteral("comfortable"));
    density->addItem(QStringLiteral("Compacte (cibles 36 px)"), QStringLiteral("compact"));
    const auto savedDensity = shared.value(QStringLiteral("appearance/density"), QStringLiteral("comfortable")).toString();
    density->setCurrentIndex(qMax(0, density->findData(savedDensity)));
    density->setAccessibleName(QStringLiteral("Densité des commandes Kyronext"));
    auto *reducedMotion = new QCheckBox(QStringLiteral("Réduire les effets visuels Kyronext (AMP et veille)"));
    const bool savedManualMotion = QSettings().value(QStringLiteral("appearance/reducedMotion"),
        shared.value(QStringLiteral("appearance/reducedMotion"), false)).toBool();
    reducedMotion->setChecked(savedManualMotion);
    reducedMotion->setAccessibleName(QStringLiteral("Réduire les animations et visualisations Kyronext AMP et veille"));
    Kyronext::setManualReducedMotionChoice(reducedMotion, reducedMotion->isChecked());
    auto *lightMode = new QCheckBox(QStringLiteral("Mode léger Kyronext"));
    lightMode->setChecked(Kyronext::lightModeFromSettings());
    lightMode->setAccessibleName(QStringLiteral("Activer le profil graphique léger Kyronext"));
    auto *profileExplanation = new QLabel(QStringLiteral(
        "Pour les ordinateurs moins puissants : utilise des commandes compactes et réduit les animations Kyronext, notamment dans AMP. "
        "La résolution et les applications installées ne changent pas. Vos choix manuels sont conservés."));
    profileExplanation->setWordWrap(true);
    auto updateProfileControls = [density, reducedMotion, lightMode] {
        const bool light = lightMode->isChecked();
        density->setEnabled(!light);
        Kyronext::displayReducedMotionChoice(reducedMotion, light);
        const QString densityExplanation = light
            ? QStringLiteral("Forcé par le mode léger. Désactivez ce mode pour retrouver les réglages manuels.")
            : QStringLiteral("Ce réglage peut être utilisé seul ou avec le mode léger.");
        density->setToolTip(densityExplanation);
        reducedMotion->setToolTip(light
            ? QStringLiteral("Réduction imposée par le mode léger. Votre préférence manuelle est mémorisée et sera restaurée à la désactivation du mode.")
            : QStringLiteral("Réduit les visualisations Kyronext et l'animation de veille."));
    };
    QObject::connect(reducedMotion, &QCheckBox::toggled, &window,
                     [reducedMotion, lightMode](bool checked) {
        if (!lightMode->isChecked()) Kyronext::setManualReducedMotionChoice(reducedMotion, checked);
    });
    QObject::connect(lightMode, &QCheckBox::toggled, &window,
                     [updateProfileControls](bool) { updateProfileControls(); });
    updateProfileControls();
    auto *font = new QSpinBox;
    font->setRange(8, 24);
    font->setSuffix(QStringLiteral(" pt"));
    font->setValue(shared.value(QStringLiteral("appearance/fontSize"), app.font().pointSize()).toInt());
    font->setAccessibleName(QStringLiteral("Taille du texte des applications"));
    form->addRow(QStringLiteral("Palette"), palette);
    form->addRow(QStringLiteral("Profil graphique"), lightMode);
    form->addRow(QString(), profileExplanation);
    form->addRow(QStringLiteral("Espacement des commandes"), density);
    form->addRow(QStringLiteral("Effets visuels"), reducedMotion);
    form->addRow(QStringLiteral("Taille des caractères"), font);
    layout->addLayout(form);
    auto apply = [&] {
        const bool manualMotion = Kyronext::manualReducedMotionChoice(reducedMotion);
        shared.setValue(QStringLiteral("appearance/palette"), palette->currentData());
        shared.setValue(QStringLiteral("appearance/lightMode"), lightMode->isChecked());
        shared.setValue(QStringLiteral("appearance/density"), density->currentData());
        shared.setValue(QStringLiteral("appearance/reducedMotion"), manualMotion);
        shared.setValue(QStringLiteral("appearance/fontSize"), font->value());
        shared.sync();
        if (shared.status() != QSettings::NoError) {
            Kyronext::showError(&window, QStringLiteral("Les préférences n’ont pas pu être enregistrées."));
            return;
        }
        const auto key = palette->currentData().toString();
        const auto selectedPalette = key == QStringLiteral("amber") ? Kyronext::Palette::Amber :
            key == QStringLiteral("high-contrast") ? Kyronext::Palette::HighContrast : Kyronext::Palette::Red;
        Kyronext::setPreferredPalette(selectedPalette);
        Kyronext::setLightModeEnabled(lightMode->isChecked());
        const auto selectedDensity = density->currentData().toString() == QStringLiteral("compact")
            ? Kyronext::Density::Compact : Kyronext::Density::Comfortable;
        Kyronext::setPreferredDensity(selectedDensity);
        Kyronext::setReducedMotionEnabled(manualMotion);
        Kyronext::applyTheme(app, selectedPalette);
        auto selectedFont = app.font(); selectedFont.setPointSize(font->value()); app.setFont(selectedFont);
        window.statusBar()->showMessage(QStringLiteral("Préférences enregistrées. Les applications déjà ouvertes conservent leurs réglages."));
    };
    auto *save = new QPushButton(QStringLiteral("Appliquer aux applications"));
    QObject::connect(save, &QPushButton::clicked, &window, apply);
    layout->addWidget(save);
    auto *backup = new QPushButton(QStringLiteral("Exporter mes préférences Kyronext…"));
    QObject::connect(backup, &QPushButton::clicked, &window, [&] {
        const auto path = QFileDialog::getSaveFileName(&window, QStringLiteral("Exporter les préférences"), QString(), QStringLiteral("JSON (*.json)"));
        if (path.isEmpty()) return;
        const QJsonObject object{{"schema", 4}, {"palette", palette->currentData().toString()},
                                 {"lightMode", lightMode->isChecked()},
                                 {"density", density->currentData().toString()},
                                 {"reducedMotion", Kyronext::manualReducedMotionChoice(reducedMotion)},
                                 {"fontSize", font->value()}};
        QSaveFile output(path);
        if (!output.open(QIODevice::WriteOnly) || output.write(QJsonDocument(object).toJson()) < 0 || !output.commit())
            Kyronext::showError(&window, QStringLiteral("Export impossible ; aucune sauvegarde n’est confirmée."));
    });
    layout->addWidget(backup);
    auto *restore = new QPushButton(QStringLiteral("Restaurer mes préférences Kyronext…"));
    QObject::connect(restore, &QPushButton::clicked, &window, [&] {
        const auto path = QFileDialog::getOpenFileName(&window, QStringLiteral("Restaurer les préférences"), QString(), QStringLiteral("JSON (*.json)"));
        if (path.isEmpty()) return;
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly) || file.size() > 65536) { Kyronext::showError(&window, QStringLiteral("Fichier illisible ou trop volumineux.")); return; }
        const auto document = QJsonDocument::fromJson(file.readAll());
        const auto object = document.object();
        const auto color = object.value("palette").toString();
        const int schema = object.value("schema").toInt(-1);
        const auto spacing = schema == 1 ? QStringLiteral("comfortable") : object.value("density").toString();
        const QJsonValue reducedMotionValue = (schema == 3 || schema == 4)
            ? object.value("reducedMotion") : QJsonValue(false);
        const QJsonValue lightModeValue = schema == 4 ? object.value("lightMode") : QJsonValue(false);
        const auto points = object.value("fontSize").toDouble(-1);
        if (!document.isObject() || (schema != 1 && schema != 2 && schema != 3 && schema != 4) ||
            (color != QStringLiteral("red") && color != QStringLiteral("amber") && color != QStringLiteral("high-contrast")) ||
            (spacing != QStringLiteral("compact") && spacing != QStringLiteral("comfortable")) ||
            ((schema == 3 || schema == 4) && !reducedMotionValue.isBool()) ||
            (schema == 4 && !lightModeValue.isBool()) ||
            !std::isfinite(points) || points < 8 || points > 24 || std::floor(points) != points) {
            Kyronext::showError(&window, QStringLiteral("Ce fichier n’est pas une sauvegarde de préférences reconnue.")); return;
        }
        if (QMessageBox::question(&window, QStringLiteral("Restaurer"), QStringLiteral("Remplacer la palette, le profil graphique, l'espacement, les effets visuels et la taille des caractères par ceux de ce fichier ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
        palette->setCurrentIndex(palette->findData(color));
        Kyronext::setManualReducedMotionChoice(reducedMotion, reducedMotionValue.toBool());
        lightMode->setChecked(lightModeValue.toBool());
        updateProfileControls();
        density->setCurrentIndex(density->findData(spacing));
        font->setValue(int(points)); apply();
    });
    layout->addWidget(restore);
    auto *advanced = new QPushButton(QStringLiteral("Réglages avancés du bureau, écran, audio et réseau…"));
    const auto settings = QStandardPaths::findExecutable(QStringLiteral("systemsettings"));
    advanced->setEnabled(!settings.isEmpty());
    advanced->setToolTip(settings.isEmpty() ? QStringLiteral("Les paramètres KDE ne sont pas installés.") : QStringLiteral("Ouvrir les composants de configuration KDE maintenus"));
    QObject::connect(advanced, &QPushButton::clicked, &window, [&] {
        if (!QProcess::startDetached(settings, {})) Kyronext::showError(&window, QStringLiteral("Les paramètres du bureau n’ont pas pu démarrer."));
    });
    layout->addWidget(advanced);
    layout->addStretch();
    window.setCentralWidget(body);
    Kyronext::addAboutMenu(&window, QStringLiteral("Kyronext Settings"), QStringLiteral("Préférences des applications, sauvegarde et accès aux composants de configuration du bureau."), QStringLiteral("Qt Widgets et QSettings"));
    window.show();
    return app.exec();
}
