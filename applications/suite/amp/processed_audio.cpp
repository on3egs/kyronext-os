#include "processed_audio.h"
#include <QAudioBuffer>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QPointer>
#include <cstring>
#include <limits>

namespace Amp {
qint64 SampleDevice::readData(char *data,qint64 maximum) {
    QMutexLocker lock(&mutex);
    const qint64 count=qMin<qint64>(qMin<qint64>(maximum/sizeof(float)/channels*channels,4096*channels),samples.size()-cursor);
    if(count<=0)return 0;
    QVector<float> chunk(count);
    std::memcpy(chunk.data(),samples.constData()+cursor,std::size_t(count)*sizeof(float));
    eq.process(chunk.data(),std::size_t(count/channels));
    std::memcpy(data,chunk.constData(),std::size_t(count)*sizeof(float));
    const auto *p=chunk.constData();
    visual.clear();
    const qint64 frames=count/channels;
    const qint64 start=qMax<qint64>(0,frames-512);
    for(qint64 f=start;f<frames;++f) {float sum=0;for(int c=0;c<channels;++c)sum+=p[f*channels+c];visual.append(sum/channels);}
    cursor+=count;return count*sizeof(float);
}
ProcessedAudio::ProcessedAudio(QObject *parent):QObject(parent),device(this) {
    connect(&decoder,&QAudioDecoder::bufferReady,this,&ProcessedAudio::appendBuffer);
    connect(&decoder,&QAudioDecoder::finished,this,&ProcessedAudio::finishDecode);
    connect(&decoder,qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),this,[this](QAudioDecoder::Error){
        if(decoding){decoding=false;emit failed(decoder.errorString());}
    });
    timer.setInterval(80);
    connect(&timer,&QTimer::timeout,this,[this]{
        if(!sink)return;
        emit positionChanged(device.positionMs());
        if(sink->state()==QAudio::IdleState && device.atEnd() && !finished){finished=true;timer.stop();emit ended();}
    });
}
void ProcessedAudio::load(const QUrl &url) {
    stop();decoder.stop();device.samples.clear();device.visual.clear();sourceFormat={};decoding=true;finished=false;failurePending=false;
    decoder.setSource(url);decoder.start();
}
void ProcessedAudio::appendBuffer() {
    const auto buffer=decoder.read();if(!buffer.isValid() || !decoding)return;
    const auto format=buffer.format();
    if(!sourceFormat.isValid())sourceFormat=format;
    if(format!=sourceFormat || format.channelCount()<1 || format.channelCount()>8 || format.sampleRate()<8000) {
        decoding=false;decoder.stop();emit failed(tr("Format PCM non pris en charge par le traitement DSP."));return;
    }
    const qint64 count=buffer.sampleCount();
    constexpr qint64 maxSamples=128*1024*1024/sizeof(float);
    if(count>maxSamples-device.samples.size()) {decoding=false;decoder.stop();device.samples.clear();emit failed(tr("Limite DSP de 128 Mio atteinte ; utilisez la lecture standard."));return;}
    const auto old=device.samples.size();device.samples.resize(old+count);
    for(qint64 i=0;i<count;++i) {
        float value=0;
        switch(format.sampleFormat()) {
        case QAudioFormat::UInt8:value=(int(buffer.constData<quint8>()[i])-128)/128.f;break;
        case QAudioFormat::Int16:value=buffer.constData<qint16>()[i]/32768.f;break;
        case QAudioFormat::Int32:value=float(double(buffer.constData<qint32>()[i])/2147483648.0);break;
        case QAudioFormat::Float:value=buffer.constData<float>()[i];break;
        default:decoding=false;decoder.stop();emit failed(tr("Échantillons PCM non reconnus."));return;
        }
        device.samples[old+i]=value;
    }
}
void ProcessedAudio::finishDecode() {
    if(!decoding)return;decoding=false;
    if(device.samples.isEmpty()){emit failed(tr("Aucun échantillon audio décodé."));return;}
    device.rate=sourceFormat.sampleRate();device.channels=sourceFormat.channelCount();device.cursor=0;
    device.eq.configure(device.rate,device.channels);
    QAudioFormat format;format.setSampleRate(device.rate);format.setChannelCount(device.channels);format.setSampleFormat(QAudioFormat::Float);
    const auto output=QMediaDevices::defaultAudioOutput();
    if(output.isNull() || !output.isFormatSupported(format)){emit failed(tr("La sortie audio ne prend pas en charge le PCM flottant de ce fichier. Lecture standard disponible."));return;}
    sink=new QAudioSink(output,format,this);sink->setVolume(muted?0:volume);
    connect(sink,&QAudioSink::stateChanged,this,[this](QAudio::State state){if(state==QAudio::StoppedState && sink && sink->error()!=QAudio::NoError)reportSinkError();});
    device.open(QIODevice::ReadOnly);emit ready();play();
}
bool ProcessedAudio::playing() const {return sink && sink->state()==QAudio::ActiveState;}
void ProcessedAudio::play() {if(!sink)return;finished=false;if(sink->state()==QAudio::SuspendedState)sink->resume();else {if(device.atEnd())device.seekMs(0);sink->start(&device);}if(sink->error()!=QAudio::NoError)reportSinkError();timer.start();}
void ProcessedAudio::reportSinkError() {
    if(failurePending || !sink)return;failurePending=true;
    const QPointer<QAudioSink> failedSink(sink);
    QTimer::singleShot(0,this,[this,failedSink]{if(failedSink && sink==failedSink.data())emit failed(tr("La sortie audio DSP a échoué ; vérifiez le périphérique et ses permissions."));});
}
void ProcessedAudio::pause() {if(sink)sink->suspend();timer.stop();}
void ProcessedAudio::stop() {decoding=false;decoder.stop();timer.stop();if(sink){sink->stop();delete sink;sink=nullptr;}device.close();device.cursor=0;device.eq.reset();}
void ProcessedAudio::seek(qint64 ms) {if(!sink)return;const bool resume=playing();sink->reset();device.close();device.seekMs(ms);device.open(QIODevice::ReadOnly);sink->start(&device);if(!resume)sink->suspend();}
}
