#pragma once
#include <algorithm>
#include <cmath>
namespace kb09_gui {
class PowerLampFade {
public:
    explicit PowerLampFade(bool initiallyOn):on(initiallyOn),startLevel(initiallyOn?1.f:0.f){}
    static constexpr double durationMs=5950.;
    static float decay(double elapsedMs) {
        const double t=std::clamp(elapsedMs/durationMs,0.,1.);
        return float(std::expm1(2.*(1.-t))/std::expm1(2.));
    }
    bool update(bool enabled,double now) {
        if(enabled==on)return false;
        startLevel=enabled?1.f:level(now);offTime=now;on=enabled;return true;
    }
    float level(double now) const {return on?1.f:startLevel*decay(now-offTime);}
    bool active(double now) const {return !on&&startLevel>0&&now-offTime<durationMs;}
private:
    bool on;float startLevel;double offTime=0;
};
}
