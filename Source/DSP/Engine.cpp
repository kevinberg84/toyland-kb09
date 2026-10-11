#include "Engine.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace sh09 {
namespace {
constexpr double pi=3.14159265358979323846;
constexpr auto continuous=std::array{
 &Controls::tune,&Controls::pulseWidth,&Controls::vcoLevel,&Controls::subLevel,&Controls::extLevel,
 &Controls::lfoRate,&Controls::lfoDelay,&Controls::vcoMod,&Controls::cutoff,&Controls::resonance,
 &Controls::vcfMod,&Controls::vcfEnv,&Controls::attack,&Controls::decay,&Controls::sustain,&Controls::release,
 &Controls::portamento,&Controls::bendVco,&Controls::bendVcf,&Controls::volume,&Controls::age,
 &Controls::velocity,&Controls::aftertouch,&Controls::extGain};
constexpr auto discrete=std::array{
 &Controls::range,&Controls::wave,&Controls::pwmMode,&Controls::subMode,&Controls::lfoWave,
 &Controls::vcfEnvMode,&Controls::vcaMode,&Controls::envMode,&Controls::modern,&Controls::lfoSync};
double clamp(double v,double lo,double hi) noexcept {return std::clamp(v,lo,hi);}
int choice(float value,int max) noexcept {return std::clamp(int(std::lround(value)),0,max);}
double expRange(double value,double lo,double hi) noexcept {return lo*std::exp(std::log(hi/lo)*value);}
double blep(double t,double dt) noexcept {
 if(t<dt){t/=dt;return t+t-t*t-1;}
 if(t>1-dt){t=(t-1)/dt;return t*t+t+t+1;}
 return 0;
}
double pulse(double t,double width,double dt) noexcept {
 const double offset=t-width+(t<width?1:0);
 return (t<width?1.0:-1.0)+blep(t,dt)-blep(offset,dt);
}
// Smooth odd bounded approximation. Analytic derivative used by the solver.
double sat(double x) noexcept {
 if(x>=3)return 1;if(x<=-3)return -1;
 const double xx=x*x;return x*(27+xx)/(27+9*xx);
}
double satDerivative(double x) noexcept {
 if(std::abs(x)>=3)return 0;
 const double xx=x*x,d=27+9*xx;return 9*(xx-9)*(xx-9)/(d*d);
}
double tiny(double x) noexcept {return std::abs(x)<1e-24?0:x;}
}

void Engine::prepare(double sampleRate) noexcept {
 fs=std::isfinite(sampleRate)?clamp(sampleRate,8000,768000):44100;
 internalFs=fs*oversamplingFactor;smoothing=1-std::exp(-1/(.008*fs));
 double sum=0;
 for(int n=0;n<firLength;++n){
  const double x=double(n-(firLength-1)/2),fc=.46/oversamplingFactor;
  const double sinc=x==0?2*fc:std::sin(2*pi*fc*x)/(pi*x);
  const double w=.42-.5*std::cos(2*pi*n/(firLength-1))+.08*std::cos(4*pi*n/(firLength-1));
  fir[size_t(n)]=sinc*w;sum+=fir[size_t(n)];
 }
 for(auto& c:fir)c/=sum;
 initialized=false;reset();
}
void Engine::reset() noexcept {
 keys.fill(0);velocities.fill(0);selected=-1;lastNote=60;pitch=60;pitchInitialized=false;selectedVelocity=1;
 phase=lfoPhase=lfoDelayElapsed=randomLfo=lastLfo=env=gateGain=0;cycle=0;stage=Stage::idle;modeGain=1;pendingMode=false;
 noiseLP=noiseDC=externalDC=outputDC=diodeVoltage=0;muted=false;follower.fill(0);filterState.fill(0);filterY.fill(0);
 randomState=0x91e10da5u;failures=0;inputHistory.fill(0);outputHistory.fill(0);frames.fill({});
 inputPos=outputPos=framePos=0;controlBend=bendTarget=pressure=pressureTarget=0;
 if(initialized)smooth=target;
 updateCircuitCondition();
}
void Engine::setControls(Controls c) noexcept {
 for(auto p:continuous){if(!std::isfinite(c.*p))c.*p=Controls{}.*p;c.*p=float(clamp(c.*p,p==&Controls::tune?-65:0,p==&Controls::tune?65:1));}
 const std::array<int,10> maximum{4,3,2,2,2,2,2,2,1,1};
 for(size_t i=0;i<discrete.size();++i){auto p=discrete[i];if(!std::isfinite(c.*p))c.*p=Controls{}.*p;c.*p=float(choice(c.*p,maximum[i]));}
 if(initialized){for(auto p:discrete)if(c.*p!=target.*p)pendingMode=true;if(c.vcaMode!=target.vcaMode)muted=false;}
 target=c;
 if(!initialized){smooth=c;initialized=true;}
}
void Engine::smoothControls() noexcept {
 for(auto p:continuous)smooth.*p=float((smooth.*p)+smoothing*((target.*p)-(smooth.*p)));
 if(pendingMode){modeGain=std::max(0.0,modeGain-1/(.002*fs));if(modeGain==0){for(auto p:discrete)smooth.*p=target.*p;pendingMode=false;}}
 else modeGain=std::min(1.0,modeGain+1/(.002*fs));
 controlBend+=smoothing*(bendTarget-controlBend);pressure+=smoothing*(pressureTarget-pressure);
 updateCircuitCondition();
}
void Engine::updateCircuitCondition() noexcept {
 // Service p.5: 100k/1k first OTA input divider; 68k/1k thereafter.
 // 2VT=52 mV is a room-temperature differential-pair model, not a measured
 // BA662 fit. 1.5 circuit volts per engine unit is an explicit calibration
 // assumption. Pole small-signal slopes stay unity as offsets/headroom vary.
 const double a=clamp(smooth.age,0,1),wear=a*a*a*a;
 constexpr std::array<double,4> dividers{101.0,69.0,69.0,69.0};
 constexpr std::array<double,4> freshOffset{.003,-.002,.004,-.003};
 constexpr std::array<double,4> wornOffset{.035,-.028,.045,-.037};
 for(size_t j=0;j<4;++j){
  otaScale[j]=.052*dividers[j]/1.5*(1-.14*wear);
  otaBias[j]=freshOffset[j]+wornOffset[j]*wear;
  otaZero[j]=std::tanh(otaBias[j]);otaSlope[j]=1-otaZero[j]*otaZero[j];
 }
 // R118 || R109 || R119 at maximum resonance is approximately 1.30 kohm.
 // D12/D13 are opposing silicon diodes; Is and nVT are behavioral values,
 // not inferred from a fixed "0.6 V" threshold or an original-part fit.
 const double mismatch=.012+.12*wear,strength=1302.0*2.5e-9*(1+.4*wear);
 diodePositive=strength*(1+mismatch);diodeNegative=strength*(1-mismatch);
 diodeThermal=.045*(1+.025*wear);
 // Retain the existing lumped VCA/output transfer, with small offset and
 // progressively reduced headroom. AGE is a deterministic condition model;
 // chronological age alone does not establish these component changes.
 vcaScale=1.8*(1-.18*wear);vcaBias=.004+.04*wear;
 vcaZero=sat(vcaBias);vcaSlope=satDerivative(vcaBias);
}
void Engine::trigger() noexcept {stage=Stage::attack;lfoDelayElapsed=0;}
void Engine::selectNote(bool repeat) noexcept {
 int next=-1;for(int n=0;n<128;++n)if(keys[size_t(n)]!=0){next=n;break;}
 const bool changed=next!=selected;
 if(next>=0){
  if(!pitchInitialized){pitch=next;pitchInitialized=true;}
  selectedVelocity=velocities[size_t(next)];lastNote=next;
  if(selected<0||(choice(target.envMode,2)==0&&(changed||repeat)))trigger();
  // Delay retriggers on the effective selected pitch, independently of ENV mode.
  if(changed||repeat)lfoDelayElapsed=0;
 }else if(selected>=0)stage=Stage::release;
 selected=next;
}
void Engine::noteOn(int note,int velocity,bool alreadyHeld) noexcept {
 if(note<0||note>127)return;if(velocity<=0){noteOff(note);return;}
 muted=false;auto& count=keys[size_t(note)];if(!alreadyHeld&&count<255)++count;
 velocities[size_t(note)]=float(std::clamp(velocity,1,127))/127;
 selectNote(note==selected);
}
void Engine::noteOff(int note) noexcept {if(note<0||note>127)return;auto& count=keys[size_t(note)];if(count>0)--count;selectNote();}
void Engine::allNotesOff() noexcept {keys.fill(0);selectNote();}
void Engine::allSoundOff() noexcept {reset();muted=true;}
void Engine::pitchBend(float v) noexcept {bendTarget=std::isfinite(v)?clamp(v,-1,1):0;}
void Engine::setAftertouch(float v) noexcept {pressureTarget=std::isfinite(v)?clamp(v,0,1):0;}
void Engine::setTempo(double bpm) noexcept {if(std::isfinite(bpm))tempo=clamp(bpm,20,400);}
double Engine::random() noexcept {randomState^=randomState<<13;randomState^=randomState>>17;randomState^=randomState<<5;return double(randomState)/2147483648.0-1;}

double Engine::stepEnvelope() noexcept {
 const double age=smooth.age,timing=1+.085*age;
 const double a=expRange(smooth.attack,.001,2.5)*timing;
 const double d=expRange(smooth.decay,.002,10)*timing;
 const double r=expRange(smooth.release,.002,10)*timing;
 const double sustain=smooth.sustain;
 switch(stage){
  case Stage::idle:env=0;break;
  case Stage::attack:env+=(1.2-env)*(1-std::exp(-std::log(6.0)/(a*internalFs)));if(env>=1){env=1;stage=Stage::decay;}break;
  case Stage::decay:env+=(sustain-env)*(1-std::exp(-std::log(1000.0)/(d*internalFs)));if(std::abs(env-sustain)<1e-5)stage=Stage::sustain;break;
  case Stage::sustain:env=sustain;break;
  case Stage::release:env*=std::exp(-std::log(1000.0)/(r*internalFs));if(env<1e-7){env=0;stage=Stage::idle;}break;
 }
 return env;
}

double Engine::filter(double input,double cutoff,double resonance) noexcept {
 const double g=std::tan(pi*clamp(cutoff,5,std::min(22000.0,internalFs*.2))/internalFs);
 const double k=4.65*resonance;
 constexpr double feedbackGain=100.0/12.0,voltsPerUnit=1.5;
 auto y=filterY;
 double node=diodeVoltage;
 bool converged=false;
 for(int iteration=0;iteration<12;++iteration){
  // Solve the shunt diode KCL as the fifth unknown in the same Newton loop.
  // No nested limiter solver and no artificial unit delay in resonance.
  const double positive=std::exp(node/diodeThermal),negative=1/positive;
  const double diodeResidual=node+diodePositive*(positive-1)-diodeNegative*(negative-1)
      -k*y[3]*voltsPerUnit/feedbackGain;
  const double diodeDerivative=1+(diodePositive*positive+diodeNegative*negative)/diodeThermal;
  const double feedback=feedbackGain*node/voltsPerUnit;
  std::array<double,4> a{},b{};
  double error=std::abs(diodeResidual);
  for(int j=0;j<4;++j){
   const double in=j==0?input-feedback:y[size_t(j-1)];
   const auto index=size_t(j);
   const double transfer=std::tanh((in-y[index])/otaScale[index]+otaBias[index]);
   const double residual=y[index]-filterState[index]-g*otaScale[index]*(transfer-otaZero[index])/otaSlope[index];
   const double derivative=g*(1-transfer*transfer)/otaSlope[index],denominator=1+derivative;
   a[size_t(j)]=derivative/denominator;b[size_t(j)]=-residual/denominator;
   error=std::max(error,std::abs(residual));
  }
  if(error<1e-13){converged=true;break;}
  const double A=a[3]*a[2]*a[1],B=a[3]*(a[2]*b[1]+b[2])+b[3];
  const double q=a[0]*k/diodeDerivative;
  std::array<double,4> delta{};
  delta[0]=(b[0]+a[0]*feedbackGain/voltsPerUnit*diodeResidual/diodeDerivative-q*B)/(1+q*A);
  for(int j=1;j<4;++j)delta[size_t(j)]=a[size_t(j)]*delta[size_t(j-1)]+b[size_t(j)];
  const double nodeDelta=(k*delta[3]*voltsPerUnit/feedbackGain-diodeResidual)/diodeDerivative;
  // Damped Newton limits excursions through the exponential knee while
  // preserving the coupled update direction. Bounded work on the audio thread.
  const double damping=std::abs(nodeDelta)>.1?.1/std::abs(nodeDelta):1;
  node+=nodeDelta*damping;
  for(size_t j=0;j<4;++j)y[j]+=delta[j]*damping;
 }
 if(!converged)++failures;
 for(size_t j=0;j<4;++j){
  if(!std::isfinite(y[j])||std::abs(y[j])>32||!std::isfinite(node)||std::abs(node)>2){filterState.fill(0);filterY.fill(0);diodeVoltage=0;++failures;return 0;}
  filterState[j]=tiny(2*y[j]-filterState[j]);
 }
 filterY=y;diodeVoltage=node;return y[3];
}

float Engine::process(float externalMono) noexcept {
 if(!initialized)setControls({});smoothControls();
 const double input=std::isfinite(externalMono)?clamp(externalMono,-32,32):0;
 inputHistory[inputPos]=input;
 double result=0;
 const bool modern=choice(smooth.modern,1)!=0;
 // Optional extended depth. Authentic retains its previous engineering mapping;
 // neither maximum is claimed as a measured hardware modulation calibration.
 const double vcoModSemitones=modern?36.0:12.0,vcfModOctaves=modern?8.0:5.0;
 const double trim=modern?std::pow(10.0,(smooth.extGain-.5)*48/20):1;
 const double lfoHz=lfoFrequency(smooth.lfoRate,modern&&choice(smooth.lfoSync,1)!=0,tempo);
 const double pole=1-std::exp(-2*pi*8000/internalFs),dc=1-std::exp(-2*pi*12/internalFs);
 // Preserve analogue noise power as the internal sampling rate changes.
 const double referencePole=1-std::exp(-2*pi*8000/176400.0);
 const double noiseScale=std::sqrt((referencePole/(2-referencePole))/(pole/(2-pole)));
 const double gateSlew=1-std::exp(-1/(.00035*internalFs)),outputCoupling=1-std::exp(-2*pi*5/internalFs);
 const double followerA=1-std::exp(-1/(.010*internalFs)),followerB=1-std::exp(-1/(.0033*internalFs));
 for(int os=0;os<oversamplingFactor;++os){
  double external=0;
  for(int j=0;j<int(inputHistory.size());++j){const int tap=j*oversamplingFactor+os;if(tap>=firLength)break;const size_t index=(inputPos+inputHistory.size()-size_t(j))%inputHistory.size();external+=fir[size_t(tap)]*inputHistory[index]*oversamplingFactor;}
  externalDC+=dc*(external*trim-externalDC);external=external*trim-externalDC;
  // Positive rectifier before the mixer: three RC sections approximate the active ladder.
  const double rectified=std::max(0.0,external)*2.2;
  follower[0]+=followerA*(rectified-follower[0]);follower[1]+=followerA*(follower[0]-follower[1]);follower[2]+=followerB*(follower[1]-follower[2]);
  for(auto& v:follower)v=tiny(clamp(v,0,8));
  lfoPhase+=lfoHz/internalFs;
  if(lfoPhase>=1){lfoPhase-=1;randomLfo=random();if(selected>=0&&choice(smooth.envMode,2)==2)stage=Stage::attack;}
  const double sine=std::sin(2*pi*lfoPhase);lfoDelayElapsed+=1/internalFs;
  const double delay=smooth.lfoDelay*1.5;
  const double delayed=delay<=1e-6?1:clamp((lfoDelayElapsed-delay)/(std::max(.02,delay*.2)),0,1);
  double lfo=sine*delayed;if(choice(smooth.lfoWave,2)==1)lfo=lfoPhase<.5?1:-1;else if(choice(smooth.lfoWave,2)==2)lfo=randomLfo;lastLfo=lfo;
  if(selected>=0){
   const double glide=smooth.portamento*5;
   if(glide<1e-5)pitch=selected;else pitch+=(selected-pitch)*(1-std::exp(-std::log(1000.0)/(glide*internalFs)));
  }
  const double conditionCents=smooth.age*(2.0+.08*(pitch-60));
  const double semitone=pitch+12*(choice(smooth.range,4)-2)+smooth.tune/100.0+conditionCents/100.0+controlBend*smooth.bendVco*12+lfo*smooth.vcoMod*vcoModSemitones;
  const double hz=clamp(440*std::exp2((semitone-69)/12),.01,internalFs*.2),dt=hz/internalFs;
  const double rawNoise=random();noiseLP+=pole*(rawNoise*noiseScale-noiseLP);noiseDC+=dc*(noiseLP-noiseDC);const double noise=noiseLP-noiseDC;
  double width=.5+.45*smooth.pulseWidth;
  if(choice(smooth.pwmMode,2)==1)width=.5+.45*smooth.pulseWidth*sine;
  else if(choice(smooth.pwmMode,2)==2)width=.5+.45*smooth.pulseWidth*env;
  width=clamp(width,.05,.95);
  double osc=2*phase-1-blep(phase,dt);
  if(choice(smooth.wave,3)==0)osc=noise*1.6;else if(choice(smooth.wave,3)==2)osc=pulse(phase,.5,dt);else if(choice(smooth.wave,3)==3)osc=pulse(phase,width,dt);
  const int subMode=choice(smooth.subMode,2);
  const double subPhase=subMode==0?(double(cycle&1u)+phase)/2:(double(cycle&3u)+phase)/4;
  const double sub=pulse(subPhase,subMode==2?.25:.5,dt/(subMode==0?2:4));
  phase+=dt;if(phase>=1){phase-=1;cycle=(cycle+1)&3u;}
  stepEnvelope();gateGain+=gateSlew*((selected>=0?1:0)-gateGain);gateGain=tiny(gateGain);
  const double envMod=choice(smooth.vcfEnvMode,2)==1?-env:env;
  const double modernPressure=modern?pressure*smooth.aftertouch*3:0;
  const double tracking=(pitch-60)/12.0;
  const double cutoffOctaves=tracking+lfo*smooth.vcfMod*vcfModOctaves+(choice(smooth.vcfEnvMode,2)==2?0:envMod*smooth.vcfEnv*8)+controlBend*smooth.bendVcf*5+modernPressure;
  const double cutoff=clamp(expRange(smooth.cutoff,10,20000)*std::exp2(cutoffOctaves+.05*smooth.age),5,22000);
  const double velocityGain=modern?1-smooth.velocity*(1-selectedVelocity):1;
  double amplitude=env;if(choice(smooth.vcaMode,2)==0)amplitude=1;else if(choice(smooth.vcaMode,2)==2)amplitude=gateGain;
  Frame frame{.55*(osc*smooth.vcoLevel+sub*smooth.subLevel),amplitude,cutoff,clamp(smooth.resonance*(1+.015*smooth.age),0,1),smooth.volume*smooth.volume*smooth.volume*velocityGain*modeGain,smooth.extLevel,choice(smooth.vcfEnvMode,2)==2?smooth.vcfEnv*8.0:0};
  Frame delayedFrame=frames[framePos];frames[framePos]=frame;framePos=(framePos+1)%alignment;
  double finalCutoff=delayedFrame.cutoff;
  finalCutoff*=std::exp2(clamp(follower[2],0,1)*delayedFrame.followerAmount);
  const double mix=delayedFrame.source+external*delayedFrame.externalLevel*.8;
  const double filtered=filter(1.6*sat(mix/1.6)+rawNoise*1e-8,finalCutoff,delayedFrame.resonance);
  const double coupled=vcaScale*(sat(filtered*delayedFrame.amplitude/vcaScale+vcaBias)-vcaZero)/vcaSlope*delayedFrame.gain;
  outputDC+=outputCoupling*(coupled-outputDC);outputDC=tiny(outputDC);
  const double out=muted?0:coupled-outputDC;
  outputHistory[outputPos]=out;
  if(os==0){for(int j=0;j<firLength;++j)result+=fir[size_t(j)]*outputHistory[(outputPos+firLength-unsigned(j))%firLength];}
  outputPos=(outputPos+1)%firLength;
 }
 inputPos=(inputPos+1)%inputHistory.size();
 return std::isfinite(result)?float(tiny(result)):0;
}
}
