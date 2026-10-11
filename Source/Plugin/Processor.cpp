#include "Processor.h"
#include "../GUI/Editor.h"
#include "../Presets/Factory.h"
#include <cmath>
#include <string_view>
static_assert(std::atomic<bool>::is_always_lock_free && std::atomic<unsigned>::is_always_lock_free && std::atomic<float>::is_always_lock_free,
              "Power and keyboard controls require lock-free atomics");
SH09Processor::SH09Processor():AudioProcessor(BusesProperties()
 .withInput("External Input",juce::AudioChannelSet::stereo(),true)
 .withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"SH09State",sh09::makeParameters()) {
 for(size_t i=0;i<raw.size();++i)raw[i]=state.getRawParameterValue(sh09::specs[i].id);
 rawVcoMod=state.getRawParameterValue("vcoMod");
 setLatencySamples(engine.latencySamples());
 clearKeyDisplay();
 startTimerHz(60);
}
void SH09Processor::timerCallback() {
 const auto serial=modSerial.load(std::memory_order_acquire);
 if(serial==deliveredModSerial)return;
 deliveredModSerial=serial;
 if(!modFeedbackValid.load(std::memory_order_acquire)||modFeedbackRevision.load()!=getProgramRevision())return;
 auto* parameter=state.getParameter("vcoMod");
 const float source=modFeedbackSource.load(),value=modFeedbackValue.load();
 const float current=state.getRawParameterValue("vcoMod")->load();
 // Host notifications and APVTS listeners belong on the message thread, never
 // in the sample loop. A newer preset or automation edit defeats stale feedback.
 if(std::abs(current-source)<1e-6f||std::abs(current-value)<1e-6f)
   {parameter->beginChangeGesture();parameter->setValueNotifyingHost(parameter->convertTo0to1(value));parameter->endChangeGesture();}
}
void SH09Processor::resetModWheel() noexcept {
 modWheelPickedUp=modWheelOverride=false;previousModWheel=-1;
 lastModParameter=rawVcoMod->load(std::memory_order_relaxed);
 modProgramRevision=programRevision.load(std::memory_order_acquire);
 modFeedbackValid.store(false,std::memory_order_release);
}
void SH09Processor::applyModWheel(int controllerValue) noexcept {
 const float wheel=float(controllerValue)/127;
 const float current=renderControls.vcoMod;
 if(!modWheelPickedUp) {
   const bool near=std::abs(wheel-current)<=1.0f/127;
   const bool crossed=previousModWheel>=0&&(previousModWheel-current)*(wheel-current)<=0;
   modWheelPickedUp=near||crossed;
 }
 previousModWheel=wheel;
 if(!modWheelPickedUp)return;
 modWheelOverride=true;modWheelTarget=wheel;renderControls.vcoMod=wheel;engine.setControls(renderControls);
 modFeedbackSource.store(lastModParameter,std::memory_order_relaxed);
 modFeedbackValue.store(wheel,std::memory_order_relaxed);
 modFeedbackRevision.store(modProgramRevision,std::memory_order_relaxed);
 modFeedbackValid.store(true,std::memory_order_release);modSerial.fetch_add(1,std::memory_order_release);
}
int SH09Processor::getNumPrograms(){return int(sh09::factoryPresets().size());}
const juce::String SH09Processor::getProgramName(int i){return juce::isPositiveAndBelow(i,getNumPrograms())?sh09::factoryPresets()[size_t(i)].name:"";}
void SH09Processor::setCurrentProgram(int index){
 if(!juce::isPositiveAndBelow(index,getNumPrograms()))return;
 for(const auto& spec:sh09::specs){
   // A retained no-op slot for old projects; factory patches have one identity.
   if(std::string_view(spec.id)=="branding")continue;
   float value=spec.initial;
   for(const auto& item:sh09::factoryPresets()[size_t(index)].values)if(std::string_view(item.first)==spec.id)value=item.second;
   auto* parameter=state.getParameter(spec.id);
   parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
 }
 currentProgram.store(index);
 programRevision.fetch_add(1,std::memory_order_release);
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
void SH09Processor::clearKeyDisplay() noexcept {
 for(auto& channel:midiHeld)channel.fill(false);
 for(auto& k:keyDisplay)k.store(0,std::memory_order_relaxed);
 for(auto& k:displayedComputerKeys)k.store(0,std::memory_order_relaxed);
 displayedKeyboardNote.store(-1,std::memory_order_relaxed);
}
void SH09Processor::clearVoiceDisplay() noexcept {
 resetModWheel();
 bendReturnRemaining=0;soundingBendTarget=0;displayedPitchBendReturning.store(false,std::memory_order_relaxed);
 displayedPitchBend.store(0,std::memory_order_relaxed);displayedPitchBendSerial.fetch_add(1,std::memory_order_release);
 clearKeyDisplay();displayedNote.store(-1,std::memory_order_relaxed);displayedHeldNote.store(-1,std::memory_order_relaxed);
 displayedEnvelope.store(0,std::memory_order_relaxed);displayedFollower.store(0,std::memory_order_relaxed);
 displayedModulatorLamp.store(false,std::memory_order_relaxed);
 displayedInput.store(0,std::memory_order_relaxed);displayedModulatorFrequency.store(0,std::memory_order_relaxed);
}
void SH09Processor::updateKeyDisplay(int note,bool down) noexcept {
 auto& key=keyDisplay[size_t(note)];const int count=key.load(std::memory_order_relaxed);
 key.store(juce::jlimit(0,255,count+(down?1:-1)),std::memory_order_relaxed);
}
void SH09Processor::prepareToPlay(double rate,int){
 bendSampleRate=rate;
 engine.prepare(rate);engine.setControls(readControls());setLatencySamples(engine.latencySamples());clearVoiceDisplay();
 queueRead.store(queueWrite.load(std::memory_order_acquire),std::memory_order_release);
}
void SH09Processor::reset(){engine.reset();clearVoiceDisplay();}
bool SH09Processor::isBusesLayoutSupported(const BusesLayout& l) const {
 auto out=l.getMainOutputChannelSet(); auto in=l.getMainInputChannelSet();
 return (out==juce::AudioChannelSet::stereo()||out==juce::AudioChannelSet::mono())
    &&(in.isDisabled()||in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo());
}
void SH09Processor::setPowerEnabled(bool enabled) noexcept {
 if(powerEnabled.exchange(enabled,std::memory_order_acq_rel)!=enabled)
   powerGeneration.fetch_add(1,std::memory_order_release);
}
void SH09Processor::queueKeyboard(int note,bool down,bool mouse) noexcept {
 if(note<0||note>127||!isPowerEnabled())return;
 auto w=queueWrite.load(std::memory_order_relaxed);
 auto next=(w+1u)%unsigned(keyboardQueue.size());
 if(next==queueRead.load(std::memory_order_acquire)){queueOverflow.store(true);return;}
 keyboardQueue[w]={note,down,powerGeneration.load(std::memory_order_acquire),mouse};queueWrite.store(next,std::memory_order_release);
}
void SH09Processor::applyPitchBend(float value) noexcept {
 bendReturnRemaining=0;soundingBendTarget=value;displayedPitchBendReturning.store(false,std::memory_order_relaxed);
 engine.pitchBend(value);
 displayedPitchBend.store(value,std::memory_order_relaxed);
 displayedPitchBendSerial.fetch_add(1,std::memory_order_release);
}
void SH09Processor::handleMidi(const juce::uint8* bytes,int n) noexcept {
 if(n<1)return; int status=bytes[0]&0xf0;
 if(bytes[0]>=0x80&&bytes[0]<0xf0)midiActivitySerial.fetch_add(1,std::memory_order_relaxed);
 if((status==0x90||status==0x80)&&n>=3){int note=bytes[1]&127;bool on=status==0x90&&bytes[2]!=0;
   auto& held=midiHeld[size_t(bytes[0]&15)][size_t(note)];
   if(on){engine.noteOn(note,bytes[2]&127,held);if(!held)updateKeyDisplay(note,true);held=true;}
   else if(held){held=false;engine.noteOff(note);updateKeyDisplay(note,false);}
 }else if(status==0xe0&&n>=3){applyPitchBend(float(((bytes[2]&127)<<7)|(bytes[1]&127))/8192.0f-1.0f);}
 else if(status==0xd0&&n>=2)engine.setAftertouch(float(bytes[1]&127)/127.0f);
 else if(status==0xb0&&n>=3){
   if(bytes[1]==1)applyModWheel(bytes[2]&127);
   else if(bytes[1]==120){engine.allSoundOff();applyPitchBend(0);clearKeyDisplay();resetModWheel();}
   else if(bytes[1]==123){engine.allNotesOff();clearKeyDisplay();}
   else if(bytes[1]==121){applyPitchBend(0);engine.setAftertouch(0);resetModWheel();renderControls.vcoMod=lastModParameter;engine.setControls(renderControls);}
 }
}
void SH09Processor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals guard;
 // Snapshot before Power: an off callback must not drain keys queued by a
 // concurrent power-on later in this block. Those belong to the next callback.
 const bool queueDidOverflow=queueOverflow.exchange(false,std::memory_order_acq_rel);
 const auto w=queueWrite.load(std::memory_order_acquire);
 const bool powered=powerEnabled.load(std::memory_order_acquire);
 const auto generation=powerGeneration.load(std::memory_order_acquire);
 if(generation!=processedPowerGeneration||powered!=processedPowerEnabled){
   // A bounded voice/history reset on the audio thread also clears bend and pressure.
   // Unlike CC120's latched mute, powering on allows an external Hold patch to pass audio.
   engine.reset();clearVoiceDisplay();
   guiBend.store(0,std::memory_order_relaxed);guiBendDirty.store(false,std::memory_order_release);
   processedPowerGeneration=generation;processedPowerEnabled=powered;
 }
 if(!powered){
   queueRead.store(w,std::memory_order_release);
   guiBendDirty.exchange(false,std::memory_order_acq_rel);
   midi.clear();buffer.clear();return;
 }
 const auto controls=readControls();
 const auto revision=programRevision.load(std::memory_order_acquire);
 if(revision!=modProgramRevision||std::abs(controls.vcoMod-lastModParameter)>1e-6f) {
   const bool ownFeedback=revision==modProgramRevision&&modWheelOverride&&std::abs(controls.vcoMod-modWheelTarget)<1e-6f;
   if(!ownFeedback){modWheelPickedUp=modWheelOverride=false;modFeedbackValid.store(false,std::memory_order_release);}
   lastModParameter=controls.vcoMod;modProgramRevision=revision;
 }
 renderControls=controls;if(modWheelOverride)renderControls.vcoMod=modWheelTarget;
 engine.setControls(renderControls);
 if(controls.modern>=.5f&&bendReturnRemaining>0)applyPitchBend(0);
 if(guiBendDirty.exchange(false,std::memory_order_acq_rel)) {
   if(guiBendReturn.load(std::memory_order_relaxed)&&controls.modern<.5f&&std::abs(soundingBendTarget)>.0001f) {
     bendReturnFrom=soundingBendTarget;
     bendReturnLength=juce::jmax(1,juce::roundToInt(bendSampleRate*benderReturnSeconds));
     bendReturnRemaining=bendReturnLength;
     displayedPitchBendReturning.store(true,std::memory_order_relaxed);
   } else applyPitchBend(guiBend.load(std::memory_order_relaxed));
 }
 if(auto* playhead=getPlayHead())if(auto position=playhead->getPosition())if(auto bpm=position->getBpm())engine.setTempo(*bpm);
 auto r=queueRead.load(std::memory_order_relaxed);
 if(queueDidOverflow){engine.allSoundOff();applyPitchBend(0);clearKeyDisplay();r=w;}
 while(r!=w){const auto event=keyboardQueue[r];
   if(event.generation==generation){if(event.down)engine.noteOn(event.note,100);else engine.noteOff(event.note);updateKeyDisplay(event.note,event.down);
     if(!event.mouse) {
       auto& count=displayedComputerKeys[size_t(event.note)];
       count.store(juce::jlimit(0,255,count.load(std::memory_order_relaxed)+(event.down?1:-1)),std::memory_order_relaxed);
     }
     if(event.mouse&&event.down)displayedKeyboardNote.store(event.note,std::memory_order_relaxed);
     else if(event.mouse&&displayedKeyboardNote.load(std::memory_order_relaxed)==event.note)displayedKeyboardNote.store(-1,std::memory_order_relaxed);}
   r=(r+1u)%unsigned(keyboardQueue.size());}
 queueRead.store(r,std::memory_order_release);
 auto input=getBusBuffer(buffer,true,0);auto output=getBusBuffer(buffer,false,0);
 auto event=midi.cbegin(); const auto end=midi.cend();
 float inputPeak=0;bool springAdvanced=false;
 for(int s=0;s<buffer.getNumSamples();++s){
   if(bendReturnRemaining>0) {
     const auto remaining=float(--bendReturnRemaining)/float(bendReturnLength);
     soundingBendTarget=bendReturnFrom*remaining*remaining;
     engine.pitchBend(soundingBendTarget);springAdvanced=true;
   }
   while(event!=end&&(*event).samplePosition<=s){const auto meta=*event;handleMidi(meta.data,meta.numBytes);++event;}
   float external=0;
   for(int ch=0;ch<input.getNumChannels();++ch){float x=input.getSample(ch,s);x=std::isfinite(x)?juce::jlimit(-32.0f,32.0f,x):0;external+=x;inputPeak=juce::jmax(inputPeak,std::abs(x));}
   if(input.getNumChannels()>1)external/=float(input.getNumChannels());
   const float sample=engine.process(external);
   for(int ch=0;ch<output.getNumChannels();++ch)output.setSample(ch,s,sample);
 }
 // Process end-of-block and zero-length flush events without losing note-offs.
 while(event!=end){const auto meta=*event;handleMidi(meta.data,meta.numBytes);++event;}
 if(springAdvanced) {
   displayedPitchBend.store(soundingBendTarget,std::memory_order_relaxed);
   displayedPitchBendReturning.store(bendReturnRemaining>0,std::memory_order_relaxed);
   displayedPitchBendSerial.fetch_add(1,std::memory_order_release);
 }
 midi.clear();displayedNote.store(engine.currentNote(),std::memory_order_relaxed);
 displayedHeldNote.store(engine.lastPlayedNote(),std::memory_order_relaxed);
 displayedEnvelope.store(engine.envelopeLevel(),std::memory_order_relaxed);displayedFollower.store(engine.followerLevel(),std::memory_order_relaxed);
 displayedModulatorLamp.store(engine.modulatorLampOn(),std::memory_order_relaxed);
 displayedInput.store(juce::jmax(inputPeak,displayedInput.load(std::memory_order_relaxed)*float(std::exp(-buffer.getNumSamples()/(getSampleRate()*.15)))),std::memory_order_relaxed);
 displayedModulatorPhase.store(float(engine.modulatorPhase()),std::memory_order_relaxed);
 displayedModulatorFrequency.store(float(engine.modulatorFrequency()),std::memory_order_relaxed);
 displayedModulatorTime.store(juce::Time::getMillisecondCounterHiRes(),std::memory_order_release);
}
juce::AudioProcessorEditor* SH09Processor::createEditor(){return new SH09Editor(*this);}
void SH09Processor::getStateInformation(juce::MemoryBlock& destination){
 auto tree=state.copyState();tree.setProperty("schema",1,nullptr);tree.setProperty("program",currentProgram.load(),nullptr);
 // A save immediately following MIDI must retain the new fader value even
 // before the message-thread parameter notification has been delivered.
 if(modFeedbackValid.load(std::memory_order_acquire)&&modFeedbackRevision.load()==getProgramRevision()) {
   const float current=state.getRawParameterValue("vcoMod")->load();
   const float value=modFeedbackValue.load();
   if(std::abs(current-modFeedbackSource.load())<1e-6f||std::abs(current-value)<1e-6f)
     for(auto child:tree)if(child.getProperty("id")==juce::var("vcoMod"))child.setProperty("value",value,nullptr);
 }
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
   programRevision.fetch_add(1,std::memory_order_release);
 }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SH09Processor;}
