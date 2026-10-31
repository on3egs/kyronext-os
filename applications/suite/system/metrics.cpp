#include "metrics.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <sys/syscall.h>
#include <unistd.h>

namespace {
QString read(const QString &path) { QFile f(path);return f.open(QIODevice::ReadOnly)?QString::fromUtf8(f.readAll()):QString(); }
QStringList fields(const QString &line) { return line.split(QRegularExpression("\\s+"),Qt::SkipEmptyParts); }
class ScopedFileDescriptor {
public:
    explicit ScopedFileDescriptor(int descriptor) : descriptor_(descriptor) {}
    ~ScopedFileDescriptor() { if (descriptor_ >= 0) close(descriptor_); }
    ScopedFileDescriptor(const ScopedFileDescriptor &) = delete;
    ScopedFileDescriptor &operator=(const ScopedFileDescriptor &) = delete;
    int get() const { return descriptor_; }
private:
    int descriptor_;
};
}
CpuCounters readCpu(const QString &file) {
    CpuCounters r;const auto values=fields(read(file).section('\n',0,0));
    if(values.size()<5 || values[0]!="cpu")return r;
    for(int i=1;i<qMin(9,int(values.size()));++i)r.total+=values[i].toULongLong();
    r.idle=values[4].toULongLong()+(values.size()>5?values[5].toULongLong():0);r.valid=true;return r;
}
double cpuPercent(CpuCounters p,CpuCounters n) {
    if(!p.valid || !n.valid || n.total<=p.total || n.idle<p.idle)return -1;
    const auto delta=n.total-p.total;return qBound(0.0,100.0*(double(delta)-double(n.idle-p.idle))/double(delta),100.0);
}
bool readProcess(qint64 pid,ProcessInfo *r,const QString &root) {
    const QString directory=root+"/"+QString::number(pid);const QString data=read(directory+"/stat");
    const int left=data.indexOf('('),right=data.lastIndexOf(')');
    if(left<0 || right<left)return false;const auto values=fields(data.mid(right+1));if(values.size()<22)return false;
    bool ok=false;const qint64 actual=data.left(left).trimmed().toLongLong(&ok);if(!ok || actual!=pid)return false;
    r->pid=pid;r->name=data.mid(left+1,right-left-1);r->uid=QFileInfo(directory).ownerId();
    r->ticks=values[11].toULongLong()+values[12].toULongLong();r->start=values[19].toULongLong();
    r->rss=qMax<qint64>(0,values[21].toLongLong())*quint64(sysconf(_SC_PAGESIZE));return true;
}
SystemSample readSystem(const QString &proc,const QString &sys,bool includeProcesses) {
    SystemSample s;s.cpu=readCpu(proc+"/stat");QMap<QString,quint64> memory;
    for(const auto &line:read(proc+"/meminfo").split('\n')) {const auto v=fields(line);if(v.size()>=2)memory[v[0].chopped(1)]=v[1].toULongLong()*1024;}
    s.memoryTotal=memory.value("MemTotal");s.memoryAvailable=memory.value("MemAvailable");
    s.swapTotal=memory.value("SwapTotal");s.swapFree=memory.value("SwapFree");
    for(const auto &line:read(proc+"/net/dev").split('\n')) {
        const int colon=line.indexOf(':');if(colon<0)continue;const auto v=fields(line.mid(colon+1));
        if(v.size()>=16)s.network[line.left(colon).trimmed()]={v[0].toULongLong(),v[8].toULongLong()};
    }
    const QDir net(sys+"/class/net");
    for(const auto &entry:net.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        if(entry=="lo")continue;
        const QString base=net.filePath(entry);NetworkLinkInfo link;link.name=entry;
        link.state=read(base+"/operstate").trimmed();
        bool carrierOk=false;const int carrier=read(base+"/carrier").trimmed().toInt(&carrierOk);
        if(carrierOk && (carrier==0 || carrier==1)){link.carrierAvailable=true;link.carrier=carrier==1;}
        bool speedOk=false;const int speed=read(base+"/speed").trimmed().toInt(&speedOk);
        if(speedOk && speed>0 && speed<=1000000)link.speedMbps=speed;
        s.networkLinks.append(link);
    }
    for(const auto &interface:QNetworkInterface::allInterfaces()) {
        if(interface.flags().testFlag(QNetworkInterface::IsLoopBack) ||
           !interface.flags().testFlag(QNetworkInterface::IsUp))continue;
        QStringList addresses;
        for(const auto &entry:interface.addressEntries()) {
            const auto address=entry.ip();
            if(address.protocol()==QAbstractSocket::IPv4Protocol ||
               address.protocol()==QAbstractSocket::IPv6Protocol)
                addresses.append(address.toString());
        }
        if(!addresses.isEmpty())s.networkAddresses.insert(interface.name(),addresses);
    }
    s.networkConfiguration=readNetworkConfiguration(proc+"/net/route");
    const QDir pciDevices(sys+"/bus/pci/devices");
    for(const auto &entry:pciDevices.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        const QString base=pciDevices.filePath(entry);
        bool classOk=false;
        const quint32 deviceClass=read(base+"/class").trimmed().toUInt(&classOk,0);
        if(!classOk || ((deviceClass>>16)&0xffu)!=0x03u)continue;
        GraphicsDeviceInfo device;
        device.pciAddress=entry;
        device.vendorId=read(base+"/vendor").trimmed();
        device.deviceId=read(base+"/device").trimmed();
        const QFileInfo driverLink(base+"/driver");
        if(driverLink.isSymLink())device.driver=QFileInfo(driverLink.symLinkTarget()).fileName();
        s.graphicsDevices.append(device);
    }
    if(includeProcesses) {
        for(const auto &entry:QDir(proc).entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
            bool ok=false;const auto pid=entry.toLongLong(&ok);ProcessInfo p;if(ok && readProcess(pid,&p,proc))s.processes.append(p);
        }
    }
    const QDir zones(sys+"/class/thermal");
    for(const auto &entry:zones.entryList({"thermal_zone*"},QDir::Dirs|QDir::NoDotAndDotDot)) {
        const QString base=zones.filePath(entry);bool ok=false;const double value=read(base+"/temp").trimmed().toDouble(&ok)/1000.0;
        if(ok && value>-100 && value<250)s.temperatures.append(read(base+"/type").trimmed()+": "+QString::number(value,'f',1)+" °C");
    }
    const QDir supplies(sys+"/class/power_supply");
    for(const auto &entry:supplies.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        const QString base=supplies.filePath(entry);
        const QString type=read(base+"/type").trimmed();
        if(type=="Mains" || type=="USB" || type=="USB_C") {
            bool onlineOk=false;const int online=read(base+"/online").trimmed().toInt(&onlineOk);
            if(onlineOk && (online==0 || online==1))s.powerSources.append({entry,type,online==1});
            continue;
        }
        if(type!="Battery")continue;
        BatteryInfo battery;battery.name=entry;battery.status=read(base+"/status").trimmed();
        bool ok=false;const int capacity=read(base+"/capacity").trimmed().toInt(&ok);
        if(ok && capacity>=0 && capacity<=100)battery.capacityPercent=capacity;
        if(battery.status=="Discharging") {
            bool energyOk=false,powerOk=false,chargeOk=false,currentOk=false;
            const quint64 energy=read(base+"/energy_now").trimmed().toULongLong(&energyOk);
            const quint64 power=read(base+"/power_now").trimmed().toULongLong(&powerOk);
            const quint64 charge=read(base+"/charge_now").trimmed().toULongLong(&chargeOk);
            const quint64 current=read(base+"/current_now").trimmed().toULongLong(&currentOk);
            const double hours=energyOk && powerOk && power>0 ? double(energy)/double(power) :
                chargeOk && currentOk && current>0 ? double(charge)/double(current) : -1.0;
            if(hours>0 && hours<=168.0)battery.remainingMinutes=qRound64(hours*60.0);
        }
        const QString fullText=read(base+"/energy_full").trimmed().isEmpty()?read(base+"/charge_full").trimmed():read(base+"/energy_full").trimmed();
        const QString designText=read(base+"/energy_full_design").trimmed().isEmpty()?read(base+"/charge_full_design").trimmed():read(base+"/energy_full_design").trimmed();
        bool fullOk=false,designOk=false;const quint64 full=fullText.toULongLong(&fullOk),design=designText.toULongLong(&designOk);
        if(fullOk && designOk && design>0 && full<=design*2)battery.healthPercent=100.0*double(full)/double(design);
        bool cyclesOk=false;const quint64 cycles=read(base+"/cycle_count").trimmed().toULongLong(&cyclesOk);
        if(cyclesOk){battery.cycles=cycles;battery.cyclesAvailable=true;}
        s.batteries.append(battery);
    }
    return s;
}
NetworkConfiguration readNetworkConfiguration(const QString &routeFile,const QString &resolvFile) {
    NetworkConfiguration result;
    for(const auto &line:read(routeFile).split('\n')) {
        const auto v=fields(line);
        if(v.size()<8 || v[0]=="Iface" || v[1]!="00000000")continue;
        bool flagsOk=false,gatewayOk=false;
        const quint32 flags=v[3].toUInt(&flagsOk,16);
        const quint32 gateway=v[2].toUInt(&gatewayOk,16);
        if(!flagsOk || !gatewayOk || !(flags&1u) || gateway==0)continue;
        result.defaultGateway=QStringLiteral("%1.%2.%3.%4")
            .arg(gateway&0xffu).arg((gateway>>8)&0xffu).arg((gateway>>16)&0xffu).arg((gateway>>24)&0xffu);
        break;
    }
    for(const auto &line:read(resolvFile).split('\n')) {
        const auto v=fields(line);
        if(v.size()<2 || v[0]!="nameserver" || result.dnsServers.size()>=8)continue;
        QHostAddress address;
        if(address.setAddress(v[1]) && !result.dnsServers.contains(address.toString()))
            result.dnsServers.append(address.toString());
    }
    return result;
}
bool terminateProcess(const ProcessInfo &expected,QString *error) {
    const auto fail=[&](QString m){if(error)*error=m;return false;};
    if(expected.pid<=1 || expected.pid==getpid())return fail("Ce processus est protege.");
#if defined(SYS_pidfd_open) && defined(SYS_pidfd_send_signal)
    const long descriptor=syscall(SYS_pidfd_open,pid_t(expected.pid),0u);
    if(descriptor<0)return fail(QStringLiteral("Ouverture sécurisée du processus impossible : %1")
        .arg(QString::fromLocal8Bit(std::strerror(errno))));
    ScopedFileDescriptor pidfd(static_cast<int>(descriptor));
    ProcessInfo current;if(!readProcess(expected.pid,&current))return fail("Le processus a deja disparu.");
    if(current.start!=expected.start)return fail("Le PID a ete reutilise : operation annulee.");
    if(geteuid()==0 || current.uid!=quint32(geteuid()))return fail("Seuls vos propres processus peuvent etre arretes sans privileges.");
    if(syscall(SYS_pidfd_send_signal,pidfd.get(),SIGTERM,static_cast<void *>(nullptr),0u)!=0)
        return fail(QStringLiteral("Envoi sécurisé de SIGTERM impossible : %1")
            .arg(QString::fromLocal8Bit(std::strerror(errno))));
    return true;
#else
    Q_UNUSED(error);
    return fail("Ce noyau ne fournit pas l'API pidfd requise pour arrêter un processus en toute sécurité.");
#endif
}
