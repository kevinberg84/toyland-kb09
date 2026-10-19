#pragma once
#include "DialogTheme.h"
#include <juce_audio_utils/juce_audio_utils.h>

namespace kb09_gui {
class AudioSettingsDialog final : public ThemedAlert {
public:
    AudioSettingsDialog(juce::AudioDeviceManager& manager, juce::Component* owner,
                        juce::Value* muteInput = nullptr, float condition = -1, std::function<void()> closed = {},
                        int midiChannel = 0, std::function<void(int)> channelChanged = {})
        : ThemedAlert("Audio / MIDI settings", "Choose your audio device and MIDI inputs.",
                      juce::MessageBoxIconType::NoIcon, owner, condition),
          contents(manager,muteInput,midiChannel,std::move(channelChanged)),onClosed(std::move(closed)) {
        viewport.setComponentID("audioSettingsViewport");
        viewport.setViewedComponent(&contents,false);
        viewport.setScrollBarsShown(true,false);
        viewport.setSize(560,450);
        addCustomComponent(&viewport);
        addButton("Close",0,juce::KeyPress(juce::KeyPress::escapeKey));
        ready=true;fitViewport();
    }
    ~AudioSettingsDialog() override { removeCustomComponent(0);if(onClosed)onClosed(); }
    static void show(juce::AudioDeviceManager& manager,juce::Component* owner,juce::Value* muteInput,std::function<void()> closed = {},
                     int midiChannel = 0,std::function<void(int)> channelChanged = {}) {
        (new AudioSettingsDialog(manager,owner,muteInput,-1,std::move(closed),midiChannel,std::move(channelChanged)))->enterModalState(true,nullptr,true);
    }
private:
    // The selector chooses its own height as drivers and channel lists change.
    // Let its content grow, and scroll instead of clipping the MIDI controls.
    class SettingsContents final : public juce::Component {
    public:
        SettingsContents(juce::AudioDeviceManager& manager,juce::Value* muteInput,int midiChannel,std::function<void(int)> channelChanged)
            :selector(manager,0,2,0,2,true,false,true,false) {
            selector.setItemHeight(32);
            addAndMakeVisible(channelLabel);addAndMakeVisible(channelMenu);
            channelLabel.setText("MIDI receive channel",juce::dontSendNotification);
            channelMenu.setComponentID("midiReceiveChannel");
            channelMenu.addItem("All channels",1);
            for(int channel=1;channel<=16;++channel)channelMenu.addItem("Channel "+juce::String(channel),channel+1);
            channelMenu.setSelectedId(juce::jlimit(0,16,midiChannel)+1,juce::dontSendNotification);
            channelMenu.setTooltip("Applies to every enabled MIDI input. All channels accepts channels 1 through 16.");
            channelMenu.onChange=[this,changed=std::move(channelChanged)] {if(changed)changed(channelMenu.getSelectedId()-1);};
            addAndMakeVisible(selector);
            if(muteInput){addAndMakeVisible(mute);mute.getToggleStateValue().referTo(*muteInput);}
            setSize(540,450);
            selector.setBounds(0,40,540,1200);
            followSelector();
        }
        void resized() override {
            mute.setBounds(18,0,juce::jmax(0,getWidth()-36),32);
            selector.setBounds(0,40,getWidth(),selector.getHeight());
            channelLabel.setBounds(18,selector.getBottom()+8,190,32);
            channelMenu.setBounds(215,selector.getBottom()+8,juce::jmax(0,getWidth()-233),32);
        }
        void childBoundsChanged(juce::Component* child) override {
            if(child==&selector)followSelector();
        }
    private:
        void followSelector(){setSize(getWidth(),selector.getBottom()+52);resized();}
        juce::AudioDeviceSelectorComponent selector;
        juce::Label channelLabel;
        juce::ComboBox channelMenu;
        juce::ToggleButton mute{"Mute audio input (prevents feedback)"};
    };
    void resized() override {
        if(!ready)return;
        if(auto* close=getButton(0))close->setTopLeftPosition((getWidth()-close->getWidth())/2,
            juce::roundToInt(getHeight()*.95f)-close->getHeight());
        fitViewport();
    }
    void childBoundsChanged(juce::Component*) override { if(ready)fitViewport(); }
    void fitViewport() {
        if(fitting)return;
        const juce::ScopedValueSetter<bool> guard(fitting,true);
        if(auto* close=getButton(0)) {
            viewport.setBounds(juce::roundToInt(getWidth()*.1f),viewport.getY(),
                               juce::roundToInt(getWidth()*.8f),
                               juce::jmax(0,close->getY()-12-viewport.getY()));
            contents.setSize(juce::jmax(0,viewport.getWidth()-viewport.getScrollBarThickness()),contents.getHeight());
        }
    }
    SettingsContents contents;
    std::function<void()> onClosed;
    juce::Viewport viewport;
    bool ready=false,fitting=false;
};
}
