#include "../Source/Plugin/Processor.h"
#include "../Source/GUI/BenderReturnMotion.h"
#include <cstdlib>
#include <new>
#include <iostream>
#include <cmath>
#include <vector>
namespace {thread_local bool track=false;thread_local unsigned allocations=0;}
void* operator new(std::size_t n){if(track)++allocations;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}
int main(){
 juce::ScopedJuceInitialiser_GUI init;SH09Processor p;p.prepareToPlay(48000,127);
 int failed=0;auto expect=[&](bool ok,const char* text){std::cout<<(ok?"PASS ":"FAIL ")<<text<<'\n';if(!ok)++failed;};
 juce::AudioBuffer<float> audio(2,127);juce::MidiBuffer midi;midi.ensureSize(8192);
 auto process=[&]{audio.clear();track=true;p.processBlock(audio,midi);track=false;};
 {
  std::vector<float> before;for(auto* parameter:p.getParameters())before.push_back(parameter->getValue());
  for(int channel:{1,2,16})for(int cc:{7,11,16,17,18,19,20,21,22,23,71,74})
      midi.addEvent(juce::MidiMessage::controllerEvent(channel,cc,(cc*7)%128),0);
  process();bool untouched=true;
  for(size_t i=0;i<before.size();++i)untouched&=p.getParameters()[int(i)]->getValue()==before[i];
  expect(untouched,"ordinary controller fader CCs do not remap or alter KB-09 parameters behind host mappings");
 }
 for(int i=0;i<300;++i)p.queueKeyboard(60,true);
 p.queueKeyboard(60,false);process();expect(p.displayedNote.load()==-1&&p.keyDisplay[60].load()==0,"queue overflow discards stale note-ons and prevents stuck keys");
 p.queueKeyboard(60,true);process();expect(p.displayedNote.load()==60,"keyboard recovers after overflow");
 midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),17);process();
 expect(p.keyDisplay[60].load()==2,"GUI and MIDI duplicate note display count");
 p.queueKeyboard(60,false);process();expect(p.keyDisplay[60].load()==1&&p.displayedNote.load()==60,"GUI note release retains held MIDI note");
 midi.addEvent(juce::MidiMessage::noteOff(1,60),126);process();expect(p.keyDisplay[60].load()==0&&p.displayedNote.load()==-1,"last duplicate release clears display and voice");
 // Regression: a retransmitted note-on interleaved with wheel traffic must
 // not acquire a second hold on the same MIDI channel.
 midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);
 for(int i=1;i<90;++i)midi.addEvent(juce::MidiMessage::pitchWheel(1,(i*173)%16384),i);
 midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(90)),40);
 midi.addEvent(juce::MidiMessage::noteOff(1,60),126);process();
 expect(p.displayedNote.load()==-1&&p.keyDisplay[60].load()==0,"repeated MIDI note-on during pitch bend releases with one note-off");
 p.reset();
 midi.addEvent(juce::MidiMessage::noteOn(1,64,juce::uint8(100)),0);
 midi.addEvent(juce::MidiMessage::noteOn(2,64,juce::uint8(100)),1);process();
 midi.addEvent(juce::MidiMessage::noteOff(3,64),0);process();
 expect(p.keyDisplay[64].load()==2,"unmatched channel note-off cannot release another channel");
 midi.addEvent(juce::MidiMessage::noteOff(1,64),0);process();
 expect(p.keyDisplay[64].load()==1&&p.displayedNote.load()==64,"same-pitch notes on separate channels retain independent ownership");
 midi.addEvent(juce::MidiMessage::noteOn(2,64,juce::uint8(0)),126);process();
 expect(p.displayedNote.load()==-1,"zero-velocity note-on releases the remaining channel");
 p.reset();
 for(int block=0;block<1000;++block){
   midi.addEvent(juce::MidiMessage::noteOn(1,48+block%24,juce::uint8(100)),0);
   for(int i=1;i<126;++i)midi.addEvent(juce::MidiMessage::pitchWheel(1,(block*337+i*179)%16384),i);
   midi.addEvent(juce::MidiMessage::noteOff(1,48+block%24),127);process();
 }
 expect(p.displayedNote.load()==-1,"125000 wheel events preserve end-of-block note-offs");
 for(int b=0;b<500;++b){for(int i=0;i<20;++i){midi.addEvent(juce::MidiMessage::noteOn(1,40+i,juce::uint8(100)),i*3);midi.addEvent(juce::MidiMessage::noteOff(1,40+i),i*3+1);}p.queueKeyboard(62,true);p.queueKeyboard(62,false);process();}
 expect(allocations==0,"normal render path performs no C++ new allocations");
 juce::AudioBuffer<float> zero(2,0);midi.addEvent(juce::MidiMessage::allSoundOff(1),0);p.processBlock(zero,midi);expect(p.displayedNote.load()==-1,"zero-sample MIDI panic flush");
 // Power must clear held voices while continuing to consume MIDI; suspending the
 // JUCE wrapper would lose note-offs and resume stuck voices when switched on.
 p.setCurrentProgram(0);p.reset();
 midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);p.queueKeyboard(64,true);
 float peak=0;
 for(int i=0;i<32;++i){process();peak=juce::jmax(peak,audio.getMagnitude(0,audio.getNumSamples()));}
 expect(p.isPowerEnabled()&&p.displayedNote.load()==60&&peak>1.0e-4f,"power test starts with an audible held MIDI note and queued GUI key");
 p.setPowerEnabled(false);
 midi.addEvent(juce::MidiMessage::noteOff(1,60),0);midi.addEvent(juce::MidiMessage::noteOn(1,67,juce::uint8(100)),23);
 p.queueKeyboard(70,true);p.queuePitchBend(.75f);
 for(int ch=0;ch<audio.getNumChannels();++ch)for(int s=0;s<audio.getNumSamples();++s)audio.setSample(ch,s,.5f);
 track=true;p.processBlock(audio,midi);track=false;
 bool keysClear=true;for(const auto& key:p.keyDisplay)keysClear=keysClear&&key.load()==0;
 expect(!p.isPowerEnabled()&&audio.getMagnitude(0,audio.getNumSamples())==0&&midi.isEmpty()&&keysClear
        &&p.displayedNote.load()==-1&&p.displayedEnvelope.load()==0&&p.displayedFollower.load()==0&&!p.displayedModulatorLamp.load(),
        "power off outputs exact silence, consumes MIDI and clears all voice displays");
 p.setPowerEnabled(true);process();
 expect(p.displayedNote.load()==-1&&audio.getMagnitude(0,audio.getNumSamples())==0,"power on does not revive notes released or received while off");
 midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);peak=0;
 for(int i=0;i<32;++i){process();peak=juce::jmax(peak,audio.getMagnitude(0,audio.getNumSamples()));}
 expect(p.displayedNote.load()==60&&peak>1.0e-4f,"fresh MIDI note sounds after power is restored");
 p.queueKeyboard(55,true);p.setPowerEnabled(false);p.setPowerEnabled(true);process();
 expect(p.displayedNote.load()==-1&&p.keyDisplay[55].load()==0&&audio.getMagnitude(0,audio.getNumSamples())==0,
        "off/on between callbacks clears the old voice and discards old queued GUI notes");
 p.setPowerEnabled(false);p.setPowerEnabled(true);p.queueKeyboard(62,true);process();
 expect(p.displayedNote.load()==62&&p.keyDisplay[62].load()==1,"fresh GUI note after a rapid power cycle is preserved");
 p.queueKeyboard(50,true);p.setPowerEnabled(false);midi.addEvent(juce::MidiMessage::noteOn(1,40,juce::uint8(100)),0);
 track=true;p.processBlock(zero,midi);track=false;
 const bool zeroDrained=midi.isEmpty()&&p.displayedNote.load()==-1&&p.keyDisplay[62].load()==0;
 p.setPowerEnabled(true);process();
 expect(zeroDrained&&p.displayedNote.load()==-1&&p.keyDisplay[50].load()==0,"zero-sample power-off callback drains MIDI and pre-off GUI queue");
 p.setPowerEnabled(false);juce::MemoryBlock saved;p.getStateInformation(saved);
 {SH09Processor restored;restored.setStateInformation(saved.getData(),int(saved.getSize()));
  expect(restored.isPowerEnabled()&&!p.isPowerEnabled(),"Power is transient and is not restored from a saved patch");}
 p.setCurrentProgram(11);p.setPowerEnabled(true);peak=0;
 for(int block=0;block<96;++block){
   for(int s=0;s<audio.getNumSamples();++s){const float signal=.25f*std::sin(float(2*juce::MathConstants<double>::pi*110*(block*127+s)/48000));
     for(int ch=0;ch<audio.getNumChannels();++ch)audio.setSample(ch,s,signal);}
   track=true;p.processBlock(audio,midi);track=false;peak=juce::jmax(peak,audio.getMagnitude(0,audio.getNumSamples()));
 }
 expect(p.displayedNote.load()==-1&&peak>1.0e-5f,"power on restores external Hold audio without requiring a MIDI note");
 expect(allocations==0,"power transitions and off callbacks perform no C++ new allocations");
 // A user preset retains a factory program reference. Reapplying that same index
 // must notify the editor that its custom filename no longer describes the sound.
 {
  SH09Processor presets;presets.setCurrentProgram(9);
  auto* cutoff=presets.state.getParameter("cutoff");const float factory=cutoff->getValue();
  const auto loadedRevision=presets.getProgramRevision();
  cutoff->setValueNotifyingHost(factory<.5f?.83f:.17f);
  const float custom=cutoff->getValue();juce::MemoryBlock customState;presets.getStateInformation(customState);
  expect(custom!=factory&&presets.getProgramRevision()==loadedRevision,
         "ordinary sound edits preserve the current preset-label revision");
  presets.setCurrentProgram(9);
  expect(presets.getCurrentProgram()==9&&cutoff->getValue()==factory&&presets.getProgramRevision()!=loadedRevision,
         "reapplying the same factory index restores its sound and advances the preset-label revision");
  const auto factoryRevision=presets.getProgramRevision();
  presets.setStateInformation(customState.getData(),int(customState.getSize()));
  expect(presets.getCurrentProgram()==9&&std::abs(cutoff->getValue()-custom)<1.0e-6f
         &&presets.getProgramRevision()!=factoryRevision,
         "custom state restore at the same index advances revision after restoring its sound");
 }
 p.setCurrentProgram(0);p.setPowerEnabled(false);p.setPowerEnabled(true);process();
 p.queueKeyboard(60,true,false);p.queueKeyboard(64,true,false);process();
 expect(p.keyDisplay[60].load()==1&&p.keyDisplay[64].load()==1&&p.displayedKeyboardNote.load()==-1,
        "computer keyboard chord tracks both notes without impersonating mouse key");
 p.queueKeyboard(60,false,false);process();
 expect(p.keyDisplay[60].load()==0&&p.keyDisplay[64].load()==1,
        "computer keyboard release preserves another held note");
 p.queueKeyboard(64,false,false);process();
 expect(p.keyDisplay[64].load()==0,"computer keyboard release clears its last held note");
 // Incoming wheel feedback is transient MIDI state, never host parameter automation.
 p.reset();
 bool wheelPositions=true;
 for(int channel=1;channel<=16;++channel)for(int wheel:{0,4096,8192,12288,16383}) {
   midi.addEvent(juce::MidiMessage::pitchWheel(channel,wheel),0);process();
   wheelPositions=wheelPositions&&std::abs(p.displayedPitchBend.load()-(float(wheel)/8192-1))<1e-7f;
 }
 expect(wheelPositions,"all MIDI channels publish the full 14-bit pitch wheel position");
 const auto serial=p.displayedPitchBendSerial.load();process();
 expect(p.displayedPitchBendSerial.load()==serial,"idle processing does not echo displayed MIDI back into the engine");
 midi.addEvent(juce::MidiMessage::controllerEvent(1,121,0),0);process();
 expect(p.displayedPitchBend.load()==0,"Reset All Controllers centers bend feedback");
 midi.addEvent(juce::MidiMessage::pitchWheel(2,0),0);
 midi.addEvent(juce::MidiMessage::pitchWheel(7,12288),127);process();
 expect(p.displayedPitchBend.load()==.5f,"end-of-block wheel events publish the final incoming position");
 midi.addEvent(juce::MidiMessage::pitchWheel(4,4096),0);p.processBlock(zero,midi);
 expect(p.displayedPitchBend.load()==-.5f,"zero-sample callbacks publish incoming wheel feedback");
 midi.addEvent(juce::MidiMessage::allSoundOff(1),0);process();
 expect(p.displayedPitchBend.load()==0,"All Sound Off centers the reset engine and displayed bender together");
 // The spring is timed in samples, not dependent on editor refresh or buffer size.
 bool springTiming=true,modernReturn=true,interruptions=true;
 for(double rate:{44100.,48000.,96000.})for(int blockSize:{1,127,512}) {
   SH09Processor spring;spring.prepareToPlay(rate,blockSize);
   juce::AudioBuffer<float> block(2,blockSize);juce::MidiBuffer events;events.ensureSize(256);
   auto render=[&]{block.clear();track=true;spring.processBlock(block,events);track=false;};
   auto mode=[&](float value){auto* parameter=spring.state.getParameter("modern");parameter->setValueNotifyingHost(parameter->convertTo0to1(value));};
   mode(0);spring.queuePitchBend(1);render();spring.queuePitchBendReturn();
   int elapsed=0;float previous=1;const int duration=int(std::lround(rate*SH09Processor::benderReturnSeconds));
   while(elapsed<duration+blockSize) {
     render();elapsed+=blockSize;const float bend=spring.displayedPitchBend.load();
     springTiming=springTiming&&bend<=previous&&bend>=0;
     const float expected=juce::jmax(0.f,1.f-float(elapsed)/float(duration));
     springTiming=springTiming&&std::abs(bend-expected)<.000001f;
     if(elapsed<duration/2)springTiming=springTiming&&bend>0;
     if(elapsed>=duration)springTiming=springTiming&&bend==0&&!spring.displayedPitchBendReturning.load();
     previous=bend;
   }
   mode(1);spring.queuePitchBend(-1);render();spring.queuePitchBendReturn();render();
   modernReturn=modernReturn&&spring.displayedPitchBend.load()==0&&!spring.displayedPitchBendReturning.load();
   mode(0);spring.queuePitchBend(1);render();spring.queuePitchBendReturn();render();
   events.addEvent(juce::MidiMessage::pitchWheel(16,4096),blockSize/2);render();render();
   interruptions=interruptions&&spring.displayedPitchBend.load()==-.5f&&!spring.displayedPitchBendReturning.load();
   spring.queuePitchBendReturn();render();spring.queuePitchBend(.75f);render();
   interruptions=interruptions&&spring.displayedPitchBend.load()==.75f&&!spring.displayedPitchBendReturning.load();
   spring.queuePitchBendReturn();render();mode(1);render();
   modernReturn=modernReturn&&spring.displayedPitchBend.load()==0&&!spring.displayedPitchBendReturning.load();
 }
 expect(springTiming,"Authentic spring returns at constant speed to exact center at its configured duration across sample rates and buffers");
 expect(modernReturn,"Modern returns immediately, including when entered during an Authentic return");
 expect(interruptions,"new controller wheel and GUI gestures cancel an in-progress spring without stale bend");
 // The visual recoil has one opposite-side lobe and never modifies pitch.
 bool recoil=true;
 for(double from:{-1.,-.25,0.,.25,1.}) {
   recoil=recoil&&kb09_gui::benderReturnPosition(from,0)==from
       &&std::abs(kb09_gui::benderReturnPosition(from,17.5)-from*.5)<1e-12
       &&kb09_gui::benderReturnPosition(from,35)==0
       &&std::abs(kb09_gui::benderReturnPosition(from,45)+from*.02)<1e-12
       &&kb09_gui::benderReturnPosition(from,55)==0
       &&kb09_gui::benderReturnPosition(from,100)==0;
   for(int t=36;t<55;++t)recoil=recoil&&kb09_gui::benderReturnPosition(from,t)*from<=0
       &&std::abs(kb09_gui::benderReturnPosition(from,t))<=std::abs(from)*.0200001;
 }
 expect(recoil,"visual spring crosses center at 35 ms, recoils once by at most 2% and rests exactly at 55 ms for both directions and short bends");
 // Observe actual oscillator pitch, rather than relying only on the display target.
 auto audibleReturn=[&](bool modern) {
   SH09Processor sound;sound.prepareToPlay(48000,64);
   auto set=[&](const char* id,float value){auto* param=sound.state.getParameter(id);param->setValueNotifyingHost(param->convertTo0to1(value));};
   set("modern",modern?1.f:0.f);set("wave",2);set("vcaMode",2);set("cutoff",1);set("vcfEnv",0);set("bendVco",1);
   juce::AudioBuffer<float> block(2,64);juce::MidiBuffer events;events.ensureSize(256);
   events.addEvent(juce::MidiMessage::noteOn(1,69,juce::uint8(100)),0);sound.queuePitchBend(1);
   auto render=[&]{block.clear();track=true;sound.processBlock(block,events);track=false;};
   for(int i=0;i<240;++i)render();
   sound.queuePitchBendReturn();std::array<float,9600> samples{};
   for(size_t start=0;start<samples.size();start+=64){render();for(size_t j=0;j<64;++j)samples[start+j]=block.getSample(0,int(j));}
   // Measure complete periods between interpolated crossings, avoiding the
   // coarse cycle-count rounding of short windows after a quicker return.
   auto frequency=[&](int begin,int end){double first=-1,last=-1;int crossings=0;
     for(int i=begin+1;i<end;++i) {
       const double before=samples[size_t(i-1)],after=samples[size_t(i)];
       if(before<=0&&after>0){const double t=double(i-1)-before/(after-before);if(first<0)first=t;last=t;++crossings;}
     }
     return crossings>=2?double(crossings-1)*48000/(last-first):0.;
   };
   const int duration=int(std::lround(48000*SH09Processor::benderReturnSeconds));
   return std::array<double,3>{frequency(0,duration/5),frequency(duration*2/5,duration*3/5),frequency(6720,9600)};
 };
 const auto authenticPitch=audibleReturn(false),modernPitch=audibleReturn(true);
 std::cout<<"Return pitch Hz Authentic="<<authenticPitch[0]<<','<<authenticPitch[1]<<','<<authenticPitch[2]
          <<" Modern="<<modernPitch[0]<<','<<modernPitch[1]<<','<<modernPitch[2]<<'\n';
 expect(authenticPitch[0]>authenticPitch[1]&&authenticPitch[1]>authenticPitch[2]*1.03
        &&authenticPitch[2]>420&&authenticPitch[2]<460,"Authentic audible pitch returns progressively and settles at the unbent note");
 // With a 35 ms stroke, the first short window shares the existing 8 ms pitch smoothing.
 // The middle window must still clearly separate immediate Modern from Authentic.
 expect(modernPitch[0]<authenticPitch[0]&&modernPitch[1]<authenticPitch[1]*.8&&std::abs(modernPitch[2]-440)<20,
        "Modern audio returns faster while retaining existing click-free smoothing");
 expect(allocations==0,"MIDI feedback and sample-timed spring perform no C++ new allocations");
 {
   SH09Processor wheel;wheel.prepareToPlay(48000,127);
   juce::AudioBuffer<float> block(2,127);juce::MidiBuffer events;events.ensureSize(1024);
   auto* amount=wheel.state.getParameter("vcoMod");
   const auto wheelAmount=[](int v){const float p=v/127.f;return p*p;};
   auto set=[&](float value){amount->setValueNotifyingHost(amount->convertTo0to1(value));};
   auto send=[&](int channel,int value){events.addEvent(juce::MidiMessage::controllerEvent(channel,1,value),0);block.clear();track=true;wheel.processBlock(block,events);track=false;};
   auto savedAmount=[&]{juce::MemoryBlock saved;wheel.getStateInformation(saved);SH09Processor restored;restored.setStateInformation(saved.getData(),int(saved.getSize()));return restored.state.getRawParameterValue("vcoMod")->load();};
   auto deliver=[&](float expected){
     const auto start=juce::Time::getMillisecondCounterHiRes();
     do {juce::Timer::callPendingTimersSynchronously();juce::Thread::sleep(10);}
     while(juce::Time::getMillisecondCounterHiRes()-start<80
       ||(std::abs(amount->getValue()-expected)>1e-6f&&juce::Time::getMillisecondCounterHiRes()-start<2000));
     juce::Timer::callPendingTimersSynchronously();
   };
   for(int mode=0;mode<2;++mode) {
     wheel.state.getParameter("modern")->setValueNotifyingHost(float(mode));set(.6f);wheel.reset();
     send(1,0);send(8,32);
     expect(std::abs(savedAmount()-.6f)<1e-6f,"mod wheel below pickup preserves the preset VCO MOD amount");
     send(16,100);
     expect(std::abs(savedAmount()-wheelAmount(100))<1e-6f,"crossing pickup changes VCO MOD and immediate save retains it in either mode");
     deliver(wheelAmount(100));expect(std::abs(wheel.state.getRawParameterValue("vcoMod")->load()-wheelAmount(100))<1e-6f,"mod wheel publishes the actual parameter off the audio thread");
     send(3,127);deliver(1);expect(std::abs(amount->getValue()-1)<1e-6f,"picked-up wheel follows the full VCO MOD range");
     set(.3f);send(1,110);deliver(.3f);expect(std::abs(amount->getValue()-.3f)<1e-6f,"manual or host parameter edit re-arms pickup");
     send(1,20);deliver(wheelAmount(20));expect(std::abs(amount->getValue()-wheelAmount(20))<1e-6f,"wheel retakes control when it crosses the manually edited fader");
     send(1,120);wheel.setCurrentProgram(0);deliver(0);
     expect(wheel.state.getRawParameterValue("vcoMod")->load()==0&&savedAmount()==0,"new preset rejects queued wheel feedback before the next audio callback");
   }
   expect(allocations==0,"CC1 pickup and per-event modulation processing perform no C++ new allocations");
 }
 {
   SH09Processor reference,wheel;reference.prepareToPlay(48000,127);wheel.prepareToPlay(48000,127);
   juce::AudioBuffer<float> baseline(2,127),moved(2,127);juce::MidiBuffer normal,events;normal.ensureSize(256);events.ensureSize(256);
   normal.addEvent(juce::MidiMessage::noteOn(1,69,juce::uint8(100)),0);events.addEvent(juce::MidiMessage::noteOn(1,69,juce::uint8(100)),0);
   for(int i=0;i<128;++i){baseline.clear();moved.clear();reference.processBlock(baseline,normal);wheel.processBlock(moved,events);}
   events.addEvent(juce::MidiMessage::controllerEvent(1,1,0),0);events.addEvent(juce::MidiMessage::controllerEvent(1,1,127),1);
   double difference=0;
   for(int i=0;i<32;++i){baseline.clear();moved.clear();track=true;reference.processBlock(baseline,normal);wheel.processBlock(moved,events);track=false;for(int j=0;j<127;++j)difference+=std::abs(baseline.getSample(0,j)-moved.getSample(0,j));}
   expect(difference/(32*127)>.01&&wheel.state.getRawParameterValue("vcoMod")->load()==0,
          "CC1 changes actual rendered sound immediately without an editor or message-thread parameter delivery");
   expect(allocations==0,"audible CC1 changes remain allocation-free on the audio thread");
 }
#if KB09_LOCAL_MIDI_TEST
 {
   SH09Processor reference,captured;reference.prepareToPlay(48000,127);captured.prepareToPlay(48000,127);
   captured.noteClickCapture=std::make_unique<NoteClickCapture>(48000,1);
   juce::AudioBuffer<float> a(2,127),b(2,127);juce::MidiBuffer ma,mb;ma.ensureSize(512);mb.ensureSize(512);
   std::vector<float> expected;expected.reserve(48000);bool identical=true;
   for(int block=0;block<400;++block) {
     if(block==0){ma.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),17);mb.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),17);}
     if(block==100){ma.addEvent(juce::MidiMessage::noteOff(1,60),63);mb.addEvent(juce::MidiMessage::noteOff(1,60),63);}
     a.clear();b.clear();track=true;reference.processBlock(a,ma);captured.processBlock(b,mb);track=false;
     for(int sample=0;sample<127;++sample){identical&=a.getSample(0,sample)==b.getSample(0,sample);if(expected.size()<48000)expected.push_back(a.getSample(0,sample));}
   }
   expect(identical,"diagnostic capture leaves every generated audio sample unchanged");
   expect(captured.noteClickCapture->ready(),"bounded diagnostic finishes after its exact sample budget");
   expect(allocations==0,"diagnostic recording allocates no memory in the audio callback");
   const auto folder=juce::File::getCurrentWorkingDirectory().getChildFile("artifacts/NoteClicks255/capture-selftest");
   expect(captured.noteClickCapture->save(folder),"diagnostic output and MIDI report save after recording completes");
   juce::MemoryBlock bytes;folder.getChildFile("synth-output.wav").loadFileAsData(bytes);
   expect(bytes.getSize()==44+expected.size()*sizeof(float)&&std::memcmp(static_cast<const char*>(bytes.getData())+44,expected.data(),expected.size()*sizeof(float))==0,
          "recorded WAV contains the exact processor output without resampling or gain changes");
   const auto report=juce::JSON::parse(folder.getChildFile("midi-and-timing.json"));const auto events=report["midi"];
   expect(events.size()==2&&int(events[0][0])==17&&int(events[1][0])==12763,"capture retains exact MIDI sample positions");
 }
#endif
 p.releaseResources();std::cout<<"RESULT failures="<<failed<<" allocations="<<allocations<<'\n';return failed?1:0;
}
