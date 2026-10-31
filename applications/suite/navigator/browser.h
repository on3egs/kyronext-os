#pragma once
#include "secure_page.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QLabel>
#include <QDockWidget>
#include <QListWidget>
#include <QWebEngineView>
#include <QWebEngineDownloadRequest>
#include <QNetworkCookie>
#include <QPointer>
#include <QMutex>
#include <memory>

namespace Navigator {
struct CookiePolicy {QMutex mutex;QSet<QString> blockedHosts;};
class BrowserWindow final : public QMainWindow {
public:
    explicit BrowserWindow(bool privateMode=false);
    ~BrowserWindow() override;
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    QWebEngineView *addTab(const QUrl &url=QUrl("kyronext://start/"));
    QWebEngineView *current() const;
    bool approveNetwork(const QUrl &url);
    bool approvePermission(const QWebEnginePermission &permission);
    void navigate(const QString &text);
    void showStored(const QString &title,const QJsonArray &items);
    void addBookmark();void clearHistory();void configureHome();void showCookies();void revokePermissions();
    void downloadRequested(QWebEngineDownloadRequest *request);
    void recordVisit(QWebEngineView *view);
    void saveStore(const QString &name,const QJsonArray &items);
    QString storePath(const QString &name) const;
    QWebEngineProfile *profile=nullptr;
    QTabWidget *tabs=nullptr;
    QLineEdit *addressBar=nullptr;
    QLabel *connection=nullptr;
    QDockWidget *downloadsDock=nullptr;
    QListWidget *downloadsList=nullptr;
    QHash<QListWidgetItem*,QPointer<QWebEngineDownloadRequest>> downloads;
    QList<QNetworkCookie> cookies;
    std::shared_ptr<CookiePolicy> cookiePolicy=std::make_shared<CookiePolicy>();
    QJsonArray bookmarks,history;
    bool privateMode=false,networkAllowed=false,historyEnabled=false;
};
}
