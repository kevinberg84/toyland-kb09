#include "../Source/Plugin/Processor.h"
#include <iostream>
#include <memory>
#include <set>

static juce::Component* findRate(juce::Component& p) {
    if(p.getComponentID()=="rateLamp")return &p;
    for(auto* c:p.getChildren())if(auto* found=findRate(*c))return found;
    return nullptr;
}
int main() {
    juce::ScopedJuceInitialiser_GUI gui;int checks=0,failures=0;
    const auto expect=[&](bool ok,const juce::String& name){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<name<<'\n';};
    for(auto wrapper:{juce::AudioProcessor::wrapperType_Standalone,juce::AudioProcessor::wrapperType_VST3,juce::AudioProcessor::wrapperType_AudioUnit,juce::AudioProcessor::wrapperType_AAX}) {
        juce::AudioProcessor::setTypeOfNextNewPlugin(wrapper);SH09Processor p;juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
        p.prepareToPlay(48000,128);
        const auto set=[&](const char* id,float v){auto* param=p.state.getParameter(id);param->setValueNotifyingHost(param->convertTo0to1(v));};
        const auto label=juce::String("wrapper ")+juce::String(int(wrapper))+": ";
        set("modern",0);set("lfoRate",1);juce::AudioBuffer<float> audio(2,128);juce::MidiBuffer midi;
        for(int i=0;i<400;++i){audio.clear();p.processBlock(audio,midi);}
        expect(p.displayedModulatorFrequency.load()>24,label+"audio snapshot starts in the steady-bright high-rate range");
        set("lfoRate",2.f/3.f);
        expect(std::abs(p.rateLampFrequency()-5)<1e-5,label+"live visual rate becomes 5 Hz without another audio callback");
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());auto* lamp=findRate(*editor);
        expect(lamp!=nullptr,label+"shared RATE renderer exists");if(!lamp)continue;
        const auto blink=[&](int ms){std::set<int> levels;const auto end=juce::Time::getMillisecondCounterHiRes()+ms;
            while(juce::Time::getMillisecondCounterHiRes()<end){juce::MessageManager::getInstance()->runDispatchLoopUntil(18);
                auto image=lamp->createComponentSnapshot(lamp->getLocalBounds(),true,3.f);levels.insert(int(image.getPixelAt(image.getWidth()/2,image.getHeight()/2).getARGB()));}
            return levels.size();};
        expect(blink(430)>=2,label+"fallback pixels blink while audio processing is idle");
        set("age",1);expect(blink(430)>=2,label+"age changes do not freeze the renderer");
        p.reset();expect(blink(430)>=2,label+"host reset does not freeze the enabled lamp");
        p.setPowerEnabled(false);juce::MessageManager::getInstance()->runDispatchLoopUntil(30);
        expect(blink(250)==1,label+"POWER off gives a stable unlit renderer");
        p.setPowerEnabled(true);expect(blink(430)>=2,label+"POWER on resumes blinking with audio idle");
        p.prepareToPlay(96000,512);expect(blink(430)>=2,label+"host sample-rate reinitialization does not freeze the lamp");
        editor.reset();editor.reset(p.createEditor());lamp=findRate(*editor);
        expect(blink(430)>=2,label+"reopening the editor while audio is idle resumes blinking");
        set("modern",1);set("lfoSync",1);set("lfoRate",7.f/20);
        expect(std::abs(p.rateLampFrequency()-2)<1e-5,label+"synced visual rate uses the default 120 BPM with audio idle");
        set("lfoRate",10.f/20);expect(std::abs(p.rateLampFrequency()-4)<1e-5,label+"sync division changes also apply without audio callbacks");
    }
    std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<'\n';return failures?1:0;
}
