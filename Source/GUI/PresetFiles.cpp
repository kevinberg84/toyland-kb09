#include "PresetFiles.h"
#include "../Plugin/Processor.h"
#include "../Presets/Factory.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstring>
#include <string_view>
#if defined(__APPLE__)
#include <locale>
#include <sstream>
#endif

namespace sh09 {
namespace {

juce::Result invalid(const juce::String& detail)
{
    return juce::Result::fail("This is not a complete supported KB-09 preset. " + detail);
}

bool number(const juce::String& text, double& value)
{
    const auto utf8 = text.trim().toStdString();
    if (utf8.empty()) return false;
#if defined(__APPLE__)
    // Xcode 16 libc++ does not provide floating-point from_chars. Preset
    // numbers still use the C locale and must consume the complete string.
    if (utf8.front() == '+') return false;
    std::istringstream input(utf8);
    input.imbue(std::locale::classic());
    input >> std::noskipws >> value;
    return !input.fail() && input.peek() == std::char_traits<char>::eof() && std::isfinite(value);
#else
    const auto parsed = std::from_chars(utf8.data(), utf8.data() + utf8.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == utf8.data() + utf8.size() && std::isfinite(value);
#endif
}

// JUCE getXmlFromBinary tolerates truncation. Check its complete envelope first.
// Only an IComponent import may carry JUCE's recognised private-state trailer.
juce::Result binaryXml(const void* source, std::size_t size, bool allowVstTrailer,
                       std::unique_ptr<juce::XmlElement>& xml)
{
    if (source == nullptr || size < 10 || size > PresetFiles::maximumFileBytes)
        return invalid("The file is empty, incomplete or larger than 4 MB.");
    const auto* bytes = static_cast<const juce::uint8*>(source);
    if (juce::ByteOrder::littleEndianInt(bytes) != 0x21324356u)
        return invalid("Its file signature is not recognised.");
    const auto length = static_cast<std::size_t>(juce::ByteOrder::littleEndianInt(bytes + 4));
    if (length == 0 || length > size - 9 || bytes[length + 8] != 0
        || std::memchr(bytes + 8, 0, length) != nullptr)
        return invalid("The stored state is incomplete.");
    const auto stateSize = length + 9;
    if (stateSize != size)
    {
        constexpr std::string_view marker = "JUCEPrivateData";
        if (!allowVstTrailer || size < stateSize + 16 + marker.size()
            || std::memcmp(bytes + size - marker.size(), marker.data(), marker.size()) != 0)
            return invalid("Unexpected data follows the preset.");
        const auto privateSize = juce::ByteOrder::littleEndianInt64(bytes + size - marker.size() - 8);
        if (privateSize != size - stateSize - 16 - marker.size()
            || juce::ByteOrder::littleEndianInt64(bytes + stateSize) != 0)
            return invalid("The VST3 state trailer is incomplete.");
    }
    const auto text = juce::String::fromUTF8(reinterpret_cast<const char*>(bytes + 8), static_cast<int>(length));
    // A real preset has 35 leaf records. Bound nesting/record count before the
    // recursive XML parser sees corrupt or deliberately deeply nested content.
    if (std::count(bytes + 8, bytes + 8 + length, static_cast<juce::uint8>('<')) > 128)
        return invalid("There are too many records in the stored state.");
    // Presets need no external entities or DTDs.
    if (text.containsIgnoreCase("<!DOCTYPE") || text.containsIgnoreCase("<!ENTITY"))
        return invalid("XML declarations of this kind are not supported.");
    xml = juce::XmlDocument::parse(text);
    return xml != nullptr ? juce::Result::ok() : invalid("The stored XML cannot be read.");
}

juce::Result decodeComponent(const juce::XmlElement& wrapper, juce::MemoryBlock& component)
{
    if (wrapper.getNumAttributes() != 0) return invalid("The VST3 wrapper is not recognised.");
    const juce::XmlElement* state = nullptr;
    bool hasController = false;
    for (auto* child : wrapper.getChildIterator())
    {
        if (child->isTextElement() && child->getText().trim().isEmpty()) continue;
        if (child->hasTagName("IComponent") && state == nullptr) state = child;
        else if (child->hasTagName("IEditController") && !hasController) hasController = true;
        else return invalid("The VST3 wrapper contains duplicate or unknown state.");
    }
    if (state == nullptr || state->getNumAttributes() != 0)
        return invalid("The VST3 component state is missing.");
    for (auto* child : state->getChildIterator())
        if (!child->isTextElement()) return invalid("The VST3 component encoding is not recognised.");

    // MemoryBlock's legacy base64 decoder trusts its decimal allocation prefix.
    // Bound it and require the canonical alphabet/length before calling it.
    const auto encoded = state->getAllSubText().trim();
    const auto dot = encoded.indexOfChar('.');
    if (dot < 1 || dot > 7) return invalid("The VST3 component encoding is incomplete.");
    const auto digits = encoded.substring(0, dot);
    if (!digits.containsOnly("0123456789")) return invalid("The VST3 component size is invalid.");
    const auto decodedSize = static_cast<std::size_t>(digits.getLargeIntValue());
    const auto data = encoded.substring(dot + 1);
    if (decodedSize == 0 || decodedSize > PresetFiles::maximumFileBytes
        || static_cast<std::size_t>(data.length()) != (decodedSize * 8 + 5) / 6
        || !data.containsOnly(".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+"))
        return invalid("The VST3 component encoding is incomplete or too large.");
    if (!component.fromBase64Encoding(encoded) || component.toBase64Encoding() != encoded)
        return invalid("The VST3 component encoding is corrupt.");
    return juce::Result::ok();
}

juce::Result validateState(const juce::XmlElement& xml)
{
    if (!xml.hasTagName("SH09State")) return invalid("It belongs to a different instrument.");
    if (xml.getNumAttributes() != 2) return invalid("The state header is incomplete or not supported.");
    if (xml.getStringAttribute("schema") != "1") return invalid("Its state version is not supported.");
    double program = 0;
    if (!number(xml.getStringAttribute("program"), program) || program != std::floor(program)
        || program < 0 || program >= static_cast<double>(factoryPresets().size()))
        return invalid("The factory-program reference is invalid.");

    std::array<bool, specs.size()> found{};
    for (auto* child : xml.getChildIterator())
    {
        if (child->isTextElement() && child->getText().trim().isEmpty()) continue;
        if (!child->hasTagName("PARAM") || child->getNumAttributes() != 2 || child->getFirstChildElement() != nullptr)
            return invalid("A parameter record is malformed.");
        const auto id = child->getStringAttribute("id");
        std::size_t index = 0;
        for (; index < specs.size(); ++index) if (id == specs[index].id) break;
        if (index == specs.size()) return invalid("It contains an unknown parameter: " + id);
        if (found[index]) return invalid("It contains the same parameter twice: " + id);
        found[index] = true;
        double value = 0;
        const auto& spec = specs[index];
        if (!number(child->getStringAttribute("value"), value) || value < spec.min || value > spec.max
            || (spec.choices[0] != 0 && value != std::floor(value)))
            return invalid("A parameter value is invalid: " + id);
    }
    for (std::size_t i = 0; i < found.size(); ++i)
        if (!found[i]) return invalid("A parameter is missing: " + juce::String(specs[i].id));
    return juce::Result::ok();
}

bool supportedFile(const juce::File& file)
{
    return file.hasFileExtension("kb09;sh09state");
}

std::unique_ptr<juce::PropertiesFile> presetLocations()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Toyland KB-09 Preset Locations";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    return std::make_unique<juce::PropertiesFile>(options);
}

} // namespace

juce::File PresetFiles::factoryDirectory()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                      .getChildFile("Toyland KB-09 Presets");
    if (folder.createDirectory().failed())
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    SH09Processor factory;
    for (int i = 0; i < factory.getNumPrograms(); ++i) {
        auto file = folder.getChildFile(juce::File::createLegalFileName(factory.getProgramName(i).replace(" / ", " - ")) + ".kb09");
        if (!file.exists()) { factory.setCurrentProgram(i); saveToFile(factory, file); }
    }
    return folder;
}

juce::File PresetFiles::recentSaveDirectory()
{
    const auto path = presetLocations()->getValue("lastSavedFolder");
    if (path.isNotEmpty()) { juce::File folder(path); if (folder.isDirectory()) return folder; }
    return factoryDirectory();
}

juce::Result PresetFiles::decodePresetData(const void* data, std::size_t size, juce::MemoryBlock& processorState)
{
    std::unique_ptr<juce::XmlElement> xml;
    if (auto result = binaryXml(data, size, false, xml); result.failed()) return result;
    if (xml->hasTagName("VST3PluginState"))
    {
        juce::MemoryBlock component;
        if (auto result = decodeComponent(*xml, component); result.failed()) return result;
        if (auto result = binaryXml(component.getData(), component.getSize(), true, xml); result.failed()) return result;
    }
    if (auto result = validateState(*xml); result.failed()) return result;
    juce::MemoryBlock validated;
    juce::AudioProcessor::copyXmlToBinary(*xml, validated);
    processorState.swapWith(validated);
    return juce::Result::ok();
}

juce::Result PresetFiles::saveToFile(SH09Processor& processor, const juce::File& file)
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (!file.hasFileExtension("kb09")) return juce::Result::fail("Choose a file ending in .kb09.");
    if (file.isDirectory() || !file.getParentDirectory().isDirectory())
        return juce::Result::fail("Choose an existing folder and a preset filename.");
    juce::MemoryBlock state, validated;
    processor.getStateInformation(state);
    if (auto result = decodePresetData(state.getData(), state.getSize(), validated); result.failed()) return result;

    // Do not replace the old preset until the entire new file has been written,
    // flushed and checked. JUCE File::replaceWithData does not check write errors.
    juce::TemporaryFile temporary(file, juce::TemporaryFile::useHiddenFile);
    {
        juce::FileOutputStream stream(temporary.getFile());
        if (stream.failedToOpen() || !stream.write(validated.getData(), validated.getSize()))
            return juce::Result::fail("The preset could not be written. Check the folder's permissions and free space.");
        stream.flush();
        if (stream.getStatus().failed() || stream.getPosition() != static_cast<juce::int64>(validated.getSize()))
            return juce::Result::fail("Writing the preset failed. The existing preset was not replaced.");
    }
    if (temporary.getFile().getSize() != static_cast<juce::int64>(validated.getSize())
        || !temporary.overwriteTargetFileWithTemporary())
        return juce::Result::fail("The preset could not be replaced. Check whether it is read-only or in use.");
    return juce::Result::ok();
}

juce::Result PresetFiles::loadFromFile(SH09Processor& processor, const juce::File& file)
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (!supportedFile(file)) return juce::Result::fail("Choose a .kb09 preset.");
    juce::FileInputStream stream(file);
    if (stream.failedToOpen()) return juce::Result::fail("The preset could not be opened. It may have been moved or deleted.");
    const auto size = stream.getTotalLength();
    if (size <= 0 || size > static_cast<juce::int64>(maximumFileBytes))
        return invalid("The file is empty or larger than 4 MB.");
    juce::MemoryBlock bytes(static_cast<std::size_t>(size)), validated;
    if (stream.read(bytes.getData(), static_cast<int>(size)) != size || stream.getStatus().failed())
        return invalid("The complete file could not be read.");
    if (auto result = decodePresetData(bytes.getData(), bytes.getSize(), validated); result.failed()) return result;
    processor.setStateInformation(validated.getData(), static_cast<int>(validated.getSize()));
    processor.updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}
                                   .withProgramChanged(true).withNonParameterStateChanged(true));
    return juce::Result::ok();
}

struct PresetFiles::Impl : std::enable_shared_from_this<Impl>
{
    Impl(SH09Processor& p, juce::Component& c) : processor(p), owner(&c) {}
    SH09Processor& processor;
    juce::Component::SafePointer<juce::Component> owner;
    juce::File directory;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::ScopedMessageBox message;
    Completion completion;
    std::function<void()> cancelled;
    bool active = true, busy = false;

    bool alive() const { return active && owner != nullptr; }

    void completed(const juce::Result& result, const juce::File& file, bool loading)
    {
        busy = false;
        if (!alive()) return;
        if (result.wasOk() && !loading) {
            auto settings = presetLocations();
            settings->setValue("lastSavedFolder", file.getParentDirectory().getFullPathName());
            settings->saveIfNeeded();
        }
        else message = juce::AlertWindow::showScopedAsync(
            juce::MessageBoxOptions{}.withIconType(juce::MessageBoxIconType::WarningIcon)
                .withTitle(loading ? "Preset not loaded" : "Preset not saved")
                .withMessage(result.getErrorMessage() + (loading ? "\n\nYour current patch has not changed." : ""))
                .withButton("OK").withAssociatedComponent(owner.getComponent()),
            [](int) {});
        // Copy before calling client code, which may close the editor/helper.
        const auto callback = completion;
        if (callback) callback(result, file, loading);
    }

    void selected(juce::File file, bool loading)
    {
        chooser.reset(); // Deferred from FileChooser's callback, so it is no longer on the stack.
        if (!alive()) return;
        if (file == juce::File{}) { cancel(); return; }
        if (loading) { completed(loadFromFile(processor, file), file, true); return; }
        const auto selectedFile = file;
        if (file.getFileExtension().isEmpty()) file = file.withFileExtension(".kb09");
        if (!file.hasFileExtension("kb09"))
        {
            completed(juce::Result::fail("Use .kb09 as the filename extension."), file, false);
            return;
        }
        // Native save dialogs confirm replacement. If adding a missing extension
        // changes the selected path, confirm replacement of that actual target too.
        if (file != selectedFile && file.existsAsFile())
        {
            const auto weak = weak_from_this();
            message = juce::AlertWindow::showScopedAsync(
                juce::MessageBoxOptions{}.withIconType(juce::MessageBoxIconType::QuestionIcon)
                    .withTitle("Replace preset?").withMessage(file.getFileName() + " already exists.")
                    .withButton("Replace").withButton("Cancel").withAssociatedComponent(owner.getComponent()),
                [weak, file](int answer)
                {
                    juce::MessageManager::callAsync([weak, file, answer]
                    {
                        if (auto self = weak.lock(); self && self->alive())
                        {
                            self->message.close();
                            if (answer == 1) self->completed(saveToFile(self->processor, file), file, false);
                            else self->cancel();
                        }
                    });
                });
            return;
        }
        completed(saveToFile(processor, file), file, false);
    }

    void cancel() { busy = false; const auto callback = cancelled; if (callback) callback(); }

    void open(bool loading, Completion callback, std::function<void()> cancelCallback)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        if (!alive() || busy) return;
        busy = true;
        completion = std::move(callback);
        cancelled = std::move(cancelCallback);
        message.close();
        directory = loading ? recentSaveDirectory() : factoryDirectory();
        auto initial = directory;
        if (!loading)
        {
            auto name = juce::File::createLegalFileName(processor.getProgramName(processor.getCurrentProgram()));
            if (name.isEmpty()) name = "My KB-09 Patch";
            initial = directory.getChildFile(name + ".kb09");
        }
        chooser = std::make_unique<juce::FileChooser>(loading ? "Load a KB-09 preset" : "Save your KB-09 preset",
                    initial, "*.kb09", true, false, owner.getComponent());
        const auto weak = weak_from_this();
        const auto flags = juce::FileBrowserComponent::canSelectFiles
            | (loading ? juce::FileBrowserComponent::openMode
                       : juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting);
        chooser->launchAsync(flags, [weak, loading](const juce::FileChooser& dialog)
        {
            const auto result = dialog.getResult();
            juce::MessageManager::callAsync([weak, result, loading]
            {
                if (auto self = weak.lock(); self && self->alive()) self->selected(result, loading);
            });
        });
    }
};

PresetFiles::PresetFiles(SH09Processor& p, juce::Component& c) : impl(std::make_shared<Impl>(p, c)) {}
PresetFiles::~PresetFiles()
{
    impl->active = false;
    impl->completion = {};
    impl->cancelled = {};
    impl->chooser.reset();
    impl->message.close();
}
void PresetFiles::savePreset() { impl->open(false, onCompleted, onCancelled); }
void PresetFiles::loadPreset() { impl->open(true, onCompleted, onCancelled); }
bool PresetFiles::isDialogOpen() const noexcept { return impl->busy; }

} // namespace sh09
