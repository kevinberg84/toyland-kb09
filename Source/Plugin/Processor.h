#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "../DSP/Engine.h"
#include "../Parameters/Parameters.h"
#include <array>
#include <atomic>
#include <limits>
class SH09Processor final : public juce::AudioProcessor, private juce::VST3ClientExtensions {
public:
 SH09Processor();
 const juce::String getName() const override {return "SH-09";}
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return std::numeric_limits<double>::infinity();}
 int getNumPrograms() override;
 int getCurrentProgram() override {return currentProgram.load();}
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
 void queueKeyboard(int,bool) noexcept;
 void queuePitchBend(float value) noexcept { guiBend.store(juce::jlimit(-1.0f,1.0f,value),std::memory_order_relaxed); guiBendDirty.store(true,std::memory_order_release); }
 juce::AudioProcessorValueTreeState state;
 std::atomic<int> displayedNote{-1};
 std::atomic<float> displayedEnvelope{0},displayedFollower{0};
 std::array<std::atomic<int>,128> keyDisplay{};
private:
 sh09::Controls readControls() const noexcept;
 void handleMidi(const juce::uint8*,int) noexcept;
 void clearKeyDisplay() noexcept;
 void updateKeyDisplay(int,bool) noexcept;
 sh09::Engine engine;
 std::array<std::atomic<float>*,sh09::specs.size()> raw{};
 std::atomic<int> currentProgram{0};
 struct KeyboardEvent {int note;bool down;};
 std::array<KeyboardEvent,256> keyboardQueue{};
 std::atomic<unsigned> queueRead{0},queueWrite{0};
 std::atomic<bool> queueOverflow{false};
 std::atomic<float> guiBend{0};
 std::atomic<bool> guiBendDirty{false};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SH09Processor)
};
