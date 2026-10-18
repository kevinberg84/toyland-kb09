#pragma once
#include <juce_graphics/juce_graphics.h>
namespace kb09_gui {
// Same light direction, opacity and five soft layers as the black-key and
// knob shadows. Scale the throw and penumbra with height above the receiver.
inline void roundedHardwareShadow(juce::Graphics& g,juce::Rectangle<float> footprint,float radius,float height) {
    juce::Graphics::ScopedSaveState save(g);
    g.setColour(juce::Colours::black.withAlpha(.07f));
    for(int i=5;i>0;--i)
        g.fillRoundedRectangle(footprint.expanded(i*.45f*height).translated(2*height,3*height),radius);
}
inline void hardwarePathShadow(juce::Graphics& g,const juce::Path& footprint,float height) {
    juce::Graphics::ScopedSaveState save(g);
    g.setColour(juce::Colours::black.withAlpha(.07f));
    for(int i=5;i>0;--i) {
        juce::Path expanded;
        juce::PathStrokeType(2*i*.45f*height,juce::PathStrokeType::curved,juce::PathStrokeType::rounded).createStrokedPath(expanded,footprint);
        expanded.addPath(footprint);expanded.setUsingNonZeroWinding(true);
        g.fillPath(expanded,juce::AffineTransform::translation(2*height,3*height));
    }
}
}
