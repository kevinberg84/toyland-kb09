#pragma once
#include "../Plugin/StandaloneMidiMap.h"
#include "DialogTheme.h"

namespace kb09_gui {
class MidiMapOverlay final : public juce::Component, public juce::SettableTooltipClient {
public:
    struct Target {juce::Rectangle<int> bounds;int index;};
    explicit MidiMapOverlay(sh09::StandaloneMidiMap& mapping) : map(mapping) {
        setComponentID("midiMapOverlay");setWantsKeyboardFocus(true);setLookAndFeel(&theme);
        setTooltip("Click a highlighted control, tap a button or move a knob/fader, then click DONE. Mappings are remembered. CLEAR ALL resets them.");
        addAndMakeVisible(status);status.setInterceptsMouseClicks(false,false);
        setup(clear,"CLEAR SELECTED",{365,626,170,30},"Remove the selected control's assignment. Its current sound value is preserved. An unassigned mod wheel returns to its normal VCO MOD function.");
        setup(clearAll,"CLEAR ALL",{545,626,170,30},"Reset every custom MIDI assignment. Click again to confirm. Built-in mod wheel, pitch bend, notes and aftertouch remain available. Sound presets are unchanged.");
        setup(done,"DONE",{1210,626,160,30},"Exit MIDI mapping and return to playing. Your assignments are saved automatically, separately from sound presets.");
        setup(options,"OPTIONS",{1030,626,170,30},"Optional correction for the selected three-position switch or AGE mapping. Normally KB-09 recognizes the control during learning. Choose direct knob/fader positioning, a toggle button, or a momentary button. Re-learning also repeats the automatic check.");
        clear.onClick=[this]{map.clear(selected);confirmClear=false;refresh();};
        clearAll.onClick=[this]{if(confirmClear){map.clearAll();confirmClear=false;}else confirmClear=true;refresh();};
        done.onClick=[this]{if(onDone)onDone();};
        options.onClick=[this] {
            map.finishLearn();refresh();
            auto menuTheme=std::make_shared<DialogTheme>(theme.age);
            juce::PopupMenu menu;menu.setLookAndFeel(menuTheme.get());
            for(int i=0;i<4;++i)menu.addItem(i+1,sh09::StandaloneMidiMap::behaviorName(sh09::StandaloneMidiMap::Behavior(i)),true,int(map.behavior(selected))==i);
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&options),
                [safe=juce::Component::SafePointer<MidiMapOverlay>(this),target=selected,menuTheme](int result) {
                    if(safe&&result>0){safe->map.cancelLearn();safe->map.setBehavior(target,sh09::StandaloneMidiMap::Behavior(result-1));safe->refresh();}
                });
        };
        refresh();
    }
    ~MidiMapOverlay() override {setLookAndFeel(nullptr);}
    juce::Rectangle<int> getMessageBounds() const noexcept {return messageBounds;}
    static constexpr const char* learnInstruction="Tap a button or move a knob/fader, then click DONE.";
    void setPassThrough(juce::Rectangle<int> top,juce::Rectangle<int> description) {headerBounds=top;descriptionBounds=description;layoutMessage();}
    void setMessageArea(juce::Rectangle<int> keyboard) {keyboardBounds=keyboard;layoutMessage();}
    void setCondition(float age) {
        if(theme.age==age)return;
        theme.age=age;theme.refreshColours();status.setColour(juce::Label::textColourId,theme.paper());repaint();
    }
    void resized() override {layoutMessage();}
    void setTargets(std::vector<Target> list) {targets=std::move(list);}
    void select(int index) {
        map.finishLearn();selected=index;confirmClear=false;map.learn(index);refresh();
    }
    void refresh() {
        juce::String text="Click a highlighted KB-09 control to assign it.";
        const bool captured=map.isCheckingButton();
        if(selected>=0)text=juce::String(sh09::specs[size_t(selected)].name)+"  -  "+(captured?map.pendingDescription():map.describe(selected))
            +(map.isLearning()?""
                :map.assignment(selected)>=0?"  -  "+juce::String(map.behavior(selected)==sh09::StandaloneMidiMap::Behavior::direct?"Direct":"Button: cycle"):"  -  Click a control to learn again");
        if(confirmClear)text="Click CONFIRM CLEAR to remove all assignments.";
        status.setText(text,juce::dontSendNotification);
        status.setColour(juce::Label::textColourId,theme.paper());
        clear.setEnabled(selected>=0);
        clearAll.setButtonText(confirmClear?"CONFIRM CLEAR":"CLEAR ALL");
        options.setVisible(selected>=0&&sh09::StandaloneMidiMap::supportsButtons(selected)&&map.assignment(selected)>=0);
        layoutMessage();
        done.setToggleState(true,juce::dontSendNotification);
        repaint();
    }
    bool hitTest(int x,int y) override {
        // DESCRIPTION remains usable while mapping; the toolbar sits above us.
        return !headerBounds.contains(x,y)&&!descriptionBounds.contains(x,y);
    }
    void paint(juce::Graphics& g) override {
        for(const auto& target:targets) {
            const bool chosen=target.index==selected;
            const auto color=chosen?juce::Colour(0xffffcc66)
                :map.assignment(target.index)>=0?juce::Colour(0xff6bd5be):juce::Colour(0xffdbb572);
            g.setColour(color.withAlpha(chosen?.2f:.08f));g.fillRoundedRectangle(target.bounds.toFloat(),3.f);
            g.setColour(color);g.drawRoundedRectangle(target.bounds.toFloat(),3.f,chosen?2.f:1.f);
        }
        g.setColour(juce::Colour(0xff090909));g.fillRoundedRectangle(messageBounds.toFloat(),6.f);
        g.setColour(theme.paper().withAlpha(.65f));g.drawRoundedRectangle(messageBounds.toFloat().reduced(1),6.f,1.f);
        const int x=messageBounds.getX(),y=messageBounds.getY();
        g.setColour(theme.paper().withAlpha(.7f));g.drawEllipse(float(x+20),float(y+16),32,32,1);
        g.setFont(DialogTheme::lettering(24));g.drawText("i",x+20,y+15,32,32,juce::Justification::centred);
        g.setColour(theme.paper());g.setFont(theme.getAlertWindowTitleFont());
        g.drawText(confirmClear?"Clear MIDI mappings?":"MIDI mapping",x+68,y+14,messageBounds.getWidth()-88,32,juce::Justification::centredLeft);
        g.setFont(theme.getAlertWindowMessageFont());
        g.drawFittedText(learnInstruction,x+20,y+84,messageBounds.getWidth()-40,40,juce::Justification::topLeft,2,1.f);
    }
    void mouseDown(const juce::MouseEvent& event) override {
        if(messageBounds.contains(event.getPosition()))return;
        for(const auto& target:targets)if(target.bounds.contains(event.getPosition())){select(target.index);return;}
    }
    bool keyPressed(const juce::KeyPress& key) override {
        if(key==juce::KeyPress::escapeKey){if(onDone)onDone();return true;}
        return false;
    }
    std::function<void()> onDone;
private:
    void layoutMessage() {
        if(getWidth()<1||getHeight()<1)return;
        const int available=keyboardBounds.isEmpty()?getWidth():keyboardBounds.getWidth();
        // Lay out only visible actions. Hiding OPTIONS must not leave a hole.
        std::vector<std::pair<juce::TextButton*,int>> actions{{&clear,175},{&clearAll,150}};
        if(options.isVisible())actions.push_back({&options,120});
        actions.push_back({&done,100});
        int desiredWidth=40-10;
        for(const auto& action:actions)desiredWidth+=action.second+10;
        const int width=juce::jmax(280,juce::jmin(desiredWidth,available-28));
        int row=0,column=20;
        for(const auto& action:actions) {
            if(column>20&&column+action.second>width-20){++row;column=20;}
            action.first->setBounds(column,120+row*46,action.second,34);
            column+=action.second+10;
        }
        const int height=170+row*46;
        const int centreY=keyboardBounds.isEmpty()?getHeight()/2:keyboardBounds.getCentreY();
        const int y=juce::jlimit(headerBounds.getBottom()+8,juce::jmax(headerBounds.getBottom()+8,getHeight()-height-12),centreY-height/2);
        const int centre=keyboardBounds.isEmpty()?getWidth()/2:keyboardBounds.getCentreX();
        messageBounds={juce::jlimit(12,juce::jmax(12,getWidth()-width-12),centre-width/2),y,width,height};
        const int x=messageBounds.getX();
        status.setBounds(x+20,y+50,width-40,28);
        for(const auto& action:actions)action.first->setBounds(action.first->getBounds().translated(x,y));
    }
    void setup(juce::TextButton& button,const char* label,juce::Rectangle<int> bounds,const char* tip) {
        addAndMakeVisible(button);button.setButtonText(label);button.setBounds(bounds);button.setTooltip(tip);
        button.setComponentID(juce::String(label).removeCharacters(" "));
    }
    sh09::StandaloneMidiMap& map;std::vector<Target> targets;DialogTheme theme;
    juce::Rectangle<int> messageBounds,headerBounds,descriptionBounds,keyboardBounds;
    int selected=-1;bool confirmClear=false;
    juce::Label status;juce::TextButton clear,clearAll,done,options;
};
}
