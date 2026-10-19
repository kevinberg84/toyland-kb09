#include "../Source/Plugin/Processor.h"
#include "../Source/GUI/UpdateChecker.h"
#include <iostream>
int main(){int fails=0;auto check=[&](bool x){if(!x)++fails;};
check(kb09::Updates::newer("0.13.14","0.13.13"));check(kb09::Updates::newer("0.14.0","0.13.99"));check(!kb09::Updates::newer("0.13.14","0.13.14"));check(!kb09::Updates::newer("0.9.0","0.13.14"));check(!kb09::Updates::newer("bad.1.0","0.13.14"));check(!kb09::Updates::validVersion("1.2"));check(!kb09::Updates::validVersion("1.2.3-beta"));check(!kb09::Updates::validVersion("1..3"));check(kb09::updateInstructions(false).contains("Standalone.exe"));
check(!kb09::updateInstructions(false).contains("INSTALL KB-09.command"));
check(kb09::updateInstructions(true).contains("INSTALL KB-09.command"));
check(!kb09::updateInstructions(true).contains("Standalone.exe"));
for(bool mac : {false,true}) {
 check(kb09::updateInstructions(mac).contains("desktop shortcut"));
 check(kb09::updateInstructions(mac).contains("Keep any personal presets"));
 check(kb09::updateInstructions(mac).contains("Save your current patch"));
}
check(kb09::updateInstructions(false,true).contains("Linux (Standalone Only).zip"));
check(!kb09::updateInstructions(false,true).contains("Standalone.exe"));
check(kb09::updateInstructions(false,true).contains("desktop shortcut"));
check(kb09::Updates::validDownloadLink("https://github.com/kevinberg84/toyland-kb09/releases/latest"));
check(kb09::Updates::validDownloadLink("https://github.com/kevinberg84/toyland-kb09/releases/tag/v1.9.0"));
check(!kb09::Updates::validDownloadLink("https://github.com/another/project/releases/latest"));
check(!kb09::Updates::validDownloadLink("https://github.com.evil.example/kevinberg84/toyland-kb09/releases/latest"));
check(!kb09::Updates::validDownloadLink("http://github.com/kevinberg84/toyland-kb09/releases/latest"));
auto feed=juce::JSON::parse(R"({"windows":"https://drive.google.com/file/d/legacy","releasePage":"https://github.com/kevinberg84/toyland-kb09/releases/tag/v1.9.0"})");
check(kb09::Updates::selectDownloadLink(feed,"windows").contains("github.com"));
feed.getDynamicObject()->removeProperty("releasePage");check(kb09::Updates::selectDownloadLink(feed,"windows").contains("drive.google.com"));
feed.getDynamicObject()->setProperty("releasePage","https://example.com/untrusted");check(kb09::Updates::selectDownloadLink(feed,"windows").isEmpty());
std::cout<<"Update version checks: "<<fails<<" failures\n";return fails?1:0;}
