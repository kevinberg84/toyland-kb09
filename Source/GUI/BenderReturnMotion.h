#pragma once
#include <cmath>

namespace kb09_gui {
// The recoil is visual only: audio reaches exact neutral after the fast stroke.
// One small lobe follows, then exact rest. Amplitude scales with release travel.
inline double benderReturnPosition(double from,double elapsedMs) noexcept {
    if(elapsedMs<=0)return from;
    if(elapsedMs<35.0)return from*(1.0-elapsedMs/35.0);
    if(elapsedMs>=55.0)return 0;
    return -from*.02*std::sin((elapsedMs-35.0)/20.0*3.14159265358979323846);
}
}
