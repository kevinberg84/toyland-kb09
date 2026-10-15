#pragma once
#include <algorithm>

namespace kb09_gui {
// Visual travel in ON-height units: OFF=2, ON=1, fully depressed=0.
// The audio power state is owned by the Button click, never this animation.
class PowerButtonTravel {
public:
    static constexpr double pressMs=12,returnMs=20;
    float sample(double now,float rest) {
        if(phase==Idle)return rest;
        if(phase==Down) {
            const float t=float(std::clamp((now-start)/pressMs,0.,1.));
            if(t<1)return from*(1-t);
            if(held){phase=Held;return 0;}
            phase=Up;start+=pressMs;
        }
        if(phase==Held)return 0;
        const float t=float(std::clamp((now-start)/returnMs,0.,1.));
        if(t>=1){phase=Idle;return rest;}
        return rest*(1-(1-t)*(1-t));
    }
    void press(double now,float rest) {from=sample(now,rest);held=true;phase=Down;start=now;}
    void release(double now) {held=false;if(phase==Down||phase==Held){phase=Up;start=now;}}
    bool isHeld() const{return held;}
    bool hasCycle() const{return phase!=Idle;}
    bool animating() const{return phase==Down||phase==Up;}
private:
    enum Phase{Idle,Down,Held,Up};Phase phase=Idle;
    double start=0;float from=2;bool held=false;
};
}
