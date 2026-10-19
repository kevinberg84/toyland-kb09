#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "UpdateInstructions.h"
#include "DialogMessages.h"
#include "StartupDialogReservation.h"

namespace kb09 {
// Public announcement endpoint only; release downloads remain user initiated.
inline constexpr const char* versionEndpoint = "https://raw.githubusercontent.com/kevinberg84/toyland-kb09-updates/main/version.json";
class Updates final : private juce::Thread {
public:
    static bool validDownloadLink(const juce::String& link) {
        const auto domain=juce::URL(link).getDomain();
        if(!link.startsWith("https://"))return false;
        if(domain=="drive.google.com"||domain=="docs.google.com")return true;
        return link.startsWith("https://github.com/kevinberg84/toyland-kb09/releases/")
            &&!link.contains("..")&&!link.contains("%")&&!link.contains("?");
    }
    static juce::String selectDownloadLink(const juce::var& json,const char* platform) {
        const auto release=json["releasePage"].toString();
        if(release.isNotEmpty())return validDownloadLink(release)?release:juce::String();
        const auto legacy=json[platform].toString();
        return validDownloadLink(legacy)?legacy:juce::String();
    }
    static Updates& instance() { static Updates service; return service; }
    bool configured() const {
#if KB09_LOCAL_DSP_TEST || KB09_LOCAL_MIDI_TEST
        return false;
#else
        return juce::String(versionEndpoint).startsWith("https://");
#endif
    }
    bool decided() { return settings->containsKey("asked"); }
    bool enabled() { return settings->getBoolValue("annual",false); }
    void consent(bool yes) { settings->setValue("asked",true);settings->setValue("annual",yes);settings->saveIfNeeded(); }
    void check(bool manual) {
        if(isThreadRunning()||!configured())return;
        settings->reload();
        const auto now=juce::Time::currentTimeMillis();
        if(!manual&&(!enabled()||now-settings->getDoubleValue("lastSuccess",0)<90.0*86400000.0||now-settings->getDoubleValue("lastAttempt",0)<86400000.0))return;
        manualCheck=manual;settings->setValue("lastAttempt",double(now));settings->saveIfNeeded();startThread();
    }
    void showMenu(juce::Component* target) {
        auto theme=std::make_shared<kb09_gui::UpdateMenuTheme>(kb09_gui::DialogTheme::conditionOf(target));
        juce::PopupMenu m;m.setLookAndFeel(theme.get());m.addSectionHeader(juce::String(JucePlugin_Name)+" "+JucePlugin_VersionString);
#if KB09_LOCAL_DSP_TEST || KB09_LOCAL_MIDI_TEST
        m.addItem(3,"Local test - public update checks disabled",false);
#else
        m.addItem(1,"Check for updates now");m.addItem(2,"Automatic checks (every 90 days)",true,enabled());
#endif
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(target).withMinimumWidth(380).withStandardItemHeight(34),[theme,owner=juce::Component::SafePointer<juce::Component>(target)](int item){auto& s=instance();if(item==1){if(s.configured())s.check(true);else juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"KB-09 updates","Update announcements are not configured in this build yet.","OK",owner.getComponent());}if(item==2)s.consent(!s.enabled());});
    }
    bool opened(juce::Component* owner = nullptr) {
        if(!configured())return true;
        if(!decided()) {
            if(prompting.load())return true;
            auto reservation=kb09_gui::StartupDialogReservation::tryAcquire();
            if(!reservation)return false;
            prompting=true;
            juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::QuestionIcon,"KB-09 updates",kb09_gui::updateConsentMessage,"Allow","No thanks",owner,juce::ModalCallbackFunction::create([reservation](int yes){auto& s=instance();s.consent(yes!=0);s.prompting=false;if(yes)s.check(false);}));
        } else check(false);
        return true;
    }
    void deliver(juce::Component* owner = nullptr) {
        if(!ready.load())return;
        auto reservation=kb09_gui::StartupDialogReservation::tryAcquire();
        if(!reservation)return;
        if(!ready.exchange(false))return;
        juce::String v,link,message;bool manual=false,ok=false;
        {juce::ScopedLock lock(resultLock);v=newVersion;link=download;message=error;manual=resultManual;ok=success;}
        if(ok&&!manual){settings->setValue("lastSuccess",double(juce::Time::currentTimeMillis()));settings->saveIfNeeded();}
        if(link.isNotEmpty())juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::InfoIcon,"KB-09 update available",kb09_gui::updateAvailableMessage(v),"Show update instructions","Not now",owner,juce::ModalCallbackFunction::create([reservation,link, safe=juce::Component::SafePointer<juce::Component>(owner)](int yes){if(yes)UpdateInstructions::show(link,safe.getComponent());}));
        else if(manual)juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"KB-09 updates",ok?"You're up to date!":"Could not check for updates. Please try again later.","OK",owner,juce::ModalCallbackFunction::create([reservation](int){}));
    }
    static bool validVersion(const juce::String& v) {
        auto parts=juce::StringArray::fromTokens(v,".","");if(parts.size()!=3)return false;for(auto& p:parts)if(p.isEmpty()||p.length()>6||!p.containsOnly("0123456789"))return false;return true;
    }
    static bool newer(const juce::String& a,const juce::String& b) {
        if(!validVersion(a)||!validVersion(b))return false;
        auto aa=juce::StringArray::fromTokens(a,".",""),bb=juce::StringArray::fromTokens(b,".","");
        if(aa.size()!=3||bb.size()!=3)return false;
        for(int i=0;i<3;++i){if(!aa[i].containsOnly("0123456789")||aa[i].isEmpty())return false;}
        for(int i=0;i<3;++i){if(aa[i].getIntValue()!=bb[i].getIntValue())return aa[i].getIntValue()>bb[i].getIntValue();}return false;
    }
private:
    Updates():Thread("KB-09 update check") {
        juce::PropertiesFile::Options o;o.applicationName="Toyland KB-09 Updates";o.filenameSuffix="settings";o.folderName="Toyland";o.osxLibrarySubFolder="Application Support";
#if KB09_LOCAL_DSP_TEST || KB09_LOCAL_MIDI_TEST
          o.applicationName=juce::String(JucePlugin_Name)+" Updates";
#endif
        settings=std::make_unique<juce::PropertiesFile>(o);
    }
    ~Updates() override {signalThreadShouldExit();stopThread(12000);}
    void run() override {
        juce::InterProcessLock lease("ToylandKB09UpdateRequest");
        if(!lease.enter(0))return;
        bool ok=false;juce::String v,link;
        auto stream=juce::URL(versionEndpoint).createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(5000).withNumRedirectsToFollow(3));
        if(stream&&!threadShouldExit()) {
            juce::MemoryBlock body;char block[2048];
            while(!threadShouldExit()&&!stream->isExhausted()&&body.getSize()<=65536){int n=stream->read(block,sizeof block);if(n<=0)break;body.append(block,size_t(n));}
            if(body.getSize()<=65536){auto json=juce::JSON::parse(body.toString());v=json["version"].toString();ok=json["product"].toString()=="KB-09"&&validVersion(v);
#if JUCE_MAC
                link=selectDownloadLink(json,"mac");
#elif JUCE_LINUX
                link=selectDownloadLink(json,"linux");
#else
                link=selectDownloadLink(json,"windows");
#endif
                if(!validDownloadLink(link))ok=false;
                if(!ok||!newer(v,JucePlugin_VersionString))link.clear();
            }
        }
        {juce::ScopedLock lock(resultLock);success=ok;resultManual=manualCheck;newVersion=v;download=link;}ready=true;
    }
    std::unique_ptr<juce::PropertiesFile> settings;juce::CriticalSection resultLock;
    std::atomic<bool> ready{false},prompting{false};bool manualCheck=false,resultManual=false,success=false;juce::String newVersion,download,error;
};
}
