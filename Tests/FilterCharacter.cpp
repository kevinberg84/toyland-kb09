#include "../Source/DSP/Engine.h"
#include "ReferenceDSP113/Engine.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Offline measurement only: all allocation and file I/O is outside Engine.
namespace {
constexpr double pi=3.14159265358979323846;
void wav(const std::filesystem::path& path,const std::vector<float>& v,int sr) {
 std::ofstream f(path,std::ios::binary);
 auto u16=[&](unsigned x){char b[]{char(x),char(x>>8)};f.write(b,2);};
 auto u32=[&](unsigned x){u16(x&65535);u16(x>>16);};
 f.write("RIFF",4);u32(36+unsigned(v.size()*4));f.write("WAVEfmt ",8);
 u32(16);u16(3);u16(1);u32(sr);u32(sr*4);u16(4);u16(32);
 f.write("data",4);u32(unsigned(v.size()*4));f.write(reinterpret_cast<const char*>(v.data()),std::streamsize(v.size()*4));
 if(!f)throw std::runtime_error("Unable to save WAV");
}
template<class Engine,class Controls>
void measure(const std::string& model,const std::filesystem::path& out,bool full,bool limits) {
 std::ofstream csv(out/(model+".csv"));
 csv<<"file,sample_rate,cutoff,resonance,age,note,block_size,frequency_hz,rms,peak,dc,solver_failures,cpu_ratio\n";
 const std::vector<int> rates=full?std::vector<int>{44100,48000,96000,192000}:std::vector<int>{48000};
 unsigned serial=0;
 auto run=[&](int sr,float cutoff,float resonance,float age,int note,int block) {
  Engine e;e.prepare(sr);Controls c;c.vcaMode=0;c.vcoLevel=c.subLevel=c.extLevel=0;
  c.vcfEnv=c.vcfMod=0;c.cutoff=cutoff;c.resonance=resonance;c.age=age;c.volume=.7f;e.setControls(c);
  if(note!=60){e.noteOn(note,100);e.noteOff(note);} // Hold the selected key-follow pitch.
  // Resolve low-frequency startup and slow growth immediately above onset.
  const int settle=int(sr*std::max(3.0,320/(10*std::pow(2000.0,cutoff)))),count=sr;
  std::vector<float> audio(count);double energy=0,peak=0,dc=0,first=-1,last=-1;int crossings=0;
  const auto start=std::chrono::steady_clock::now();
  for(int offset=0;offset<settle+count;offset+=block)for(int i=offset;i<std::min(offset+block,settle+count);++i) {
   const float value=e.process(0);
   if(!std::isfinite(value))throw std::runtime_error("Nonfinite filter output");
   if(i<settle)continue;
   const auto n=size_t(i-settle);audio[n]=value;energy+=double(value)*value;peak=std::max(peak,std::abs(double(value)));dc+=value;
   if(n>0&&audio[n-1]<0&&value>=0){const double t=double(n-1)-audio[n-1]/double(value-audio[n-1]);if(first<0)first=t;last=t;++crossings;}
  }
  const double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
  const double hz=crossings>2?double(crossings-1)*sr/(last-first):0;
  const auto name=model+"_"+std::to_string(serial++)+".wav";wav(out/name,audio,sr);
  csv<<name<<','<<sr<<','<<cutoff<<','<<resonance<<','<<age<<','<<note<<','<<block<<','<<hz<<','<<std::sqrt(energy/count)<<','<<peak<<','<<dc/count<<','<<e.solverFailures()<<','<<wall/(double(settle+count)/sr)<<'\n';
  csv.flush();
 };
 if(!limits)for(int sr:rates)for(float cutoff:{.4f,.6f,.8f})for(float resonance:{.9f,1.f})for(float age:{0.f,.5f,1.f})run(sr,cutoff,resonance,age,60,256);
 if(limits){for(int sr:{44100,48000,96000,192000})for(float cutoff:{.95f,1.f})for(float age:{0.f,1.f})run(sr,cutoff,1,age,60,64);}
 if(full&&!limits){
  // Fine onset sweep plus below/above-onset cases, including low-frequency startup.
  for(float res:{.0f,.7f,.84f,.855f,.86f,.865f,.87f,.875f,.88f,.89f,.95f})for(float age:{0.f,1.f})run(48000,.6f,res,age,60,64);
  for(float cutoff:{.1f,.25f,.95f,1.f})for(float age:{0.f,1.f})run(48000,cutoff,1,age,60,512);
  for(int note:{48,72})run(48000,.6f,1,0,note,128);
  // Block partition invariance is exact because the engine is sample based.
  for(int block:{1,32,64,127,512,1024})run(48000,.6f,1,1,60,block);
 }
 std::cout<<model<<": "<<serial<<" stationary measurements saved\n";
}
}
int main(int argc,char** argv) {
 if(argc<3){std::cerr<<"Usage: SH09FilterCharacter baseline|current OUTPUT [full|limits]\n";return 2;}
 const std::filesystem::path out(argv[2]);std::filesystem::create_directories(out);
 try {
  if(std::string(argv[1])=="baseline")measure<sh09_reference::Engine,sh09_reference::Controls>("baseline",out,argc>3,argc>3&&std::string(argv[3])=="limits");
  else if(std::string(argv[1])=="current")measure<sh09::Engine,sh09::Controls>("current",out,argc>3,argc>3&&std::string(argv[3])=="limits");
  else throw std::runtime_error("Unknown model");
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
