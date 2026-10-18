#include "Processor.h"
#include "StandaloneMidiInputs.h"
#include "DesktopShortcut.h"
#include "ChoiceSnapProbe.h"
#include "ResourceProbe.h"
#include "../GUI/DesktopShortcutPrompt.h"
#include "../GUI/StartupDialogReservation.h"
#include "../GUI/PresetFiles.h"
#include "../GUI/Editor.h"
#include "../GUI/ClosePrompt.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#if JUCE_WINDOWS
#include <windows.h>
#endif

namespace {
class InstrumentWindow final : public juce::StandaloneFilterWindow {
public:
    InstrumentWindow(const juce::String& title, juce::Colour colour,
                     std::unique_ptr<juce::StandalonePluginHolder> holder)
        : StandaloneFilterWindow(title, colour, std::move(holder)) {
        setUsingNativeTitleBar(true);
        setTitleBarButtonsRequired(juce::DocumentWindow::allButtons,false);
        if(auto* editor=dynamic_cast<SH09Editor*>(getAudioProcessor()->getActiveEditor()))
            editor->prepareStandaloneWindow();
    }
    bool keyPressed(const juce::KeyPress& key) override {
        if (key.getModifiers().isCommandDown() && !key.getModifiers().isAltDown()
            && (key.getKeyCode() == 'w' || key.getKeyCode() == 'W')) {
            closeButtonPressed(); return true;
        }
        return StandaloneFilterWindow::keyPressed(key);
    }
    void closeButtonPressed() override {
        if (auto* app = juce::JUCEApplicationBase::getInstance()) app->systemRequestedQuit();
    }
};

class InstrumentApp final : public juce::JUCEApplication, private juce::Timer {
public:
    const juce::String getApplicationName() override { return juce::String(JucePlugin_Name)+" Standalone"+(startupProbe()?" StartupProbe":""); }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return false; }
    void anotherInstanceStarted(const juce::String&) override { ++activationHandoffs;requestForeground(); }

    void initialise(const juce::String&) override {
        // JUCE's device-error alerts have no associated component. This default
        // applies only inside the standalone process, never inside a DAW.
        juce::LookAndFeel::setDefaultLookAndFeel(&windowTheme);
        windowTheme.conditionProvider=[this] {
            if(window)if(auto* p=dynamic_cast<SH09Processor*>(window->getAudioProcessor()))
                return p->state.getRawParameterValue("age")->load();
            return 0.f;
        };
        juce::PropertiesFile::Options options;
        // Retain audio/MIDI preferences from earlier standalone versions.
        options.applicationName = juce::String(JucePlugin_Name)+(startupProbe()?" StartupProbe":"");
        options.filenameSuffix = ".settings";
        options.osxLibrarySubFolder = "Application Support";
        const bool noteClickProbe=getCommandLineParameters().contains("--note-click-probe=");
        if(noteClickProbe)options.doNotSave=true;
        properties.setStorageParameters(options);
        auto* settings = properties.getUserSettings();
        if(noteClickProbe) {
            auto normal=options;normal.applicationName=juce::String(JucePlugin_Name);
            if(const auto xml=juce::XmlDocument::parse(normal.getDefaultFile()))
                if(const auto* audio=xml->getChildByAttribute("name","audioSetup"))
                    if(const auto* setup=audio->getFirstChildElement())settings->setValue("audioSetup",setup);
            const juce::XmlElement noMidi("KB09_MIDI_INPUTS");settings->setValue("standaloneMidiInputs",&noMidi);
            if(auto setup=settings->getXmlValue("audioSetup")) {setup->deleteAllChildElements();settings->setValue("audioSetup",setup.get());}
            settings->removeValue("explicitStartupPreset");
        }
        settings->removeValue("filterState"); // Never restore an unsaved session.
        auto holder = std::make_unique<juce::StandalonePluginHolder>(settings, false);
        holder->stopPlaying();
        auto* processor = dynamic_cast<SH09Processor*>(holder->processor.get());
        if (processor == nullptr) { quit(); return; }
        midiInputs=std::make_unique<sh09::StandaloneMidiInputs>(holder->deviceManager,*settings);
        if(processor->midiMap) {
            processor->midiMap->restore(settings->getValue("standaloneMidiMap"));
            processor->midiMap->onChanged=[this,processor] {
                properties.getUserSettings()->setValue("standaloneMidiMap",processor->midiMap->save());
                properties.saveIfNeeded();
            };
        }
        processor->setCurrentProgram(0);
        const auto startup = settings->getValue("explicitStartupPreset");
        if (startup.isNotEmpty()) {
            const auto result = sh09::PresetFiles::loadFromFile(*processor, juce::File(startup));
            if (result.failed()) settings->removeValue("explicitStartupPreset");
        }
#if KB09_LOCAL_MIDI_TEST
        const bool captureClicks=getCommandLineParameters().contains("--capture-clicks=");
        if(captureClicks)if(auto* device=holder->deviceManager.getCurrentAudioDevice())
            processor->noteClickCapture=std::make_unique<NoteClickCapture>(device->getCurrentSampleRate(),30);
#endif
        holder->startPlaying();
        window = std::make_unique<InstrumentWindow>(getApplicationName(), juce::Colour(0xff151a1d), std::move(holder));
        if(auto* editor=dynamic_cast<SH09Editor*>(processor->getActiveEditor()))
            editor->setAudioSettingsObserver([this](bool editing){if(midiInputs)midiInputs->setEditing(editing);});
        presets = std::make_unique<sh09::PresetFiles>(*processor, *window);
        presets->onCancelled = [this] { closing = false; };
        presets->onCompleted = [this](const juce::Result& result, const juce::File& file, bool loading) {
            if (!closing || loading) return;
            closing = false;
            if (result.failed()) return;
            properties.getUserSettings()->setValue("explicitStartupPreset", file.getFullPathName());
            properties.saveIfNeeded();
            quit();
        };
        window->setVisible(true);
        requestForeground();
#if KB09_LOCAL_MIDI_TEST
        if(captureClicks) {
            captureTimer.tick=[this,processor]{
                if(!processor->noteClickCapture||!processor->noteClickCapture->ready())return;
                captureTimer.stopTimer();
                const auto path=getCommandLineParameters().fromFirstOccurrenceOf("--capture-clicks=",false,false).trim().unquoted();
                const juce::File folder(path);processor->noteClickCapture->save(folder);
                if(window){juce::DynamicObject::Ptr device=new juce::DynamicObject;
                    device->setProperty("xruns",window->getDeviceManager().getXRunCount());
                    device->setProperty("audioCpuPercent",window->getDeviceManager().getCpuUsage()*100.);
                    folder.getChildFile("device.json").replaceWithText(juce::JSON::toString(juce::var(device.get())));}
            };captureTimer.startTimer(250);
        }
        if(getCommandLineParameters().contains("--audio-health-report=")) {
            const juce::File folder(getCommandLineParameters().fromFirstOccurrenceOf("--audio-health-report=",false,false).trim().unquoted());
            folder.createDirectory();
            const double started=juce::Time::getMillisecondCounterHiRes();
            healthTimer.tick=[this,folder,started] {
                if(!window)return;
                auto& manager=window->getDeviceManager();
                juce::DynamicObject::Ptr sample=new juce::DynamicObject;
                sample->setProperty("elapsedSeconds",(juce::Time::getMillisecondCounterHiRes()-started)*.001);
                sample->setProperty("recordedAt",juce::Time::getCurrentTime().toISO8601(true));
                sample->setProperty("audioCpuPercent",manager.getCpuUsage()*100.);
                sample->setProperty("xruns",manager.getXRunCount());
                sample->setProperty("deviceType",manager.getCurrentAudioDeviceType());
                if(auto* device=manager.getCurrentAudioDevice()) {
                    sample->setProperty("deviceName",device->getName());
                    sample->setProperty("sampleRate",device->getCurrentSampleRate());
                    sample->setProperty("bufferSamples",device->getCurrentBufferSizeSamples());
                }
                healthHistory.add(juce::var(sample.get()));
                if(healthHistory.size()>120)healthHistory.remove(0);
                juce::DynamicObject::Ptr report=new juce::DynamicObject;
                report->setProperty("recordsAudio",false);
                report->setProperty("recordsMidi",false);
                report->setProperty("samples",juce::var(healthHistory));
                folder.getChildFile("audio-health.json").replaceWithText(juce::JSON::toString(juce::var(report.get())));
            };
            healthTimer.tick();healthTimer.startTimer(2000);
        }
#endif
        if(startupProbe()) {
           #if KB09_LOCAL_MIDI_TEST
            if(noteClickProbe) {
                const auto path=getCommandLineParameters().fromFirstOccurrenceOf("--note-click-probe=",false,false).trim().unquoted();
                resourceProbe=std::make_unique<ResourceProbe>(*processor,window->getDeviceManager(),*window,juce::File(path),[this]{quit();},true);return;
            }
            if(getCommandLineParameters().contains("--resource-probe=")) {
                const auto path=getCommandLineParameters().fromFirstOccurrenceOf("--resource-probe=",false,false).trim().unquoted();
                resourceProbe=std::make_unique<ResourceProbe>(*processor,window->getDeviceManager(),*window,juce::File(path),[this]{quit();});return;
            }
            if(getCommandLineParameters().contains("--snap35-probe=")) {
                choiceProbe=std::make_unique<ChoiceSnapProbe>(*window,[this](juce::var frames) {
                    const auto path=getCommandLineParameters().fromFirstOccurrenceOf("--snap35-probe=",false,false).trim().unquoted();
                    juce::File(path).replaceWithText(juce::JSON::toString(frames));quit();
                });return;
            }
           #endif
            juce::Timer::callAfterDelay(900,[this] {
                if(juce::JUCEApplicationBase::getInstance()!=this)return;
                juce::DynamicObject::Ptr result=new juce::DynamicObject;
                result->setProperty("visible",window&&window->isShowing());
                result->setProperty("foreground",appIsForeground());result->setProperty("handoffs",activationHandoffs);
                result->setProperty("activationAttempts",activationAttempts);
                if(window) {
                    juce::Array<juce::var> backends;
                    for(auto* type:window->getDeviceManager().getAvailableDeviceTypes()) {
                        type->scanForDevices();
                        juce::DynamicObject::Ptr backend=new juce::DynamicObject;
                        backend->setProperty("name",type->getTypeName());
                        juce::Array<juce::var> outputs;
                        for(const auto& name:type->getDeviceNames(false))outputs.add(name);
                        backend->setProperty("outputs",juce::var(outputs));
                        backends.add(juce::var(backend.get()));
                    }
                    result->setProperty("audioBackends",juce::var(backends));
                }
                const auto path=getCommandLineParameters().fromFirstOccurrenceOf("--startup-probe=",false,false).trim().unquoted();
                juce::File(path).replaceWithText(juce::JSON::toString(juce::var(result.get())));quit();
            });
        } else if(!settings->getBoolValue("desktopShortcutDoNotAsk",false))startTimer(350);
    }

    void systemRequestedQuit() override {
        if (!window || !presets) { quit(); return; }
        if (closing || presets->isDialogOpen()) return;
        if(shortcutQuestion||shortcutNotice)return;
        closing = true;
        if(auto* processor=dynamic_cast<SH09Processor*>(window->getAudioProcessor())) {
            windowTheme.age=processor->state.getRawParameterValue("age")->load();
            windowTheme.refreshColours();
            window->setLookAndFeel(&windowTheme);
        }
        question = std::make_unique<kb09_gui::ClosePrompt>(window.get());
        question->enterModalState(true, juce::ModalCallbackFunction::create([this](int answer) {
                juce::MessageManager::callAsync([this, answer] {
                    question.reset();
                    if (answer == 1) presets->savePreset();
                    else if (answer == 2) {
                        properties.getUserSettings()->removeValue("explicitStartupPreset");
                        properties.saveIfNeeded(); quit();
                    } else closing = false;
                });
            }), false);
        question->grabKeyboardFocus();
    }

    void shutdown() override {
        activationTimer.stopTimer();captureTimer.stopTimer();healthTimer.stopTimer();stopTimer();shortcutQuestion.reset();shortcutNotice.reset();
        question.reset(); presets.reset(); midiInputs.reset(); window.reset();
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
        properties.getUserSettings()->removeValue("filterState");
        properties.saveIfNeeded();
    }

private:
   #if KB09_LOCAL_MIDI_TEST
    std::unique_ptr<ChoiceSnapProbe> choiceProbe;
    std::unique_ptr<ResourceProbe> resourceProbe;
    juce::Array<juce::var> healthHistory;
   #endif
    bool startupProbe() const {
       #if KB09_LOCAL_MIDI_TEST
        return getCommandLineParameters().contains("--startup-probe=")||getCommandLineParameters().contains("--snap35-probe=")||getCommandLineParameters().contains("--resource-probe=")||getCommandLineParameters().contains("--note-click-probe=");
       #else
        return false;
       #endif
    }
    struct ActivationTimer final:juce::Timer {
        std::function<void()> tick;
        void timerCallback() override {if(tick)tick();}
    } activationTimer,captureTimer,healthTimer;
    bool appIsForeground() const {
       #if JUCE_WINDOWS
        DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);return pid==GetCurrentProcessId();
       #else
        return window&&window->isActiveWindow();
       #endif
    }
    void requestForeground() {
       #if JUCE_WINDOWS
        activationAnchor=GetForegroundWindow();
       #endif
        activationRetries=3;foreground();
        if(!window)return; // Initialisation will make its own request when ready.
        activationTimer.tick=[this] {
            if(!window||appIsForeground()){activationTimer.stopTimer();return;}
           #if JUCE_WINDOWS
            // A user who changed applications during startup keeps their focus.
            if(GetForegroundWindow()!=activationAnchor){activationTimer.stopTimer();return;}
           #endif
            foreground();if(--activationRetries<=0)activationTimer.stopTimer();
        };
        // Try again after the peer has painted and pending activation messages
        // have run. A launch is no longer dependent on one synchronous attempt.
        activationTimer.startTimer(100);
    }
    void timerCallback() override {
        if(!window||closing){stopTimer();return;}
        auto reservation=kb09_gui::StartupDialogReservation::tryAcquire();
        if(!reservation)return;
        stopTimer();
        shortcutQuestion=std::make_unique<kb09_gui::DesktopShortcutPrompt>(window.get());
        shortcutQuestion->enterModalState(true,juce::ModalCallbackFunction::create([this,reservation](int answer) {
            const bool suppress=shortcutQuestion&&shortcutQuestion->shouldStopAsking();
            juce::MessageManager::callAsync([this,answer,suppress,reservation] {
                shortcutQuestion.reset();if(!window)return;
                auto* settings=properties.getUserSettings();
                if(suppress)settings->setValue("desktopShortcutDoNotAsk",true);
                if(answer==1) {
                    const auto result=sh09::createDesktopShortcut(juce::File::getSpecialLocation(juce::File::currentApplicationFile),
                        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),JucePlugin_Name);
                    if(result.wasOk())settings->setValue("desktopShortcutDoNotAsk",true);
                    else {
                        shortcutNotice=std::make_unique<kb09_gui::ThemedAlert>("Desktop shortcut",result.getErrorMessage(),juce::MessageBoxIconType::InfoIcon,window.get());
                        shortcutNotice->addButton("OK",0,juce::KeyPress(juce::KeyPress::returnKey),juce::KeyPress(juce::KeyPress::escapeKey));
                        shortcutNotice->enterModalState(true,juce::ModalCallbackFunction::create([this](int){juce::MessageManager::callAsync([this]{shortcutNotice.reset();});}),false);
                    }
                }
                properties.saveIfNeeded();
            });
        }),false);
        shortcutQuestion->grabKeyboardFocus();
    }
    void foreground() {
        if (!window) return;
        ++activationAttempts;
        juce::Process::makeForegroundProcess();
        window->setVisible(true);window->setMinimised(false);
        auto* target=static_cast<juce::Component*>(window.get());
        if(auto* modal=juce::Component::getCurrentlyModalComponent())target=modal->getTopLevelComponent();
       #if JUCE_WINDOWS
        target->toFront(true);
        if (auto* peer = target->getPeer()) {
            const auto hwnd = static_cast<HWND>(peer->getNativeHandle());
            if (hwnd != nullptr) {
                if(IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
                const auto previous = GetForegroundWindow();
                const auto foregroundThread = previous != nullptr ? GetWindowThreadProcessId(previous, nullptr) : 0;
                const auto ownThread = GetCurrentThreadId();
                const bool attached = foregroundThread != 0 && foregroundThread != ownThread
                    && AttachThreadInput(ownThread, foregroundThread, TRUE) != 0;
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                BringWindowToTop(hwnd);
                SetForegroundWindow(hwnd);
                SetActiveWindow(hwnd);
                SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                if (attached) AttachThreadInput(ownThread, foregroundThread, FALSE);
            }
        }
       #else
        target->toFront(true);
       #endif
        target->grabKeyboardFocus();
    }
    juce::ApplicationProperties properties;
    std::unique_ptr<sh09::StandaloneMidiInputs> midiInputs;
    std::unique_ptr<InstrumentWindow> window;
    std::unique_ptr<sh09::PresetFiles> presets;
    std::unique_ptr<kb09_gui::ClosePrompt> question;
    std::unique_ptr<kb09_gui::DesktopShortcutPrompt> shortcutQuestion;
    std::unique_ptr<kb09_gui::ThemedAlert> shortcutNotice;
    kb09_gui::DialogTheme windowTheme;
    int activationRetries=0,activationAttempts=0,activationHandoffs=0;
   #if JUCE_WINDOWS
    HWND activationAnchor=nullptr;
   #endif
    bool closing = false;
};
}
JUCE_CREATE_APPLICATION_DEFINE(InstrumentApp)
