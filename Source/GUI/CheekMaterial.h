#pragma once
#include "PowerDetails.h"
#include "BinaryData.h"
#include "AgeAppearance.h"

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
        struct Patch {float x,y,rx,ry;int firstStage;};
        const std::array<std::array<Patch,14>,2> patches{{
            {{{.28f,.27f,.30f,.025f,1},{.45f,.52f,.31f,.035f,1},{.34f,.77f,.31f,.025f,1},
              {.96f,.21f,.23f,.035f,2},{.41f,.34f,.61f,.085f,2},{.06f,.44f,.23f,.035f,2},
              {.51f,.61f,.60f,.05f,2},{.54f,.83f,.52f,.04f,2},{.24f,.072f,.45f,.06f,3},
              {.55f,.40f,.61f,.06f,3},{.46f,.71f,.51f,.04f,3},{.55f,.90f,.65f,.04f,3},
              {.25f,.98f,.65f,.05f,4},{.70f,.02f,.65f,.04f,4}}},
            {{{.40f,.14f,.31f,.04f,1},{.52f,.36f,.41f,.035f,1},{.52f,.65f,.43f,.035f,1},
              {.96f,.11f,.25f,.035f,2},{.52f,.25f,.65f,.075f,2},{.98f,.47f,.28f,.04f,2},
              {.43f,.56f,.57f,.07f,2},{.62f,.79f,.52f,.035f,2},{.77f,.025f,.46f,.05f,3},
              {.95f,.28f,.25f,.04f,3},{.97f,.66f,.26f,.035f,3},{.51f,.74f,.60f,.07f,3},
              {.72f,.98f,.65f,.05f,4},{.52f,.90f,.65f,.055f,4}}}
        }};
        for(int side=0;side<2;++side) {
            juce::Image worn(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
            {juce::Graphics g(worn);g.drawImage(atlas.getClippedImage(side?juce::Rectangle<int>(704,15,132,1435):juce::Rectangle<int>(233,15,137,1435)),worn.getBounds().toFloat());}
            auto diffuse=worn.createCopy();juce::ImageConvolutionKernel blur(31);
            blur.createGaussianBlur(9);blur.applyToImage(diffuse,worn,worn.getBounds());
            auto detail=worn.createCopy();juce::ImageConvolutionKernel deGrain(7);
            deGrain.createGaussianBlur(2);deGrain.applyToImage(detail,worn,worn.getBounds());
            const juce::Image::BitmapData fresh(images[size_t(side)][0],juce::Image::BitmapData::readOnly);
            const juce::Image::BitmapData photograph(worn,juce::Image::BitmapData::readOnly),matte(diffuse,juce::Image::BitmapData::readOnly);
            const juce::Image::BitmapData scarDetail(detail,juce::Image::BitmapData::readOnly);
            for(int stage=1;stage<5;++stage) {
                auto& result=images[size_t(side)][size_t(stage)];
                result=juce::Image(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
                juce::Image::BitmapData output(result,juce::Image::BitmapData::writeOnly);
                for(int y=0;y<result.getHeight();++y)for(int x=0;x<result.getWidth();++x) {
                    const auto clean=fresh.getPixelColour(x,y),photo=photograph.getPixelColour(x,y);
                    const float u=(x+.5f)/result.getWidth(),v=(y+.5f)/result.getHeight();
                    // Assign a scar a birth stage, then retain its final contrast.
                    // Fixed local patches accumulate; no opacity ramp on damage.
                    int birth=4;
                    for(const auto& patch:patches[size_t(side)]) {
                        const float dx=(u-patch.x)/patch.rx,dy=(v-patch.y)/patch.ry;
                        if(dx*dx+dy*dy<=1.f)birth=juce::jmin(birth,patch.firstStage);
                    }
                    const auto low=matte.getPixelColour(x,y);
                    // Separate actual cuts from the fine moulding grain before
                    // assigning damage stages; grain must never form reveal patches.
                    const auto cut=scarDetail.getPixelColour(x,y);
                    const float difference=juce::jmax(std::abs(cut.getFloatRed()-low.getFloatRed()),
                        std::abs(cut.getFloatGreen()-low.getFloatGreen()),std::abs(cut.getFloatBlue()-low.getFloatBlue()));
                    const float scar=power_detail::smoothRange(.04f,.10f,difference);
                    const auto plastic=photo.interpolatedWith(low,scar);
                    const float coverage=.5f*photo.getFloatAlpha(),progress=float(stage)*.25f;
                    const auto channel=[&](float c,float material,float damaged) {
                        const float base=c+coverage*progress*(material-c);
                        const float fixedScar=c+coverage*(damaged-c);
                        return juce::jlimit(0.f,1.f,base+(stage>=birth?scar:0.f)*(fixedScar-base));
                    };
                    // Only low-contrast plastic dulling changes with age. Each
                    // strong scratch appears at full contrast and stays there.
                    output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                        channel(clean.getFloatRed(),plastic.getFloatRed(),photo.getFloatRed()),
                        channel(clean.getFloatGreen(),plastic.getFloatGreen(),photo.getFloatGreen()),
                        channel(clean.getFloatBlue(),plastic.getFloatBlue(),photo.getFloatBlue()),clean.getFloatAlpha()));
                }
            }
        }
    }
    void paint(juce::Graphics& g,int side,float age,float y) const {
        juce::Graphics::ScopedSaveState save(g);
        const float x=side?1445.f:0.f;
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
