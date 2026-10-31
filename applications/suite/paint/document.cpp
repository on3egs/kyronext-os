#include "document.h"
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QSaveFile>
#include <QTransform>

namespace {
bool fail(QString *error, const QString &message) { if (error) *error = message; return false; }
bool validSize(QSize size) { return size.width() > 0 && size.height() > 0 &&
    qint64(size.width()) * size.height() <= PaintDocument::MaximumPixels; }
}
PaintDocument::PaintDocument() { create(1024, 768); }
bool PaintDocument::create(int width, int height, QString *error) {
    if (!validSize({width, height})) return fail(error, "Dimensions invalides : maximum 16 mégapixels.");
    QImage replacement(width, height, QImage::Format_ARGB32_Premultiplied);
    if (replacement.isNull()) return fail(error, "Mémoire insuffisante pour cette image.");
    replacement.fill(Qt::white); current = replacement;
    past.clear(); future.clear(); dirty = false; return true;
}
bool PaintDocument::load(const QString &file, QString *error) {
    QImageReader::setAllocationLimit(128);
    QImageReader reader(file); reader.setAutoTransform(true);
    if (!validSize(reader.size())) return fail(error, "Image invalide ou supérieure à 16 mégapixels.");
    QImage replacement = reader.read();
    if (replacement.isNull()) return fail(error, reader.errorString());
    replacement = replacement.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (replacement.isNull()) return fail(error, "Mémoire insuffisante.");
    current = replacement; past.clear(); future.clear(); dirty = false; return true;
}
bool PaintDocument::save(const QString &file, QByteArray format, QString *error) {
    format = format.toLower();
    if (format != "png" && format != "jpg" && format != "jpeg" && format != "bmp")
        return fail(error, "Choisir un fichier PNG, JPEG ou BMP.");
    QImage output = current;
    if (format != "png") {
        output = QImage(current.size(), QImage::Format_RGB32); output.fill(Qt::white);
        QPainter flatten(&output); flatten.drawImage(0, 0, current); flatten.end();
    }
    QSaveFile destination(file);
    if (!destination.open(QIODevice::WriteOnly)) return fail(error, destination.errorString());
    QImageWriter writer(&destination, format); writer.setQuality(92);
    if (!writer.write(output)) { destination.cancelWriting(); return fail(error, writer.errorString()); }
    if (!destination.commit()) return fail(error, destination.errorString());
    dirty = false; return true;
}
void PaintDocument::trimHistory() {
    const auto bytes = qMax<qint64>(1, current.sizeInBytes());
    const int limit = qBound(1, int((32LL * 1024 * 1024) / bytes), 32);
    while (past.size() > limit) past.removeFirst();
    while (future.size() > limit) future.removeFirst();
}
void PaintDocument::beginStroke() { past.append(current); future.clear(); trimHistory(); }
void PaintDocument::line(QPoint from, QPoint to, QColor color, int width, bool erase) {
    QPainter painter(&current); painter.setRenderHint(QPainter::Antialiasing);
    if (erase) painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(QPen(color, qBound(1, width, 200), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(from, to); dirty = true;
}
bool PaintDocument::undo() {
    if (past.isEmpty()) return false;
    future.append(current); current = past.takeLast(); dirty = true; trimHistory(); return true;
}
bool PaintDocument::redo() {
    if (future.isEmpty()) return false;
    past.append(current); current = future.takeLast(); dirty = true; trimHistory(); return true;
}

void PaintDocument::shape(QPoint from, QPoint to, QColor color, int width, int kind) {
    beginStroke(); QPainter painter(&current); painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, qBound(1,width,200), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    const QRect rect=QRect(from,to).normalized();
    if(kind==1) painter.drawRect(rect); else if(kind==2) painter.drawEllipse(rect); else painter.drawLine(from,to);
    dirty=true;
}
void PaintDocument::fill(QPoint position, QColor color) {
    if(!current.rect().contains(position)) return;
    const QRgb old=current.pixel(position); const QRgb replacement=qPremultiply(color.rgba());
    if(old==replacement) return;
    beginStroke();
    // Scanline flood fill uses short spans rather than a pixel-sized queue.
    QVector<QPoint> pending{position};
    while(!pending.isEmpty()) {
        const QPoint p=pending.takeLast();
        if(!current.rect().contains(p) || current.pixel(p)!=old) continue;
        int left=p.x(), right=p.x();
        while(left>0 && current.pixel(left-1,p.y())==old) --left;
        while(right+1<current.width() && current.pixel(right+1,p.y())==old) ++right;
        for(int x=left;x<=right;++x) current.setPixel(x,p.y(),replacement);
        for(int y : {p.y()-1,p.y()+1}) {
            if(y<0 || y>=current.height()) continue;
            bool span=false;
            for(int x=left;x<=right;++x) {
                const bool matching=current.pixel(x,y)==old;
                if(matching && !span) pending.append({x,y});
                span=matching;
            }
        }
    }
    dirty=true;
}
void PaintDocument::text(QPoint position,const QString &value,QColor color,const QFont &font) {
    if(value.isEmpty()) return;
    beginStroke(); QPainter painter(&current); painter.setPen(color); painter.setFont(font);
    painter.drawText(position,value); dirty=true;
}
bool PaintDocument::crop(QRect area,QString *error) {
    area=area.normalized().intersected(current.rect());
    if(area.isEmpty()) return fail(error,"Selection vide.");
    QImage replacement=current.copy(area);
    if(replacement.isNull()) return fail(error,"Memoire insuffisante.");
    beginStroke(); current=replacement; dirty=true; trimHistory(); return true;
}
bool PaintDocument::resizeImage(QSize size,QString *error) {
    if(!validSize(size)) return fail(error,"Dimensions invalides : maximum 16 megapixels.");
    QImage replacement=current.scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    if(replacement.isNull()) return fail(error,"Memoire insuffisante.");
    beginStroke(); current=replacement; dirty=true; trimHistory(); return true;
}
void PaintDocument::rotateClockwise() {
    const QImage replacement=current.transformed(QTransform().rotate(90));
    if(replacement.isNull()) return;
    beginStroke(); current=replacement; dirty=true; trimHistory();
}
