#include <juce_audio_utils/juce_audio_utils.h>
#include <cmath>
class Lab final : public juce::AudioProcessor {
public:
    Lab() : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
        state(*this, nullptr, "Lab", {std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"cutoff",1},"Cutoff",juce::NormalisableRange<float>(40,18000,0,0.3f),5000.0f),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gain",1},"Gain",0.0f,0.5f,0.15f)}) {}
    const juce::String getName() const override { return "SH09 VST Lab"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Init"; }
    void changeProgramName(int,const juce::String&) override {}
    void prepareToPlay(double rate,int) override { sr=rate; phase=lp=0; on=false; }
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& l) const override { return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo(); }
    void processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) override {
        juce::ScopedNoDenormals guard;
        auto it=midi.cbegin();
        const float a=1.0f-std::exp(-2.0f*juce::MathConstants<float>::pi*state.getRawParameterValue("cutoff")->load()/float(sr));
        for(int i=0;i<audio.getNumSamples();++i) {
            while(it!=midi.cend() && (*it).samplePosition<=i) { auto m=(*it).getMessage(); if(m.isNoteOn()){hz=juce::MidiMessage::getMidiNoteInHertz(m.getNoteNumber());on=true;} if(m.isNoteOff()||m.isAllNotesOff()||m.isAllSoundOff()) on=false; ++it; }
            float v=on?float(std::sin(phase*juce::MathConstants<double>::twoPi))*state.getRawParameterValue("gain")->load():0;
            phase+=hz/sr; phase-=std::floor(phase); lp+=a*(v-lp);
            for(int c=0;c<audio.getNumChannels();++c)audio.setSample(c,i,lp);
        }
        midi.clear();
    }
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override { return new juce::GenericAudioProcessorEditor(*this); }
    void getStateInformation(juce::MemoryBlock& b) override { auto xml=state.copyState().createXml(); copyXmlToBinary(*xml,b); }
    void setStateInformation(const void* d,int n) override { if(auto xml=getXmlFromBinary(d,n);xml && xml->hasTagName("Lab"))state.replaceState(juce::ValueTree::fromXml(*xml)); }
private:
    juce::AudioProcessorValueTreeState state;
    double sr=44100,phase=0,hz=440;float lp=0;bool on=false;
};
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new Lab; }
