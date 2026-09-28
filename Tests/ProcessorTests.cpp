#include "../Source/Plugin/Processor.h"
#include <cstdlib>
#include <new>
#include <iostream>
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
 for(int b=0;b<500;++b){for(int i=0;i<20;++i){midi.addEvent(juce::MidiMessage::noteOn(1,40+i,juce::uint8(100)),i*3);midi.addEvent(juce::MidiMessage::noteOff(1,40+i),i*3+1);}p.queueKeyboard(62,true);p.queueKeyboard(62,false);process();}
 expect(allocations==0,"normal render path performs no C++ new allocations");
 juce::AudioBuffer<float> zero(2,0);midi.addEvent(juce::MidiMessage::allSoundOff(1),0);p.processBlock(zero,midi);expect(p.displayedNote.load()==-1,"zero-sample MIDI panic flush");
 p.releaseResources();std::cout<<"RESULT failures="<<failed<<" allocations="<<allocations<<'\n';return failed?1:0;
}
