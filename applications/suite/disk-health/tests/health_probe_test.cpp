#include "../health_probe.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class DiskHealthTest final : public QObject {
    Q_OBJECT
private slots:
    void parsesAtaHealth() {
        const auto result = parseSmartHealth(R"({"smart_status":{"passed":true}})", 0);
        QVERIFY(result.state == HealthState::Healthy);
    }
    void reportsAtaFailure() {
        const auto result = parseSmartHealth(R"({"smart_status":{"passed":false}})", 8);
        QVERIFY(result.state == HealthState::Warning);
    }
    void doesNotCallHealthCleanWhenCommandReportsOtherErrors() {
        const auto result = parseSmartHealth(R"({"smart_status":{"passed":true}})", 2);
        QVERIFY(result.state == HealthState::Unknown);
    }
    void parsesNvmeWarnings() {
        QVERIFY(parseSmartHealth(R"({"nvme_smart_health_information_log":{"critical_warning":0}})", 0).state ==
                HealthState::Healthy);
        QVERIFY(parseSmartHealth(R"({"nvme_smart_health_information_log":{"critical_warning":2}})", 0).state ==
                HealthState::Warning);
    }
    void treatsMissingOrMalformedDataAsUnknown() {
        QVERIFY(parseSmartHealth("not-json", 0).state == HealthState::Unknown);
        QVERIFY(parseSmartHealth(R"({})", 2).state == HealthState::Unknown);
    }
    void discoversOnlyExpectedPhysicalDeviceNames() {
        QTemporaryDir temp;
        QVERIFY(temp.isValid());
        auto addDisk = [&](const QString &name, const QString &model) {
            const QString device = temp.path() + QLatin1Char('/') + name + QStringLiteral("/device");
            QVERIFY(QDir().mkpath(device));
            QFile file(device + QStringLiteral("/model"));
            if (file.open(QIODevice::WriteOnly)) file.write(model.toUtf8());
        };
        addDisk(QStringLiteral("sda"), QStringLiteral("Test SATA"));
        addDisk(QStringLiteral("nvme2n1"), QStringLiteral("Test NVMe"));
        const QString mmcDevice = temp.path() + QStringLiteral("/mmcblk0/device");
        QVERIFY(QDir().mkpath(mmcDevice));
        QFile mmcName(mmcDevice + QStringLiteral("/name"));
        QVERIFY(mmcName.open(QIODevice::WriteOnly));
        mmcName.write("Test MMC");
        mmcName.close();
        addDisk(QStringLiteral("loop0"), QStringLiteral("Loop device"));
        const auto disks = discoverDisks(temp.path());
        QCOMPARE(disks.size(), 3);
        QCOMPARE(disks[0].device, QStringLiteral("/dev/mmcblk0"));
        QCOMPARE(disks[0].model, QStringLiteral("Test MMC"));
        QCOMPARE(disks[1].device, QStringLiteral("/dev/nvme2n1"));
        QCOMPARE(disks[2].device, QStringLiteral("/dev/sda"));
        QCOMPARE(disks[2].model, QStringLiteral("Test SATA"));
    }
};

QTEST_GUILESS_MAIN(DiskHealthTest)
#include "health_probe_test.moc"
