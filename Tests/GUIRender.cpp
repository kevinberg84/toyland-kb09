#include "../Source/Plugin/Processor.h"
#include "../Source/GUI/ClosePrompt.h"
#include "../Source/GUI/KeyboardFeedback.h"
#include <BinaryData.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
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

void checkPersonalArtwork(Checks& checks) {
    const auto chassis=juce::ImageFileFormat::loadFrom(BinaryData::chassispersonalaged_png,BinaryData::chassispersonalaged_pngSize);
    checks.expect(chassis.isValid()&&chassis.getWidth()==1774&&chassis.getHeight()==887,
                  "personal wear endpoint keeps the clean chassis registration and dimensions");
    const auto cap=juce::ImageFileFormat::loadFrom(BinaryData::faderpersonaloverhead_png,BinaryData::faderpersonaloverhead_pngSize);
    checks.expect(cap.isValid()&&cap.hasAlphaChannel(),"corrected top-and-side-stripe cap has a transparent surrounding canvas");

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
            // Compare visible, premultiplied colour. Unpremultiplying a dust
            // edge with alpha 1 vs 2 can amplify rounding into a false jump.
            const auto channel=[](juce::uint8 colour,juce::uint8 alpha){return int(colour)*int(alpha)/255;};
            const auto delta = std::max({std::abs(channel(p.getRed(),p.getAlpha()) - channel(q.getRed(),q.getAlpha())),
                                        std::abs(channel(p.getGreen(),p.getAlpha()) - channel(q.getGreen(),q.getAlpha())),
                                        std::abs(channel(p.getBlue(),p.getAlpha()) - channel(q.getBlue(),q.getAlpha())),
                                        std::abs(int(p.getAlpha()) - int(q.getAlpha()))});
            if (delta > threshold) ++changed;
        }
    return changed;
}

struct PixelChanges {
    int count=0;
    juce::Rectangle<int> bounds;
};
PixelChanges visibleChanges(const juce::Image& before,const juce::Image& after,int threshold=12) {
    PixelChanges result;
    if(!before.isValid()||!after.isValid()||before.getBounds()!=after.getBounds())return result;
    const juce::Image::BitmapData a(before,juce::Image::BitmapData::readOnly),b(after,juce::Image::BitmapData::readOnly);
    for(int y=0;y<before.getHeight();++y)for(int x=0;x<before.getWidth();++x) {
        const auto p=a.getPixelColour(x,y),q=b.getPixelColour(x,y);
        const auto channel=[](juce::uint8 c,juce::uint8 alpha){return int(c)*int(alpha)/255;};
        const int delta=std::max({std::abs(channel(p.getRed(),p.getAlpha())-channel(q.getRed(),q.getAlpha())),
            std::abs(channel(p.getGreen(),p.getAlpha())-channel(q.getGreen(),q.getAlpha())),
            std::abs(channel(p.getBlue(),p.getAlpha())-channel(q.getBlue(),q.getAlpha())),
            std::abs(int(p.getAlpha())-int(q.getAlpha()))});
        if(delta>threshold) {
            const juce::Rectangle<int> pixel(x,y,1,1);
            result.bounds=result.count==0?pixel:result.bounds.getUnion(pixel);++result.count;
        }
    }
    return result;
}

constexpr int controlRenderScale=4;
juce::Image renderLinearControl(juce::Slider& slider,float position,juce::Colour background) {
    juce::Image image(juce::Image::ARGB,120*controlRenderScale,160*controlRenderScale,true,juce::SoftwareImageType());
    juce::Graphics g(image);g.fillAll(background);g.addTransform(juce::AffineTransform::scale(float(controlRenderScale)));
    // Exercise the actual preceding scale/slot draw and the production cap/post.
    slider.getLookAndFeel().drawLinearSlider(g,30,20,45,110,position,20,130,juce::Slider::LinearVertical,slider);
    return image;
}
juce::Rectangle<int> controlPixels(juce::Rectangle<int> area) {
    return {area.getX()*controlRenderScale,area.getY()*controlRenderScale,
            area.getWidth()*controlRenderScale,area.getHeight()*controlRenderScale};
}

void processMidi(SH09Processor& processor, const juce::MidiMessage& message) {
    juce::AudioBuffer<float> buffer(2, blockSize);
    buffer.clear();
    juce::MidiBuffer midi;
    midi.addEvent(message, 0);
    processor.processBlock(buffer, midi);
    settleEditor();
}

bool writePNG(const juce::Image& image, const juce::File& file);

void checkKeyboardFeedback(SH09Processor& processor, juce::Component& editor, const juce::File& directory, Checks& checks) {
    auto* keyboard = findComponent(editor, "keyboard");
    checks.expect(keyboard != nullptr, "keyboard is available for MIDI visual-feedback checks");
    if (keyboard == nullptr) return;
    const auto blackNote=[](int note) {const int pitch=note%12;return pitch==1||pitch==3||pitch==6||pitch==8||pitch==10;};
    const auto keyPoint=[&](int note,bool marker) {
        int whites=0;for(int pitch=53;pitch<note;++pitch)if(!blackNote(pitch))++whites;
        const float width=float(keyboard->getWidth())/19;
        const float height=float(keyboard->getHeight())*(blackNote(note)?.64f:1.0f);
        return juce::Point<float>((float(whites)+(blackNote(note)?0.0f:.5f))*width,marker?height-7:height*.80f);
    };
    const auto markerIs=[&](const juce::Image& image,int note,juce::Colour expected) {
        const auto point=keyPoint(note,true).roundToInt();
        const auto colour=image.getPixelAt(point.x,point.y);
        return std::abs(int(colour.getRed())-int(expected.getRed()))<8
            && std::abs(int(colour.getGreen())-int(expected.getGreen()))<8
            && std::abs(int(colour.getBlue())-int(expected.getBlue()))<8;
    };
    const juce::Colour amberBar(0xffdbb572),blueBar(0xff65c8ff);
    const auto markerAbsent=[&](const juce::Image& image,int note) {
        return !markerIs(image,note,amberBar)&&!markerIs(image,note,blueBar);
    };
    const auto mouseEvent=[&](int note) {
        const auto point=keyPoint(note,false);const auto now=juce::Time::getCurrentTime();
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,
            juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,
            keyboard,keyboard,now,point,now,1,false);
    };
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
        checks.expect(markerIs(held,note,amberBar),name+" held MIDI note has an amber lower-edge bar");
        processMidi(processor, juce::MidiMessage::noteOff(1, note));
        const auto restored = keyboard->createComponentSnapshot(keyboard->getLocalBounds(), true, 1.0f);
        checks.expect(processor.keyDisplay[size_t(note)].load() == 0 && changedPixels(released, restored) == 0,
                      name + " MIDI note-off restores the identical released keyboard appearance");

        keyboard->mouseDown(mouseEvent(note));
        const auto immediate=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        checks.expect(processor.displayedNote.load()==-1&&markerIs(immediate,note,amberBar),
                      name+" mouse press shows amber immediately, before any audio callback");
        checks.expect(writePNG(immediate,directory.getChildFile(note==60?"keyboard_mouse_white.png":"keyboard_mouse_black.png")),
                      name+" immediate mouse-held keyboard PNG export");
        processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
        const auto processed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        checks.expect(processor.displayedNote.load()==note&&markerIs(processed,note,amberBar),
                      name+" queued mouse note remains amber after audio processing");
        keyboard->mouseUp(mouseEvent(note));
        const auto immediateRelease=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        checks.expect(processor.displayedNote.load()==note&&changedPixels(released,immediateRelease)==0,
                      name+" mouse release removes its highlight immediately before audio processing");
        processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
        const auto mouseReleased=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        checks.expect(processor.displayedNote.load()==-1&&changedPixels(released,mouseReleased)==0,
                      name+" mouse release clears the note and both visual marks");
    }
    keyboard->mouseDown(mouseEvent(55));
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    keyboard->mouseDrag(mouseEvent(64));
    auto mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.displayedNote.load()==55&&markerIs(mixed,64,amberBar)&&markerAbsent(mixed,55),
                  "upward mouse drag moves amber and releases the old key before audio processing");
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.displayedNote.load()==64&&processor.displayedKeyboardNote.load()==64
                  &&processor.keyDisplay[55].load()==0&&markerIs(mixed,64,amberBar),
                  "processed upward drag keeps the destination selected when the previous mouse note is released");
    keyboard->mouseUp(mouseEvent(64));
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    processMidi(processor,juce::MidiMessage::noteOn(1,60,juce::uint8(100)));
    keyboard->mouseDown(mouseEvent(64));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(markerIs(mixed,60,amberBar)&&markerAbsent(mixed,64),
                  "higher mouse note is shaded without a coloured bar while lower MIDI-held note retains amber");
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    keyboard->mouseDrag(mouseEvent(55));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(markerIs(mixed,55,amberBar),"lower mouse drag shows amber immediately before its queued note is processed");
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.displayedNote.load()==55&&processor.keyDisplay[64].load()==0
                  &&markerIs(mixed,55,amberBar)&&markerAbsent(mixed,60),
                  "processed mouse drag transfers low-note priority while the MIDI note remains held");
    keyboard->mouseDrag(mouseEvent(64));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.displayedNote.load()==55&&markerAbsent(mixed,55)
                  &&markerIs(mixed,60,amberBar)&&markerAbsent(mixed,64),
                  "upward mouse drag immediately restores lower MIDI priority without stale amber on the old mouse key");
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.displayedNote.load()==60&&processor.keyDisplay[55].load()==0
                  &&markerIs(mixed,60,amberBar)&&markerAbsent(mixed,64),
                  "processed upward mouse drag preserves the lower held MIDI note");
    keyboard->mouseUp(mouseEvent(64));
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.displayedNote.load()==60&&processor.keyDisplay[55].load()==0&&markerIs(mixed,60,amberBar),
                  "mouse release restores the still-held MIDI note and its amber bar");
    keyboard->mouseDown(mouseEvent(60));
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    keyboard->mouseDrag(mouseEvent(64));
    mixed=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.keyDisplay[60].load()==2&&markerIs(mixed,60,amberBar)&&markerAbsent(mixed,64),
                  "moving the mouse away from an overlapping MIDI pitch retains that MIDI key and its priority immediately");
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    checks.expect(processor.keyDisplay[60].load()==1&&processor.displayedNote.load()==60,
                  "processed same-pitch mouse release preserves the real MIDI contribution");
    keyboard->mouseUp(mouseEvent(64));
    processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
    processMidi(processor,juce::MidiMessage::noteOff(1,60));
    auto* computer=dynamic_cast<kb09_gui::KeyboardFeedback*>(keyboard);
    checks.expect(computer!=nullptr,"real keyboard exposes direct computer performance feedback");
    if(computer) {
        const auto released=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        for(const int note:{60,61}) {
            computer->setComputerNote(note,true);
            auto image=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
            checks.expect(processor.keyDisplay[size_t(note)].load()==0&&markerIs(image,note,amberBar),
                "computer key is visibly pressed before the audio callback");
            processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
            image=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
            checks.expect(processor.displayedNote.load()==note&&markerIs(image,note,amberBar),
                "computer key stays visibly pressed after audio consumes it");
            computer->setComputerNote(note,false);
            image=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
            checks.expect(changedPixels(released,image)==0,
                "computer key visibly releases before the next audio callback");
            processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
        }
        processMidi(processor,juce::MidiMessage::noteOn(1,60,juce::uint8(100)));
        computer->setComputerNote(60,true);
        processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
        computer->setComputerNote(60,false);
        checks.expect(markerIs(keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f),60,amberBar),
            "computer key release preserves same-pitch MIDI visual feedback");
        processMidi(processor,juce::MidiMessage::noteOff(1,60));
    }
    const auto unlatched=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    setParameter(processor,"vcaMode",0);
    processMidi(processor,juce::MidiMessage::noteOn(1,60,juce::uint8(100)));
    processMidi(processor,juce::MidiMessage::noteOff(1,60));
    const auto latched=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
    checks.expect(processor.keyDisplay[60].load()==0&&processor.displayedHeldNote.load()==60
                  &&markerIs(latched,60,amberBar)&&changedPixels(unlatched,latched,12)>100,
                  "VCA HOLD keeps the last sounding key visibly depressed after key-up");
    if(computer) {
        computer->setComputerNote(61,true);
        processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
        computer->setComputerNote(61,false);
        processMidi(processor,juce::MidiMessage::controllerEvent(1,121,0));
        checks.expect(markerIs(keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f),61,amberBar),
            "computer-key performance also latches the pressed key in HOLD");
    }
    setParameter(processor,"vcaMode",1);
    checks.expect(changedPixels(unlatched,keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f))==0,
                  "leaving VCA HOLD restores the released key appearance");
}

void checkFineGestures(SH09Processor& processor,juce::Component& editor,Checks& checks) {
    for(const auto* id:{"volume","cutoff","bender"}) {
        auto* slider=dynamic_cast<juce::Slider*>(findComponent(editor,id));
        if(!slider){checks.expect(false,juce::String(id)+" fine control exists");continue;}
        auto event=[&](juce::Point<float> point,bool shift) {
            const auto now=juce::Time::getCurrentTime();
            return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,
                juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier|(shift?juce::ModifierKeys::shiftModifier:0)),
                1,0,0,0,0,slider,slider,now,slider->getLocalBounds().toFloat().getCentre(),now,1,true);
        };
        const double centre=(slider->getMinimum()+slider->getMaximum())*.5;
        auto delta=[&](bool shift) {
            slider->setValue(centre,juce::sendNotificationSync);
            const auto start=slider->getLocalBounds().toFloat().getCentre();
            slider->mouseDown(event(start,shift));
            slider->mouseDrag(event(start+juce::Point<float>(15,-15),shift));
            const double change=std::abs(slider->getValue()-centre);
            slider->mouseUp(event(start,shift));return change;
        };
        const double normal=delta(false),fine=delta(true);
        checks.expect(normal>0&&fine>0&&fine<normal*.15,juce::String(id)+" SHIFT drag is ten times finer");
    }
    auto* age=dynamic_cast<juce::Slider*>(findComponent(editor,"age"));
    checks.expect(age!=nullptr,"AGE click-to-position control exists");
    if(age) {
        const double saved=age->getValue();
        const std::array<const char*,10> phrases{{"Nice 'n' New","Barely Broken In","Like New-ish",
            "Lightly Loved","Seen Some Shit","Vintage, Baby!","Sold As-Is","Smells Like 1983",
            "Fire Hazard","Who Let You Have This?"}};
        for(int i=0;i<10;++i)for(double fraction:{0.0,.05,.099})
            checks.expect(age->getTextFromValue(i*.1+fraction)==phrases[size_t(i)],
                          "AGE phrase band "+juce::String(i)+" at "+juce::String(fraction,3));
        checks.expect(age->getTextFromValue(1)==phrases.back(),"AGE maximum uses final phrase");
        age->setValue(0,juce::sendNotificationSync);
        const juce::Point<float> point(age->getWidth()*.8f,age->getHeight()*.5f);
        const auto now=juce::Time::getCurrentTime();
        const juce::MouseEvent click(juce::Desktop::getInstance().getMainMouseSource(),point,
            juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,
            age,age,now,point,now,1,false);
        age->mouseDown(click);
        checks.expect(age->getValue()>.65&&processor.state.getRawParameterValue("age")->load()>.65f,
                      "AGE track click immediately moves thumb and parameter before release");
        age->mouseUp(click);age->setValue(saved,juce::sendNotificationSync);settleEditor();
    }
    setParameter(processor,"modern",1);settleEditor();
    for(const auto* id:{"velocity","aftertouch","extGain"}) {
        auto* control=dynamic_cast<juce::Slider*>(findComponent(editor,id));
        checks.expect(control&&control->isEnabled(),juce::String(id)+" utility enabled in Modern mode");
        if(!control)continue;
        const double saved=control->getValue();control->setValue(control->getMinimum(),juce::sendNotificationSync);
        const juce::Point<float> point(control->getWidth()*.8f,control->getHeight()*.5f);
        const auto now=juce::Time::getCurrentTime();
        const juce::MouseEvent click(juce::Desktop::getInstance().getMainMouseSource(),point,
            juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,control,control,now,point,now,1,false);
        control->mouseDown(click);
        checks.expect(control->valueToProportionOfLength(control->getValue())>.65,
                      juce::String(id)+" track click immediately moves thumb before release");
        control->mouseUp(click);control->setValue(saved,juce::sendNotificationSync);
    }
    setParameter(processor,"modern",1);setParameter(processor,"lfoSync",1);settleEditor();
    auto* rate=dynamic_cast<juce::Slider*>(findComponent(editor,"lfoRate"));
    checks.expect(rate&&rate->getTextFromValue(0)=="4 bars"&&rate->getTextFromValue(1)=="1/64 T","RATE value box presents SYNC divisions");
    setParameter(processor,"modern",0);settleEditor();
    checks.expect(rate&&rate->getTextFromValue(1)=="10.00","Authentic RATE value box keeps numeric scale");
    if(auto* keyboard=findComponent(editor,"keyboard")) {
        const auto snap=[&]{return keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);};
        const auto authentic=snap();
        setParameter(processor,"modern",1);settleEditor();const auto labelled=snap();
        checks.expect(changedPixels(authentic,labelled)==0,"C octave labels remain visible in both Authentic and Modern modes");
        const float oldRange=processor.state.getRawParameterValue("range")->load();
        setParameter(processor,"range",oldRange==0?1:0);settleEditor();
        checks.expect(changedPixels(labelled,snap(),5)>15,"RANGE updates octave labels without touching the keyboard");
        setParameter(processor,"range",oldRange);setParameter(processor,"modern",0);settleEditor();
        checks.expect(changedPixels(authentic,snap())==0,"restoring RANGE keeps the same Authentic C key labels");
    }

}

void checkOpaqueCaps(juce::Component& editor, Checks& checks) {
    auto* slider=dynamic_cast<juce::Slider*>(findComponent(editor,"cutoff"));
    checks.expect(slider!=nullptr,"cutoff available for actual cap-renderer opacity check");
    if(slider==nullptr)return;
    const std::array<juce::Image,2> images{{renderLinearControl(*slider,75,juce::Colours::red),
                                           renderLinearControl(*slider,75,juce::Colours::blue)}};
    const std::array<juce::Image,2> moved{{renderLinearControl(*slider,105,juce::Colours::red),
                                          renderLinearControl(*slider,105,juce::Colours::blue)}};
    // These cores are on opposite sides OUTSIDE the opaque slot (x49.3..55.7).
    // Comparing moved-cap renders proves the slot/scales cannot pass by themselves.
    for(const auto area:{juce::Rectangle<int>(44,74,3,3),juce::Rectangle<int>(58,74,3,3)}) {
        const auto roi=controlPixels(area);
        const bool opaque=changedPixels(images[0].getClippedImage(roi),images[1].getClippedImage(roi),2)==0;
        const bool exposed=changedPixels(moved[0].getClippedImage(roi),moved[1].getClippedImage(roi),20)>roi.getWidth()*roi.getHeight()/2;
        checks.expect(opaque&&exposed,juce::String(area.getX()<52?"left":"right")
                      +" cap core outside slot is opaque; moved-cap negative control exposes the background");
    }
    checks.expect(changedPixels(images[0],images[1])>1000,
                  "cap-opacity check retains genuinely different surrounding backgrounds");
}

void checkRotaryTicks(SH09Processor& processor,juce::Component& editor,Checks& checks) {
    const float initialAge=processor.state.getRawParameterValue("age")->load();
    constexpr float origin=10,diameter=100,centreX=origin+diameter*.497f,centreY=origin+diameter*.410f;
    // Independent visual contract: a predominantly round metal top and a short
    // black radial mark. It is not a copy of the renderer's tick-drawing code.
    constexpr float projectedCos=.987688f,maskRadius=diameter*.29f;
    const auto movingTicks=[&](const std::array<juce::Image,3>& images,const std::array<float,3>& angles) {
        for(size_t state=0;state<images.size();++state) {
            int darkAndMoving=0;
            for(const float radius:{.09f,.13f,.18f,.225f}) {
                const int x=juce::roundToInt((centreX+std::sin(angles[state])*radius*diameter)*controlRenderScale);
                const int y=juce::roundToInt((centreY-std::cos(angles[state])*radius*diameter*projectedCos)*controlRenderScale);
                const auto own=images[state].getPixelAt(x,y),other=images[(state+1)%images.size()].getPixelAt(x,y);
                const int ownLight=std::max({int(own.getRed()),int(own.getGreen()),int(own.getBlue())});
                const int otherLight=std::max({int(other.getRed()),int(other.getGreen()),int(other.getBlue())});
                if(ownLight<85&&otherLight-ownLight>35)++darkAndMoving;
            }
            if(darkAndMoving<3)return false;
        }
        return true;
    };
    for(const auto* id:{"range","wave","volume"}) {
        auto* slider=dynamic_cast<juce::Slider*>(findComponent(editor,id));
        checks.expect(slider!=nullptr,juce::String(id)+" is available for native top-tick checks");
        if(slider==nullptr)continue;
        const double original=slider->getValue();
        const auto rotary=slider->getRotaryParameters();
        for(const float age:{0.0f,.70f}) {
            setParameter(processor,"age",age);settleEditor();
            std::array<juce::Image,3> images;
            std::array<float,3> angles{};
            for(size_t state=0;state<images.size();++state) {
                const double requested=slider->getMinimum()+(slider->getMaximum()-slider->getMinimum())*double(state)*.5;
                slider->setValue(requested,juce::sendNotificationSync);settleEditor();
                // Use the snapped live value: waveform's midpoint is a detent,
                // not an invented continuous value between the two middle stops.
                const float normalized=float(slider->valueToProportionOfLength(slider->getValue()));
                angles[state]=rotary.startAngleRadians+(rotary.endAngleRadians-rotary.startAngleRadians)*normalized;
                images[state]=juce::Image(juce::Image::ARGB,120*controlRenderScale,120*controlRenderScale,true,juce::SoftwareImageType());
                juce::Graphics g(images[state]);g.fillAll(juce::Colour(0xff232927));
                g.addTransform(juce::AffineTransform::scale(float(controlRenderScale)));
                slider->getLookAndFeel().drawRotarySlider(g,int(origin),int(origin),int(diameter),int(diameter),
                    normalized,rotary.startAngleRadians,rotary.endAngleRadians,*slider);
            }
            const auto context=juce::String(id)+" Age "+juce::String(age,2);
            checks.expect(movingTicks(images,angles),context+" has a visible moving black top tick at minimum, midpoint and maximum");
            bool fixedLighting=true;
            for(size_t state=1;state<images.size();++state) {
                const juce::Image::BitmapData a(images[0],juce::Image::BitmapData::readOnly),b(images[state],juce::Image::BitmapData::readOnly);
                for(int y=0;y<images[0].getHeight();++y)for(int x=0;x<images[0].getWidth();++x) {
                    const float dx=((float(x)+.5f)/controlRenderScale-centreX)/maskRadius;
                    const float dy=((float(y)+.5f)/controlRenderScale-centreY)/(maskRadius*projectedCos);
                    if(dx*dx+dy*dy<=1.0f)continue;
                    // The newly restored collar index also rotates, inside its narrow annulus.
                    const float cx=((float(x)+.5f)/controlRenderScale-(origin+diameter*.497f))/(diameter);
                    const float cy=((float(y)+.5f)/controlRenderScale-(origin+diameter*.505f))/(diameter*projectedCos);
                    const float collarRadius=std::sqrt(cx*cx+cy*cy);
                    if(collarRadius>.35f&&collarRadius<.49f)continue;
                    const auto p=a.getPixelColour(x,y),q=b.getPixelColour(x,y);
                    const int delta=std::max({std::abs(int(p.getRed())-int(q.getRed())),std::abs(int(p.getGreen())-int(q.getGreen())),
                                              std::abs(int(p.getBlue())-int(q.getBlue()))});
                    if(delta>2)fixedLighting=false;
                }
            }
            checks.expect(fixedLighting,context+" rotation leaves body, skirt, shadow and lighting outside the top and collar indicator masks fixed");
            const std::array<juce::Image,3> staticTick{{images[0],images[0],images[0]}};
            checks.expect(!movingTicks(staticTick,angles),context+" tick detector rejects a static or absent moving indicator");
        }
        slider->setValue(original,juce::sendNotificationSync);settleEditor();
    }
    setParameter(processor,"age",initialAge);settleEditor();
}

void checkCapLoss(SH09Processor& processor,juce::Component& editor,Checks& checks) {
    constexpr std::array<const char*,18> faderIDs{{"lfoRate","lfoDelay","vcoMod","pulseWidth","vcoLevel","subLevel","extLevel",
        "cutoff","resonance","vcfEnv","vcfMod","attack","decay","sustain","release","bendVco","bendVcf","portamento"}};
    std::array<juce::Slider*,faderIDs.size()> sliders{};
    std::array<juce::Rectangle<int>,faderIDs.size()> bounds{};
    bool allPresent=true;
    for(size_t i=0;i<faderIDs.size();++i) {
        sliders[i]=dynamic_cast<juce::Slider*>(findComponent(editor,faderIDs[i]));
        if(sliders[i]!=nullptr)bounds[i]=sliders[i]->getBounds();else allPresent=false;
    }
    checks.expect(allPresent,"all 18 physical faders are available for individual cap-loss checks");
    if(!allPresent)return;
    std::array<float,legacyParameterIDs.size()> expected{};
    for(size_t i=0;i<expected.size();++i)expected[i]=processor.state.getParameter(legacyParameterIDs[i])->getValue();
    const float initialAge=processor.state.getRawParameterValue("age")->load();
    const auto capture=[&](float age) {
        setParameter(processor,"age",age);settleEditor();
        std::array<juce::Image,faderIDs.size()> images;
        for(size_t i=0;i<images.size();++i)
            images[i]=sliders[i]->createComponentSnapshot(sliders[i]->getLocalBounds(),true,2.0f);
        return images;
    };
    struct Event {const char* id;float onset;};
    for(const auto event:{Event{"vcoLevel",.64f},Event{"release",.88f}}) {
        const auto before=capture(event.onset-.0001f),after=capture(event.onset+.0001f),later=capture(event.onset+.003f);
        juce::StringArray changed;
        bool targetFixed=false;
        for(size_t i=0;i<sliders.size();++i) {
            if(changedPixels(before[i],after[i],12)>40)changed.add(faderIDs[i]);
            if(juce::String(faderIDs[i])==event.id)targetFixed=changedPixels(after[i],later[i],12)==0;
        }
        checks.expect(changed.size()==1&&changed[0]==event.id,
                      juce::String("Age ")+juce::String(event.onset,2)+" abruptly removes only the "+event.id+" cap");
        checks.expect(targetFixed,juce::String(event.id)+" post appears at full size and stays fixed after cap loss");
    }
    bool metadataSame=true,soundSame=true;
    for(size_t i=0;i<sliders.size();++i)
        metadataSame=metadataSame&&findComponent(editor,faderIDs[i])==sliders[i]&&bounds[i]==sliders[i]->getBounds()
            &&sliders[i]->isEnabled()&&sliders[i]->getSliderStyle()==juce::Slider::LinearVertical
            &&sliders[i]->hitTest(sliders[i]->getWidth()/2,sliders[i]->getHeight()/2);
    for(size_t i=0;i<expected.size();++i)if(juce::String(legacyParameterIDs[i])!="age")
        soundSame=soundSame&&std::abs(expected[i]-processor.state.getParameter(legacyParameterIDs[i])->getValue())<1.e-6f;
    checks.expect(metadataSame&&soundSame,"cap loss preserves every fader identity, hit area, enablement and sound parameter value");
    // Compare two production positions at fixed Age: a slot alone must fail.
    setParameter(processor,"age",.95f);settleEditor();
    for(const auto* id:{"vcoLevel","release"}) {
        auto* slider=dynamic_cast<juce::Slider*>(findComponent(editor,id));
        const auto at75=renderLinearControl(*slider,75,juce::Colour(0xff232927));
        const auto at100=renderLinearControl(*slider,100,juce::Colour(0xff232927));
        const auto thinBlade=[](const PixelChanges& pixels) {
            return pixels.count>=60&&pixels.bounds.getHeight()>=10*controlRenderScale
                &&pixels.bounds.getHeight()<=23*controlRenderScale
                &&pixels.bounds.getWidth()>0&&pixels.bounds.getWidth()<6*controlRenderScale
                &&pixels.bounds.getWidth()*5<pixels.bounds.getHeight()*2;
        };
        bool visibleAtBoth=true;
        for(const auto y:{64,89}) {
            const auto roi=controlPixels({47,y,11,23});
            visibleAtBoth=visibleAtBoth&&thinBlade(visibleChanges(at75.getClippedImage(roi),at100.getClippedImage(roi)));
        }
        checks.expect(visibleAtBoth,juce::String(id)+" has a visible thin longitudinal blade at two actual 4x-rendered positions");
        const auto roi=controlPixels({47,64,11,23});
        checks.expect(!thinBlade(visibleChanges(at100.getClippedImage(roi),at100.getClippedImage(roi))),
                      juce::String(id)+" blade-presence detector rejects an unchanged blank slot");
        const double original=slider->getValue();
        slider->setValue(.41,juce::sendNotificationSync);settleEditor();
        checks.expect(std::abs(processor.state.getRawParameterValue(id)->load()-.41f)<1.e-5f,
                      juce::String(id)+" remains attached and adjustable with its cap missing");
        slider->setValue(original,juce::sendNotificationSync);settleEditor();
    }
    juce::MemoryBlock state;processor.getStateInformation(state);
    SH09Processor restored;restored.setStateInformation(state.getData(),int(state.getSize()));
    bool roundTrips=true;
    for(const auto* id:legacyParameterIDs)
        roundTrips=roundTrips&&std::abs(processor.state.getParameter(id)->getValue()-restored.state.getParameter(id)->getValue())<1.e-6f;
    checks.expect(roundTrips,"aged state with both missing caps round-trips all 35 existing parameters");
    setParameter(processor,"age",initialAge);settleEditor();
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
    for(const float onset:{.60f}) {
        const auto before=capture(onset-.0001f),appeared=capture(onset+.0001f),later=capture(onset+.005f);
        const int jump=changedPixels(before,appeared,12),drift=changedPixels(appeared,later,12);
        checks.expect(jump>30,"stain event "+juce::String(onset,2)+" appears abruptly in actual panel pixels");
        checks.expect(drift<juce::jmax(20,jump/5),"stain event "+juce::String(onset,2)+" stays fixed after appearance");
    }
    for(const float onset:{.56f,.94f}) {
        capture(onset-.0001f);
        const auto before=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,2.0f);
        capture(onset+.0001f);
        const auto appeared=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,2.0f);
        if(onset<.9f) {
            int changedKeys=0;
            for(int key=0;key<19;++key) {
                const int left=juce::roundToInt(float(key)*before.getWidth()/19)+2;
                const int right=juce::roundToInt(float(key+1)*before.getWidth()/19)-2;
                const juce::Rectangle<int> face(left,before.getHeight()*64/100,right-left,before.getHeight()*36/100);
                if(changedPixels(before.getClippedImage(face),appeared.getClippedImage(face),20)>10)++changedKeys;
            }
            checks.expect(changedKeys==1,"exactly one white key gains a discrete hairline crack at Age .56");
        } else checks.expect(changedPixels(before,appeared,20)==0,"no second key crack appears at the former Age .94 threshold");
    }
    const float width=float(keyboard->getWidth())/19;
    juce::Colour healthyIvory;
    for(const float age:{0.0f,1.0f}) {
        capture(age);
        const auto keys=keyboard->createComponentSnapshot(keyboard->getLocalBounds(),true,1.0f);
        const auto ivory=keys.getPixelAt(juce::roundToInt(width*.5f),keyboard->getHeight()-40);
        if(age==0)healthyIvory=ivory;
        else checks.expect(healthyIvory.getBlue()-ivory.getBlue()>=25
                           && ivory.getFloatBlue()/ivory.getFloatRed()<healthyIvory.getFloatBlue()/healthyIvory.getFloatRed()-.10f,
                           "aged ivory visibly yellows while retaining the photographed key shading");
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

void checkKnobMetalAging(SH09Processor& processor,juce::Component& editor,
                        const juce::File& directory,Checks& checks) {
    auto* slider=dynamic_cast<juce::Slider*>(findComponent(editor,"range"));
    checks.expect(slider!=nullptr,"range knob is available for production metal-aging evidence");
    if(slider==nullptr)return;
    const float originalAge=processor.state.getRawParameterValue("age")->load();
    const auto rotary=slider->getRotaryParameters();
    const auto capture=[&](float age) {
        setParameter(processor,"age",age);settleEditor();
        juce::Image image(juce::Image::ARGB,120*controlRenderScale,120*controlRenderScale,true,juce::SoftwareImageType());
        juce::Graphics g(image);g.fillAll(juce::Colour(0xff232927));
        g.addTransform(juce::AffineTransform::scale(float(controlRenderScale)));
        slider->getLookAndFeel().drawRotarySlider(g,10,10,100,100,.5f,
            rotary.startAngleRadians,rotary.endAngleRadians,*slider);
        return image;
    };
    constexpr std::array<float,4> ages{{0.0f,.3f,.7f,1.0f}};
    constexpr std::array<const char*,4> names{{"age0","age3","age7","age10"}};
    std::array<juce::Image,ages.size()> images;
    for(size_t i=0;i<ages.size();++i) {
        images[i]=capture(ages[i]);
        const auto file=directory.getChildFile(juce::String("knob_wear_")+names[i]+".png");
        checks.expect(writePNG(images[i],file),juce::String("knob ")+names[i]+" actual-render metal-aging closeup export");
        std::cout<<"IMAGE "<<file.getFullPathName()<<'\n';
    }
    const auto light=[](juce::Colour colour) {
        return (double(colour.getRed())+double(colour.getGreen())+double(colour.getBlue()))/3.0;
    };
    const auto meanLight=[&](const juce::Image& image,juce::Rectangle<int> area) {
        double total=0;
        for(int y=area.getY();y<area.getBottom();++y)for(int x=area.getX();x<area.getRight();++x)
            total+=light(image.getPixelAt(x,y));
        return total/double(area.getWidth()*area.getHeight());
    };
    // Remove the best-fitting overall brightness/contrast change before measuring
    // small-scale change. Uniform tint or dulling alone cannot masquerade as pits.
    const auto textureChange=[&](const juce::Image& fresh,const juce::Image& aged,juce::Rectangle<int> area) {
        const double freshMean=meanLight(fresh,area),agedMean=meanLight(aged,area);
        double variance=0,covariance=0;
        for(int y=area.getY();y<area.getBottom();++y)for(int x=area.getX();x<area.getRight();++x) {
            const double a=light(fresh.getPixelAt(x,y))-freshMean;
            variance+=a*a;covariance+=a*(light(aged.getPixelAt(x,y))-agedMean);
        }
        const double slope=variance>1.0e-9?covariance/variance:0;
        const auto residual=[&](int x,int y) {
            return light(aged.getPixelAt(x,y))-agedMean
                -slope*(light(fresh.getPixelAt(x,y))-freshMean);
        };
        const auto interior=area.reduced(2);
        double texture=0;
        for(int y=interior.getY();y<interior.getBottom();++y)for(int x=interior.getX();x<interior.getRight();++x)
            texture+=std::abs(residual(x,y)-(residual(x-2,y)+residual(x+2,y)+residual(x,y-2)+residual(x,y+2))*.25);
        return texture/double(interior.getWidth()*interior.getHeight());
    };
    // Both patches lie fully inside visible silver surfaces, away from the tick,
    // black grip, bevel edges, cast shadow and decorative panel behind the knob.
    for(const auto surface:{std::pair<const char*,juce::Rectangle<int>>{"top interior",{48,48,23,14}},
                            std::pair<const char*,juce::Rectangle<int>>{"front silver skirt",{48,96,24,8}}}) {
        const auto area=controlPixels(surface.second);
        std::array<double,ages.size()> means{},texture{};
        for(size_t i=0;i<ages.size();++i) {
            means[i]=meanLight(images[i],area);
            texture[i]=textureChange(images[0],images[i],area);
        }
        const juce::String label(surface.first);
        checks.expect(means[0]>means[1]+1&&means[1]>means[2]+1&&means[2]>means[3]+1,
                      label+" reflections dull progressively at AGE 0, 3, 7 and 10");
        checks.expect(means[2]<means[0]*.88,
                      label+" AGE 7 is visibly duller than clean metal (mean "+juce::String(means[0],1)+" to "+juce::String(means[2],1)+")");
        auto tintOnly=images[0].createCopy();
        for(int y=area.getY();y<area.getBottom();++y)for(int x=area.getX();x<area.getRight();++x) {
            const auto value=juce::uint8(juce::jlimit(0,255,juce::roundToInt(light(images[0].getPixelAt(x,y))*.62+16)));
            tintOnly.setPixelAt(x,y,juce::Colour(value,value,value));
        }
        const double negative=textureChange(images[0],tintOnly,area);
        if(label=="top interior") {
        checks.expect(texture[2]>.8&&texture[2]>negative*3+.5,
                      label+" AGE 7 adds fine surface variation beyond a tint-only negative control ("+juce::String(texture[2],2)+" vs "+juce::String(negative,2)+")");
        checks.expect(texture[1]>.1&&texture[2]>texture[1]+.1&&texture[3]>texture[2]+.1,
                      label+" fine pitting grows gradually across AGE 3, 7 and 10 ("+juce::String(texture[1],2)+", "+juce::String(texture[2],2)+", "+juce::String(texture[3],2)+")");
        } else {
            double warmth=0;
            for(int y=area.getY();y<area.getBottom();++y)for(int x=area.getX();x<area.getRight();++x) {
                const auto c=images[3].getPixelAt(x,y);warmth+=std::abs(int(c.getRed())-int(c.getBlue()));
            }
            warmth/=double(area.getWidth()*area.getHeight());
            checks.expect(warmth<5,"aged collar stays neutral metal without brown corrosion");
            checks.expect(texture[3]<4,"aged collar dulls without the knob top's pitted corrosion texture");
        }
    }
    checks.expect(changedPixels(capture(.6999f),capture(.7001f),2)==0,
                  "adjacent AGE values keep knob pit locations fixed without an abrupt damage event");
    checks.expect(changedPixels(images[0],capture(0.0f))==0,
                  "lowering AGE restores identical clean knob pixels after maximum wear");
    setParameter(processor,"age",originalAge);settleEditor();
}

void checkSizeChoices(juce::AudioProcessorEditor& editor, const juce::File& directory, Checks& checks) {
    auto* small=dynamic_cast<juce::Button*>(findComponent(editor,"sizeS"));
    auto* medium=dynamic_cast<juce::Button*>(findComponent(editor,"sizeM"));
    auto* large=dynamic_cast<juce::Button*>(findComponent(editor,"sizeL"));
    auto* maximise=dynamic_cast<juce::Button*>(findComponent(editor,"maximise"));
    checks.expect(small!=nullptr&&medium!=nullptr&&large!=nullptr&&maximise!=nullptr,
                  "S, M, L and maximise actions are exposed by the actual editor");
    if(small==nullptr||medium==nullptr||large==nullptr||maximise==nullptr)return;
    if(!small->onClick||!medium->onClick||!large->onClick||!maximise->onClick) {
        checks.expect(false,"all editor size buttons have working actions");return;
    }
    medium->onClick();settleEditor();
    const auto mediumSize=juce::Point<int>(editor.getWidth(),editor.getHeight());
    small->onClick();settleEditor();
    const auto smallSize=juce::Point<int>(editor.getWidth(),editor.getHeight());
    checks.expect(smallSize.x<mediumSize.x&&smallSize.y<mediumSize.y,
                  "S requests a genuinely smaller viewport than enlarged M");
    checks.expect(smallSize.x>=480&&smallSize.y>=260,
                  "S retains a usable viewport on the fixture display");
    checkFooterGeometry(editor,checks,"S button-selected viewport");
    std::vector<juce::Rectangle<float>> headerBounds;
    bool visible=true,distinct=true;
    for(const auto* id:{"sizeS","sizeM","sizeL","maximise","factoryPreset","savePreset","loadPreset"}) {
        auto* component=findComponent(editor,id);
        visible=visible&&fitsVisibleAncestors(component,editor);
        if(component!=nullptr) {
            const auto bounds=editor.getLocalArea(component,component->getLocalBounds().toFloat());
            for(const auto& previous:headerBounds)distinct=distinct&&!previous.intersects(bounds);
            headerBounds.push_back(bounds);
        }
    }
    checks.expect(visible&&distinct,"S keeps size, maximise and preset actions visible and separate");
    const auto snapshot=editor.createComponentSnapshot(editor.getLocalBounds(),true,1.0f);
    checks.expect(writePNG(snapshot,directory.getChildFile("init_small.png")),"button-selected S viewport PNG export");
    for(const auto* mode:{"S","M"}) {
        if(juce::String(mode)=="S")small->onClick();else medium->onClick();
        settleEditor();
        const auto preceding=juce::Point<int>(editor.getWidth(),editor.getHeight());
        if(juce::String(mode)=="S")large->onClick();else maximise->onClick();
        settleEditor();
        checks.expect(editor.getWidth()>=preceding.x&&editor.getHeight()>=preceding.y,
                      juce::String(mode)+" maximise requests a larger viewport");
        checkFooterGeometry(editor,checks,juce::String(mode)+" to L viewport");
        maximise->onClick();settleEditor();
        checks.expect(preceding==juce::Point<int>(editor.getWidth(),editor.getHeight()),
                      juce::String(mode)+" viewport dimensions restore exactly after maximise");
    }
    medium->onClick();settleEditor();
}

void renderBenderPositions(SH09Processor& processor, juce::Component& editor,
                          const juce::File& directory, Checks& checks) {
    auto* bender = dynamic_cast<juce::Slider*>(findComponent(editor, "bender"));
    checks.expect(bender != nullptr, "bender is available for position snapshot evidence");
    if (bender == nullptr) return;
    const float initialAge=processor.state.getRawParameterValue("age")->load();
    const double initialBend=bender->getValue();
    checks.expect(bender->getTextFromValue(-1)=="-100.0%"&&bender->getTextFromValue(0)=="0.0%"&&bender->getTextFromValue(1)=="+100.0%",
                  "bender readout reports signed position independently of depth");
    const auto now=juce::Time::getCurrentTime();
    const auto point=bender->getLocalBounds().getCentre().toFloat();
    const juce::MouseEvent down(juce::Desktop::getInstance().getMainMouseSource(),point,
        juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,bender,bender,now,point,now,1,false);
    bender->mouseDown(down);settleEditor();
    checks.expect(bender->getCurrentPopupDisplay()!=nullptr,"bender mouse gesture opens the actual value popup");
    bender->mouseUp(down);settleEditor();
    const std::array<const char*, 3> names {"left", "neutral", "right"};
    for(const auto age:{0.0f,.91f,.92f}) {
        setParameter(processor,"age",age);settleEditor();
        std::array<juce::Image,3> positions;
        const auto prefix=age==0.0f?juce::String("bender_"):"bender_age"+juce::String(juce::roundToInt(age*100))+"_";
        for (size_t i = 0; i < positions.size(); ++i) {
            bender->setValue(double(i) - 1.0, juce::sendNotificationSync);
            positions[i] = bender->createComponentSnapshot(bender->getLocalBounds(), true, 3.0f);
            const auto file = directory.getChildFile(prefix + names[i] + ".png");
            checks.expect(writePNG(positions[i], file), prefix + names[i] + " actual-render PNG export");
            std::cout << "IMAGE " << file.getFullPathName() << '\n';
        }
        checks.expect(changedPixels(positions[0], positions[1], 12) >= 100
                      && changedPixels(positions[1], positions[2], 12) >= 100
                      && changedPixels(positions[0], positions[2], 12) >= 100,
                      prefix + "left, neutral and right positions visibly differ");
        const bool springCallback=bool(bender->onDragEnd);
        if(springCallback)bender->onDragEnd();
        const auto restored=bender->createComponentSnapshot(bender->getLocalBounds(),true,3.0f);
        checks.expect(springCallback&&std::abs(bender->getValue())<1.0e-12&&changedPixels(positions[1],restored)==0,
                      prefix + "actual drag-end callback restores zero and identical neutral pixels");
    }
    const auto capture=[&](float age) {
        setParameter(processor,"age",age);settleEditor();
        return bender->createComponentSnapshot(bender->getLocalBounds(),true,3.0f);
    };
    // Isolate the bender so moving lamps/keys and other Age events cannot pass
    // the break test. Adjacent samples distinguish an abrupt break from patina.
    const auto below=capture(.9199f),at=capture(.92f),above=capture(.9201f);
    checks.expect(changedPixels(below,at,12)>=100,
                  "bender tip visibly breaks at Age .92 in actual renderer");
    checks.expect(changedPixels(at,above,12)==0,
                  "bender break is discrete; adjacent post-onset Age adds no visible damage event");
    checks.expect(changedPixels(below,capture(.9199f))==0,
                  "lowering Age below .92 restores the identical intact bender tip");
    setParameter(processor,"age",initialAge);bender->setValue(initialBend,juce::sendNotificationSync);settleEditor();
}

void renderHardwareDetails(SH09Processor& processor,juce::Component& editor,
                           const juce::File& directory,Checks& checks) {
    auto* power=dynamic_cast<juce::Button*>(findComponent(editor,"power"));
    if(power==nullptr||power->getParentComponent()==nullptr||!power->onClick) {
        checks.expect(false,"POWER is available for on/off appearance evidence");return;
    }
    auto* panel=power->getParentComponent();
    const bool originalPower=processor.isPowerEnabled();
    const float originalAge=processor.state.getRawParameterValue("age")->load();
    for(const float age:{0.0f,.70f}) {
        setParameter(processor,"age",age);settleEditor();
        const auto suffix=age==0?juce::String("age0"):juce::String("age7");
        for(const bool enabled:{true,false}) {
            if(processor.isPowerEnabled()!=enabled)power->onClick();
            settleEditor();
            const auto detail=panel->createComponentSnapshot({170,432,75,91},true,4.0f);
            checks.expect(processor.isPowerEnabled()==enabled&&writePNG(detail,directory.getChildFile(
                "power_"+suffix+(enabled?"_on.png":"_off.png"))),"POWER "+suffix+(enabled?" ON":" OFF")+" actual-panel export and click state");
        }
        if(!processor.isPowerEnabled())power->onClick();
        settleEditor();
        checks.expect(writePNG(panel->createComponentSnapshot({310,130,280,225},true,3.0f),
            directory.getChildFile("knobs_"+suffix+".png")),"knobs "+suffix+" actual-panel detail export");
        checks.expect(writePNG(panel->createComponentSnapshot({65,420,183,320},true,3.0f),
            directory.getChildFile("performance_"+suffix+".png")),"performance controls "+suffix+" actual-panel detail export");
    }
    if(processor.isPowerEnabled()!=originalPower)power->onClick();
    setParameter(processor,"age",originalAge);settleEditor();
}

void checkBenderAudioCallbacks(SH09Processor& processor,juce::Component& editor,Checks& checks) {
    auto* bender=dynamic_cast<juce::Slider*>(findComponent(editor,"bender"));
    checks.expect(bender!=nullptr&&bool(bender->onValueChange)&&bool(bender->onDragEnd),
                  "bender retains value-change and spring-return callbacks");
    if(bender==nullptr||!bender->onValueChange||!bender->onDragEnd)return;
    juce::MemoryBlock savedState;processor.getStateInformation(savedState);
    const auto originalValue=bender->getValue();
    processor.setCurrentProgram(0);
    setParameter(processor,"wave",2);setParameter(processor,"vcaMode",2);
    setParameter(processor,"vcoLevel",.8f);setParameter(processor,"subLevel",0);
    setParameter(processor,"cutoff",1);setParameter(processor,"resonance",0);
    setParameter(processor,"vcfEnv",0);setParameter(processor,"bendVco",1);setParameter(processor,"bendVcf",0);
    const auto frequency=[&] {
        juce::AudioBuffer<float> audio(2,blockSize);juce::MidiBuffer midi;
        bool below=false;int cycles=0;float peak=0;
        constexpr int settlingBlocks=32,measuredBlocks=96;
        for(int block=0;block<settlingBlocks+measuredBlocks;++block) {
            audio.clear();processor.processBlock(audio,midi);
            if(block<settlingBlocks)continue;
            for(int sample=0;sample<blockSize;++sample) {
                const float value=audio.getSample(0,sample);peak=std::max(peak,std::abs(value));
                // Hysteresis rejects small near-zero noise/ringing crossings.
                if(value<-.005f)below=true;
                else if(value>.005f&&below){++cycles;below=false;}
            }
        }
        return peak>.01f?double(cycles)*sampleRate/double(measuredBlocks*blockSize):0.0;
    };
    for(const auto age:{.91f,.92f}) {
        setParameter(processor,"age",age);processor.reset();
        bender->setValue(0,juce::sendNotificationSync);bender->onDragEnd();
        processMidi(processor,juce::MidiMessage::noteOn(1,69,juce::uint8(100)));
        const auto neutral=frequency();
        bender->setValue(1,juce::sendNotificationSync);const auto high=frequency();
        bender->setValue(-1,juce::sendNotificationSync);const auto low=frequency();
        bender->onDragEnd();const auto returned=frequency();
        const auto context="bender Age "+juce::String(age,2);
        checks.expect(neutral>400&&neutral<480&&high>neutral*1.9&&high<neutral*2.1
                      &&low>neutral*.46&&low<neutral*.54,
                      context+" intact/broken visual state still drives opposing octave bends in real audio");
        checks.expect(std::abs(bender->getValue())<1.0e-12&&std::abs(returned-neutral)<neutral*.025,
                      context+" drag-end callback returns the rendered audio to neutral pitch");
        processMidi(processor,juce::MidiMessage::allNotesOff(1));
    }
    processor.setStateInformation(savedState.getData(),int(savedState.getSize()));
    processor.reset();bender->setValue(originalValue,juce::sendNotificationSync);settleEditor();
}

void checkOptionalDescriptions(juce::Component& editor,Checks& checks) {
    auto* button=dynamic_cast<juce::TextButton*>(findComponent(editor,"descriptions"));
    checks.expect(button!=nullptr&&!button->getToggleState(),"DESCRIPTION is available and starts off");
    if(button==nullptr)return;
    button->setToggleState(true,juce::sendNotificationSync);
    checks.expect(button->getToggleState(),"DESCRIPTION can enable optional hover help");
    checks.expect(button->getButtonText()=="DESCRIPTION ON", "DESCRIPTION has an explicit enabled indication");
    checks.expect(button->findColour(juce::TextButton::buttonOnColourId)!=button->findColour(juce::TextButton::buttonColourId), "DESCRIPTION enabled colour differs from off");
    button->setToggleState(false,juce::sendNotificationSync);
    checks.expect(!button->getToggleState(),"DESCRIPTION can disable hover help again");
    auto* age=dynamic_cast<juce::Slider*>(findComponent(editor,"age"));
    auto* bender=dynamic_cast<juce::Slider*>(findComponent(editor,"bender"));
    if(age==nullptr||bender==nullptr)return;
    checks.expect(age->getHeight()>=30,"AGE has an enlarged grab area");
    checks.expect(age->getTooltip().contains("release"),"AGE help explains live sound and deferred material update");
    auto* cutoff=dynamic_cast<juce::Slider*>(findComponent(editor,"cutoff"));
    checks.expect(cutoff!=nullptr&&cutoff->getTooltip().contains("harmonics"),"Parameter hover help explains its function");
    const auto render=[&]{return bender->createComponentSnapshot(bender->getLocalBounds(),true,1.0f);};
    double start=juce::Time::getMillisecondCounterHiRes();
    auto before=render();const auto fullMs=juce::Time::getMillisecondCounterHiRes()-start;
    age->onDragStart();start=juce::Time::getMillisecondCounterHiRes();
    auto coldFrame=age->getParentComponent()->createComponentSnapshot(age->getParentComponent()->getLocalBounds(),true,.75f);
    const auto coldMs=juce::Time::getMillisecondCounterHiRes()-start;
    checks.expect(coldFrame.isValid()&&coldMs<60,"AGE remains responsive before preview caches are ready");
    start=juce::Time::getMillisecondCounterHiRes();auto preview=render();
    const auto previewMs=juce::Time::getMillisecondCounterHiRes()-start;
    age->onDragEnd();auto after=render();
    bool same=before.getBounds()==after.getBounds();
    for(int y=0;y<before.getHeight()&&same;++y)for(int x=0;x<before.getWidth()&&same;++x)
        same=before.getPixelAt(x,y)==after.getPixelAt(x,y);
    checks.expect(same,"AGE drag restores the exact full-detail material after release");
    std::cout<<"AGE bender material full_ms="<<fullMs<<" live_preview_ms="<<previewMs<<"\n";
    auto* panel=age->getParentComponent();
    panel->getProperties().set("renderConditionPreviews",true);
    const double deadline=juce::Time::getMillisecondCounterHiRes()+2500;
    while(!bool(panel->getProperties().getWithDefault("conditionPreviewReady",false))&&juce::Time::getMillisecondCounterHiRes()<deadline)
        juce::MessageManager::getInstance()->runDispatchLoopUntil(20);
    checks.expect(bool(panel->getProperties().getWithDefault("conditionPreviewReady",false)),"AGE complete material frames warm during idle");
    const auto original=age->getValue();
    age->onDragStart();
    checks.expect(!bender->isVisible(),"AGE drag uses cached hardware rather than expensive live child paints");
    start=juce::Time::getMillisecondCounterHiRes();
    age->setValue(.81,juce::sendNotificationSync);
    auto live=panel->createComponentSnapshot(panel->getLocalBounds(),true,.75f);
    const auto frameMs=juce::Time::getMillisecondCounterHiRes()-start;
    checks.expect(std::abs(age->getValue()-.81)<.00001&&age->isVisible()&&live.isValid(),"AGE thumb and parameter update while gesture remains held");
    checks.expect(frameMs<60,"AGE cached instrument preview renders within 60 ms");
    age->onDragEnd();
    checks.expect(bender->isVisible(),"AGE release restores all live hardware controls");
    age->setValue(original,juce::sendNotificationSync);
    panel->getProperties().set("renderConditionPreviews",false);
    std::cout<<"AGE complete cached preview_ms="<<frameMs<<"\n";

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
    component->keyPressed(juce::KeyPress(juce::KeyPress::homeKey));
    juce::BubbleMessageComponent* bubble=nullptr;
    for(auto* child:component->getParentComponent()->getChildren())
        if(auto* candidate=dynamic_cast<juce::BubbleMessageComponent*>(child))bubble=candidate;
    checks.expect(bubble!=nullptr && bubble->isVisible() && bubble->getName()=="LFO",
                  "switch value bubble shows the selected LFO mode");
    if(bubble!=nullptr) {
        const auto image=bubble->createComponentSnapshot(bubble->getLocalBounds(),true,2.0f);
        int darkPixels=0;
        // Inspect the centre, away from the border and pointer: text must render.
        for(int y=image.getHeight()/3;y<image.getHeight()*2/3;++y)
            for(int x=image.getWidth()/4;x<image.getWidth()*3/4;++x)
                if(image.getPixelAt(x,y).getAlpha()>128 && image.getPixelAt(x,y).getBrightness()<.35f)++darkPixels;
        checks.expect(darkPixels>15,"switch bubble contains readable dark text on its pale background");
        component->keyPressed(juce::KeyPress(juce::KeyPress::endKey));
        checks.expect(bubble->getName()=="ENV","switch value bubble updates to the selected ENV mode");
        bubble->setVisible(false);
    }
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

struct Fixture {
    const char* name;
    int program;
    bool modern;
    float age;
    bool heldNote;
    bool external;
};

void renderFixture(const Fixture& fixture, const juce::File& directory, Checks& checks) {
    SH09Processor processor;
    processor.setCurrentProgram(0);
    processor.prepareToPlay(sampleRate, blockSize);
    std::unique_ptr<juce::AudioProcessorEditor, EditorDeleter> editor(processor.createEditorAndMakeActive(), {&processor});
    if (editor == nullptr) throw std::runtime_error("Processor did not create an editor");
    editor->setVisible(true);
    settleEditor();
    if (fixture.program == 0) {
        checkSizeChoices(*editor,directory,checks);
        checkSwitchAttachment(processor, *editor, checks);
        checkOptionalDescriptions(*editor,checks);
        checkKeyboardFeedback(processor, *editor, directory, checks);
        checkOpaqueCaps(*editor, checks);
        checkFineGestures(processor,*editor,checks);
        checkRotaryTicks(processor,*editor,checks);
        checkKnobMetalAging(processor,*editor,directory,checks);
        checkCapLoss(processor,*editor,checks);
        checkValueBubbleColours(*editor, checks);
        renderBenderPositions(processor, *editor, directory, checks);
        renderHardwareDetails(processor,*editor,directory,checks);
        checkBenderAudioCallbacks(processor,*editor,checks);
        checkAgeEvents(processor, *editor, checks);
    }

    // Change the patch after creating the editor to exercise attachment redraw.
    processor.setCurrentProgram(fixture.program);
    setParameter(processor,"age",fixture.age);
    if (fixture.modern) {
        setParameter(processor, "modern", 1.0f);
        setParameter(processor, "velocity", 0.8f);
        setParameter(processor, "aftertouch", 0.6f);
        setParameter(processor, "cutoff", 0.43f);
        setParameter(processor, "resonance", 0.6f);
    }
    advanceAudio(processor, fixture.heldNote, fixture.external);
    settleEditor();
    bool anyHeld=false;
    for(const auto& key:processor.keyDisplay)anyHeld=anyHeld||key.load()!=0;
    checks.expect(anyHeld==fixture.heldNote,juce::String(fixture.name)+" matches its explicit held-key fixture state");

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
                     "plus an unheld personal_age7 preview, a button-selected S viewport, and bender positions at Ages 0/.91/.92;\n"
                     "checks discrete bender breakage, spring-return audio and wide/tall footer geometry.\n";
        return argc == 2 ? 0 : 2;
    }
    juce::ScopedJuceInitialiser_GUI initialiseGUI;
    Checks checks;
    const auto directory = juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1]));
    try {
        if (auto result = directory.createDirectory(); result.failed()) throw std::runtime_error(result.getErrorMessage().toStdString());
        checks.expect(juce::MessageManager::getInstance()->isThisTheMessageThread(), "fixture runs on the JUCE message thread");
        checkPersonalArtwork(checks);
        checkCompatibility(checks);
        {
            juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
            SH09Processor standalone;
            juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
            std::unique_ptr<juce::AudioProcessorEditor, EditorDeleter> editor(standalone.createEditorAndMakeActive(), {&standalone});
            auto* left=findComponent(*editor,"updates");
            auto* right=findComponent(*editor,"sizeL");
            checks.expect(left&&right&&left->getParentComponent()==right->getParentComponent()
                &&left->getX()==25&&right->getParentComponent()->getWidth()-right->getRight()==25,
                "standalone top row matches the footer's left and right insets");
            auto* footerLeft=findComponent(*editor,"modern");
            auto* footerRight=findComponent(*editor,"externalAudioHelp");
            checks.expect(footerLeft&&footerRight&&footerLeft->getX()==25
                &&footerRight->getParentComponent()->getWidth()-footerRight->getRight()==25,
                "footer has the same symmetric 25-unit insets as the header");
            const auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,2.0f);
            checks.expect(writePNG(image,directory.getChildFile("standalone_header.png")),"actual standalone header export");
        }
        {
            kb09_gui::ClosePrompt prompt(nullptr);
            const auto selected=[&](int index){return prompt.getButton(index)->getToggleState();};
            checks.expect(selected(1),"close prompt starts on Don't Save");
            prompt.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
            checks.expect(selected(2),"Right includes Cancel in the close prompt");
            prompt.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
            checks.expect(selected(0),"Right wraps Cancel to Save");
            prompt.keyPressed(juce::KeyPress(juce::KeyPress::leftKey));
            checks.expect(selected(2),"Left wraps Save to Cancel");
            prompt.keyPressed(juce::KeyPress(juce::KeyPress::upKey));
            checks.expect(selected(1),"Up steps Cancel to Don't Save");
            prompt.keyPressed(juce::KeyPress(juce::KeyPress::downKey));
            checks.expect(selected(2),"Down steps Don't Save to Cancel");
            int answer=-1;
            prompt.enterModalState(false,juce::ModalCallbackFunction::create([&](int value){answer=value;}),false);
            prompt.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
            for(int attempt=0;answer<0&&attempt<16;++attempt)settleEditor();
            checks.expect(answer==0,"Enter activates highlighted Cancel");
        }
        for (const auto fixture : {Fixture{"init",0,false,0.0f,false,false},
                                   Fixture{"aged_modern",14,true,.82f,true,false},
                                   Fixture{"external_follower",12,false,0.0f,false,true},
                                   Fixture{"worn",14,true,1.0f,true,false},
                                   Fixture{"personal_age7",14,true,.7f,false,false},
                                   Fixture{"fully_aged",14,true,1.0f,false,false}})
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


