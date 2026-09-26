#include "Processor.h"
#include "../GUI/Editor.h"
#include "../Presets/Factory.h"
#include <cmath>
SH09Processor::SH09Processor():AudioProcessor(BusesProperties()
 .withInput("External Input",juce::AudioChannelSet::stereo(),true)
 .withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"SH09State",sh09::makeParameters()) {
 for(size_t i=0;i<raw.size();++i)raw[i]=state.getRawParameterValue(sh09::specs[i].id);
 clearKeyDisplay();
}
int SH09Processor::getNumPrograms(){return int(sh09::factoryPresets().size());}
const juce::String SH09Processor::getProgramName(int i){return juce::isPositiveAndBelow(i,getNumPrograms())?sh09::factoryPresets()[size_t(i)].name:"";}
void SH09Processor::setCurrentProgram(int index){
 if(!juce::isPositiveAndBelow(index,getNumPrograms()))return;
 for(const auto& spec:sh09::specs){
   if(juce::StringRef(spec.id)=="branding")continue;
   float value=spec.initial;
   for(const auto& item:sh09::factoryPresets()[size_t(index)].values)if(juce::StringRef(item.first)==spec.id)value=item.second;
   auto* parameter=state.getParameter(spec.id);
   parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
 }
 currentProgram.store(index);
}
sh09::Controls SH09Processor::readControls() const noexcept {
 sh09::Controls c;
 size_t i=0;
 #define LOAD(field) c.field=raw[i++]->load(std::memory_order_relaxed)
 LOAD(tune);LOAD(range);LOAD(wave);LOAD(pulseWidth);LOAD(pwmMode);LOAD(vcoLevel);LOAD(subLevel);LOAD(extLevel);LOAD(subMode);
 LOAD(lfoRate);LOAD(lfoDelay);LOAD(lfoWave);LOAD(vcoMod);LOAD(cutoff);LOAD(resonance);LOAD(vcfMod);LOAD(vcfEnv);LOAD(vcfEnvMode);
 LOAD(vcaMode);LOAD(attack);LOAD(decay);LOAD(sustain);LOAD(release);LOAD(envMode);LOAD(portamento);LOAD(bendVco);LOAD(bendVcf);
 LOAD(volume);LOAD(age);LOAD(modern);LOAD(velocity);LOAD(aftertouch);LOAD(lfoSync);LOAD(extGain);
 #undef LOAD
 return c;
}
void SH09Processor::clearKeyDisplay() noexcept {for(auto& k:keyDisplay)k.store(0,std::memory_order_relaxed);}
void SH09Processor::prepareToPlay(double rate,int){
 engine.prepare(rate);engine.setControls(readControls());clearKeyDisplay();displayedNote.store(-1);
 queueRead.store(queueWrite.load(std::memory_order_acquire),std::memory_order_release);
}
void SH09Processor::reset(){engine.reset();clearKeyDisplay();displayedNote.store(-1);}
bool SH09Processor::isBusesLayoutSupported(const BusesLayout& l) const {
 auto out=l.getMainOutputChannelSet(); auto in=l.getMainInputChannelSet();
 return (out==juce::AudioChannelSet::stereo()||out==juce::AudioChannelSet::mono())
    &&(in.isDisabled()||in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo());
}
void SH09Processor::queueKeyboard(int note,bool down) noexcept {
 if(note<0||note>127)return;
 auto w=queueWrite.load(std::memory_order_relaxed);
 auto next=(w+1u)%unsigned(keyboardQueue.size());
 if(next==queueRead.load(std::memory_order_acquire)){queueOverflow.store(true);return;}
 keyboardQueue[w]={note,down};queueWrite.store(next,std::memory_order_release);
}
void SH09Processor::handleMidi(const juce::uint8* bytes,int n) noexcept {
 if(n<1)return; int status=bytes[0]&0xf0;
 if((status==0x90||status==0x80)&&n>=3){int note=bytes[1]&127;bool on=status==0x90&&bytes[2]!=0;
   if(on){engine.noteOn(note,bytes[2]&127);keyDisplay[size_t(note)].store(1,std::memory_order_relaxed);}
   else{engine.noteOff(note);keyDisplay[size_t(note)].store(0,std::memory_order_relaxed);}
 }else if(status==0xe0&&n>=3){engine.pitchBend(float(((bytes[2]&127)<<7)|(bytes[1]&127))/8192.0f-1.0f);}
 else if(status==0xd0&&n>=2)engine.setAftertouch(float(bytes[1]&127)/127.0f);
 else if(status==0xb0&&n>=3){
   if(bytes[1]==120){engine.allSoundOff();clearKeyDisplay();}
   else if(bytes[1]==123){engine.allNotesOff();clearKeyDisplay();}
   else if(bytes[1]==121){engine.pitchBend(0);engine.setAftertouch(0);}
 }
}
void SH09Processor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals guard;
 engine.setControls(readControls());
 if(guiBendDirty.exchange(false,std::memory_order_acq_rel))engine.pitchBend(guiBend.load(std::memory_order_relaxed));
 if(auto* playhead=getPlayHead())if(auto position=playhead->getPosition())if(auto bpm=position->getBpm())engine.setTempo(*bpm);
 if(queueOverflow.exchange(false)){engine.allSoundOff();clearKeyDisplay();}
 auto r=queueRead.load(std::memory_order_relaxed); const auto w=queueWrite.load(std::memory_order_acquire);
 while(r!=w){const auto event=keyboardQueue[r];if(event.down)engine.noteOn(event.note,100);else engine.noteOff(event.note);keyDisplay[size_t(event.note)].store(event.down?1:0,std::memory_order_relaxed);r=(r+1u)%unsigned(keyboardQueue.size());}
 queueRead.store(r,std::memory_order_release);
 auto input=getBusBuffer(buffer,true,0);auto output=getBusBuffer(buffer,false,0);
 auto event=midi.cbegin(); const auto end=midi.cend();
 for(int s=0;s<buffer.getNumSamples();++s){
   while(event!=end&&(*event).samplePosition<=s){const auto meta=*event;handleMidi(meta.data,meta.numBytes);++event;}
   float external=0;
   for(int ch=0;ch<input.getNumChannels();++ch){float x=input.getSample(ch,s);external+=std::isfinite(x)?juce::jlimit(-32.0f,32.0f,x):0;}
   if(input.getNumChannels()>1)external/=float(input.getNumChannels());
   const float sample=engine.process(external);
   for(int ch=0;ch<output.getNumChannels();++ch)output.setSample(ch,s,sample);
 }
 // Process end-of-block and zero-length flush events without losing note-offs.
 while(event!=end){const auto meta=*event;handleMidi(meta.data,meta.numBytes);++event;}
 midi.clear();displayedNote.store(engine.currentNote(),std::memory_order_relaxed);
 displayedEnvelope.store(engine.envelopeLevel(),std::memory_order_relaxed);displayedFollower.store(engine.followerLevel(),std::memory_order_relaxed);
}
juce::AudioProcessorEditor* SH09Processor::createEditor(){return new SH09Editor(*this);}
void SH09Processor::getStateInformation(juce::MemoryBlock& destination){
 auto tree=state.copyState();tree.setProperty("schema",1,nullptr);tree.setProperty("program",currentProgram.load(),nullptr);
 if(auto xml=tree.createXml())copyXmlToBinary(*xml,destination);
}
void SH09Processor::setStateInformation(const void* data,int size){
 if(size<=0||size>4*1024*1024)return;
 if(auto xml=getXmlFromBinary(data,size);xml&&xml->hasTagName("SH09State")){
   auto tree=juce::ValueTree::fromXml(*xml);if(int(tree.getProperty("schema",1))!=1)return;
   for(int i=0;i<tree.getNumChildren();++i){auto child=tree.getChild(i);const auto id=child.getProperty("id").toString();
     if(auto* p=state.getParameter(id)){double v=child.getProperty("value");if(!std::isfinite(v))return;auto range=p->getNormalisableRange();child.setProperty("value",juce::jlimit(range.start,range.end,float(v)),nullptr);}
   }
   currentProgram.store(juce::jlimit(0,getNumPrograms()-1,int(tree.getProperty("program",0))));state.replaceState(tree);
 }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SH09Processor;}
