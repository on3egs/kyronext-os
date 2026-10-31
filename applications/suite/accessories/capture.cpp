#include "imageio.h"
#include "help.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QAction>
#include <QKeySequence>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QMenuBar>
#include <QPainter>
#include <QPushButton>
#include <QProcess>
#include <QScreen>
#include <QStringList>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QKeyEvent>

class RegionDialog final : public QDialog {
    QPixmap background; QPoint start; QRect area; bool dragging = false;
public:
    explicit RegionDialog(QPixmap image, QScreen *screen) : background(std::move(image)) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setGeometry(screen->geometry()); setCursor(Qt::CrossCursor); setModal(true);
        setAccessibleName("Sélection de zone : glisser la souris, Échap pour annuler");
    }
    QRect selection() const {
        auto normalized = area.normalized().intersected(rect());
        auto ratioX = background.width() / double(width()), ratioY = background.height() / double(height());
        return QRect(qRound(normalized.x() * ratioX), qRound(normalized.y() * ratioY), qRound(normalized.width() * ratioX), qRound(normalized.height() * ratioY));
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this); painter.drawPixmap(rect(), background);
        painter.fillRect(rect(), QColor(0, 0, 0, 80));
        painter.setPen(QPen(QColor("#ef4968"), 2, Qt::DashLine)); painter.drawRect(area.normalized());
        painter.setPen(Qt::white); painter.drawText(20, 32, "Glissez pour sélectionner une zone · Échap : annuler");
    }
    void mousePressEvent(QMouseEvent *event) override { if (event->button() == Qt::LeftButton) { start = event->position().toPoint(); dragging = true; area = QRect(start, start); update(); } }
    void mouseMoveEvent(QMouseEvent *event) override { if (dragging) { area = QRect(start, event->position().toPoint()); update(); } }
    void mouseReleaseEvent(QMouseEvent *event) override { if (dragging && event->button() == Qt::LeftButton) { area = QRect(start, event->position().toPoint()); dragging = false; if (selection().width() > 1 && selection().height() > 1) accept(); else reject(); } }
    void keyPressEvent(QKeyEvent *event) override { if (event->key() == Qt::Key_Escape) reject(); else QDialog::keyPressEvent(event); }
};

class Capture final : public QMainWindow {
    QLabel *preview = new QLabel("La capture commence uniquement après votre clic.");
    QComboBox *screens = new QComboBox; QPushButton *save = new QPushButton("Enregistrer PNG…"); QImage image;
public:
    Capture() {
        setWindowTitle("Kyronext Capture"); resize(760, 560); setMinimumSize(400, 300);
        auto *body = new QWidget; auto *layout = new QVBoxLayout(body);
        auto *heading = new QLabel("KYRONEXT / CAPTURE"); layout->addWidget(heading);
        for (auto *screen : QGuiApplication::screens()) screens->addItem(screen->name());
        screens->setAccessibleName("Écran à capturer"); layout->addWidget(screens);
        auto *whole = new QPushButton("Capturer l'écran sélectionné"); auto *region = new QPushButton("Choisir une zone…");
        layout->addWidget(whole); layout->addWidget(region);
        preview->setAlignment(Qt::AlignCenter); preview->setWordWrap(true); preview->setMinimumHeight(140); layout->addWidget(preview, 1);
        layout->addWidget(save); save->setEnabled(false); setCentralWidget(body);
        connect(whole, &QPushButton::clicked, this, [this]{ capture(false); });
        connect(region, &QPushButton::clicked, this, [this]{ capture(true); });
        connect(save, &QPushButton::clicked, this, [this]{ saveImage(); });
        auto *file = menuBar()->addMenu("&Fichier"); file->addAction("Quitter", this, &QWidget::close, QKeySequence::Quit);
        Kyronext::addAboutMenu(this, "Kyronext Capture", "Capture native sous X11 avec repli sur Spectacle et le portail du bureau.", "Qt 6 Widgets / Gui — LGPL/GPL");
        AccessoryHelp::install(this, "Kyronext Capture", "Choisissez un écran, puis Capturer ou Choisir une zone. La fenêtre se masque brièvement avant la capture.\nPour une zone, glissez la souris ; Échap annule.\nLa capture reste en mémoire. Enregistrer PNG ouvre le choix d'un fichier ; Annuler ne crée aucun fichier.\nSi la capture native n'est pas disponible (notamment sous Wayland), Kyronext ouvre Spectacle, qui utilise le portail de capture du bureau.");
        if (!supported()) statusBar()->showMessage("Capture native indisponible ici : Spectacle gérera la capture avec les autorisations du bureau.");
    }
private:
    bool supported() const { return qEnvironmentVariable("XDG_SESSION_TYPE") != "wayland" && QGuiApplication::platformName() == "xcb"; }
    bool startSpectacle(bool region) {
        const QStringList args = region ? QStringList{QStringLiteral("--region")} : QStringList{QStringLiteral("--fullscreen")};
        if (!QProcess::startDetached(QStringLiteral("spectacle"), args)) {
            Kyronext::showError(this, "La capture native n’est pas disponible et Spectacle n’est pas installé.");
            return false;
        }
        statusBar()->showMessage("Spectacle est ouvert. La capture est gérée par le bureau.");
        return true;
    }
    void capture(bool region) {
        if (!supported()) { startSpectacle(region); return; }
        auto available = QGuiApplication::screens();
        if (screens->currentIndex() < 0 || screens->currentIndex() >= available.size()) { Kyronext::showError(this, "Écran non disponible."); return; }
        auto *screen = available[screens->currentIndex()];
        if (qint64(screen->size().width()) * screen->size().height() * screen->devicePixelRatio() * screen->devicePixelRatio() > 32000000) {
            Kyronext::showError(this, "Écran trop volumineux pour cette première version (32 millions de pixels maximum)."); return;
        }
        bool previousQuit = qApp->quitOnLastWindowClosed(); qApp->setQuitOnLastWindowClosed(false);
        hide();
        QTimer::singleShot(350, this, [this, screen, region, previousQuit]{
            auto restore = [this, previousQuit]{ show(); qApp->setQuitOnLastWindowClosed(previousQuit); };
            if (!QGuiApplication::screens().contains(screen)) { restore(); Kyronext::showError(this, "L'écran a été déconnecté."); return; }
            auto pixmap = screen->grabWindow(0);
            if (pixmap.isNull()) { restore(); startSpectacle(region); return; }
            QImage next = pixmap.toImage();
            if (region) {
                RegionDialog dialog(pixmap, screen);
                if (dialog.exec() != QDialog::Accepted) { restore(); return; }
                next = next.copy(dialog.selection());
            }
            restore(); image = next; save->setEnabled(!image.isNull());
            preview->setPixmap(QPixmap::fromImage(image).scaled(preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            statusBar()->showMessage(QString("Capture en mémoire : %1 × %2 pixels. Rien n'est enregistré sans votre choix.").arg(image.width()).arg(image.height()));
        });
    }
    void saveImage() {
        if (image.isNull()) return;
        auto name = QFileDialog::getSaveFileName(this, "Enregistrer la capture", "Kyronext-capture.png", "PNG (*.png)");
        if (name.isEmpty()) return;
        if (!name.endsWith(".png", Qt::CaseInsensitive)) name += ".png";
        QString error;
        if (!AccessoryImages::save(image, name, error)) Kyronext::showError(this, error);
        else statusBar()->showMessage("Capture enregistrée : " + name);
    }
};
int main(int argc, char **argv) {
    QApplication app(argc, argv); Kyronext::configureApplication("Kyronext Capture", "0.1.0");
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings()); Capture window; window.show(); return app.exec();
}
