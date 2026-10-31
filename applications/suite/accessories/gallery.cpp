#include "imageio.h"
#include "help.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QAction>
#include <QCloseEvent>
#include <QKeySequence>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStatusBar>
#include <QStackedWidget>
#include <QTimer>
#include <QToolBar>
#include <QTransform>

class Gallery final : public QMainWindow {
    QLabel *picture = new QLabel;
    QStackedWidget *workspace = new QStackedWidget;
    QStringList files; QString path; int index = -1; QImage image; bool dirty = false;
    QTimer slide;
public:
    Gallery() {
        setWindowTitle("Kyronext Gallery"); resize(980, 720); setMinimumSize(400, 320);
        picture->setAlignment(Qt::AlignCenter); picture->setAccessibleName("Image courante");
        workspace->addWidget(Kyronext::emptyState(
            tr("Aucune image ouverte"),
            tr("Choisissez Ouvrir pour une image ou Dossier pour parcourir un dossier sélectionné."),
            workspace));
        workspace->addWidget(picture);
        setCentralWidget(workspace);
        auto *file = menuBar()->addMenu("&Fichier");
        file->addAction("Ouvrir une image…", this, [this]{ chooseFile(); }, QKeySequence::Open);
        file->addAction("Choisir un dossier…", this, [this]{ chooseFolder(); });
        file->addAction("Enregistrer…", this, [this]{ save(); }, QKeySequence::Save);
        file->addAction("Quitter", this, &QWidget::close, QKeySequence::Quit);
        auto *bar = addToolBar("Images"); bar->setMovable(false);
        bar->addAction("Ouvrir", this, [this]{ chooseFile(); });
        bar->addAction("Dossier", this, [this]{ chooseFolder(); });
        bar->addAction("Précédente", this, [this]{ navigate(-1); });
        bar->addAction("Suivante", this, [this]{ navigate(1); });
        bar->addAction("Tourner à gauche", this, [this]{ rotate(-90); });
        bar->addAction("Tourner à droite", this, [this]{ rotate(90); });
        auto *show = bar->addAction("Diaporama"); show->setCheckable(true);
        slide.setInterval(4000);
        connect(show, &QAction::toggled, this, [this](bool on){ if (on) slide.start(); else slide.stop(); });
        connect(&slide, &QTimer::timeout, this, [this]{ if (!dirty && files.size() > 1) navigate(1); });
        Kyronext::addAboutMenu(this, "Kyronext Gallery", "Visionneuse locale avec rotation et diaporama facultatif.", "Qt 6 Widgets / Gui — LGPL/GPL");
        AccessoryHelp::install(this, "Kyronext Gallery", "Ouvrez une image ou choisissez un dossier. Précédente/Suivante parcourent uniquement ce dossier.\nTourner modifie les pixels en mémoire ; Enregistrer écrit le fichier choisi après confirmation d'écrasement.\nLe diaporama est facultatif et se met en pause lorsqu'une rotation n'est pas enregistrée.\nFormats : PNG, JPEG, BMP ; limite de 16 millions de pixels. Le JPEG supprime la transparence sur fond blanc.");
        statusBar()->showMessage("Choisissez une image ou un dossier. Aucun dossier personnel n'est parcouru automatiquement.");
    }
    bool confirm() {
        if (!dirty) return true;
        auto result = QMessageBox::question(this, "Image modifiée", "Enregistrer la rotation avant de continuer ?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
        if (result == QMessageBox::Save) return save();
        return result == QMessageBox::Discard;
    }
    bool open(const QString &name) {
        QString error; auto next = AccessoryImages::load(name, error);
        if (next.isNull()) { slide.stop(); Kyronext::showError(this, error); return false; }
        if (!confirm()) return false;
        image = next; path = QFileInfo(name).absoluteFilePath(); dirty = false;
        workspace->setCurrentWidget(picture);
        setWindowTitle(QFileInfo(path).fileName() + " — Kyronext Gallery"); render(); return true;
    }
protected:
    void resizeEvent(QResizeEvent *event) override { QMainWindow::resizeEvent(event); render(); }
    void closeEvent(QCloseEvent *event) override { if (confirm()) event->accept(); else event->ignore(); }
private:
    void chooseFile() {
        auto name = QFileDialog::getOpenFileName(this, "Ouvrir une image", {}, "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!name.isEmpty() && open(name)) { files = {QFileInfo(name).absoluteFilePath()}; index = 0; }
    }
    void chooseFolder() {
        auto folder = QFileDialog::getExistingDirectory(this, "Choisir le dossier à consulter");
        if (folder.isEmpty()) return;
        QDir dir(folder);
        auto names = dir.entryInfoList({"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.PNG", "*.JPG", "*.JPEG", "*.BMP"}, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase);
        if (names.isEmpty()) { Kyronext::notify(this, "Aucune image prise en charge dans ce dossier."); return; }
        if (names.size() > 10000) { Kyronext::showError(this, "Dossier trop volumineux : limite de 10 000 images."); return; }
        if (!open(names.first().absoluteFilePath())) return;
        files.clear(); for (const auto &name : names) files << name.absoluteFilePath(); index = 0;
    }
    void navigate(int direction) {
        if (files.isEmpty()) return;
        int next = (index + direction + files.size()) % files.size();
        if (open(files[next])) index = next;
    }
    void rotate(int degrees) {
        if (image.isNull()) return;
        slide.stop(); image = image.transformed(QTransform().rotate(degrees)); dirty = true; render();
    }
    bool save() {
        if (image.isNull()) return false;
        auto name = QFileDialog::getSaveFileName(this, "Enregistrer l'image tournée", path, "Images (*.png *.jpg *.jpeg *.bmp)");
        if (name.isEmpty()) return false;
        if (QFileInfo(name).suffix().isEmpty()) name += ".png";
        QString error;
        if (!AccessoryImages::save(image, name, error)) { Kyronext::showError(this, error); return false; }
        path = QFileInfo(name).absoluteFilePath(); dirty = false;
        setWindowTitle(QFileInfo(path).fileName() + " — Kyronext Gallery"); render(); return true;
    }
    void render() {
        if (image.isNull()) return;
        picture->setPixmap(QPixmap::fromImage(image).scaled(picture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusBar()->showMessage(QString("%1 × %2 pixels · %3%4").arg(image.width()).arg(image.height()).arg(QFileInfo(path).fileName(), dirty ? " · rotation non enregistrée" : ""));
    }
};
int main(int argc, char **argv) {
    QApplication app(argc, argv); QImageReader::setAllocationLimit(128);
    Kyronext::configureApplication("Kyronext Gallery", "0.1.0"); Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    Gallery window; if (argc == 2) window.open(QString::fromLocal8Bit(argv[1]));
    window.show(); return app.exec();
}
