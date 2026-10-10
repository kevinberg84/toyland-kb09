#pragma once
#include "DialogTheme.h"
#include <juce_audio_utils/juce_audio_utils.h>

namespace kb09_gui {
class AudioSettingsDialog final : public ThemedAlert {
public:
    AudioSettingsDialog(juce::AudioDeviceManager& manager, juce::Component* owner,
                        juce::Value* muteInput = nullptr, float condition = -1)
        : ThemedAlert("Audio / MIDI settings", "Choose your audio device and MIDI inputs.",
                      juce::MessageBoxIconType::NoIcon, owner, condition),
          selector(manager,0,2,0,2,true,false,true,false) {
        contents.setSize(540,450);
        contents.addAndMakeVisible(selector);
        selector.setBounds(0,40,540,400);
        if(muteInput){contents.addAndMakeVisible(mute);mute.getToggleStateValue().referTo(*muteInput);mute.setBounds(18,0,500,30);}
        addCustomComponent(&contents);
        addButton("Close",0,juce::KeyPress(juce::KeyPress::escapeKey));
    }
    ~AudioSettingsDialog() override { removeCustomComponent(0); }
    static void show(juce::AudioDeviceManager& manager,juce::Component* owner,juce::Value* muteInput) {
        (new AudioSettingsDialog(manager,owner,muteInput))->enterModalState(true,nullptr,true);
    }
private:
    juce::Component contents;
    juce::AudioDeviceSelectorComponent selector;
    juce::ToggleButton mute{"Mute audio input (prevents feedback)"};
};
}
