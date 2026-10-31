#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Amp {
// Ten independent peaking biquads per channel; samples remain floating point
// until the final safety limiter. No audio leaves this class without processing.
class Equalizer {
public:
    static constexpr std::array<double,10> frequencies{31.25,62.5,125,250,500,1000,2000,4000,8000,16000};
    void configure(int rate, int channels) { sampleRate=rate; channelCount=channels; states.assign(channels,{}); limited=0; update(); }
    void setEnabled(bool value) { enabled=value; reset(); }
    bool isEnabled() const { return enabled; }
    void setBand(int index, double db) { if(index>=0 && index<10) { gains[index]=std::clamp(db,-12.0,12.0); update(); } }
    void setPreamp(double db) { preamp=std::pow(10.0,std::clamp(db,-24.0,6.0)/20.0); }
    void reset() { for(auto &channel:states) channel={}; }
    std::uint64_t limitedSamples() const { return limited; }
    void process(float *samples, std::size_t frames) {
        for(std::size_t f=0;f<frames;++f) for(int c=0;c<channelCount;++c) {
            double x=samples[f*channelCount+c];
            if(enabled) {
                x*=preamp;
                for(int b=0;b<10;++b) {
                    auto &s=states[c][b];const auto &k=coefficients[b];
                    const double y=k.b0*x+s.z1;
                    s.z1=k.b1*x-k.a1*y+s.z2;s.z2=k.b2*x-k.a2*y;x=y;
                }
            }
            if(!std::isfinite(x)) { x=0; ++limited; }
            else if(std::abs(x)>1) { x=std::clamp(x,-1.0,1.0); ++limited; }
            samples[f*channelCount+c]=float(x);
        }
    }
private:
    struct Coeff { double b0=1,b1=0,b2=0,a1=0,a2=0; };
    struct State { double z1=0,z2=0; };
    void update() {
        for(int i=0;i<10;++i) {
            if(sampleRate<=0 || frequencies[i]>=sampleRate*.49 || gains[i]==0) { coefficients[i]={};continue; }
            const double a=std::pow(10.0,gains[i]/40.0),w=2*3.14159265358979323846*frequencies[i]/sampleRate;
            const double alpha=std::sin(w)/(2*1.4),norm=1+alpha/a;
            coefficients[i]={(1+alpha*a)/norm,-2*std::cos(w)/norm,(1-alpha*a)/norm,-2*std::cos(w)/norm,(1-alpha/a)/norm};
        }
    }
    int sampleRate=48000,channelCount=2;
    bool enabled=true;
    double preamp=1;
    std::array<double,10> gains{};
    std::array<Coeff,10> coefficients{};
    std::vector<std::array<State,10>> states{2};
    std::uint64_t limited=0;
};
}
