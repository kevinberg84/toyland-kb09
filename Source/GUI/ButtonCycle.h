#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace kb09_gui {
// Shared action navigation. Only the explicitly registered action row is
// navigated; arrows in audio-device selectors and other input fields stay local.
class ButtonCycle final : private juce::KeyListener, private juce::MouseListener {
public:
    ~ButtonCycle() { detach(); }
    void setActions(std::initializer_list<juce::TextButton*> list) {
        detach();
        for (auto* button : list) if (button) {
            actions.emplace_back(button);
            button->addKeyListener(this);button->addMouseListener(this, false);
            button->setWantsKeyboardFocus(true);
        }
        refresh();
    }
    void add(juce::TextButton* button) {
        if (!button) return;
        actions.emplace_back(button);button->addKeyListener(this);button->addMouseListener(this,false);
        button->setWantsKeyboardFocus(true);refresh();
    }
    void select(juce::TextButton* button, bool focus = true) {
        if (eligible(button)) selected=button;
        refresh();
        if (focus && selected && selected->isShowing()) selected->grabKeyboardFocus();
    }
    juce::TextButton* current() const noexcept { return selected.getComponent(); }
    void refresh() {
        if (!eligible(selected)) {
            selected=nullptr;
            for (auto& button : actions) if (eligible(button)) {selected=button;break;}
        }
        for (auto& button : actions) if (button)
            button->setToggleState(button==selected,juce::dontSendNotification);
    }
    bool handle(const juce::KeyPress& key) {
        const bool previous=key==juce::KeyPress::leftKey||key==juce::KeyPress::upKey;
        const bool next=key==juce::KeyPress::rightKey||key==juce::KeyPress::downKey;
        if (!previous && !next && key!=juce::KeyPress::returnKey) return false;
        refresh();
        std::vector<juce::TextButton*> enabled;
        for(auto& button:actions) if(eligible(button)) enabled.push_back(button);
        if(enabled.empty()) return false;
        if(previous||next) {
            auto found=std::find(enabled.begin(),enabled.end(),selected.getComponent());
            int index=found==enabled.end()?0:int(found-enabled.begin());
            index=(index+(previous?-1:1)+int(enabled.size()))%int(enabled.size());
            select(enabled[size_t(index)]);return true;
        }
        selected->triggerClick();return true;
    }
private:
    static bool eligible(juce::TextButton* button) {return button&&button->isVisible()&&button->isEnabled();}
    void detach() {
        for(auto& button:actions) if(button) {button->removeKeyListener(this);button->removeMouseListener(this);}
        actions.clear();
    }
    bool keyPressed(const juce::KeyPress& key,juce::Component*) override {return handle(key);}
    void mouseDown(const juce::MouseEvent& event) override {
        if(auto* button=dynamic_cast<juce::TextButton*>(event.eventComponent)) select(button,false);
    }
    std::vector<juce::Component::SafePointer<juce::TextButton>> actions;
    juce::Component::SafePointer<juce::TextButton> selected;
};
}
