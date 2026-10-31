#include "archive_backend.h"
#include <kyronext/ui.h>
#include "appearance.h"
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QStatusBar>
#include <QDir>
#include <QLocale>

class ArchiveWindow final:public QMainWindow {
public:
    ArchiveWindow(){
        setWindowTitle("Kyronext Archive");resize(900,650);auto *body=new QWidget;auto *layout=new QVBoxLayout(body);layout->setContentsMargins(20,16,20,16);
        auto *brand=new QLabel("KYRONEXT / ARCHIVE");brand->setObjectName("KyronextAppBrand");brand->setStyleSheet("font-size:20px;font-weight:700;color:#ef3c60");layout->addWidget(brand);
        auto *row=new QHBoxLayout;auto *open=new QPushButton("Ouvrir ZIP / TAR");auto *create=new QPushButton("Creer depuis un dossier");extract=new QPushButton("Extraire dans un nouveau dossier");extract->setEnabled(false);row->addWidget(open);row->addWidget(create);row->addWidget(extract);layout->addLayout(row);
        location=new QLabel("Choisissez une archive locale.");location->setWordWrap(true);location->setTextFormat(Qt::PlainText);layout->addWidget(location);
        table=new QTableWidget(0,3);table->setHorizontalHeaderLabels({"Nom","Type","Taille decompressee"});table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);table->setEditTriggers(QAbstractItemView::NoEditTriggers);layout->addWidget(table);
        auto *limits=new QLabel("ZIP et TAR non compresse : 64 Mio par archive, 256 Mio extraits, 10 000 entrees. Liens, ZIP64 et extensions TAR refuses. Aucun ecrasement.");limits->setWordWrap(true);layout->addWidget(limits);setCentralWidget(body);
        connect(open,&QPushButton::clicked,this,[this]{const auto path=QFileDialog::getOpenFileName(this,"Ouvrir",{},"Archives (*.zip *.tar)");if(!path.isEmpty())load(path);});
        connect(create,&QPushButton::clicked,this,[this]{createArchive();});connect(extract,&QPushButton::clicked,this,[this]{extractArchive();});
        addOrganizerAppearance(this);Kyronext::addAboutMenu(this,"Kyronext Archive","Gestion locale ZIP/TAR avec limites et extraction protegee.","Qt Core, Gui, Widgets ; KDE KArchive");
    }
private:
    QString current;QLabel *location;QTableWidget *table;QPushButton *extract;
    void load(const QString &path){QVector<ArchiveItem> items;QString error;if(!SafeArchive::list(path,&items,&error)){Kyronext::showError(this,error);return;}current=path;location->setText(path);extract->setEnabled(true);table->setRowCount(items.size());int row=0;for(const auto &e:items){table->setItem(row,0,new QTableWidgetItem(e.name));table->setItem(row,1,new QTableWidgetItem(e.directory?"Dossier":"Fichier"));table->setItem(row++,2,new QTableWidgetItem(e.directory?"-":QLocale().formattedDataSize(e.size)));}statusBar()->showMessage(QString("%1 entrees verifiees.").arg(items.size()));}
    void createArchive(){const auto folder=QFileDialog::getExistingDirectory(this,"Dossier a archiver");if(folder.isEmpty())return;QString filter;auto output=QFileDialog::getSaveFileName(this,"Nouvelle archive",{},"ZIP (*.zip);;TAR (*.tar)",&filter);if(output.isEmpty())return;if(QFileInfo(output).suffix().isEmpty())output+=filter.startsWith("TAR")?".tar":".zip";QString error;if(!SafeArchive::create(folder,output,&error)){Kyronext::showError(this,error);return;}load(output);}
    void extractArchive(){if(current.isEmpty())return;const auto parent=QFileDialog::getExistingDirectory(this,"Choisir le dossier parent de l'extraction");if(parent.isEmpty())return;bool ok;const auto name=QInputDialog::getText(this,"Nouveau dossier d'extraction","Nom du nouveau dossier",QLineEdit::Normal,QFileInfo(current).completeBaseName()+"-extrait",&ok);if(!ok)return;if(name.isEmpty() || name=="." || name==".." || name.contains('/') || name.contains('\\')){Kyronext::showError(this,"Nom de dossier invalide.");return;}QString error;if(!SafeArchive::extract(current,QDir(parent).filePath(name),&error)){Kyronext::showError(this,error);return;}statusBar()->showMessage("Extraction terminee dans "+QDir(parent).filePath(name));}
};
int main(int argc,char **argv){QApplication app(argc,argv);Kyronext::configureApplication("Kyronext Archive","0.1.0");Kyronext::applyTheme(app,Kyronext::paletteFromSettings());ArchiveWindow window;window.show();return app.exec();}
