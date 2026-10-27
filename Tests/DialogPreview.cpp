#include "../Source/GUI/DialogTheme.h"
#include "../Source/GUI/DialogMessages.h"
#include "../Source/GUI/ClosePrompt.h"
#include "../Source/GUI/UpdateInstructions.h"
#include "../Source/GUI/AudioSettingsDialog.h"
#include "../Source/Presets/Factory.h"
#include <iostream>
#include "AudioLayoutDevice.h"

namespace {
void png(juce::Component& c,const juce::File& file) {
    const auto image=c.createComponentSnapshot(c.getLocalBounds(),true,1.5f);
    file.deleteFile();auto stream=file.createOutputStream();
    if(!stream||!juce::PNGImageFormat().writeImageToStream(image,*stream))throw std::runtime_error("PNG export failed");
}
}
int main(int argc,char** argv){
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc!=2)return 2;
    const juce::File out(juce::String::fromUTF8(argv[1]));out.createDirectory();
    juce::Component stage;stage.setSize(1800,1400);
    {
        const auto expect=[](bool ok,const char* label){if(!ok)throw std::runtime_error(label);};
        kb09_gui::ClosePrompt close(&stage);
        expect(close.getButton(1)->getToggleState(),"Save prompt must retain Don't Save default");
        close.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
        expect(close.getButton(2)->getToggleState(),"right arrow must select Cancel");
        close.keyPressed(juce::KeyPress(juce::KeyPress::downKey));
        expect(close.getButton(0)->getToggleState(),"down arrow must wrap to Save");
        close.keyPressed(juce::KeyPress(juce::KeyPress::upKey));
        expect(close.getButton(2)->getToggleState(),"up arrow must wrap to Cancel");
        kb09_gui::ThemedAlert alert("Keyboard actions","",juce::MessageBoxIconType::InfoIcon,&stage);
        alert.addButton("First",1);alert.addButton("Second",2);alert.addButton("Last",0);
        alert.getButton(1)->setEnabled(false);
        alert.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
        expect(alert.getButton(2)->getToggleState(),"arrows must skip disabled actions");
        int first=0,last=0;
        alert.getButton(0)->onClick=[&]{++first;};alert.getButton(2)->onClick=[&]{++last;};
        alert.keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
        juce::MessageManager::getInstance()->runDispatchLoopUntil(50);
        expect(first==0&&last==1,"Enter must invoke only the selected action");
        kb09::UpdateInstructions instructions("https://example.invalid/",&stage,false);
        expect(instructions.navigation.current()->getButtonText()=="Close","update instructions default must be Close");
        instructions.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
        expect(instructions.navigation.current()->getButtonText()=="Open downloads","custom download action must join navigation");
        instructions.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
        expect(instructions.navigation.current()->getButtonText()=="Copy instructions","custom copy action must join navigation");
        instructions.keyPressed(juce::KeyPress(juce::KeyPress::rightKey));
        expect(instructions.navigation.current()->getButtonText()=="Close","custom actions must wrap to Close");
        std::cout<<"PASS dialog arrows wrap in both directions, skip disabled actions, include custom actions and Enter invokes only selected action.\n";
    }
    for(int level=0;level<3;++level){
        const float age=float(level)*.5f;kb09_gui::DialogTheme ownerTheme(age);stage.setLookAndFeel(&ownerTheme);
        kb09_gui::ClosePrompt reference(&stage);
        auto checkFonts=[&](kb09_gui::ThemedAlert& a) {
            if(a.theme.getAlertWindowTitleFont()!=reference.theme.getAlertWindowTitleFont()
                ||a.theme.getAlertWindowMessageFont()!=reference.theme.getAlertWindowMessageFont()
                ||a.theme.getAlertWindowFont()!=reference.theme.getAlertWindowFont())
                throw std::runtime_error("Dialog typography differs from Save/Don't Save reference");
            for(int n=0;n<a.getNumButtons();++n) {
                auto* b=dynamic_cast<juce::TextButton*>(a.getButton(n));auto* expected=dynamic_cast<juce::TextButton*>(reference.getButton(0));
                if(!b||!expected||a.theme.getTextButtonFont(*b,b->getHeight())!=reference.theme.getTextButtonFont(*expected,expected->getHeight()))
                    throw std::runtime_error("Dialog button font differs from Save/Don't Save reference");
            }
        };
        auto capture=[&](juce::Component& c,const juce::String& name){stage.addAndMakeVisible(c);png(c,out.getChildFile(name+"-"+juce::String(level)+".png"));stage.removeChildComponent(&c);};
        auto alert=[&](const char* id,const char* title,const juce::String& message,juce::MessageBoxIconType icon,juce::StringArray buttons){
            kb09_gui::ThemedAlert a(title,message,icon,&stage,age);int i=0;for(auto& b:buttons)a.addButton(b,++i);checkFonts(a);capture(a,id);
        };
        using I=juce::MessageBoxIconType;
        auto menuCapture=[&](bool presets){
            kb09_gui::UpdateMenuTheme updateTheme(age);
            juce::PopupMenu menu;menu.setLookAndFeel(presets?&ownerTheme:&updateTheme);
            if(presets){int i=0;for(const auto& p:sh09::factoryPresets()){++i;menu.addItem(i,p.name,true,i==1);}}
            else{menu.addSectionHeader("Toyland KB-09 1.1.2");menu.addItem(1,"Check for updates now");menu.addItem(2,"Automatic checks (every 90 days)",true,true);}
            menu.showMenuAsync(juce::PopupMenu::Options().withParentComponent(&stage).withTargetScreenArea({10,10,1,1}).withMinimumWidth(presets?400:380).withStandardItemHeight(presets?36:34),[](int){});
            if(stage.getNumChildComponents()>0)png(*stage.getChildComponent(stage.getNumChildComponents()-1),out.getChildFile(juce::String(presets?"preset-menu":"update-menu")+"-"+juce::String(level)+".png"));
            juce::PopupMenu::dismissAllActiveMenus();
        };
        menuCapture(false);menuCapture(true);
        alert("external-audio","External audio + envelope follower",kb09_gui::externalAudioMessage,I::InfoIcon,{"OK"});
        alert("update-consent","KB-09 updates",kb09_gui::updateConsentMessage,I::QuestionIcon,{"Allow","No thanks"});
        alert("update-available-existing-190","KB-09 update available","Version 1.10.0 is available. Installation is manual and usually takes a few minutes, depending on your download speed. KB-09 will show you the steps.\n\nYour saved presets and audio/MIDI preferences are kept. Save your current patch before closing KB-09. Installing this update is optional.",I::InfoIcon,{"Show update instructions","Not now"});
        alert("update-available","KB-09 update available",kb09_gui::updateAvailableMessage("1.10.0", "- Fix the false error after loading a valid preset.\n- Add a top shadow to the performance bay.\n- Show release highlights in future update prompts."),I::InfoIcon,{"Show update instructions","Not now"});
        alert("up-to-date","KB-09 updates","You're up to date!",I::InfoIcon,{"OK"});
        alert("update-error","KB-09 updates","Could not check for updates. Please try again later.",I::InfoIcon,{"OK"});
        alert("updates-unconfigured","KB-09 updates","Update announcements are not configured in this build yet.",I::InfoIcon,{"OK"});
        alert("preset-load-error","Preset not loaded","The preset could not be opened. It may have been moved or deleted.\n\nYour current patch has not changed.",I::WarningIcon,{"OK"});
        alert("preset-invalid","Preset not loaded","The file is empty or larger than 4 MB.\n\nYour current patch has not changed.",I::WarningIcon,{"OK"});
        alert("preset-save-error","Preset not saved","The preset could not be written. Check the folder's permissions and available disk space.",I::WarningIcon,{"OK"});
        alert("preset-replace","Replace preset?","My KB-09 Patch.kb09 already exists.",I::QuestionIcon,{"Replace","Cancel"});
        alert("audio-device-error","Error when trying to open audio device!","The audio device could not be opened. It may be in use by another application.",I::WarningIcon,{"OK"});
        for(int selection=0;selection<3;++selection){kb09_gui::ClosePrompt close(&stage);for(int i=0;i<3;++i)close.getButton(i)->setToggleState(i==selection,juce::dontSendNotification);capture(close,"close-"+juce::String(selection));}
        for(bool mac:{false,true}){kb09::UpdateInstructions instructions("https://drive.google.com/",&stage,mac,age);checkFonts(instructions);capture(instructions,mac?"instructions-mac":"instructions-windows");
            for(auto* child:instructions.getChildren()) for(auto* b:child->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(b);button&&button->getButtonText()=="Copy instructions")button->setButtonText("Copied!");
            capture(instructions,mac?"instructions-mac-copied":"instructions-windows-copied");}
        juce::AudioDeviceManager manager;juce::Value mute(true);
        kb09_gui::AudioSettingsDialog settings(manager,&stage,&mute,age);checkFonts(settings);capture(settings,"audio-midi-settings");
        stage.setLookAndFeel(nullptr);
    }
    audio_layout_test::Manager layoutManager;
    {
        kb09_gui::ClosePrompt reference(&stage);
        juce::Component scaledCanvas;stage.addAndMakeVisible(scaledCanvas);scaledCanvas.setSize(1483,816);
        for(float zoom:{.55f,1.f,1.6f}) {
            scaledCanvas.setTransform(juce::AffineTransform::scale(zoom));
            kb09_gui::ThemedAlert external("External audio + envelope follower",kb09_gui::externalAudioMessage,juce::MessageBoxIconType::InfoIcon,&scaledCanvas);
            if(std::abs(static_cast<juce::Component&>(external).getDesktopScaleFactor()-static_cast<juce::Component&>(reference).getDesktopScaleFactor())>1e-5f)
                throw std::runtime_error("Canvas zoom changed actual dialog desktop scale relative to Save/Don't Save");
        }
        stage.removeChildComponent(&scaledCanvas);
        std::cout<<"PASS dialog desktop scale matches Save/Don't Save at canvas zoom 0.55, 1.0 and 1.6.\n";
    }
    if(layoutManager.initialise(2,2,nullptr,false).isNotEmpty())return 3;
    juce::Value muteInput(true);
    for(const auto size:{juce::Point<int>(1800,1400),juce::Point<int>(1024,768),juce::Point<int>(800,600)}) {
        stage.setSize(size.x,size.y);
        int changedChannel=-1;
        kb09_gui::AudioSettingsDialog settings(layoutManager,&stage,&muteInput,.7f,{},0,[&](int channel){changedChannel=channel;});
        stage.addAndMakeVisible(settings);
        // Force a shorter window as on high-DPI/small-work-area desktops.
        settings.setSize(juce::jmin(settings.getWidth(),size.x-40),juce::jmin(settings.getHeight(),size.y-50));
        auto* viewport=dynamic_cast<juce::Viewport*>(settings.getCustomComponent(0));
        auto* close=settings.getButton(0);
        if(viewport==nullptr||close==nullptr||viewport->getBottom()+12>close->getY()
           ||!settings.getLocalBounds().contains(close->getBounds()))return 4;
        auto* content=viewport->getViewedComponent();
        if(content==nullptr)return 5;
        juce::ComboBox* receive=nullptr;
        for(auto* child:content->getChildren())if(child->getComponentID()=="midiReceiveChannel")receive=dynamic_cast<juce::ComboBox*>(child);
        if(receive==nullptr||receive->getNumItems()!=17||receive->getSelectedId()!=1)return 8;
        receive->setSelectedId(17,juce::sendNotificationSync);
        if(changedChannel!=16||!content->getLocalBounds().contains(receive->getBounds()))return 9;
        for(auto* child:content->getChildren())if(auto* selector=dynamic_cast<juce::AudioDeviceSelectorComponent*>(child)) {
            for(auto* row:selector->getChildren())if(!selector->getLocalBounds().contains(row->getBounds()))return 6;
        }
        viewport->setViewPosition(0,content->getHeight());
        if(viewport->getViewArea().getBottom()<content->getHeight())return 7;
        png(settings,out.getChildFile("audio-midi-inputs-"+juce::String(size.x)+"x"+juce::String(size.y)+".png"));
        stage.removeChildComponent(&settings);
    }
    std::cout<<"PASS audio input/output rows: MIDI content reachable and Close separate at 1800x1400, 1024x768, 800x600.\n";
    std::cout<<"PASS all message and action-button fonts match Save/Don't Save. Rendered 63 actual dialog/menu states. No update requests, device changes, installs or release changes.\n";
}
