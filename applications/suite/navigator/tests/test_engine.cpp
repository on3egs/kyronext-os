#include "secure_page.h"
#include "browser.h"
#include <kyronext/ui.h>
#include <QtTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QTcpServer>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QProcess>
#include <QWebEngineView>
#include <QSettings>

class TlsServer final : public QTcpServer {
public:
    QSslCertificate certificate;QSslKey key;
protected:
    void incomingConnection(qintptr socket) override {auto *connection=new QSslSocket(this);if(!connection->setSocketDescriptor(socket)){delete connection;return;}connection->setLocalCertificate(certificate);connection->setPrivateKey(key);connect(connection,&QSslSocket::readyRead,connection,[connection]{connection->readAll();connection->write("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<title>Untrusted TLS</title>");connection->disconnectFromHost();});connect(connection,&QSslSocket::disconnected,connection,&QObject::deleteLater);connection->startServerEncryption();}
};
class EngineTests final : public QObject {
    Q_OBJECT
private slots:
    void localNavigationAndPrivateProfile() {
        QWebEngineProfile profile;Navigator::hardenProfile(&profile);QVERIFY(profile.isOffTheRecord());QCOMPARE(profile.persistentCookiesPolicy(),QWebEngineProfile::NoPersistentCookies);QVERIFY(!profile.settings()->testAttribute(QWebEngineSettings::AllowRunningInsecureContent));
        profile.installUrlSchemeHandler("kyronext",new Navigator::StartHandler(&profile));
        QTemporaryDir dir;const auto path=dir.filePath("local.html");QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));file.write("<!doctype html><title>Kyronext local test</title><p>Local page</p>");file.close();
        Navigator::SecurePage page(&profile);const auto local=QUrl::fromLocalFile(path);QVERIFY(!page.canNavigate(local));page.allowLocalFile(local);QVERIFY(page.canNavigate(local));QSignalSpy loaded(&page,&QWebEnginePage::loadFinished);page.load(local);QTRY_VERIFY_WITH_TIMEOUT(!loaded.isEmpty(),20000);QVERIFY(loaded.last()[0].toBool());QCOMPARE(page.title(),QString("Kyronext local test"));
        loaded.clear();page.load(QUrl("kyronext://start/"));QTRY_VERIFY_WITH_TIMEOUT(!loaded.isEmpty(),20000);QVERIFY(loaded.last()[0].toBool());QCOMPARE(page.title(),QString("Kyronext Start"));
        QWebEngineView view;view.setPage(new Navigator::SecurePage(&profile,&view));view.resize(1000,700);view.show();QSignalSpy screenshotLoaded(view.page(),&QWebEnginePage::loadFinished);view.load(QUrl("kyronext://start/"));QTRY_VERIFY_WITH_TIMEOUT(!screenshotLoaded.isEmpty(),20000);QTest::qWait(1000);const auto rendered=view.grab();QVERIFY(rendered.save("navigator-start-preview.png"));QVERIFY2(rendered.toImage().pixelColor(20,20).lightness()<128,"Le moteur a charge le DOM mais la capture ne montre pas la page sombre.");
    }
    void refusesPermissionsByDefault() {
        QWebEngineProfile profile;Navigator::hardenProfile(&profile);Navigator::SecurePage page(&profile);
        const auto origin=QUrl("https://example.invalid");auto permission=profile.queryPermission(origin,QWebEnginePermission::PermissionType::Geolocation);QVERIFY(permission.isValid());
        QSignalSpy denied(&page,&Navigator::SecurePage::permissionDenied);
        QVERIFY(QMetaObject::invokeMethod(&page,"permissionRequested",Qt::DirectConnection,Q_ARG(QWebEnginePermission,permission)));QCOMPARE(denied.size(),1);QCOMPARE(permission.state(),QWebEnginePermission::State::Denied);
    }
    void refusesUntrustedCertificate() {
        QTemporaryDir directory;const auto keyPath=directory.filePath("key.pem"),certPath=directory.filePath("cert.pem");
        QProcess openssl;openssl.start("openssl",{"req","-x509","-newkey","rsa:2048","-nodes","-keyout",keyPath,"-out",certPath,"-days","1","-subj","/CN=localhost","-addext","subjectAltName=DNS:localhost,IP:127.0.0.1"});QVERIFY(openssl.waitForFinished(15000));QCOMPARE(openssl.exitCode(),0);
        QFile cert(certPath),key(keyPath);QVERIFY(cert.open(QIODevice::ReadOnly));QVERIFY(key.open(QIODevice::ReadOnly));TlsServer server;server.certificate=QSslCertificate(cert.readAll());server.key=QSslKey(key.readAll(),QSsl::Rsa);QVERIFY(server.listen(QHostAddress::LocalHost));
        QWebEngineProfile profile;Navigator::hardenProfile(&profile);Navigator::SecurePage page(&profile);page.networkDecision=[](const QUrl &url){return url.host()=="127.0.0.1";};QSignalSpy errors(&page,&Navigator::SecurePage::securityError),finished(&page,&QWebEnginePage::loadFinished);page.load(QUrl("https://127.0.0.1:"+QString::number(server.serverPort())+"/"));QTRY_VERIFY_WITH_TIMEOUT(!finished.isEmpty(),20000);QVERIFY(!finished.last()[0].toBool());QVERIFY(!errors.isEmpty());
    }
    void fullWindowAndTabs() {
        QTemporaryDir data;QVERIFY(data.isValid());const auto previousData=qgetenv("XDG_DATA_HOME");qputenv("XDG_DATA_HOME",data.path().toUtf8());
        QCoreApplication::setOrganizationName("KyronextTests");QCoreApplication::setApplicationName("NavigatorUi");QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,data.path());
        Kyronext::applyTheme(*qApp,Kyronext::Palette::Red);
        auto *window=new Navigator::BrowserWindow;window->resize(1050,720);window->show();auto *tabs=window->findChild<QTabWidget*>();QVERIFY(tabs);QCOMPARE(tabs->count(),1);auto *page=qobject_cast<QWebEngineView*>(tabs->currentWidget());QVERIFY(page);QTRY_COMPARE_WITH_TIMEOUT(page->title(),QString("Kyronext Start"),20000);QTest::qWait(500);
        QVERIFY(window->grab().save("navigator-window-preview.png"));QTest::keyClick(window,Qt::Key_T,Qt::ControlModifier);QTRY_COMPARE(tabs->count(),2);QTest::keyClick(window,Qt::Key_W,Qt::ControlModifier);QTRY_COMPARE(tabs->count(),1);
        window->close();QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);if(previousData.isNull())qunsetenv("XDG_DATA_HOME");else qputenv("XDG_DATA_HOME",previousData);
    }
};
int main(int argc,char **argv){QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);Navigator::registerStartScheme();QApplication app(argc,argv);if(Navigator::unsafeEnvironment()){qCritical("Unsafe engine environment: no sandbox bypass permitted for tests");return 2;}EngineTests tests;return QTest::qExec(&tests,argc,argv);}
#include "test_engine.moc"
