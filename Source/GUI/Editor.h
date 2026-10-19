#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "WindowSizing.h"
class SH09Processor;
class SH09Editor final : public juce::AudioProcessorEditor {
public:
    explicit SH09Editor(SH09Processor&);
    ~SH09Editor() override;
    void resized() override;
    void paint(juce::Graphics&) override;
    void parentHierarchyChanged() override;
    void visibilityChanged() override;
    void prepareStandaloneWindow();
    void setAudioSettingsObserver(std::function<void(bool)>);
    void setStandaloneMidiChannelObserver(std::function<void(int)>);
private:
    class Panel;
    sh09::WindowSizing sizing;
    std::unique_ptr<Panel> panel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SH09Editor)
};
