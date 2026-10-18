#include "../Source/GUI/StartupDialogReservation.h"
#include <iostream>

int main() {
    juce::ScopedJuceInitialiser_GUI gui;int checks=0,failures=0;
    const auto expect=[&](bool ok,const char* text){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<text<<std::endl;};
    const auto pump=[] {juce::MessageManager::getInstance()->runDispatchLoopUntil(80);};
    for(const auto* first:{"KB-09 updates","Create a desktop shortcut?"}) {
        bool finished=false;
        auto reservation=kb09_gui::StartupDialogReservation::tryAcquire();
        expect(bool(reservation),"first startup request reserves its asynchronous creation slot");
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::QuestionIcon,first,"Startup ordering regression fixture","Not now",nullptr,
            juce::ModalCallbackFunction::create([reservation,&finished](int){finished=true;}));
        reservation.reset();
        expect(!kb09_gui::StartupDialogReservation::tryAcquire(),"second request waits even before the first asynchronous dialog exists");
        pump();
        expect(juce::Component::getNumCurrentlyModalComponents()==1,"only one startup dialog becomes modal");
        expect(!kb09_gui::StartupDialogReservation::tryAcquire(),"second request continues waiting while the first dialog is visible");
        if(auto* modal=juce::Component::getCurrentlyModalComponent())modal->exitModalState(0);
        pump();
        expect(finished,"dismissal callback completes without a nested modal loop");
        auto next=kb09_gui::StartupDialogReservation::tryAcquire();
        expect(bool(next),"deferred second request can proceed after dismissal");
        next.reset();
    }
    juce::AlertWindow unrelated("Other dialog","An unrelated modal window must also delay startup prompts",juce::MessageBoxIconType::NoIcon);
    unrelated.enterModalState(false,nullptr,false);
    expect(!kb09_gui::StartupDialogReservation::tryAcquire(),"an unrelated modal dialog blocks startup prompt creation");
    unrelated.exitModalState(0);pump();
    expect(bool(kb09_gui::StartupDialogReservation::tryAcquire()),"startup slot becomes available after unrelated modal dismissal");
    std::cout<<"RESULT failures="<<failures<<" checks="<<checks<<std::endl;return failures?1:0;
}
