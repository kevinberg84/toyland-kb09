#include "../Source/Plugin/Processor.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <utility>

namespace {
struct GestureInterleave final : juce::AudioProcessorParameter::Listener {
    std::function<void()> onBegin;
    int invocations = 0;
    void parameterValueChanged(int, float) override {}
    void parameterGestureChanged(int, bool starting) override {
        if (starting && onBegin) {
            // A moved-from std::function need not be empty (libc++ can retain
            // a small callable). Clear it explicitly so this interleave is once.
            auto callback = std::exchange(onBegin, {});
            ++invocations;
            callback();
        }
    }
};
void pump() {
    const auto end = juce::Time::getMillisecondCounterHiRes() + 100;
    while (juce::Time::getMillisecondCounterHiRes() < end) {
        // Exercise the real platform event loop, as a DAW does. On macOS,
        // sleeping does not deliver the run-loop work that schedules timers.
        juce::MessageManager::getInstance()->runDispatchLoopUntil(5);
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
    // Establish the interleave before the later wheel event. A loaded CI
    // runner may need longer than one 100 ms slice to dispatch its first timer.
    for (int retry = 0; interleave.onBegin && retry < 20; ++retry) pump();
    expect(!interleave.onBegin && interleave.invocations == 1,
           "delayed publication interleave was exercised exactly once");
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
    send(117);
    juce::MemoryBlock pendingState;processor.getStateInformation(pendingState);
    auto pending=juce::AudioProcessor::getXmlFromBinary(pendingState.getData(),int(pendingState.getSize()));
    bool audioValueReady=false;
    if(pending)for(auto* child:pending->getChildIterator())if(child->getStringAttribute("id")=="vcoMod")
        audioValueReady=std::abs(float(child->getDoubleAttribute("value"))-117.f/127)<1e-6f;
    // Audio takeover is immediate; GUI publication follows a platform timer.
    // Wait for that observable condition instead of assuming a loaded Mac CI
    // runner dispatches every timer inside one fixed 100 ms slice.
    const auto deadline=juce::Time::getMillisecondCounterHiRes()+2000;
    while(std::abs(parameter->getValue()-117.f/127)>=1e-6f&&juce::Time::getMillisecondCounterHiRes()<deadline)pump();
    if(!audioValueReady||std::abs(parameter->getValue()-117.f/127)>=1e-6f)
        std::cout<<"takeover diagnostics: immediate="<<audioValueReady<<" published="<<parameter->getValue()<<" expected="<<117.f/127<<'\n';
    expect(audioValueReady&&std::abs(parameter->getValue() - 117.f / 127) < 1e-6f,
           "VST3 wheel works immediately after host automation and publishes through the platform timer");
    std::cout << "RESULT failures=" << failures << '\n';
    return failures ? 1 : 0;
}
