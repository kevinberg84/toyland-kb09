#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace kb09_gui {
class ClosePrompt final : public juce::AlertWindow, private juce::KeyListener {
public:
    explicit ClosePrompt(juce::Component* owner)
        : juce::AlertWindow("Save your KB-09 patch before closing?",
            "Save opens this patch next time. Don't Save starts with Calibrated Init next time.",
            juce::MessageBoxIconType::QuestionIcon, owner) {
        addButton("Save", 1);
        addButton("Don't Save", 2);
        addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        for (int i = 0; i < getNumButtons(); ++i)
            if (auto* button = getButton(i)) button->addKeyListener(this);
        setWantsKeyboardFocus(true);
        select(1); // The non-saving choice is the safe keyboard default.
    }

    bool keyPressed(const juce::KeyPress& key) override { return handleKey(key); }

private:
    bool keyPressed(const juce::KeyPress& key, juce::Component*) override { return handleKey(key); }
    bool handleKey(const juce::KeyPress& key) {
        if (key == juce::KeyPress::leftKey || key == juce::KeyPress::upKey) { select((selected + 2) % 3); return true; }
        if (key == juce::KeyPress::rightKey || key == juce::KeyPress::downKey) { select((selected + 1) % 3); return true; }
        if (key == juce::KeyPress::returnKey) {
            if (auto* button = getButton(selected)) button->triggerClick();
            return true;
        }
        if (key == juce::KeyPress::escapeKey) { triggerButtonClick("Cancel"); return true; }
        return juce::AlertWindow::keyPressed(key);
    }
    void select(int index) {
        selected = index;
        for (int i = 0; i < 3; ++i) if (auto* button = getButton(i)) {
            button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffdbb572));
            button->setColour(juce::TextButton::textColourOnId, juce::Colour(0xff151a1d));
            button->setToggleState(i == selected, juce::dontSendNotification);
        }
        if (auto* button = getButton(selected)) button->grabKeyboardFocus();
    }
    int selected = 1;
};

}
