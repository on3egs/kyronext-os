#pragma once
#include "policy.h"
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineCertificateError>
#include <QWebEnginePermission>
#include <QWebEngineFileSystemAccessRequest>
#include <QWebEngineDesktopMediaRequest>
#include <QWebEngineUrlScheme>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QBuffer>
#include <functional>

namespace Navigator {
inline void registerStartScheme() {QWebEngineUrlScheme scheme("kyronext");scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);scheme.setFlags(QWebEngineUrlScheme::SecureScheme|QWebEngineUrlScheme::LocalScheme);QWebEngineUrlScheme::registerScheme(scheme);}
class StartHandler final : public QWebEngineUrlSchemeHandler {
public:
    explicit StartHandler(QObject *parent=nullptr):QWebEngineUrlSchemeHandler(parent){}
    void requestStarted(QWebEngineUrlRequestJob *job) override {
        const auto url=job->requestUrl();if(url.host()!="start" || (url.path()!="/" && !url.path().isEmpty()) || job->requestMethod()!="GET"){job->fail(QWebEngineUrlRequestJob::UrlNotFound);return;}
        auto *buffer=new QBuffer(job);buffer->setData(startHtml(links()).toUtf8());buffer->open(QIODevice::ReadOnly);job->reply("text/html",buffer);
    }
};
inline void hardenProfile(QWebEngineProfile *profile) {
    profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    profile->setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory);profile->setPushServiceEnabled(false);
    auto *settings=profile->settings();settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls,false);settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,false);
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent,false);settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,false);settings->setAttribute(QWebEngineSettings::JavascriptCanPaste,false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled,false);settings->setAttribute(QWebEngineSettings::WebRTCPublicInterfacesOnly,true);settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled,false);
}
class SecurePage final : public QWebEnginePage {
    Q_OBJECT
public:
    explicit SecurePage(QWebEngineProfile *profile,QObject *parent=nullptr):QWebEnginePage(profile,parent) {
        connect(this,&QWebEnginePage::certificateError,this,[this](QWebEngineCertificateError error){error.rejectCertificate();emit securityError(tr("Certificat HTTPS refuse. Utilisez une connexion valide ; aucun contournement n'est propose."));});
        connect(this,&QWebEnginePage::permissionRequested,this,[this](QWebEnginePermission permission){
            using P=QWebEnginePermission::PermissionType;
            const auto type=permission.permissionType();
            const bool supported=type==P::Geolocation || type==P::MediaAudioCapture || type==P::MediaVideoCapture || type==P::MediaAudioVideoCapture;
            if(!supported || !webUrl(permission.origin(),true) || !permissionDecision || !permissionDecision(permission)){permission.deny();emit permissionDenied(permission.origin());}else permission.grant();
        });
        connect(this,&QWebEnginePage::fileSystemAccessRequested,this,[](QWebEngineFileSystemAccessRequest request){request.reject();});
        connect(this,&QWebEnginePage::desktopMediaRequested,this,[](QWebEngineDesktopMediaRequest request){request.cancel();});
    }
    std::function<bool(const QUrl&)> networkDecision;
    std::function<bool(const QWebEnginePermission&)> permissionDecision;
    void allowLocalFile(const QUrl &url) {approvedLocalFile=url;}
    bool canNavigate(const QUrl &url) const {
        if(url.scheme()=="kyronext")return url.host()=="start" && url.path().size()<=1;
        if(url==QUrl("about:blank"))return true;
        if(url.isLocalFile())return url==approvedLocalFile && url.host().isEmpty() && QFileInfo(url.toLocalFile()).isFile();
        return webUrl(url);
    }
signals:
    void securityError(const QString &message);void permissionDenied(const QUrl &origin);
protected:
    bool acceptNavigationRequest(const QUrl &url,NavigationType,bool mainFrame) override {
        if(!canNavigate(url)){emit securityError(tr("Adresse ou protocole refuse."));return false;}
        if(mainFrame && webUrl(url) && (!networkDecision || !networkDecision(url)))return false;
        return true;
    }
private:
    QUrl approvedLocalFile;
};
}
