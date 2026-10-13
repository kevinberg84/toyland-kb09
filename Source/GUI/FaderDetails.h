#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PowerDetails.h"
#include <map>

namespace kb09_gui {
// Two recessed felt strips meet at a dark slit. Individual deposits are
// accumulated at fixed positions; existing grains keep their full contrast.
inline const juce::Image& faderDustCover(uint32_t seed,int stage) {
    static std::map<uint64_t,juce::Image> cache;
    auto& image=cache[(uint64_t(seed)<<3)|uint64_t(stage)];
    if(image.isValid())return image;
    constexpr int scale=4;
    image=juce::Image(juce::Image::ARGB,32,560,true,juce::SoftwareImageType());
    {
        juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
        for(int y=0;y<pixels.height;++y)for(int x=0;x<pixels.width;++x) {
            const float u=(x+.5f)/pixels.width,v=(y+.5f)/scale;
            const float fringe=.28f+.055f*power_detail::smoothNoise(v*.9f,u*12,seed);
            const float material=power_detail::smoothRange(.5f-fringe,.53f-fringe,std::abs(u-.5f));
            const float fibre=power_detail::noise(seed+uint32_t(x)*7919u+uint32_t(y)*37u);
            const float value=.014f+material*(.044f+.020f*fibre);
            pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(value,value*.99f,value*.90f,1));
        }
    }
    juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale(float(scale)));
    for(uint32_t i=0;i<uint32_t(stage*400);++i) {
        const uint32_t key=seed+i*7919u;
        const float u=power_detail::noise(key+31),v=power_detail::noise(key+67);
        const float x=8.f*((i%2?.67f:.035f)+.295f*u),y=5.f+130.f*v;
        const float radius=.08f+.22f*power_detail::noise(key+113);
        g.setColour(juce::Colour(i%5?0xffa3977d:0xff6e6658).withAlpha(.35f+.40f*power_detail::noise(key+199)));
        g.fillEllipse(x-radius,y-radius,radius*2,radius*(1.f+power_detail::noise(key+227)));
        if(i%11==0) {
            juce::Path fibre;fibre.startNewSubPath(x,y);
            fibre.quadraticTo(x+.20f,y+.25f,x-.10f,y+.50f+.45f*power_detail::noise(key+293));
            g.strokePath(fibre,juce::PathStrokeType(.085f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
    }
    return image;
}
inline void paintFaderDustCover(juce::Graphics& g,juce::Rectangle<float> opening,uint32_t seed,int stage) {
    juce::Graphics::ScopedSaveState save(g);
    juce::Path clip;clip.addRoundedRectangle(opening,opening.getWidth()*.5f);g.reduceClipRegion(clip);
    // The end pockets remain deeper and darker than the felt along the slot.
    const auto strips=opening.reduced(.35f,3.f);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(faderDustCover(seed,stage),strips);
    g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.8f),0,opening.getY(),
        juce::Colours::transparentBlack,0,opening.getY()+7,false));
    g.fillRect(opening.withHeight(7));
    g.setGradientFill(juce::ColourGradient(juce::Colours::transparentBlack,0,opening.getBottom()-7,
        juce::Colours::black.withAlpha(.65f),0,opening.getBottom(),false));
    g.fillRect(opening.withTop(opening.getBottom()-7));
}
} // namespace kb09_gui
