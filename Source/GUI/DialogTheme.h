#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ButtonCycle.h"

namespace kb09_gui {
struct DialogFonts {
    DialogFonts();
    juce::Typeface::Ptr regular, semibold;
};
// Per-window ownership keeps this theme out of the host's own windows.
class DialogTheme : public juce::LookAndFeel_V4 {
public:
    explicit DialogTheme(float condition = 0);
private:
    juce::SharedResourcePointer<DialogFonts> fonts;
public:
    float age = 0;
    // The Save / Don't Save prompt is the shared typography reference.
    static constexpr float titleFontHeight=22, messageFontHeight=17, buttonFontHeight=16;
    std::function<float()> conditionProvider;
    juce::Colour paper() const;
    virtual bool printWearEnabled() const {return true;}
    static juce::Font lettering(float height, bool bold = false);
    static float conditionOf(juce::Component*);
    void refreshColours();
    void wear(juce::Graphics&, juce::Rectangle<int>, int seed = 19) const;
    void brand(juce::Graphics&, juce::Rectangle<float>) const;
    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;
    juce::Font getAlertWindowFont() override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getPopupMenuFont() override;
    void drawAlertBox(juce::Graphics&, juce::AlertWindow&, const juce::Rectangle<int>&, juce::TextLayout&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawLabel(juce::Graphics&,juce::Label&) override;
    void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
    void drawPopupMenuBackground(juce::Graphics&, int, int) override;
    void drawPopupMenuSectionHeader(juce::Graphics&, const juce::Rectangle<int>&, const juce::String&) override;
    void getIdealPopupMenuSectionHeaderSizeWithOptions(const juce::String&,int,int&,int&,const juce::PopupMenu::Options&) override;
    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>&, bool, bool, bool, bool, bool,
                           const juce::String&, const juce::String&, const juce::Drawable*, const juce::Colour*) override;
    juce::AlertWindow* createAlertWindow(const juce::String&, const juce::String&, const juce::String&,
        const juce::String&, const juce::String&, juce::MessageBoxIconType, int, juce::Component*) override;
};

class ThemedAlert : public juce::AlertWindow,private juce::Timer {
public:
    ThemedAlert(const juce::String& title, const juce::String& message, juce::MessageBoxIconType icon,
                juce::Component* owner = nullptr, float condition = -1);
    ~ThemedAlert() override { setLookAndFeel(nullptr); }
    void addButton(const juce::String& name,int value,const juce::KeyPress& key1={},const juce::KeyPress& key2={}) {
        juce::AlertWindow::addButton(name,value,key1,key2);
        navigation.add(dynamic_cast<juce::TextButton*>(getButton(getNumButtons()-1)));
    }
    void setNavigationActions(std::initializer_list<juce::TextButton*> buttons) {navigation.setActions(buttons);}
    void selectAction(int index) {navigation.select(dynamic_cast<juce::TextButton*>(getButton(index)));}
    bool keyPressed(const juce::KeyPress& key) override {
        return navigation.handle(key)||juce::AlertWindow::keyPressed(key);
    }
    DialogTheme theme;
    ButtonCycle navigation;
    // Modal backgrounds are fixed. Native close-click capture can otherwise
    // arrive as a fresh-looking press and latch AlertWindow's component dragger.
    void mouseDown(const juce::MouseEvent&) override {}
    void mouseDrag(const juce::MouseEvent&) override {}
    void mouseUp(const juce::MouseEvent& event) override {
        juce::AlertWindow::mouseUp(event);
    }
private:
    void timerCallback() override;
    juce::Component::SafePointer<juce::Component> conditionOwner;
};

class UpdateMenuTheme final : public DialogTheme {
public:
    using DialogTheme::DialogTheme;
};
}
