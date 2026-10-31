#include "archive_backend.h"
#include "calendar_store.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace {
bool put(const QString &path,const QByteArray &data){QFile f(path);return f.open(QIODevice::WriteOnly) && f.write(data)==data.size();}
QByteArray get(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
void little(QByteArray &b,quint32 value,int size){for(int i=0;i<size;++i)b.append(char(value>>(8*i)));}
QByteArray zip(const QByteArray &name,quint32 mode=0100644,quint32 size=0) {
    QByteArray local;little(local,0x04034b50,4);little(local,20,2);for(int i=0;i<3;++i)little(local,0,2);little(local,0,2);little(local,0,4);little(local,0,4);little(local,size,4);little(local,name.size(),2);little(local,0,2);local+=name;
    QByteArray central;little(central,0x02014b50,4);little(central,0x0314,2);little(central,20,2);for(int i=0;i<4;++i)little(central,0,2);little(central,0,4);little(central,0,4);little(central,size,4);little(central,name.size(),2);for(int i=0;i<4;++i)little(central,0,2);little(central,mode<<16,4);little(central,0,4);central+=name;
    QByteArray end;little(end,0x06054b50,4);little(end,0,2);little(end,0,2);little(end,1,2);little(end,1,2);little(end,central.size(),4);little(end,local.size(),4);little(end,0,2);return local+central+end;
}
QByteArray tar(const QByteArray &name,char type='0') {
    QByteArray h(512,'\0');h.replace(0,qMin(100,int(name.size())),name.left(100));h.replace(100,8,"0000644\0",8);h.replace(124,12,"00000000000\0",12);h.replace(148,8,QByteArray(8,' '));h[156]=type;h.replace(257,6,"ustar\0",6);
    quint32 sum=0;for(const auto c:h)sum+=quint8(c);auto checksum=QByteArray::number(sum,8).rightJustified(6,'0');checksum.append('\0');checksum.append(' ');h.replace(148,8,checksum);return h+QByteArray(1024,'\0');
}
}
class OrganizerTest:public QObject {
    Q_OBJECT
private slots:
    void zipAndTarRoundTrip() {
        for(const QString format:{"zip","tar"}) {
            QTemporaryDir dir;QVERIFY(dir.isValid());QVERIFY(QDir(dir.path()).mkdir("source"));QVERIFY(QDir(dir.filePath("source")).mkdir("empty"));QVERIFY(QDir(dir.filePath("source")).mkdir("nested"));QVERIFY(put(dir.filePath("source/nested/child.txt"),"nested data"));
            const QByteArray text="Kyronext \xc3\xa9t\xc3\xa9\n";QVERIFY(put(dir.filePath("source/document.txt"),text));QString error;
            const auto archive=dir.filePath("creation."+format),output=dir.filePath("result");
            QVERIFY2(SafeArchive::create(dir.filePath("source"),archive,&error),qPrintable(error));
            QVector<ArchiveItem> items;QVERIFY2(SafeArchive::list(archive,&items,&error),qPrintable(error));QVERIFY(items.size()>=2);
            QVERIFY2(SafeArchive::extract(archive,output,&error),qPrintable(error));QCOMPARE(get(output+"/document.txt"),text);QVERIFY(QFileInfo(output+"/empty").isDir());QCOMPARE(get(output+"/nested/child.txt"),QByteArray("nested data"));
            QVERIFY(!SafeArchive::extract(archive,output,&error));QCOMPARE(get(output+"/document.txt"),text);
            const auto original=get(archive);QVERIFY(!SafeArchive::create(dir.filePath("source"),archive,&error));QCOMPARE(get(archive),original);
        }
    }
    void unsafeZipRejected() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QString error;QVector<ArchiveItem> items;
        QVERIFY(put(dir.filePath("valid.zip"),zip("empty.txt")));QVERIFY2(SafeArchive::list(dir.filePath("valid.zip"),&items,&error),qPrintable(error));QCOMPARE(items.size(),1);
        for(const auto name:{QByteArray("../escape"),QByteArray("/absolute"),QByteArray("folder/../../escape"),QByteArray("C:/windows"),QByteArray("folder\\escape")}) {
            const auto path=dir.filePath("bad.zip");QVERIFY(put(path,zip(name)));QVERIFY2(!SafeArchive::list(path,&items,&error),name.constData());QVERIFY(!SafeArchive::extract(path,dir.filePath("out"),&error));QVERIFY(!QFileInfo::exists(dir.filePath("escape")));
        }
        QVERIFY(put(dir.filePath("link.zip"),zip("link",0120777)));QVERIFY(!SafeArchive::list(dir.filePath("link.zip"),&items,&error));
        QVERIFY(put(dir.filePath("big.zip"),zip("big",0100644,SafeArchive::MaximumFileBytes+1)));QVERIFY(!SafeArchive::list(dir.filePath("big.zip"),&items,&error));
    }
    void unsafeTarAndLinksRejected() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QString error;QVector<ArchiveItem> items;
        for(const auto name:{QByteArray("../escape"),QByteArray("/absolute")}){QVERIFY(put(dir.filePath("bad.tar"),tar(name)));QVERIFY(!SafeArchive::list(dir.filePath("bad.tar"),&items,&error));}
        QVERIFY(put(dir.filePath("link.tar"),tar("link",'2')));QVERIFY(!SafeArchive::list(dir.filePath("link.tar"),&items,&error));
        QVERIFY(QDir(dir.path()).mkdir("source"));QVERIFY(put(dir.filePath("secret"),"original"));QVERIFY(QFile::link(dir.filePath("secret"),dir.filePath("source/link")));
        QVERIFY(!SafeArchive::create(dir.filePath("source"),dir.filePath("links.zip"),&error));QCOMPARE(get(dir.filePath("secret")),QByteArray("original"));
    }
    void calendarPersistsAndRefusesConflicts() {
        QTemporaryDir dir;QVERIFY(dir.isValid());const auto path=dir.filePath("events.json");QString error;
        QVector<CalendarEvent> events={{"id-1","Événement Kyronext","Notes personnelles",QDate(2026,11,1),QTime(9,30)}};
        QVERIFY(saveCalendar(path,events,{},&error));QVector<CalendarEvent> loaded;QVERIFY(loadCalendar(path,&loaded,&error));QCOMPARE(loaded.size(),1);QCOMPARE(loaded[0].title,events[0].title);QCOMPARE(loaded[0].date,events[0].date);
        const auto old=calendarFingerprint(path);QVERIFY(put(path,"donnees externes"));QVERIFY(!saveCalendar(path,events,old,&error));QCOMPARE(get(path),QByteArray("donnees externes"));
        events[0].date=QDate();QVERIFY(!saveCalendar(dir.filePath("invalid.json"),events,{},&error));QVERIFY(!QFileInfo::exists(dir.filePath("invalid.json")));
        QVERIFY(!loadCalendar(path,&loaded,&error));
    }
};
QTEST_GUILESS_MAIN(OrganizerTest)
#include "organizer_test.moc"
