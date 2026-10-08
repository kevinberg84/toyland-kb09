#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace sh09 {
struct RateDivision { const char* label; double beats; };
// Quarter-note beats, ordered from slowest to fastest. D = dotted, T = triplet.
inline constexpr std::array<RateDivision,21> rateDivisions{{
 {"4 bars",16}, {"2 bars",8}, {"1 bar",4}, {"1/2 D",3}, {"1/2",2},
 {"1/4 D",1.5}, {"1/2 T",4.0/3}, {"1/4",1}, {"1/8 D",.75},
 {"1/4 T",2.0/3}, {"1/8",.5}, {"1/16 D",.375}, {"1/8 T",1.0/3},
 {"1/16",.25}, {"1/32 D",.1875}, {"1/16 T",1.0/6}, {"1/32",.125},
 {"1/64 D",.09375}, {"1/32 T",1.0/12}, {"1/64",.0625}, {"1/64 T",1.0/24}
}};
inline const RateDivision& rateDivision(double value) noexcept {
 const auto index=std::clamp(int(std::floor(std::clamp(value,0.0,1.0)*(rateDivisions.size()-1)+.5)),0,int(rateDivisions.size()-1));
 return rateDivisions[size_t(index)];
}
inline double lfoFrequency(double value,bool synced,double bpm) noexcept {
 return synced?bpm/(60*rateDivision(value).beats):.2*std::pow(125.0,value);
}
}
