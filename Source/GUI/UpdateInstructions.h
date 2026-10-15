#pragma once
#include "DialogTheme.h"

namespace kb09 {
#if JUCE_MAC
inline constexpr bool macUpdateInstructions = true;
#else
inline constexpr bool macUpdateInstructions = false;
#endif
#if JUCE_LINUX
inline constexpr bool linuxUpdateInstructions = true;
#else
inline constexpr bool linuxUpdateInstructions = false;
#endif
inline juce::String updateInstructions(bool mac, bool isLinux = false) {
    if(isLinux)return
        "Save your current patch before closing KB-09. Your saved presets and audio/MIDI preferences stay in your user account. "
        "Keep any personal presets you saved inside the old download folder.\n\n"
        "1. Download Toyland KB-09 for Linux (Standalone Only).zip and extract the entire ZIP.\n"
        "2. Close KB-09, then replace the old extracted files with the new ones.\n"
        "3. Run Start KB-09.sh, or mark Toyland KB-09 Standalone executable and open it.\n"
        "4. If you used Create desktop shortcut.sh, run it again after moving the folder.\n"
        "5. Open TOYLAND / KB-09 and confirm the new version.\n\n"
        "No DAW or plug-in installation is needed. No files are downloaded or installed automatically.";
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
class UpdateInstructions final : public kb09_gui::ThemedAlert {
public:
    explicit UpdateInstructions(const juce::String& link, juce::Component* owner = nullptr, bool mac = macUpdateInstructions, float age = -1)
        : ThemedAlert("How to update KB-09", updateInstructions(mac,linuxUpdateInstructions),
                      juce::MessageBoxIconType::InfoIcon, owner, age), downloadLink(link) {
        controls.setSize(450, 36);
        controls.addAndMakeVisible(download);
        controls.addAndMakeVisible(copy);
        download.setButtonText("Open download folder");
        copy.setButtonText("Copy instructions");
        download.setBounds(0, 2, 240, 30);
        copy.setBounds(250, 2, 200, 30);
        download.onClick = [this] { juce::URL(downloadLink).launchInDefaultBrowser(); };
        copy.onClick = [this,mac] {
            juce::SystemClipboard::copyTextToClipboard(updateInstructions(mac,linuxUpdateInstructions)
                                                       + "\n\nDownload: " + downloadLink);
            copy.setButtonText("Copied!");
        };
        addCustomComponent(&controls);
        addButton("Close", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        setNavigationActions({&download,&copy,dynamic_cast<juce::TextButton*>(getButton(0))});
        selectAction(0);
    }
    ~UpdateInstructions() override { removeCustomComponent(0); }
    static void show(const juce::String& link, juce::Component* owner = nullptr) {
        auto* dialog = new UpdateInstructions(link, owner);
        dialog->enterModalState(true, nullptr, true);
    }
private:
    juce::String downloadLink;
    juce::Component controls;
    juce::TextButton download, copy;
};
}
