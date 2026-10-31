#include "health_probe.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

QVector<DiskHealth> discoverDisks(const QString &sysBlockRoot) {
    QVector<DiskHealth> result;
    const QDir root(sysBlockRoot);
    const QRegularExpression physicalName(QStringLiteral("^(sd[a-z]+|nvme[0-9]+n[0-9]+|mmcblk[0-9]+|vd[a-z]+)$"));
    const QStringList names = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &name : names) {
        if (!physicalName.match(name).hasMatch()) continue;
        const QString path = root.filePath(name);
        if (!QFile::exists(path + QStringLiteral("/device"))) continue;
        QString model;
        for (const QString &attribute : {QStringLiteral("model"), QStringLiteral("name")}) {
            QFile modelFile(path + QStringLiteral("/device/") + attribute);
            if (modelFile.open(QIODevice::ReadOnly)) {
                model = QString::fromLocal8Bit(modelFile.readAll()).trimmed();
                if (!model.isEmpty()) break;
            }
        }
        if (model.isEmpty()) model = QStringLiteral("Modèle non fourni par le noyau");
        result.append({QStringLiteral("/dev/") + name, model, HealthState::Unknown,
                       QStringLiteral("Diagnostic SMART non effectué.")});
    }
    return result;
}

SmartHealthResult parseSmartHealth(const QByteArray &json, int exitCode) {
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        return {HealthState::Unknown, QStringLiteral("Réponse SMART absente ou illisible.")};

    const QJsonObject root = document.object();
    const QJsonObject smart = root.value(QStringLiteral("smart_status")).toObject();
    if (smart.value(QStringLiteral("passed")).isBool()) {
        if (!smart.value(QStringLiteral("passed")).toBool())
            return {HealthState::Warning, QStringLiteral("Le périphérique signale un échec SMART.")};
        if ((exitCode & 0x08) != 0)
            return {HealthState::Warning, QStringLiteral("SMART signale un état d'échec malgré le résultat JSON.")};
        if (exitCode != 0)
            return {HealthState::Unknown, QStringLiteral("SMART indique un état global positif, mais la commande a aussi signalé une erreur.")};
        return {HealthState::Healthy, QStringLiteral("Aucun échec SMART global signalé.")};
    }

    const QJsonObject nvme = root.value(QStringLiteral("nvme_smart_health_information_log")).toObject();
    const QJsonValue critical = nvme.value(QStringLiteral("critical_warning"));
    if (critical.isDouble()) {
        if (critical.toInt() != 0)
            return {HealthState::Warning, QStringLiteral("Le contrôleur NVMe signale un avertissement critique.")};
        if (exitCode != 0)
            return {HealthState::Unknown, QStringLiteral("NVMe ne signale pas d'avertissement critique, mais la commande SMART a aussi signalé une erreur.")};
        return {HealthState::Healthy, QStringLiteral("Aucun avertissement critique NVMe signalé.")};
    }

    if ((exitCode & 0x08) != 0)
        return {HealthState::Warning, QStringLiteral("smartctl signale un échec ; les détails du contrôleur ne sont pas disponibles.")};
    if (exitCode != 0)
        return {HealthState::Unknown, QStringLiteral("Lecture SMART impossible ou incomplète avec les droits de cette session.")};
    return {HealthState::Unknown, QStringLiteral("Ce contrôleur ne fournit pas de résultat de santé interprétable.")};
}
