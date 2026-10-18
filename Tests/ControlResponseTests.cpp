#include "../Source/Plugin/Processor.h"
#include "../Source/Parameters/ControlResponse.h"
#include <iostream>
#include <memory>

juce::Component* find(juce::Component& parent,const juce::String& id) {
    if(parent.getComponentID()==id)return &parent;
    for(auto* child:parent.getChildren())if(auto* result=find(*child,id))return result;
    return nullptr;
}
int main() {
    juce::ScopedJuceInitialiser_GUI gui;
    int failures=0,checks=0;
    const auto expect=[&](bool ok,const juce::String& label){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<label<<'\n';};
    const auto close=[](double a,double b){return std::abs(a-b)<1e-6;};
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
    SH09Processor processor;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
    processor.prepareToPlay(48000,64);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    const auto set=[&](const char* id,float v){auto* p=processor.state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(v));};
    const auto value=[&](const char* id){return processor.state.getRawParameterValue(id)->load();};
    auto& map=*processor.midiMap;
    for(const auto* id:{"portamento","vcoMod","vcfMod","vcfEnv"}) {
        auto* slider=dynamic_cast<juce::Slider*>(find(*editor,id));
        expect(slider!=nullptr,juce::String(id)+" real GUI fader exists");if(!slider)continue;
        expect(close(slider->proportionOfLengthToValue(.1),.01)&&close(slider->proportionOfLengthToValue(.5),.25),juce::String(id)+" first tick and midpoint have the requested effect amounts");
        bool inverse=true,monotonic=true;double previous=-1;
        for(int i=0;i<=100;++i){double p=i/100.0,e=slider->proportionOfLengthToValue(p);inverse&=close(slider->valueToProportionOfLength(e),p);monotonic&=e>=previous;previous=e;}
        expect(inverse&&monotonic&&close(previous,1)&&close(slider->proportionOfLengthToValue(0),0),juce::String(id)+" GUI mapping is monotonic, invertible and preserves both endpoints");
        expect(slider->getTextFromValue(.25)=="5.00"&&close(slider->getValueFromText("5.00"),.25),juce::String(id)+" numeric entry/readout match the printed tick position");
        set(id,.25f);juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
        expect(close(value(id),.25)&&close(slider->valueToProportionOfLength(slider->getValue()),.5),juce::String(id)+" host parameter amount is unchanged and cap displays its inverse position");
        juce::MemoryBlock state;processor.getStateInformation(state);SH09Processor restored;restored.setStateInformation(state.getData(),int(state.getSize()));
        expect(close(restored.state.getRawParameterValue(id)->load(),.25),juce::String(id)+" saved/restored legacy effect amount remains unchanged");
        map.clearAll();map.assign(sh09::StandaloneMidiMap::indexFor(id),1,23);
        map.controller(1,23,0);map.controller(1,23,32);map.pump();
        expect(close(value(id),.25),juce::String(id)+" MIDI pickup waits for inverse position, not the stored effect amount");
        map.controller(1,23,64);map.pump();
        expect(close(value(id),(64.0/127)*(64.0/127)),juce::String(id)+" learned MIDI matches the GUI square curve");
        set(id,.81f);map.controller(1,23,50);map.pump();
        expect(close(value(id),.81),juce::String(id)+" manual parameter edit re-arms MIDI pickup");
        map.controller(1,23,120);map.pump();
        expect(close(value(id),(120.0/127)*(120.0/127)),juce::String(id)+" crossing the new inverse pickup position resumes control");
        map.controller(1,23,127);map.pump();expect(close(value(id),1),juce::String(id)+" MIDI maximum is unchanged");
        map.controller(1,23,0);map.pump();expect(close(value(id),0),juce::String(id)+" MIDI minimum is unchanged");
    }
    map.clearAll();
    for(const auto* id:{"cutoff","lfoRate","attack","decay","release","volume","pulseWidth"}) {
        auto* s=dynamic_cast<juce::Slider*>(find(*editor,id));
        expect(s&&close(s->proportionOfLengthToValue(.5),.5),juce::String(id)+" existing physical response is retained");
    }
#if JUCE_LINUX
    std::unique_ptr<juce::AudioIODeviceType> jack(juce::AudioIODeviceType::createAudioIODeviceType_JACK());
    std::unique_ptr<juce::AudioIODeviceType> alsa(juce::AudioIODeviceType::createAudioIODeviceType_ALSA());
    expect(jack&&jack->getTypeName()=="JACK","Linux build exposes the JACK backend");
    expect(alsa&&alsa->getTypeName()=="ALSA","Linux build retains the ALSA backend");
#endif
    editor.reset();
    std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<'\n';return failures?1:0;
}
