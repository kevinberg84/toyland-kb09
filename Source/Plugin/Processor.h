#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "../DSP/Engine.h"
#include "../Parameters/Parameters.h"
#include "StandaloneMidiMap.h"
#include "../../Assets/Branding.h"
#include <array>
#include <atomic>
#include <limits>
class SH09Processor final : public juce::AudioProcessor, private juce::VST3ClientExtensions, private juce::Timer,
                            private juce::AudioProcessorValueTreeState::Listener {
public:
 SH09Processor();
 ~SH09Processor() override {stopTimer();state.removeParameterListener("vcoMod",this);}
 const juce::String getName() const override {return sh09::branding::productName;}
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return std::numeric_limits<double>::infinity();}
 int getNumPrograms() override;
 int getCurrentProgram() override {return currentProgram.load();}
 unsigned getProgramRevision() const noexcept {return programRevision.load(std::memory_order_acquire);}
 void setCurrentProgram(int) override;
 const juce::String getProgramName(int) override;
 void changeProgramName(int,const juce::String&) override {}
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void reset() override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool hasEditor() const override {return true;}
 juce::AudioProcessorEditor* createEditor() override;
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::VST3ClientExtensions* getVST3ClientExtensions() override {return this;}
 bool getPluginHasMainInput() const override {return false;}
 bool isPowerEnabled() const noexcept {return powerEnabled.load(std::memory_order_acquire);}
 void setPowerEnabled(bool enabled) noexcept;
 void queueKeyboard(int,bool,bool mouse=true) noexcept;
 static constexpr double benderReturnSeconds=.035;
 // Approved preview timing; the editor and audio spring use the value above.
 static constexpr double proposedBenderReturnSeconds=.035;
 void queuePitchBend(float value) noexcept { guiBendReturn.store(false,std::memory_order_relaxed);guiBend.store(juce::jlimit(-1.0f,1.0f,value),std::memory_order_relaxed);guiBendDirty.store(true,std::memory_order_release); }
 void queuePitchBendReturn() noexcept {guiBend.store(0,std::memory_order_relaxed);guiBendReturn.store(true,std::memory_order_relaxed);guiBendDirty.store(true,std::memory_order_release);}
 juce::AudioProcessorValueTreeState state;
 // Present only in the standalone; custom mappings never enter sound presets.
 std::unique_ptr<sh09::StandaloneMidiMap> midiMap;
 std::atomic<unsigned> midiActivitySerial{0};
 // Audio-thread bend target, shared with the editor without MIDI echo or host automation writes.
 std::atomic<float> displayedPitchBend{0};
 std::atomic<unsigned> displayedPitchBendSerial{0};
 std::atomic<bool> displayedPitchBendReturning{false};
 std::atomic<int> displayedNote{-1};
 // Last oscillator pitch for the VCA HOLD keyboard latch.
 std::atomic<int> displayedHeldNote{-1};
 // GUI contribution already consumed by the audio thread. The editor can
 // replace it with its current mouse note without hiding same-pitch MIDI.
 std::atomic<int> displayedKeyboardNote{-1};
 std::atomic<float> displayedEnvelope{0},displayedFollower{0};
 std::atomic<bool> displayedModulatorLamp{false};
 std::atomic<float> displayedModulatorPhase{0},displayedModulatorFrequency{0},displayedInput{0};
 std::atomic<double> displayedModulatorTime{0};
 // Computer-key contribution consumed by audio; replaced by the live GUI
 // contribution during painting, just like the live mouse contribution.
 std::array<std::atomic<int>,128> displayedComputerKeys{};
 std::array<std::atomic<int>,128> keyDisplay{};
private:
 sh09::Controls readControls() const noexcept;
 void handleMidi(const juce::uint8*,int) noexcept;
 void applyPitchBend(float) noexcept;
 void timerCallback() override;
 void parameterChanged(const juce::String&,float) override;
 void resetModWheel() noexcept;
 void applyModWheel(int) noexcept;
 void clearKeyDisplay() noexcept;
 void clearVoiceDisplay() noexcept;
 void updateKeyDisplay(int,bool) noexcept;
 // One hold per MIDI channel and pitch; GUI holds remain independent.
 std::array<std::array<bool,128>,16> midiHeld{};
 sh09::Engine engine;
 std::array<std::atomic<float>*,sh09::specs.size()> raw{};
 std::atomic<float>* rawVcoMod=nullptr;
 std::atomic<int> currentProgram{0};
 // Transient patch-replacement notification: reselecting the same factory index
 // still replaces a custom sound. Ordinary parameter edits do not advance it.
 std::atomic<unsigned> programRevision{0};
 // Power is a transient performance control, independent of saved parameters.
 // The generation preserves an off/on cycle even if both clicks precede a callback.
 std::atomic<bool> powerEnabled{true};
 std::atomic<unsigned> powerGeneration{0};
 unsigned processedPowerGeneration=0;
 bool processedPowerEnabled=true;
 struct KeyboardEvent {int note;bool down;unsigned generation;bool mouse;};
 std::array<KeyboardEvent,256> keyboardQueue{};
 std::atomic<unsigned> queueRead{0},queueWrite{0};
 std::atomic<bool> queueOverflow{false};
 std::atomic<float> guiBend{0};
 std::atomic<bool> guiBendDirty{false};
 std::atomic<bool> guiBendReturn{false};
 int bendReturnRemaining=0,bendReturnLength=1;
 double bendSampleRate=48000;
 float bendReturnFrom=0,soundingBendTarget=0;
 sh09::Controls renderControls;
 bool modWheelPickedUp=false,modWheelOverride=false;
 float previousModWheel=-1,modWheelTarget=0;
 unsigned modProgramRevision=0,modParameterRevision=0,deliveredModSerial=0;
 std::atomic<unsigned> modParameterEdits{0};
 // Single audio-thread writer; odd serials denote an incomplete mailbox update.
 std::atomic<unsigned> modSerial{0},modFeedbackRevision{0},modFeedbackParameterRevision{0};
 std::atomic<float> modFeedbackValue{0};
 std::atomic<bool> modFeedbackValid{false};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SH09Processor)
};
