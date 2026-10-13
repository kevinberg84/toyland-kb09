#include "../Source/GUI/PresetFiles.h"
#include "../Source/Plugin/Processor.h"
#include <array>
#include <cmath>
#include <iostream>

namespace {
juce::MemoryBlock binary(const juce::XmlElement& xml)
{
    juce::MemoryBlock data;
    juce::AudioProcessor::copyXmlToBinary(xml, data);
    return data;
}

juce::MemoryBlock wrapped(const juce::MemoryBlock& raw, bool privateTrailer)
{
    auto component = raw;
    if (privateTrailer)
    {
        juce::MemoryOutputStream extra;
        extra.writeInt64(0);
        juce::ValueTree metadata("JUCEPrivateData");
        metadata.setProperty("Bypass", false, nullptr);
        metadata.writeToStream(extra);
        const auto size = static_cast<juce::int64>(extra.getDataSize()) - 8;
        extra.writeInt64(size);
        extra << "JUCEPrivateData";
        component.append(extra.getData(), extra.getDataSize());
    }
    juce::XmlElement xml("VST3PluginState");
    xml.createNewChildElement("IComponent")->addTextElement(component.toBase64Encoding());
    return binary(xml);
}

auto values(SH09Processor& processor)
{
    std::array<float, sh09::specs.size()> result{};
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = processor.state.getRawParameterValue(sh09::specs[i].id)->load();
    return result;
}
} // namespace

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    SH09Processor processor;
    int passed = 0, failed = 0;
    const auto expect = [&](bool success, const juce::String& description)
    {
        std::cout << (success ? "PASS " : "FAIL ") << description << '\n';
        success ? ++passed : ++failed;
    };

    // Files are owned individually by TemporaryFile; no recursive folder deletion.
    juce::TemporaryFile saved(".kb09"), legacy(".sh09state"), invalidFile(".kb09");
    processor.setCurrentProgram(9);
    for (std::size_t i = 0; i < sh09::specs.size(); ++i)
    {
        const auto& spec = sh09::specs[i];
        const auto fraction = static_cast<float>((i * 7 + 3) % 35) / 34.0f;
        auto value = spec.min + fraction * (spec.max - spec.min);
        if (spec.choices[0] != 0) value = std::round(value);
        auto* parameter = processor.state.getParameter(spec.id);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    }
    const auto intended = values(processor);
    expect(sh09::PresetFiles::saveToFile(processor, saved.getFile()).wasOk(), "save a custom complete .kb09 preset");
    processor.setCurrentProgram(0);
    processor.setPowerEnabled(false);
    expect(sh09::PresetFiles::loadFromFile(processor, saved.getFile()).wasOk()
           && values(processor) == intended && processor.getCurrentProgram() == 9,
           "load restores all 35 custom parameter values and program reference exactly");
    expect(!processor.isPowerEnabled(), "loading a preset does not change transient Power");
    expect(sh09::PresetFiles::saveToFile(processor, legacy.getFile()).failed(), "new exports require the KB-09 extension");
    expect(saved.getFile().copyFileTo(legacy.getFile()), "prepare an earlier-extension compatibility fixture");
    processor.setCurrentProgram(0);
    expect(sh09::PresetFiles::loadFromFile(processor, legacy.getFile()).wasOk() && values(processor) == intended,
           "load processor state with .sh09state extension");

    juce::MemoryBlock valid;
    processor.getStateInformation(valid);
    auto validXml = juce::AudioProcessor::getXmlFromBinary(valid.getData(), static_cast<int>(valid.getSize()));
    const auto rejectBytes = [&](const juce::MemoryBlock& bytes, const juce::String& description)
    {
        const auto before = values(processor);
        const auto beforeProgram = processor.getCurrentProgram();
        juce::MemoryBlock untouched("sentinel", 8);
        const auto original = untouched;
        const bool decodeRejected = sh09::PresetFiles::decodePresetData(bytes.getData(), bytes.getSize(), untouched).failed();
        const bool wrote = invalidFile.getFile().replaceWithData(bytes.getData(), bytes.getSize());
        const bool loadRejected = sh09::PresetFiles::loadFromFile(processor, invalidFile.getFile()).failed();
        expect(decodeRejected && untouched == original && wrote && loadRejected
               && values(processor) == before && processor.getCurrentProgram() == beforeProgram,
               description + "; patch/output remain unchanged");
    };
    const auto rejectXml = [&](const std::function<void(juce::XmlElement&)>& edit, const juce::String& description)
    {
        juce::XmlElement xml(*validXml);
        edit(xml);
        rejectBytes(binary(xml), description);
    };

    auto corrupt = valid;
    static_cast<char*>(corrupt.getData())[0] = 0;
    rejectBytes(corrupt, "reject wrong binary signature");
    corrupt = valid;
    corrupt.setSize(corrupt.getSize() - 1);
    rejectBytes(corrupt, "reject truncated binary envelope");
    corrupt = valid;
    const char junk = 'x';
    corrupt.append(&junk, 1);
    rejectBytes(corrupt, "reject unexpected trailing data");
    rejectXml([](auto& xml) { xml.setTagName("OtherSynth"); }, "reject another instrument's state");
    rejectXml([](auto& xml) { xml.removeAttribute("schema"); }, "reject missing schema");
    rejectXml([](auto& xml) { xml.setAttribute("schema", 2); }, "reject future schema");
    rejectXml([](auto& xml) { xml.removeAttribute("program"); }, "reject missing program reference");
    rejectXml([&](auto& xml) { xml.setAttribute("program", processor.getNumPrograms()); }, "reject out-of-range program reference");
    rejectXml([](auto& xml) { xml.setAttribute("program", "2.5"); }, "reject fractional program reference");
    rejectXml([](auto& xml) { xml.setAttribute("other", 1); }, "reject unknown state-header field");
    rejectXml([](auto& xml) { xml.removeChildElement(xml.getFirstChildElement(), true); }, "reject incomplete parameter set");
    rejectXml([](auto& xml) { xml.addChildElement(new juce::XmlElement(*xml.getFirstChildElement())); }, "reject duplicate parameter");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setAttribute("id", "unknown"); }, "reject unknown parameter");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setTagName("OTHER"); }, "reject wrong parameter node type");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->removeAttribute("value"); }, "reject missing value");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setAttribute("value", "NaN"); }, "reject NaN value");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setAttribute("value", "inf"); }, "reject infinite value");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setAttribute("value", "junk"); }, "reject nonnumeric value");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setAttribute("value", "0.5junk"); }, "reject numeric value with trailing junk");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->setAttribute("value", "99999"); }, "reject out-of-range value");
    rejectXml([](auto& xml)
    {
        for (auto* child : xml.getChildIterator())
            if (child->getStringAttribute("id") == "range") child->setAttribute("value", "1.5");
    }, "reject fractional switch position");
    rejectXml([](auto& xml) { xml.getFirstChildElement()->createNewChildElement("nested"); }, "reject nested parameter data");
    rejectXml([](auto& xml)
    {
        auto* deepest = &xml;
        for (int i = 0; i < 160; ++i) deepest = deepest->createNewChildElement("nested");
    }, "reject excessive nesting before XML parsing");
    juce::MemoryBlock oversized(sh09::PresetFiles::maximumFileBytes + 1, true);
    rejectBytes(oversized, "reject file larger than 4 MB");

    for (const auto trailer : { false, true })
    {
        const auto wrapper = wrapped(valid, trailer);
        expect(legacy.getFile().replaceWithData(wrapper.getData(), wrapper.getSize()), "write hosted-state fixture");
        processor.setCurrentProgram(0);
        const auto loaded = sh09::PresetFiles::loadFromFile(processor, legacy.getFile());
        if (loaded.failed()) std::cout << loaded.getErrorMessage() << '\n';
        expect(loaded.wasOk() && values(processor) == intended && processor.getCurrentProgram() == 9,
               trailer ? "import JUCE VST3 wrapper with private trailer" : "import JUCE VST3 wrapper without private trailer");
    }
    const auto validWrapper = wrapped(valid, true);
    auto wrapperXml = juce::AudioProcessor::getXmlFromBinary(validWrapper.getData(), static_cast<int>(validWrapper.getSize()));
    wrapperXml->getFirstChildElement()->deleteAllChildElements();
    wrapperXml->getFirstChildElement()->addTextElement("9999999999999.A");
    rejectBytes(binary(*wrapperXml), "reject malicious hosted-state allocation prefix before decoding");
    wrapperXml->getFirstChildElement()->deleteAllChildElements();
    wrapperXml->getFirstChildElement()->addTextElement("10.A");
    rejectBytes(binary(*wrapperXml), "reject incomplete hosted-state encoding");

    bool bankPassed = true;
    for (int program = 0; program < processor.getNumPrograms(); ++program)
    {
        processor.setCurrentProgram(program);
        const auto reference = values(processor);
        bankPassed &= sh09::PresetFiles::saveToFile(processor, saved.getFile()).wasOk();
        processor.setCurrentProgram((program + 1) % processor.getNumPrograms());
        bankPassed &= sh09::PresetFiles::loadFromFile(processor, saved.getFile()).wasOk();
        bankPassed &= values(processor) == reference && processor.getCurrentProgram() == program;
    }
    expect(bankPassed && processor.getNumPrograms() == 24, "all 24 factory programs repeatedly save/overwrite/load exactly");

    // Optional path to the actual bank exported by Tests/Host.cpp. This tests real
    // JUCE-host output, in addition to synthetic edge cases above.
    if (argc > 1)
    {
        const juce::File folder = juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1]));
        const auto files = folder.findChildFiles(juce::File::findFiles, false, "*.sh09state");
        expect(files.size() == 24, "actual exported bank contains 24 .sh09state files");
        for (const auto& file : files)
        {
            const auto result = sh09::PresetFiles::loadFromFile(processor, file);
            expect(result.wasOk(), "load actual exported bank: " + file.getFileName());
            if (result.failed()) std::cout << result.getErrorMessage() << '\n';
        }
    }

    // Validate the optional local listening presets through the same loader as LOAD.
    if (argc > 2)
    {
        const juce::File folder(juce::String::fromUTF8(argv[2]));
        const auto files = folder.findChildFiles(juce::File::findFiles, false, "*.kb09");
        expect(files.size() == 7, "local DSP listening bank contains seven presets");
        for (const auto& file : files)
        {
            const auto result = sh09::PresetFiles::loadFromFile(processor, file);
            expect(result.wasOk(), "load local listening preset: " + file.getFileName());
            if (result.failed()) std::cout << result.getErrorMessage() << '\n';
            const auto expected = values(processor);
            const bool savedOK = sh09::PresetFiles::saveToFile(processor, saved.getFile()).wasOk();
            processor.setCurrentProgram(1);
            const bool restored = sh09::PresetFiles::loadFromFile(processor, saved.getFile()).wasOk();
            expect(savedOK && restored && values(processor) == expected,
                   "local listening preset save/reload: " + file.getFileName());
        }
    }

    const auto beforeFailure = values(processor);
    juce::MemoryBlock savedBeforeFailure;
    const bool readBefore = saved.getFile().loadFileAsData(savedBeforeFailure);
    expect(sh09::PresetFiles::loadFromFile(processor, saved.getFile().withFileExtension("missing.kb09")).failed()
           && values(processor) == beforeFailure, "missing file returns an error without changing the patch");
    expect(sh09::PresetFiles::saveToFile(processor, saved.getFile().withFileExtension("txt")).failed(), "unsupported save extension is rejected");
    expect(sh09::PresetFiles::saveToFile(processor, saved.getFile().getChildFile("unavailable.kb09")).failed(), "invalid destination returns a save error");
    juce::MemoryBlock afterFailure;
    expect(readBefore && saved.getFile().loadFileAsData(afterFailure) && afterFailure == savedBeforeFailure,
           "failed save attempts leave the previously saved preset intact");

    {
        juce::Component owner;
        sh09::PresetFiles helper(processor, owner);
        expect(!helper.isDialogOpen(), "helper starts without a modal dialog");
    }
    std::cout << "RESULT passed=" << passed << " failures=" << failed << '\n';
    return failed == 0 ? 0 : 1;
}
