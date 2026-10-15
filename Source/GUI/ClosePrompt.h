#pragma once
#include "DialogTheme.h"
namespace kb09_gui {
class ClosePrompt final : public ThemedAlert {
public:
    explicit ClosePrompt(juce::Component* owner)
        : ThemedAlert("Save your KB-09 patch before closing?",
            "Save opens this patch next time. Don't Save starts with Calibrated Init next time.",
            juce::MessageBoxIconType::QuestionIcon, owner) {
        addButton("Save", 1);
        addButton("Don't Save", 2);
        addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        setWantsKeyboardFocus(true);
        selectAction(1); // Preserve the approved Don't Save keyboard default.
    }

};

}
