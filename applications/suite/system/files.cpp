#include "fileops.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDateTime>
#include <QDir>
#include <QKeySequence>
#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QListWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QLocale>
#include <KJob>

class FolderPage final:public QWidget {
public:
    QFileSystemModel *model;QTreeView *details;QListView *icons;QStackedWidget *views;QString directory;
    FolderPage(const QString &path) {
        model=new QFileSystemModel(this);model->setReadOnly(true);model->setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
        details=new QTreeView;details->setModel(model);details->setRootIsDecorated(false);details->setItemsExpandable(false);
        details->setSelectionMode(QAbstractItemView::SingleSelection);details->setSortingEnabled(true);details->sortByColumn(0,Qt::AscendingOrder);
        details->header()->setSectionResizeMode(0,QHeaderView::Stretch);
        icons=new QListView;icons->setModel(model);icons->setViewMode(QListView::IconMode);icons->setIconSize({48,48});icons->setResizeMode(QListView::Adjust);
        icons->setSelectionMode(QAbstractItemView::SingleSelection);views=new QStackedWidget;views->addWidget(details);views->addWidget(icons);
        auto *layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->addWidget(views);navigate(path);
    }
    bool navigate(const QString &path) {
        QFileInfo value(path);if(!value.isDir() || !value.isReadable())return false;directory=value.absoluteFilePath();
        const auto root=model->setRootPath(directory);details->setRootIndex(root);icons->setRootIndex(root);return true;
    }
    QString selected()const {const QModelIndex index=views->currentIndex()==0?details->currentIndex():icons->currentIndex();return index.isValid()?model->filePath(index):QString();}
};
class FilesWindow final:public QMainWindow {
public:
    FilesWindow() {
        setWindowTitle("Kyronext Files");resize(1100,720);
        auto *split=new QSplitter;places=new QListWidget;places->setMaximumWidth(240);places->setAccessibleName("Emplacements et favoris");
        tabs=new QTabWidget;tabs->setTabsClosable(true);split->addWidget(places);split->addWidget(tabs);split->setStretchFactor(1,1);setCentralWidget(split);
        auto *navigation=addToolBar("Navigation");navigation->setMovable(false);
        connect(navigation->addAction("Dossier parent"),&QAction::triggered,this,[this]{if(page())navigate(QFileInfo(page()->directory).absolutePath());});
        connect(navigation->addAction("Accueil"),&QAction::triggered,this,[this]{navigate(QDir::homePath());});
        address=new QLineEdit;address->setAccessibleName("Adresse du dossier local");navigation->addWidget(address);
        connect(address,&QLineEdit::returnPressed,this,[this]{navigate(address->text());});
        auto *search=new QLineEdit;search->setPlaceholderText("Filtrer ce dossier");search->setAccessibleName("Filtre local des noms");navigation->addWidget(search);
        connect(search,&QLineEdit::textChanged,this,[this](const QString &text){if(page()){page()->model->setNameFilters(text.isEmpty()?QStringList():QStringList{"*"+text+"*"});page()->model->setNameFilterDisables(false);}});
        auto *file=menuBar()->addMenu("Fichier");
        auto *newTab=file->addAction("Nouvel onglet");newTab->setShortcut(QKeySequence("Ctrl+T"));connect(newTab,&QAction::triggered,this,[this]{addTab(QDir::homePath());});
        connect(file->addAction("Nouveau dossier"),&QAction::triggered,this,[this]{newFolder();});
        auto *close=file->addAction("Fermer l'onglet");close->setShortcut(QKeySequence("Ctrl+W"));connect(close,&QAction::triggered,this,[this]{closeTab(tabs->currentIndex());});
        auto *edit=menuBar()->addMenu("Operations");
        auto *copy=edit->addAction("Copier");copy->setShortcut(QKeySequence::Copy);connect(copy,&QAction::triggered,this,[this]{copyClipboard(false);});
        auto *cut=edit->addAction("Couper");cut->setShortcut(QKeySequence::Cut);connect(cut,&QAction::triggered,this,[this]{copyClipboard(true);});
        auto *paste=edit->addAction("Coller");paste->setShortcut(QKeySequence::Paste);connect(paste,&QAction::triggered,this,[this]{pasteClipboard();});
        edit->addSeparator();
        connect(edit->addAction("Copier vers..."),&QAction::triggered,this,[this]{transfer(false);});
        connect(edit->addAction("Deplacer vers..."),&QAction::triggered,this,[this]{transfer(true);});
        connect(edit->addAction("Renommer..."),&QAction::triggered,this,[this]{rename();});
        auto *trash=edit->addAction("Mettre a la corbeille");trash->setShortcut(QKeySequence::Delete);connect(trash,&QAction::triggered,this,[this]{trashSelected();});
        connect(edit->addAction("Proprietes"),&QAction::triggered,this,[this]{properties();});
        auto *view=menuBar()->addMenu("Affichage");
        connect(view->addAction("Details"),&QAction::triggered,this,[this]{if(page())page()->views->setCurrentIndex(0);});
        connect(view->addAction("Grille"),&QAction::triggered,this,[this]{if(page()){page()->icons->setViewMode(QListView::IconMode);page()->views->setCurrentIndex(1);}});
        connect(view->addAction("Liste"),&QAction::triggered,this,[this]{if(page()){page()->icons->setViewMode(QListView::ListMode);page()->views->setCurrentIndex(1);}});
        auto *hidden=view->addAction("Afficher les fichiers caches");hidden->setCheckable(true);connect(hidden,&QAction::toggled,this,[this](bool show){if(page())page()->model->setFilter(QDir::AllEntries|QDir::NoDotAndDotDot|(show?QDir::Hidden:QDir::Filter(0)));});
        connect(view->addAction("Ajouter ce dossier aux favoris"),&QAction::triggered,this,[this]{if(!page())return;auto values=QSettings().value("places/favorites").toStringList();if(!values.contains(page()->directory))values.append(page()->directory);QSettings().setValue("places/favorites",values);loadPlaces();});
        Kyronext::addAboutMenu(this,"Kyronext Files","Explorateur local avec operations asynchrones KDE KIO.","Qt Core, Gui, Widgets ; KDE KIO sous LGPL selon composants");
        connect(places,&QListWidget::itemActivated,this,[this](QListWidgetItem *item){navigate(item->data(Qt::UserRole).toString());});
        connect(tabs,&QTabWidget::tabCloseRequested,this,[this](int i){closeTab(i);});connect(tabs,&QTabWidget::currentChanged,this,[this]{if(page())address->setText(page()->directory);});
        loadPlaces();addTab(QDir::homePath());
    }
private:
    QListWidget *places;QTabWidget *tabs;QLineEdit *address;
    FolderPage *page()const{return static_cast<FolderPage*>(tabs->currentWidget());}
    void loadPlaces() {
        places->clear();auto add=[this](const QString &label,const QString &path){if(!QFileInfo(path).isDir())return;auto *item=new QListWidgetItem(label,places);item->setData(Qt::UserRole,path);item->setToolTip(path);};
        add("Dossier personnel",QDir::homePath());
        for(auto pair:{qMakePair(QStandardPaths::DocumentsLocation,QString("Documents")),qMakePair(QStandardPaths::DownloadLocation,QString("Telechargements")),qMakePair(QStandardPaths::MusicLocation,QString("Musique")),qMakePair(QStandardPaths::PicturesLocation,QString("Images"))})add(pair.second,QStandardPaths::writableLocation(pair.first));
        for(const auto &value:QSettings().value("places/favorites").toStringList())add("Favori : "+QFileInfo(value).fileName(),value);
        for(const auto &volume:QStorageInfo::mountedVolumes())if(volume.isValid() && volume.isReady() && !volume.rootPath().startsWith("/proc") && !volume.rootPath().startsWith("/sys"))add("Volume : "+volume.rootPath(),volume.rootPath());
    }
    void addTab(const QString &path) {
        if(tabs->count()>=16){Kyronext::showError(this,"Fermez un onglet (16 maximum).");return;}
        auto *value=new FolderPage(path);tabs->addTab(value,QFileInfo(path).fileName().isEmpty()?"/":QFileInfo(path).fileName());tabs->setCurrentWidget(value);
        auto open=[this,value](const QModelIndex &index){const QString path=value->model->filePath(index);if(QFileInfo(path).isDir())navigate(path);else if(!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))Kyronext::showError(this,"Aucune application n'a pu ouvrir ce fichier.");};
        connect(value->details,&QTreeView::doubleClicked,this,open);connect(value->icons,&QListView::doubleClicked,this,open);address->setText(value->directory);
    }
    void closeTab(int index){if(index<0)return;auto *value=tabs->widget(index);tabs->removeTab(index);delete value;if(tabs->count()==0)addTab(QDir::homePath());}
    void navigate(const QString &path) {if(!page())return;if(!page()->navigate(path)){Kyronext::showError(this,"Dossier inaccessible ou absent.");return;}address->setText(page()->directory);tabs->setTabText(tabs->currentIndex(),QFileInfo(page()->directory).fileName().isEmpty()?"/":QFileInfo(page()->directory).fileName());}
    void run(KJob *job,const QString &error) {
        if(!job){Kyronext::showError(this,error);return;}
        statusBar()->showMessage("Operation KIO en cours...");
        connect(job,&KJob::result,this,[this](KJob *finished){if(finished->error())Kyronext::showError(this,finished->errorText());statusBar()->showMessage(finished->error()?"Operation interrompue.":"Operation terminee.",5000);});
    }
    void copyClipboard(bool cut) {
        if(!page())return;const auto selected=page()->selected();if(selected.isEmpty())return;
        auto *data=new QMimeData;data->setUrls({QUrl::fromLocalFile(selected)});data->setData("application/x-kde-cutselection",cut?"1":"0");QApplication::clipboard()->setMimeData(data);
        statusBar()->showMessage(cut?"Element marque pour deplacement : choisissez un dossier puis Coller.":"Element copie dans le presse-papiers.",5000);
    }
    void pasteClipboard() {
        if(!page())return;const auto *data=QApplication::clipboard()->mimeData();const auto urls=data->urls();
        if(urls.size()!=1 || !urls.first().isLocalFile()){Kyronext::showError(this,"Choisir un seul fichier ou dossier local a coller dans cette premiere version.");return;}
        const bool cut=data->data("application/x-kde-cutselection")=="1";const auto source=urls.first();
        const auto destination=QUrl::fromLocalFile(QDir(page()->directory).filePath(QFileInfo(source.toLocalFile()).fileName()));QString error;
        run(cut?moveLocal(source,destination,&error):copyLocal(source,destination,&error),error);
    }
    void transfer(bool move) {
        if(!page())return;const QString source=page()->selected();if(source.isEmpty())return;
        const QString destination=QFileDialog::getExistingDirectory(this,move?"Deplacer vers":"Copier vers",page()->directory);if(destination.isEmpty())return;
        const QUrl to=QUrl::fromLocalFile(QDir(destination).filePath(QFileInfo(source).fileName()));QString error;
        run(move?moveLocal(QUrl::fromLocalFile(source),to,&error):copyLocal(QUrl::fromLocalFile(source),to,&error),error);
    }
    bool validName(const QString &name){return !name.trimmed().isEmpty() && name!="." && name!=".." && !name.contains('/') && !name.contains('\\') && !name.contains('\n') && !name.contains('\r');}
    void rename() {
        if(!page())return;const QString source=page()->selected();if(source.isEmpty())return;bool ok;
        const QString name=QInputDialog::getText(this,"Renommer","Nouveau nom",QLineEdit::Normal,QFileInfo(source).fileName(),&ok);if(!ok)return;
        if(!validName(name)){Kyronext::showError(this,"Nom invalide.");return;}QString error;
        run(moveLocal(QUrl::fromLocalFile(source),QUrl::fromLocalFile(QFileInfo(source).dir().filePath(name)),&error),error);
    }
    void trashSelected() {
        if(!page())return;const QString source=page()->selected();if(source.isEmpty())return;
        QMessageBox question(QMessageBox::Question,"Mettre a la corbeille",{},QMessageBox::Yes|QMessageBox::Cancel,this);question.setTextFormat(Qt::PlainText);question.setDefaultButton(QMessageBox::Cancel);
        question.setText(QString("Placer %1 dans la corbeille ? Aucune suppression definitive n'est demandee.").arg(QFileInfo(source).fileName()));if(question.exec()!=QMessageBox::Yes)return;
        QString error;run(trashLocal(QUrl::fromLocalFile(source),&error),error);
    }
    void newFolder() {
        if(!page())return;bool ok;const QString name=QInputDialog::getText(this,"Nouveau dossier","Nom",QLineEdit::Normal,{},&ok);if(!ok)return;
        if(!validName(name)){Kyronext::showError(this,"Nom invalide.");return;}
        if(!QDir(page()->directory).mkdir(name))Kyronext::showError(this,"Creation impossible : nom existant ou permissions insuffisantes.");
    }
    void properties() {
        if(!page())return;const QFileInfo info(page()->selected());if(info.filePath().isEmpty())return;
        QMessageBox box(this);box.setWindowTitle("Proprietes du fichier");box.setTextFormat(Qt::PlainText);
        box.setText(QString("%1\n\nTaille : %2\nProprietaire : %3 (UID %4)\nLecture : %5 ; ecriture : %6 ; execution : %7\nDerniere modification : %8")
            .arg(info.absoluteFilePath(),QLocale().formattedDataSize(info.size()),info.owner(),QString::number(info.ownerId()),info.isReadable()?"oui":"non",info.isWritable()?"oui":"non",info.isExecutable()?"oui":"non",info.lastModified().toString(Qt::ISODate)));box.exec();
    }
};
int main(int argc,char **argv){QApplication app(argc,argv);Kyronext::configureApplication("Kyronext Files","0.1.0");Kyronext::applyTheme(app,Kyronext::paletteFromSettings());FilesWindow window;window.show();return app.exec();}
