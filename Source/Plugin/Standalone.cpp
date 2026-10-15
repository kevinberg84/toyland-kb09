#include "Processor.h"
#include "StandaloneMidiInputs.h"
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

class InstrumentApp final : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return juce::String(JucePlugin_Name)+" Standalone"; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return false; }
    void anotherInstanceStarted(const juce::String&) override { foreground(); }

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
        options.applicationName = JucePlugin_Name;
        options.filenameSuffix = ".settings";
        options.osxLibrarySubFolder = "Application Support";
        properties.setStorageParameters(options);
        auto* settings = properties.getUserSettings();
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
        foreground();
    }

    void systemRequestedQuit() override {
        if (!window || !presets) { quit(); return; }
        if (closing || presets->isDialogOpen()) return;
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
        question.reset(); presets.reset(); midiInputs.reset(); window.reset();
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
        properties.getUserSettings()->removeValue("filterState");
        properties.saveIfNeeded();
    }

private:
    void foreground() {
        if (!window) return;
        juce::Process::makeForegroundProcess();
        window->setMinimised(false);
       #if JUCE_WINDOWS
        window->toFront(true);
        if (auto* peer = window->getPeer()) {
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
        window->toFront(true);
       #endif
        window->grabKeyboardFocus();
    }
    juce::ApplicationProperties properties;
    std::unique_ptr<sh09::StandaloneMidiInputs> midiInputs;
    std::unique_ptr<InstrumentWindow> window;
    std::unique_ptr<sh09::PresetFiles> presets;
    std::unique_ptr<kb09_gui::ClosePrompt> question;
    kb09_gui::DialogTheme windowTheme;
    bool closing = false;
};
}
JUCE_CREATE_APPLICATION_DEFINE(InstrumentApp)
