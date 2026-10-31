#pragma once
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>

struct CpuCounters { quint64 total=0, idle=0; bool valid=false; };
struct ProcessInfo { qint64 pid=0; QString name; quint32 uid=0; quint64 ticks=0,start=0,rss=0; };
struct BatteryInfo {
    QString name,status;
    int capacityPercent=-1;
    double healthPercent=-1;
    qint64 remainingMinutes=-1;
    quint64 cycles=0;
    bool cyclesAvailable=false;
};
struct PowerSourceInfo {
    QString name,type;
    bool online=false;
};
struct NetworkLinkInfo {
    QString name,state;
    bool carrierAvailable=false,carrier=false;
    int speedMbps=-1;
};
struct NetworkConfiguration {
    QString defaultGateway;
    QStringList dnsServers;
};
struct GraphicsDeviceInfo {
    QString pciAddress,vendorId,deviceId,driver;
};
struct SystemSample {
    CpuCounters cpu;
    quint64 memoryTotal=0,memoryAvailable=0,swapTotal=0,swapFree=0;
    QMap<QString,QPair<quint64,quint64>> network;
    QVector<ProcessInfo> processes;
    QStringList temperatures;
    QVector<BatteryInfo> batteries;
    QVector<PowerSourceInfo> powerSources;
    QVector<NetworkLinkInfo> networkLinks;
    QMap<QString, QStringList> networkAddresses;
    NetworkConfiguration networkConfiguration;
    QVector<GraphicsDeviceInfo> graphicsDevices;
};
CpuCounters readCpu(const QString &file);
bool readProcess(qint64 pid, ProcessInfo *result, const QString &procRoot="/proc");
SystemSample readSystem(const QString &procRoot="/proc",const QString &sysRoot="/sys",
                        bool includeProcesses=true);
NetworkConfiguration readNetworkConfiguration(const QString &routeFile="/proc/net/route",
                                                const QString &resolvFile="/etc/resolv.conf");
double cpuPercent(CpuCounters previous,CpuCounters current);
bool terminateProcess(const ProcessInfo &expected,QString *error);
