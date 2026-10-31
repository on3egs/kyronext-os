#pragma once
#include <QUrl>
#include <QUrlQuery>
#include <QStringList>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QRegularExpression>
#include <QFileInfo>
#include <QSet>

namespace Navigator {
inline bool webUrl(const QUrl &url,bool httpsOnly=false) {
    const QString scheme=url.scheme().toLower();
    if(!url.isValid() || url.host().isEmpty() || url.authority(QUrl::FullyEncoded).contains('@') || !url.userName().isEmpty() || !url.password().isEmpty())return false;
    if(scheme!="https" && (httpsOnly || scheme!="http"))return false;
    const QString encoded=url.toString(QUrl::FullyEncoded);
    const auto decoded=QUrl::fromPercentEncoding(encoded.toUtf8());
    for(QChar c:decoded)if(c.category()==QChar::Other_Control)return false;
    return !encoded.contains('\\') && url.port(443)>0;
}
inline QUrl address(const QString &input) {
    const QString text=input.trimmed();if(text.isEmpty())return QUrl("kyronext://start/");
    if(text.contains(QRegularExpression("[\\x00-\\x1f\\x7f]")))return {};
    if(text.startsWith("kyronext://start")) {const QUrl u(text);return u.host()=="start" && u.path().size()<=1?QUrl("kyronext://start/"):QUrl();}
    if(text.contains("://")){const QUrl u(text,QUrl::StrictMode);return webUrl(u)?u:QUrl();}
    // Schemes such as javascript:, data: and file: are never entered as code.
    if(text.contains(':'))return {};
    if(!text.contains(QRegularExpression("\\s")) && text.contains('.')) {const QUrl u("https://"+text,QUrl::StrictMode);if(webUrl(u))return u;}
    QUrl search("https://duckduckgo.com/");QUrlQuery query;query.addQueryItem("q",text);search.setQuery(query);return search;
}
inline QString origin(const QUrl &url) {QUrl o;o.setScheme(url.scheme());o.setHost(url.host());o.setPort(url.port());return o.toString(QUrl::FullyEncoded);}
inline QUrl historyUrl(QUrl url) {url.setQuery(QString());url.setFragment(QString());return webUrl(url)?url:QUrl();}
inline bool unsafeEnvironment() {
    const auto disabled=qgetenv("QTWEBENGINE_DISABLE_SANDBOX");
    return (!disabled.isEmpty() && disabled!="0") || !qgetenv("QTWEBENGINE_CHROMIUM_FLAGS").isEmpty() || !qgetenv("QTWEBENGINE_REMOTE_DEBUGGING").isEmpty() || !qgetenv("QTWEBENGINEPROCESS_PATH").isEmpty();
}
inline QJsonObject readObject(const QString &path,QStringList *errors=nullptr) {
    QFile f(path);if(!f.open(QIODevice::ReadOnly)){if(errors)errors->append("Configuration inaccessible : "+path);return {};}
    if(f.size()>1024*1024){if(errors)errors->append("Configuration trop volumineuse.");return {};}
    QJsonParseError error;const auto document=QJsonDocument::fromJson(f.readAll(),&error);
    if(error.error!=QJsonParseError::NoError || !document.isObject()){if(errors)errors->append("Configuration JSON invalide.");return {};}
    return document.object();
}
struct Link {QString id,label;QUrl url;bool visible=true;QString status;};
inline QList<Link> links(const QString &systemPath="/etc/kyronext/links.json",QString userPath={},QStringList *errors=nullptr) {
    if(userPath.isEmpty()){QString root=qEnvironmentVariable("XDG_CONFIG_HOME");if(!QDir::isAbsolutePath(root))root=QDir::homePath()+"/.config";userPath=root+"/kyronext/links.json";}
    QList<Link> result;const auto system=readObject(systemPath,errors);if(system.value("schema_version").toInt()!=1)return result;
    for(const auto &entry:system.value("links").toArray()) {
        const auto e=entry.toObject();const QUrl url(e.value("url").toString(),QUrl::StrictMode);const auto id=e.value("id").toString(),label=e.value("label").toString();
        if(!webUrl(url,true) || !QRegularExpression("^[a-z0-9-]{1,64}$").match(id).hasMatch() || label.isEmpty() || label.size()>100)continue;
        bool duplicate=false;for(const auto &link:result)if(link.id==id)duplicate=true;if(duplicate)continue;
        result.append({id,label,url,e.value("visible").toBool(true),e.value("verification").toObject().value("status").toString()});
    }
    if(!QFile::exists(userPath))return result;
    const auto user=readObject(userPath,errors);if(user.value("schema_version").toInt()!=1)return result;
    QSet<QString> seen;
    for(const auto &entry:user.value("links").toArray()) {
        const auto e=entry.toObject();const auto id=e.value("id").toString();if(seen.contains(id)){if(errors)errors->append("Identifiant utilisateur duplique.");continue;}seen.insert(id);
        for(auto &link:result)if(link.id==id) {
            if(e.contains("url")){const QUrl u(e.value("url").toString(),QUrl::StrictMode);if(webUrl(u,true)){link.url=u;link.status="user";}else if(errors)errors->append("Adresse utilisateur rejetee : "+link.id);}
            if(e.contains("visible") && e.value("visible").isBool())link.visible=e.value("visible").toBool();
            const auto label=e.value("label").toString();if(!label.isEmpty() && label.size()<=100)link.label=label;
        }
    }
    return result;
}
inline bool writeArray(const QString &path,const QJsonArray &array) {
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return false;
    if(!file.setPermissions(QFile::ReadOwner|QFile::WriteOwner))return false;
    const auto bytes=QJsonDocument(array).toJson();return file.write(bytes)==bytes.size() && file.commit();
}
inline QJsonArray readArray(const QString &path) {QFile f(path);if(!f.open(QIODevice::ReadOnly) || f.size()>1024*1024)return {};const auto doc=QJsonDocument::fromJson(f.readAll());return doc.isArray()?doc.array():QJsonArray();}
inline QString startHtml(const QList<Link> &entries) {
    QString cards;
    for(const auto &link:entries)if(link.visible)cards+="<a href=\""+link.url.toString(QUrl::FullyEncoded).toHtmlEscaped()+"\"><strong>"+link.label.toHtmlEscaped()+"</strong><small>"+link.url.host().toHtmlEscaped()+ (link.status=="to-confirm"?" · adresse a confirmer":"")+"</small></a>";
    return "<!doctype html><html lang=fr><meta charset=utf-8><meta http-equiv=Content-Security-Policy content=\"default-src 'none'; style-src 'unsafe-inline'; script-src 'none'; form-action 'none'\"><title>Kyronext Start</title><style>body{margin:0;background:#0d1016;color:#edf0f5;font:18px system-ui,sans-serif;padding:8vh 7vw}h1{font-size:clamp(40px,8vw,84px);letter-spacing:-.05em;margin:.4em 0}em{font-style:normal;color:#ef3c60}p,small{color:#aab4c5}nav{display:flex;gap:16px;flex-wrap:wrap;margin-top:48px}a{color:#f5f6fa;background:#191e28;border:1px solid #363e4b;border-radius:12px;padding:24px;text-decoration:none;min-width:190px}a:hover,a:focus{border-color:#ef3c60}small{display:block;margin-top:10px;font-size:14px}</style><p>KYRONEXT SYSTEMS — BY MANIX</p><h1>Kyronext <em>Start.</em></h1><p>Une page locale. Votre point de depart.</p><p>Utilisez la barre d'adresse pour naviguer ou rechercher.</p><nav>"+cards+"</nav><p>Navigator experimental : pour le Web quotidien, utilisez Firefox ESR.</p></html>";
}
}
