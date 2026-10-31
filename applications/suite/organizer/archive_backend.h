#pragma once
#include <QString>
#include <QVector>
struct ArchiveItem { QString name; qint64 size=0; bool directory=false; };
namespace SafeArchive {
constexpr qint64 MaximumArchiveBytes=64LL*1024*1024;
constexpr qint64 MaximumFileBytes=64LL*1024*1024;
constexpr qint64 MaximumTotalBytes=256LL*1024*1024;
constexpr int MaximumEntries=10000;
bool safeName(const QString &name);
bool list(const QString &archive,QVector<ArchiveItem> *items,QString *error);
bool extract(const QString &archive,const QString &newDirectory,QString *error);
bool create(const QString &sourceDirectory,const QString &newArchive,QString *error);
}
