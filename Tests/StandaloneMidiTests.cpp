#include "../Source/Plugin/Processor.h"
#include "../Source/GUI/MidiMapOverlay.h"
#include "../Source/GUI/ClosePrompt.h"
#include <iostream>
#include <functional>
#include <cstdlib>
#include <new>

namespace {thread_local bool trackMidiAllocations=false;thread_local unsigned midiAllocations=0;}
void* operator new(std::size_t n){if(trackMidiAllocations)++midiAllocations;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}

namespace {
juce::Component* find(juce::Component& parent,const juce::String& id) {
    if(parent.getComponentID()==id)return &parent;
    for(auto* child:parent.getChildren())if(auto* match=find(*child,id))return match;
    return nullptr;
}
struct Interleave : juce::AudioProcessorParameter::Listener {
    std::function<void()> action;
    void parameterValueChanged(int,float) override {}
    void parameterGestureChanged(int,bool starting) override {
        if(starting&&action){auto callback=std::exchange(action,{});callback();}
    }
};
}
int main(int argc,char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    int failures=0,checks=0;
    auto expect=[&](bool ok,const juce::String& description){++checks;std::cout<<(ok?"PASS ":"FAIL ")<<description<<'\n';if(!ok)++failures;};
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
    SH09Processor processor;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
    processor.prepareToPlay(48000,64);
    auto& map=*processor.midiMap;
    auto index=[](const char* id){return sh09::StandaloneMidiMap::indexFor(id);};
    auto set=[&](const char* id,float value){auto* p=processor.state.getParameter(id);p->setValueNotifyingHost(p->convertTo0to1(value));};
    auto get=[&](const char* id){return processor.state.getRawParameterValue(id)->load();};
    juce::AudioBuffer<float> audio(2,64);juce::MidiBuffer midi;
    auto send=[&](int ch,int cc,int v){midi.addEvent(juce::MidiMessage::controllerEvent(ch,cc,v),31);audio.clear();trackMidiAllocations=true;processor.processBlock(audio,midi);trackMidiAllocations=false;};
    auto close=[](float a,float b){return std::abs(a-b)<1e-5f;};

    map.learn(index("cutoff"));send(3,74,100);map.pump();
    expect(map.assignment(index("cutoff"))==2*128+74&&!map.isLearning(),"learn captures CC and channel from the actual timed MIDI stream");
    set("cutoff",.5f);send(3,74,0);map.pump();
    expect(close(get("cutoff"),.5f),"pickup prevents an initial physical fader jump");
    send(2,74,127);map.pump();expect(close(get("cutoff"),.5f),"other MIDI channels do not control the assignment");
    send(3,74,100);
    expect(close(map.value(size_t(index("cutoff")),.5f),100.f/127),"crossing activates the new audio value before GUI publication");
    juce::MemoryBlock patch;processor.getStateInformation(patch);
    SH09Processor restored;restored.setStateInformation(patch.getData(),int(patch.getSize()));
    expect(close(restored.state.getRawParameterValue("cutoff")->load(),100.f/127),"immediate preset save contains the pending mapped sound value");
    map.pump();expect(close(get("cutoff"),100.f/127),"message-thread publication updates the visible parameter");
    set("cutoff",.3f);send(3,74,110);map.pump();expect(close(get("cutoff"),.3f),"mouse edit re-arms pickup and defeats the previous override");
    send(3,74,0);map.pump();expect(close(get("cutoff"),0),"physical crossing retakes control after a mouse edit");
    send(3,74,127);
    auto* cutoff=processor.state.getParameter("cutoff");Interleave race;
    race.action=[&]{set("cutoff",.25f);};cutoff->addListener(&race);map.pump();cutoff->removeListener(&race);
    expect(close(get("cutoff"),.25f),"a real edit during publication defeats stale queued mapping feedback");

    // Every exposed sound parameter reaches its legal endpoints through MIDI.
    for(size_t i=0;i<sh09::StandaloneMidiMap::count;++i) {
        map.clearAll();map.assign(int(i),16,119);
        send(16,119,0);send(16,119,127);map.pump();
        auto* p=processor.state.getParameter(sh09::specs[i].id);
        expect(close(p->getValue(),1),juce::String(sh09::specs[i].id)+" maps to its legal maximum");
        send(16,119,0);map.pump();expect(close(p->getValue(),0),juce::String(sh09::specs[i].id)+" maps to its legal minimum");
    }
    processor.setCurrentProgram(0);map.clearAll();map.assign(index("age"),1,20);
    send(1,20,50);map.pump();expect(close(get("age"),.5f),"AGE uses exactly five discrete positions");
    map.assign(index("pwmMode"),1,21);send(1,21,64);map.pump();
    expect(close(get("pwmMode"),1),"three-position switches select the middle state");
    map.assign(index("modern"),1,22);send(1,22,127);map.pump();send(1,22,0);map.pump();
    expect(close(get("modern"),0),"momentary 0/127 buttons drive two-position switches");
    const auto saved=map.save();processor.setCurrentProgram(4);
    expect(map.save()==saved,"factory preset selection preserves controller assignments");
    map.clearAll();map.restore(saved);expect(map.save()==saved,"persistent mapping XML round-trips without sound preset data");
    map.assign(index("resonance"),1,20);
    expect(map.assignment(index("age"))==-1&&map.assignment(index("resonance"))==20,"reassigning a physical control removes its old destination");
    map.learn(index("cutoff"));send(1,7,0);map.cancelLearn();map.pump();
    expect(map.assignment(index("cutoff"))==-1,"canceling learn discards an already queued capture");
    map.learn(index("cutoff"));send(1,7,0);map.learn(index("volume"));map.pump();
    send(2,8,0);map.pump();expect(map.assignment(index("cutoff"))==-1&&map.assignment(index("volume"))==136,"changing learn targets rejects an older capture");
    map.learn(index("cutoff"));send(1,123,0);
    expect(map.isLearning(),"all-notes-off stays reserved and cannot become an assignment");map.cancelLearn();
    map.clearAll();set("vcoMod",0);set("cutoff",.5f);map.assign(index("cutoff"),1,1);
    send(1,1,0);send(1,1,127);map.pump();
    expect(close(get("cutoff"),1)&&close(get("vcoMod"),0),"custom CC1 assignment replaces built-in mod-wheel action without double triggering");
    map.clearAll();send(1,1,0);send(1,1,127);map.pump();
    juce::MemoryBlock pendingWheel;processor.getStateInformation(pendingWheel);
    auto pendingWheelXml=juce::AudioProcessor::getXmlFromBinary(pendingWheel.getData(),int(pendingWheel.getSize()));
    bool wheelAudioReady=false;
    if(pendingWheelXml)for(auto* child:pendingWheelXml->getChildIterator())if(child->getStringAttribute("id")=="vcoMod")
        wheelAudioReady=close(float(child->getDoubleAttribute("value")),1);
    const auto wheelDeadline=juce::Time::getMillisecondCounterHiRes()+2000;
    while(!close(get("vcoMod"),1)&&juce::Time::getMillisecondCounterHiRes()<wheelDeadline)
        juce::MessageManager::getInstance()->runDispatchLoopUntil(10);
    if(!wheelAudioReady||!close(get("vcoMod"),1))std::cout<<"clear-all wheel diagnostics: immediate="<<wheelAudioReady<<" published="<<get("vcoMod")<<'\n';
    expect(wheelAudioReady&&close(get("vcoMod"),1),"clear all restores the built-in wheel immediately and publishes through the platform timer");
    const float unchanged=get("cutoff");map.clearAll();
    expect(close(get("cutoff"),unchanged),"clear all preserves the current sound");
    set("cutoff",0);map.assign(index("cutoff"),1,74);send(1,74,0);send(1,74,100);
    map.clearAll();expect(close(get("cutoff"),100.f/127),"clear all preserves even a MIDI value awaiting GUI publication");
    expect(processor.getLatencySamples()==128,"mapping adds no audio buffering or reported latency");

    using Behavior=sh09::StandaloneMidiMap::Behavior;
    auto buttonLearn=[&](const char* id,int first,int released) {
        map.clearAll();map.learn(index(id));send(1,23,first);map.pump();
        expect(map.isLearning()&&map.isCheckingButton(),"endpoint-only multi-position learn waits for the release boundary");
        if(released>=0){send(1,23,released);map.pump();}
        map.finishLearn();
    };
    buttonLearn("vcaMode",127,-1);
    expect(map.behavior(index("vcaMode"))==Behavior::toggleButton,"press/release with no release message automatically identifies a toggle button");
    set("vcaMode",0);send(1,23,0);map.pump();expect(close(get("vcaMode"),1),"toggle OFF press cycles HOLD to ENV");
    send(1,23,127);map.pump();expect(close(get("vcaMode"),2),"toggle ON press cycles ENV to GATE");
    send(1,23,0);map.pump();expect(close(get("vcaMode"),0),"toggle OFF press wraps GATE to HOLD");
    send(1,23,0);map.pump();expect(close(get("vcaMode"),0),"duplicate toggle MIDI feedback does not advance twice");
    send(1,23,127);send(1,23,0);map.pump();expect(close(get("vcaMode"),2),"multiple button presses before GUI publication each advance once");
    set("vcaMode",0);send(1,23,127);map.pump();expect(close(get("vcaMode"),1),"button cycles from the current mouse-edited switch position");
    const auto buttonSaved=map.save();map.clearAll();map.restore(buttonSaved);
    expect(map.behavior(index("vcaMode"))==Behavior::toggleButton&&map.save()==buttonSaved,"learned button behavior survives a settings round trip");
    buttonLearn("vcaMode",0,-1);
    expect(map.behavior(index("vcaMode"))==Behavior::toggleButton,"learning a toggle button that starts with OFF is also recognized");
    set("vcaMode",0);send(1,23,127);map.pump();expect(close(get("vcaMode"),1),"toggle cycles correctly regardless of its starting light state");
    buttonLearn("vcaMode",127,0);
    expect(map.behavior(index("vcaMode"))==Behavior::momentaryHigh,"a release message automatically identifies a momentary button");
    set("vcaMode",0);send(1,23,127);send(1,23,0);map.pump();expect(close(get("vcaMode"),1),"momentary press advances once and release does not skip ENV");
    send(1,23,127);send(1,23,127);send(1,23,0);map.pump();expect(close(get("vcaMode"),2),"momentary repeated ON messages are suppressed until release");
    buttonLearn("vcaMode",0,127);
    expect(map.behavior(index("vcaMode"))==Behavior::momentaryLow,"inverted momentary button polarity is recognized automatically");
    set("vcaMode",0);send(1,23,0);send(1,23,127);map.pump();expect(close(get("vcaMode"),1),"inverted momentary button advances on press only");
    map.clearAll();map.learn(index("vcaMode"));send(1,24,0);map.pump();send(1,24,64);map.pump();
    expect(!map.isLearning()&&map.behavior(index("vcaMode"))==Behavior::direct,"an intermediate knob/fader value automatically selects direct positioning");
    send(1,24,0);map.pump();expect(close(get("vcaMode"),0),"direct fader selects HOLD");
    send(1,24,64);map.pump();expect(close(get("vcaMode"),1),"direct fader selects ENV");
    send(1,24,127);map.pump();expect(close(get("vcaMode"),2),"direct fader selects GATE without cycling");
    map.setBehavior(index("vcaMode"),Behavior::toggleButton);
    expect(map.behavior(index("vcaMode"))==Behavior::toggleButton,"optional correction changes a learned control without remapping its CC");
    map.setBehavior(index("vcaMode"),Behavior::direct);
    send(1,24,64);map.pump();expect(close(get("vcaMode"),1),"optional direct correction restores absolute positioning");
    map.restore("<KB09MidiMap version=\"1\"><CC parameter=\"vcaMode\" channel=\"1\" number=\"23\"/></KB09MidiMap>");
    expect(map.behavior(index("vcaMode"))==Behavior::direct&&map.assignment(index("vcaMode"))==23,"existing test mappings without type metadata remain intact and direct until re-learned");
    buttonLearn("age",127,-1);set("age",0);
    for(int stage=1;stage<=5;++stage){send(1,23,stage%2?0:127);map.pump();expect(close(get("age"),float(stage%5)*.25f),"button cycles through AGE stage "+juce::String(stage%5+1));}
    map.clearAll();map.assign(index("cutoff"),1,74,Behavior::toggleButton);
    expect(map.behavior(index("cutoff"))==Behavior::direct,"continuous sound parameters cannot accidentally acquire a switch-cycling mode");
    map.learn(index("vcaMode"));send(1,23,127);map.pump();map.cancelLearn();map.finishLearn();
    expect(map.assignment(index("vcaMode"))==-1,"canceled button checks do not install a partial assignment");

    {
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
        SH09Processor timed;
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
        SH09Processor reference;
        timed.prepareToPlay(48000,64);reference.prepareToPlay(48000,64);
        timed.midiMap->assign(index("volume"),1,7);
        juce::AudioBuffer<float> whole(2,64),first(2,31),second(2,33);whole.clear();first.clear();second.clear();
        juce::MidiBuffer events;events.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);
        events.addEvent(juce::MidiMessage::controllerEvent(1,7,0),31);
        events.addEvent(juce::MidiMessage::controllerEvent(1,7,127),31);
        timed.processBlock(whole,events);
        events.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);reference.processBlock(first,events);
        reference.state.getParameter("volume")->setValueNotifyingHost(1);reference.processBlock(second,events);
        bool equal=true;
        for(int s=0;s<64;++s)equal&=close(whole.getSample(0,s),s<31?first.getSample(0,s):second.getSample(0,s-31));
        expect(equal,"mapped MIDI changes sound at its sample offset, matching a split-block reference");
    }

    processor.setCurrentProgram(0);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    editor->setSize(1483,816);
    auto footerIsEven=[&](juce::Component& root) {
        for(const auto* id:{"modern","age","velocity","aftertouch","lfoSync","extGain","descriptions","externalAudioHelp"}) {
            if(!find(root,id))return false;
        }
        // Inspect final composited pixels, not the invisible interaction boxes.
        auto* panel=find(root,"externalAudioHelp")->getParentComponent();
        const auto image=panel->createComponentSnapshot({0,747,1483,42},true,4.f,juce::SoftwareImageType());
        const juce::Image::BitmapData pixels(image,juce::Image::BitmapData::readOnly);
        std::vector<juce::Range<int>> runs;
        for(int x=0;x<image.getWidth();++x) {
            int count=0;
            for(int y=0;y<image.getHeight();++y) {
                const auto a=pixels.getPixelColour(x,y),b=pixels.getPixelColour(1,y);
                if(juce::jmax(std::abs(int(a.getRed())-int(b.getRed())),std::abs(int(a.getGreen())-int(b.getGreen())),std::abs(int(a.getBlue())-int(b.getBlue())))>4)++count;
            }
            if(count<3)continue;
            if(runs.empty()||x-runs.back().getEnd()>100)runs.emplace_back(x,x+1);
            else runs.back().setEnd(x+1);
        }
        if(runs.size()!=8)return false;
        int smallest=image.getWidth(),largest=0;
        for(size_t i=1;i<runs.size();++i) {
            const int gap=runs[i].getStart()-runs[i-1].getEnd();
            smallest=juce::jmin(smallest,gap);largest=juce::jmax(largest,gap);
        }
        return largest-smallest<=9&&std::abs(runs.front().getStart()-(image.getWidth()-runs.back().getEnd()))<=9;
    };
    expect(footerIsEven(*editor),"standalone footer has equal gaps between visible control groups");
    {
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_VST3);
        SH09Processor vst;
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
        std::unique_ptr<juce::AudioProcessorEditor> vstEditor(vst.createEditor());vstEditor->setSize(1483,816);
        expect(footerIsEven(*vstEditor),"VST footer has the same equal gaps as standalone");
        auto* help=find(*vstEditor,"externalAudioHelp");
        kb09_gui::ThemedAlert alert("External audio","Shared style",juce::MessageBoxIconType::InfoIcon,help);
        kb09_gui::ClosePrompt reference(vstEditor.get());
        expect(alert.theme.getAlertWindowMessageFont()==reference.theme.getAlertWindowMessageFont()
            &&alert.theme.getAlertWindowTitleFont()==reference.theme.getAlertWindowTitleFont()
            &&close(static_cast<juce::Component&>(alert).getDesktopScaleFactor(),static_cast<juce::Component&>(reference).getDesktopScaleFactor()),
            "VST dialogs match the Save/Don't Save font sizes and effective desktop scale");
        if(argc>1) {
            auto directory=juce::File(argv[1]);directory.createDirectory();
            juce::MessageManager::getInstance()->runDispatchLoopUntil(120);
            auto image=vstEditor->createComponentSnapshot(vstEditor->getLocalBounds(),true,1.5f);
            juce::MemoryOutputStream bytes;juce::PNGImageFormat{}.writeImageToStream(image,bytes);
            if(!directory.getChildFile("vst-footer.png").replaceWithData(bytes.getData(),bytes.getDataSize()))return 1;
        }
    }
    auto* button=dynamic_cast<juce::TextButton*>(find(*editor,"midiMap"));
    expect(button&&button->getTooltip().contains("CLEAR ALL"),"MIDI MAP has DESCRIPTION help explaining learning and reset");
    button->setToggleState(true,juce::dontSendNotification);button->onClick();
    auto* overlay=dynamic_cast<kb09_gui::MidiMapOverlay*>(find(*editor,"midiMapOverlay"));
    expect(overlay&&overlay->isVisible(),"MIDI MAP opens the real highlighted editor overlay");
    kb09_gui::ClosePrompt fontReference(nullptr);
    auto* mappingTheme=dynamic_cast<kb09_gui::DialogTheme*>(&overlay->getLookAndFeel());
    expect(mappingTheme&&mappingTheme->getAlertWindowTitleFont()==fontReference.theme.getAlertWindowTitleFont()
        &&mappingTheme->getAlertWindowMessageFont()==fontReference.theme.getAlertWindowMessageFont(),
        "MIDI mapping uses the Save/Don't Save title/body font sizes and Jost typefaces");
    auto* clearAll=dynamic_cast<juce::TextButton*>(find(*editor,"CLEARALL"));
    auto* clear=dynamic_cast<juce::TextButton*>(find(*editor,"CLEARSELECTED"));
    auto* done=dynamic_cast<juce::TextButton*>(find(*editor,"DONE"));
    auto* referenceButton=dynamic_cast<juce::TextButton*>(fontReference.getButton(1));
    expect(mappingTheme&&done&&referenceButton&&mappingTheme->getTextButtonFont(*done,done->getHeight())
        ==fontReference.theme.getTextButtonFont(*referenceButton,referenceButton->getHeight()),
        "mapping action buttons match the Save/Don't Save reference lettering and size");
    expect(button->getTooltip().length()<170&&overlay->getTooltip().length()<170
        &&overlay->getTooltip().contains("then click DONE"),"DESCRIPTION gives a short learn-and-DONE instruction without a button-type questionnaire");
    expect(clearAll&&clear&&done&&!clearAll->getTooltip().isEmpty()&&!clear->getTooltip().isEmpty()&&!done->getTooltip().isEmpty(),"all mapping actions have DESCRIPTION help");
    auto actionGapsAreEven=[&] {
        std::vector<juce::Rectangle<int>> actions;
        for(auto* child:overlay->getChildren())if(auto* action=dynamic_cast<juce::TextButton*>(child);action&&action->isVisible())actions.push_back(action->getBounds());
        std::sort(actions.begin(),actions.end(),[](auto a,auto b){return a.getY()==b.getY()?a.getX()<b.getX():a.getY()<b.getY();});
        for(size_t i=1;i<actions.size();++i)if(actions[i].getY()==actions[i-1].getY()&&actions[i].getX()-actions[i-1].getRight()!=10)return false;
        return true;
    };
    expect(actionGapsAreEven()&&done->getX()-clearAll->getRight()==10,"hidden OPTIONS leaves no empty slot between CLEAR ALL and DONE");
    overlay->select(index("cutoff"));send(1,74,0);map.pump();overlay->refresh();
    expect(map.assignment(index("cutoff"))==74,"highlighted control learns the selected destination");
    expect(done->getToggleState(),"MIDI mapping starts with DONE as its keyboard default");
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
    expect(clear->getToggleState()&&!done->getToggleState(),"right arrow wraps from DONE to CLEAR SELECTED");
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::downKey));
    expect(clearAll->getToggleState(),"down arrow selects CLEAR ALL");
    overlay->refresh();
    expect(clearAll->getToggleState(),"MIDI updates preserve keyboard action selection");
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
    expect(clear->getToggleState(),"left arrow selects the previous visible action");
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::upKey));
    expect(done->getToggleState(),"up arrow wraps back to DONE and skips hidden OPTIONS");
    if(argc>1) {
        auto directory=juce::File(argv[1]);directory.createDirectory();
        auto capture=[&](const char* name){juce::MessageManager::getInstance()->runDispatchLoopUntil(120);auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f);juce::MemoryOutputStream bytes;expect(juce::PNGImageFormat{}.writeImageToStream(image,bytes)&&directory.getChildFile(name).replaceWithData(bytes.getData(),bytes.getDataSize()),juce::String("actual editor screenshot: ")+name);};
        capture("midi-map-assigned.png");
        overlay->keyPressed(juce::KeyPress(juce::KeyPress::rightKey));capture("midi-map-arrow-clear-selected.png");
        overlay->keyPressed(juce::KeyPress(juce::KeyPress::rightKey));capture("midi-map-arrow-clear-all.png");
        overlay->keyPressed(juce::KeyPress(juce::KeyPress::rightKey));capture("midi-map-arrow-done.png");
        overlay->select(index("resonance"));capture("midi-map-learn.png");
        overlay->select(index("vcaMode"));send(1,23,127);map.pump();overlay->refresh();capture("midi-map-button-check.png");
        const auto box=overlay->getMessageBounds();bool actionsFit=true;
        for(auto* action:{clear,clearAll,done})actionsFit&=box.contains(action->getBounds());
        expect(actionsFit&&overlay->getLocalBounds().contains(box)&&box.getHeight()<=216&&box.getWidth()<=660,"compact mapping strip and its actions fit the unscaled editor");
        map.finishLearn();overlay->refresh();capture("midi-map-button-ready.png");
        overlay->keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
        auto* options=dynamic_cast<juce::TextButton*>(find(*editor,"OPTIONS"));
        expect(options&&options->getToggleState(),"visible OPTIONS participates in arrow cycling");
        overlay->keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
        expect(actionGapsAreEven(),"visible OPTIONS uses the same compact gap as every other action");
        overlay->setVisible(false);set("age",.75f);capture("sold-as-is-key-cracks.png");
        set("age",1.f);capture("fire-hazard-key-cracks.png");
        overlay->setVisible(true);
        set("age",0);overlay->refresh();
        const auto writeImage=[&](const juce::Image& image,const char* name) {
            juce::MemoryOutputStream bytes;return juce::PNGImageFormat{}.writeImageToStream(image,bytes)
                &&directory.getChildFile(name).replaceWithData(bytes.getData(),bytes.getDataSize());
        };
        kb09_gui::ClosePrompt actualReference(editor.get());
        auto* referenceAction=dynamic_cast<juce::TextButton*>(actualReference.getButton(1));
        for(const auto size:{juce::Point<int>(700,450),juce::Point<int>(1483,816),juce::Point<int>(2100,1150)}) {
            editor->setSize(size.x,size.y);juce::MessageManager::getInstance()->runDispatchLoopUntil(220);overlay->refresh();
            const float scale=juce::Component::getApproximateScaleFactorForComponent(overlay);
            const float alertScale=static_cast<juce::Component&>(actualReference).getDesktopScaleFactor();
            expect(overlay->getParentComponent()==editor.get()
                &&close(scale*mappingTheme->getAlertWindowMessageFont().getHeight(),alertScale*actualReference.theme.getAlertWindowMessageFont().getHeight())
                &&close(scale*mappingTheme->getAlertWindowTitleFont().getHeight(),alertScale*actualReference.theme.getAlertWindowTitleFont().getHeight())
                &&close(scale*mappingTheme->getTextButtonFont(*done,done->getHeight()).getHeight(),alertScale*actualReference.theme.getTextButtonFont(*referenceAction,referenceAction->getHeight()).getHeight()),
                "effective title/body/button screen size matches Save/Don't Save at editor width "+juce::String(size.x));
            expect(overlay->getLocalBounds().contains(overlay->getMessageBounds()),"mapping window fits the editor at width "+juce::String(size.x));
            if(size.x>=1483) {
                const auto offset=overlay->getMessageBounds().getCentre()-overlay->getLocalBounds().getCentre();
                expect(std::abs(offset.x)<=1&&std::abs(offset.y)<=1,"mapping window is centered on the synth within integer rounding at editor width "+juce::String(size.x));
            }
            {
                bool unobstructed=true;
                for(size_t i=0;i<sh09::StandaloneMidiMap::count;++i)if(auto* control=find(*editor,sh09::specs[i].id))
                    unobstructed&=!overlay->getMessageBounds().intersects(overlay->getLocalArea(control,control->getLocalBounds()));
                expect(unobstructed,"mapping strip leaves all sound-control targets exposed at width "+juce::String(size.x));
            }
        }
        editor->setSize(1483,816);juce::MessageManager::getInstance()->runDispatchLoopUntil(220);overlay->refresh();
        const auto strip=overlay->createComponentSnapshot(overlay->getMessageBounds(),true,1.5f);
        const auto save=actualReference.createComponentSnapshot(actualReference.getLocalBounds(),true,1.5f);
        juce::Image comparison(juce::Image::RGB,juce::jmax(strip.getWidth(),save.getWidth()),strip.getHeight()+save.getHeight()+32,true,juce::SoftwareImageType());
        {juce::Graphics g(comparison);g.fillAll(juce::Colour(0xff151515));g.drawImageAt(strip,0,0);g.drawImageAt(save,0,strip.getHeight()+32);}
        expect(writeImage(comparison,"mapping-save-same-scale.png"),"actual MIDI strip and Save/Don't Save exported together at identical raster scale");
    }
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
    if(!clearAll->getToggleState())overlay->keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
    expect(clearAll->getToggleState(),"keyboard reaches CLEAR ALL after optional actions");
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(60);
    expect(map.assignment(index("cutoff"))==74&&clearAll->getToggleState(),"Enter requests clear-all confirmation and preserves selected action");
    overlay->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));juce::MessageManager::getInstance()->runDispatchLoopUntil(60);
    expect(map.assignment(index("cutoff"))==-1&&!map.isLearning(),"second Enter confirms clear-all without activating DONE");
    overlay->select(index("cutoff"));done->onClick();
    expect(!overlay->isVisible()&&!map.isLearning(),"DONE closes mapping and cancels pending learn");
    editor.reset();
    expect(restored.midiMap==nullptr,"DAW/undefined processors do not instantiate standalone custom mapping");
    map.clearAll();processor.setCurrentProgram(0);
    const char* destinations[]={"cutoff","resonance","attack","decay","lfoRate","volume","subLevel","extLevel"};
    for(int i=0;i<8;++i)map.assign(index(destinations[i]),1,20+i);
    midi.ensureSize(8192);
    for(int block=0;block<500;++block) {
        midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);
        for(int i=0;i<8;++i)midi.addEvent(juce::MidiMessage::controllerEvent(1,20+i,(block*11+i*7)%128),i*7+1);
        midi.addEvent(juce::MidiMessage::noteOff(1,60),63);
        audio.clear();trackMidiAllocations=true;processor.processBlock(audio,midi);trackMidiAllocations=false;
    }
    expect(processor.displayedNote.load()==-1,"dense mapped controller traffic preserves note-offs");
    {
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
        auto selected=std::make_unique<SH09Processor>();
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
        selected->prepareToPlay(48000,64);
        juce::MidiBuffer events;events.ensureSize(4096);juce::AudioBuffer<float> block(2,64);
        auto render=[&] {block.clear();trackMidiAllocations=true;selected->processBlock(block,events);trackMidiAllocations=false;};
        expect(selected->standaloneMidiChannel()==0,"new standalone defaults to all MIDI channels");
        for(int channel=1;channel<=16;++channel) {
            selected->reset();selected->setStandaloneMidiChannel(channel);
            events.addEvent(juce::MidiMessage::noteOn(channel%16+1,60,juce::uint8(100)),0);render();
            expect(selected->displayedNote.load()==-1,"receive channel "+juce::String(channel)+" excludes other channels");
            events.addEvent(juce::MidiMessage::noteOn(channel,60,juce::uint8(100)),0);render();
            expect(selected->displayedNote.load()==60,"receive channel "+juce::String(channel)+" accepts its notes");
        }
        selected->reset();selected->setStandaloneMidiChannel(0);
        events.addEvent(juce::MidiMessage::noteOn(1,64,juce::uint8(100)),0);
        events.addEvent(juce::MidiMessage::noteOn(16,64,juce::uint8(100)),1);render();
        expect(selected->keyDisplay[64].load()==2,"All accepts independent same-pitch notes on channels 1 and 16");
        selected->queueKeyboard(67,true);selected->setStandaloneMidiChannel(1);render();
        expect(selected->keyDisplay[64].load()==1&&selected->keyDisplay[67].load()==1,"channel change releases excluded MIDI notes and preserves selected MIDI and GUI holds");
        selected->setStandaloneMidiChannel(2);render();
        expect(selected->keyDisplay[64].load()==0&&selected->displayedNote.load()==67,"switching receive channels leaves no excluded stuck note");
        events.addEvent(juce::MidiMessage::pitchWheel(1,16383),0);render();
        expect(selected->displayedPitchBend.load()==0,"excluded channel cannot change pitch bend");
        events.addEvent(juce::MidiMessage::pitchWheel(2,16383),0);render();
        expect(selected->displayedPitchBend.load()>.9f,"selected channel pitch bend works");
        selected->setStandaloneMidiChannel(3);render();
        expect(selected->displayedPitchBend.load()==0,"changing channel clears excluded MIDI bend");
        selected->queuePitchBend(.5f);render();selected->setStandaloneMidiChannel(4);render();
        expect(selected->displayedPitchBend.load()==.5f,"receive channel does not reset the GUI bender");
        selected->midiMap->learn(index("cutoff"));
        events.addEvent(juce::MidiMessage::controllerEvent(3,74,127),0);render();selected->midiMap->pump();
        expect(selected->midiMap->assignment(index("cutoff"))==-1,"excluded controller channel is ignored by MIDI Learn");
        events.addEvent(juce::MidiMessage::controllerEvent(4,74,127),0);render();selected->midiMap->pump();
        expect(selected->midiMap->assignment(index("cutoff"))==3*128+74,"selected controller channel is available to MIDI Learn");
        selected->midiMap->cancelLearn();selected->setCurrentProgram(1);selected->reset();
        juce::MemoryBlock saved;selected->getStateInformation(saved);selected->setStandaloneMidiChannel(5);selected->setStateInformation(saved.getData(),int(saved.getSize()));
        expect(selected->standaloneMidiChannel()==5,"preset loading and reset do not restore the app receive channel");
        selected->setStandaloneMidiChannel(99);expect(selected->standaloneMidiChannel()==0,"invalid stored receive channel falls back to All");
        juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_VST3);
        auto vst=std::make_unique<SH09Processor>();juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
        vst->prepareToPlay(48000,64);vst->setStandaloneMidiChannel(1);
        events.addEvent(juce::MidiMessage::noteOn(16,72,juce::uint8(100)),0);vst->processBlock(block,events);
        expect(vst->standaloneMidiChannel()==0&&vst->displayedNote.load()==72,"DAW plug-ins ignore the standalone-only receive preference");
    }
    expect(midiAllocations==0,"learning and all mapped render paths perform zero C++ new allocations");
    std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<" allocations="<<midiAllocations<<'\n';return failures?1:0;
}
