#pragma once
#include "DialogTheme.h"
namespace kb09_gui {
class DesktopShortcutPrompt final : public ThemedAlert {
public:
    explicit DesktopShortcutPrompt(juce::Component* owner)
        : ThemedAlert("Create a desktop shortcut?",
            "Open KB-09 from your desktop without finding its folder. Keep the application in its current location so the shortcut keeps working.",
            juce::MessageBoxIconType::QuestionIcon,owner) {
        dontAsk.setButtonText("Don't ask again");dontAsk.setSize(360,30);
        addCustomComponent(&dontAsk);
        addButton("Create shortcut",1);
        addButton("Not now",0,juce::KeyPress(juce::KeyPress::escapeKey));
        selectAction(1);setWantsKeyboardFocus(true);
    }
    ~DesktopShortcutPrompt() override {removeCustomComponent(0);}
    bool shouldStopAsking() const {return dontAsk.getToggleState();}
    juce::ToggleButton dontAsk;
};
}
