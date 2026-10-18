#pragma once
#include <juce_graphics/juce_graphics.h>

namespace kb09_gui {
// Keep the source artwork coordinates intact. Remove only the two blank
// strips approved in TopStrip149; live children use the matching translation.
struct CompactPanelLayout {
    static constexpr int width=1483,sourceHeight=816,reduction=26,height=790;
    static constexpr int firstCut=41,firstEnd=54,secondCut=77,secondEnd=90;
    static void draw(juce::Graphics& g,const juce::Image& source) {
        int destination=0;
        for(const auto segment:{juce::Range<int>(0,firstCut),juce::Range<int>(firstEnd,secondCut),juce::Range<int>(secondEnd,sourceHeight)}) {
            juce::Graphics::ScopedSaveState save(g);
            g.reduceClipRegion(juce::Rectangle<int>(0,destination,width,segment.getLength()));
            g.drawImage(source,juce::Rectangle<float>(0,float(destination-segment.getStart()),float(width),float(sourceHeight)));
            destination+=segment.getLength();
        }
    }
};
}
