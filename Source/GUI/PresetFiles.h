#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>
#include <memory>

class SH09Processor;

namespace sh09 {

/** Preset-file UI and bounded, validated disk I/O. Use on the message thread.

    Own this helper for the editor lifetime, after the processor reference:
      PresetFiles presetFiles { processor, *this };
      saveButton.onClick = [this] { presetFiles.savePreset(); };
      loadButton.onClick = [this] { presetFiles.loadPreset(); };

    New .kb09 files contain the processor's complete binary XML state.
    Earlier exports remain readable for compatibility. Power, held keys and
    window placement are intentionally not saved.
    A load is validated before the processor is touched. File dialogs are async
    and are cancelled when this helper/editor is destroyed. No audio-thread I/O.
*/
class PresetFiles final {
public:
    using Completion = std::function<void(const juce::Result&, const juce::File&, bool wasLoad)>;
    static constexpr std::size_t maximumFileBytes = 4 * 1024 * 1024;

    PresetFiles(SH09Processor&, juce::Component& dialogOwner);
    ~PresetFiles();

    void savePreset();
    void loadPreset();
    bool isDialogOpen() const noexcept;

    // Copied when a dialog is opened. Called on success or error, not cancellation.
    // Errors also receive a readable async alert. This may update a status label.
    Completion onCompleted;

    // Synchronous message-thread entry points for tests / explicit file actions.
    // The caller has already selected/confirmed any replacement at this layer.
    static juce::Result saveToFile(SH09Processor&, const juce::File&);
    static juce::Result loadFromFile(SH09Processor&, const juce::File&);

    // Pure validation/extraction; output is assigned only on success. Does not
    // instantiate a plug-in or restore host/controller/bypass data from wrappers.
    static juce::Result decodePresetData(const void*, std::size_t,
                                         juce::MemoryBlock& processorState);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetFiles)
};

} // namespace sh09
