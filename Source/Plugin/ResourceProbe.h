#pragma once
#include "Processor.h"
#if KB09_LOCAL_MIDI_TEST
class ResourceProbe final : private juce::Timer {
public:
    ResourceProbe(SH09Processor& p,juce::AudioDeviceManager& d,juce::Component& root,juce::File report,std::function<void()> finished,bool clicks=false)
      :processor(p),devices(d),owner(root),file(std::move(report)),done(std::move(finished)),noteClicks(clicks) {
        processor.state.getParameter("age")->setValueNotifyingHost(noteClicks?0.f:1.f);
        if(noteClicks){processor.state.getParameter("attack")->setValueNotifyingHost(.6f);processor.state.getParameter("release")->setValueNotifyingHost(.65f);}
        began=juce::Time::getMillisecondCounterHiRes();startTimer(50);
    }
private:
    static juce::Slider* find(juce::Component& c,const juce::String& id) {
        if(c.getComponentID()==id)return dynamic_cast<juce::Slider*>(&c);
        for(auto* child:c.getChildren())if(auto* match=find(*child,id))return match;
        return nullptr;
    }
    void timerCallback() override {
        const double elapsed=(juce::Time::getMillisecondCounterHiRes()-began)/1000.;
        const int next=elapsed<5?0:elapsed<9?1:elapsed<13?2:3;
        if(next!=phase){if(phase>=0)phaseXruns[size_t(phase)]=devices.getXRunCount()-phaseXrunStart;phaseXrunStart=devices.getXRunCount();phase=next;if(noteClicks&&phase==3)owner.setVisible(false);file.getSiblingFile(file.getFileName()+".phase").replaceWithText(juce::String(phase));if(phase==1)xrunsBefore=devices.getXRunCount();}
        if(phase>=2&&++ticks%(noteClicks?1:5)==0) {
            if(note>=0)processor.queueKeyboard(note,false,false);
            note=53+(sequence++%24);processor.queueKeyboard(note,true,false);
            if(phase==3&&!noteClicks) {
                if(auto* range=find(owner,"range"))range->setValue(sequence%5,juce::sendNotificationSync);
                if(auto* wave=find(owner,"wave"))wave->setValue(sequence%4,juce::sendNotificationSync);
            }
        }
        audio[size_t(phase)].add(devices.getCpuUsage()*100.);
        if(elapsed>=17) {
            stopTimer();phaseXruns[size_t(phase)]=devices.getXRunCount()-phaseXrunStart;if(note>=0)processor.queueKeyboard(note,false,false);
            juce::DynamicObject::Ptr result=new juce::DynamicObject;
            if(auto* device=devices.getCurrentAudioDevice()) {
                result->setProperty("outputDevice",device->getName());result->setProperty("sampleRate",device->getCurrentSampleRate());result->setProperty("bufferSize",device->getCurrentBufferSizeSamples());
            }
            juce::Array<juce::var> phases;
            for(size_t i=0;i<audio.size();++i) {
                juce::DynamicObject::Ptr p=new juce::DynamicObject;double sum=0,peak=0;
                for(double value:audio[i]){sum+=value;peak=juce::jmax(peak,value);}
                p->setProperty("phase",int(i));p->setProperty("xruns",phaseXruns[i]);p->setProperty("audioCpuMeanPercent",sum/juce::jmax(1,audio[i].size()));p->setProperty("audioCpuPeakPercent",peak);phases.add(juce::var(p.get()));
            }
            result->setProperty("phases",juce::var(phases));result->setProperty("xrunsDuringMeasuredPhases",devices.getXRunCount()-xrunsBefore);
            file.replaceWithText(juce::JSON::toString(juce::var(result.get())));done();
        }
    }
    SH09Processor& processor;juce::AudioDeviceManager& devices;juce::Component& owner;juce::File file;std::function<void()> done;
    bool noteClicks=false;std::array<int,4> phaseXruns{};int phaseXrunStart=0;
    double began=0;int phase=-1,ticks=0,note=-1,sequence=0,xrunsBefore=0;std::array<juce::Array<double>,4> audio;
};
#endif
