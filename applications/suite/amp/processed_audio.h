#pragma once
#include "dsp.h"
#include <QObject>
#include <QAudioDecoder>
#include <QAudioSink>
#include <QIODevice>
#include <QTimer>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

namespace Amp {
class SampleDevice final : public QIODevice {
public:
    explicit SampleDevice(QObject *parent=nullptr):QIODevice(parent) {}
    QVector<float> samples,visual;
    Equalizer eq;
    int rate=48000,channels=2;
    qint64 cursor=0;
    bool isSequential() const override {return true;}
    bool atEnd() const override {QMutexLocker lock(&mutex);return cursor>=samples.size();}
    qint64 bytesAvailable() const override {QMutexLocker lock(&mutex);return (samples.size()-cursor)*qint64(sizeof(float))+QIODevice::bytesAvailable();}
    void seekMs(qint64 ms) {QMutexLocker lock(&mutex);cursor=qBound<qint64>(0,ms*rate/1000*channels,samples.size());eq.reset();}
    void setBand(int band,double gain) {QMutexLocker lock(&mutex);eq.setBand(band,gain);}
    void setPreamp(double gain) {QMutexLocker lock(&mutex);eq.setPreamp(gain);}
    void setEqEnabled(bool enabled) {QMutexLocker lock(&mutex);eq.setEnabled(enabled);}
    QVector<float> snapshot() const {QMutexLocker lock(&mutex);return visual;}
    qint64 positionMs() const {QMutexLocker lock(&mutex);return cursor/channels*1000/rate;}
    std::uint64_t limitedCount() const {QMutexLocker lock(&mutex);return eq.limitedSamples();}
protected:
    qint64 readData(char *data,qint64 max) override;
    qint64 writeData(const char *,qint64) override {return -1;}
private:
    mutable QMutex mutex;
};
class ProcessedAudio final : public QObject {
    Q_OBJECT
public:
    explicit ProcessedAudio(QObject *parent=nullptr);
    ~ProcessedAudio() override {stop();}
    void load(const QUrl &url);
    void play(); void pause(); void stop(); void seek(qint64 ms);
    void setVolume(float v) {volume=qBound(0.f,v,1.f);if(sink)sink->setVolume(muted?0:volume);}
    void setMuted(bool v) {muted=v;if(sink)sink->setVolume(muted?0:volume);}
    SampleDevice device;
    qint64 duration() const {return device.rate?device.samples.size()/device.channels*1000/device.rate:0;}
    bool playing() const;
signals:
    void ready();void failed(const QString &message);void ended();void positionChanged(qint64 position);
private:
    void appendBuffer(); void finishDecode(); void reportSinkError();
    QAudioDecoder decoder;
    QAudioSink *sink=nullptr;
    QTimer timer;
    QAudioFormat sourceFormat;
    float volume=.65f; bool muted=false,decoding=false,finished=false,failurePending=false;
};
}
