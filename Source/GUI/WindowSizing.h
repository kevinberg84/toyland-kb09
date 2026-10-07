#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>
#include <memory>

namespace sh09 {

/** Message-thread-only editor/window sizing, independent of the panel artwork.

    Integration:
      - Own this helper for the editor lifetime; add WindowSizing.cpp to the target.
      - Forward M/L/maximise controls to chooseMedium/chooseLarge/toggleMaximise.
      - Forward only the EMPTY custom header's mouse events to the header methods.
        Do not forward slider, button, keyboard or text-field gestures.
      - In resized(), fit the design canvas into contentBounds(getLocalBounds().toFloat()).
        Scale uniformly and centre it there. Paint the remaining area as letterbox.
      - Call peerAttached() from parentHierarchyChanged() and visibilityChanged().
        It performs one bounded initial fit after peer scale stabilises; later
        user size choices and custom resizing are not periodically overridden.
      - Do not reapply a fixed window aspect ratio: large mode needs a monitor-shaped
        window. This helper clears the default constrainer's fixed aspect ratio.

    M fits the design aspect inside 90% of the current monitor's work-area width
    AND height. L fills that work area (taskbar stays accessible). The standalone
    uses its OWN JUCE native window's maximise API. VST3 asks the host to resize the
    editor; it never maximises the host/DAW. Floating-window movement is available
    only after conservative native ownership/geometry/title checks. Other hosts
    retain placement control. Native DAW caption snapping remains host/OS behaviour;
    the custom header supports top-edge snap where safe movement is available.

    Research references (checked against local JUCE 9.0.2 source):
      JUCE Displays::Display::userBounds is logical, DPI-aware work area (userArea deprecated).
      JUCE Component::getLocalArea handles host scale transforms.
      JUCE ResizableWindow::setFullScreen uses SW_SHOWMAXIMIZED on Windows.
      https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getancestor
      https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos
      https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-monitorinfo
      https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow
*/
class WindowSizing final {
public:
    enum class Mode { custom, medium, large };

    WindowSizing(juce::AudioProcessorEditor&, int designWidth, int designHeight);
    ~WindowSizing();

    void chooseMedium();
    void chooseLarge();
    void toggleMaximise();
    void restore();
    void peerAttached();

    void beginHeaderDrag(const juce::MouseEvent&);
    void dragHeader(const juce::MouseEvent&);
    void endHeaderDrag(const juce::MouseEvent&);
    void doubleClickHeader();

    bool isLarge() const;
    bool canMoveWindow() const;
    bool isSnapArmed() const;
    Mode getMode() const;
    juce::String placementStatus() const;
    juce::Rectangle<float> contentBounds(juce::Rectangle<float> available) const noexcept;

    // Called on the message thread when mode/snap indication changes. Use to repaint
    // the maximise icon or a subtle snap affordance; do not destroy this object here.
    std::function<void()> onChange;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowSizing)
};
}
