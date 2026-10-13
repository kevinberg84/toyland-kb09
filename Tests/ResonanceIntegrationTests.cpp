#include "../Source/DSP/Engine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {
int failures=0,checks=0;
void check(bool value,const char* message){++checks;if(!value)++failures;std::cout<<(value?"PASS ":"FAIL ")<<message<<'\n';}
double quantile(std::vector<double> x,double p){std::sort(x.begin(),x.end());const double v=p*(x.size()-1);const auto i=size_t(v);return x[i]+(x[std::min(i+1,x.size()-1)]-x[i])*(v-i);}
struct Levels{double spread=0,worstDip=0,below3=0;};
Levels levels(const std::vector<double>& y,int sr,int milliseconds){
 const auto n=size_t(sr*milliseconds/1000);std::vector<double> db;
 for(size_t i=size_t(sr);i+n<=y.size()-size_t(sr);i+=n){double sum=0;for(size_t j=0;j<n;++j)sum+=y[i+j]*y[i+j];db.push_back(10*std::log10(std::max(sum/n,1e-30)));}
 const double median=quantile(db,.5);size_t below=0;for(double v:db)below+=v<median-3;
 return {quantile(db,.9)-quantile(db,.1),median-*std::min_element(db.begin(),db.end()),double(below)/db.size()};
}
std::vector<double> sweep(sh09::FilterModel model,float level,float wave,int sr=48000){
 sh09::Engine e(model);e.prepare(sr);sh09::Controls c;c.modern=0;c.age=0;
 c.vcoLevel=level;c.subLevel=c.extLevel=c.vcfEnv=c.vcoMod=0;
 c.wave=wave;c.resonance=1;c.cutoff=.58f;c.vcfMod=.45f;c.lfoRate=0;
 c.vcaMode=1;c.attack=c.decay=0;c.sustain=.7f;c.volume=1;
 e.setControls(c);e.noteOn(53,127);
 for(int i=0;i<sr*6;++i)e.process(0);
 std::vector<double> y(size_t(sr*18));bool finite=true;
 for(auto& v:y){v=e.process(0);finite&=std::isfinite(v)&&std::abs(v)<2;}
 check(finite&&e.solverFailures()==0,"actual Authentic slow-MOD path remains bounded and converges");return y;
}
double staticRms(sh09::FilterModel model,float res,float wave,float level,int sr){
 sh09::Engine e(model);e.prepare(sr);sh09::Controls c;c.vcaMode=0;c.vcfEnv=0;
 c.wave=wave;c.vcoLevel=level;c.resonance=res;c.cutoff=.75f;c.volume=.7f;e.setControls(c);e.noteOn(60,127);
 for(int i=0;i<sr;++i)e.process(0);double sum=0;
 for(int i=0;i<sr/2;++i){const double v=e.process(0);sum+=v*v;}
 check(e.solverFailures()==0,"ordinary patch solver converges");return std::sqrt(sum/(sr/2));
}
double freeFrequency(sh09::FilterModel model,float resonance,float cutoff,int sr){
 sh09::Engine e(model);e.prepare(sr);sh09::Controls c;c.vcaMode=0;
 c.vcoLevel=c.subLevel=c.extLevel=c.vcfEnv=0;c.resonance=resonance;c.cutoff=cutoff;c.volume=1;e.setControls(c);
 for(int i=0;i<sr*3;++i)e.process(0);double previous=e.process(0),first=0,last=0;int count=0;
 for(int i=1;i<sr/2;++i){const double v=e.process(0);if(previous<0&&v>=0){const double t=i-1-previous/(v-previous);if(count==0)first=t;last=t;++count;}previous=v;}
 check(count>3&&e.solverFailures()==0,"free filter sustains a converged oscillator during tuning checks");
 return count>3?(count-1)*sr/(last-first):0;
}
}
int main(){
 using sh09::FilterModel;
 for(float wave:{1.f,2.f})for(float level:{.5f,1.f}){
  auto old=sweep(FilterModel::legacy,level,wave),revised=sweep(FilterModel::revised,level,wave);
  const auto a=levels(old,48000,50),b=levels(revised,48000,50);
  const auto ta=levels(old,48000,250),tb=levels(revised,48000,250);
  std::cout<<"MEASURE wave="<<wave<<" VCO="<<level<<" 50msSpread="<<a.spread<<","<<b.spread
   <<" 250msWorstDip="<<ta.worstDip<<","<<tb.worstDip<<" 50msFractionBelow3dB="<<a.below3<<","<<b.below3<<'\n';
  check(b.spread<a.spread*.7,"revised filter materially reduces actual slow-sweep level spread");
  check(tb.worstDip<ta.worstDip,"revised filter reduces the worst sustained sweep dip");
 }
 for(int sr:{44100,48000,96000})for(float wave:{1.f,2.f})for(float level:{.2f,1.f}){
  const auto a=staticRms(FilterModel::legacy,0,wave,level,sr),b=staticRms(FilterModel::revised,0,wave,level,sr);
  const double change=20*std::log10(b/a);std::cout<<"MEASURE ordinary sr="<<sr<<" wave="<<wave<<" level="<<level<<" changeDb="<<change<<'\n';
  check(std::abs(change)<.25,"nonresonant ordinary patch level stays within 0.25 dB");
 }
 for(int sr:{44100,48000,96000})for(float cutoff:{.4f,.6f,.9f})for(float resonance:{.88f,.91f,.95f,1.f}){
  const double a=freeFrequency(FilterModel::legacy,resonance,cutoff,sr),b=freeFrequency(FilterModel::revised,resonance,cutoff,sr);
  const double cents=1200*std::log2(b/a);
  std::cout<<"MEASURE tuning sr="<<sr<<" cutoff="<<cutoff<<" resonance="<<resonance<<" changeCents="<<cents<<'\n';
  check(std::isfinite(cents)&&std::abs(cents)<2,"resonant pitch calibration retains existing tuning within two cents");
 }
 {
  sh09::Engine e;e.prepare(48000);sh09::Controls c;c.vcaMode=0;c.resonance=1;c.vcoLevel=0;c.vcfEnv=0;e.setControls(c);
  for(int i=0;i<96000;++i)e.process(0);e.allSoundOff();double sum=0;
  for(int i=0;i<2048;++i)sum+=std::abs(e.process(0));check(sum==0,"reset/all-sound-off clears and mutes the new diode state");
 }
 std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<'\n';return failures?1:0;
}
