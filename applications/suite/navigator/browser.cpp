#include "browser.h"
#include <kyronext/ui.h>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QShortcut>
#include <QCloseEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QWebEngineCookieStore>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineHistory>
#include <QAuthenticator>
#include <QDateTime>
#include <QMutexLocker>
#include <QProcess>

namespace Navigator {
static bool question(QWidget *parent,const QString &title,const QString &text) {QMessageBox dialog(QMessageBox::Question,title,text,QMessageBox::Yes|QMessageBox::No,parent);dialog.setTextFormat(Qt::PlainText);dialog.setDefaultButton(QMessageBox::No);return dialog.exec()==QMessageBox::Yes;}
static bool sameCookie(const QNetworkCookie &a,const QNetworkCookie &b){return a.name()==b.name() && a.domain()==b.domain() && a.path()==b.path();}
static bool cookieForHost(const QNetworkCookie &cookie,const QString &host){QString domain=cookie.domain().toLower();while(domain.startsWith('.'))domain.remove(0,1);return !domain.isEmpty() && (host==domain || host.endsWith('.'+domain));}
BrowserWindow::BrowserWindow(bool privateMode):privateMode(privateMode) {
    setAttribute(Qt::WA_DeleteOnClose);resize(1100,760);setMinimumSize(640,440);
    setWindowTitle(privateMode?tr("Kyronext Navigator - session privee experimentale"):tr("Kyronext Navigator - experimental"));
    profile=new QWebEngineProfile(this);hardenProfile(profile);profile->installUrlSchemeHandler("kyronext",new StartHandler(profile));
    const auto policy=cookiePolicy;
    profile->cookieStore()->setCookieFilter([policy](const QWebEngineCookieStore::FilterRequest &request){QMutexLocker lock(&policy->mutex);return !request.thirdParty && !policy->blockedHosts.contains(request.origin.host().toLower());});
    connect(profile->cookieStore(),&QWebEngineCookieStore::cookieAdded,this,[this](const QNetworkCookie &cookie){for(auto &old:cookies)if(sameCookie(old,cookie)){old=cookie;return;}if(cookies.size()<1000)cookies.append(cookie);});
    connect(profile->cookieStore(),&QWebEngineCookieStore::cookieRemoved,this,[this](const QNetworkCookie &cookie){for(int i=cookies.size()-1;i>=0;--i)if(sameCookie(cookies[i],cookie))cookies.removeAt(i);});
    connect(profile,&QWebEngineProfile::downloadRequested,this,&BrowserWindow::downloadRequested);
    bookmarks=readArray(storePath("bookmarks.json"));if(!privateMode)history=readArray(storePath("history.json"));
    historyEnabled=!privateMode && QSettings().value("privacy/history",false).toBool();
    auto *body=new QWidget(this);auto *layout=new QVBoxLayout(body);layout->setContentsMargins(0,0,0,0);
    auto *notice=new QLabel(tr("EXPERIMENTAL - Qt WebEngine n'est pas couvert par le support securite Debian 13. Utilisez Firefox ESR pour le Web quotidien."),body);notice->setWordWrap(true);notice->setStyleSheet("background:#422328;color:#fff;padding:8px;");layout->addWidget(notice);
    tabs=new QTabWidget(body);tabs->setDocumentMode(true);tabs->setTabsClosable(true);tabs->setMovable(true);tabs->setAccessibleName(tr("Onglets Web"));layout->addWidget(tabs,1);setCentralWidget(body);
    connect(tabs,&QTabWidget::tabCloseRequested,this,[this](int index){auto *widget=tabs->widget(index);tabs->removeTab(index);delete widget;if(tabs->count()==0)addTab();});
    connect(tabs,&QTabWidget::currentChanged,this,[this]{if(auto *view=current()){addressBar->setText(view->url().toDisplayString(QUrl::RemoveUserInfo));connection->setText(view->url().scheme()=="https"?tr("HTTPS"):view->url().scheme()=="http"?tr("HTTP non chiffre"):tr("Page locale"));}});
    auto *bar=addToolBar(tr("Navigation"));bar->setMovable(false);
    bar->addAction(tr("Precedent"),this,[this]{if(current())current()->back();});bar->addAction(tr("Suivant"),this,[this]{if(current())current()->forward();});bar->addAction(tr("Recharger"),this,[this]{if(current())current()->reload();});bar->addAction(tr("Start"),this,[this]{if(current())current()->load(QUrl("kyronext://start/"));});
    addressBar=new QLineEdit(bar);addressBar->setPlaceholderText(tr("Adresse ou recherche - Entree"));addressBar->setAccessibleName(tr("Adresse Web ou recherche"));bar->addWidget(addressBar);connect(addressBar,&QLineEdit::returnPressed,this,[this]{navigate(addressBar->text());});
    bar->addAction(tr("+ Onglet"),this,[this]{addTab();});connection=new QLabel(tr("Page locale"),bar);bar->addWidget(connection);
    downloadsDock=new QDockWidget(tr("Telechargements"),this);auto *downloadBody=new QWidget(downloadsDock);auto *downloadLayout=new QVBoxLayout(downloadBody);downloadsList=new QListWidget(downloadBody);downloadsList->setAccessibleName(tr("Etat des telechargements"));downloadLayout->addWidget(downloadsList);auto *cancel=new QPushButton(tr("Annuler le telechargement selectionne"),downloadBody);downloadLayout->addWidget(cancel);downloadsDock->setWidget(downloadBody);addDockWidget(Qt::BottomDockWidgetArea,downloadsDock);downloadsDock->hide();connect(cancel,&QPushButton::clicked,this,[this]{auto request=downloads.value(downloadsList->currentItem());if(request)request->cancel();});
    auto *file=menuBar()->addMenu(tr("&Fichier"));file->addAction(tr("Nouvel onglet"),QKeySequence::AddTab,this,[this]{addTab();});file->addAction(tr("Nouvelle session privee"),this,[]{(new BrowserWindow(true))->show();});
    file->addAction(tr("Ouvrir une page HTML locale..."),QKeySequence::Open,this,[this]{const QString path=QFileDialog::getOpenFileName(this,tr("Page locale"),{},tr("HTML (*.html *.htm);;Tous fichiers (*)"));if(!path.isEmpty())addTab(QUrl::fromLocalFile(path));});
    file->addAction(tr("Ouvrir cette adresse dans Firefox ESR"),this,[this]{const auto program=QStandardPaths::findExecutable("firefox-esr");if(program.isEmpty() || !current() || !webUrl(current()->url())){QMessageBox::information(this,tr("Firefox ESR"),tr("Installez Firefox ESR et choisissez une adresse Web pour utiliser cette solution de secours."));return;}if(!QProcess::startDetached(program,{current()->url().toString(QUrl::FullyEncoded)}))QMessageBox::warning(this,tr("Firefox ESR"),tr("Impossible de lancer Firefox ESR."));});
    file->addSeparator();file->addAction(tr("Quitter cette fenetre"),QKeySequence::Quit,this,&QWidget::close);
    auto *favorites=menuBar()->addMenu(tr("&Favoris"));favorites->addAction(tr("Ajouter cette page..."),QKeySequence(Qt::CTRL|Qt::Key_D),this,&BrowserWindow::addBookmark);favorites->addAction(tr("Afficher les favoris"),this,[this]{showStored(tr("Favoris"),bookmarks);});
    auto *privacy=menuBar()->addMenu(tr("&Confidentialite"));auto *remember=privacy->addAction(tr("Conserver l'historique local"));remember->setCheckable(true);remember->setChecked(historyEnabled);remember->setEnabled(!privateMode);connect(remember,&QAction::toggled,this,[this](bool enabled){historyEnabled=enabled;QSettings().setValue("privacy/history",enabled);});
    auto *showHistory=privacy->addAction(tr("Afficher l'historique"),this,[this]{showStored(tr("Historique sans requetes ni fragments"),history);});showHistory->setEnabled(!privateMode);privacy->addAction(tr("Effacer l'historique enregistre"),this,&BrowserWindow::clearHistory);
    privacy->addAction(tr("Cookies de ce site..."),this,&BrowserWindow::showCookies);privacy->addAction(tr("Reinitialiser les permissions de ce site"),this,&BrowserWindow::revokePermissions);
    auto *view=menuBar()->addMenu(tr("&Affichage"));view->addAction(tr("Zoom +"),QKeySequence::ZoomIn,this,[this]{if(current())current()->setZoomFactor(qMin(3.0,current()->zoomFactor()+.1));});view->addAction(tr("Zoom -"),QKeySequence::ZoomOut,this,[this]{if(current())current()->setZoomFactor(qMax(.3,current()->zoomFactor()-.1));});view->addAction(tr("Zoom 100 %"),QKeySequence(Qt::CTRL|Qt::Key_0),this,[this]{if(current())current()->setZoomFactor(1);});view->addAction(downloadsDock->toggleViewAction());view->addAction(tr("Choisir la page d'accueil..."),this,&BrowserWindow::configureHome);
    view->addAction(tr("Plein ecran"),QKeySequence(Qt::Key_F11),this,[this]{if(isFullScreen())showNormal();else showFullScreen();});
    auto *location=new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_L),this);connect(location,&QShortcut::activated,this,[this]{addressBar->setFocus();addressBar->selectAll();});auto *closeTab=new QShortcut(QKeySequence::Close,this);connect(closeTab,&QShortcut::activated,this,[this]{emit tabs->tabCloseRequested(tabs->currentIndex());});
    Kyronext::addAboutMenu(this,"Kyronext Navigator",tr("Prototype de navigateur, moteur Qt WebEngine/Chromium. Ne convient pas au Web sensible ou quotidien sur Debian 13."),"Qt 6 Widgets/WebEngine, Chromium et bibliotheques tierces : licences correspondantes Debian. Creations Kyronext : droits reserves.");
    QUrl home=QUrl("kyronext://start/");if(!privateMode){const QUrl saved(QSettings().value("home/url").toString());if(webUrl(saved))home=saved;}addTab(home);
}
BrowserWindow::~BrowserWindow(){delete tabs;tabs=nullptr;delete profile;profile=nullptr;}
QWebEngineView *BrowserWindow::current() const{return tabs?qobject_cast<QWebEngineView*>(tabs->currentWidget()):nullptr;}
QWebEngineView *BrowserWindow::addTab(const QUrl &url) {
    auto *view=new QWebEngineView(tabs);auto *page=new SecurePage(profile,view);if(url.isLocalFile())page->allowLocalFile(url);view->setPage(page);page->networkDecision=[this](const QUrl &u){return approveNetwork(u);};page->permissionDecision=[this](const QWebEnginePermission &p){return approvePermission(p);};
    connect(page,&SecurePage::securityError,this,[this](const QString &message){statusBar()->showMessage(message);});
    connect(view,&QWebEngineView::titleChanged,this,[this,view](const QString &title){const int index=tabs->indexOf(view);if(index>=0)tabs->setTabText(index,title.left(40));});
    connect(view,&QWebEngineView::urlChanged,this,[this,view](const QUrl &u){if(view==current()){addressBar->setText(u.toDisplayString(QUrl::RemoveUserInfo));connection->setText(u.scheme()=="https"?tr("HTTPS"):u.scheme()=="http"?tr("HTTP non chiffre"):tr("Page locale"));}});
    connect(view,&QWebEngineView::loadFinished,this,[this,view](bool ok){if(ok)recordVisit(view);else statusBar()->showMessage(tr("Chargement interrompu ou impossible. Verifiez la connexion, l'adresse et le certificat."));});
    connect(page,&QWebEnginePage::newWindowRequested,this,[this](QWebEngineNewWindowRequest &request){if(request.isUserInitiated())request.openIn(addTab(QUrl("about:blank"))->page());else statusBar()->showMessage(tr("Fenetre automatique bloquee."));});
    connect(page,&QWebEnginePage::authenticationRequired,this,[this](const QUrl &url,QAuthenticator *auth){if(!webUrl(url,true)){statusBar()->showMessage(tr("Authentification refusee sur une connexion non HTTPS."));return;}QDialog dialog(this);dialog.setWindowTitle(tr("Authentification - ")+origin(url));auto *layout=new QVBoxLayout(&dialog);auto *label=new QLabel(tr("Identifiants utilises uniquement pour cette session. Aucun enregistrement par Navigator."),&dialog);label->setWordWrap(true);layout->addWidget(label);auto *user=new QLineEdit(&dialog);user->setPlaceholderText(tr("Identifiant"));auto *password=new QLineEdit(&dialog);password->setPlaceholderText(tr("Mot de passe"));password->setEchoMode(QLineEdit::Password);layout->addWidget(user);layout->addWidget(password);auto *buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);if(dialog.exec()==QDialog::Accepted){auth->setUser(user->text());auth->setPassword(password->text());password->clear();}});
    const int index=tabs->addTab(view,privateMode?tr("Prive"):tr("Nouvel onglet"));tabs->setCurrentIndex(index);view->load(url);return view;
}
bool BrowserWindow::approveNetwork(const QUrl &url) {
    if(networkAllowed)return true;
    networkAllowed=question(this,tr("Navigation Web experimentale"),tr("Qt WebEngine dans Debian 13 ne beneficie pas du support securite Debian. Firefox ESR est recommande pour le Web quotidien. Autoriser la navigation Web dans cette fenetre pour cette session ?\n\nDestination : ")+origin(url));return networkAllowed;
}
bool BrowserWindow::approvePermission(const QWebEnginePermission &permission) {
    using P=QWebEnginePermission::PermissionType;QString purpose;
    switch(permission.permissionType()){case P::Geolocation:purpose=tr("votre position");break;case P::MediaAudioCapture:purpose=tr("le microphone");break;case P::MediaVideoCapture:purpose=tr("la camera");break;case P::MediaAudioVideoCapture:purpose=tr("la camera et le microphone");break;default:return false;}
    return question(this,tr("Permission du site"),origin(permission.origin())+tr(" demande l'acces a ")+purpose+tr(". Autoriser pour cette session ?"));
}
void BrowserWindow::navigate(const QString &text) {const QUrl url=address(text);if(url.isEmpty()){statusBar()->showMessage(tr("Adresse refusee : utilisez HTTP/HTTPS ou une recherche."));return;}if(current())current()->load(url);}
QString BrowserWindow::storePath(const QString &name) const {const auto root=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);QDir().mkpath(root);return root+'/'+name;}
void BrowserWindow::saveStore(const QString &name,const QJsonArray &items) {if(!writeArray(storePath(name),items))QMessageBox::warning(this,tr("Enregistrement impossible"),tr("Les donnees locales n'ont pas ete enregistrees. Verifiez les permissions et l'espace disque."));}
void BrowserWindow::addBookmark() {
    if(!current() || !webUrl(current()->url())){statusBar()->showMessage(tr("Seules les adresses Web peuvent etre ajoutees aux favoris."));return;}
    if(bookmarks.size()>=1000){statusBar()->showMessage(tr("Limite de 1000 favoris atteinte."));return;}
    bool accepted=false;const auto title=QInputDialog::getText(this,tr("Ajouter un favori"),tr("Titre"),QLineEdit::Normal,current()->title().left(200),&accepted);if(!accepted || title.trimmed().isEmpty())return;
    bookmarks.append(QJsonObject{{"title",title.left(200)},{"url",current()->url().toString(QUrl::FullyEncoded)}});saveStore("bookmarks.json",bookmarks);
}
void BrowserWindow::showStored(const QString &title,const QJsonArray &items) {
    QDialog dialog(this);dialog.setWindowTitle(title);dialog.resize(700,440);auto *layout=new QVBoxLayout(&dialog);auto *list=new QListWidget(&dialog);layout->addWidget(list);
    for(const auto &entry:items){const auto object=entry.toObject();const QUrl url(object.value("url").toString());if(!webUrl(url))continue;auto *item=new QListWidgetItem(object.value("title").toString().left(200)+"\n"+url.toDisplayString(QUrl::RemoveUserInfo),list);item->setData(Qt::UserRole,url);}
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Open|QDialogButtonBox::Close,&dialog);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);auto open=[this,&dialog,list]{if(auto *item=list->currentItem()){addTab(item->data(Qt::UserRole).toUrl());dialog.accept();}};connect(buttons,&QDialogButtonBox::accepted,&dialog,open);connect(list,&QListWidget::itemDoubleClicked,&dialog,[open]{open();});dialog.exec();
}
void BrowserWindow::recordVisit(QWebEngineView *view) {
    if(privateMode || !historyEnabled)return;const auto url=historyUrl(view->url());if(url.isEmpty())return;
    history.append(QJsonObject{{"url",url.toString(QUrl::FullyEncoded)},{"title",view->title().left(200)},{"time",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}});while(history.size()>500)history.removeFirst();saveStore("history.json",history);
}
void BrowserWindow::clearHistory(){if(!question(this,tr("Effacer l'historique"),tr("Effacer l'historique enregistre par Navigator ?")))return;history={};saveStore("history.json",history);for(int i=0;i<tabs->count();++i)qobject_cast<QWebEngineView*>(tabs->widget(i))->history()->clear();}
void BrowserWindow::configureHome(){bool ok=false;const auto text=QInputDialog::getText(this,tr("Page d'accueil"),tr("Adresse HTTP/HTTPS, ou kyronext://start/ pour la page locale"),QLineEdit::Normal,QSettings().value("home/url","kyronext://start/").toString(),&ok);if(!ok)return;const QUrl url(text.trimmed());if(webUrl(url) || url==QUrl("kyronext://start/"))QSettings().setValue("home/url",url.toString());else QMessageBox::warning(this,tr("Adresse invalide"),tr("La page d'accueil n'a pas ete modifiee."));}
void BrowserWindow::showCookies(){
    if(!current() || !webUrl(current()->url()))return;const auto host=current()->url().host().toLower();QDialog dialog(this);dialog.setWindowTitle(tr("Cookies - ")+host);dialog.resize(560,380);auto *layout=new QVBoxLayout(&dialog);auto *description=new QLabel(tr("Cookies en memoire uniquement. Les valeurs ne sont pas affichees. Les cookies tiers sont bloques."),&dialog);description->setWordWrap(true);layout->addWidget(description);auto *list=new QListWidget(&dialog);layout->addWidget(list);QList<QNetworkCookie> selected;for(const auto &cookie:cookies)if(cookieForHost(cookie,host)){selected.append(cookie);list->addItem(QString::fromUtf8(cookie.name())+" - "+cookie.domain());}
    auto *block=new QCheckBox(tr("Bloquer les cookies de ce domaine pendant cette session"),&dialog);{QMutexLocker guard(&cookiePolicy->mutex);block->setChecked(cookiePolicy->blockedHosts.contains(host));}layout->addWidget(block);connect(block,&QCheckBox::toggled,&dialog,[this,host](bool enabled){QMutexLocker guard(&cookiePolicy->mutex);if(enabled)cookiePolicy->blockedHosts.insert(host);else cookiePolicy->blockedHosts.remove(host);});auto *remove=new QPushButton(tr("Effacer les cookies listes"),&dialog);layout->addWidget(remove);connect(remove,&QPushButton::clicked,&dialog,[this,list,selected]{for(const auto &cookie:selected)profile->cookieStore()->deleteCookie(cookie);list->clear();});auto *close=new QDialogButtonBox(QDialogButtonBox::Close,&dialog);layout->addWidget(close);connect(close,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
}
void BrowserWindow::revokePermissions(){if(!current() || !webUrl(current()->url()))return;const auto url=QUrl(origin(current()->url()));if(!question(this,tr("Permissions du site"),tr("Reinitialiser les autorisations de ")+origin(url)+" ?"))return;for(auto permission:profile->listPermissionsForOrigin(url))permission.reset();statusBar()->showMessage(tr("Permissions reinitialisees pour cette session."));}
void BrowserWindow::downloadRequested(QWebEngineDownloadRequest *request) {
    QString suggested=QFileInfo(request->suggestedFileName()).fileName();if(suggested.isEmpty())suggested="telechargement";
    const auto path=QFileDialog::getSaveFileName(this,tr("Enregistrer le telechargement"),QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+'/'+suggested);
    if(path.isEmpty()){request->cancel();return;}const QFileInfo file(path);request->setDownloadDirectory(file.absolutePath());request->setDownloadFileName(file.fileName());
    auto *item=new QListWidgetItem(file.fileName(),downloadsList);downloads.insert(item,request);auto update=[item,request]{QString state;switch(request->state()){case QWebEngineDownloadRequest::DownloadCompleted:state="termine";break;case QWebEngineDownloadRequest::DownloadCancelled:state="annule";break;case QWebEngineDownloadRequest::DownloadInterrupted:state="erreur : "+request->interruptReasonString();break;default:state=QString::number(request->receivedBytes())+" / "+QString::number(request->totalBytes())+" octets";}item->setText(request->downloadFileName()+" - "+state);};connect(request,&QWebEngineDownloadRequest::receivedBytesChanged,this,update);connect(request,&QWebEngineDownloadRequest::stateChanged,this,update);request->accept();downloadsDock->show();update();
}
void BrowserWindow::closeEvent(QCloseEvent *event){int active=0;for(const auto &request:downloads)if(request && request->state()==QWebEngineDownloadRequest::DownloadInProgress)++active;if(active && !question(this,tr("Telechargements actifs"),tr("Fermer cette fenetre et interrompre ses telechargements ?"))){event->ignore();return;}event->accept();}
}
