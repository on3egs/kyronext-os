#include "metrics.h"
#include "fileops.h"
#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <KJob>
#include <unistd.h>

namespace {
bool put(const QString &path,const QByteArray &data){QFile f(path);return f.open(QIODevice::WriteOnly) && f.write(data)==data.size();}
QByteArray get(const QString &path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
}
class SystemTest:public QObject {
    Q_OBJECT
private slots:
    void cpuDeltaAndGuestAccounting() {
        QTemporaryDir dir;QVERIFY(dir.isValid());const QString stat=dir.filePath("stat");
        QVERIFY(put(stat,"cpu 100 20 30 200 10 5 3 2 500 100\n"));const auto first=readCpu(stat);
        QCOMPARE(first.total,quint64(370));QCOMPARE(first.idle,quint64(210));
        QVERIFY(put(stat,"cpu 200 20 30 300 10 5 3 2 500 100\n"));const auto second=readCpu(stat);
        QCOMPARE(cpuPercent(first,second),50.0);QCOMPARE(cpuPercent(second,first),-1.0);QCOMPARE(cpuPercent({},second),-1.0);
    }
    void fixtureMemoryNetworkAndProcess() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QDir root(dir.path());QVERIFY(root.mkpath("net"));QVERIFY(root.mkpath("42"));
        QVERIFY(put(dir.filePath("stat"),"cpu 1 0 1 8\n"));
        QVERIFY(put(dir.filePath("meminfo"),"MemTotal: 1024 kB\nMemAvailable: 512 kB\nSwapTotal: 256 kB\nSwapFree: 128 kB\n"));
        QVERIFY(put(dir.filePath("net/dev")," eth0: 100 1 0 0 0 0 0 0 500 1 0 0 0 0 0 0\n"));
        QStringList values;for(int n=0;n<24;++n)values.append("0");values[0]="S";values[11]="12";values[12]="3";values[19]="999";values[21]="2";
        QVERIFY(put(dir.filePath("42/stat"),("42 (my (worker)) "+values.join(' ')).toUtf8()));
        const auto sample=readSystem(dir.path(),dir.filePath("no-sys"));
        QCOMPARE(sample.memoryTotal,quint64(1024*1024));QCOMPARE(sample.memoryAvailable,quint64(512*1024));
        QCOMPARE(sample.swapFree,quint64(128*1024));QCOMPARE(sample.network["eth0"].first,quint64(100));
        QCOMPARE(sample.network["eth0"].second,quint64(500));QCOMPARE(sample.processes.size(),1);
        QCOMPARE(sample.processes[0].name,QString("my (worker)"));QCOMPARE(sample.processes[0].ticks,quint64(15));
        QCOMPARE(sample.processes[0].start,quint64(999));QCOMPARE(sample.processes[0].rss,quint64(2*sysconf(_SC_PAGESIZE)));
        QVERIFY(sample.temperatures.isEmpty());
        const auto lightweight=readSystem(dir.path(),dir.filePath("no-sys"),false);
        QVERIFY(lightweight.processes.isEmpty());
    }
    void batteryCapacityHealthAndCycles() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QDir root(dir.path());
        QVERIFY(root.mkpath("class/power_supply/BAT0"));
        const auto bat=root.filePath("class/power_supply/BAT0");
        QVERIFY(put(bat+"/type","Battery\n"));QVERIFY(put(bat+"/status","Discharging\n"));
        QVERIFY(put(bat+"/capacity","68\n"));QVERIFY(put(bat+"/energy_full","34000000\n"));
        QVERIFY(put(bat+"/energy_now","20000000\n"));QVERIFY(put(bat+"/power_now","10000000\n"));
        QVERIFY(put(bat+"/energy_full_design","50000000\n"));QVERIFY(put(bat+"/cycle_count","421\n"));
        const auto sample=readSystem(dir.path(),dir.path());
        QCOMPARE(sample.batteries.size(),1);QCOMPARE(sample.batteries[0].name,QString("BAT0"));
        QCOMPARE(sample.batteries[0].status,QString("Discharging"));QCOMPARE(sample.batteries[0].capacityPercent,68);
        QCOMPARE(sample.batteries[0].healthPercent,68.0);QCOMPARE(sample.batteries[0].remainingMinutes,qint64(120));
        QVERIFY(sample.batteries[0].cyclesAvailable);QCOMPARE(sample.batteries[0].cycles,quint64(421));
        QVERIFY(QFile::remove(bat+"/energy_now"));QVERIFY(QFile::remove(bat+"/power_now"));
        QVERIFY(put(bat+"/charge_now","2500000\n"));QVERIFY(put(bat+"/current_now","500000\n"));
        const auto chargeEstimate=readSystem(dir.path(),dir.path());
        QCOMPARE(chargeEstimate.batteries[0].remainingMinutes,qint64(300));
        QVERIFY(put(bat+"/status","Charging\n"));
        const auto charging=readSystem(dir.path(),dir.path());
        QCOMPARE(charging.batteries[0].remainingMinutes,qint64(-1));
    }
    void acAndUsbPowerSourcesAreReadOnly() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QDir root(dir.path());
        QVERIFY(root.mkpath("class/power_supply/AC"));QVERIFY(root.mkpath("class/power_supply/USB-C"));
        QVERIFY(put(root.filePath("class/power_supply/AC/type"),"Mains\n"));
        QVERIFY(put(root.filePath("class/power_supply/AC/online"),"1\n"));
        QVERIFY(put(root.filePath("class/power_supply/USB-C/type"),"USB_C\n"));
        QVERIFY(put(root.filePath("class/power_supply/USB-C/online"),"0\n"));
        const auto sample=readSystem(dir.path(),dir.path());
        QCOMPARE(sample.powerSources.size(),2);
        QCOMPARE(sample.powerSources[0].name,QString("AC"));
        QCOMPARE(sample.powerSources[0].type,QString("Mains"));
        QVERIFY(sample.powerSources[0].online);
        QCOMPARE(sample.powerSources[1].name,QString("USB-C"));
        QVERIFY(!sample.powerSources[1].online);
    }
    void networkLinkStateIsReadOnly() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QDir root(dir.path());
        QVERIFY(root.mkpath("class/net/enp1s0"));QVERIFY(root.mkpath("class/net/wlan0"));QVERIFY(root.mkpath("class/net/lo"));
        QVERIFY(put(root.filePath("class/net/enp1s0/operstate"),"up\n"));
        QVERIFY(put(root.filePath("class/net/enp1s0/carrier"),"1\n"));
        QVERIFY(put(root.filePath("class/net/enp1s0/speed"),"1000\n"));
        QVERIFY(put(root.filePath("class/net/wlan0/operstate"),"down\n"));
        QVERIFY(put(root.filePath("class/net/wlan0/carrier"),"0\n"));
        const auto sample=readSystem(dir.path(),dir.path());
        QCOMPARE(sample.networkLinks.size(),2);
        QCOMPARE(sample.networkLinks[0].name,QString("enp1s0"));
        QVERIFY(sample.networkLinks[0].carrier);QCOMPARE(sample.networkLinks[0].speedMbps,1000);
        QCOMPARE(sample.networkLinks[1].name,QString("wlan0"));
        QVERIFY(!sample.networkLinks[1].carrier);QCOMPARE(sample.networkLinks[1].state,QString("down"));
    }
    void graphicsInventoryUsesPciClassAndDriverLink() {
        QTemporaryDir dir;QVERIFY(dir.isValid());QDir root(dir.path());
        QVERIFY(root.mkpath("bus/pci/devices/0000:00:02.0"));
        QVERIFY(root.mkpath("bus/pci/devices/0000:00:14.0"));
        const auto gpu=root.filePath("bus/pci/devices/0000:00:02.0");
        QVERIFY(put(gpu+"/class","0x030000\n"));
        QVERIFY(put(gpu+"/vendor","0x8086\n"));
        QVERIFY(put(gpu+"/device","0x5917\n"));
        const auto network=root.filePath("bus/pci/devices/0000:00:14.0");
        QVERIFY(put(network+"/class","0x020000\n"));
        const auto sample=readSystem(dir.filePath("proc"),dir.path(),false);
        QCOMPARE(sample.graphicsDevices.size(),1);
        QCOMPARE(sample.graphicsDevices[0].pciAddress,QString("0000:00:02.0"));
        QCOMPARE(sample.graphicsDevices[0].vendorId,QString("0x8086"));
        QCOMPARE(sample.graphicsDevices[0].deviceId,QString("0x5917"));
        QVERIFY(sample.graphicsDevices[0].driver.isEmpty());
    }
    void networkRouteAndDnsParsingIsLocalAndBounded() {
        QTemporaryDir dir;QVERIFY(dir.isValid());
        const auto routes=dir.filePath("route"),resolv=dir.filePath("resolv.conf");
        QVERIFY(put(routes,"Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT\n"
                           "eth0 00000000 0101A8C0 0003 0 0 100 00000000 0 0 0\n"));
        QVERIFY(put(resolv,"# local resolver configuration\n"
                           "nameserver 192.168.1.1\n"
                           "nameserver 2001:db8::53\n"
                           "nameserver invalid\n"));
        const auto config=readNetworkConfiguration(routes,resolv);
        QCOMPARE(config.defaultGateway,QString("192.168.1.1"));
        QCOMPARE(config.dnsServers.size(),2);
        QCOMPARE(config.dnsServers[0],QString("192.168.1.1"));
        QCOMPARE(config.dnsServers[1],QString("2001:db8::53"));
    }
    void terminationIdentityAndSignal() {
        QProcess child;child.start("/bin/sleep",{"30"});QVERIFY(child.waitForStarted());
        ProcessInfo process;QVERIFY(readProcess(child.processId(),&process));QString error;
        auto changed=process;++changed.start;QVERIFY(!terminateProcess(changed,&error));QVERIFY(child.state()==QProcess::Running);
        QVERIFY(terminateProcess(process,&error));QVERIFY(child.waitForFinished(5000));
        ProcessInfo protectedProcess;protectedProcess.pid=1;QVERIFY(!terminateProcess(protectedProcess,&error));
    }
    void copyMoveAndOverwriteProtection() {
        QTemporaryDir dir;QVERIFY(dir.isValid());const auto source=dir.filePath("source.txt"),copy=dir.filePath("copie.txt"),moved=dir.filePath("renomme.txt");
        const QByteArray contents="Kyronext \xc3\xa9t\xc3\xa9\n";QVERIFY(put(source,contents));QString error;
        auto *job=copyLocal(QUrl::fromLocalFile(source),QUrl::fromLocalFile(copy),&error);QVERIFY2(job,qPrintable(error));QVERIFY2(job->exec(),qPrintable(job->errorText()));
        QCOMPARE(get(copy),contents);QCOMPARE(get(source),contents);
        QVERIFY(!copyLocal(QUrl::fromLocalFile(source),QUrl::fromLocalFile(copy),&error));QCOMPARE(get(copy),contents);
        job=moveLocal(QUrl::fromLocalFile(copy),QUrl::fromLocalFile(moved),&error);QVERIFY(job);QVERIFY(job->exec());
        QVERIFY(!QFileInfo::exists(copy));QCOMPARE(get(moved),contents);
        QVERIFY(!copyLocal(QUrl("https://example.invalid/a"),QUrl::fromLocalFile(copy),&error));
        QVERIFY(!copyLocal(QUrl("file://remote.invalid/a"),QUrl::fromLocalFile(copy),&error));
        QVERIFY(QDir(dir.path()).mkdir("folder"));QVERIFY(!validateDestination(QUrl::fromLocalFile(dir.filePath("folder")),QUrl::fromLocalFile(dir.filePath("folder/sub")),&error));
    }
};
QTEST_GUILESS_MAIN(SystemTest)
#include "system_test.moc"
