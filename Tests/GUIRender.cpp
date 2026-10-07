#include "../Source/Plugin/Processor.h"
#include <BinaryData.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

// Offline fixture: the real processor/editor run on the JUCE message thread.
// No desktop peer, audio device, DAW, or replacement drawing code is involved.
namespace {
constexpr double sampleRate = 48000.0;
constexpr int blockSize = 256;
constexpr std::array<const char*, 35> legacyParameterIDs {{
    "tune", "range", "wave", "pulseWidth", "pwmMode", "vcoLevel", "subLevel", "extLevel", "subMode",
    "lfoRate", "lfoDelay", "lfoWave", "vcoMod", "cutoff", "resonance", "vcfMod", "vcfEnv", "vcfEnvMode",
    "vcaMode", "attack", "decay", "sustain", "release", "envMode", "portamento", "bendVco", "bendVcf",
    "volume", "age", "modern", "velocity", "aftertouch", "lfoSync", "extGain", "branding"
}};

struct Checks {
    int failures = 0;
    int total = 0;
    juce::String log;
    void expect(bool passed, const juce::String& description) {
        ++total;
        const auto line = juce::String(passed ? "PASS " : "FAIL ") + description;
        std::cout << line << '\n';
        log += line + "\n";
        if (!passed) ++failures;
    }
};

// Check the wear itself, independently of key tint, position, final opacity, or
// small polish highlights. Those can make final screenshots differ even when a
// copied stain is used. This is a bounded duplicate detector, not proof against
// every arbitrary affine/image transformation or a claim of photographic realism.
struct WearAlphaShape {
    static constexpr int side = 64;
    std::array<float, side * side> alpha {};
    bool valid = false;
};

WearAlphaShape wearAlphaShape(const juce::Image& image) {
    WearAlphaShape result;
    if (!image.isValid()) return result;
    int maximum = 0;
    for (int y = 0; y < image.getHeight(); ++y)
        for (int x = 0; x < image.getWidth(); ++x)
            maximum = std::max(maximum, int(image.getPixelAt(x, y).getAlpha()));
    if (maximum < 32) return result;

    int left = image.getWidth(), top = image.getHeight(), right = -1, bottom = -1;
    for (int y = 0; y < image.getHeight(); ++y)
        for (int x = 0; x < image.getWidth(); ++x)
            if (float(image.getPixelAt(x, y).getAlpha()) >= float(maximum) * .15f) {
                left = std::min(left, x); right = std::max(right, x);
                top = std::min(top, y); bottom = std::max(bottom, y);
            }
    if (right < left || bottom < top) return result;
    const auto normalised = image.getClippedImage({left, top, right - left + 1, bottom - top + 1})
                                 .rescaled(WearAlphaShape::side, WearAlphaShape::side,
                                           juce::Graphics::highResamplingQuality);
    for (int y = 0; y < WearAlphaShape::side; ++y)
        for (int x = 0; x < WearAlphaShape::side; ++x)
            result.alpha[size_t(y * WearAlphaShape::side + x)] =
                float(normalised.getPixelAt(x, y).getAlpha()) / float(maximum);
    result.valid = true;
    return result;
}

// Translation and independent x/y scaling disappear through the content bounds.
// Test all eight quarter-turn/reflection combinations; RGB colour is never read.
double wearShapeDifference(const WearAlphaShape& a, const WearAlphaShape& b) {
    if (!a.valid || !b.valid) return 0.0;
    constexpr int side = WearAlphaShape::side;
    double best = 1.0;
    for (int transform = 0; transform < 8; ++transform) {
        double difference = 0.0, mass = 0.0;
        int supportDifference = 0, supportUnion = 0;
        for (int y = 0; y < side; ++y) for (int x = 0; x < side; ++x) {
            int tx = transform >= 4 ? side - 1 - x : x, ty = y;
            for (int rotation = 0; rotation < transform % 4; ++rotation) {
                const int oldX = tx;
                tx = side - 1 - ty; ty = oldX;
            }
            const float av = a.alpha[size_t(y * side + x)];
            const float bv = b.alpha[size_t(ty * side + tx)];
            difference += std::abs(av - bv);
            mass += std::max(av, bv);
            const bool as = av >= .2f, bs = bv >= .2f;
            supportDifference += as != bs ? 1 : 0;
            supportUnion += as || bs ? 1 : 0;
        }
        const auto opacityDifference = mass > 0.0 ? difference / mass : 0.0;
        const auto outlineDifference = supportUnion > 0 ? double(supportDifference) / double(supportUnion) : 0.0;
        best = std::min(best, std::min(opacityDifference, outlineDifference));
    }
    return best;
}

void checkWearAtlas(Checks& checks) {
    const auto atlas = juce::ImageFileFormat::loadFrom(BinaryData::wearatlas_png, BinaryData::wearatlas_pngSize);
    checks.expect(atlas.isValid() && atlas.hasAlphaChannel() && atlas.getWidth() == 1448 && atlas.getHeight() == 1086,
                  "wear atlas is the registered 1448x1086 RGBA source");
    if (!atlas.isValid() || atlas.getWidth() != 1448 || atlas.getHeight() != 1086) return;

    std::array<WearAlphaShape, 37> shapes;
    for (int tile = 0; tile < int(shapes.size()); ++tile) {
        // Measured source gutters: generated marks start 36px below the nominal
        // grid. The two-pixel x inset excludes isolated neighbouring edge pixels.
        const auto crop = atlas.getClippedImage({(tile % 8) * 181 + 2, (tile / 8) * 181 + 36, 177, 181});
        int visible = 0, edgePixels = 0;
        for (int y = 0; y < crop.getHeight(); ++y)
            for (int x = 0; x < crop.getWidth(); ++x)
                if (crop.getPixelAt(x, y).getAlpha() > 25) {
                    ++visible;
                    if (x == 0 || y == 0 || x == crop.getWidth() - 1 || y == crop.getHeight() - 1) ++edgePixels;
                }
        shapes[size_t(tile)] = wearAlphaShape(crop);
        checks.expect(shapes[size_t(tile)].valid && visible >= 32 && visible < crop.getWidth() * crop.getHeight() * 9 / 10,
                      "wear tile " + juce::String(tile) + " contains a visible mark on transparent background");
        checks.expect(edgePixels <= 4,
                      "wear tile " + juce::String(tile) + " is isolated from neighbouring marks (edge pixels "
                      + juce::String(edgePixels) + ")");
    }

    const auto compareGroup = [&shapes, &checks](int first, int end, const juce::String& name) {
        double closest = 1.0;
        int closestA = -1, closestB = -1;
        for (int a = first; a < end; ++a) for (int b = first; b < a; ++b) {
            const auto distance = wearShapeDifference(shapes[size_t(a)], shapes[size_t(b)]);
            if (distance < closest) { closest = distance; closestA = a; closestB = b; }
        }
        // Four percent tolerates antialiasing/resampling differences in a copy;
        // colour, placement, and a different overall opacity cannot rescue it.
        checks.expect(closest >= .04, name + " have distinct normalised alpha shapes, including quarter turns/mirrors"
                      + " (closest tiles " + juce::String(closestA) + "/" + juce::String(closestB)
                      + ", difference " + juce::String(closest, 4) + ")");
    };
    compareGroup(0, 32, "all 32 individual key stains");
    compareGroup(32, 34, "the two independently sourced key cracks");
    compareGroup(34, 37, "the three individual rotary corrosion patches");
    compareGroup(0, 37, "all 37 assigned wear marks across material groups");

    // Negative control: explicitly verify rejection of a mirrored/quarter-turned
    // copy. RGB, position and size are excluded by the alpha normalisation above.
    auto clone = shapes[0];
    for (int y = 0; y < WearAlphaShape::side; ++y)
        for (int x = 0; x < WearAlphaShape::side; ++x)
            clone.alpha[size_t(y * WearAlphaShape::side + x)] =
                shapes[0].alpha[size_t((WearAlphaShape::side - 1 - x) * WearAlphaShape::side
                                      + (WearAlphaShape::side - 1 - y))];
    checks.expect(shapes[0].valid && wearShapeDifference(shapes[0], clone) < .00001,
                  "wear duplicate detector rejects a mirrored/quarter-turned copy (negative control)");
}

void setParameter(SH09Processor& processor, const char* id, float value) {
    auto* parameter = processor.state.getParameter(id);
    if (parameter == nullptr) throw std::runtime_error((juce::String("Missing parameter: ") + id).toStdString());
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    parameter->endChangeGesture();
}

void settleEditor() {
    // Allows APVTS attachments, editor timers, and deferred initial sizing to run.
    juce::MessageManager::getInstance()->runDispatchLoopUntil(120);
    juce::Timer::callPendingTimersSynchronously();
}

juce::Component* findComponent(juce::Component& parent, const juce::String& id) {
    if (parent.getComponentID() == id) return &parent;
    for (int i = 0; i < parent.getNumChildComponents(); ++i)
        if (auto* match = findComponent(*parent.getChildComponent(i), id)) return match;
    return nullptr;
}

juce::String componentText(const juce::Label& label) { return label.getText(); }
juce::String componentText(const juce::TextButton& button) { return button.getButtonText(); }

template <typename ComponentType>
ComponentType* findText(juce::Component& parent, const juce::String& text) {
    if (auto* match = dynamic_cast<ComponentType*>(&parent); match != nullptr && componentText(*match) == text)
        return match;
    for (int i = 0; i < parent.getNumChildComponents(); ++i)
        if (auto* match = findText<ComponentType>(*parent.getChildComponent(i), text)) return match;
    return nullptr;
}

bool fitsVisibleAncestors(juce::Component* component, juce::Component& editor) {
    if (component == nullptr || component->getLocalBounds().isEmpty()) return false;
    const auto bounds = editor.getLocalArea(component, component->getLocalBounds().toFloat());
    if (bounds.isEmpty()) return false;
    // Check in one coordinate space, after every enclosing transform. A component
    // can be marked visible while entirely outside its parent or editor clip.
    for (auto* ancestor = component; ancestor != nullptr; ancestor = ancestor->getParentComponent()) {
        if (!ancestor->isVisible()) return false;
        const auto clip = editor.getLocalArea(ancestor, ancestor->getLocalBounds().toFloat());
        if (!clip.expanded(1.0f).contains(bounds)) return false; // one output-pixel rounding tolerance
        if (ancestor == &editor) return true;
    }
    return false;
}

void checkFooterGeometry(juce::Component& editor, Checks& checks, const juce::String& context) {
    juce::StringArray clippedControls, clippedLabels;
    for (const auto* id : {"modern", "age", "velocity", "aftertouch", "lfoSync", "extGain"})
        if (!fitsVisibleAncestors(findComponent(editor, id), editor)) clippedControls.add(id);
    if (!fitsVisibleAncestors(findText<juce::TextButton>(editor, "EXT AUDIO"), editor))
        clippedControls.add("EXT AUDIO");
    for (const auto* text : {"CONDITION / AGE", "VELOCITY", "AFTERTOUCH", "INPUT TRIM"})
        if (!fitsVisibleAncestors(findText<juce::Label>(editor, text), editor)) clippedLabels.add(text);
    checks.expect(clippedControls.isEmpty(), context + " footer controls fit transformed ancestor/editor bounds"
                  + (clippedControls.isEmpty() ? juce::String{} : ": " + clippedControls.joinIntoString(", ")));
    checks.expect(clippedLabels.isEmpty(), context + " footer labels fit transformed ancestor/editor bounds"
                  + (clippedLabels.isEmpty() ? juce::String{} : ": " + clippedLabels.joinIntoString(", ")));
}

int changedPixels(const juce::Image& before, const juce::Image& after, int threshold = 0) {
    if (!before.isValid() || !after.isValid() || before.getBounds() != after.getBounds()) return -1;
    int changed = 0;
    const juce::Image::BitmapData a(before, juce::Image::BitmapData::readOnly);
    const juce::Image::BitmapData b(after, juce::Image::BitmapData::readOnly);
    for (int y = 0; y < before.getHeight(); ++y)
        for (int x = 0; x < before.getWidth(); ++x) {
            const auto p = a.getPixelColour(x, y), q = b.getPixelColour(x, y);
            const auto delta = std::max({std::abs(int(p.getRed()) - int(q.getRed())),
                                        std::abs(int(p.getGreen()) - int(q.getGreen())),
                                        std::abs(int(p.getBlue()) - int(q.getBlue())),
                                        std::abs(int(p.getAlpha()) - int(q.getAlpha()))});
            if (delta > threshold) ++changed;
        }
    return changed;
}

void processMidi(SH09Processor& processor, const juce::MidiMessage& message) {
    juce::AudioBuffer<float> buffer(2, blockSize);
    buffer.clear();
    juce::MidiBuffer midi;
    midi.addEvent(message, 0);
    processor.processBlock(buffer, midi);
    settleEditor();
}

void checkKeyboardFeedback(SH09Processor& processor, juce::Component& editor, Checks& checks) {
    auto* keyboard = findComponent(editor, "keyboard");
    checks.expect(keyboard != nullptr, "keyboard is available for MIDI visual-feedback checks");
    if (keyboard == nullptr) return;
    processMidi(processor, juce::MidiMessage::allNotesOff(1));
    for (const auto note : {60, 61}) {
        const auto name = juce::String(note == 60 ? "white C60" : "black C#61");
        // Capture only the keyboard. LFO lamps, envelopes and meters keep moving;
        // their changes must not count as evidence that a key became highlighted.
        const auto released = keyboard->createComponentSnapshot(keyboard->getLocalBounds(), true, 1.0f);
        processMidi(processor, juce::MidiMessage::noteOn(1, note, juce::uint8(100)));
        const auto held = keyboard->createComponentSnapshot(keyboard->getLocalBounds(), true, 1.0f);
        checks.expect(processor.keyDisplay[size_t(note)].load() != 0
                      && processor.displayedNote.load() == note && changedPixels(released, held, 12) >= 100,
                      name + " MIDI note-on visibly changes keyboard pixels");
        processMidi(processor, juce::MidiMessage::noteOff(1, note));
        const auto restored = keyboard->createComponentSnapshot(keyboard->getLocalBounds(), true, 1.0f);
        checks.expect(processor.keyDisplay[size_t(note)].load() == 0 && changedPixels(released, restored) == 0,
                      name + " MIDI note-off restores the identical released keyboard appearance");
    }
}

void checkOpaqueCaps(juce::Component& editor, Checks& checks) {
    auto* slider=dynamic_cast<juce::Slider*>(findComponent(editor,"cutoff"));
    checks.expect(slider!=nullptr,"cutoff available for actual cap-renderer opacity check");
    if(slider==nullptr)return;
    std::array<juce::Image,2> images;
    for(int i=0;i<2;++i) {
        images[size_t(i)]=juce::Image(juce::Image::ARGB,120,160,true,juce::SoftwareImageType());
        juce::Graphics g(images[size_t(i)]);
        g.fillAll(i==0?juce::Colours::red:juce::Colours::blue);
        // Exercise the production renderer including the preceding scale/slot
        // draw, which formerly left the brush at 13% alpha before the cap.
        slider->getLookAndFeel().drawLinearSlider(g,30,20,45,110,75,20,130,juce::Slider::LinearVertical,*slider);
    }
    checks.expect(changedPixels(images[0].getClippedImage({49,73,7,5}),images[1].getClippedImage({49,73,7,5}))==0,
                  "solid cap core hides both contrasting backgrounds with identical pixels");
    checks.expect(changedPixels(images[0],images[1])>1000,
                  "cap-opacity check retains genuinely different surrounding backgrounds");
}

void checkAgeEvents(SH09Processor& processor, juce::Component& editor, Checks& checks) {
    auto* keyboard=findComponent(editor,"keyboard");
    if(keyboard==nullptr)return;
    auto* panel=keyboard->getParentComponent();
    const auto capture=[&](float age) {
        setParameter(processor,"age",age);settleEditor();
        return panel->createComponentSnapshot(panel->getLocalBounds(),true,1.0f);
    };
    // Compare the real renderer across tiny threshold crossings. An ordinary
    // continuous tint cannot create this many >12-level pixel changes.
    for(const float onset:{.25f,.38f,.49f,.60f,.72f,.86f}) {
        const auto before=capture(onset-.0001f),appeared=capture(onset+.0001f),later=capture(onset+.005f);
        const int jump=changedPixels(before,appeared,12),drift=changedPixels(appeared,later,12);
        checks.expect(jump>30,"stain event "+juce::String(onset,2)+" appears abruptly in actual panel pixels");
        checks.expect(drift<juce::jmax(20,jump/5),"stain event "+juce::String(onset,2)+" stays fixed after appearance");
    }
    for(const float onset:{.82f,.94f}) {
        capture(onset-.0001f);
        const auto before=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,2.0f);
        capture(onset+.0001f);
        const auto appeared=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,2.0f);
        checks.expect(changedPixels(before,appeared,20)>10,"key crack "+juce::String(onset,2)+" first appears as a discrete trace");
    }
    const float width=float(keyboard->getWidth())/19;
    for(const float age:{0.0f,1.0f}) {
        capture(age);
        const auto keys=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        std::vector<juce::Image> whites,blacks;
        for(int i=0;i<19;++i) {
            const int x=juce::roundToInt((float(i)+.5f)*width)-10;
            whites.push_back(keys.getClippedImage({x,keyboard->getHeight()-70,20,50}).createCopy());
        }
        int before=0;
        for(int note=53;note<=84;++note) {
            const int pitch=note%12;
            const bool black=pitch==1||pitch==3||pitch==6||pitch==8||pitch==10;
            if(black) {
                const int x=juce::roundToInt(float(before)*width)-8;
                const int y=juce::roundToInt(float(keyboard->getHeight())*.64f)-52;
                blacks.push_back(keys.getClippedImage({x,y,16,36}).createCopy());
            } else ++before;
        }
        const auto unique=[](const std::vector<juce::Image>& samples) {
            for(size_t i=0;i<samples.size();++i)for(size_t j=0;j<i;++j)
                if(changedPixels(samples[i],samples[j])==0)return false;
            return true;
        };
        const auto name=age==0?"healthy":"aged";
        checks.expect(whites.size()==19&&unique(whites),juce::String("all 19 ")+name+" white-key samples have distinct actual pixels");
        checks.expect(blacks.size()==13&&unique(blacks),juce::String("all 13 ")+name+" black-key samples have distinct actual pixels");
    }
    setParameter(processor,"age",0.0f);settleEditor();
}

double luminance(juce::Colour colour) {
    const auto linear = [](double value) { return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4); };
    return 0.2126 * linear(colour.getFloatRed()) + 0.7152 * linear(colour.getFloatGreen())
         + 0.0722 * linear(colour.getFloatBlue());
}

void checkValueBubbleColours(juce::Component& editor, Checks& checks) {
    auto* slider = dynamic_cast<juce::Slider*>(findComponent(editor, "cutoff"));
    checks.expect(slider != nullptr, "cutoff slider exposes its value-bubble colour configuration");
    if (slider == nullptr) return;
    // JUCE Slider::PopupDisplayComponent uses TooltipWindow::textColourId.
    // This checks configured colours only; it does not claim that a hover popup
    // was opened or that its placement/antialiased glyphs were inspected.
    const auto text = slider->findColour(juce::TooltipWindow::textColourId, true);
    const auto background = slider->getLookAndFeel().findColour(juce::BubbleComponent::backgroundColourId);
    const auto a = luminance(text), b = luminance(background);
    const auto contrast = (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
    checks.expect(text.isOpaque() && background.isOpaque() && contrast >= 4.5,
                  "configured value-bubble text/background contrast >= 4.5:1 (actual " + juce::String(contrast, 2) + ":1)");
}

bool writePNG(const juce::Image& image, const juce::File& file) {
    if (!image.isValid()) return false;
    if (auto stream = file.createOutputStream()) {
        stream->setPosition(0);
        if (stream->truncate().failed()) return false;
        const bool written = juce::PNGImageFormat().writeImageToStream(image, *stream);
        stream->flush();
        return written && stream->getStatus().wasOk();
    }
    return false;
}

void renderBenderPositions(SH09Processor& processor, juce::Component& editor,
                          const juce::File& directory, Checks& checks) {
    auto* bender = dynamic_cast<juce::Slider*>(findComponent(editor, "bender"));
    checks.expect(bender != nullptr, "bender is available for position snapshot evidence");
    if (bender == nullptr) return;
    std::array<juce::Image, 3> positions;
    const std::array<const char*, 3> names {"left", "neutral", "right"};
    for (size_t i = 0; i < positions.size(); ++i) {
        bender->setValue(double(i) - 1.0, juce::sendNotificationSync);
        positions[i] = bender->createComponentSnapshot(bender->getLocalBounds(), true, 3.0f);
        const auto file = directory.getChildFile(juce::String("bender_") + names[i] + ".png");
        checks.expect(writePNG(positions[i], file), "actual bender " + juce::String(names[i]) + " PNG export");
        std::cout << "IMAGE " << file.getFullPathName() << '\n';
    }
    checks.expect(changedPixels(positions[0], positions[1], 12) >= 100
                  && changedPixels(positions[1], positions[2], 12) >= 100
                  && changedPixels(positions[0], positions[2], 12) >= 100,
                  "left, neutral and right bender positions visibly differ");
    bender->setValue(0.0, juce::sendNotificationSync);
    processMidi(processor, juce::MidiMessage::allNotesOff(1));
    const auto restored = bender->createComponentSnapshot(bender->getLocalBounds(), true, 3.0f);
    checks.expect(changedPixels(positions[1], restored) == 0, "bender neutral appearance restores after both extremes");
}

void checkSwitchAttachment(SH09Processor& processor, juce::Component& editor, Checks& checks) {
    auto* component = findComponent(editor, "pwmMode");
    checks.expect(component != nullptr, "PWM physical switch is available for interaction");
    if (component == nullptr) return;
    auto* parameter = processor.state.getParameter("pwmMode");
    const auto original = parameter->convertFrom0to1(parameter->getValue());
    // The panel order is LFO / MANUAL / ENV; host values remain Manual=0/LFO=1/Env=2.
    const bool homeHandled = component->keyPressed(juce::KeyPress(juce::KeyPress::homeKey));
    checks.expect(homeHandled && parameter->convertFrom0to1(parameter->getValue()) == 1.0f,
                  "PWM keyboard Home selects the physical LFO endpoint");
    const bool endHandled = component->keyPressed(juce::KeyPress(juce::KeyPress::endKey));
    checks.expect(endHandled && parameter->convertFrom0to1(parameter->getValue()) == 2.0f,
                  "PWM keyboard End selects the physical envelope endpoint");
    const auto before = component->createComponentSnapshot(component->getLocalBounds(), true, 1.0f);
    // Simulate off-message-thread host automation. No component is touched by this worker.
    std::thread automation([parameter] { parameter->setValueNotifyingHost(parameter->convertTo0to1(0.0f)); });
    automation.join();
    settleEditor();
    const auto after = component->createComponentSnapshot(component->getLocalBounds(), true, 1.0f);
    bool changed = false;
    if (before.isValid() && after.isValid() && before.getBounds() == after.getBounds())
        for (int y = 0; y < before.getHeight() && !changed; ++y)
            for (int x = 0; x < before.getWidth() && !changed; ++x)
                changed = before.getPixelAt(x, y) != after.getPixelAt(x, y);
    checks.expect(changed && parameter->convertFrom0to1(parameter->getValue()) == 0.0f,
                  "worker-thread automation visibly redraws the physical PWM switch");
    setParameter(processor, "pwmMode", original);
    settleEditor();
}

void checkCompatibility(Checks& checks) {
    SH09Processor source;
    const auto& parameters = source.getParameters();
    checks.expect(parameters.size() == int(legacyParameterIDs.size()), "all 35 legacy parameter slots retained");
    bool sameOrder = parameters.size() == int(legacyParameterIDs.size());
    for (size_t i = 0; i < legacyParameterIDs.size(); ++i) {
        const auto* parameter = source.state.getParameter(legacyParameterIDs[i]);
        sameOrder = sameOrder && parameter != nullptr && i < size_t(parameters.size()) && parameter == parameters[int(i)];
    }
    checks.expect(sameOrder, "legacy parameter IDs and host indices retained");
    if (!sameOrder) return;

    checks.expect(source.getName() == "Toyland KB-09", "processor has the single Toyland KB-09 identity");
    auto* legacy = source.state.getParameter("branding");
    checks.expect(!legacy->isAutomatable(), "deprecated branding slot is not advertised as automatable");
    checks.expect(legacy->getText(0.0f, 128) == "Toyland KB-09" && legacy->getText(1.0f, 128) == "Toyland KB-09",
                  "both legacy branding values display the current identity");

    // Distinct values exercise the whole parameter mapping, including choice ranges.
    for (size_t i = 0; i + 1 < legacyParameterIDs.size(); ++i)
        parameters[int(i)]->setValueNotifyingHost(float((i * 7 + 3) % 31) / 30.0f);

    for (int legacyValue : {0, 1}) {
        setParameter(source, "branding", float(legacyValue));
        std::array<float, legacyParameterIDs.size()> expected {};
        for (size_t i = 0; i < expected.size(); ++i) expected[i] = parameters[int(i)]->getValue();
        juce::MemoryBlock state;
        source.getStateInformation(state);
        auto xml = juce::AudioProcessor::getXmlFromBinary(state.getData(), int(state.getSize()));
        checks.expect(xml != nullptr && xml->hasTagName("SH09State") && xml->getIntAttribute("schema", -1) == 1,
                      "legacy SH09State root and schema 1, branding=" + juce::String(legacyValue));
        SH09Processor restored;
        restored.setStateInformation(state.getData(), int(state.getSize()));
        bool matches = true;
        for (size_t i = 0; i < expected.size(); ++i)
            matches = matches && std::abs(restored.getParameters()[int(i)]->getValue() - expected[i]) <= 1.0e-6f;
        checks.expect(matches, "all parameter values round-trip with legacy branding=" + juce::String(legacyValue));
        restored.setCurrentProgram(0);
        checks.expect(restored.state.getRawParameterValue("branding")->load() == float(legacyValue),
                      "factory patch retains inert legacy slot=" + juce::String(legacyValue));
    }
}

juce::String componentKind(juce::Component& component) {
    if (dynamic_cast<juce::Slider*>(&component) != nullptr) return "Slider";
    if (dynamic_cast<juce::ComboBox*>(&component) != nullptr) return "ComboBox";
    if (dynamic_cast<juce::Button*>(&component) != nullptr) return "Button";
    if (dynamic_cast<juce::Label*>(&component) != nullptr) return "Label";
    return "Component"; // Physical choice controls may be custom components.
}

void collectComponents(juce::Component& component, juce::Component& editor, juce::Array<juce::var>& entries,
                       juce::StringArray& visibleIDs, bool ancestorsVisible, int depth = 0) {
    const bool visible = ancestorsVisible && component.isVisible();
    if (visible && component.getComponentID().isNotEmpty()) visibleIDs.addIfNotAlreadyThere(component.getComponentID());
    auto object = std::make_unique<juce::DynamicObject>();
    const auto bounds = editor.getLocalArea(&component, component.getLocalBounds());
    object->setProperty("depth", depth);
    object->setProperty("id", component.getComponentID());
    object->setProperty("name", component.getName());
    object->setProperty("title", component.getTitle());
    object->setProperty("kind", componentKind(component));
    object->setProperty("visible", visible);
    object->setProperty("enabled", component.isEnabled());
    object->setProperty("x", bounds.getX());
    object->setProperty("y", bounds.getY());
    object->setProperty("width", bounds.getWidth());
    object->setProperty("height", bounds.getHeight());
    entries.add(juce::var(object.release()));
    for (int i = 0; i < component.getNumChildComponents(); ++i)
        collectComponents(*component.getChildComponent(i), editor, entries, visibleIDs, visible, depth + 1);
}

void advanceAudio(SH09Processor& processor, bool heldNote, bool externalSignal) {
    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;
    // Freeze the meters at a reproducible musical state, using actual DSP output.
    for (int block = 0; block < 96; ++block) {
        buffer.clear();
        if (externalSignal) {
            for (int sample = 0; sample < blockSize; ++sample) {
                const auto t = double(block * blockSize + sample) / sampleRate;
                const auto signal = float(0.42 * std::sin(juce::MathConstants<double>::twoPi * 110.0 * t));
                buffer.setSample(0, sample, signal);
                buffer.setSample(1, sample, signal);
            }
        }
        if (heldNote && block == 0) midi.addEvent(juce::MidiMessage::noteOn(1, 60, juce::uint8(100)), 0);
        processor.processBlock(buffer, midi);
    }
}

struct EditorDeleter {
    SH09Processor* processor;
    void operator()(juce::AudioProcessorEditor* editor) const {
        if (editor != nullptr) {
            processor->editorBeingDeleted(editor);
            delete editor;
        }
    }
};

struct Fixture { const char* name; int program; bool agedModern; bool external; };

void renderFixture(const Fixture& fixture, const juce::File& directory, Checks& checks) {
    SH09Processor processor;
    processor.setCurrentProgram(0);
    processor.prepareToPlay(sampleRate, blockSize);
    std::unique_ptr<juce::AudioProcessorEditor, EditorDeleter> editor(processor.createEditorAndMakeActive(), {&processor});
    if (editor == nullptr) throw std::runtime_error("Processor did not create an editor");
    editor->setVisible(true);
    settleEditor();
    if (fixture.program == 0) {
        checkSwitchAttachment(processor, *editor, checks);
        checkKeyboardFeedback(processor, *editor, checks);
        checkOpaqueCaps(*editor, checks);
        checkValueBubbleColours(*editor, checks);
        renderBenderPositions(processor, *editor, directory, checks);
        checkAgeEvents(processor, *editor, checks);
    }

    // Change the patch after creating the editor to exercise attachment redraw.
    processor.setCurrentProgram(fixture.program);
    if (fixture.agedModern) {
        setParameter(processor, "modern", 1.0f);
        setParameter(processor, "age", juce::String(fixture.name)=="worn"?1.0f:0.82f);
        setParameter(processor, "velocity", 0.8f);
        setParameter(processor, "aftertouch", 0.6f);
        setParameter(processor, "cutoff", 0.43f);
        setParameter(processor, "resonance", 0.6f);
    }
    advanceAudio(processor, fixture.agedModern, fixture.external);
    settleEditor();

    for (const auto size : {juce::Point<int>(3840, 2160), juce::Point<int>(2880, 1620)}) {
        editor->setSize(size.x, size.y);
        settleEditor();
        const auto stem = juce::String(fixture.name) + "_" + juce::String(size.x) + "x" + juce::String(size.y);
        checks.expect(editor->getWidth() == size.x && editor->getHeight() == size.y, stem + " editor dimensions");
        checkFooterGeometry(*editor, checks, stem);
        auto snapshot = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
        checks.expect(snapshot.isValid() && snapshot.getWidth() == size.x && snapshot.getHeight() == size.y,
                      stem + " snapshot dimensions");
        auto file = directory.getChildFile(stem + ".png");
        checks.expect(writePNG(snapshot, file), stem + " PNG export");

        juce::Array<juce::var> entries;
        juce::StringArray visibleIDs;
        collectComponents(*editor, *editor, entries, visibleIDs, true);
        checks.expect(!visibleIDs.contains("branding"), stem + " omits the deprecated branding selector");
        // Record control presence without assuming implementation-specific widget types.
        for (const auto* id : {"range", "wave", "pwmMode", "subMode", "lfoWave", "vcfEnvMode", "vcaMode", "envMode", "keyboard", "savePreset", "loadPreset"})
            checks.expect(visibleIDs.contains(id), stem + " exposes physical choice " + id);
        checks.expect(directory.getChildFile(stem + ".components.json").replaceWithText(juce::JSON::toString(juce::var(entries), false)),
                      stem + " component metadata export");
        std::cout << "IMAGE " << file.getFullPathName() << '\n';
    }
    if (fixture.program == 0) {
        // Unlike the two 16:9 exports, these exercise both horizontal and vertical
        // letterboxing. This remains a peerless layout test, not native L/window sizing.
        for (const auto size : {juce::Point<int>(3840, 1400), juce::Point<int>(1440, 2160)}) {
            editor->setSize(size.x, size.y);
            settleEditor();
            const auto context = "init_" + juce::String(size.x) + "x" + juce::String(size.y);
            checks.expect(editor->getWidth() == size.x && editor->getHeight() == size.y,
                          context + " nondefault viewport dimensions");
            checkFooterGeometry(*editor, checks, context);
        }
    }
    editor.reset();
    processor.releaseResources();
}
}

int main(int argc, char** argv) {
    if (argc != 2 || juce::String(argv[1]) == "--help") {
        std::cout << "Usage: SH09GUIRender <output-directory>\n"
                     "Exports actual editor PNGs and component JSON at 3840x2160 and 2880x1620,\n"
                     "plus three small bender-position PNGs; checks wide/tall footer geometry without PNG export.\n";
        return argc == 2 ? 0 : 2;
    }
    juce::ScopedJuceInitialiser_GUI initialiseGUI;
    Checks checks;
    const auto directory = juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1]));
    try {
        if (auto result = directory.createDirectory(); result.failed()) throw std::runtime_error(result.getErrorMessage().toStdString());
        checks.expect(juce::MessageManager::getInstance()->isThisTheMessageThread(), "fixture runs on the JUCE message thread");
        checkWearAtlas(checks);
        checkCompatibility(checks);
        for (const auto fixture : {Fixture{"init", 0, false, false}, Fixture{"aged_modern", 14, true, false},
                                   Fixture{"external_follower", 12, false, true},Fixture{"worn",14,true,false}})
            renderFixture(fixture, directory, checks);
    } catch (const std::exception& error) {
        checks.expect(false, juce::String("fixture exception: ") + error.what());
    }
    const auto summary = "RESULT failures=" + juce::String(checks.failures) + " checks=" + juce::String(checks.total) + "\n";
    std::cout << summary;
    if (!directory.getChildFile("GUIRender.log").replaceWithText(checks.log + summary)) {
        std::cerr << "Unable to write GUIRender.log\n";
        return 1;
    }
    return checks.failures == 0 ? 0 : 1;
}
