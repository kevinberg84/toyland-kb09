#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace kb09_gui {
// Shared widget API: computer performance input updates its visual state on
// the message thread, independently of when the audio callback consumes it.
class KeyboardFeedback : public juce::Component {
public:
    virtual void setComputerNote(int note, bool down) = 0;
};
}
