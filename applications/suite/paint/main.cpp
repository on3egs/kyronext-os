#include "document.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QActionGroup>
#include <QAction>
#include <QKeySequence>
#include <QTabWidget>
#include <QScrollArea>
#include <QPainter>
#include <QMouseEvent>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QStatusBar>
#include <QInputDialog>
#include <QLineEdit>
#include <QCloseEvent>
#include <QFileInfo>
#include <QScreen>
#include <functional>

class Canvas final : public QWidget {
public:
    PaintDocument document;
    QString path;
    QColor color = QColor("#ef3c60");
    QFont textFont = QFont("Sans Serif",20);
    int width = 8, tool = 0;
    QRect selection;
    std::function<void()> changed;
    std::function<void(QColor)> colorPicked;
    explicit Canvas(QWidget *parent=nullptr):QWidget(parent) {
        setAccessibleName(tr("Zone de dessin")); setCursor(Qt::CrossCursor); resizeToImage();
    }
    void setZoom(qreal value) { zoom=value; resizeToImage(); update(); }
    void resizeToImage() { setFixedSize((QSizeF(document.image().size())*zoom).toSize()); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        for(int y=0;y<height();y+=16) for(int x=0;x<QWidget::width();x+=16)
            painter.fillRect(x,y,16,16,((x/16+y/16)%2)?QColor("#c7c7c7"):QColor("#eeeeee"));
        painter.scale(zoom,zoom); painter.drawImage(0,0,document.image());
        if(drawing && tool>=2 && tool<=4) {
            painter.setPen(QPen(color,width,Qt::DashLine));
            if(tool==2) painter.drawLine(start,previous);
            else if(tool==3) painter.drawRect(QRect(start,previous).normalized());
            else painter.drawEllipse(QRect(start,previous).normalized());
        }
        if(!selection.isEmpty()) { painter.setPen(QPen(QColor("#313a52"),1/zoom,Qt::DashLine)); painter.drawRect(selection); }
    }
    void mousePressEvent(QMouseEvent *event) override {
        if(event->button()!=Qt::LeftButton) return;
        start=previous=point(event);
        if(tool==5) { document.fill(start,color); signalChange(); return; }
        if(tool==6) { if(colorPicked) colorPicked(document.image().pixelColor(start)); return; }
        if(tool==7) {
            bool ok=false; const QString value=QInputDialog::getText(this,tr("Texte"),tr("Texte a dessiner"),QLineEdit::Normal,{},&ok);
            if(ok) { document.text(start,value,color,textFont); signalChange(); } return;
        }
        drawing=true;
        if(tool<=1) { document.beginStroke(); document.line(previous,previous,color,width,tool==1); signalChange(); }
        if(tool==8) { selection={}; update(); }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if(!drawing) return;
        const QPoint position=point(event);
        if(tool<=1) document.line(previous,position,color,width,tool==1);
        previous=position;
        if(tool==8) selection=QRect(start,position).normalized().intersected(document.image().rect());
        signalChange();
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        if(event->button()!=Qt::LeftButton || !drawing) return;
        previous=point(event); drawing=false;
        if(tool>=2 && tool<=4) document.shape(start,previous,color,width,tool-2);
        signalChange();
    }
private:
    qreal zoom=1.0; bool drawing=false; QPoint previous,start;
    QPoint point(QMouseEvent *event) const {
        const QPoint p=(event->position()/zoom).toPoint();
        return {qBound(0,p.x(),document.image().width()-1),qBound(0,p.y(),document.image().height()-1)};
    }
    void signalChange() { update(); if(changed) changed(); }
};

class PaintWindow final : public QMainWindow {
public:
    PaintWindow() {
        setWindowTitle("Kyronext Paint"); tabs = new QTabWidget;
        tabs->setTabsClosable(true); setCentralWidget(tabs);
        auto *file = menuBar()->addMenu(tr("&Fichier"));
        auto *newAction = file->addAction(tr("&Nouveau")); newAction->setShortcut(QKeySequence::New);
        connect(newAction, &QAction::triggered, this, [this] { newDocument(); });
        auto *openAction = file->addAction(tr("&Ouvrir…")); openAction->setShortcut(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, [this] {
            const QString path = QFileDialog::getOpenFileName(this, tr("Ouvrir une image"), {},
                tr("Images (*.png *.jpg *.jpeg *.bmp)"));
            if (!path.isEmpty()) open(path);
        });
        auto *saveAction = file->addAction(tr("&Enregistrer")); saveAction->setShortcut(QKeySequence::Save);
        connect(saveAction, &QAction::triggered, this, [this] { if (canvas()) save(canvas(), false); });
        auto *saveAsAction = file->addAction(tr("Enregistrer &sous…")); saveAsAction->setShortcut(QKeySequence::SaveAs);
        connect(saveAsAction, &QAction::triggered, this, [this] { if (canvas()) save(canvas(), true); });
        file->addSeparator();
        auto *closeTab = file->addAction(tr("Fermer le document")); closeTab->setShortcut(QKeySequence::Close);
        connect(closeTab, &QAction::triggered, this, [this] { closeDocument(tabs->currentIndex()); });
        auto *quit = file->addAction(tr("&Quitter")); quit->setShortcut(QKeySequence::Quit);
        connect(quit, &QAction::triggered, this, &QWidget::close);
        auto *edit = menuBar()->addMenu(tr("É&dition"));
        undoAction = edit->addAction(tr("Annuler")); undoAction->setShortcut(QKeySequence::Undo);
        redoAction = edit->addAction(tr("Rétablir")); redoAction->setShortcut(QKeySequence::Redo);
        connect(undoAction, &QAction::triggered, this, [this] { if (canvas()) { canvas()->document.undo(); refresh(); } });
        connect(redoAction, &QAction::triggered, this, [this] { if (canvas()) { canvas()->document.redo(); refresh(); } });
        auto *imageMenu=menuBar()->addMenu(tr("&Image"));
        auto *crop=imageMenu->addAction(tr("Recadrer la selection"));
        connect(crop,&QAction::triggered,this,[this]{if(!canvas())return;QString error;if(!canvas()->document.crop(canvas()->selection,&error))Kyronext::showError(this,error);else{canvas()->selection={};refresh();}});
        auto *resizeAction=imageMenu->addAction(tr("Redimensionner"));
        connect(resizeAction,&QAction::triggered,this,[this]{
            if(!canvas())return;bool ok;
            const int w=QInputDialog::getInt(this,tr("Redimensionner"),tr("Largeur"),canvas()->document.image().width(),1,4096,1,&ok);if(!ok)return;
            const int h=QInputDialog::getInt(this,tr("Redimensionner"),tr("Hauteur"),canvas()->document.image().height(),1,4096,1,&ok);if(!ok)return;
            QString error;if(!canvas()->document.resizeImage({w,h},&error))Kyronext::showError(this,error);refresh();
        });
        auto *rotate=imageMenu->addAction(tr("Rotation de 90 degres"));
        connect(rotate,&QAction::triggered,this,[this]{if(canvas()){canvas()->document.rotateClockwise();canvas()->selection={};refresh();}});
        auto *appearance = menuBar()->addMenu(tr("A&ffichage"));
        auto *red = appearance->addAction(tr("Graphite / rouge"));
        auto *amber = appearance->addAction(tr("Graphite / ambre"));
        connect(red, &QAction::triggered, this, [] { setPalette(Kyronext::Palette::Red); });
        connect(amber, &QAction::triggered, this, [] { setPalette(Kyronext::Palette::Amber); });
        auto *tools = addToolBar(tr("Outils de dessin")); tools->setMovable(false);
        toolChoice=new QComboBox;
        toolChoice->addItems({tr("Pinceau"),tr("Gomme"),tr("Ligne"),tr("Rectangle"),tr("Ellipse"),tr("Remplissage"),tr("Pipette"),tr("Texte"),tr("Selection")});
        toolChoice->setAccessibleName(tr("Outil actif")); tools->addWidget(toolChoice);
        connect(toolChoice,&QComboBox::currentIndexChanged,this,[this]{configureCanvas();});
        auto *fontAction=tools->addAction(tr("Police"));
        connect(fontAction,&QAction::triggered,this,[this]{ if(!canvas())return; bool ok; auto f=QFontDialog::getFont(&ok,canvas()->textFont,this);if(ok)canvas()->textFont=f; });
        auto *color = tools->addAction(tr("Couleur…"));
        connect(color, &QAction::triggered, this, [this] {
            QColor value = QColorDialog::getColor(paintColor, this, tr("Couleur du pinceau"), QColorDialog::ShowAlphaChannel);
            if (value.isValid()) { paintColor = value; configureCanvas(); }
        });
        tools->addWidget(new QLabel(tr(" Épaisseur ")));
        thickness = new QSpinBox; thickness->setRange(1, 200); thickness->setValue(8);
        thickness->setAccessibleName(tr("Épaisseur du pinceau en pixels")); tools->addWidget(thickness);
        connect(thickness, &QSpinBox::valueChanged, this, [this] { configureCanvas(); });
        tools->addSeparator(); tools->addWidget(new QLabel(tr(" Zoom ")));
        zoom = new QComboBox; zoom->addItems({"25 %", "50 %", "100 %", "200 %", "400 %"});
        zoom->setCurrentIndex(2); zoom->setAccessibleName(tr("Zoom de l'image")); tools->addWidget(zoom);
        connect(zoom, &QComboBox::currentIndexChanged, this, [this] { configureCanvas(); });
        Kyronext::addAboutMenu(this, "Kyronext Paint", tr("Dessin natif avec pinceau, transparence et sauvegarde d'images."),
            "Qt Core, Gui et Widgets ; plugins d'images Qt du système");
        connect(tabs, &QTabWidget::tabCloseRequested, this, [this](int index) { closeDocument(index); });
        connect(tabs, &QTabWidget::currentChanged, this, [this] { configureCanvas(); refresh(); });
        const QSize screen = QApplication::primaryScreen()->availableGeometry().size();
        resize(qMin(1100, screen.width() * 9 / 10), qMin(800, screen.height() * 9 / 10));
        addCanvas(new Canvas); refresh();
    }
    void open(const QString &path) {
        if (tabs->count() >= 8) { Kyronext::showError(this, tr("Fermez un document avant d'en ouvrir un autre (8 maximum).")); return; }
        auto *value = new Canvas;
        QString error;
        if (!value->document.load(path, &error)) { delete value; Kyronext::showError(this, error); return; }
        value->path = path; value->resizeToImage(); addCanvas(value);
    }
protected:
    void closeEvent(QCloseEvent *event) override {
        for (int index = tabs->count() - 1; index >= 0; --index) {
            tabs->setCurrentIndex(index);
            if (!confirm(canvas(index))) { event->ignore(); return; }
        }
        event->accept();
    }
private:
    QTabWidget *tabs;
    QAction *undoAction, *redoAction;
    QComboBox *toolChoice;
    QSpinBox *thickness;
    QComboBox *zoom;
    QColor paintColor = QColor("#ef3c60");
    static void setPalette(Kyronext::Palette palette) {
        Kyronext::setPreferredPalette(palette); Kyronext::applyTheme(*qApp, palette);
    }
    Canvas *canvas(int index = -1) const {
        auto *scroll = qobject_cast<QScrollArea *>(tabs->widget(index < 0 ? tabs->currentIndex() : index));
        return scroll ? static_cast<Canvas *>(scroll->widget()) : nullptr;
    }
    void addCanvas(Canvas *value) {
        auto *scroll = new QScrollArea; scroll->setWidget(value); scroll->setAlignment(Qt::AlignCenter);
        value->changed = [this] { refresh(); };
        value->colorPicked=[this](QColor c){paintColor=c;configureCanvas();};
        tabs->addTab(scroll, tr("Sans titre")); tabs->setCurrentWidget(scroll); configureCanvas(); refresh();
    }
    void newDocument() {
        if (tabs->count() >= 8) { Kyronext::showError(this, tr("Limite de 8 documents ouverts.")); return; }
        bool ok;
        int width = QInputDialog::getInt(this, tr("Nouveau document"), tr("Largeur en pixels"), 1024, 1, 4096, 1, &ok);
        if (!ok) return;
        int height = QInputDialog::getInt(this, tr("Nouveau document"), tr("Hauteur en pixels"), 768, 1, 4096, 1, &ok);
        if (!ok) return;
        auto *value = new Canvas; QString error;
        if (!value->document.create(width, height, &error)) { delete value; Kyronext::showError(this, error); return; }
        value->resizeToImage(); addCanvas(value);
    }
    bool save(Canvas *value, bool saveAs) {
        QString path = value->path;
        if (saveAs || path.isEmpty()) {
            QString filter;
            path = QFileDialog::getSaveFileName(this, tr("Enregistrer l'image"), path,
                "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)", &filter);
            if (path.isEmpty()) return false;
            if (QFileInfo(path).suffix().isEmpty()) path += filter.startsWith("JPEG") ? ".jpg" : filter.startsWith("BMP") ? ".bmp" : ".png";
        }
        const QByteArray format = QFileInfo(path).suffix().toLatin1().toLower();
        if ((format == "jpg" || format == "jpeg" || format == "bmp") && value->document.image().hasAlphaChannel()) {
            if (QMessageBox::question(this, tr("Fond opaque"), tr("Ce format sera enregistré sur fond blanc. Continuer ?")) != QMessageBox::Yes)
                return false;
        }
        QString error;
        if (!value->document.save(path, format, &error)) { Kyronext::showError(this, error); return false; }
        value->path = path; refresh(); return true;
    }
    bool confirm(Canvas *value) {
        if (!value || !value->document.modified()) return true;
        auto result = QMessageBox::warning(this, tr("Modifications non enregistrées"),
            tr("Enregistrer les modifications de %1 ?").arg(value->path.isEmpty() ? tr("Sans titre") : QFileInfo(value->path).fileName()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        return result == QMessageBox::Discard || (result == QMessageBox::Save && save(value, false));
    }
    void closeDocument(int index) {
        if (index < 0 || !confirm(canvas(index))) return;
        QWidget *page = tabs->widget(index); tabs->removeTab(index); delete page; refresh();
    }
    void configureCanvas() {
        if (!canvas()) return;
        canvas()->width = thickness->value(); canvas()->color = paintColor; canvas()->tool = toolChoice->currentIndex();
        static const qreal factors[] = {0.25, 0.5, 1, 2, 4};
        canvas()->setZoom(factors[qMax(0, zoom->currentIndex())]);
    }
    void refresh() {
        for (int index = 0; index < tabs->count(); ++index) {
            Canvas *value = canvas(index);
            tabs->setTabText(index, (value->path.isEmpty() ? tr("Sans titre") : QFileInfo(value->path).fileName()) +
                (value->document.modified() ? " *" : ""));
        }
        undoAction->setEnabled(canvas() && canvas()->document.canUndo());
        redoAction->setEnabled(canvas() && canvas()->document.canRedo());
        if (canvas()) {
            canvas()->resizeToImage(); canvas()->update();
            statusBar()->showMessage(tr("%1 × %2 pixels — UTF-8 pour les chemins — historique limité en mémoire")
                .arg(canvas()->document.image().width()).arg(canvas()->document.image().height()));
        } else statusBar()->clearMessage();
    }
};
int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Kyronext::configureApplication("Kyronext Paint", "0.1.0");
    Kyronext::applyTheme(app, Kyronext::paletteFromSettings());
    PaintWindow window;
    if (argc > 1) window.open(QString::fromLocal8Bit(argv[1]));
    window.show(); return app.exec();
}
