#pragma once
#include <array>
#include <cstdint>

namespace sh09 {
struct Controls {
 float tune=0,range=2,wave=1,pulseWidth=.5f,pwmMode=0;
 float vcoLevel=.8f,subLevel=0,extLevel=0,subMode=0;
 float lfoRate=.3f,lfoDelay=0,lfoWave=0,vcoMod=0;
 float cutoff=.7f,resonance=0,vcfMod=0,vcfEnv=.4f,vcfEnvMode=0;
 float vcaMode=1,attack=.01f,decay=.3f,sustain=.7f,release=.2f,envMode=0;
 float portamento=0,bendVco=.1667f,bendVcf=0,volume=.7f;
 float age=0,modern=0,velocity=0,aftertouch=0,lfoSync=0,extGain=.5f;
};

class Engine {
public:
 void prepare(double sampleRate) noexcept;
 void reset() noexcept;
 void setControls(Controls controls) noexcept;
 void noteOn(int note,int velocity) noexcept;
 void noteOff(int note) noexcept;
 void allNotesOff() noexcept;
 void allSoundOff() noexcept;
 void pitchBend(float bipolar) noexcept;
 void setAftertouch(float value) noexcept;
 void setTempo(double bpm) noexcept;
 float process(float externalMono) noexcept;
 int currentNote() const noexcept {return selected;}
 float envelopeLevel() const noexcept {return float(env);}
 float followerLevel() const noexcept {return float(follower[2]);}
 int latencySamples() const noexcept {return 128;}
 std::uint64_t solverFailures() const noexcept {return failures;}
 static constexpr int oversamplingFactor=4;
private:
 static constexpr int firLength=513, alignment=256;
 struct Frame {double source=0,env=0,gate=0,cutoff=1000,resonance=0,gain=0,externalLevel=0;};
 enum class Stage {idle,attack,decay,sustain,release};
 Controls target{},smooth{};
 bool initialized=false;
 double fs=44100,internalFs=176400,smoothing=0,controlBend=0,bendTarget=0,pressure=0,pressureTarget=0,tempo=120;
 std::array<unsigned char,128> keys{};
 std::array<float,128> velocities{};
 int selected=-1,lastNote=60;
 double pitch=60,selectedVelocity=1;
 bool pitchInitialized=false;
 double phase=0,lfoPhase=0,lfoDelayElapsed=0,randomLfo=0,env=0,gateGain=0,modeGain=1;
 unsigned int cycle=0;
 Stage stage=Stage::idle;
 double noiseLP=0,noiseDC=0,externalDC=0;
 std::array<double,3> follower{};
 std::array<double,4> filterState{},filterY{};
 std::uint32_t randomState=0x91e10da5u;
 std::uint64_t failures=0;
 std::array<double,firLength> fir{},outputHistory{};
 std::array<double,(firLength+3)/4> inputHistory{};
 std::array<Frame,alignment> frames{};
 unsigned int inputPos=0,outputPos=0,framePos=0;
 void selectNote(bool repeat=false) noexcept;
 void trigger() noexcept;
 double stepEnvelope() noexcept;
 double filter(double input,double cutoff,double resonance) noexcept;
 double random() noexcept;
 void smoothControls() noexcept;
};
}
