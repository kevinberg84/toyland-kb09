#include "Processor.h"
#include "../GUI/PresetFiles.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace {
class InstrumentWindow final : public juce::StandaloneFilterWindow {
public:
    using StandaloneFilterWindow::StandaloneFilterWindow;
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
    const juce::String getApplicationName() override { return "Toyland KB-09 Standalone"; }
    const juce::String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return false; }
    void anotherInstanceStarted(const juce::String&) override { foreground(); }

    void initialise(const juce::String&) override {
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
        processor->setCurrentProgram(0);
        const auto startup = settings->getValue("explicitStartupPreset");
        if (startup.isNotEmpty()) {
            const auto result = sh09::PresetFiles::loadFromFile(*processor, juce::File(startup));
            if (result.failed()) settings->removeValue("explicitStartupPreset");
        }
        holder->startPlaying();
        window = std::make_unique<InstrumentWindow>(getApplicationName(), juce::Colour(0xff151a1d), std::move(holder));
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
        // The editor installs a native title bar asynchronously, recreating its peer.
        juce::Timer::callAfterDelay(150, [] {
            if (auto* app = dynamic_cast<InstrumentApp*>(juce::JUCEApplicationBase::getInstance())) app->foreground();
        });
    }

    void systemRequestedQuit() override {
        if (!window || !presets) { quit(); return; }
        if (closing || presets->isDialogOpen()) return;
        closing = true;
        question = juce::AlertWindow::showScopedAsync(
            juce::MessageBoxOptions{}.withIconType(juce::MessageBoxIconType::QuestionIcon)
                .withTitle("Save your KB-09 patch before closing?")
                .withMessage("Save opens this patch next time. Don't Save starts with Calibrated Init next time.")
                .withButton("Save").withButton("Don't Save").withButton("Cancel")
                .withAssociatedComponent(window.get()),
            [this](int answer) {
                juce::MessageManager::callAsync([this, answer] {
                    question.close();
                    if (answer == 1) presets->savePreset();
                    else if (answer == 2) {
                        properties.getUserSettings()->removeValue("explicitStartupPreset");
                        properties.saveIfNeeded(); quit();
                    } else closing = false;
                });
            });
    }

    void shutdown() override {
        question.close(); presets.reset(); window.reset();
        properties.getUserSettings()->removeValue("filterState");
        properties.saveIfNeeded();
    }

private:
    void foreground() {
        if (!window) return;
        juce::Process::makeForegroundProcess();
        window->setMinimised(false);
       #if JUCE_WINDOWS
        // A newly recreated native peer can lose Windows' foreground allowance.
        // Raise once without leaving the instrument permanently above other apps.
        window->setAlwaysOnTop(true);
        window->toFront(true);
        window->setAlwaysOnTop(false);
       #else
        window->toFront(true);
       #endif
        window->grabKeyboardFocus();
    }
    juce::ApplicationProperties properties;
    std::unique_ptr<InstrumentWindow> window;
    std::unique_ptr<sh09::PresetFiles> presets;
    juce::ScopedMessageBox question;
    bool closing = false;
};
}
JUCE_CREATE_APPLICATION_DEFINE(InstrumentApp)
