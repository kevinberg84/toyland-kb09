#pragma once
#include <algorithm>
#include <cmath>
namespace kb09_gui {
inline float knobTroughDust(float phase,float relief) {
    phase=std::fmod(phase,5.f);if(phase<0)phase+=5;
    const float trough=std::round(phase);
    if(trough==4)return 0; // broad smooth separator
    const float edge=std::clamp((.185f-std::abs(phase-trough))/.035f,0.f,1.f);
    const float t=std::clamp((relief-.20f)/.08f,0.f,1.f);
    return edge*(1-t*t*(3-2*t)); // slightly narrower dust bands, with a narrow feather; upper flanks/crests stay clear
}
}
