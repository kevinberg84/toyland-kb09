#pragma once
#include <algorithm>
namespace kb09_gui {
// Targets are integer detents. Repeated drag samples for the same target must
// never restart the clock or strand the display at an intermediate angle.
class DetentSnapMotion {
public:
    static constexpr double durationMs=20.;
    void reset(int detent) {from=double(detent);target=detent;ready=true;moving=false;}
    void retarget(int detent,double now) {
        if(!ready){reset(detent);return;}
        if(detent==target)return;
        from=sample(now);target=detent;started=now;moving=from!=double(target);
    }
    double sample(double now) {
        if(!moving)return double(target);
        const double t=std::clamp((now-started)/durationMs,0.,1.);
        if(t>=1.){moving=false;from=double(target);return double(target);}
        return from+(double(target)-from)*t;
    }
    bool active() const {return moving;}
private:
    double from=0,started=0;int target=0;bool ready=false,moving=false;
};
}
