#pragma once
#include <juce_core/juce_core.h>
namespace sh09 {
inline juce::String desktopExecArgument(juce::String path) {
    path=path.replace("\\","\\\\").replace("\"","\\\"").replace("`","\\`").replace("$","\\$").replace("%","%%");
    return "\""+path+"\"";
}
inline juce::Result createDesktopShortcut(const juce::File& application,const juce::File& desktop,juce::String name) {
    if(!application.exists())return juce::Result::fail("The application could not be found.");
    if(application.getFullPathName().containsAnyOf("\r\n"))return juce::Result::fail("The application path contains an unsupported line break.");
    name=juce::File::createLegalFileName(name);
    auto result=desktop.createDirectory();if(result.failed())return result;
   #if JUCE_WINDOWS
    const auto target=desktop.getChildFile(name+".lnk");
   #elif JUCE_MAC
    const auto target=desktop.getChildFile(name+".app");
   #else
    const auto target=desktop.getChildFile(name+".desktop");
   #endif
    if(target.exists()||target.isSymbolicLink())return juce::Result::fail("A desktop item with this name already exists. It was left unchanged.");
   #if JUCE_WINDOWS
    return application.createShortcut("Open "+name,target)?juce::Result::ok():juce::Result::fail("Windows could not create the desktop shortcut.");
   #elif JUCE_MAC
    return application.createSymbolicLink(target,false)?juce::Result::ok():juce::Result::fail("macOS could not create the desktop shortcut.");
   #else
    const auto escapedName=name.replace("\\","\\\\").replace("\n","\\n");
    const auto content="[Desktop Entry]\nType=Application\nVersion=1.0\nName="+escapedName+"\nExec="+desktopExecArgument(application.getFullPathName())+"\nTerminal=false\nCategories=AudioVideo;Audio;Midi;\n";
    if(!target.create().wasOk())return juce::Result::fail("The desktop launcher could not be created.");
    if(!target.replaceWithText(content)||!target.setExecutePermission(true)){target.deleteFile();return juce::Result::fail("The desktop launcher could not be written or made executable.");}
    return juce::Result::ok();
   #endif
}
}
