#include "../Source/Plugin/Processor.h"
#include <cstdlib>
#include <new>
#include <iostream>
#include <cmath>
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
 p.releaseResources();std::cout<<"RESULT failures="<<failed<<" allocations="<<allocations<<'\n';return failed?1:0;
}
