#pragma once
#include <juce_graphics/juce_graphics.h>
namespace kb09_gui {
// Preserve the approved outer silhouette while using the relit photographic
// material. The newly thinner top lip and enlarged face are intentional.
inline juce::Image knobPhotoWithOriginalCoverage(const juce::Image& original,const juce::Image& relit) {
    auto result=relit.createCopy();
    const juce::Image::BitmapData old(original,juce::Image::BitmapData::readOnly),source(relit,juce::Image::BitmapData::readOnly);
    juce::Image::BitmapData pixels(result,juce::Image::BitmapData::writeOnly);
    for(int y=0;y<old.height;++y)for(int x=0;x<old.width;++x) {
        const auto alpha=old.getPixelColour(x,y).getAlpha();auto colour=source.getPixelColour(x,y);
        if(alpha>0&&colour.getAlpha()<128) {
            for(float scale=.997f;scale>.95f;scale-=.003f) {
                const int sx=juce::jlimit(0,old.width-1,int(old.width*.497f+(x-old.width*.497f)*scale));
                const int sy=juce::jlimit(0,old.height-1,int(old.height*.505f+(y-old.height*.505f)*scale));
                colour=source.getPixelColour(sx,sy);if(colour.getAlpha()>=128)break;
            }
        }
        pixels.setPixelColour(x,y,colour.withAlpha(alpha));
    }
    return result;
}
}
