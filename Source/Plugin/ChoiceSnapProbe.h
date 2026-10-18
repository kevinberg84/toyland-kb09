#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#if KB09_LOCAL_MIDI_TEST
// Runs input sequences in a real, visible test-standalone peer. Frame records
// are emitted by the real knob paint callback, not by an offline sampler.
class ChoiceSnapProbe final : private juce::Timer {
public:
    ChoiceSnapProbe(juce::Component& root,std::function<void(juce::var)> finished):owner(&root),done(std::move(finished)){startTimer(8);}
private:
    static juce::Component* find(juce::Component& c,const juce::String& id) {
        if(c.getComponentID()==id)return &c;
        for(auto* child:c.getChildren())if(auto* match=find(*child,id))return match;
        return nullptr;
    }
    juce::MouseEvent event(juce::Point<float> point,bool down) {
        const auto now=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,
            juce::ModifierKeys(down?juce::ModifierKeys::leftButtonModifier:0),1,0,0,0,0,slider,slider,now,start,now,1,true);
    }
    void timerCallback() override {
        if(!owner){stopTimer();return;}
        if(tick==0) {
            slider=dynamic_cast<juce::Slider*>(find(*owner,index==0?"range":"wave"));
            if(!slider){stopTimer();done(juce::var());return;}
            slider->getProperties().set("recordChoiceSnap",false);
            slider->setValue(slider->getMaximum(),juce::sendNotificationSync);
            slider->setValue(0,juce::sendNotificationSync);
            start=slider->getLocalBounds().toFloat().getCentre();
        }
        if(tick==16) {
            slider->getProperties().set("choiceSnapFrames",juce::var(juce::Array<juce::var>()));
            slider->getProperties().set("recordChoiceSnap",true);slider->mouseDown(event(start,true));
        }
        if(tick>=17&&tick<=36)slider->mouseDrag(event(start+juce::Point<float>(float(tick-16)*2,0),true));
        if(tick==37)slider->mouseUp(event(start+juce::Point<float>(40,0),false));
        if(tick==52)slider->setValue(2,juce::sendNotificationSync);
        if(tick==54)slider->setValue(0,juce::sendNotificationSync);
        if(tick==72)slider->setValue(1,juce::sendNotificationSync);
        if(tick==88) {
            juce::DynamicObject::Ptr record=new juce::DynamicObject;
            record->setProperty("control",slider->getComponentID());record->setProperty("frames",slider->getProperties()["choiceSnapFrames"]);
            results.add(juce::var(record.get()));slider->getProperties().set("recordChoiceSnap",false);
            if(++index==2){stopTimer();done(juce::var(results));return;}
            tick=-1;
        }
        ++tick;
    }
    juce::Component::SafePointer<juce::Component> owner;
    std::function<void(juce::var)> done;juce::Slider* slider=nullptr;
    juce::Point<float> start;int tick=0,index=0;juce::Array<juce::var> results;
};
#endif
