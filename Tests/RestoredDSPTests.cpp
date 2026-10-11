#include "../Source/DSP/Engine.h"
#include "ReferenceDSP113/Engine.h"
#include <cmath>
#include <iostream>

// Audio regression: the restored core must reproduce the preserved 1.1.3
// signal path exactly. Reference code is frozen and compiled independently.
int main() {
 int failures=0,checks=0;
 for(double sr:{44100.,48000.,96000.,192000.})
 for(float wave:{0.f,1.f,2.f,3.f}) for(float age:{0.f,.5f,1.f})
 for(float resonance:{0.f,.85f,1.f}) {
  sh09::Engine restored;sh09_reference::Engine reference;
  restored.prepare(sr);reference.prepare(sr);
  sh09::Controls c;sh09_reference::Controls r;
  c.wave=r.wave=wave;c.age=r.age=age;c.resonance=r.resonance=resonance;
  c.cutoff=r.cutoff=.6f;c.vcaMode=r.vcaMode=0;c.vcfEnv=r.vcfEnv=0;
  c.extLevel=r.extLevel=.3f;c.subLevel=r.subLevel=.25f;
  c.lfoRate=r.lfoRate=1;c.vcoMod=r.vcoMod=.2f;c.vcfMod=r.vcfMod=.1f;
  c.modern=r.modern=wave==3?1.f:0.f;
  restored.setControls(c);reference.setControls(r);
  restored.noteOn(60,100);reference.noteOn(60,100);
  bool same=true,finite=true;double energy=0;
  for(int i=0;i<int(sr*.2);++i) {
   if(i==1000){restored.pitchBend(.7f);reference.pitchBend(.7f);}
   if(i==2000){restored.noteOn(48,110);reference.noteOn(48,110);}
   if(i==3000){restored.noteOff(48);reference.noteOff(48);}
   const float input=float(.1*std::sin(i*.071));
   const float a=restored.process(input),b=reference.process(input);
   same&=a==b;finite&=std::isfinite(a)&&std::abs(a)<4;energy+=double(a)*a;
  }
  ++checks;if(!same||!finite||energy==0)++failures;
 }
 // Isolated feedback, including low and high cutoff and a continuous AGE sweep.
 for(double sr:{44100.,48000.,96000.}) for(float cutoff:{.25f,.6f,.95f}) {
  sh09::Engine a;sh09_reference::Engine b;a.prepare(sr);b.prepare(sr);
  sh09::Controls c;sh09_reference::Controls r;
  c.vcoLevel=r.vcoLevel=c.subLevel=r.subLevel=c.extLevel=r.extLevel=0;
  c.vcaMode=r.vcaMode=0;c.vcfEnv=r.vcfEnv=0;
  c.resonance=r.resonance=1;c.cutoff=r.cutoff=cutoff;
  a.setControls(c);b.setControls(r);bool same=true,finite=true;double energy=0;
  for(int i=0;i<int(sr*3);++i) {
   if(i%127==0){c.age=r.age=float(i)/float(sr*3);a.setControls(c);b.setControls(r);}
   const auto x=a.process(0),y=b.process(0);same&=x==y;finite&=std::isfinite(x)&&std::abs(x)<4;energy+=double(x)*x;
  }
  ++checks;if(!same||!finite||energy==0||a.solverFailures()!=b.solverFailures())++failures;
 }
 std::cout<<"RESULT restored/reference scenarios="<<checks<<" failures="<<failures<<" (sample-for-sample comparison)\n";
 return failures?1:0;
}
