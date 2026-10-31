#pragma once
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QFileInfo>
#include <QSaveFile>
#include <QPainter>

namespace AccessoryImages {
inline QImage load(const QString &path, QString &error) {
    QImageReader reader(path); reader.setAutoTransform(true);
    const auto size = reader.size();
    if (!size.isValid() || qint64(size.width()) * size.height() > 16000000) {
        error = QStringLiteral("Image illisible ou supérieure à 16 millions de pixels."); return {};
    }
    auto image = reader.read();
    if (image.isNull()) error = reader.errorString();
    return image;
}
inline bool save(const QImage &source, const QString &path, QString &error) {
    auto format = QFileInfo(path).suffix().toLower().toLatin1();
    if (format == "jpg") format = "jpeg";
    if (source.isNull() || !QList<QByteArray>{"png", "jpeg", "bmp"}.contains(format)) {
        error = QStringLiteral("Image absente ou format non pris en charge (PNG, JPEG, BMP)."); return false;
    }
    QImage image = source;
    if (format == "jpeg" && source.hasAlphaChannel()) {
        image = QImage(source.size(), QImage::Format_RGB32); image.fill(Qt::white);
        QPainter painter(&image); painter.drawImage(0, 0, source);
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { error = file.errorString(); return false; }
    QImageWriter writer(&file, format);
    if (!writer.write(image)) { error = writer.errorString(); file.cancelWriting(); return false; }
    if (!file.commit()) { error = file.errorString(); return false; }
    return true;
}
}
