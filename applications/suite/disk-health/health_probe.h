#pragma once

#include <QString>
#include <QVector>

enum class HealthState { Healthy, Warning, Unknown };

struct DiskHealth {
    QString device;
    QString model;
    HealthState state = HealthState::Unknown;
    QString summary;
};

struct SmartHealthResult {
    HealthState state = HealthState::Unknown;
    QString summary;
};

QVector<DiskHealth> discoverDisks(const QString &sysBlockRoot = QStringLiteral("/sys/block"));
SmartHealthResult parseSmartHealth(const QByteArray &json, int exitCode);
