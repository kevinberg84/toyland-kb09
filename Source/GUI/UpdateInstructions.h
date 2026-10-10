#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace kb09 {
#if JUCE_MAC
inline constexpr bool macUpdateInstructions = true;
#else
inline constexpr bool macUpdateInstructions = false;
#endif
inline juce::String updateInstructions(bool mac) {
    juce::String text =
        "Installation is manual and usually takes a few minutes, depending on your download speed. "
        "Installing the update is optional.\n\n"
        "Your saved presets and audio/MIDI preferences are kept. Save your current patch before closing KB-09. "
        "Keep any personal presets you saved inside the old download folder.\n\n"
        "1. Open the download folder and download the ZIP for your computer.\n"
        "2. Extract the entire ZIP. Save your patch and close KB-09.\n";
    text += mac
        ? "3. For standalone only: replace your old extracted KB-09 folder with the new one, then open Toyland KB-09 Standalone. "
          "Alternatively, run INSTALL KB-09.command to install the standalone in your Applications folder; it offers a desktop shortcut.\n"
        : "3. For standalone only: replace your old extracted KB-09 folder with the new one, then open Toyland KB-09 Standalone.exe. "
          "Alternatively, run INSTALL KB-09.cmd to install it and optionally create a desktop shortcut.\n";
    text +=
        "4. If you use plug-ins: close your DAW before running the installer. Reopen it afterward and rescan plug-ins if needed.\n"
        "5. Open TOYLAND / KB-09 and confirm the new version number. Use the new copy or installed shortcut from now on.\n\n"
        "Copy these instructions before closing KB-09 so you can keep them beside your download. "
        "No files are downloaded or installed until you take those steps.";
    return text;
}

// Uses the same AlertWindow styling as the other KB-09 dialogs. The two
// custom buttons deliberately leave instructions open while copying/browsing.
class UpdateInstructions final : public juce::AlertWindow {
public:
    explicit UpdateInstructions(const juce::String& link)
        : AlertWindow("How to update KB-09", updateInstructions(macUpdateInstructions),
                      juce::MessageBoxIconType::InfoIcon), downloadLink(link) {
        controls.setSize(450, 36);
        controls.addAndMakeVisible(download);
        controls.addAndMakeVisible(copy);
        download.setButtonText("Open download folder");
        copy.setButtonText("Copy instructions");
        download.setBounds(0, 2, 240, 30);
        copy.setBounds(250, 2, 200, 30);
        download.onClick = [this] { juce::URL(downloadLink).launchInDefaultBrowser(); };
        copy.onClick = [this] {
            juce::SystemClipboard::copyTextToClipboard(updateInstructions(macUpdateInstructions)
                                                       + "\n\nDownload: " + downloadLink);
            copy.setButtonText("Copied!");
        };
        addCustomComponent(&controls);
        addButton("Close", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    }
    ~UpdateInstructions() override { removeCustomComponent(0); }
    static void show(const juce::String& link) {
        auto* dialog = new UpdateInstructions(link);
        dialog->enterModalState(true, nullptr, true);
    }
private:
    juce::String downloadLink;
    juce::Component controls;
    juce::TextButton download, copy;
};
}
