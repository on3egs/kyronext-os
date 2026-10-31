#pragma once
#include "processed_audio.h"
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>

class AudioVisualizer final : public QWidget {
public:
    explicit AudioVisualizer(Amp::ProcessedAudio *audio,QWidget *parent=nullptr):QWidget(parent),audio(audio) {
        setMinimumHeight(88);setAccessibleName(tr("Visualisation du signal audio DSP"));
        timer.setInterval(80);connect(&timer,&QTimer::timeout,this,qOverload<>(&QWidget::update));timer.start();
    }
    void setMode(int value) {mode=value;update();}
    void setActive(bool active) {setVisible(active);if(active)timer.start();else timer.stop();}
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.fillRect(rect(),QColor("#0a0e14"));
        const auto data=audio->device.snapshot();
        if(data.isEmpty() || !audio->playing()) {p.setPen(QColor("#9ba5b6"));p.drawText(rect(),Qt::AlignCenter,tr("Visualisations : signal DSP local uniquement"));return;}
        p.setRenderHint(QPainter::Antialiasing);p.setPen(QPen(QColor("#ef3c60"),2));
        double rms=0;for(float x:data)rms+=x*x;rms=std::sqrt(rms/data.size());
        if(mode==1) {
            QPolygonF points;for(int i=0;i<data.size();++i)points.append(QPointF(i*double(width())/(data.size()-1),height()*.5-data[i]*height()*.45));p.drawPolyline(points);
        } else if(mode==2 || mode==3) {
            const double level=std::clamp(rms*3.0,0.0,1.0);
            if(mode==2)p.fillRect(QRectF(8,16,(width()-16)*level,height()-32),QColor("#ef3c60"));
            else {const double w=(width()-16)*level;p.fillRect(QRectF((width()-w)/2,height()*.35,w,height()*.3),QColor("#ef3c60"));}
        } else {
            // Direct DFT of a bounded, windowed block. Frequencies derive from
            // decoded samples, not timers or random values.
            const int count=qMin(256,int(data.size()));
            for(int band=0;band<32;++band) {
                const int k=1+band*3;double real=0,imag=0;
                for(int n=0;n<count;++n) {const double window=.5-.5*std::cos(2*3.141592653589793*n/(count-1));const double angle=2*3.141592653589793*k*n/count;real+=data[n]*window*std::cos(angle);imag-=data[n]*window*std::sin(angle);}
                const double magnitude=std::sqrt(real*real+imag*imag)/(count*.5);
                const double level=std::clamp((20*std::log10(magnitude+1e-6)+60)/60,0.0,1.0);
                p.fillRect(QRectF(band*width()/32.0+2,height()*(1-level),width()/32.0-4,height()*level),QColor("#ef3c60"));
            }
        }
    }
private:
    Amp::ProcessedAudio *audio;QTimer timer;int mode=0;
};
