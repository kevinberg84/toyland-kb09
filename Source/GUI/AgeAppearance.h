#pragma once
#include <array>
#include <algorithm>
#include <cmath>

namespace kb09_gui {
inline constexpr std::array<const char*,5> ageNames{{
    "Minty","Seen Some Shit","Vintage, Baby!","Sold As-Is","Fire Hazard"}};
inline int appearanceStage(float value) noexcept {
    return int(std::lround(std::clamp(value,0.f,1.f)*4.f));
}
inline float appearanceAge(float value) noexcept {return float(appearanceStage(value))*.25f;}
}
