#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_data_structures/juce_data_structures.h>

namespace sh09 {
// Input intent is independent of the audio driver's state XML. An absent or
// temporarily busy controller must not turn a remembered choice into "none".
class MidiInputSelection {
public:
    using Devices=juce::Array<juce::MidiDeviceInfo>;
    bool restore(const juce::XmlElement* xml) {
        if(!xml || !(xml->hasTagName("KB09_MIDI_INPUTS")||xml->hasTagName("DEVICESETUP")))return false;
        selected.clear();
        for(auto* item:xml->getChildWithTagNameIterator("MIDIINPUT")) {
            const juce::MidiDeviceInfo info(item->getStringAttribute("name"),item->getStringAttribute("identifier"));
            if(info.identifier.isNotEmpty()||info.name.isNotEmpty())selected.addIfNotAlreadyThere(info);
        }
        return true;
    }
    juce::XmlElement save() const {
        juce::XmlElement xml("KB09_MIDI_INPUTS");
        for(const auto& info:selected) {
            auto* item=xml.createNewChildElement("MIDIINPUT");
            item->setAttribute("name",info.name);item->setAttribute("identifier",info.identifier);
        }
        return xml;
    }
    void capture(const Devices& available,const juce::StringArray& enabled) {
        // Retain inputs that cannot currently be selected in the device list.
        for(int i=selected.size();--i>=0;)
            for(const auto& device:available)
                if(device.identifier==selected[i].identifier || uniqueNameMatch(device,selected[i],available)) {
                    selected.remove(i);break;
                }
        for(const auto& device:available)if(enabled.contains(device.identifier))selected.addIfNotAlreadyThere(device);
    }
    juce::StringArray wanted(const Devices& available) {
        juce::StringArray result;
        for(auto& saved:selected) {
            const juce::MidiDeviceInfo* match=nullptr;
            for(const auto& device:available)if(device.identifier==saved.identifier){match=&device;break;}
            if(!match)for(const auto& device:available)
                if(uniqueNameMatch(device,saved,available)){match=&device;break;}
            if(match){saved=*match;result.addIfNotAlreadyThere(match->identifier);}
        }
        return result;
    }
    void observe(const Devices& available,const juce::StringArray& enabled) {
        // A list change is a connection event, not a user's change of selection.
        if(haveObservation && available==lastAvailable && enabled!=lastEnabled)capture(available,enabled);
        acknowledge(available,enabled);
    }
    void acknowledge(const Devices& available,const juce::StringArray& enabled) {
        lastAvailable=available;lastEnabled=enabled;haveObservation=true;
    }
private:
    bool uniqueNameMatch(const juce::MidiDeviceInfo& device,const juce::MidiDeviceInfo& saved,const Devices& available) const {
        if(saved.name.isEmpty()||device.name!=saved.name)return false;
        int devices=0,choices=0;
        for(const auto& d:available)devices+=d.name==saved.name;
        for(const auto& d:selected)choices+=d.name==saved.name;
        return devices==1&&choices==1;
    }
    Devices selected,lastAvailable;
    juce::StringArray lastEnabled;
    bool haveObservation=false;
};

// Runs only on the standalone message thread. No file access, enumeration or
// device opening is added to processBlock or to the plug-in formats.
class StandaloneMidiInputs final : private juce::ChangeListener,private juce::Timer {
public:
    StandaloneMidiInputs(juce::AudioDeviceManager& devices,juce::PropertiesFile& preferences)
        :manager(devices),settings(preferences) {
        auto saved=settings.getXmlValue("standaloneMidiInputs");
        if(!selection.restore(saved.get())) {
            auto legacy=settings.getXmlValue("audioSetup");
            if(!selection.restore(legacy.get())) {
                const auto available=juce::MidiInput::getAvailableDevices();
                selection.capture(available,enabled(available));
            }
        }
        update(true);manager.addChangeListener(this);startTimer(1000);
    }
    ~StandaloneMidiInputs() override {
        if(editing)update(false);
        stopTimer();manager.removeChangeListener(this);store();
    }
    void setEditing(bool active) {
        if(active) {
            const auto available=juce::MidiInput::getAvailableDevices();
            selection.acknowledge(available,enabled(available));editing=true;
        } else {update(false);editing=false;}
    }
private:
    juce::StringArray enabled(const MidiInputSelection::Devices& available) const {
        juce::StringArray result;
        for(const auto& d:available)if(manager.isMidiInputDeviceEnabled(d.identifier))result.add(d.identifier);
        return result;
    }
    void changeListenerCallback(juce::ChangeBroadcaster*) override { update(false); }
    void timerCallback() override { update(true); }
    void update(bool retry) {
        const auto available=juce::MidiInput::getAvailableDevices();
        // Only changes made in AUDIO / MIDI are new user choices. A driver
        // reset or failed connection outside that dialog must not erase intent.
        if(editing)selection.observe(available,enabled(available));
        else selection.acknowledge(available,enabled(available));
        const auto wanted=selection.wanted(available);
        for(const auto& old:lastAvailable)if(!available.contains(old))manager.setMidiInputDeviceEnabled(old.identifier,false);
        for(const auto& d:available) {
            const bool desired=wanted.contains(d.identifier);
            if(manager.isMidiInputDeviceEnabled(d.identifier)!=desired
                && (!desired || retry || available!=lastAvailable))
                manager.setMidiInputDeviceEnabled(d.identifier,desired);
        }
        // A failed open remains pending; programmatic opens are not user edits.
        selection.acknowledge(available,enabled(available));lastAvailable=available;
        store();
    }
    void store() {
        const auto xml=selection.save();settings.setValue("standaloneMidiInputs",&xml);
        if(const auto audio=manager.createStateXml())settings.setValue("audioSetup",audio.get());
        settings.saveIfNeeded();
    }
    juce::AudioDeviceManager& manager;
    juce::PropertiesFile& settings;
    MidiInputSelection selection;
    MidiInputSelection::Devices lastAvailable;
    bool editing=false;
};
}
