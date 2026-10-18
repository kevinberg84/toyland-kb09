#pragma once
#include <algorithm>
#include <cmath>
#include <string_view>

namespace sh09::controlResponse {
// Stored/automated values remain effect amounts. Only physical GUI/MIDI
// positions are tapered, preserving the sound of existing patches/projects.
inline bool squared(std::string_view id) noexcept {
    return id=="portamento"||id=="vcoMod"||id=="vcfMod"||id=="vcfEnv";
}
inline double unit(double v) noexcept {return std::isfinite(v)?std::clamp(v,0.0,1.0):0.0;}
inline double effect(std::string_view id,double position) noexcept {
    position=unit(position);return squared(id)?position*position:position;
}
inline double position(std::string_view id,double effectValue) noexcept {
    effectValue=unit(effectValue);return squared(id)?std::sqrt(effectValue):effectValue;
}
}
