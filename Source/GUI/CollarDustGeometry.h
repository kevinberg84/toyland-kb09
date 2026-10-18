#pragma once
#include <array>
#include <cmath>
#include <algorithm>

namespace kb09_gui {
template<size_t N>
float collarRadiusAt(const std::array<float,N>& radii,float angle) {
    constexpr float tau=6.2831853071795864769f;
    float wrapped=std::fmod(angle,tau);if(wrapped<0)wrapped+=tau;
    const float phase=wrapped*float(N)/tau;
    const auto index=std::min(N-1,size_t(phase));
    const float blend=phase-float(index);
    return radii[index]+(radii[(index+1)%N]-radii[index])*blend;
}
}
