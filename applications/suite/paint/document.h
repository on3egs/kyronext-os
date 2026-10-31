#pragma once
#include <QImage>
#include <QColor>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QRect>
#include <QFont>

class PaintDocument {
public:
    static constexpr qint64 MaximumPixels = 4096LL * 4096;
    PaintDocument();
    bool create(int width, int height, QString *error = nullptr);
    bool load(const QString &file, QString *error = nullptr);
    bool save(const QString &file, QByteArray format, QString *error = nullptr);
    void beginStroke();
    void line(QPoint from, QPoint to, QColor color, int width, bool erase = false);
    void shape(QPoint from, QPoint to, QColor color, int width, int kind);
    void fill(QPoint position, QColor color);
    void text(QPoint position, const QString &value, QColor color, const QFont &font);
    bool crop(QRect area, QString *error = nullptr);
    bool resizeImage(QSize size, QString *error = nullptr);
    void rotateClockwise();
    bool undo();
    bool redo();
    const QImage &image() const { return current; }
    bool modified() const { return dirty; }
    bool canUndo() const { return !past.isEmpty(); }
    bool canRedo() const { return !future.isEmpty(); }
private:
    QImage current;
    QVector<QImage> past, future;
    bool dirty = false;
    void trimHistory();
};
