#include "../Source/DSP/Engine.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <string>
#include <vector>

namespace {thread_local bool trackDspAllocations=false;thread_local unsigned dspAllocations=0;}
void* operator new(std::size_t n){if(trackDspAllocations)++dspAllocations;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}

namespace {
int failed=0,passed=0;
void check(bool condition,const std::string& name){std::cout<<(condition?"PASS ":"FAIL ")<<name<<'\n';condition?++passed:++failed;}
sh09::Controls clean(){sh09::Controls c;c.vcaMode=0;c.cutoff=1;c.vcfEnv=0;c.volume=.7f;c.pulseWidth=0;return c;}
std::vector<double> render(sh09::Engine& e,double seconds,double sr){std::vector<double> out(size_t(seconds*sr));for(auto& v:out)v=e.process(0);return out;}
double rms(const std::vector<double>& v){double energy=0;for(double x:v)energy+=x*x;return std::sqrt(energy/double(std::max(size_t(1),v.size())));}
double frequency(const std::vector<double>& v,double sr){std::vector<double> crossings;for(size_t i=1;i<v.size();++i)if(v[i-1]<0&&v[i]>=0)crossings.push_back(double(i-1)-v[i-1]/(v[i]-v[i-1]));if(crossings.size()<3)return 0;return double(crossings.size()-1)*sr/(crossings.back()-crossings.front());}
void fft(std::vector<std::complex<double>>& a){const size_t n=a.size();for(size_t i=1,j=0;i<n;++i){size_t bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(a[i],a[j]);}for(size_t len=2;len<=n;len<<=1){const auto wlen=std::polar(1.0,-2*3.14159265358979323846/double(len));for(size_t i=0;i<n;i+=len){std::complex<double>w=1;for(size_t j=0;j<len/2;++j){auto u=a[i+j],v=a[i+j+len/2]*w;a[i+j]=u+v;a[i+j+len/2]=u-v;w*=wlen;}}}}
double aliasDb(const std::vector<double>& signal,int bin){std::vector<std::complex<double>> spectrum(signal.size());for(size_t i=0;i<signal.size();++i)spectrum[i]=signal[i]*(.5-.5*std::cos(2*3.14159265358979323846*double(i)/double(signal.size())));fft(spectrum);double wanted=0,unwanted=0;for(int i=10;i<int(signal.size()/2);++i){bool harmonic=false;for(int b=bin;b<int(signal.size()/2);b+=bin)if(std::abs(i-b)<=8)harmonic=true;(harmonic?wanted:unwanted)+=std::norm(spectrum[size_t(i)]);}return 10*std::log10((unwanted+1e-30)/(wanted+1e-30));}
}
int main(){
 const auto start=std::chrono::steady_clock::now();
 for(double sr:{44100.,48000.,88200.,96000.,192000.}){
  for(int range:{0,2,4}){sh09::Engine e;e.prepare(sr);auto c=clean();c.range=float(range);e.setControls(c);e.noteOn(69,127);render(e,.12,sr);auto audio=render(e,.20,sr);const double measured=frequency(audio,sr),expected=440*std::exp2(range-2);check(std::abs(measured/expected-1)<.001,"pitch "+std::to_string(int(sr))+" range "+std::to_string(range)+" Hz="+std::to_string(measured));}
  for(int mode=0;mode<3;++mode){sh09::Engine e;e.prepare(sr);auto c=clean();c.vcoLevel=0;c.subLevel=1;c.subMode=float(mode);e.setControls(c);e.noteOn(69,127);render(e,.10,sr);auto audio=render(e,.20,sr);const double measured=frequency(audio,sr),expected=mode==0?220:110;check(std::abs(measured/expected-1)<.001,"divider "+std::to_string(int(sr))+" mode "+std::to_string(mode));}
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=clean();c.vcaMode=1;c.attack=.4f;e.setControls(c);e.noteOn(60,100);render(e,.01,48000);e.noteOn(72,100);check(e.currentNote()==60,"higher held note does not replace low note");e.noteOn(48,100);check(e.currentNote()==48,"lower held note replaces selected note");e.noteOff(48);check(e.currentNote()==60,"release restores lowest held note");e.noteOff(60);check(e.currentNote()==72,"release order restores remaining high note");e.noteOff(72);check(e.currentNote()==-1,"all keys released");e.noteOn(60,100);e.noteOn(60,100);e.noteOff(60);check(e.currentNote()==60,"duplicate note-on reference count");e.noteOff(60);check(e.currentNote()==-1,"duplicate note second release");e.noteOn(60,0);check(e.currentNote()==-1,"velocity zero is note-off");
  e.noteOn(60,127);render(e,.004,48000);double before=e.envelopeLevel();e.noteOn(72,127);check(std::abs(e.envelopeLevel()-before)<1e-7,"ignored higher note does not reset capacitor");e.allNotesOff();render(e,2,48000);check(e.envelopeLevel()==0,"envelope release reaches exact silence");
 }
 {
  for(float a:{0.f,.5f,1.f}){sh09::Engine e;e.prepare(48000);auto c=clean();c.vcaMode=1;c.attack=a;e.setControls(c);e.noteOn(60,100);int count=0;while(e.envelopeLevel()<.999f&&count<200000){e.process(0);++count;}double expected=.001*std::pow(2500.,a);check(std::abs(count/48000.-expected)<std::max(.0002,expected*.005),"ADSR attack endpoint "+std::to_string(a)+" seconds="+std::to_string(count/48000.));}
  sh09::Engine e;e.prepare(48000);auto c=clean();c.vcaMode=1;c.attack=0;c.decay=0;c.sustain=.4f;c.release=0;c.envMode=1;e.setControls(c);e.noteOn(60,100);render(e,.03,48000);double level=e.envelopeLevel();e.noteOn(48,100);e.process(0);check(std::abs(e.envelopeLevel()-level)<.002,"Gate legato retains sustain");e.allNotesOff();render(e,.04,48000);check(e.envelopeLevel()==0,"minimum release finite and silent");
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=clean();c.vcoLevel=c.subLevel=c.extLevel=0;c.vcfEnvMode=2;e.setControls(c);double output=0;for(int i=0;i<48000;++i)output+=std::abs(e.process(float(.4*std::sin(2*3.14159265358979323846*220*i/48000))));check(e.followerLevel()>.1f,"external follower detects before external mixer slider");check(output/48000<1e-6,"external mixer zero prevents audio but retains detector");render(e,.5,48000);check(e.followerLevel()<1e-6,"external follower releases on silence");
  c.extLevel=1;e.setControls(c);double energy=0;for(int i=0;i<24000;++i){double v=e.process(float(.4*std::sin(2*3.14159265358979323846*220*i/48000)));energy+=v*v;}check(energy/24000>.0001,"external audio traverses filter and hold VCA");e.allSoundOff();check(e.currentNote()==-1&&e.envelopeLevel()==0,"all sound off clears note and envelope");double afterMute=0;for(int i=0;i<4800;++i)afterMute+=std::abs(e.process(.5f));check(afterMute==0,"CC120 silences Hold until next note or VCA interaction");e.noteOn(60,100);energy=0;for(int i=0;i<4800;++i)energy+=std::abs(e.process(.5f));check(energy>0,"new note clears CC120 mute");
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=clean();c.wave=3;c.pulseWidth=1;c.subLevel=1;c.subMode=2;e.setControls(c);e.noteOn(69,127);render(e,.5,48000);auto v=render(e,1,48000);double mean=0;for(double x:v)mean+=x;check(std::abs(mean/v.size())<.0002,"asymmetric pulse and sub output DC coupling");
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=clean();c.vcoLevel=c.subLevel=0;c.resonance=1;c.cutoff=.60f;e.setControls(c);render(e,1.0,48000);auto audio=render(e,.5,48000);std::cout<<"MEASURE self oscillation RMS="<<rms(audio)<<" Hz="<<frequency(audio,48000)<<'\n';check(rms(audio)>.002,"filter self-oscillates without main/sub input");check(e.solverFailures()==0,"self-oscillation nonlinear solver converges");
 }
 {
  constexpr int n=8192,bin=1100;constexpr double sr=44100,hz=sr*bin/n;
  sh09::Engine e;e.prepare(sr);auto c=clean();c.tune=float(1200*std::log2(hz/(440*std::exp2((114.-69)/12))));e.setControls(c);e.noteOn(114,127);render(e,.15,sr);std::vector<double> signal(n),naive(n);for(int i=0;i<n;++i){signal[size_t(i)]=e.process(0);double phase=std::fmod(double(i)*hz/sr,1.0);naive[size_t(i)]=2*phase-1;}double actual=aliasDb(signal,bin),baseline=aliasDb(naive,bin);std::cout<<"MEASURE alias residual dB="<<actual<<" naive="<<baseline<<'\n';check(actual<-40&&actual<baseline-25,"coherent high saw alias suppression");
 }
 // Measure audio, rather than copying the modulation expression into a getter test.
 // At 2 Hz the square LFO provides stable positive and negative measurement windows.
 for(double sr:{44100.,192000.}){
  for(bool modern:{false,true}){
   sh09::Engine e;e.prepare(sr);auto c=clean();c.modern=modern?1.f:0.f;c.vcoMod=1;c.lfoWave=1;c.lfoRate=float(std::log(10.)/std::log(125.));
   e.setControls(c);e.noteOn(69,127);render(e,.08,sr);const auto upper=frequency(render(e,.12,sr),sr);
   render(e,.13,sr);const auto lower=frequency(render(e,.14,sr),sr);const double factor=modern?8.:2.;
   check(std::abs(upper/(440*factor)-1)<.003&&std::abs(lower/(440/factor)-1)<.003,
         std::string(modern?"Modern +/-36 semitone":"Authentic +/-12 semitone")+" VCO MOD endpoints "+std::to_string(int(sr))+
         " upperHz="+std::to_string(upper)+" lowerHz="+std::to_string(lower));
  }
  auto filterDepthRms=[&](bool modern,float depth){
   sh09::Engine e;e.prepare(sr);auto c=clean();c.modern=modern?1.f:0.f;c.vcoLevel=0;c.extLevel=1;c.vcfMod=depth;
   c.lfoWave=1;c.lfoRate=float(std::log(10.)/std::log(125.));e.setControls(c);e.noteOn(60,127);
   double energy=0;int samples=0;
   for(int i=0;i<int(.47*sr);++i){const auto value=e.process(float(.02*std::sin(2*3.14159265358979323846*220*i/sr)));
    if(i>int(.38*sr)){energy+=value*value;++samples;}}
   return std::sqrt(energy/samples);
  };
  const double authentic=filterDepthRms(false,1),modern=filterDepthRms(true,1),matched=filterDepthRms(true,.625f);
  std::cout<<"MEASURE VCF negative MOD 220Hz output RMS "<<sr<<" Authentic="<<authentic<<" Modern="<<modern<<" ModernDepth0.625="<<matched<<'\n';
  check(authentic>1e-5&&modern<authentic*.1,"Modern maximum VCF MOD extends below the old filter sweep "+std::to_string(int(sr)));
  check(std::abs(matched/authentic-1)<1e-6,"Modern VCF depth 0.625 preserves the old maximum sweep "+std::to_string(int(sr)));
 }
 for(double sr:{44100.,48000.,88200.,96000.,192000.}){
  sh09::Engine e;e.prepare(sr);auto c=clean();c.resonance=1;c.vcoLevel=c.subLevel=c.extLevel=1;c.vcfMod=c.vcoMod=c.vcfEnv=1;c.vcfEnvMode=2;c.age=1;c.modern=1;c.extGain=1;c.lfoRate=1;e.setControls(c);e.noteOn(127,127);bool finite=true;double peak=0;std::uint32_t random=17;
  trackDspAllocations=true;
  for(int i=0;i<int(sr*.55);++i){if(i%173==0){random=random*1664525u+1013904223u;c.cutoff=float(random&65535)/65535;c.pulseWidth=1-c.cutoff;c.wave=float((random>>16)&3);c.vcaMode=float((random>>20)%3);e.setControls(c);e.pitchBend(float((random&255)/127.5-1));e.noteOn(int(random%128),100);if(i%346==0)e.allNotesOff();}double v=e.process(i%127==0?std::numeric_limits<float>::infinity():float(8*std::sin(i*.57)));finite=finite&&std::isfinite(v);peak=std::max(peak,std::abs(v));}
  trackDspAllocations=false;
  check(finite&&peak<2,"bounded full-depth Modern stress "+std::to_string(int(sr))+" peak="+std::to_string(peak));std::cout<<"MEASURE solver failures="<<e.solverFailures()<<'\n';check(e.solverFailures()==0,"stress solver convergence "+std::to_string(int(sr)));
 }
 check(dspAllocations==0,"full-depth Modern processing and control changes perform no C++ new allocations");
 {
  for(double sr:{44100.,96000.}){sh09::Engine e;e.prepare(sr);auto c=clean();c.portamento=.1f;c.lfoRate=1;e.setControls(c);e.noteOn(48,100);e.noteOn(36,100);render(e,.01,sr);e.allNotesOff();double frozen=e.currentPitch();render(e,.1,sr);check(e.currentPitch()==frozen,"portamento freezes with no held key "+std::to_string(int(sr)));e.noteOn(60,100);render(e,.5,sr);check(std::abs(e.currentPitch()-60)<.03,"portamento reaches new note "+std::to_string(int(sr)));}
  sh09::Engine e;e.prepare(48000);auto c=clean();c.lfoRate=1;c.lfoDelay=1;e.setControls(c);e.noteOn(60,100);render(e,.2,48000);check(e.modulatorLevel()==0,"sine LFO initial delay");e.noteOn(72,100);render(e,1.405,48000);check(std::abs(e.modulatorLevel())>.001,"ignored higher note does not restart sine delay");c.lfoWave=1;e.setControls(c);render(e,.01,48000);check(std::abs(e.modulatorLevel())==1,"square LFO bypasses sine delay");
  e.prepare(48000);c.lfoWave=2;c.lfoRate=1;e.setControls(c);std::vector<double> held;for(int i=0;i<6000;++i){e.process(0);if(i>100&&i%100==0)held.push_back(e.modulatorLevel());}int jumps=0;for(size_t i=1;i<held.size();++i)if(held[i]!=held[i-1])++jumps;check(jumps==3,"random LFO holds between 25Hz clock events");
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=clean();c.vcoLevel=0;c.extLevel=1;e.setControls(c);std::vector<double> impulse(1024);for(size_t i=0;i<impulse.size();++i)impulse[i]=e.process(i==0?.001f:0);auto peak=std::max_element(impulse.begin(),impulse.end());int index=int(std::distance(impulse.begin(),peak));check(index>=e.latencySamples()&&index<=e.latencySamples()+3,"external impulse latency includes filter phase samples="+std::to_string(index));
  e.prepare(48000);c=clean();c.resonance=1;c.cutoff=.65f;e.setControls(c);e.noteOn(69,127);bool finite=true;double sum=0;const auto cpuStart=std::chrono::steady_clock::now();for(int i=0;i<48000*15;++i){auto value=e.process(0);finite=finite&&std::isfinite(value);sum+=value*value;}const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-cpuStart).count();check(finite&&sum>1&&e.solverFailures()==0,"15-second continuous resonant processing");std::cout<<"MEASURE 48k worst-resonance core CPU realtime ratio="<<seconds/15<<" wall_seconds="<<seconds<<'\n';
 }
 {
  double reference=0;
  for(double sr:{44100.,48000.,88200.,96000.,192000.}){sh09::Engine e;e.prepare(sr);auto c=clean();c.wave=0;c.cutoff=.8f;e.setControls(c);e.noteOn(60,100);render(e,.1,sr);auto audio=render(e,.8,sr);double level=rms(audio);if(reference==0)reference=level;std::cout<<"MEASURE noise RMS "<<sr<<"="<<level<<'\n';check(level>.001&&std::abs(level/reference-1)<.15,"noise power sample-rate invariance "+std::to_string(int(sr)));}
 }
 {
  sh09::Engine a,b;a.prepare(48000);b.prepare(48000);auto c=clean();c.age=.7f;a.setControls(c);b.setControls(c);a.noteOn(61,100);b.noteOn(61,100);bool same=true;for(int i=0;i<12000;++i)same=same&&a.process(0)==b.process(0);check(same,"deterministic condition and oscillator rendering");
  c.cutoff=std::numeric_limits<float>::quiet_NaN();c.range=std::numeric_limits<float>::infinity();a.setControls(c);check(std::isfinite(a.process(std::numeric_limits<float>::quiet_NaN())),"nonfinite controls and input rejected");
 }
 {
  double full=0,previous=-1;
  for(float volume:{0.f,.1f,.25f,.5f,.75f,1.f}) {
   sh09::Engine e;e.prepare(48000);auto c=clean();c.volume=volume;e.setControls(c);e.noteOn(60,127);
   render(e,.1,48000);const double level=rms(render(e,.2,48000));
   check(volume==0?level==0:level>previous,"volume audio taper increases across the complete knob travel "+std::to_string(volume));
   if(volume==1)full=level;previous=level;
  }
  sh09::Engine e;e.prepare(48000);auto c=clean();c.volume=.5f;e.setControls(c);e.noteOn(60,127);render(e,.1,48000);
  check(std::abs(rms(render(e,.2,48000))/full-.125)<.002,"volume midpoint has useful -18dB gain and top preserves unity");
  for(double bpm:{60.,120.,180.}) for(size_t index=0;index<sh09::rateDivisions.size();++index) {
   sh09::Engine clock;clock.prepare(48000);auto controls=clean();controls.modern=controls.lfoSync=1;
   controls.lfoRate=float(index)/float(sh09::rateDivisions.size()-1);clock.setControls(controls);clock.setTempo(bpm);
   double elapsedCycles=0,last=clock.modulatorPhase();
   for(int i=0;i<24000;++i){clock.process(0);const double phase=clock.modulatorPhase();elapsedCycles+=phase-last+(phase<last?1:0);last=phase;}
   const double expected=bpm/(60*sh09::rateDivisions[index].beats);
   check(std::abs(elapsedCycles/.5-expected)<1e-5,"SYNC audio clock division "+std::string(sh09::rateDivisions[index].label)+" BPM="+std::to_string(bpm));
  }
  sh09::Engine free;free.prepare(48000);c=clean();c.lfoRate=1;c.lfoSync=1;c.modern=0;free.setControls(c);free.setTempo(180);free.process(0);
  check(std::abs(free.modulatorFrequency()-25)<1e-9,"Authentic retains free 25Hz endpoint despite saved SYNC setting");
 }
 {
  // Measure phase, not audio crossings: AGE=0 must have no clock drift or jitter.
  for(double sr:{44100.,48000.,96000.}) for(float rate:{0.f,.5f,1.f}) for(bool modern:{false,true}) {
   sh09::Engine clock;clock.prepare(sr);auto c=clean();c.age=0;c.lfoRate=rate;c.modern=modern?1.f:0.f;clock.setControls(c);
   const double hz=sh09::lfoFrequency(rate,false,120,modern);
   double previous=clock.modulatorPhase(),cycles=0,maxStepError=0;
   for(int i=0;i<int(sr);++i) {
    clock.process(0);const double phase=clock.modulatorPhase();
    const double step=phase-previous+(phase<previous?1:0);
    cycles+=step;maxStepError=std::max(maxStepError,std::abs(step-hz/sr));previous=phase;
   }
   check(std::abs(cycles-hz)<1e-8&&maxStepError<1e-12,
         "fresh LFO has stable phase increments and no cumulative drift at "+std::to_string(int(sr))+" Hz rate="+std::to_string(hz));
  }
  check(std::abs(sh09::lfoFrequency(0,false,120,true)-.05)<1e-12,"Modern minimum is one cycle per 20 seconds");
  check(std::abs(sh09::lfoFrequency(0,false,120)-.2)<1e-12,"Authentic minimum retains published 0.2 Hz");
  bool preserved=true,monotonic=true;double last=0;
  for(int i=0;i<=10000;++i){const double v=i/10000.;const auto hz=sh09::lfoFrequency(v,false,120,true);monotonic&=hz>last;last=hz;if(v>=.2)preserved&=hz==sh09::lfoFrequency(v,false,120);}
  check(preserved&&monotonic,"Modern extension is monotonic and preserves upper 80 percent of RATE travel");
 }
 const auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();std::cout<<"RESULT "<<passed<<" passed "<<failed<<" failed wall_seconds="<<elapsed<<'\n';return failed?1:0;
}
