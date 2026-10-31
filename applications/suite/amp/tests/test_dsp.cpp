#include "dsp.h"
#include "processed_audio.h"
#include <QtTest>
#include <cstring>

class DspTests final : public QObject {
    Q_OBJECT
    static std::vector<float> sine(int channels=1) {
        std::vector<float> x(48000*channels);
        for(int i=0;i<48000;++i)x[i*channels]=float(.05*std::sin(2*3.141592653589793*1000*i/48000));
        return x;
    }
    static double rms(const std::vector<float> &x) {double s=0;for(std::size_t i=4096;i<x.size();++i)s+=x[i]*x[i];return std::sqrt(s/(x.size()-4096));}
private slots:
    void neutralAndBypass() {
        auto x=sine(),original=x;Amp::Equalizer eq;eq.configure(48000,1);eq.process(x.data(),x.size());QVERIFY(x==original);
        eq.setBand(5,12);eq.setEnabled(false);eq.process(x.data(),x.size());QVERIFY(x==original);
    }
    void gainAndPreamp() {
        auto x=sine();const double before=rms(x);Amp::Equalizer eq;eq.configure(48000,1);eq.setBand(5,6);eq.process(x.data(),x.size());
        const double gain=20*std::log10(rms(x)/before);QVERIFY2(gain>5.8 && gain<6.2,qPrintable(QString::number(gain)));
        x=sine();eq.setBand(5,0);eq.reset();eq.setPreamp(-6);eq.process(x.data(),x.size());const double reduction=20*std::log10(rms(x)/before);QVERIFY(reduction>-6.1 && reduction< -5.9);
    }
    void channelsAndImpulse() {
        auto x=sine(2);Amp::Equalizer eq;eq.configure(48000,2);eq.setBand(5,12);eq.process(x.data(),x.size()/2);
        for(std::size_t i=1;i<x.size();i+=2)QCOMPARE(x[i],0.f);
        std::vector<float> impulse(96000);impulse[0]=.2f;eq.reset();eq.process(impulse.data(),48000);for(float value:impulse)QVERIFY(std::isfinite(value) && std::abs(value)<=1);
    }
    void safetyLimiter() {
        std::vector<float> x(1024,2.f);Amp::Equalizer eq;eq.configure(48000,1);eq.process(x.data(),x.size());for(float value:x)QVERIFY(value<=1 && value>=-1);QVERIFY(eq.limitedSamples()>0);
    }
    void actualOutputDeviceProcessesSamples() {
        Amp::SampleDevice d;d.channels=1;d.rate=48000;d.eq.configure(48000,1);d.eq.setPreamp(-6);d.samples=QVector<float>(64,.2f);d.open(QIODevice::ReadOnly);
        const auto bytes=d.read(64*sizeof(float));QCOMPARE(bytes.size(),64*qsizetype(sizeof(float)));float sample=0;std::memcpy(&sample,bytes.constData(),sizeof(float));QVERIFY(sample>.099f && sample<.102f);QVERIFY(!d.visual.isEmpty());
        d.seekMs(0);QCOMPARE(d.cursor,qint64(0));
    }
};
QTEST_APPLESS_MAIN(DspTests)
#include "test_dsp.moc"
