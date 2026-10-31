#include "health_probe.h"

#include <kyronext/ui.h>
#include <QApplication>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QProcess>
#include <QPushButton>
#include <QFileInfo>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

static QString trustedSmartctlPath() {
    for (const QString &candidate : {QStringLiteral("/usr/sbin/smartctl"),
                                     QStringLiteral("/usr/bin/smartctl"),
                                     QStringLiteral("/sbin/smartctl")}) {
        const QFileInfo executable(candidate);
        if (executable.isFile() && executable.isExecutable()) return candidate;
    }
    return {};
}

class DiskHealthWindow final : public QMainWindow {
public:
    DiskHealthWindow() {
        setWindowTitle(tr("Kyronext Disk Health"));
        resize(760, 430);
        auto *body = new QWidget(this);
        auto *layout = new QVBoxLayout(body);
        auto *title = new QLabel(tr("État de santé des disques"));
        title->setStyleSheet(QStringLiteral("font-size:22px;font-weight:700;color:#ef3c60"));
        layout->addWidget(title);
        auto *notice = new QLabel(tr("Diagnostic local en lecture seule. Aucun disque n'est modifié. "
            "Aucun accès administrateur n'est demandé. Un état inconnu signifie que le contrôleur, "
            "SMART ou les permissions ne permettent pas de conclure ; ce n'est pas une certification du disque."));
        notice->setWordWrap(true);
        layout->addWidget(notice);
        table = new QTableWidget(0, 3);
        table->setHorizontalHeaderLabels({tr("Périphérique"), tr("Modèle"), tr("Résultat")});
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        layout->addWidget(table, 1);
        auto *actions = new QHBoxLayout;
        refresh = new QPushButton(tr("Actualiser le diagnostic"));
        actions->addWidget(refresh);
        status = new QLabel;
        status->setWordWrap(true);
        actions->addWidget(status, 1);
        layout->addLayout(actions);
        setCentralWidget(body);
        connect(refresh, &QPushButton::clicked, this, [this] { inspect(); });
        Kyronext::addAboutMenu(this, tr("Kyronext Disk Health"),
            tr("Diagnostic SMART local en lecture seule, sans privilèges administrateur."),
            QStringLiteral("Qt 6 ; smartmontools facultatif — composants Debian"));
        inspect();
    }

private:
    QTableWidget *table = nullptr;
    QPushButton *refresh = nullptr;
    QLabel *status = nullptr;
    QVector<DiskHealth> disks;
    int current = 0;
    QProcess *process = nullptr;
    bool timedOut = false;

    void inspect() {
        if (process && process->state() != QProcess::NotRunning) return;
        disks = discoverDisks();
        current = 0;
        table->setRowCount(disks.size());
        for (int row = 0; row < disks.size(); ++row) {
            table->setItem(row, 0, new QTableWidgetItem(disks[row].device));
            table->setItem(row, 1, new QTableWidgetItem(disks[row].model));
            table->setItem(row, 2, new QTableWidgetItem(tr("En attente")));
        }
        const QString smartctl = trustedSmartctlPath();
        if (disks.isEmpty()) {
            status->setText(tr("Aucun disque physique reconnu via /sys/block."));
            return;
        }
        if (smartctl.isEmpty()) {
            for (int row = 0; row < disks.size(); ++row)
                table->item(row, 2)->setText(tr("Inconnu — smartmontools absent"));
            status->setText(tr("Installez smartmontools depuis les dépôts Debian vérifiés si vous souhaitez cette lecture."));
            return;
        }
        status->setText(tr("Interrogation SMART en lecture seule, sans élévation de privilèges…"));
        runNext(smartctl);
    }

    void runNext(const QString &smartctl) {
        if (current >= disks.size()) {
            refresh->setEnabled(true);
            status->setText(tr("Analyse terminée. Les états SMART ne remplacent pas une sauvegarde."));
            return;
        }
        const int row = current++;
        process = new QProcess(this);
        QProcess *running = process;
        timedOut = false;
        running->setProgram(smartctl);
        running->setArguments({QStringLiteral("-j"), QStringLiteral("-H"), disks[row].device});
        refresh->setEnabled(false);
        connect(running, &QProcess::finished, this, [this, row, smartctl, running](int code, QProcess::ExitStatus exitStatus) {
            if (process != running) return;
            const auto result = timedOut
                ? SmartHealthResult{HealthState::Unknown, tr("La lecture SMART a dépassé le délai de 10 secondes.")}
                : exitStatus == QProcess::NormalExit
                    ? parseSmartHealth(running->readAllStandardOutput(), code)
                    : SmartHealthResult{HealthState::Unknown, tr("Lecture interrompue ou impossible.")};
            timedOut = false;
            QString label;
            if (result.state == HealthState::Healthy) label = tr("Aucun échec signalé");
            else if (result.state == HealthState::Warning) label = tr("Avertissement");
            else label = tr("Inconnu");
            table->item(row, 2)->setText(label + QStringLiteral(" — ") + result.summary);
            running->deleteLater();
            process = nullptr;
            runNext(smartctl);
        });
        connect(running, &QProcess::errorOccurred, this, [this, row, smartctl, running](QProcess::ProcessError error) {
            if (process != running || error != QProcess::FailedToStart) return;
            const QString reason = timedOut
                ? tr("Inconnu — délai de démarrage dépassé")
                : tr("Inconnu — smartctl n'a pas pu démarrer");
            table->item(row, 2)->setText(reason);
            timedOut = false;
            running->deleteLater();
            process = nullptr;
            runNext(smartctl);
        });
        running->start();
        QTimer::singleShot(10000, running, [this, running] {
            if (process == running && running->state() != QProcess::NotRunning) {
                timedOut = true;
                running->kill();
            }
        });
    }
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Kyronext::configureApplication(QStringLiteral("Kyronext Disk Health"), QStringLiteral("0.1.0"));
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    DiskHealthWindow window;
    window.show();
    return app.exec();
}
