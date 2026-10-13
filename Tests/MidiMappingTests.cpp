#include "../Source/Plugin/Processor.h"
#include <cmath>
#include <functional>
#include <iostream>

namespace {
struct GestureInterleave final : juce::AudioProcessorParameter::Listener {
    std::function<void()> onBegin;
    void parameterValueChanged(int, float) override {}
    void parameterGestureChanged(int, bool starting) override {
        if (starting && onBegin) { auto callback = std::move(onBegin); callback(); }
    }
};
void pump() {
    const auto end = juce::Time::getMillisecondCounterHiRes() + 100;
    while (juce::Time::getMillisecondCounterHiRes() < end) {
        juce::Thread::sleep(5);
        juce::Timer::callPendingTimersSynchronously();
    }
}
}
int main() {
    juce::ScopedJuceInitialiser_GUI init;
    int failures = 0;
    auto expect = [&](bool ok, const char* description) {
        std::cout << (ok ? "PASS " : "FAIL ") << description << '\n';
        if (!ok) ++failures;
    };
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_VST3);
    SH09Processor processor;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
    processor.prepareToPlay(48000, 64);
    juce::AudioBuffer<float> audio(2, 64);
    juce::MidiBuffer midi;
    auto* parameter = processor.state.getParameter("vcoMod");
    auto send = [&](int value) {
        midi.addEvent(juce::MidiMessage::controllerEvent(1, 1, value), 0);
        audio.clear(); processor.processBlock(audio, midi);
    };
    send(0); send(64);
    GestureInterleave interleave;
    interleave.onBegin = [&] { send(100); };
    parameter->addListener(&interleave);
    pump();
    send(110); pump();
    expect(std::abs(parameter->getValue() - 110.f / 127) < 1e-6f,
           "new wheel events during delayed GUI publication do not lose control");
    parameter->removeListener(&interleave);

    // Simulate the host sending an older notified value back at a later block.
    send(120); pump(); send(120);
    parameter->setValueNotifyingHost(64.f / 127);
    send(125); pump();
    expect(std::abs(parameter->getValue() - 125.f / 127) < 1e-6f,
           "VST3 wheel retakes control immediately after delayed host parameter feedback");
    processor.setCurrentProgram(0); send(4); pump();
    expect(std::abs(parameter->getValue() - 4.f / 127) < 1e-6f,
           "VST3 first wheel movement works when MIDI skips the zero value");

    send(120);
    parameter->setValueNotifyingHost(.3f);
    pump();
    expect(std::abs(parameter->getValue() - .3f) < 1e-6f,
           "real host automation supersedes queued MIDI feedback");
    audio.clear(); processor.processBlock(audio, midi);
    send(117); pump();
    expect(std::abs(parameter->getValue() - 117.f / 127) < 1e-6f,
           "VST3 wheel works on its next move after host automation");
    std::cout << "RESULT failures=" << failures << '\n';
    return failures ? 1 : 0;
}
