#include "../Source/DSP/Engine.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

namespace {
constexpr double pi=3.14159265358979323846;
int failures=0,checks=0;
void check(bool ok,const char* name){++checks;failures+=!ok;std::cout<<(ok?"PASS ":"FAIL ")<<name<<'\n';}
sh09::Controls self(float age=0,float resonance=1){sh09::Controls c;c.vcoLevel=c.subLevel=c.extLevel=0;c.vcaMode=0;c.vcfEnv=0;c.cutoff=.6f;c.resonance=resonance;c.age=age;c.volume=.7f;return c;}
std::vector<double> capture(sh09::Engine& e,int sr,double settle=2){for(int i=0;i<int(sr*settle);++i)e.process(0);std::vector<double> x(size_t(sr/2));for(auto& v:x)v=e.process(0);return x;}
double rms(const std::vector<double>& x){double sum=0;for(double v:x)sum+=v*v;return std::sqrt(sum/x.size());}
double hz(const std::vector<double>& x,int sr){double first=-1,last=-1;int count=0;for(size_t i=1;i<x.size();++i)if(x[i-1]<0&&x[i]>=0){double t=double(i-1)-x[i-1]/(x[i]-x[i-1]);if(first<0)first=t;last=t;++count;}return count>2?double(count-1)*sr/(last-first):0;}
double harmonic(const std::vector<double>& x,int sr,double freq,int index){std::complex<double> sum{};double weights=0;for(size_t i=0;i<x.size();++i){double w=.5-.5*std::cos(2*pi*i/(x.size()-1));sum+=x[i]*w*std::polar(1.0,-2*pi*freq*index*double(i)/sr);weights+=w;}return 2*std::abs(sum)/weights;}
}
int main(){
 double reference=0;
 for(int sr:{44100,48000,96000,192000}){
  sh09::Engine e;e.prepare(sr);auto c=self();e.setControls(c);auto fresh=capture(e,sr);double f=hz(fresh,sr);
  bool finite=true;double peak=0,dc=0;for(double v:fresh){finite&=std::isfinite(v);peak=std::max(peak,std::abs(v));dc+=v;}
  check(finite&&rms(fresh)>.1&&peak<.4,"healthy filter self-oscillates strongly with all mixer sources muted");
  check(std::abs(dc/fresh.size())<.001,"self-oscillation is AC coupled");
  if(reference==0)reference=f;check(std::abs(f/reference-1)<.001,"self-oscillation frequency is sample-rate invariant");
  const double h1=harmonic(fresh,sr,f,1),h2=harmonic(fresh,sr,f,2),h3=harmonic(fresh,sr,f,3);
  check(h3/h1>.005&&h3/h1<.04,"healthy filter has restrained natural odd harmonics");
  e.noteOn(72,100);e.noteOff(72);auto octave=capture(e,sr,.5);check(std::abs(1200*std::log2(hz(octave,sr)/f)-1200)<1,"filter key tracking retains an octave within one cent");
  e.prepare(sr);c=self(1);e.setControls(c);auto aged=capture(e,sr);double fa=hz(aged,sr);
  check(harmonic(aged,sr,fa,2)/harmonic(aged,sr,fa,1)>3*h2/h1,"maximum condition introduces modest even-harmonic asymmetry");
  e.prepare(sr);c=self(0,.84f);e.setControls(c);check(rms(capture(e,sr,.5))<1e-5,"subcritical feedback does not fabricate an oscillator");
  e.prepare(sr);c=self(0,.9f);e.setControls(c);check(rms(capture(e,sr))>.03,"feedback sustains oscillation above onset");
  check(e.solverFailures()==0,"nonlinear feedback solver converges at tested sample rates");
  for(float cutoff:{.95f,1.f}){e.prepare(sr);c=self(1);c.cutoff=cutoff;e.setControls(c);auto top=capture(e,sr);double ftop=hz(top,sr);check(rms(top)>.1&&ftop>12000&&ftop<21000&&e.solverFailures()==0,"near-maximum cutoff sustains bounded self-oscillation at every rate");}
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=self();c.cutoff=.25f;e.setControls(c);auto low=capture(e,48000,8);
  check(rms(low)>.08&&hz(low,48000)>50&&hz(low,48000)<80,"low-frequency feedback starts and sustains without an injected tone");
  check(e.solverFailures()==0,"low-frequency solver converges");
 }
 {
  sh09::Engine e;e.prepare(48000);auto c=self();e.setControls(c);capture(e,48000);
  double previous=e.process(0),maxStep=0,peak=0;
  for(int i=0;i<48000*4;++i){c.age=float(i<96000?i:192000-i)/96000;e.setControls(c);double v=e.process(0);maxStep=std::max(maxStep,std::abs(v-previous));peak=std::max(peak,std::abs(v));previous=v;}
  check(maxStep<.08&&peak<.4&&e.solverFailures()==0,"continuous AGE sweeps preserve a bounded, click-free limit cycle");
  e.allSoundOff();for(int i=0;i<1024;++i)e.process(0);double sum=0;for(int i=0;i<1024;++i)sum+=std::abs(e.process(0));check(sum==0,"all-sound-off silences the self-oscillating filter");
 }
 std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<'\n';return failures?1:0;
}
