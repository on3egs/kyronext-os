#pragma once

#include <QRegularExpression>
#include <QString>

struct AptSimulationSummary {
    int upgrades = 0;
    int installs = 0;
    int removals = 0;
    int heldBack = 0;
};

inline AptSimulationSummary summarizeAptSimulation(const QString &output) {
    AptSimulationSummary summary;
    static const QRegularExpression totals(
        R"((\d+) upgraded,\s*(\d+) newly installed,\s*(\d+) to remove and\s*(\d+) not upgraded)");
    const auto match = totals.match(output);
    if (match.hasMatch()) {
        summary.upgrades = match.captured(1).toInt();
        summary.installs = match.captured(2).toInt();
        summary.removals = match.captured(3).toInt();
        summary.heldBack = match.captured(4).toInt();
    }
    return summary;
}
