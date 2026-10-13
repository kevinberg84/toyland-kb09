#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace sh09::detail {
// Reduced OTA/feedback model, not a complete BA662 transistor model. State
// normalization preserves unity small-signal input/output gain. Later-stage
// gain ratios follow the 100k local-feedback / 68k interstage resistors.
// Absolute headroom and diode scales are engineering calibration parameters.
inline double otaSaturation(double x) noexcept {
 if(x>=3)return 1;if(x<=-3)return -1;
 const double xx=x*x;return x*(27+xx)/(27+9*xx);
}
inline double otaDerivative(double x) noexcept {
 if(std::abs(x)>=3)return 0;
 const double xx=x*x,d=27+9*xx;return 9*(xx-9)*(xx-9)/(d*d);
}

inline double resonantPitchCorrection(double resonance) noexcept {
 // Calibrate out the change in nonlinear oscillation frequency. Below onset
 // the pole tuning is untouched. Linear interpolation is continuous in RES;
 // cutoff itself is never quantized or snapped to pitches.
 struct Point {double resonance,factor;};
 constexpr std::array<Point,9> points{{
  {4.0/4.65,1.0},{.87,.9973258164208164},{.88,.9851606345400584},
  {.90,.9630091337848791},{.92,.9440932892647041},
  {.94,.9277532982307272},{.96,.9134203213141073},
  {.98,.9006798434332427},{1.0,.8892295438486978}}};
 if(resonance<=points.front().resonance)return 1;
 for(size_t i=1;i<points.size();++i)if(resonance<=points[i].resonance){
  const auto a=points[i-1],b=points[i];
  const double t=(resonance-a.resonance)/(b.resonance-a.resonance);
  return a.factor+t*(b.factor-a.factor);
 }
 return points.back().factor;
}

inline double revisedResonantFilter(double input,double cutoff,double resonance,
 double internalFs,std::array<double,4>& state,std::array<double,4>& previous,
 double& diodeNode,std::uint64_t& failures) noexcept {
 constexpr double pi=3.14159265358979323846;
 constexpr double outputNormalization=.6200804173378996;
 constexpr double lastHeadroom=2.2*outputNormalization;
 constexpr double poleGain=100.0/68.0;
 constexpr std::array<double,4> headroom{
  lastHeadroom*poleGain*poleGain*poleGain,
  lastHeadroom*poleGain*poleGain,lastHeadroom*poleGain,lastHeadroom};
 constexpr double L=2.8*outputNormalization;
 constexpr double alpha=2e-6,beta=12,normalization=1+alpha*beta;
 const double g=std::tan(pi*std::clamp(cutoff,5.0,std::min(22000.0,internalFs*.2))/internalFs)*resonantPitchCorrection(resonance);
 const double k=4.65*resonance;
 auto y=previous;double d=diodeNode;bool converged=false;
 for(int iteration=0;iteration<16;++iteration){
  const double argument=std::clamp(beta*d,-60.0,60.0);
  // Share one exponential between sinh and cosh. No lookup quantization or
  // feedback delay; the diode voltage is part of the same Newton system.
  const double positive=std::exp(argument),negative=1/positive;
  const double rd=d+.5*alpha*(positive-negative)-normalization*k*y[3]/L;
  const double cd=1+.5*alpha*beta*(positive+negative);
  const double feedback=L*d,feedbackDerivative=normalization*k/cd;
  std::array<double,4> a{},b{};double error=std::abs(L*rd);
  for(int j=0;j<4;++j){
   const auto index=size_t(j);
   const double in=j==0?input-feedback:y[index-1];
   const double scale=headroom[index],v=(in-y[index])/scale;
   const double residual=y[index]-state[index]-g*scale*otaSaturation(v);
   const double derivative=g*otaDerivative(v),denominator=1+derivative;
   a[index]=derivative/denominator;b[index]=-residual/denominator;
   error=std::max(error,std::abs(residual));
  }
  if(error<1e-13){converged=true;break;}
  const double A=a[3]*a[2]*a[1],B=a[3]*(a[2]*b[1]+b[2])+b[3];
  const double q=a[0]*feedbackDerivative;
  std::array<double,4> dy{};
  dy[0]=(b[0]-q*B+a[0]*L*rd/cd)/(1+q*A);
  for(size_t j=1;j<4;++j)dy[j]=a[j]*dy[j-1]+b[j];
  const double dd=(normalization*k*dy[3]/L-rd)/cd;
  const double damping=std::abs(dd)>.25?.25/std::abs(dd):1;
  for(size_t j=0;j<4;++j)y[j]+=damping*dy[j];
  d+=damping*dd;
 }
 if(!converged)++failures;
 for(size_t j=0;j<4;++j){
  if(!std::isfinite(y[j])||std::abs(y[j])>32||!std::isfinite(d)){
   state.fill(0);previous.fill(0);diodeNode=0;++failures;return 0;
  }
 }
 for(size_t j=0;j<4;++j){
  const double next=2*y[j]-state[j];state[j]=std::abs(next)<1e-24?0:next;
 }
 diodeNode=d;previous=y;return y[3];
}
}
