#include "../apt_simulation.h"
#include <QtTest>

class AptSimulationTest final : public QObject {
    Q_OBJECT
private slots:
    void countsUpgradesAndRemovalsWithoutExecutingThem() {
        const QString output = QStringLiteral(
            "Inst libc6 [2.36-9] (2.36-9+deb12u1 Debian:stable-security)\n"
            "Inst kyronext-demo (0.2 Debian:stable)\n"
            "Remv old-package [1.0]\n"
            "1 upgraded, 1 newly installed, 1 to remove and 3 not upgraded.\n");
        const auto counts = summarizeAptSimulation(output);
        QCOMPARE(counts.upgrades, 1);
        QCOMPARE(counts.installs, 1);
        QCOMPARE(counts.removals, 1);
        QCOMPARE(counts.heldBack, 3);
    }

    void handlesNoChangesAndLocalizedOrUnknownOutputSafely() {
        const auto counts = summarizeAptSimulation(QStringLiteral(
            "0 upgraded, 0 newly installed, 0 to remove and 0 not upgraded.\n"));
        QCOMPARE(counts.upgrades, 0);
        QCOMPARE(counts.removals, 0);
        QCOMPARE(counts.heldBack, 0);
        const auto unknown = summarizeAptSimulation(QStringLiteral("APT unavailable\n"));
        QCOMPARE(unknown.upgrades, 0);
        QCOMPARE(unknown.removals, 0);
    }
};

QTEST_APPLESS_MAIN(AptSimulationTest)
#include "apt_simulation_test.moc"
