#include "../Source/DSP/Engine.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void u16(std::ofstream& o,std::uint16_t v){char b[]{char(v&255),char((v>>8)&255)};o.write(b,2);}
void u32(std::ofstream& o,std::uint32_t v){u16(o,std::uint16_t(v));u16(o,std::uint16_t(v>>16));}
bool wav(const std::filesystem::path& path,const std::vector<float>& audio,int rate){std::ofstream o(path,std::ios::binary);o.write("RIFF",4);u32(o,36+std::uint32_t(audio.size()*2));o.write("WAVEfmt ",8);u32(o,16);u16(o,1);u16(o,1);u32(o,std::uint32_t(rate));u32(o,std::uint32_t(rate*2));u16(o,2);u16(o,16);o.write("data",4);u32(o,std::uint32_t(audio.size()*2));for(float v:audio)u16(o,std::uint16_t(std::int16_t(std::clamp(v,-1.f,1.f)*32767)));return bool(o);}
int main(int argc,char** argv){const std::filesystem::path out=argc>1?argv[1]:"artifacts/Audio";std::filesystem::create_directories(out);constexpr int rate=48000;const auto start=std::chrono::steady_clock::now();double duration=0;
 for(int patch=0;patch<7;++patch){sh09::Engine e;e.prepare(rate);sh09::Controls c;c.cutoff=.8f;c.attack=.05f;c.decay=.45f;c.release=.3f;c.volume=.7f;std::string name;
  if(patch==0){name="01_bass";c.range=1;c.subLevel=.4f;c.cutoff=.42f;c.resonance=.3f;}
  if(patch==1){name="02_resonant_lead";c.wave=3;c.resonance=.75f;c.cutoff=.56f;c.pwmMode=1;c.pulseWidth=.65f;c.lfoRate=.35f;}
  if(patch==2){name="03_random_modulation";c.lfoWave=2;c.lfoRate=.65f;c.vcfMod=.6f;c.cutoff=.5f;c.vcaMode=0;}
  if(patch==3){name="04_self_oscillation";c.vcoLevel=0;c.resonance=1;c.cutoff=.62f;c.vcfEnv=0;c.vcaMode=0;}
  if(patch==4){name="05_external_follower";c.vcoLevel=0;c.extLevel=1;c.vcfEnvMode=2;c.cutoff=.32f;c.vcfEnv=.8f;c.resonance=.5f;c.vcaMode=0;}
  if(patch==5){name="06_condition";c.age=1;c.wave=3;c.pwmMode=1;c.subLevel=.25f;c.cutoff=.6f;}
  if(patch==6){name="07_live_external_stimulus";}
  e.setControls(c);std::vector<float> samples(rate*5);
  for(int i=0;i<int(samples.size());++i){const double t=double(i)/rate;float external=float(.25*std::sin(2*3.14159265358979323846*110*t)*(t-std::floor(t)<.45?1:0));if(i%rate==0)e.noteOn(48+(i/rate)%4*3,100);if(i%rate==rate*3/4)e.allNotesOff();samples[size_t(i)]=patch==6?external:e.process(external);}
  if(!wav(out/(name+".wav"),samples,rate))return 1;duration+=5;std::cout<<name<<" solver_failures="<<e.solverFailures()<<'\n';
 }
 const double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();std::cout<<"Rendered "<<duration<<" seconds in "<<wall<<" seconds. Includes filesystem and stimulus time; ratio="<<wall/duration<<'\n';
}
