#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <cmath>
int main(int argc,char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    if(argc<2){std::cerr<<"Usage: SH09Host plugin.vst3\n";return 2;}
    juce::VST3PluginFormat format;
    juce::OwnedArray<juce::PluginDescription> descriptions;
    format.findAllTypesForFile(descriptions,juce::String::fromUTF8(argv[1]));
    if(descriptions.isEmpty()){std::cerr<<"FAIL: scan\n";return 1;}
    int failures=0;
    for(auto* d:descriptions) {
        juce::String error;
        auto plugin=format.createInstanceFromDescription(*d,48000,256,error);
        if(!plugin){std::cerr<<"FAIL: instantiate "<<error<<"\n";return 1;}
        std::cout<<"Loaded "<<d->name<<" JUCE "<<juce::SystemStats::getJUCEVersion()<<"\n";
        for(double rate:{44100.0,48000.0,88200.0,96000.0,192000.0}) {
            for(int block:{1,17,64,127,256,1024,8192}) {
                plugin->setRateAndBufferSizeDetails(rate,block);
                plugin->prepareToPlay(rate,block);
                juce::AudioBuffer<float> audio(juce::jmax(2,plugin->getTotalNumOutputChannels(),plugin->getTotalNumInputChannels()),block);
                juce::MidiBuffer midi; double energy=0; int offset=block>17?17:0;
                for(int b=0;b<juce::jmax(10,int(rate*0.08/block));++b) {
                    audio.clear();midi.clear();
                    if(b==0)midi.addEvent(juce::MidiMessage::noteOn(1,69,(juce::uint8)100),offset);
                    plugin->processBlock(audio,midi);
                    for(int c=0;c<audio.getNumChannels();++c)for(int s=0;s<block;++s){const float x=audio.getSample(c,s);if(!std::isfinite(x)||std::abs(x)>8)++failures;energy+=double(x)*x;}
                }
                if(energy<1e-12){++failures;std::cerr<<"FAIL: silence "<<rate<<"/"<<block<<"\n";}
                plugin->releaseResources();
                std::cout<<"Render "<<rate<<"/"<<block<<" energy="<<energy<<"\n";
            }
        }
        auto& parameters=plugin->getParameters();
        if(parameters.isEmpty()){++failures;} else {
            auto* p=parameters[0];p->setValueNotifyingHost(0.37f);
            juce::MemoryBlock state;plugin->getStateInformation(state);
            p->setValueNotifyingHost(0.81f);plugin->setStateInformation(state.getData(),int(state.getSize()));
            if(std::abs(p->getValue()-0.37f)>0.02f){++failures;std::cerr<<"FAIL: state roundtrip\n";}
        }
        if(plugin->hasEditor()){
            for(int i=0;i<3;++i){std::unique_ptr<juce::AudioProcessorEditor> editor(plugin->createEditorIfNeeded());if(!editor)++failures;else{editor->setSize(editor->getWidth()+i,editor->getHeight());plugin->editorBeingDeleted(editor.get());}}
        }else ++failures;
    }
    std::cout<<(failures?"FAIL ":"PASS ")<<failures<<" failures\n";
    return failures?1:0;
}
