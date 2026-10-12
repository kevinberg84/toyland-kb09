#pragma once
#include "PowerDetails.h"
#include "BinaryData.h"
#include "AgeAppearance.h"
#include <vector>

namespace kb09_gui {
// Separate clean mouldings and two distinct photo-informed wear sprites.
// No mirrored asset or shared damage stamp. Endpoints are prepared once and
// selected only in the static panel cache; moving controls do not rebuild them.
class CheekMaterial {
public:
    static constexpr int width=55,height=732,scale=3;
    CheekMaterial() {
        for(int side=0;side<2;++side)for(int endpoint=0;endpoint<1;++endpoint) {
            auto& image=images[size_t(side)][size_t(endpoint)];
            image=juce::Image(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
            juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
            const auto seed=side?0x7e21a53bu:0x18b64cd9u;
            for(int y=0;y<image.getHeight();++y)for(int x=0;x<image.getWidth();++x) {
                const float px=(x+.5f)/scale,py=(y+.5f)/scale;
                const float qx=std::abs(px-width*.5f)-(width*.5f-12),qy=std::abs(py-height*.5f)-(height*.5f-12);
                const float dx=juce::jmax(qx,0.f),dy=juce::jmax(qy,0.f);
                const float distance=std::sqrt(dx*dx+dy*dy)+juce::jmin(juce::jmax(qx,qy),0.f)-12;
                const float alpha=juce::jlimit(0.f,1.f,.5f-distance*scale);
                if(alpha<=0)continue;
                const float grain=power_detail::smoothNoise(px*5.3f,py*5.3f,seed)-.5f;
                const float stipple=power_detail::smoothNoise(px*1.7f,py*1.7f,seed+17)-.5f;
                const float handling=power_detail::smoothNoise(px*.12f,py*.09f,seed+31)-.5f;
                const float broad=power_detail::smoothNoise(px*.026f,py*.018f,seed+73)-.5f;
                const float edge=px;
                const float inner=side?px:float(width)-px;
                const float edgeLight=std::exp(-std::pow((edge-6.5f)/2.6f,2.f));
                const float topLight=std::exp(-std::pow((py-6)/4.f,2.f));
                const float lowerFold=power_detail::smoothRange(height-12.f,height-1.f,py);
                float light=.122f+.030f*(1-px/width)+.025f*(1-py/height);
                light+=endpoint*(.058f+.045f*handling+.034f*broad);
                light+=grain*(.026f+endpoint*.085f)+stipple*(.014f+endpoint*.043f);
                light+=(side?.095f:.175f)*edgeLight+(.18f-endpoint*.13f)*topLight;
                light*=1-.36f*lowerFold;
                light*=.62f+.38f*power_detail::smoothRange(0,3,inner);
                pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                    juce::jlimit(0.f,1.f,light*.99f),juce::jlimit(0.f,1.f,light*1.01f),juce::jlimit(0.f,1.f,light),alpha));
            }
        }
        const auto atlas=juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(
            BinaryData::cheekspersonalindependentv1_png,BinaryData::cheekspersonalindependentv1_pngSize));
        // Trace complete individual scratches and cuts, including the lower
        // halves. Each has a fixed birth stage; there is no final-stage catch-all.
        struct Mark {float x1,y1,x2,y2,radius;int birth;};
        const std::array<std::vector<Mark>,2> marks{{
            {{.14f,.048f,.30f,.085f,12.f,3}, {.90f,.21f,.98f,.212f,8.f,2},
             {.10f,.44f,.14f,.444f,7.f,1}, {.62f,.39f,.76f,.395f,4.5f,3},
             {.34f,.716f,.45f,.706f,5.f,2}, {.08f,.939f,.12f,.963f,7.f,3},
             {.10f,.966f,.28f,.987f,9.f,4},
             {.47f,.136f,.83f,.133f,2.5f,1}, {.14f,.260f,.38f,.250f,2.7f,2},
             {.15f,.315f,.82f,.365f,3.f,2}, {.15f,.375f,.73f,.433f,3.f,3},
             {.14f,.540f,.43f,.534f,2.5f,1}, {.33f,.526f,.46f,.519f,2.f,3},
             {.14f,.603f,.76f,.640f,3.f,3}, {.13f,.614f,.57f,.633f,2.5f,1},
             {.47f,.811f,.96f,.833f,2.7f,4}, {.13f,.865f,.32f,.859f,2.5f,2},
             {.31f,.946f,.54f,.951f,2.2f,3}, {.10f,.044f,.21f,.028f,7.f,4}},
            {{.88f,.11f,.98f,.114f,12.f,1}, {.96f,.27f,.98f,.274f,8.f,2},
             {.88f,.48f,.98f,.484f,8.f,3}, {.96f,.674f,.98f,.679f,8.f,2},
             {.51f,.052f,.61f,.082f,4.5f,2}, {.53f,.341f,.62f,.337f,2.5f,1},
             {.72f,.950f,.83f,.961f,6.f,3}, {.88f,.030f,.96f,.049f,9.f,3},
             {.89f,.018f,.97f,.025f,9.f,4}, {.75f,.978f,.94f,.966f,10.f,4},
             {.15f,.185f,.85f,.210f,2.8f,1}, {.20f,.331f,.94f,.383f,3.f,2},
             {.43f,.392f,.88f,.397f,2.5f,3}, {.56f,.434f,.87f,.430f,2.3f,1},
             {.10f,.522f,.50f,.555f,2.6f,3}, {.15f,.551f,.77f,.590f,2.8f,4},
             {.10f,.610f,.65f,.596f,2.7f,2}, {.18f,.670f,.70f,.731f,2.8f,3},
             {.50f,.751f,.91f,.737f,2.8f,1}, {.14f,.811f,.87f,.796f,2.8f,2},
             {.42f,.861f,.85f,.833f,2.5f,4}, {.35f,.903f,.80f,.877f,2.5f,3},
             {.30f,.941f,.51f,.952f,2.3f,2}}
        }};
        for(int side=0;side<2;++side) {
            juce::Image worn(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
            {juce::Graphics g(worn);g.drawImage(atlas.getClippedImage(side?juce::Rectangle<int>(704,15,132,1435):juce::Rectangle<int>(233,15,137,1435)),worn.getBounds().toFloat());}
            const juce::Image::BitmapData fresh(images[size_t(side)][0],juce::Image::BitmapData::readOnly);
            const juce::Image::BitmapData photograph(worn,juce::Image::BitmapData::readOnly);
            // Preserve the original photographic plastic, fine grain and
            // damage colour. Remove scheduled marks only from the undercoat;
            // reveal their original photo detail at each fixed birth stage.
            const int w=worn.getWidth(),h=worn.getHeight(),n=w*h;
            std::array<std::vector<float>,5> masks;
            for(auto& mask:masks)mask.resize(size_t(n),0.f);
            for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                const juce::Point<float> point((x+.5f)/scale,(y+.5f)/scale);
                for(const auto& mark:marks[size_t(side)]) {
                    const juce::Point<float> a(mark.x1*width,mark.y1*height),b(mark.x2*width,mark.y2*height);
                    const auto line=b-a;const float length=line.getDistanceSquaredFromOrigin();
                    const float along=length>0?juce::jlimit(0.f,1.f,((point.x-a.x)*line.x+(point.y-a.y)*line.y)/length):0.f;
                    const float distance=point.getDistanceFrom(a+line*along);
                    const float cover=1.f-power_detail::smoothRange(mark.radius-1.2f,mark.radius,distance);
                    for(int stage=mark.birth;stage<5;++stage)masks[size_t(stage)][size_t(y*w+x)]=juce::jmax(masks[size_t(stage)][size_t(y*w+x)],cover);
                }
            }
            // Copy intact photographic plastic from the same horizontal
            // position. Preserve real grain and the moulding's light profile;
            // do not smear a chip's shadow or edge through an averaged fill.
            std::vector<juce::Colour> undercoat(size_t(n),juce::Colours::transparentBlack);
            for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                const size_t at=size_t(y*w+x);
                if(masks[4][at]<=0){undercoat[at]=photograph.getPixelColour(x,y);continue;}
                int sy=-1;
                for(int offset:{137,-137,229,-229,317,-317,431,-431,613,-613}) {
                    const int candidate=y+offset;
                    if(candidate>60&&candidate<h-60&&masks[4][size_t(candidate*w+x)]<=0){sy=candidate;break;}
                }
                const auto original=fresh.getPixelColour(x,y);
                if(sy<0){undercoat[at]=original;continue;}
                const auto sample=photograph.getPixelColour(x,sy),reference=fresh.getPixelColour(x,sy);
                const float ratio=juce::jlimit(.15f,1.7f,original.getBrightness()/juce::jmax(.025f,reference.getBrightness()));
                const float tone=(sample.getFloatRed()+sample.getFloatGreen()+sample.getFloatBlue())/3.f;
                undercoat[at]=juce::Colour::fromFloatRGBA(juce::jlimit(0.f,1.f,tone*ratio*.99f),
                    juce::jlimit(0.f,1.f,tone*ratio*1.01f),juce::jlimit(0.f,1.f,tone*ratio),original.getFloatAlpha());
            }
            for(int stage=1;stage<5;++stage) {
                auto& result=images[size_t(side)][size_t(stage)];
                result=juce::Image(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
                juce::Image::BitmapData output(result,juce::Image::BitmapData::writeOnly);
                for(int y=0;y<result.getHeight();++y)for(int x=0;x<result.getWidth();++x) {
                    const auto clean=fresh.getPixelColour(x,y),photo=photograph.getPixelColour(x,y);
                    const size_t at=size_t(y*w+x);
                    const float all=masks[4][at],revealed=all>0?masks[size_t(stage)][at]/all:0.f;
                    const auto body=photo.interpolatedWith(undercoat[at],all);
                    const float coverage=.5f*photo.getFloatAlpha(),progress=float(stage)*.25f;
                    const auto channel=[&](float c,float damaged,float paint) {
                        // At max AGE this resolves exactly to the earlier
                        // photographic endpoint, with no capsule-shaped colour
                        // replacement or flat global average underneath a cut.
                        return juce::jlimit(0.f,1.f,c+coverage*(progress*(paint-c)+revealed*(damaged-paint)));
                    };
                    // Full fixed-contrast local cuts are added one at a time.
                    output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                        channel(clean.getFloatRed(),photo.getFloatRed(),body.getFloatRed()),
                        channel(clean.getFloatGreen(),photo.getFloatGreen(),body.getFloatGreen()),
                        channel(clean.getFloatBlue(),photo.getFloatBlue(),body.getFloatBlue()),clean.getFloatAlpha()));
                }
            }
        }
    }
    void paint(juce::Graphics& g,int side,float age,float y,float totalWidth) const {
        juce::Graphics::ScopedSaveState save(g);
        const float x=side?totalWidth-width:0.f;
        const juce::Rectangle<float> bounds(x,y,float(width),float(height));
        g.setColour(juce::Colour(0xff0a0d0e));g.fillRect(bounds);
        juce::Path outline;outline.addRoundedRectangle(bounds,12);g.reduceClipRegion(outline);
        g.setOpacity(1);g.drawImage(images[size_t(side)][size_t(appearanceStage(age))],bounds);
        if(side) {g.setColour(juce::Colours::black.withAlpha(.12f));g.fillPath(outline);}
        // The independently generated photo-informed endpoint already holds
        // each cheek's cuts, scuffs and larger corner damage. Do not stamp a
        // second native damage pattern over it or mirror either crop.

    }
private:
    std::array<std::array<juce::Image,5>,2> images;
};
inline const CheekMaterial& cheekMaterial() {static const CheekMaterial material;return material;}
}
