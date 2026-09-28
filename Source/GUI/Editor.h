#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
class SH09Processor;
class SH09Editor final : public juce::AudioProcessorEditor {
public:
    explicit SH09Editor(SH09Processor&);
    ~SH09Editor() override;
    void resized() override;
private:
    class Panel;
    std::unique_ptr<Panel> panel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SH09Editor)
};
