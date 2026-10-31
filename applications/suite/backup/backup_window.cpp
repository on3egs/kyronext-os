#include "backup_window.h"

#include <kyronext/ui.h>

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

BackupWindow::BackupWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("Kyronext Backup"));
    resize(760, 560);

    auto *body = new QWidget(this);
    auto *layout = new QVBoxLayout(body);
    layout->setContentsMargins(24, 20, 24, 18);
    layout->setSpacing(12);

    auto *brand = new QLabel(tr("KYRONEXT  /  BACKUP"), body);
    brand->setStyleSheet(QStringLiteral("font-size:22px;font-weight:700;letter-spacing:2px;color:#ef3c60"));
    layout->addWidget(brand);

    auto *intro = new QLabel(tr("Créez une archive locale de dossiers que vous choisissez."), body);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *sourceLabel = new QLabel(tr("Dossiers source sélectionnés"), body);
    sourceLabel->setStyleSheet(QStringLiteral("font-weight:600"));
    layout->addWidget(sourceLabel);
    sources_ = new QListWidget(body);
    sources_->setAccessibleName(tr("Dossiers source inclus dans la sauvegarde"));
    sources_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    sources_->setMinimumHeight(170);
    layout->addWidget(sources_, 1);

    auto *sourceButtons = new QHBoxLayout;
    addSource_ = new QPushButton(tr("Ajouter un dossier…"), body);
    removeSources_ = new QPushButton(tr("Retirer la sélection"), body);
    addSource_->setToolTip(tr("Choisir explicitement un dossier à sauvegarder"));
    removeSources_->setToolTip(tr("Retirer les dossiers sélectionnés de cette sauvegarde"));
    sourceButtons->addWidget(addSource_);
    sourceButtons->addWidget(removeSources_);
    sourceButtons->addStretch();
    layout->addLayout(sourceButtons);

    auto *destinationLabel = new QLabel(tr("Emplacement de l’archive"), body);
    destinationLabel->setStyleSheet(QStringLiteral("font-weight:600"));
    layout->addWidget(destinationLabel);
    auto *destinationRow = new QHBoxLayout;
    destination_ = new QLineEdit(body);
    destination_->setPlaceholderText(tr("Choisissez un nouvel emplacement .tar.gz"));
    destination_->setAccessibleName(tr("Chemin de l’archive de sauvegarde"));
    browseDestination_ = new QPushButton(tr("Parcourir…"), body);
    destinationRow->addWidget(destination_, 1);
    destinationRow->addWidget(browseDestination_);
    layout->addLayout(destinationRow);

    auto *notice = new QLabel(
        tr("Confidentialité : aucun privilège administrateur n’est utilisé. Les archives ne sont pas chiffrées ; "
           "choisissez un emplacement de confiance et protégez-le selon vos besoins."), body);
    notice->setWordWrap(true);
    notice->setStyleSheet(QStringLiteral("padding:10px;border:1px solid #70404b;border-radius:6px;color:#ffd8df;background:#21171c"));
    layout->addWidget(notice);

    progress_ = new QProgressBar(body);
    progress_->setRange(0, 1);
    progress_->setValue(0);
    progress_->setTextVisible(false);
    progress_->hide();
    layout->addWidget(progress_);

    auto *actions = new QHBoxLayout;
    actions->addStretch();
    cancel_ = new QPushButton(tr("Annuler"), body);
    cancel_->hide();
    start_ = new QPushButton(tr("Créer la sauvegarde"), body);
    start_->setDefault(true);
    actions->addWidget(cancel_);
    actions->addWidget(start_);
    layout->addLayout(actions);
    setCentralWidget(body);
    statusBar()->showMessage(tr("Aucun dossier sélectionné"));

    connect(addSource_, &QPushButton::clicked, this, [this] { addSource(); });
    connect(removeSources_, &QPushButton::clicked, this, [this] {
        const auto selected = sources_->selectedItems();
        for (auto *item : selected) delete sources_->takeItem(sources_->row(item));
        statusBar()->showMessage(tr("%1 dossier(s) sélectionné(s)").arg(sources_->count()));
    });
    connect(browseDestination_, &QPushButton::clicked, this, [this] { chooseDestination(); });
    connect(start_, &QPushButton::clicked, this, [this] { startBackup(); });
    connect(cancel_, &QPushButton::clicked, this, [this] {
        cancelRequested_.store(true);
        statusBar()->showMessage(tr("Annulation demandée…"));
        cancel_->setEnabled(false);
    });

    auto *appearance = menuBar()->addMenu(tr("Apparence"));
    connect(appearance->addAction(tr("Graphite / rouge")), &QAction::triggered, this, [] {
        Kyronext::applyTheme(*qApp, Kyronext::Palette::Red);
    });
    connect(appearance->addAction(tr("Graphite / ambre")), &QAction::triggered, this, [] {
        Kyronext::applyTheme(*qApp, Kyronext::Palette::Amber);
    });
    Kyronext::addAboutMenu(this, tr("Kyronext Backup"),
        tr("Sauvegarde locale explicite en archive TAR compressée, sans accès administrateur."),
        QStringLiteral("Qt 6 ; tar GNU ; SHA-256 via Qt Cryptographic Hash"));
}

BackupWindow::~BackupWindow() {
    if (worker_ && worker_->isRunning()) {
        cancelRequested_.store(true);
        worker_->wait();
    }
}

void BackupWindow::addSource() {
    const QString directory = QFileDialog::getExistingDirectory(
        this, tr("Choisir un dossier source"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (directory.isEmpty()) return;
    const QString canonical = QFileInfo(directory).canonicalFilePath();
    if (canonical.isEmpty()) {
        Kyronext::showError(this, tr("Ce dossier n’est pas accessible."));
        return;
    }
    for (int i = 0; i < sources_->count(); ++i) {
        if (sources_->item(i)->text() == canonical) return;
    }
    sources_->addItem(canonical);
    statusBar()->showMessage(tr("%1 dossier(s) sélectionné(s)").arg(sources_->count()));
}

void BackupWindow::chooseDestination() {
    const QString suggested = QStringLiteral("Kyronext-Backup-%1.tar.gz")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Choisir un nouvel emplacement"), QDir::home().filePath(suggested),
        tr("Archive TAR compressée (*.tar.gz *.tgz)"));
    if (!path.isEmpty()) destination_->setText(path);
}

void BackupWindow::startBackup() {
    if (sources_->count() == 0) {
        Kyronext::showError(this, tr("Ajoutez au moins un dossier source."));
        return;
    }
    const QString target = destination_->text().trimmed();
    if (target.isEmpty() || QFileInfo(target).fileName().isEmpty()) {
        Kyronext::showError(this, tr("Choisissez un fichier de destination."));
        return;
    }
    if (QFileInfo::exists(target) || QFileInfo(target).isSymLink()) {
        Kyronext::showError(this, tr("Ce fichier existe déjà. Choisissez un nouvel emplacement ; rien n’a été remplacé."));
        return;
    }

    QStringList paths;
    for (int i = 0; i < sources_->count(); ++i) paths.append(sources_->item(i)->text());
    cancelRequested_.store(false);
    workerResult_ = {};
    setBusy(true);
    statusBar()->showMessage(tr("Création de l’archive… Vous pouvez demander l’annulation."));

    worker_ = QThread::create([this, paths, target] {
        workerResult_ = createBackupArchive(paths, target, &cancelRequested_);
    });
    connect(worker_, &QThread::finished, this, [this] { finishBackup(); });
    worker_->start();
}

void BackupWindow::finishBackup() {
    if (worker_) {
        worker_->wait();
        worker_->deleteLater();
        worker_ = nullptr;
    }
    setBusy(false);
    if (workerResult_.success) {
        const QString message = tr("Archive créée : %1\nTaille : %2 octets\nSHA-256 : %3\n\n"
                                   "Conservez aussi cette empreinte pour vérifier le fichier plus tard.")
            .arg(destination_->text().trimmed()).arg(workerResult_.bytes).arg(workerResult_.sha256);
        statusBar()->showMessage(tr("Sauvegarde terminée"));
        QMessageBox dialog(QMessageBox::Information, tr("Sauvegarde terminée"), message,
                           QMessageBox::Ok, this);
        auto *copy = dialog.addButton(tr("Copier SHA-256"), QMessageBox::ActionRole);
        dialog.exec();
        if (dialog.clickedButton() == copy) {
            QApplication::clipboard()->setText(workerResult_.sha256);
            statusBar()->showMessage(tr("Empreinte SHA-256 copiée dans le presse-papiers"));
        }
    } else if (workerResult_.cancelled) {
        statusBar()->showMessage(tr("Sauvegarde annulée ; aucune archive finale n’a été publiée."));
    } else {
        const QString message = workerResult_.error.isEmpty()
            ? tr("La sauvegarde a échoué. Aucune archive finale n’a été publiée.")
            : workerResult_.error;
        statusBar()->showMessage(tr("Échec de la sauvegarde"));
        Kyronext::showError(this, message);
    }
}

void BackupWindow::setBusy(bool busy) {
    sources_->setEnabled(!busy);
    addSource_->setEnabled(!busy);
    removeSources_->setEnabled(!busy);
    destination_->setEnabled(!busy);
    browseDestination_->setEnabled(!busy);
    start_->setEnabled(!busy);
    cancel_->setEnabled(busy);
    cancel_->setVisible(busy);
    progress_->setVisible(busy);
    if (busy) progress_->setRange(0, 0);
    else { progress_->setRange(0, 1); progress_->setValue(0); }
}
