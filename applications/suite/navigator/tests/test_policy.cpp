#include "policy.h"
#include <QtTest>
#include <QTemporaryDir>

class PolicyTests final : public QObject {
    Q_OBJECT
private slots:
    void addresses() {
        QCOMPARE(Navigator::address("example.org").scheme(),QString("https"));
        QCOMPARE(Navigator::address("https://example.org/a").host(),QString("example.org"));
        QVERIFY(Navigator::address("javascript:alert(1)").isEmpty());QVERIFY(Navigator::address("file:///etc/passwd").isEmpty());QVERIFY(Navigator::address("data:text/html,hello").isEmpty());
        QVERIFY(Navigator::address("https://user:password@example.org").isEmpty());QVERIFY(!Navigator::webUrl(QUrl("https://@example.org")));
        const QString query="Kyronext & les fichiers";QCOMPARE(QUrlQuery(Navigator::address(query)).queryItemValue("q"),query);
        QCOMPARE(Navigator::historyUrl(QUrl("https://example.org/page?token=secret#private")).toString(),QString("https://example.org/page"));
    }
    void centralizedLinks() {
        QTemporaryDir directory;QVERIFY(directory.isValid());const auto system=directory.filePath("system.json"),user=directory.filePath("user.json");
        auto write=[](const QString &path,const QByteArray &bytes){QFile file(path);if(!file.open(QIODevice::WriteOnly))return false;return file.write(bytes)==bytes.size();};
        QVERIFY(write(system,QByteArrayLiteral("{\"schema_version\":1,\"links\":[{\"id\":\"official\",\"label\":\"Kyronext\",\"url\":\"https://kironext.be\"},{\"id\":\"bad\",\"label\":\"Rejected\",\"url\":\"javascript:alert(1)\"}]}")));
        QVERIFY(write(user,QByteArrayLiteral("{\"schema_version\":1,\"links\":[{\"id\":\"official\",\"visible\":false,\"url\":\"https://user:pass@other.example\"}]}")));QStringList errors;const auto links=Navigator::links(system,user,&errors);QCOMPARE(links.size(),1);QCOMPARE(links[0].url.host(),QString("kironext.be"));QVERIFY(!links[0].visible);QVERIFY(!errors.isEmpty());
        const auto html=Navigator::startHtml({{"official","<img src=bad>",QUrl("https://kironext.be"),true,"to-confirm"}});QVERIFY(!html.contains("<img src=bad>"));QVERIFY(html.contains("&lt;img"));QVERIFY(html.contains("script-src 'none'"));
    }
    void storesAndEnvironment() {
        QTemporaryDir directory;const auto path=directory.filePath("favorites.json");const QJsonArray data{QJsonObject{{"url","https://example.org"}}};QVERIFY(Navigator::writeArray(path,data));QCOMPARE(Navigator::readArray(path),data);QVERIFY(!Navigator::writeArray(directory.path(),data));QCOMPARE(Navigator::readArray(path),data);
        const auto old=qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");qputenv("QTWEBENGINE_CHROMIUM_FLAGS","--external-flag");QVERIFY(Navigator::unsafeEnvironment());if(old.isNull())qunsetenv("QTWEBENGINE_CHROMIUM_FLAGS");else qputenv("QTWEBENGINE_CHROMIUM_FLAGS",old);
    }
};
QTEST_APPLESS_MAIN(PolicyTests)
#include "test_policy.moc"
