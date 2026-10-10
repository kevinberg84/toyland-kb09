#pragma once
#include <juce_core/juce_core.h>
namespace kb09_gui {
inline const juce::String externalAudioMessage =
    "Route DAW audio to the instrument's External Input and raise EXT in the mixer.\n\n"
    "Without MIDI, set VCA to HOLD and lower VCO / SUB.\n\n"
    "Select ENV FOL'R in VCF, lower CUTOFF and raise ENV for touch-sensitive filtering.\n\n"
    "The follower listens before the EXT fader.";
inline const juce::String updateConsentMessage = "Allow an update check when you open KB-09, every 90 days? Checks send no presets or personal information. Downloads always require your click.";
inline juce::String updateAvailableMessage(const juce::String& version) {
    return "Version "+version+" is available. Installation is manual and usually takes a few minutes, depending on your download speed. KB-09 will show you the steps.\n\nYour saved presets and audio/MIDI preferences are kept. Save your current patch before closing KB-09. Installing this update is optional.";
}
}
