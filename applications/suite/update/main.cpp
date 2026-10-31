#include "apt_simulation.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

class UpdateWindow final : public QMainWindow {
public:
    UpdateWindow() {
        setWindowTitle(tr("Kyronext Update"));
        resize(820, 620);
        watchdog = new QTimer(this);
        watchdog->setSingleShot(true);
        connect(watchdog, &QTimer::timeout, this, [this] {
            if (process && process->state() != QProcess::NotRunning) {
                timedOut = true;
                process->kill();
                refresh->setEnabled(true);
                summary->setText(tr("La simulation a dépassé 60 secondes ; elle a été arrêtée."));
                statusBar()->showMessage(tr("Aucun paquet n'a été modifié."));
            }
        });
        auto *body = new QWidget;
        auto *layout = new QVBoxLayout(body);
        layout->setContentsMargins(24, 20, 24, 20);
        auto *title = new QLabel(tr("Mises à jour disponibles"));
        title->setStyleSheet(QStringLiteral("font-size:22px;font-weight:700;"));
        layout->addWidget(title);
        auto *notice = new QLabel(tr(
            "Cette vérification simule APT avec les listes déjà présentes. "
            "Elle ne télécharge rien et n'installe ni ne supprime aucun paquet. "
            "Pour obtenir des résultats récents, utilisez d'abord les outils "
            "de mise à jour Debian/KDE maintenus."));
        notice->setWordWrap(true);
        layout->addWidget(notice);
        auto *actions = new QHBoxLayout;
        refresh = new QPushButton(tr("Analyser les paquets"));
        refresh->setDefault(true);
        summary = new QLabel(tr("Aucune analyse effectuée."));
        summary->setWordWrap(true);
        actions->addWidget(refresh);
        actions->addWidget(summary, 1);
        layout->addLayout(actions);
        output = new QPlainTextEdit;
        output->setReadOnly(true);
        output->setPlaceholderText(tr("Le résultat de la simulation APT apparaîtra ici."));
        output->setAccessibleName(tr("Résultat de l'analyse simulée APT"));
        layout->addWidget(output, 1);
        setCentralWidget(body);
        connect(refresh, &QPushButton::clicked, this, [this] { runSimulation(); });
        Kyronext::addAboutMenu(this, tr("Kyronext Update"),
            tr("Consultation en lecture seule des mises à jour visibles dans les index APT locaux."),
            QStringLiteral("Qt 6 Widgets ; APT Debian (simulation uniquement)"));
        statusBar()->showMessage(tr("Lecture seule — aucune modification système"));
    }

private:
    QPushButton *refresh = nullptr;
    QLabel *summary = nullptr;
    QPlainTextEdit *output = nullptr;
    QProcess *process = nullptr;
    QTimer *watchdog = nullptr;
    bool timedOut = false;

    void runSimulation() {
        if (process && process->state() != QProcess::NotRunning) return;
        if (process) process->deleteLater();
        process = new QProcess(this);
        timedOut = false;
        process->setProgram(QStringLiteral("/usr/bin/apt-get"));
        process->setArguments({QStringLiteral("-s"), QStringLiteral("-o"),
                               QStringLiteral("Debug::NoLocking=1"), QStringLiteral("upgrade")});
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
        environment.insert(QStringLiteral("DEBIAN_FRONTEND"), QStringLiteral("noninteractive"));
        process->setProcessEnvironment(environment);
        refresh->setEnabled(false);
        summary->setText(tr("Analyse des index APT locaux…"));
        output->clear();
        connect(process, &QProcess::readyReadStandardOutput, this, [this] {
            output->appendPlainText(QString::fromLocal8Bit(process->readAllStandardOutput()));
        });
        connect(process, &QProcess::readyReadStandardError, this, [this] {
            output->appendPlainText(QString::fromLocal8Bit(process->readAllStandardError()));
        });
        connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this](int code, QProcess::ExitStatus status) {
            watchdog->stop();
            refresh->setEnabled(true);
            if (timedOut) return;
            const QString result = output->toPlainText();
            if (status != QProcess::NormalExit || code != 0) {
                summary->setText(tr("La simulation APT a échoué (code %1). Aucun changement n'a été effectué.").arg(code));
                statusBar()->showMessage(tr("Vérifiez que APT est installé et que ses index locaux sont lisibles."));
                return;
            }
            const auto counts = summarizeAptSimulation(result);
            summary->setText(tr("%1 mise(s) à niveau simulée(s), %2 installation(s), %3 suppression(s), %4 retenue(s).")
                .arg(counts.upgrades).arg(counts.installs).arg(counts.removals).arg(counts.heldBack));
            statusBar()->showMessage(tr("Analyse terminée. Aucun paquet n'a été modifié."));
        });
        connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                refresh->setEnabled(true);
                summary->setText(tr("APT est introuvable ; aucune modification n'a été effectuée."));
                Kyronext::showError(this, tr("Impossible de démarrer /usr/bin/apt-get."));
            }
        });
        process->start();
        if (!process->waitForStarted(1500)) return;
        watchdog->start(60000);
    }
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext Update"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    UpdateWindow window;
    window.show();
    return app.exec();
}
