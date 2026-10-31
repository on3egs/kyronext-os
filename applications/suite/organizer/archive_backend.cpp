#include "archive_backend.h"
#include <KArchive>
#include <KArchiveDirectory>
#include <KArchiveFile>
#include <KZip>
#include <KTar>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QSet>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QtEndian>
#include <memory>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <linux/fs.h>

namespace {
bool fail(QString *error,const QString &text){if(error)*error=text;return false;}
QString normalized(QString name){while(name.startsWith("./"))name.remove(0,2);while(name.endsWith('/'))name.chop(1);return name;}
quint16 u16(const QByteArray &b,int p){return qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(b.constData()+p));}
quint32 u32(const QByteArray &b,int p){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(b.constData()+p));}
bool inspectNames(const QByteArray &b,bool zip,QString *error) {
    QSet<QString> names;quint64 total=0;int count=0;
    auto entry=[&](QString name,quint64 bytes,bool directory){
        if(!SafeArchive::safeName(name))return fail(error,"Nom interne dangereux ou non pris en charge.");
        name=normalized(name);if(name.isEmpty() || name==".")return true;
        if(names.contains(name))return fail(error,"Entree dupliquee dans l'archive.");names.insert(name);
        if(++count>SafeArchive::MaximumEntries || bytes>quint64(SafeArchive::MaximumFileBytes))return fail(error,"Limite de taille ou de nombre d'entrees depassee.");
        if(!directory)total+=bytes;
        return total<=quint64(SafeArchive::MaximumTotalBytes) || fail(error,"Archive trop volumineuse une fois extraite.");
    };
    if(zip) {
        int end=-1;for(int p=b.size()-22;p>=qMax(0,int(b.size())-65557);--p)
            if(b.mid(p,4)==QByteArray("PK\5\6",4) && p+22+u16(b,p+20)==b.size()){end=p;break;}
        if(end<0)return fail(error,"Repertoire ZIP invalide.");
        if(u16(b,end+4)!=0 || u16(b,end+6)!=0 || u16(b,end+8)!=u16(b,end+10))return fail(error,"ZIP multi-volume non pris en charge.");
        const auto entries=u16(b,end+10);const auto offset=u32(b,end+16),length=u32(b,end+12);
        if(entries==65535 || offset==0xffffffff || length==0xffffffff)return fail(error,"ZIP64 non pris en charge dans cette version.");
        if(entries>SafeArchive::MaximumEntries || quint64(offset)+length>quint64(end))return fail(error,"Repertoire ZIP hors limites.");
        int p=int(offset);
        for(int n=0;n<entries;++n) {
            if(p+46>end || b.mid(p,4)!=QByteArray("PK\1\2",4))return fail(error,"Entree ZIP invalide.");
            const int nameSize=u16(b,p+28),extra=u16(b,p+30),comment=u16(b,p+32);
            if(p+46+nameSize+extra+comment>end)return fail(error,"Entree ZIP tronquee.");
            const auto raw=b.mid(p+46,nameSize);const QString name=QString::fromUtf8(raw);
            const quint32 local=u32(b,p+42);
            if(u16(b,p+34)!=0 || (u16(b,p+8)&1)!=0 || quint64(local)+30>quint64(offset) || b.mid(local,4)!=QByteArray("PK\3\4",4))return fail(error,"Entete ZIP locale invalide ou archive chiffree.");
            const auto localName=u16(b,local+26),localExtra=u16(b,local+28);
            if(quint64(local)+30+localName+localExtra>quint64(offset) || b.mid(local+30,localName)!=raw || u16(b,local+8)!=u16(b,p+10))return fail(error,"Noms ou compression ZIP incoherents.");
            const auto mode=u32(b,p+38)>>16;
            if((mode&S_IFMT)==S_IFLNK || ((mode&S_IFMT)!=0 && (mode&S_IFMT)!=S_IFREG && (mode&S_IFMT)!=S_IFDIR))return fail(error,"Liens et fichiers speciaux refuses.");
            if(!entry(name,u32(b,p+24),name.endsWith('/')))return false;
            p+=46+nameSize+extra+comment;
        }
        if(quint64(p)!=quint64(offset)+length)return fail(error,"Repertoire ZIP incoherent.");
    } else {
        qint64 p=0;
        while(p+512<=b.size()) {
            const auto h=b.mid(p,512);if(h==QByteArray(512,'\0'))break;
            const char type=h[156];if(type!='0' && type!='\0' && type!='5')return fail(error,"Liens, extensions TAR et fichiers speciaux refuses.");
            const auto field=[](QByteArray value){const int zero=value.indexOf('\0');return zero<0?value:value.left(zero);};
            QString name=QString::fromUtf8(field(h.left(100)));const auto prefix=field(h.mid(345,155));
            if(!prefix.isEmpty())name=QString::fromUtf8(prefix)+"/"+name;
            bool ok=false;const quint64 size=field(h.mid(124,12)).trimmed().toULongLong(&ok,8);
            if(!ok || !entry(name,size,type=='5'))return fail(error,"Entree TAR invalide ou hors limites.");
            const quint64 next=quint64(p)+512+((size+511)/512)*512;if(next>quint64(b.size()))return fail(error,"TAR tronque.");p=qint64(next);
        }
    }
    return true;
}
std::unique_ptr<KArchive> engine(const QString &path,QIODevice *device,QString *error) {
    if(path.endsWith(".zip",Qt::CaseInsensitive))return std::make_unique<KZip>(device);
    if(path.endsWith(".tar",Qt::CaseInsensitive))return std::make_unique<KTar>(device);
    fail(error,"Choisir une archive ZIP ou TAR non compressee.");return {};
}
bool load(const QString &path,QByteArray *data,QString *error) {
    if(!QFileInfo(path).isFile() || QFileInfo(path).isSymLink())return fail(error,"Choisir un fichier archive ordinaire, sans lien.");
    QFile file(path);if(!file.open(QIODevice::ReadOnly))return fail(error,file.errorString());
    if(file.size()>SafeArchive::MaximumArchiveBytes)return fail(error,"Archive depassant 64 Mio refusee.");
    *data=file.read(SafeArchive::MaximumArchiveBytes+1);if(data->size()>SafeArchive::MaximumArchiveBytes)return fail(error,"Archive trop grande.");
    return inspectNames(*data,path.endsWith(".zip",Qt::CaseInsensitive),error);
}
bool walk(const KArchiveDirectory *directory,const QString &prefix,QVector<ArchiveItem> *items,QString *error,int depth=0) {
    if(depth>64)return fail(error,"Arborescence trop profonde.");
    for(const auto &name:directory->entries()) {
        const auto *entry=directory->entry(name);const QString path=prefix+name;
        if(!entry || !SafeArchive::safeName(path) || !entry->symLinkTarget().isEmpty())return fail(error,"Nom ou lien refuse.");
        if(items->size()>=SafeArchive::MaximumEntries)return fail(error,"Trop d'entrees.");
        if(entry->isDirectory()){items->append({path,0,true});if(!walk(static_cast<const KArchiveDirectory*>(entry),path+"/",items,error,depth+1))return false;}
        else if(entry->isFile()){const auto *file=static_cast<const KArchiveFile*>(entry);if(file->size()<0 || file->size()>SafeArchive::MaximumFileBytes)return fail(error,"Fichier interne trop grand.");items->append({path,file->size(),false});}
        else return fail(error,"Type de fichier refuse.");
    }
    return true;
}
bool commitNew(const QString &from,const QString &to,QString *error) {
    const auto source=QFile::encodeName(from),destination=QFile::encodeName(to);
    if(syscall(SYS_renameat2,AT_FDCWD,source.constData(),AT_FDCWD,destination.constData(),RENAME_NOREPLACE)!=0)
        return fail(error,QString::fromLocal8Bit(std::strerror(errno)));
    return true;
}
}
bool SafeArchive::safeName(const QString &name) {
    if(name.isEmpty() || name.size()>1024 || name.startsWith('/') || name.contains('\\') || (name.size()>1 && name[1]==':'))return false;
    for(const auto ch:name)if(ch.unicode()<32 || ch.unicode()==127)return false;
    const auto parts=name.split('/');if(parts.size()>64)return false;
    for(const auto &part:parts)if(part=="..")return false;
    return true;
}
bool SafeArchive::list(const QString &path,QVector<ArchiveItem> *items,QString *error) {
    items->clear();QByteArray data;if(!load(path,&data,error))return false;QBuffer buffer(&data);auto archive=engine(path,&buffer,error);
    if(!archive || !archive->open(QIODevice::ReadOnly))return fail(error,"Archive illisible.");
    if(!walk(archive->directory(),{},items,error)){items->clear();return false;}return true;
}
bool SafeArchive::extract(const QString &path,const QString &destination,QString *error) {
    const QFileInfo output(destination);if(output.exists() || output.isSymLink())return fail(error,"La destination existe deja : aucun ecrasement.");
    if(!QFileInfo(output.absolutePath()).isDir())return fail(error,"Dossier parent absent.");
    QByteArray data;if(!load(path,&data,error))return false;QBuffer buffer(&data);auto archive=engine(path,&buffer,error);
    if(!archive || !archive->open(QIODevice::ReadOnly))return fail(error,"Archive illisible.");
    QVector<ArchiveItem> items;if(!walk(archive->directory(),{},&items,error))return false;
    QTemporaryDir staging(output.absolutePath()+"/.kyronext-extract-XXXXXX");if(!staging.isValid())return fail(error,"Creation du dossier temporaire impossible.");
    qint64 total=0;
    for(const auto &item:items) {
        const QString target=staging.filePath(item.name);
        if(item.directory){if(!QDir().mkpath(target))return fail(error,"Creation du dossier impossible.");QFile::setPermissions(target,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner);continue;}
        if(!QDir().mkpath(QFileInfo(target).absolutePath()))return fail(error,"Creation du dossier impossible.");
        const auto *entry=archive->directory()->entry(item.name);if(!entry || !entry->isFile())return fail(error,"Entree incoherente.");
        std::unique_ptr<QIODevice> source(static_cast<const KArchiveFile*>(entry)->createDevice());if(!source)return fail(error,"Lecture impossible.");
        QFile out(target);if(!out.open(QIODevice::WriteOnly|QIODevice::NewOnly))return fail(error,out.errorString());out.setPermissions(QFile::ReadOwner|QFile::WriteOwner);
        qint64 written=0;while(!source->atEnd()) {
            const auto chunk=source->read(65536);if(chunk.isEmpty() && !source->atEnd())return fail(error,"Erreur de decompression.");written+=chunk.size();total+=chunk.size();
            if(written>item.size || written>MaximumFileBytes || total>MaximumTotalBytes)return fail(error,"Taille decompressee hors limites.");
            if(out.write(chunk)!=chunk.size())return fail(error,out.errorString());
        }
        if(written!=item.size)return fail(error,"Taille decompressee incoherente.");out.close();
    }
    if(!commitNew(staging.path(),output.absoluteFilePath(),error))return false;staging.setAutoRemove(false);return true;
}
bool SafeArchive::create(const QString &directory,const QString &destination,QString *error) {
    const QFileInfo source(directory),output(destination);if(!source.isDir() || source.isSymLink() || source.canonicalFilePath()=="/")return fail(error,"Choisir un dossier local ordinaire.");
    if(output.exists() || output.isSymLink())return fail(error,"L'archive existe deja : aucun ecrasement.");
    if(output.absoluteFilePath().startsWith(source.canonicalFilePath()+"/"))return fail(error,"Choisir une destination hors du dossier source.");
    QVector<QPair<QString,QString>> files;QVector<QString> folders;qint64 total=0;int count=0;
    QDirIterator iterator(source.absoluteFilePath(),QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System,QDirIterator::Subdirectories);
    while(iterator.hasNext()) {
        const auto path=iterator.next();const auto info=iterator.fileInfo();const auto name=QDir(source.absoluteFilePath()).relativeFilePath(path);
        if(info.isSymLink() || (!info.isFile() && !info.isDir()) || !safeName(name) || name.toUtf8().size()>100)return fail(error,"Lien, type special ou nom trop long refuse.");
        if(++count>MaximumEntries)return fail(error,"Trop d'entrees.");if(info.isDir())folders.append(name);else{total+=info.size();if(info.size()>MaximumFileBytes || total>MaximumTotalBytes)return fail(error,"Dossier trop volumineux.");files.append({path,name});}
    }
    QTemporaryFile temporary(output.absolutePath()+"/.kyronext-archive-XXXXXX");if(!temporary.open())return fail(error,"Creation temporaire impossible.");
    auto archive=engine(destination,&temporary,error);if(!archive || !archive->open(QIODevice::WriteOnly))return fail(error,"Creation d'archive impossible.");
    for(const auto &name:folders)if(!archive->writeDir(name,{},{}))return fail(error,"Ecriture du dossier impossible.");
    qint64 actualTotal=0;
    for(const auto &pair:files) {
        QFile input(pair.first);if(QFileInfo(pair.first).isSymLink() || !input.open(QIODevice::ReadOnly))return fail(error,"Source modifiee ou illisible.");
        const qint64 expected=input.size();const auto data=input.read(MaximumFileBytes+1);
        if(input.error()!=QFileDevice::NoError || data.size()!=expected || input.size()!=expected)return fail(error,"Source modifiee ou erreur de lecture.");
        actualTotal+=data.size();if(data.size()>MaximumFileBytes || actualTotal>MaximumTotalBytes)return fail(error,"Source trop grande.");
        if(!archive->writeFile(pair.second,data))return fail(error,"Ecriture de l'archive impossible.");
    }
    if(!archive->close())return fail(error,"Finalisation impossible.");
    if(QFileInfo(temporary.fileName()).size()>MaximumArchiveBytes)return fail(error,"Archive finale depassant 64 Mio.");
    return commitNew(temporary.fileName(),output.absoluteFilePath(),error);
}
