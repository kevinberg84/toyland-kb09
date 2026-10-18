#pragma once
#include "PowerDetails.h"
namespace kb09_gui {
// Continue neighboring photographic paint across an obsolete screw seat.
// Match each row's intact inboard lighting, preserving the panel's lower fold.
inline juce::Image repairScrewSeat(const juce::Image& original,const juce::Image& donor,bool right) {
    juce::Image result(juce::Image::ARGB,256,256,true,juce::SoftwareImageType());
    const juce::Image::BitmapData input(original,juce::Image::BitmapData::readOnly),source(donor,juce::Image::BitmapData::readOnly);
    juce::Image::BitmapData output(result,juce::Image::BitmapData::writeOnly);
    for(int y=0;y<256;++y) {
        float delta[3]{};float count=0;
        for(int yy=juce::jmax(0,y-24);yy<=juce::jmin(256,y+24);++yy)
            for(int xx=right?0:248;xx<(right?9:257);++xx) {
                const auto a=input.getPixelColour(xx,yy),b=source.getPixelColour(xx,yy);
                const float weight=std::exp(-float((yy-y)*(yy-y))/144.f);
                delta[0]+=weight*(a.getFloatRed()-b.getFloatRed());delta[1]+=weight*(a.getFloatGreen()-b.getFloatGreen());delta[2]+=weight*(a.getFloatBlue()-b.getFloatBlue());count+=weight;
            }
        for(auto& d:delta)d/=float(count);
        for(int x=0;x<256;++x) {
            const float dx=(x+.5f)*.25f-32,dy=(y+.5f)*.25f-32;
            const float alpha=1.f-power_detail::smoothRange(24.f,30.f,std::hypot(dx,dy));
            if(alpha<=0)continue;
            const auto c=source.getPixelColour(x,y);
            output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(juce::jlimit(0.f,1.f,c.getFloatRed()+delta[0]),juce::jlimit(0.f,1.f,c.getFloatGreen()+delta[1]),juce::jlimit(0.f,1.f,c.getFloatBlue()+delta[2]),alpha));
        }
    }
    return result;
}
}
