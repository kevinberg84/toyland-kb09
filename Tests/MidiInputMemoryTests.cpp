#include "../Source/Plugin/StandaloneMidiInputs.h"
#include <iostream>

int main() {
    sh09::MidiInputSelection memory;
    using Devices=sh09::MidiInputSelection::Devices;
    const juce::MidiDeviceInfo mpk("MPK249","mpk-port-1"),other("Other controller","other-port"),moved("MPK249","mpk-port-2");
    const Devices both{mpk,other},one{mpk},none;
    int checks=0,failures=0;
    auto expect=[&](bool ok,const char* text){++checks;failures+=!ok;std::cout<<(ok?"PASS ":"FAIL ")<<text<<'\n';};
    memory.capture(both,{mpk.identifier});
    const auto saved=memory.save();
    sh09::MidiInputSelection reopened;
    expect(reopened.restore(&saved),"dedicated MIDI selection round-trips independently of audio setup");
    expect(reopened.wanted(both)==juce::StringArray{mpk.identifier},"next session selects the previous controller only");
    reopened.acknowledge(both,{mpk.identifier});
    reopened.observe(none,{});
    expect(reopened.wanted(none).isEmpty(),"unplugged controller is not opened");
    const auto absent=reopened.save();sh09::MidiInputSelection later;later.restore(&absent);
    expect(later.wanted(one)==juce::StringArray{mpk.identifier},"closing while disconnected retains choice for a later session");
    reopened.observe(both,{});
    expect(reopened.wanted(both)==juce::StringArray{mpk.identifier},"hot-plug reconnect restores only the remembered device");
    reopened.acknowledge(both,{});reopened.observe(both,{});
    expect(reopened.wanted(both)==juce::StringArray{mpk.identifier},"failed or busy input remains pending instead of being forgotten");
    reopened.acknowledge(both,{mpk.identifier});reopened.observe(both,{});
    expect(reopened.wanted(both).isEmpty(),"explicitly disabling the controller is respected");
    const auto disabled=reopened.save();sh09::MidiInputSelection noInput;noInput.restore(&disabled);
    expect(noInput.wanted(both).isEmpty(),"explicit no-input choice persists across restart");
    reopened.observe(both,{other.identifier});
    expect(reopened.wanted(both)==juce::StringArray{other.identifier},"choosing a different input changes the default");
    reopened.observe(both,{mpk.identifier,other.identifier});
    expect(reopened.wanted(both).size()==2,"multiple deliberately enabled inputs are retained");
    sh09::MidiInputSelection movedMemory;movedMemory.restore(&saved);
    expect(movedMemory.wanted(Devices{moved,other})==juce::StringArray{moved.identifier},"unique device-name fallback handles a changed port identifier");
    const auto migrated=movedMemory.save();sh09::MidiInputSelection migratedMemory;migratedMemory.restore(&migrated);
    expect(migratedMemory.wanted(Devices{moved,other})==juce::StringArray{moved.identifier},"replacement identifier is saved for future sessions");
    sh09::MidiInputSelection ambiguous;ambiguous.restore(&saved);
    expect(ambiguous.wanted(Devices{moved,{"MPK249","mpk-port-3"}}).isEmpty(),"ambiguous same-name controllers are not selected by guesswork");
    expect(ambiguous.wanted(Devices{mpk,moved})==juce::StringArray{mpk.identifier},"exact identifier wins when names are duplicated");
    juce::XmlElement legacy("DEVICESETUP");auto* input=legacy.createNewChildElement("MIDIINPUT");
    input->setAttribute("name",mpk.name);input->setAttribute("identifier",mpk.identifier);
    sh09::MidiInputSelection migratedLegacy;
    expect(migratedLegacy.restore(&legacy)&&migratedLegacy.wanted(both)==juce::StringArray{mpk.identifier},"existing JUCE MIDI preferences migrate without resetting the choice");
    expect(!migratedLegacy.restore(nullptr),"missing audio state cannot overwrite remembered MIDI intent");
    std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<'\n';return failures?1:0;
}
