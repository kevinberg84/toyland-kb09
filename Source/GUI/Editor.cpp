#include "Editor.h"
#include "KeyboardFeedback.h"
#include "PresetFiles.h"
#include "PowerDetails.h"
#include "CheekMaterial.h"
#include "BenderMaterial.h"
#include "FaderDetails.h"
#include "RustMaterial.h"
#include "AgeAppearance.h"
#include "UpdateChecker.h"
#include "AudioSettingsDialog.h"
#include "DialogMessages.h"
#include "../Plugin/Processor.h"
#include "BinaryData.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#include <cmath>
#include <vector>
#include <array>

namespace {
using kb09_gui::appearanceAge;
constexpr int designW=1483, bodyH=750, visibleBodyH=732, headerH=34, designH=816;
constexpr float panelViewSin=.190809f,panelViewCos=.981627f;
const juce::Colour ink(0xffe9e7dd), muted(0xffb4b7b5), amber(0xffdbb572);
juce::Colour agedInk(float age) {
    return ink.interpolatedWith(juce::Colour(0xffd1bd86),juce::jlimit(0.0f,1.0f,age)*.73f);
}
juce::Font font(float size,bool bold=false) {
    return juce::Font(juce::FontOptions("Arial",size,bold?juce::Font::bold:juce::Font::plain));
}
void caption(juce::Graphics& g,const juce::String& text,juce::Rectangle<float> box,
             float size=10.0f,juce::Justification align=juce::Justification::centred,bool bold=true,
             juce::Colour colour=ink) {
    g.setColour(colour);g.setFont(font(size,bold));
    g.drawFittedText(text,box.toNearestInt(),align,text.containsChar('\n')?2:1,.8f);
}
enum class PanelGlyph { sine, doubleTriangle, saw, square, pulse, envelope, invertedEnvelope, gate };
void panelGlyph(juce::Graphics& g,PanelGlyph shape,juce::Rectangle<float> box,
                juce::Colour colour,float stroke=.9f) {
    box=box.withSizeKeepingCentre(box.getWidth()*.72f,box.getHeight());
    juce::Path path;
    const auto point=[&](float x,float y) {return juce::Point<float>(box.getX()+x*box.getWidth(),box.getY()+y*box.getHeight());};
    const auto start=[&](float x,float y) {path.startNewSubPath(point(x,y));};
    const auto line=[&](float x,float y) {path.lineTo(point(x,y));};
    if(shape==PanelGlyph::sine) {
        for(int i=0;i<=32;++i) {
            const float x=float(i)/32,y=.5f-.44f*std::sin(x*juce::MathConstants<float>::twoPi);
            if(i==0)start(x,y);else line(x,y);
        }
    } else if(shape==PanelGlyph::doubleTriangle) {
        start(0,1);line(.25f,0);line(.5f,1);line(.75f,0);line(1,1);
    } else if(shape==PanelGlyph::saw) {
        start(0,1);line(1,0);line(1,1);
    } else if(shape==PanelGlyph::envelope||shape==PanelGlyph::invertedEnvelope) {
        const bool inverted=shape==PanelGlyph::invertedEnvelope;
        if(inverted) {
            start(0,.05f);line(.18f,.95f);line(.30f,.61f);line(.45f,.66f);
            line(.61f,.38f);line(.78f,.34f);line(1,.10f);
        } else {
            start(0,.9f);line(.10f,.9f);line(.25f,.1f);
            path.cubicTo(point(.38f,.62f),point(.57f,.83f),point(1,.9f));
        }
    } else if(shape==PanelGlyph::gate) {
        start(0,.9f);line(.2f,.9f);line(.2f,.1f);line(.8f,.1f);line(.8f,.9f);line(1,.9f);
    } else {
        const float end=shape==PanelGlyph::pulse?.27f:.46f;
        start(0,.9f);line(0,.1f);line(end,.1f);line(end,.9f);line(1,.9f);if(shape!=PanelGlyph::gate)line(1,.1f);
    }
    g.setColour(colour);g.strokePath(path,juce::PathStrokeType(stroke,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
}
struct Artwork {
    static juce::Image load(const void* data,int size) {
        // Keep persistent artwork in CPU memory. Native Windows images otherwise
        // retain Direct2D resources across editor destruction and DLL unloading.
        return juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(data,size));
    }
    juce::Image chassis=load(BinaryData::chassisblack_png,BinaryData::chassisblack_pngSize);
    juce::Image chassisAged=load(BinaryData::chassispersonalaged_png,BinaryData::chassispersonalaged_pngSize);
    std::array<juce::Image,5> panelWear;
    juce::Image stains=load(BinaryData::chassisstains_png,BinaryData::chassisstains_pngSize);
    // Normalize the new sprite's padding so its silver skirt keeps the previous
    // on-panel width. Only the upper knob is narrower, as in the owner's photo.
    juce::Image knob=load(BinaryData::knobpersonaloverheadv5_png,BinaryData::knobpersonaloverheadv5_pngSize)
        .getClippedImage({70,70,1114,1114});
    juce::Image fader=load(BinaryData::fadersmoothoverheadv1_png,BinaryData::fadersmoothoverheadv1_pngSize);
    juce::Image faderDull;
    juce::Image rail=load(BinaryData::frontrailgrain_png,BinaryData::frontrailgrain_pngSize);
    juce::Image keys=load(BinaryData::keys_png,BinaryData::keys_pngSize);
    juce::Image whiteKey=keys.getClippedImage({303,65,198,1070});
    juce::Image blackKey=keys.getClippedImage({763,68,159,615});
    juce::Image marks,marksAged;
    Artwork() {
        panelWear=kb09_gui::makePanelWearStages(chassisAged,chassis);
        // Smooth photographed plastic keeps its broad lighting. Compress the
        // fresh highlight for the dull endpoint; the silver insert is native.
        faderDull=fader.createCopy();
        juce::Image::BitmapData dull(faderDull,juce::Image::BitmapData::readWrite);
        for(int y=0;y<faderDull.getHeight();++y)for(int x=0;x<faderDull.getWidth();++x) {
            const auto c=dull.getPixelColour(x,y);
            const float value=(c.getFloatRed()+c.getFloatGreen()+c.getFloatBlue())/3.f;
            const float matte=.035f+.42f*value;
            dull.setPixelColour(x,y,juce::Colour::fromFloatRGBA(matte*.98f,matte,matte,c.getFloatAlpha()));
        }
        auto original=load(BinaryData::marks_png,BinaryData::marks_pngSize);
        auto worn=load(BinaryData::marksaged_png,BinaryData::marksaged_pngSize);
        marks=juce::Image(juce::Image::ARGB,original.getWidth(),original.getHeight(),true,juce::SoftwareImageType());
        marksAged=juce::Image(juce::Image::ARGB,original.getWidth(),original.getHeight(),true,juce::SoftwareImageType());
        juce::Image::BitmapData input(original,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData output(marks,juce::Image::BitmapData::writeOnly);
        juce::Image::BitmapData agedInput(worn,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData agedOutput(marksAged,juce::Image::BitmapData::writeOnly);
        // Preserve the supplied mark shapes; their white coverage becomes screen-print opacity.
        for(int y=0;y<original.getHeight();++y)for(int x=0;x<original.getWidth();++x) {
            const auto c=input.getPixelColour(x,y);
            const float light=(c.getFloatRed()+c.getFloatGreen()+c.getFloatBlue())/3.0f;
            const float coverage=juce::jlimit(0.0f,1.0f,(light-.065f)/.89f);
            output.setPixelColour(x,y,ink.withAlpha(coverage));
            const auto aged=agedInput.getPixelColour(x*worn.getWidth()/original.getWidth(),y*worn.getHeight()/original.getHeight());
            const float wornLight=(aged.getFloatRed()+aged.getFloatGreen()+aged.getFloatBlue())/3.0f;
            // The generated wear can only remove original ink, never change
            // the supplied letterforms or introduce coverage outside them.
            agedOutput.setPixelColour(x,y,agedInk(1).withAlpha(coverage*juce::jlimit(0.0f,1.0f,(wornLight-.07f)/.83f)));
        }
    }
};
const Artwork& artwork() {static const Artwork a;return a;}
// Keep component caches in CPU memory too, including offscreen host editors.
// A host's temporary graphics context must not make our caches retain GPU
// resources across plug-in unload. Repaints invalidate only the affected area.
class SoftwareComponentCache final : public juce::CachedComponentImage {
public:
    explicit SoftwareComponentCache(juce::Component& component):owner(component) {}
    void paint(juce::Graphics& g) override {
        const auto bounds=owner.getLocalBounds();
        const float scale=juce::jlimit(1.f,4.f,g.getInternalContext().getPhysicalPixelScaleFactor());
        const int width=juce::jmax(1,juce::roundToInt(bounds.getWidth()*scale));
        const int height=juce::jmax(1,juce::roundToInt(bounds.getHeight()*scale));
        if(image.getWidth()!=width||image.getHeight()!=height) {
            image=juce::Image(juce::Image::ARGB,width,height,true,juce::SoftwareImageType());valid.clear();
        }
        if(!valid.containsRectangle(bounds)) {
            juce::RectangleList<int> dirty(bounds);dirty.subtract(valid);
            juce::RectangleList<int> pixels;
            for(const auto& area:dirty)
                pixels.add((area.toFloat()*scale).getSmallestIntegerContainer());
            for(const auto& area:pixels)image.clear(area);
            juce::Graphics raster(image);raster.reduceClipRegion(pixels);
            raster.addTransform(juce::AffineTransform::scale(scale));
            // Image blits inherit the current fill's opacity. Reset it after
            // clearing. Clip in physical pixels to avoid half-pixel cache seams
            // at fractional Windows display scales.
            raster.setColour(juce::Colours::black);
            owner.paintEntireComponent(raster,true);valid=bounds;
        }
        g.setOpacity(owner.getAlpha());g.drawImage(image,bounds.toFloat());
    }
    bool invalidateAll() override {valid.clear();return true;}
    bool invalidate(const juce::Rectangle<int>& area) override {valid.subtract(area);return true;}
    void releaseResources() override {image={};valid.clear();}
private:
    juce::Component& owner;juce::Image image;juce::RectangleList<int> valid;
};
void bufferInSoftware(juce::Component& component,bool enabled) {
    if(enabled) {
        if(dynamic_cast<SoftwareComponentCache*>(component.getCachedComponentImage())==nullptr)
            component.setCachedComponentImage(new SoftwareComponentCache(component));
    } else component.setCachedComponentImage(nullptr);
}
void caseStains(juce::Graphics& g,float age) {
    if(age<.60f)return;
    juce::Graphics::ScopedSaveState save(g);
    juce::Path metal;metal.addRectangle(55.0f,float(headerH),1389.0f,365.0f);
    metal.addRectangle(65.0f,float(headerH+366),257.0f,335.0f);g.reduceClipRegion(metal);
    g.setOpacity(.22f);
    {
        // Preserve the user's favourite envelope coffee ring. All other old
        // generic case marks have been superseded by their personal unit.
        const juce::Rectangle<int> box(1420,0,300,320);
        const auto target=juce::Rectangle<float>((box.getX()-2)*1500.0f/1770+40,
            headerH+(box.getY()-7)*750.0f/873-45,box.getWidth()*1500.0f/1770,box.getHeight()*750.0f/873);
        g.drawImage(artwork().stains,target.getX(),target.getY(),target.getWidth(),target.getHeight(),
                    box.getX(),box.getY(),box.getWidth(),box.getHeight());
    }
}

// Fixed, independent microgeometry for each surface. Age changes dust density
// through opacity, never enlarges a stain or stamps the same texture elsewhere.
uint32_t surfaceHash(uint32_t value) {
    value^=value>>16;value*=0x7feb352du;value^=value>>15;value*=0x846ca68bu;return value^(value>>16);
}
float surfaceNoise(uint32_t seed) {return float(surfaceHash(seed)&0xffffu)/65535.0f;}
// Photo-informed deposits stay registered to the panel as AGE changes.
// Cache the grain once so aging never generates a moving random dust field.
const juce::Image& panelDust() {
    static const juce::Image image=[] {
        juce::Image result(juce::Image::ARGB,designW*2,designH*2,true,juce::SoftwareImageType());
        juce::Graphics g(result);g.addTransform(juce::AffineTransform::scale(2.0f));
        juce::Path metal;metal.addRectangle(60.0f,65.0f,1380.0f,326.0f);
        metal.addRectangle(69.0f,404.0f,249.0f,335.0f);g.reduceClipRegion(metal);
        // Density varies near controls; every visible deposit is a discrete
        // grain. No blurred gradients or translucent clouds cover the paint.
        for(uint32_t i=0;i<26000;++i) {
            const uint32_t seed=surfaceHash(880301u+i*7919u);
            float x=0,y=0;
            if(i%4<2) {
                // Narrow, uneven deposits hugging the two collars and the
                // sheltered bridge between them, never a cloud below a knob.
                const float cx=i%2?356.0f:456.0f,cy=203.4f;
                const float a=surfaceNoise(seed+17)*juce::MathConstants<float>::twoPi;
                const float radius=38.0f+7.0f*std::pow(surfaceNoise(seed+31),2.4f);
                x=cx+std::cos(a)*radius;y=cy+std::sin(a)*radius;
                if(surfaceNoise(seed+53)>juce::jlimit(0.0f,1.0f,(cy+35-y)/13))continue;
            } else if(i%4==2) {
                // Soft irregular density field joining the sheltered collar edges.
                x=382+62*surfaceNoise(seed+17);y=169+66*surfaceNoise(seed+31);
                const float dx=(x-411)/23,dy=(y-201-5*std::sin(x*.19f))/15;
                const float mottling=.18f+.82f*kb09_gui::power_detail::smoothNoise(x*.22f,y*.23f,6181u);
                if(surfaceNoise(seed+47)>.44f*std::exp(-dx*dx-dy*dy)*mottling)continue;
            } else {
                // Performance bay perimeter, PORTAMENTO recess corners and
                // bender opening lips. Keep its broad open paint clear.
                const int edge=int(surfaceNoise(seed+17)*8);
                if(edge>=3&&edge<=5)continue;
                const float t=surfaceNoise(seed+31),inset=1+5*std::pow(surfaceNoise(seed+47),2);
                if(edge==0){x=70+inset;y=409+322*t;}
                else if(edge==1){x=302-inset;y=409+322*t;}
                else if(edge==2){x=72+226*t;y=738-inset;}
                else if(edge==3){x=231+inset;y=526+128*t;}
                else if(edge==4){x=310-inset;y=526+128*t;}
                else if(edge==5){x=233+75*t;y=654-inset;}
                else {
                    x=102+165*t;
                    const float centreY=headerH+(edge==6?643.f:672.f);
                    // Feathered grain density, uneven along the lip; no hard
                    // band edge or repeated straight cut through the powder.
                    const float offset=(surfaceNoise(seed+67)-.5f)*12.f;
                    const float density=std::exp(-offset*offset/12.f)
                        *(.25f+.75f*kb09_gui::power_detail::smoothNoise(x*.11f,centreY*.08f,7193u));
                    if(surfaceNoise(seed+71)>density)continue;
                    y=centreY+offset+1.1f*std::sin(x*.093f);
                }
                x+=8;
            }
            if(i%4!=3)x-=13;
            // A fine powder, occasional lint, and sparse brighter grains.
            const float size=.27f+.45f*std::pow(surfaceNoise(seed+89),2.0f);
            const float tone=surfaceNoise(seed+101);
            g.setColour(juce::Colour(0xffc2c3b9).interpolatedWith(juce::Colour(0xff797f7b),tone)
                        .withAlpha(.32f+.36f*surfaceNoise(seed+113)));
            g.fillEllipse(x,y,size*(i%79==0?2.7f:1.0f),size);
        }
        return result;
    }();
    return image;
}
const juce::Image& railDust() {
    static const juce::Image layer=[] {
        juce::Image image(juce::Image::ARGB,2764,86,true,juce::SoftwareImageType());juce::Graphics g(image);
        g.addTransform(juce::AffineTransform::scale(2.0f));
        for(uint32_t i=0;i<145000;++i) {
            const auto seed=surfaceHash(113879u+i*3571u);
            const float x=1382*surfaceNoise(seed),v=surfaceNoise(seed+19);
            // Trapped powder in the pebble recesses and the sheltered keybed edge.
            const float y=43*(i%3==0?v*v:v);
            const float size=.26f+.54f*surfaceNoise(seed+31);
            g.setColour(juce::Colour(0xffb5afa0).interpolatedWith(juce::Colour(0xff77796f),surfaceNoise(seed+47))
                        .withAlpha(.40f+.47f*surfaceNoise(seed+61)));
            g.fillEllipse(x,y,size*(i%107==0?2.4f:1),size*.70f);
        }
        return image;
    }();return layer;
}
void paintPanelDust(juce::Graphics& g,float age) {
    if(age<=0)return;
    juce::Graphics::ScopedSaveState save(g);
    g.setOpacity(std::pow(juce::jlimit(0.0f,1.0f,age),1.35f));
    g.drawImage(panelDust(),juce::Rectangle<float>(0,0,designW,designH));
}
struct KnobMetalWear {
    juce::Image matte;
    std::array<juce::Image,3> pits;
    KnobMetalWear() {
        const auto& photo=artwork().knob;
        const int width=photo.getWidth(),height=photo.getHeight();
        matte=juce::Image(juce::Image::ARGB,width,height,true,juce::SoftwareImageType());
        for(auto& image:pits)image=juce::Image(juce::Image::ARGB,width,height,true,juce::SoftwareImageType());
        const juce::Image::BitmapData input(photo,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData dull(matte,juce::Image::BitmapData::readWrite);
        for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
            const float u=(float(x)+.5f)/width,v=(float(y)+.5f)/height;
            const auto colour=input.getPixelColour(x,y);
            const float tx=(u-.497f)/.248f,ty=(v-.410f)/.258f;
            const float sx=(u-.500f)/.455f,sy=(v-.505f)/.455f;
            const float gx=(u-.500f)/.326f,gy=(v-juce::jlimit(.422f,.478f,v))/.326f;
            const bool metal=tx*tx+ty*ty<1||(sx*sx+sy*sy<1&&gx*gx+gy*gy>1);
            // Keep oxidation off the opaque grip and the transparent outline.
            if(!metal||colour.getAlpha()<240||colour.getBrightness()<.30f)continue;
            const float light=(colour.getFloatRed()+colour.getFloatGreen()+colour.getFloatBlue())/3;
            const bool top=tx*tx+ty*ty<1;
            dull.setPixelColour(x,y,top?juce::Colour::fromFloatRGBA(.35f+.22f*light,.33f+.21f*light,.30f+.19f*light,1)
                :juce::Colour::fromFloatRGBA(.36f+.19f*light,.37f+.19f*light,.36f+.19f*light,1));
        }
        // This fixed material field is generated once. AGE changes only its
        // opacity, so neither pitting locations nor grain jump as AGE is moved.
        constexpr std::array<uint32_t,3> seeds{{12011u,41117u,93973u}};
        for(size_t index=0;index<pits.size();++index) {
            juce::Image::BitmapData output(pits[index],juce::Image::BitmapData::writeOnly);
            for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
                const float topX=((float(x)+.5f)/width-.497f)/.248f;
                const float topY=((float(y)+.5f)/height-.410f)/.258f;
                if(dull.getPixelColour(x,y).getAlpha()==0||topX*topX+topY*topY>=1)continue;
                const float u=(float(x)+.5f)/width*172,v=(float(y)+.5f)/height*172;
                const int cellX=int(u),cellY=int(v);
                float cavity=0,lip=0;uint32_t pitKey=0;
                // Search neighbouring cells so fully jittered pit centres can
                // overlap cell edges without a visible grid or squared cuts.
                for(int row=-1;row<=1;++row)for(int column=-1;column<=1;++column) {
                    const int cx=cellX+column,cy=cellY+row;
                    const uint32_t key=surfaceHash(seeds[index]+uint32_t(cx)*7919u+uint32_t(cy)*104729u);
                    if(surfaceNoise(key+19)>.72f)continue;
                    const float dx=u-cx-surfaceNoise(key+31),dy=v-cy-surfaceNoise(key+47);
                    const float radius=.12f+.32f*surfaceNoise(key+61),aspect=.65f+.55f*surfaceNoise(key+73);
                    const float distance=std::sqrt(dx*dx+dy*dy/(aspect*aspect))/radius;
                    const float depth=juce::jlimit(0.0f,1.0f,(1.12f-distance)/.25f);
                    if(depth>cavity){cavity=depth;pitKey=key;}
                    lip=juce::jmax(lip,juce::jlimit(0.0f,1.0f,1-std::abs(distance-1.12f)/.15f)
                        *juce::jlimit(0.0f,1.0f,(dx+dy)/radius));
                }
                if(cavity>0) {
                    const float tone=surfaceNoise(pitKey+89);
                    output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(.25f+.14f*tone,.22f+.12f*tone,
                        .18f+.10f*tone,cavity*(.78f+.17f*surfaceNoise(pitKey+97))));
                } else if(lip>0)output.setPixelColour(x,y,juce::Colour(0xffc3b7a0).withAlpha(lip*.32f));
            }
        }
    }
};
const KnobMetalWear& knobMetalWear() {static const KnobMetalWear material;return material;}
void keySurfaceWear(juce::Graphics& g,juce::Rectangle<float> r,int serial,bool black,float age) {
    juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(r.reduced(.8f).toNearestInt());
    const auto seed=uint32_t(serial+1)*7919u;
    if(black&&age>0) {
        juce::Graphics::ScopedSaveState topOnly(g);g.reduceClipRegion(r.withTrimmedBottom(17).reduced(1).toNearestInt());
        for(uint32_t mark=0;mark<uint32_t(9*age);++mark) {
            const float x=r.getX()+r.getWidth()*surfaceNoise(seed+mark*317),y=r.getY()+r.getHeight()*.83f*surfaceNoise(seed+mark*971);
            const float radius=3+7*surfaceNoise(seed+mark*41);
            juce::ColourGradient rub(juce::Colour(mark%2?0xff777771:0xff050706).withAlpha(.11f),x,y,
                juce::Colours::transparentBlack,x+radius,y+radius,true);
            g.setGradientFill(rub);g.fillEllipse(x-radius,y-radius,radius*2,radius*2);
        }
        for(uint32_t nick=0;nick<uint32_t(28*age);++nick) {
            const float x=r.getX()+r.getWidth()*(.06f+.87f*surfaceNoise(seed+nick*89)),y=r.getY()+r.getHeight()*.85f*surfaceNoise(seed+nick*701);
            g.setColour(juce::Colour(nick%3?0xff9b9b91:0xff020403).withAlpha(.23f));
            g.drawLine(x,y,x+.3f+1.6f*surfaceNoise(seed+nick*113),y+.4f*surfaceNoise(seed+nick*29),.19f);
        }
    }
    if(age>0) {
        juce::Graphics::ScopedSaveState scratched(g);
        g.reduceClipRegion(r.withTrimmedBottom(black?17.f:6.f).reduced(1).toNearestInt());
        // AGE adds fixed-strength marks in equal count increments. Their paths
        // never move, lengthen or fade as later marks are added.
        const float ivoryWear=black?0.f:1.f;
        for(uint32_t scratch=0;scratch<uint32_t((black?54.f:55.f)*age);++scratch) {
            const float x=r.getX()+r.getWidth()*(.14f+.68f*surfaceNoise(seed+scratch*127));
            const float y=r.getY()+r.getHeight()*(black?(.12f+.68f*surfaceNoise(seed+scratch*419)):(.68f+.25f*surfaceNoise(seed+scratch*419)));
            const float dx=(surfaceNoise(seed+scratch*199)-.5f)*16,dy=1+(9+2*ivoryWear)*surfaceNoise(seed+scratch*733);
            juce::Path trace;trace.startNewSubPath(x,y);trace.quadraticTo(x+dx*.2f+1,y+dy*.5f,x+dx,y+dy);
            g.setColour(juce::Colour(black?0xffb7b9ae:0xff696e68).withAlpha((1+.65f*ivoryWear)*(.12f+.12f*surfaceNoise(seed+scratch*37))));
            g.strokePath(trace,juce::PathStrokeType(.11f+.08f*surfaceNoise(seed+scratch*59)+.025f*ivoryWear,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
    }
    // Fine dust follows protected back edges, gaps and the long side bevels.
    // Each particle is smaller than a body pixel; 4K rasterisation resolves it.
    for(uint32_t i=0;i<38;++i) {
        const float nx=surfaceNoise(seed+i*19),ny=surfaceNoise(seed+i*43+7);
        const bool edge=i%3==0;
        const float xx=r.getX()+(edge?(i%2?.025f:.95f):nx*.94f+.03f)*r.getWidth();
        const float yy=r.getY()+(edge?ny*.94f:ny*ny*.24f)*r.getHeight();
        const float size=.17f+.24f*surfaceNoise(seed+i*23+13);
        g.setColour(juce::Colour(black?0xffb7b0a0:0xff837861).withAlpha(age*(black?.35f:.24f)));
        g.fillEllipse(xx,yy,size,size*.72f);
        if(i%17==0)g.drawLine(xx,yy,xx+.7f,yy+.2f,.13f);
    }
    // Individual grime specks appear once at fixed positions. They are tiny
    // irregular pigment/soil inclusions, following the owner's key close-ups.
    if(age>=.18f+float(serial)*.023f)for(uint32_t i=0;i<7;++i) {
        const float xx=r.getX()+(.10f+.78f*surfaceNoise(seed+i*67+29))*r.getWidth();
        const float yy=r.getY()+(.30f+.59f*surfaceNoise(seed+i*47+151))*r.getHeight();
        const float radius=.22f+.48f*surfaceNoise(seed+i*31+59);
        g.setColour(juce::Colour(black?0xffb5a48a:0xff6f5634).withAlpha(black?.13f:.17f));
        g.fillEllipse(xx,yy,radius*(.6f+surfaceNoise(seed+i*11)),radius);
        g.setColour(juce::Colour(black?0xff8a7b66:0xff9a7c4b).withAlpha(.045f));
        g.fillEllipse(xx-radius,yy-radius,radius*3,radius*2.6f);
    }
}

void sliderPost(juce::Graphics& g,juce::Rectangle<float> capBounds,float age,bool releasePost,float lean=0,uint32_t wearSeed=0) {
    // The blade's broad plane follows the slot. Seen almost from above and
    // without camera yaw, its narrow longitudinal edge is the visible surface.
    // The owner's photos establish this orientation, not physical dimensions.
    juce::Graphics::ScopedSaveState save(g);g.setOpacity(1.0f);
    const float cx=capBounds.getCentreX();
    const float edgeLength=capBounds.getHeight()*.62f*panelViewCos;
    const float lift=7.0f*panelViewSin;
    const float top=capBounds.getCentreY()-edgeLength*.5f-lift;
    const float bottom=capBounds.getCentreY()+edgeLength*.5f;
    const float half=.85f,shoulder=1.15f,step=bottom-2.7f;
    // The reference shows the broad blade face in shadow, behind its narrow
    // bright cut edge. Keep both planes longitudinal, with a small end bevel.
    juce::Path side;side.startNewSubPath(cx-.15f+lean,top+.15f);
    side.lineTo(cx+2.25f+lean,top-1.0f);side.lineTo(cx+2.25f,bottom-1.2f);
    side.lineTo(cx-.15f,bottom);side.closeSubPath();
    g.setColour(juce::Colours::black.withAlpha(.55f));
    g.fillPath(side,juce::AffineTransform::translation(.65f,.8f));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff65665d),cx-.2f,top,
        juce::Colour(0xff202521),cx+2.25f,bottom,false));g.fillPath(side);
    g.setColour(juce::Colour(0xffaaa997));
    g.drawLine(cx-.15f+lean,top+.15f,cx+2.25f+lean,top-1.f,.35f);
    juce::Path edge;edge.startNewSubPath(cx-half+lean,top);
    edge.lineTo(cx+half+lean,top+.20f);edge.lineTo(cx+half,step);
    edge.lineTo(cx+shoulder,step+.25f);edge.lineTo(cx+shoulder,bottom);
    edge.lineTo(cx-shoulder,bottom);edge.lineTo(cx-shoulder,step-.20f);
    edge.lineTo(cx-half,step-.20f);edge.closeSubPath();
    // A short contact shadow gives height without exposing the broad blade face.
    g.setColour(juce::Colours::black.withAlpha(.43f));
    g.fillPath(edge,juce::AffineTransform::translation(.55f,lift*.62f));
    juce::ColourGradient metal(juce::Colour(0xffb8bbb2),cx-half,top,
                               juce::Colour(0xff767c73),cx+half,bottom,false);
    metal.addColour(releasePost?.36:.60,juce::Colour(0xffd0d1c5));
    g.setGradientFill(metal);g.fillPath(edge);
    g.setColour(juce::Colour(0xff777b70));
    g.drawLine(cx-half+lean,top+.35f,cx-half,step-.35f,.42f);
    // The cut edge is a thin lengthwise glint, not a wide horizontal paddle.
    g.setColour(juce::Colour(0xffd4d6cb));
    g.drawLine(cx-half+.30f+lean,top+.30f,cx-half+.30f,step-.45f,.40f);
    g.drawLine(cx-half+.12f+lean,top+.18f,cx+half-.12f+lean,top+.33f,.45f);
    g.setColour(juce::Colour(0xff92988f));
    g.drawLine(cx+half+.10f,step+.35f,cx+shoulder-.08f,step+.35f,.38f);
    g.setColour(juce::Colour(0xffb1b4a8).withAlpha(age*.20f));
    g.fillRect(cx-shoulder+.16f,step+.8f,shoulder*2-.32f,bottom-step-.9f);
    { juce::Graphics::ScopedSaveState rust(g);g.reduceClipRegion(edge);
      const uint32_t seed=wearSeed^(releasePost?19331u:7717u);
      for(uint32_t i=0;i<uint32_t(32*age);++i) {
          const float yy=top+(bottom-top)*surfaceNoise(seed+i*37);
          const float xx=cx-half+half*2*surfaceNoise(seed+i*61);
          g.setColour(juce::Colour(i%3?0xff74694d:0xff352d23).withAlpha(.32f));
          g.fillEllipse(xx,yy,.15f+.23f*surfaceNoise(seed+i*19),.35f);
      }
    }
    // Small, distinct interruptions keep the two blades from sharing a stamp.
    const float markY=top+(releasePost?.64f:.38f)*(bottom-top);
    g.setColour(juce::Colour(0xff373e37).withAlpha(.65f));
    g.drawLine(cx-.36f,markY,cx+.34f,markY+.28f,.34f);
}

void cap(juce::Graphics& g,juce::Rectangle<float> r,bool small=false,bool indexed=true,float age=0,uint32_t wearSeed=0) {
    const auto ink=agedInk(age);
    if(!small) {
        juce::Graphics::ScopedSaveState save(g);
        g.setOpacity(1.0f);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        // The cap's broad TOP now faces the camera. The central top index
        // continues over the two narrow sides, never over the front lip.
        const auto photo=juce::Rectangle<float>(r.getCentreX()-14,r.getCentreY()-9,28,18);
        const auto source=juce::Rectangle<int>(125,242,1240,565);
        juce::Path solid;solid.addRoundedRectangle(photo.reduced(.7f,.5f),2.0f);
        g.setColour(juce::Colour(0xff171719));g.fillPath(solid);
        g.drawImage(artwork().fader,photo.getX(),photo.getY(),photo.getWidth(),photo.getHeight(),
                    source.getX(),source.getY(),source.getWidth(),source.getHeight());
        if(age>0) {
            g.setColour(juce::Colours::black.withAlpha(age*.10f));
            g.fillPath(solid);
            // Silver loses its narrow photographic shine gradually; it neither
            // yellows with the printing nor acquires the exposed posts' rust.
            g.setOpacity(age*.90f);
            g.drawImage(artwork().faderDull,photo.getX(),photo.getY(),photo.getWidth(),photo.getHeight(),
                        source.getX(),source.getY(),source.getWidth(),source.getHeight());
        }
        g.setOpacity(1);
        const auto strip=juce::Rectangle<float>(photo.getX()+1.2f,photo.getCentreY()-1.8f,photo.getWidth()-2.4f,2.25f);
        // Native silver geometry stays sharp at any editor scale, including the
        // two tiny side wraps. It is independent of the photographed plastic.
        juce::Path insert;const float left=photo.getX(),right=photo.getRight(),cy=photo.getCentreY();
        insert.startNewSubPath(left+1.4f,cy-1.8f);insert.lineTo(right-1.4f,cy-1.8f);
        insert.quadraticTo(right-.45f,cy-1.8f,right-.25f,cy-.8f);insert.lineTo(right-.15f,cy+1.6f);
        insert.quadraticTo(right-.2f,cy+1.9f,right-.65f,cy+1.35f);insert.lineTo(right-1.6f,cy+.45f);
        insert.lineTo(left+1.6f,cy+.45f);insert.lineTo(left+.65f,cy+1.35f);
        insert.quadraticTo(left+.2f,cy+1.9f,left+.15f,cy+1.6f);insert.lineTo(left+.25f,cy-.8f);
        insert.quadraticTo(left+.45f,cy-1.8f,left+1.4f,cy-1.8f);insert.closeSubPath();
        const auto metal=[age](juce::Colour fresh,juce::Colour old){return fresh.interpolatedWith(old,age);};
        juce::ColourGradient silver(metal(juce::Colour(0xffa2abad),juce::Colour(0xff727a78)),left,cy-1,
            metal(juce::Colour(0xffbdc5c4),juce::Colour(0xff8c918e)),right,cy+1,false);
        silver.addColour(.18,metal(juce::Colour(0xffe8eeee),juce::Colour(0xff8e9491)));
        silver.addColour(.40,metal(juce::Colour(0xffa9b3b4),juce::Colour(0xff7b827f)));
        silver.addColour(.68,metal(juce::Colour(0xffd5dcda),juce::Colour(0xff8a908c)));
        g.setGradientFill(silver);g.fillPath(insert);
        g.setColour(juce::Colours::white.withAlpha(.65f*(1-age)));
        g.drawLine(left+1.8f,cy-1.65f,right-1.8f,cy-1.65f,.28f);
        if(age<1) {
            // A compact left-hand reflection, visible at normal window sizes.
            juce::ColourGradient halo(juce::Colours::white.withAlpha(.55f*(1-age)),left+3,cy-.7f,
                juce::Colours::transparentWhite,left+4.5f,cy+.8f,true);
            g.setGradientFill(halo);g.fillEllipse(left+1.5f,cy-2.2f,3,3);
            g.setColour(juce::Colours::white.withAlpha(.93f*(1-age)));g.fillEllipse(left+2.55f,cy-1.15f,.9f,.9f);
        }
        if(age>0) {
            juce::Graphics::ScopedSaveState metalClip(g);g.reduceClipRegion(insert);
            for(uint32_t i=0;i<uint32_t(88*age);++i) {
                const auto seed=surfaceHash(wearSeed+i*7919u+3119u);
                const float x=strip.getX()+strip.getWidth()*surfaceNoise(seed),y=strip.getY()+strip.getHeight()*surfaceNoise(seed+19);
                const float size=.10f+.32f*surfaceNoise(seed+31);
                g.setColour(juce::Colour(i%4?0xff555b59:0xffd2d7d3).withAlpha(i%4?.30f:.22f));
                g.fillEllipse(x,y,size,size*.66f);
            }
        }
        // Polished plastic develops only fine, independently placed handling
        // scuffs. There is no molded pebbled texture on the new cap surface.
        if(age>0)for(uint32_t i=0;i<uint32_t(28*age);++i) {
            const auto seed=surfaceHash(wearSeed+i*733u+1487u);
            const float x=left+3+21*surfaceNoise(seed),y=cy+(i%2?-6.9f:2.f)+3.8f*surfaceNoise(seed+13);
            juce::Path scuff;scuff.startNewSubPath(x,y);scuff.quadraticTo(x+.3f,y+.7f,x+.4f+1.5f*surfaceNoise(seed+31),y+1.3f);
            g.setColour(juce::Colour(0xffa0a6a0).withAlpha(.19f));
            g.strokePath(scuff,juce::PathStrokeType(.11f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
        return;
    }
    if(!indexed) {
        // The owner's 131030/131039/131053 photos show a raised black
        // tongue with six transverse ribs over its rounded finger surface.
        // The pale rib edges are reflections/wear, not an inserted index.
        const auto point=[&](float xx,float yy) {return juce::Point<float>(r.getX()+xx*r.getWidth(),r.getY()+yy*r.getHeight());};
        juce::Path tongue;tongue.startNewSubPath(point(.08f,.24f));
        tongue.cubicTo(point(.10f,.07f),point(.25f,.02f),point(.50f,.02f));
        tongue.cubicTo(point(.77f,.02f),point(.94f,.10f),point(.94f,.27f));
        tongue.lineTo(point(.97f,.85f));tongue.quadraticTo(point(.90f,.99f),point(.72f,1));
        tongue.lineTo(point(.13f,.96f));tongue.lineTo(point(.04f,.80f));tongue.closeSubPath();
        g.setColour(juce::Colours::black.withAlpha(.52f));g.fillPath(tongue,juce::AffineTransform::translation(1.3f,2.0f));
        juce::ColourGradient plastic(juce::Colour(0xff373a3a),r.getCentreX(),r.getY(),
                                    juce::Colour(0xff070909),r.getCentreX(),r.getBottom(),false);
        plastic.addColour(.48,juce::Colour(0xff252829));plastic.addColour(.69,juce::Colour(0xff111314));
        g.setGradientFill(plastic);g.fillPath(tongue);
        g.setColour(juce::Colour(0xff030405));g.strokePath(tongue,juce::PathStrokeType(.65f));
        for(int i=0;i<6;++i) {
            const float yy=r.getY()+r.getHeight()*(.13f+.098f*float(i));
            const float inset=r.getWidth()*(i==0?.21f:.12f);
            g.setColour(juce::Colour(0xff050708));g.drawLine(r.getX()+inset,yy+.6f,r.getRight()-inset,yy+.6f,.85f);
            g.setColour(juce::Colour(0xffa0a09a).withAlpha(.34f+age*.16f));
            g.drawLine(r.getX()+inset,yy,r.getRight()-inset,yy,.65f);
        }
        g.setColour(juce::Colour(0xff92938c).withAlpha(.17f+age*.15f));
        g.drawLine(r.getX()+1.7f,r.getBottom()-2.1f,r.getRight()-1.4f,r.getBottom()-1.7f,.6f);
        return;
    }
    for(int i=5;i>0;--i) {
        g.setColour(juce::Colours::black.withAlpha(.035f*float(6-i)));
        g.fillRoundedRectangle(r.expanded(float(i)*.65f).translated(1.3f,2.5f),1.7f);
    }
    juce::Path p;
    p.startNewSubPath(r.getX()+1.5f,r.getY());
    p.lineTo(r.getRight()-1.5f,r.getY());p.lineTo(r.getRight(),r.getBottom()-1);
    p.quadraticTo(r.getRight(),r.getBottom(),r.getRight()-1,r.getBottom());
    p.lineTo(r.getX()+1,r.getBottom());p.quadraticTo(r.getX(),r.getBottom(),r.getX(),r.getBottom()-1);
    p.closeSubPath();
    juce::ColourGradient plastic(juce::Colour(0xff555653),r.getX(),r.getY(),
                                juce::Colour(0xff0b0c0c),r.getX()+r.getWidth()*.15f,r.getBottom(),false);
    plastic.addColour(.13,juce::Colour(0xff252728));plastic.addColour(.58,juce::Colour(0xff303233));
    plastic.addColour(.67,juce::Colour(0xff191b1d));g.setGradientFill(plastic);g.fillPath(p);
    g.setColour(juce::Colour(0xff090b0c));g.strokePath(p,juce::PathStrokeType(.8f));
    g.setColour(juce::Colours::white.withAlpha(.20f));
    g.drawLine(r.getX()+2,r.getY()+1,r.getRight()-2,r.getY()+1,.8f);
    g.drawLine(r.getX()+1,r.getY()+2,r.getX()+1,r.getBottom()-3,.5f);
    for(int i=0;i<42;++i) {
        const float xx=r.getX()+2+float((i*13)%41)/41*(r.getWidth()-4);
        const float yy=r.getY()+3+float((i*19)%43)/43*(r.getHeight()-6);
        g.setColour(juce::Colours::white.withAlpha(i%3==0?.085f:.035f));g.fillRect(xx,yy,.42f,.33f);
    }
    g.setColour(ink.darker(.08f));
    if(small && indexed)g.fillRect(r.getX()+2,r.getCentreY()-.65f,r.getWidth()-4,1.3f);
    else if(small) {
        for(int i=0;i<4;++i) {
            const float yy=r.getY()+4+float(i)*2.6f;
            g.setColour(juce::Colour(0xff090b0c));g.drawLine(r.getX()+2,yy,r.getRight()-2,yy,.8f);
            g.setColour(juce::Colours::white.withAlpha(.16f));g.drawLine(r.getX()+2,yy+.7f,r.getRight()-2,yy+.7f,.45f);
        }
    }
    else {
        g.fillRect(r.getX()+3,r.getY()+r.getHeight()*.37f,r.getWidth()-6,1.55f);
        g.setColour(juce::Colours::white.withAlpha(.25f));
        g.drawLine(r.getX()+3,r.getY()+r.getHeight()*.37f,r.getRight()-3,r.getY()+r.getHeight()*.37f,.35f);
    }
}
void projectedKnob(juce::Graphics& g,juce::Rectangle<float> area,float angle,float age,uint32_t seed) {
    // Final small inset correction retains the approved body height/camera.
    juce::Graphics::ScopedSaveState save(g);
    const float d=juce::jmin(area.getWidth(),area.getHeight())*.95f;
    const auto sprite=area.withSizeKeepingCentre(d,d);
    const auto top=juce::Point<float>(sprite.getX()+d*.497f,sprite.getY()+d*.410f);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.setOpacity(1);g.drawImage(artwork().knob,sprite,juce::RectanglePlacement::stretchToFit);
    if(age>0) {
        g.setColour(juce::Colours::black.withAlpha(age*.12f));
        g.drawImage(artwork().knob,sprite,juce::RectanglePlacement::stretchToFit,true);
        const auto& material=knobMetalWear();
        g.setOpacity(age*.84f);g.drawImage(material.matte,sprite,juce::RectanglePlacement::stretchToFit);
        const size_t index=seed==12011u?0:seed==41117u?1:2;
        g.setOpacity(std::pow(age,1.35f));g.drawImage(material.pits[index],sprite,juce::RectanglePlacement::stretchToFit);
        g.setOpacity(1);
    }
    for(uint32_t i=0;i<58;++i) {
        const float a=(surfaceNoise(seed+i*71)-.5f)*juce::MathConstants<float>::pi;
        const auto p=top+juce::Point<float>(std::sin(a)*d*.335f,
            std::cos(a)*d*.335f*panelViewCos+d*(.025f+.050f*surfaceNoise(seed+i*79)));
        const float size=d*(.002f+.003f*surfaceNoise(seed+i*37));
        g.setColour(juce::Colour(0xffc6beaa).withAlpha(age*.35f));g.fillEllipse(p.x,p.y,size,size*.70f);
    }
    // The native opaque black tick rotates; photographic lighting stays fixed.
    const auto direction=juce::Point<float>(std::sin(angle),-std::cos(angle)*panelViewCos);
    g.setColour(juce::Colour(0xff141718));
    // One third of the metal top's diameter, with a clear square-ended mark.
    g.drawLine({top+direction*d*.079f,top+direction*d*.245f},d*.028f);
    // Matching radial index on the flared metal collar; body lighting stays fixed.
    const auto collar=juce::Point<float>(top.x,sprite.getY()+d*.505f);
    g.drawLine({collar+direction*d*.390f,collar+direction*d*.440f},d*.028f);
}
void screwOxidation(juce::Graphics& g,juce::Point<float> c,float age,uint32_t seed) {
    if(age<=0)return;
    // Chalky white/gray deposits on the head, concentrated in the rim and
    // protected recess edges. Panel corrosion stays in the photographic layer.
    juce::Graphics::ScopedSaveState save(g);
    juce::Path head;head.addEllipse(c.x-7.5f,c.y-7.5f,15,15);g.reduceClipRegion(head);
    // Oxidised steel has broad diffuse shading, without the fresh photo's
    // bright specular spots. Only two heads acquire small red-brown patches.
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff555650).withAlpha(age*.96f),c.x-5,c.y-6,
        juce::Colour(0xff292b28).withAlpha(age*.96f),c.x+5,c.y+7,false));g.fillAll();
    g.setColour(juce::Colour(0xff151714).withAlpha(age*.84f));
    const float angle=surfaceNoise(seed+17)*juce::MathConstants<float>::pi;
    const auto axis=juce::Point<float>(std::cos(angle),std::sin(angle));
    const auto across=juce::Point<float>(-axis.y,axis.x);
    g.drawLine({c-axis*3.5f,c+axis*3.5f},1.1f);g.drawLine({c-across*3.5f,c+across*3.5f},1.1f);
    const auto headIndex=(seed-5011u)/7717u;
    if(headIndex==1||headIndex==5) {
        g.setColour(juce::Colour(0xff674331).withAlpha(age*age*.72f));
        g.fillEllipse(c.x-5.8f,c.y+1,5.5f,3.5f);g.fillEllipse(c.x+2,c.y-4,2.7f,4);
    }
    if(headIndex==2||headIndex==6) {
        const float wear=juce::jlimit(0.f,1.f,(age-.4f)/.6f);
        g.setColour(juce::Colour(0xff151714).withAlpha(wear*.85f));
        g.drawLine(c.x-3,c.y-2,c.x+3,c.y+2,1.0f+wear*1.2f);
        g.drawLine(c.x+1.8f,c.y-3,c.x-1.7f,c.y+3,.9f+wear);
        g.setColour(juce::Colour(0xff78766a).withAlpha(wear*.32f));
        g.drawLine(c.x-2.8f,c.y-1,c.x+2.4f,c.y+2.2f,.55f);
    }
    for(uint32_t i=0;i<4;++i) {
        const float a=surfaceNoise(seed+i*113)*juce::MathConstants<float>::twoPi;
        const auto p=c+juce::Point<float>(std::sin(a),std::cos(a))*5.7f;
        const float size=1.4f+surfaceNoise(seed+i*83);
        g.setColour(juce::Colour(0xff77786e).withAlpha(age*.24f));
        g.fillEllipse(p.x-size*.5f,p.y-size*.3f,size,size*.6f);
    }
    for(uint32_t i=0;i<90;++i) {
        const float a=surfaceNoise(seed+i*23)*juce::MathConstants<float>::twoPi;
        const float radius=4.1f+2.9f*surfaceNoise(seed+i*31+7);
        const auto p=c+juce::Point<float>(std::sin(a),std::cos(a))*radius;
        const float size=.25f+.95f*surfaceNoise(seed+i*19+3);
        g.setColour(juce::Colour(i%3?0xff7c7b70:0xff383c35).withAlpha(age*(.24f+.44f*surfaceNoise(seed+i*47))));
        g.fillEllipse(p.x,p.y,size,size*(.5f+surfaceNoise(seed+i*41)));
    }
}
class HardwareLook final : public kb09_gui::DialogTheme {
public:
    bool conditionDragging=false,renderingPreview=false;
    HardwareLook() {
        setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff202326));
        setColour(juce::ComboBox::textColourId,ink);
        setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff454a4c));
        setColour(juce::ComboBox::arrowColourId,muted);
        setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff202326));
        setColour(juce::PopupMenu::textColourId,ink);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff514a3c));
        setColour(juce::Label::textColourId,ink);
        setColour(juce::TextButton::buttonColourId,juce::Colour(0xff242729));
        setColour(juce::TextButton::textColourOffId,ink);
        setColour(juce::TooltipWindow::backgroundColourId,juce::Colour(0xffeeeae0));
        setColour(juce::TooltipWindow::textColourId,juce::Colour(0xff22262a));
        setColour(juce::BubbleComponent::backgroundColourId,juce::Colour(0xffeeeae0));
        setColour(juce::BubbleComponent::outlineColourId,juce::Colour(0xff756850));
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override {return font(11);}
    juce::Font getTextButtonFont(juce::TextButton& b,int h) override {return b.findParentComponentOfClass<juce::AlertWindow>() ? DialogTheme::getTextButtonFont(b,h) : font(11,true);}
    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour& c,bool over,bool down) override { if(b.findParentComponentOfClass<juce::AlertWindow>()) DialogTheme::drawButtonBackground(g,b,c,over,down); else juce::LookAndFeel_V4::drawButtonBackground(g,b,c,over,down); }
    void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down) override { if(b.findParentComponentOfClass<juce::AlertWindow>()) DialogTheme::drawButtonText(g,b,over,down); else juce::LookAndFeel_V4::drawButtonText(g,b,over,down); }
    juce::Font getSliderPopupFont(juce::Slider&) override {return font(15,true);}
    juce::Font getLabelFont(juce::Label& l) override {return l.getFont();}
    void drawLabel(juce::Graphics& g,juce::Label& l) override {juce::LookAndFeel_V4::drawLabel(g,l);}
    void drawComboBox(juce::Graphics& g,int w,int h,bool d,int x,int y,int bw,int bh,juce::ComboBox& c) override {juce::LookAndFeel_V4::drawComboBox(g,w,h,d,x,y,bw,bh,c);}
    // Printed scales belong to the panel, so their outer numerals and
    // shared strokes are neither clipped by hit areas nor redrawn with caps.
    void drawFaderScale(juce::Graphics& g,juce::Slider& slider) {
        const auto bounds=getSliderLayout(slider).sliderBounds.translated(slider.getX(),slider.getY());
        const float cx=float(bounds.getCentreX());
        const int y=bounds.getY(),h=bounds.getHeight();
        const bool compact=bool(slider.getProperties().getWithDefault("compact",false));
        const auto ink=agedInk(age);
            for(int t=0;t<=10;++t) {
                const float yy=float(y)+float(h)*float(t)/10;
                const bool major=t%5==0;
                const float arm=compact?(major?7.0f:5.5f):(major?9.5f:7.5f);
                const float left=float(slider.getProperties().getWithDefault("tickLeftSpan",arm+5));
                const float right=float(slider.getProperties().getWithDefault("tickRightSpan",arm+5));
                g.setColour(ink.withAlpha(major?.90f:.73f));
                g.drawLine(cx-left,yy,cx-(major?5.5f:7.f),yy,major?1.25f:.80f);
                g.drawLine(cx+(major?5.5f:7.f),yy,cx+right,yy,major?1.25f:.80f);
            }
            for(int i=0;i<3;++i) {
                const float yy=float(y)+float(h)*float(i)*.5f;
                if(bool(slider.getProperties().getWithDefault("numbersLeft",true)))
                    caption(g,juce::String(10-i*5),{cx-29.0f,yy-4,11,8},6.6f,juce::Justification::centredRight,false,ink);
                if(bool(slider.getProperties().getWithDefault("numbersRight",true)))
                    caption(g,juce::String(10-i*5),{cx+18.0f,yy-4,11,8},6.6f,juce::Justification::centredLeft,false,ink);
            }
    }
    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,
                          const juce::Slider::SliderStyle style,juce::Slider& slider) override {
        if(!g.clipRegionIntersects(juce::Rectangle<int>(-30,-14,slider.getWidth()+60,slider.getHeight()+28)))return;
        const auto ink=agedInk(age);
        const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f-((style==juce::Slider::LinearHorizontal&&slider.getY()>=headerH+visibleBodyH)?4.f:0.f);
        if(style==juce::Slider::LinearVertical) {
            const bool compact=bool(slider.getProperties().getWithDefault("compact",false));
            const auto rim=juce::Rectangle<float>(cx-4.8f,float(y)-2,9.6f,float(h)+4);
            // A dark, pill-ended opening with a narrow rolled lip. Avoid a
            // bright uniform outline that makes the track look drawn on.
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff363937),rim.getX(),rim.getY(),
                juce::Colour(0xff101211),rim.getRight(),rim.getY(),false));
            g.fillRoundedRectangle(rim,4.8f);
            const auto opening=rim.reduced(.85f,.7f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff010202),opening.getX(),opening.getY(),
                juce::Colour(0xff080a09),opening.getRight(),opening.getY(),false));
            g.fillRoundedRectangle(opening,opening.getWidth()*.5f);
            kb09_gui::paintFaderDustCover(g,opening,uint32_t(slider.getComponentID().hashCode()),kb09_gui::appearanceStage(age));
            g.setColour(juce::Colour(0xff96998f).withAlpha(.08f*(1-age*.6f)));
            g.drawLine(cx-4.5f,float(y)+4,cx-4.5f,float(y+h)-4,.45f);
            const auto capBounds=juce::Rectangle<float>(cx-(compact?12.5f:15),pos-10,compact?25.0f:30.0f,23);
            const bool releasePost=slider.getComponentID()=="release";
            const auto id=slider.getComponentID();
            const bool missing=(id=="vcoLevel"&&age>=.50f)||(id=="lfoDelay"&&age>=.75f)
                ||(id=="extLevel"&&age>=.75f)||(releasePost&&age>=1.f)||(id=="pulseWidth"&&age>=1.f);
            // Cap loss is a discrete visual event. The original slider, hit
            // area, attachment and value remain available beneath the drawing.
            const auto seed=uint32_t(id.hashCode());
            const float progress=age; // 0/25/50/75/100% of each cap's individual lean and twist.
            const float lean=(surfaceNoise(seed^0x773ab21u)*2.f-1.f)*1.7f*progress;
            if(missing)sliderPost(g,capBounds,age,releasePost,lean,seed);
            else {
                juce::Graphics::ScopedSaveState aligned(g);
                g.addTransform(juce::AffineTransform::translation(lean,0));
                // Stable independent random alignment per physical cap. AGE
                // scales each unique signed angle; no section/alternation rule.
                const float severity=id=="cutoff"?.23f:id=="resonance"?.28f:1.f;
                const float degrees=(float(surfaceHash(seed^685u)&65535u)/65535.f*18.f-9.f)*severity;
                const float twist=juce::degreesToRadians(degrees)*progress;
                g.addTransform(juce::AffineTransform::rotation(twist,capBounds.getCentreX(),capBounds.getCentreY()));
                cap(g,capBounds,false,true,age,uint32_t(id.hashCode()));
            }
        } else {
            if(slider.getComponentID()=="bender") {
                const float amount=float(slider.getValue());
                const float centre=float(slider.getWidth())*.5f;
                juce::Graphics::ScopedSaveState wheelGeometry(g);
                g.addTransform(juce::AffineTransform::scale(float(slider.getWidth())/143.f,1.f,centre,0));
                auto well=juce::Rectangle<float>(centre-63.5f,13,127,22);
                g.setColour(juce::Colour(0xff080909));g.fillRoundedRectangle(well.expanded(1.3f),1.1f);
                g.setColour(juce::Colour(0xff393b38).withAlpha(.55f));g.drawLine(well.getX(),well.getBottom()+.6f,well.getRight(),well.getBottom()+.6f,.65f);
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff030303),well.getX(),well.getY(),
                                                     juce::Colour(0xff202020),well.getX(),well.getBottom(),false));
                g.fillRoundedRectangle(well,.7f);
                // One rigid PB-4-like disk and finger tab, extruded along its
                // hidden axle. Rotate object vertices first; the shared 11-degree
                // camera then projects every face without screen-space reshaping.
                const float angle=amount*.610865f,sa=std::sin(angle),ca=std::cos(angle);
                constexpr float radius=60.0f,depth=13.5f;
                constexpr float rootWidth=7.f;
                constexpr float rearRound=.40f,frontRound=1.15f;
                // A thin central web carries a substantially thicker molded rim.
                const auto frontDepth=[](juce::Point<float> p) {
                    const float t=juce::jlimit(0.0f,1.0f,(p.getDistanceFromOrigin()-51.0f)/5.0f);
                    return 5.4f+8.1f*t*t*(3-2*t);
                };
                const float wheelBaseline=13.0f+radius*panelViewSin;
                const bool broken=age>=1.f;
                const auto project=[centre,wheelBaseline,sa,ca](juce::Point<float> p,float axleDepth) {
                    const float xx=p.x*ca+p.y*sa,zz=p.y*ca-p.x*sa;
                    return juce::Point<float>(centre+xx,wheelBaseline-zz*panelViewSin+axleDepth*panelViewCos);
                };
                constexpr int circleSegments=256,vertexCapacity=circleSegments+64;
                std::array<juce::Point<float>,vertexCapacity> outline{};
                int count=0;
                const float rootAngle=std::asin((broken?21.0f:rootWidth)/radius),rootHeight=std::sqrt(radius*radius-rootWidth*rootWidth);
                for(int i=0;i<=circleSegments;++i) {
                    const float a=rootAngle+(juce::MathConstants<float>::twoPi-2*rootAngle)*float(i)/float(circleSegments);
                    outline[size_t(count++)]={radius*std::sin(a),radius*std::cos(a)};
                }
                // The tab replaces the small crown arc, so there is no detached
                // face or changing-width overlay sliding across the disk rim.
                if(broken) {
                    // Asymmetric broken shoulders and a shallow ragged valley,
                    // matching the traced break rather than a row of saw teeth.
                    const std::array<juce::Point<float>,20> tab{{
                        {-20.6f,56.8f},{-18.4f,59.5f},{-17.1f,60.2f},{-15.7f,57.8f},
                        {-13.2f,56.6f},{-15.1f,53.7f},{-10.5f,49.8f},{-8.9f,50.4f},
                        {-6.1f,50.9f},{-3.6f,50.1f},{-.8f,50.4f},{2.3f,49.8f},
                        {5.2f,49.7f},{8.6f,50.1f},{10.0f,52.8f},{13.4f,55.0f},
                        {12.1f,59.5f},{15.2f,62.9f},{17.0f,60.2f},{20.5f,56.6f}}};
                    for(const auto p:tab)outline[size_t(count++)]=p;
                }
                // Healthy tab is shaded as one rounded solid after the disk;
                // the closed crown chord remains its hidden attachment point.
                // Each shared fracture edge leans by a different fixed amount
                // through the plastic. Keep both neighboring facets connected,
                // and keep the pattern rigid as the wheel turns.
                const auto fractureOffset=[&](int i) {
                    if(!broken||i<=circleSegments)return juce::Point<float>();
                    const float taper=juce::jlimit(0.0f,1.0f,float(juce::jmin(i-circleSegments,count-1-i))/3.0f);
                    const auto seed=uint32_t(i-circleSegments)*7919u+0x5b31u;
                    return juce::Point<float>((surfaceNoise(seed)-.5f)*4.0f*taper,
                                              (surfaceNoise(seed+137u)-.5f)*1.7f*taper);
                };
                const auto surfacePoint=[&](int i,float t) {
                    auto local=outline[size_t(i)];
                    if(i<=circleSegments)local*=1.f-(t>0?frontRound:rearRound)/radius;
                    return project(local+fractureOffset(i)*t,frontDepth(local)*t);
                };
                const auto section=[&](float z) {
                    const float t=z==0?0.0f:1.0f;
                    juce::Path p;p.startNewSubPath(surfacePoint(0,t));
                    for(int i=1;i<count;++i)p.lineTo(surfacePoint(i,t));
                    p.closeSubPath();return broken?p:p.createPathWithRoundedCorners(1.8f);
                };
                const auto rear=section(0),front=section(depth);
                juce::Graphics::ScopedSaveState save(g);
                g.reduceClipRegion(juce::Rectangle<int>(int(well.getX()),0,int(well.getWidth()),int(well.getBottom())));
                g.setColour(juce::Colours::black.withAlpha(.40f));g.fillPath(rear,juce::AffineTransform::translation(.7f,1.3f));
                g.fillPath(front,juce::AffineTransform::translation(.7f,1.3f));
                g.setColour(juce::Colour(0xff080b0b));g.fillPath(rear);
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff141615).darker(age*.20f),centre-45,16,
                                                     juce::Colour(0xff050606),centre+45,well.getBottom(),false));g.fillPath(front);
                // Smooth cylinder shading is evaluated per sample, rather than
                // giving each mesh strip its own flat colour and antialias seam.
                // The inverse projection uses the same rigid axle/camera above.
                const auto plasticLight=[age=age](float nx,float ny,float nz,float axial) {
                    const float diffuse=juce::jmax(0.0f,-.270f*nx+.090f*ny+.959f*nz);
                    const float half=juce::jlimit(0.0f,1.0f,-.137f*nx+.125f*ny+.982f*nz);
                    const float fill=std::pow(juce::jmax(0.0f,.62f*std::abs(nx)+.78f*nz),7.0f);
                    // Keep the existing broad highlight in the same place. Shape
                    // its boundary instead of adding another reflected light.
                    const float specular=kb09_gui::polishedBenderHighlight(half,age);
                    const float lightShape=.70f+.30f*std::exp(-((axial-1.2f)*(axial-1.2f))/1.8f);
                    return .068f+.064f*diffuse+.090f*fill+.95f*specular*lightShape*(1-age*.42f)+age*.008f;
                };
                const int samples=conditionDragging?1:juce::jlimit(1,4,int(std::ceil(g.getInternalContext().getPhysicalPixelScaleFactor())));
                const juce::Rectangle<float> rimBounds(centre-radius-1,12,2*radius+2,
                                                       wheelBaseline+depth*panelViewCos-11);
                juce::Image rim(juce::Image::ARGB,juce::roundToInt(rimBounds.getWidth()*samples),
                                juce::roundToInt(rimBounds.getHeight()*samples),true);
                {
                    juce::Image::BitmapData pixels(rim,juce::Image::BitmapData::writeOnly);
                    const juce::Image::BitmapData scuffPixels(kb09_gui::benderScuffs().rim[size_t(kb09_gui::appearanceStage(age))],juce::Image::BitmapData::readOnly);
                    for(int yy=0;yy<rim.getHeight();++yy)for(int xx=0;xx<rim.getWidth();++xx) {
                        const float px=rimBounds.getX()+(float(xx)+.5f)/samples-centre;
                        const float py=rimBounds.getY()+(float(yy)+.5f)/samples;
                        if(std::abs(px)>=radius)continue;
                        float z=std::sqrt(radius*radius-px*px);
                        float axial=(py-wheelBaseline+z*panelViewSin)/panelViewCos;
                        if(axial<-.18f||axial>depth+.18f)continue;
                        float r=radius;
                        for(int iteration=0;iteration<2;++iteration) {
                            const float bevel=axial<depth*.5f?rearRound:frontRound;
                            const float end=juce::jlimit(0.0f,bevel,juce::jmin(axial,depth-axial));
                            r=radius-bevel+std::sqrt(juce::jmax(0.0f,bevel*bevel-(end-bevel)*(end-bevel)));
                            if(std::abs(px)>=r)break;
                            z=std::sqrt(r*r-px*px);
                            axial=(py-wheelBaseline+z*panelViewSin)/panelViewCos;
                        }
                        if(std::abs(px)>=r||axial<-.18f||axial>depth+.18f)continue;
                        const float localAngle=std::atan2(px*ca-z*sa,z*ca+px*sa);
                        if(std::abs(localAngle)<rootAngle)continue;
                        float radial=1,ny=0;
                        if(axial<rearRound)ny=juce::jlimit(-1.0f,0.0f,(axial-rearRound)/rearRound);
                        else if(axial>depth-frontRound)ny=juce::jlimit(0.0f,1.0f,(axial-(depth-frontRound))/frontRound);
                        radial=std::sqrt(juce::jmax(0.0f,1-ny*ny));
                        float value=plasticLight(px/r*radial,ny,z/r*radial,axial);
                        if(age>0) {
                            const float scratch=kb09_gui::sampleBenderScuff(scuffPixels,(localAngle+juce::MathConstants<float>::pi)/juce::MathConstants<float>::twoPi,axial/depth);
                            const float grazingLight=juce::jmax(0.f,-.270f*px/r*radial+.090f*ny+.959f*z/r*radial);
                            value=value*(1-.75f*scratch)+(.18f+.25f*grazingLight)*scratch;
                        }
                        // Fine raised parting seam follows the molded disc around its circumference.
                        const float seam=(axial-depth*.50f)/.48f;
                        const float litArc=std::pow(juce::jmax(0.0f,-.30f*px/r+.954f*z/r),10.0f);
                        value+=(.105f*std::exp(-seam*seam)-.045f*std::exp(-(seam-1.15f)*(seam-1.15f)))*(1-age*.38f)*litArc;
                        const auto grainKey=uint32_t(int((localAngle+juce::MathConstants<float>::pi)*380))
                                           +uint32_t(juce::jmax(0,int(axial*6)))*1177u;
                        value+=(surfaceNoise(0x3a71u+grainKey)-.5f)*(.005f+age*.007f);
                        const float coverage=juce::jlimit(0.0f,1.0f,(juce::jmin(axial,depth-axial)+.15f)/.30f);
                        pixels.setPixelColour(xx,yy,juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,coverage));
                    }
                }
                g.drawImage(rim,rimBounds);
                // The tab and its fracture are planar rigid faces, with the same
                // light as the curved rim. Sort their faces back to front.
                std::array<int,vertexCapacity> order{};std::array<float,vertexCapacity> distance{};
                for(int i=0;i<count;++i) {
                    const auto a=outline[size_t(i)],b=outline[size_t((i+1)%count)];
                    distance[size_t(i)]=((a.y+b.y)*ca-(a.x+b.x)*sa)*.5f;
                    int j=i;while(j>0&&distance[size_t(order[size_t(j-1)])]>distance[size_t(i)]) {
                        order[size_t(j)]=order[size_t(j-1)];--j;
                    }
                    order[size_t(j)]=i;
                }
                for(int index=0;index<count;++index) {
                    const int i=order[size_t(index)];
                    if(i<circleSegments||!broken)continue;
                    const auto a=outline[size_t(i)],b=outline[size_t((i+1)%count)];
                    const float dx=b.x-a.x,dz=b.y-a.y,length=std::sqrt(dx*dx+dz*dz);
                    if(length<.0001f)continue;
                    const float nx=(-dz*ca+dx*sa)/length,nz=(dx*ca+dz*sa)/length;
                    if(nz<=0)continue;
                    const int next=(i+1)%count;
                    juce::Path wall;wall.startNewSubPath(surfacePoint(i,0));wall.lineTo(surfacePoint(next,0));
                    wall.lineTo(surfacePoint(next,1));wall.lineTo(surfacePoint(i,1));wall.closeSubPath();
                    const bool fracture=broken&&i>=circleSegments;
                    const auto shade=[&](float axial) {
                        // Matte fracture facets have distinct plane lighting;
                        // a broad reflection would soften the jagged cut.
                        const float diffuse=juce::jmax(0.0f,-.54f*nx+.842f*nz);
                        const float value=fracture?.029f+.185f*diffuse*diffuse
                            +.024f*surfaceNoise(uint32_t(i)*917u):plasticLight(nx,0,nz,axial)*(1+(!broken&&a.y>radius?.22f*(1-age):0.f));
                        auto colour=juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,1);
                        if(fracture)colour=colour.interpolatedWith(juce::Colour(0xff777777),.08f);
                        return colour;
                    };
                    const auto middle=(a+b)*.5f,backPoint=project(middle,0),frontPoint=project(middle,depth);
                    juce::ColourGradient reflection(shade(0),backPoint.x,backPoint.y,shade(depth),frontPoint.x,frontPoint.y,false);
                    reflection.addColour(.22,shade(depth*.22f));reflection.addColour(.70,shade(depth*.70f));

                    if(fracture)g.setColour(shade(depth*.45f));else g.setGradientFill(reflection);
                    g.fillPath(wall);
                    if(fracture) {
                        juce::Graphics::ScopedSaveState rough(g);g.reduceClipRegion(wall);
                        for(uint32_t grain=0;grain<18;++grain) {
                            const auto key=uint32_t(i)*7919u+grain*113u;
                            const float along=surfaceNoise(key+13),across=surfaceNoise(key+31);
                            const auto local=a+(b-a)*along;
                            const auto shift=fractureOffset(i)+(fractureOffset(next)-fractureOffset(i))*along;
                            const auto point=project(local+shift*across,frontDepth(local)*across);
                            g.setColour(juce::Colour(grain%3?0xffa1a1a0:0xff080909).withAlpha(.20f));
                            g.fillEllipse(point.x,point.y,.27f,.20f);
                        }
                    }
                    // Overlap the antialias coverage with the identical material.
                    if(fracture)g.setColour(shade(depth*.45f));else g.setGradientFill(reflection);
                    g.strokePath(wall,juce::PathStrokeType(.16f));
                    const float fractureEdge=.12f+.32f*juce::jmax(0.0f,-.54f*nx+.842f*nz);
                    g.setColour(juce::Colour(0xff8c9087).withAlpha((fracture?fractureEdge:.32f)*(1-age*.45f)));
                    g.drawLine({surfacePoint(i,.15f/depth),surfacePoint(next,.15f/depth)},.45f);
                    g.setColour(juce::Colour(0xff010202).withAlpha(.55f));
                    g.drawLine({surfacePoint(i,1),surfacePoint(next,1)},.35f);
                }
                const auto channel=[&](float r,float axle) {
                    juce::Path p;
                    for(int i=0;i<=circleSegments;++i) {
                        const float a=juce::MathConstants<float>::twoPi*float(i)/float(circleSegments);
                        const auto q=project({r*std::sin(a),r*std::cos(a)},axle);
                        if(i==0)p.startNewSubPath(q);else p.lineTo(q);
                    }
                    return p;
                };
                {
                    juce::Graphics::ScopedSaveState surface(g);g.reduceClipRegion(front);
                    // Recessed web and the rounded step under the thick rim.
                    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff343735).darker(age*.4f),centre,21,juce::Colour(0xff080b0b),centre,well.getBottom(),false));
                    g.fillPath(channel(54,frontDepth({0,54})));
                    g.setColour(juce::Colour(0xff090c0c));g.fillPath(channel(51.5f,frontDepth({0,51.5f})));
                    // Faint scratches on the exposed web are rigidly projected
                    // with the wheel, and stay underneath the protected raised rib.
                    for(unsigned i=0;i<unsigned(40*age);++i) {
                        const unsigned key=0x68e231u+i*7919u;
                        const float a=surfaceNoise(key)*juce::MathConstants<float>::twoPi;
                        const float r=38.f+14.f*surfaceNoise(key+59);
                        const juce::Point<float> local(r*std::sin(a),r*std::cos(a));
                        const auto end=local+juce::Point<float>((surfaceNoise(key+137)-.5f)*4.f,1.f+2.f*surfaceNoise(key+193));
                        const auto p=project(local,frontDepth(local)),q=project(end,frontDepth(end));
                        g.setColour(juce::Colour(0xffa4aaa3).withAlpha(.12f+.12f*surfaceNoise(key+317)));
                        g.drawLine({p,q},.12f);
                    }
                    {
                        // Raised concentric moulding under the outer rim, with a
                        // circular cross-section and continuous surface normals.
                        juce::Graphics::ScopedSaveState ringClip(g);
                        g.reduceClipRegion(channel(54,frontDepth({0,54})));
                        kb09_gui::paintBenderUndersideRing(g,centre,wheelBaseline,age,samples);
                    }
                    // Fixed object-space grain follows the rigid part. Repaints
                    // never move its particles; dust increases only with AGE.
                    for(uint32_t i=0;i<190;++i) {
                        const float a=juce::MathConstants<float>::twoPi*surfaceNoise(0x2b37u+i*19u);
                        const float r=58.0f*std::sqrt(surfaceNoise(0x53a9u+i*41u));
                        const auto p=project({r*std::sin(a),r*std::cos(a)},frontDepth({r*std::sin(a),r*std::cos(a)}));
                        const float size=.15f+.18f*surfaceNoise(0x713bu+i*17u);
                        g.setColour(juce::Colour(i%3?0xffc4beab:0xff020404).withAlpha(i%3?.020f+age*.095f:.060f));
                        g.fillEllipse(p.x,p.y,size,size*.75f);
                    }
                    for(uint32_t i=0;i<24;++i) {
                        const auto p=project({(surfaceNoise(0x4387u+i*31u)-.5f)*8.0f,
                            rootHeight+(broken?3.0f:23.0f)*surfaceNoise(0x68bdu+i*23u)},depth);
                        g.setColour(juce::Colour(0xffb8b39e).withAlpha(.015f+age*.075f));g.fillEllipse(p.x,p.y,.22f,.17f);
                    }
                }
                for(uint32_t i=0;i<110;++i) {
                    const float a=(surfaceNoise(0x37c9u+i*47u)-.5f)*juce::MathConstants<float>::twoPi;
                    if(std::abs(a)<rootAngle||std::cos(a+angle)<=0)continue;
                    const auto p=project({radius*std::sin(a),radius*std::cos(a)},depth*surfaceNoise(0x61d3u+i*29u));
                    const float size=.16f+.18f*surfaceNoise(0x7a39u+i*13u);
                    g.setColour(juce::Colour(0xffbcb6a4).withAlpha(.015f+age*.095f));g.fillEllipse(p.x,p.y,size,size*.8f);
                }
                if(!broken)kb09_gui::paintRoundedBenderTab(g,centre,wheelBaseline,angle,age,samples);
            } else {
                g.setColour(juce::Colour(0xff05080a));g.fillRoundedRectangle(float(x),cy-3,float(w),6,3);
                g.setColour(juce::Colours::white.withAlpha(.18f));g.drawLine(float(x),cy+3,float(x+w),cy+3,.6f);
                g.setColour(ink.withAlpha(.35f));g.fillRect(float(x),cy-1,juce::jmax(0.0f,pos-float(x)),1.5f);
                cap(g,{pos-8,cy-8,16,17},true);
            }
        }
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,
                          float start,float end,juce::Slider& slider) override {
        if(!g.clipRegionIntersects(slider.getLocalBounds().expanded(5)))return;
        const auto area=juce::Rectangle<float>(float(x),float(y),float(w),float(h));
        const auto ink=agedInk(age);
        const auto c=area.getCentre();const float d=juce::jmin(float(w),float(h));
        const float a=start+(end-start)*value;
        if(slider.getComponentID()=="tune") {
            auto disc=juce::Rectangle<float>(c.x-d*.38f,c.y-d*.38f,d*.76f,d*.76f);
            g.setColour(juce::Colours::black.withAlpha(.5f));g.fillEllipse(disc.expanded(1).translated(1,2));
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff565b5c),disc.getX(),disc.getY(),
                                                juce::Colour(0xff141719),disc.getRight(),disc.getBottom(),false));
            g.fillEllipse(disc);
            g.setColour(juce::Colour(0xff0b0d0e));g.drawEllipse(disc,.8f);
            const auto bowl=disc.reduced(d*.09f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff080b0c),bowl.getCentreX(),bowl.getY(),
                juce::Colour(0xff3e4242),bowl.getCentreX(),bowl.getBottom(),false));g.fillEllipse(bowl);
            for(uint32_t grain=0;grain<180;++grain) {
                const float theta=surfaceNoise(grain*7919u+57)*juce::MathConstants<float>::twoPi;
                const float radius=std::sqrt(surfaceNoise(grain*113u+91))*bowl.getWidth()*.47f;
                const auto q=bowl.getCentre()+juce::Point<float>(std::cos(theta),std::sin(theta))*radius;
                g.setColour(juce::Colour(0xffb2ac98).withAlpha(age*(.23f+.30f*surfaceNoise(grain+53))));
                g.fillEllipse(q.x,q.y,.22f,.18f);
            }
            auto p=c+juce::Point<float>(std::sin(a),-std::cos(a))*d*.26f;
            g.setColour(ink);g.drawLine({c,p},1.4f);
        } else {
            const uint32_t seed=slider.getComponentID()=="range"?12011u:slider.getComponentID()=="wave"?41117u:93973u;
            projectedKnob(g,area,a,age,seed);
        }
    }
};
// Relative drags keep fine adjustment consistent for knobs, faders and bender.
class DescriptionTips final : public juce::TooltipWindow {
public:
 DescriptionTips(juce::Component* owner,juce::TextButton& toggle):TooltipWindow(owner,500),button(toggle){}
 juce::String getTipFor(juce::Component& c) override {
  if(&c==&button)return "DESCRIPTION: enable or disable explanations when you hover over controls. Value readouts are always available.";
  return button.getToggleState()?juce::TooltipWindow::getTipFor(c):juce::String();
 }
private: juce::TextButton& button;
};
class FineSlider final : public juce::Slider {
public:
 FineSlider() { setSliderSnapsToMousePosition(false); }
 void mouseDown(const juce::MouseEvent& e) override {
  juce::Slider::mouseDown(e);last=e.position;position=valueToProportionOfLength(getValue());
 }
 void mouseDrag(const juce::MouseEvent& e) override {
  const auto delta=e.position-last;last=e.position;
  const bool rotary=getSliderStyle()==RotaryHorizontalVerticalDrag;
  const double distance=rotary?delta.x-delta.y:isVertical()?-delta.y:delta.x;
  const double extent=rotary?190.0:juce::jmax(1,isVertical()?getHeight():getWidth());
  position=juce::jlimit(0.0,1.0,position+distance/extent*(e.mods.isShiftDown()?.1:1.0));
  setValue(proportionOfLengthToValue(position),juce::sendNotificationSync);repaint();
 }
private: juce::Point<float> last;double position=0;
};
class SlideSwitch final : public juce::Component, public juce::SettableTooltipClient {
public:
    SlideSwitch(juce::RangedAudioParameter& p,std::vector<int> positions,juce::StringArray names,bool left=false)
        : order(std::move(positions)),labels(std::move(names)),labelsLeft(left),
          attachment(p,[this](float v){value=juce::roundToInt(v);repaint();},nullptr) {
        initial=juce::roundToInt(p.convertFrom0to1(p.getDefaultValue()));
        setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::PointingHandCursor);
        attachment.sendInitialUpdate();
    }
    ~SlideSwitch() override {if(dragging)attachment.endGesture();}
    void paint(juce::Graphics& g) override {
        const auto* look=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=look!=nullptr?look->age:0;
        const auto ink=agedInk(getY()<headerH+visibleBodyH?age:0);
        const float xx=actuatorX();
        const auto id=getComponentID();
        const bool middleOnLeft=labelsLeft||id=="lfoWave"||id=="vcaMode";
        const float middle=float(getHeight())*.5f,throwStep=order.size()==3?12.0f:10.0f;
        const float top=middle-throwStep*float(order.size()-1)*.5f,bottom=middle+throwStep*float(order.size()-1)*.5f;
        const float labelStep=order.size()==3?36.0f:20.0f;
        g.setColour(juce::Colour(0xff62696b));g.fillRoundedRectangle(xx-5,top-10,10,bottom-top+20,1.8f);
        g.setColour(juce::Colour(0xff030608));g.fillRoundedRectangle(xx-4,top-9,8,bottom-top+18,1.2f);
        for(size_t i=0;i<order.size();++i) {
            const float yy=top+throwStep*float(i),labelY=middle+(float(i)-float(order.size()-1)*.5f)*labelStep;
            // Original slide-switch legends have no detent tick marks.
            auto textBox=middleOnLeft?juce::Rectangle<float>(0,labelY-13,xx-9,26):
                                   juce::Rectangle<float>(xx+11,labelY-13,float(getWidth())-xx-11,26);
            // Top and bottom legends sit near the short actuator's axis;
            // the middle legend sits beside it, as on the original panel.
            const juce::Rectangle<float> axialText(juce::jmax(0.0f,xx-14),labelY-11,28,22);
            if(id=="lfoWave") {
                if(i<2)panelGlyph(g,i==0?PanelGlyph::sine:PanelGlyph::square,
                                 {i==0?xx-7:xx-23,labelY-4,16,8},ink);
                else caption(g,labels[int(i)],{0,labelY-12,float(getWidth()),24},7.6f,juce::Justification::centred,true,ink);
            } else if(id=="subMode") {
                if(i==0) {
                    panelGlyph(g,PanelGlyph::square,{xx-7,labelY-16,15,8},ink);
                    caption(g,labels[int(i)],axialText.translated(0,4),7.3f,juce::Justification::centred,true,ink);
                } else {
                    const auto words=i==1?textBox.withY(labelY-13).withHeight(21):axialText;
                    caption(g,labels[int(i)],words,7.3f,juce::Justification::centred,true,ink);
                    panelGlyph(g,i==1?PanelGlyph::square:PanelGlyph::pulse,
                               {words.getCentreX()-7.5f,labelY+11,15,8},ink);
                }
            } else if(id=="pwmMode") {
                if(i==0) {
                    panelGlyph(g,PanelGlyph::doubleTriangle,{xx-7,labelY-14,15,10},ink);
                    caption(g,"LFO",axialText.translated(0,6),7.6f,juce::Justification::centred,true,ink);
                } else if(i==1)caption(g,"MANUAL",textBox,6.9f,juce::Justification::centredLeft,true,ink);
                else {
                    caption(g,"ENV",axialText.translated(0,-3),7.6f,juce::Justification::centred,true,ink);
                    panelGlyph(g,PanelGlyph::envelope,{xx-7,labelY+7,15,8},ink);
                }
            } else if(id=="vcfEnvMode"||id=="vcaMode") {
                if(i==0)caption(g,labels[0],axialText,7.3f,juce::Justification::centred,true,ink);
                else {
                    const auto words=i==1?textBox:axialText;
                    caption(g,id=="vcaMode"&&i==2?"GATE":"ENV",words.translated(0,-5),7.6f,juce::Justification::centred,true,ink);
                    const auto glyph=i==1?PanelGlyph::envelope:id=="vcaMode"?PanelGlyph::gate:PanelGlyph::invertedEnvelope;
                    panelGlyph(g,glyph,{words.getCentreX()-8,labelY+5,16,8},ink);
                }
            } else if(id=="envMode") {
                // Personal photos 131048/131154 place the top and bottom
                // legends on the switch axis, with only middle GATE at left.
                if(i==0) {
                    caption(g,"GATE",{axialText.getX(),labelY-15,axialText.getWidth(),10},7.3f,juce::Justification::centred,true,ink);
                    caption(g,"+",{axialText.getX(),labelY-6,axialText.getWidth(),8},7.0f,juce::Justification::centred,true,ink);
                    caption(g,"TRIG",{axialText.getX(),labelY+1,axialText.getWidth(),10},7.3f,juce::Justification::centred,true,ink);
                } else caption(g,labels[int(i)],i==1?textBox:axialText,7.6f,
                               i==1?juce::Justification::centredRight:juce::Justification::centred,true,ink);
            } else caption(g,labels[int(i)],textBox,7.6f,labelsLeft?juce::Justification::centredRight:juce::Justification::centredLeft,true,ink);
            if(value==order[i])cap(g,{xx-5.5f,yy-9,11,18},true,false,age);
        }
        
    }
    void mouseDown(const juce::MouseEvent& e) override {
        dragging=true;pointerDownX=e.position.x;
        draggingSlot=isActuatorGesture();attachment.beginGesture();select(e.position.y,false);showValue(0);
    }
    void mouseDrag(const juce::MouseEvent& e) override {select(e.position.y,false);showValue(0);}
    void mouseEnter(const juce::MouseEvent&) override {showValue(0);}
    void mouseExit(const juce::MouseEvent&) override {if(!dragging)popup.setVisible(false);}
    void mouseUp(const juce::MouseEvent&) override {if(dragging){attachment.endGesture();dragging=false;showValue(800);}}
    void mouseDoubleClick(const juce::MouseEvent&) override {
        if(dragging){attachment.endGesture();dragging=false;}
        attachment.setValueAsCompleteGesture(float(initial));showValue(800);
    }
    bool keyPressed(const juce::KeyPress& key) override {
        auto it=std::find(order.begin(),order.end(),value);int i=int(std::distance(order.begin(),it));
        if(key==juce::KeyPress::upKey||key==juce::KeyPress::leftKey)--i;
        else if(key==juce::KeyPress::downKey||key==juce::KeyPress::rightKey)++i;
        else if(key==juce::KeyPress::homeKey)i=0;
        else if(key==juce::KeyPress::endKey)i=int(order.size())-1;
        else return false;
        attachment.setValueAsCompleteGesture(float(order[size_t(juce::jlimit(0,int(order.size())-1,i))]));showValue(800);return true;
    }
private:
    std::vector<int> order;juce::StringArray labels;bool labelsLeft=false,dragging=false,draggingSlot=false;int value=0,initial=0;
    juce::BubbleMessageComponent popup;
    void showValue(int duration) {
        if(auto* parent=getParentComponent()) {
            if(popup.getParentComponent()!=parent)parent->addChildComponent(popup);
            popup.setAlwaysOnTop(true);
            auto it=std::find(order.begin(),order.end(),value);
            const int index=int(std::distance(order.begin(),it));
            const auto label=labels[juce::jlimit(0,labels.size()-1,index)].replace("\n"," ");
            popup.setName(label);
            juce::AttributedString text; text.setJustification(juce::Justification::centred);
            // The bubble is pale; panel ink is pale too and would hide its text.
            text.append(label,font(15,true),juce::Colour(0xff22262a));
            popup.showAt(this,text,duration,true,false);
        }
    }
    float pointerDownX=0;
    juce::ParameterAttachment attachment;
    float actuatorX() const {
        const auto id=getComponentID();
        return id=="lfoWave"||id=="vcaMode"?float(getWidth())*.5f:labelsLeft?float(getWidth()-12):12.0f;
    }
    bool isActuatorGesture() const {return std::abs(pointerDownX-actuatorX())<11;}
    void select(float y,bool complete) {
        // Resolve against the same axis as paint, retaining the original
        // press position so crossing a legend during a drag cannot jump modes.
        draggingSlot=isActuatorGesture();
        const float step=draggingSlot?(order.size()==3?12.0f:10.0f):(order.size()==3?36.0f:20.0f);
        const float top=float(getHeight())*.5f-step*float(order.size()-1)*.5f;
        int i=juce::jlimit(0,int(order.size())-1,juce::roundToInt((y-top)/step));
        if(complete)attachment.setValueAsCompleteGesture(float(order[size_t(i)]));
        else attachment.setValueAsPartOfGesture(float(order[size_t(i)]));
    }
};
bool isBlack(int n) {const int a=n%12;return a==1||a==3||a==6||a==8||a==10;}
class Keyboard final : public kb09_gui::KeyboardFeedback {
public:
    explicit Keyboard(SH09Processor& p):processor(p) {
        setComponentID("keyboard");
        setMouseCursor(juce::MouseCursor::PointingHandCursor);setWantsKeyboardFocus(true);
        setTitle("32-key low-note-priority keyboard");
    }
    ~Keyboard() override {release();}
    void repaintNote(int note) {
        if(note>=53&&note<=84)repaint(keyRect(note).expanded(5).toNearestInt());
    }
    void setComputerNote(int note,bool down) override {
        if(note<0||note>=128||computerNotes[size_t(note)]==down)return;
        computerNotes[size_t(note)]=down;
        processor.queueKeyboard(note,down,false);
        repaint();
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff06090b));
        const auto* material=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=juce::jlimit(0.0f,1.0f,material!=nullptr&&material->renderingPreview?material->age:appearanceAge(processor.state.getRawParameterValue("age")->load()));
        // Continue the performance-bay paint through the first ivory's narrow
        // side clearance. Keep the rounded key, its hit area and all controls fixed.
        {
            juce::Graphics::ScopedSaveState seam(g);
            g.reduceClipRegion(juce::Rectangle<int>(0,0,1,getHeight()));
            const auto& art=artwork();
            const juce::Rectangle<float> face(-266,-6,268,344);
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(art.chassis,face.getX(),face.getY(),face.getWidth(),face.getHeight(),105,530,158,220);
            g.setOpacity(1);
            g.drawImage(art.panelWear[size_t(kb09_gui::appearanceStage(age))],face.getX(),face.getY(),face.getWidth(),face.getHeight(),105,530,158,220);
        }
        const float gloss=1.0f-age*.90f;
        std::array<bool,128> held{};
        int effectiveNote=-1;
        const bool powered=processor.isPowerEnabled();
        const int consumedMouseNote=processor.displayedKeyboardNote.load(std::memory_order_relaxed);
        // Replace the audio thread's previous mouse contribution with the live
        // mouse position. Subtract only one count: MIDI on the same pitch stays
        // held. Upward drags and releases then respond before the next callback.
        for(int n=0;n<128;++n) {
            const int midiCount=processor.keyDisplay[size_t(n)].load(std::memory_order_relaxed)-(n==consumedMouseNote?1:0)
                -processor.displayedComputerKeys[size_t(n)].load(std::memory_order_relaxed);
            held[size_t(n)]=powered&&(midiCount>0||n==mouseNote||computerNotes[size_t(n)]);
            if(effectiveNote<0&&held[size_t(n)])effectiveNote=n;
        }
        // HOLD keeps the oscillator on the last played pitch after key-up.
        // Reflect that latched pitch on the keyboard until another note or
        // VCA mode replaces it; physically held notes still take priority.
        if(powered&&effectiveNote<0&&processor.state.getRawParameterValue("vcaMode")->load()<.5f) {
            const int latched=processor.displayedHeldNote.load(std::memory_order_relaxed);
            if(latched>=53&&latched<=84){held[size_t(latched)]=true;effectiveNote=latched;}
        }
        const float scale=juce::jlimit(1.f,4.f,g.getInternalContext().getPhysicalPixelScaleFactor());
        const int range=juce::roundToInt(processor.state.getRawParameterValue("range")->load());
        if(!whiteMaterial.isValid()||whiteMaterialAge!=age) {
            whiteMaterial=artwork().whiteKey.createCopy();whiteMaterialAge=age;
            juce::Image::BitmapData pixels(whiteMaterial,juce::Image::BitmapData::readWrite);
            const auto tint=agedInk(age);
            const float red=1+(tint.getFloatRed()-1)*age*.85f;
            const float green=1+(tint.getFloatGreen()-1)*age*.85f;
            const float blue=1+(tint.getFloatBlue()-1)*age*.85f;
            for(int y=0;y<whiteMaterial.getHeight();++y)for(int x=0;x<whiteMaterial.getWidth();++x) {
                const auto c=pixels.getPixelColour(x,y);
                pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(c.getFloatRed()*red,c.getFloatGreen()*green,c.getFloatBlue()*blue,c.getFloatAlpha()));
            }
        }
        const juce::Point<int> size(getWidth(),getHeight());
        if(keyCacheAge!=age||keyCacheRange!=range||keyCacheSize!=size||std::abs(keyCacheScale-scale)>.01f) {
            keyImages={};keyCacheAge=age;keyCacheRange=range;keyCacheSize=size;keyCacheScale=scale;
        }
        for(int pass=0;pass<2;++pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass) {
            auto r=keyRect(n);
            if(!g.clipRegionIntersects(r.expanded(5).toNearestInt()))continue;
            const bool down=held[size_t(n)];
            const float depressed=down?3.5f:0;
            r=r.withTrimmedTop(depressed);
            const auto paintKey=[&](juce::Graphics& g) {
            juce::Graphics::ScopedSaveState keyClip(g);
            if(pass==0) {
                // The photographed edge is oblique; use symmetric physical
                // corner geometry instead of inheriting its cropped left corner.
                juce::Path outline;outline.addRoundedRectangle(r.reduced(.65f,0),5.2f);g.reduceClipRegion(outline);
                const auto ivory=juce::Colour(0xffefede6).interpolatedWith(juce::Colour(0xffd1bd86),age*.72f);
                auto key=r.reduced(.65f,0);
                const float frontHeight=down?0.f:2.8f;
                auto face=key.withTrimmedTop(key.getHeight()-frontHeight);
                auto top=key.withTrimmedBottom(frontHeight);
                juce::ColourGradient enamel(ivory.darker(.23f),0,top.getY(),ivory,0,top.getBottom(),false);
                enamel.addColour(.07,ivory.darker(.09f));enamel.addColour(.55,ivory.brighter(.04f*gloss));
                enamel.addColour(.975,ivory.brighter(.09f*gloss));g.setGradientFill(enamel);g.fillRect(top);
                g.setGradientFill(juce::ColourGradient(ivory.darker(.19f),0,face.getY(),ivory.darker(.04f),0,face.getBottom(),false));
                g.fillRoundedRectangle(face,1.4f);
                g.setColour(juce::Colours::white.withAlpha(.55f*gloss));g.drawLine(key.getX()+1,top.getY()+2,key.getX()+1,top.getBottom()-2,.8f);
                g.setColour(juce::Colour(0xff635e54).withAlpha(.28f));g.drawLine(key.getRight()-1,top.getY(),key.getRight()-1,top.getBottom(),1.2f);
                g.setColour(juce::Colours::white.withAlpha(.62f*gloss));g.drawLine(key.getX()+2,face.getY(),key.getRight()-2,face.getY(),.8f);
                g.setColour(juce::Colours::black.withAlpha(.26f));g.fillRect(key.getX()+1,key.getBottom()-2,key.getWidth()-2,2.0f);
                // Retain the photographed enamel and subtle reflections even
                // at full age; a restrained tint supplies polymer yellowing.
                g.setOpacity(1.0f-age*.22f);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(whiteMaterial,top,juce::RectanglePlacement::stretchToFit);
                g.setOpacity(1.0f);
                // Symmetric top bevels border dark gaps. The photo supplies only
                // enamel, never an oblique side or an exaggerated front face.
                g.setColour(juce::Colours::white.withAlpha(.22f*gloss));
                g.drawRoundedRectangle(top.reduced(1.1f),4.3f,.55f);
                g.setColour(juce::Colours::black.withAlpha(.30f));
                g.drawRoundedRectangle(key.reduced(.2f),5.0f,.65f);
                if(!down) {
                    g.setGradientFill(juce::ColourGradient(ivory.darker(.09f),0,face.getY(),ivory.darker(.17f),0,face.getBottom(),false));
                    g.fillRect(face);
                    g.setColour(ivory.brighter(.07f).withAlpha(.45f*gloss));
                    g.drawLine(key.getX()+3,face.getY(),key.getRight()-3,face.getY(),.6f);
                }
                const float variation=float(n-53)/31;
                // Polymer and printing age separately. Strong warm ivory at the
                // endpoint follows the uneven cream keys in original-unit photos.
                g.setColour(juce::Colour(0xffb9a267).withAlpha(.008f+variation*.025f+age*(.067f+variation*.034f)));
                g.fillRoundedRectangle(key.reduced(.5f),4.7f);
            } else {
                for(int i=5;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.07f));g.fillRoundedRectangle(r.expanded(float(i)*.45f).translated(2,3),1.8f);}
                juce::Path outline;outline.addRoundedRectangle(r,2.5f);g.reduceClipRegion(outline);
                auto face=r.withTrimmedTop(r.getHeight()-3),top=r.withTrimmedBottom(3);
                const auto matte=[age](juce::Colour fresh,juce::Colour worn) {return fresh.interpolatedWith(worn,age);};
                juce::ColourGradient plastic(matte(juce::Colour(0xff111517),juce::Colour(0xff151614)),top.getX(),top.getY(),
                                            matte(juce::Colour(0xff171b1d),juce::Colour(0xff191a17)),top.getRight(),top.getY(),false);
                plastic.addColour(.08,matte(juce::Colour(0xff464c4e),juce::Colour(0xff242520)));
                plastic.addColour(.17,matte(juce::Colour(0xff292e30),juce::Colour(0xff1c1d19)));
                plastic.addColour(.80,matte(juce::Colour(0xff242a2c),juce::Colour(0xff1b1c18)));
                plastic.addColour(.94,matte(juce::Colour(0xff0b1013),juce::Colour(0xff131411)));
                g.setGradientFill(plastic);g.fillRoundedRectangle(top,1.5f);
                g.setGradientFill(juce::ColourGradient(matte(juce::Colour(0xff0a0d0f),juce::Colour(0xff10110f)),0,face.getY(),
                    matte(juce::Colour(0xff202629),juce::Colour(0xff181915)),0,face.getBottom(),false));g.fillRoundedRectangle(face,1.8f);
                g.setColour(juce::Colours::white.withAlpha(.24f*gloss));g.drawLine(top.getX()+3,top.getY()+2,top.getX()+3,top.getBottom()-2,.6f);
                g.drawLine(face.getX()+2,face.getY(),face.getRight()-2,face.getY(),.7f);
                // Keep a little uneven photographic reflection at full age:
                // the black plastic wears, but does not turn into flat paint.
                const float reflected=.34f+.06f*surfaceNoise(uint32_t(n)*6197u);
                g.setOpacity(1.0f-age*(1.0f-reflected));g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(artwork().blackKey,r,juce::RectanglePlacement::stretchToFit);
                g.setOpacity(1.0f);
                // The moulded front chamfer is visible from the centred view.
                // Paint it after the photographic surface, so that it survives.
                juce::Path bevel;bevel.startNewSubPath(r.getX()+4,r.getBottom()-10);
                bevel.lineTo(r.getRight()-4,r.getBottom()-10);bevel.lineTo(r.getRight()-1,r.getBottom()-1);
                bevel.lineTo(r.getX()+1,r.getBottom()-1);bevel.closeSubPath();
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff555953),0,r.getBottom()-10,
                    juce::Colour(0xff0b0d0c),0,r.getBottom()-1,false));g.fillPath(bevel);
                g.setColour(juce::Colour(0xffaeb6b2).withAlpha(.24f*(1-age*.5f)));
                g.drawLine(r.getX()+4,r.getBottom()-10,r.getRight()-4,r.getBottom()-10,.65f);
                // A worn surface spreads its reflected light across a wider area.
                juce::ColourGradient reflection(juce::Colours::transparentWhite,top.getX(),top.getY(),
                    juce::Colours::transparentWhite,top.getRight(),top.getY(),false);
                reflection.addColour(.15,juce::Colours::white.withAlpha(age*.055f));
                reflection.addColour(.40,juce::Colours::white.withAlpha(age*.025f));
                g.setGradientFill(reflection);g.fillRoundedRectangle(top,1.5f);
                g.setColour(juce::Colour(0xff6b6253).withAlpha(.004f+float(n-53)*.0012f));g.fillRoundedRectangle(r.reduced(.6f),1.2f);
            }
            // Each note receives independent tint and microgeometry over its
            // photographic material, rather than repeated wear decals.
            const int serial=n-53;
            // Tiny individual polish reflections keep even pristine keys from
            // sharing an identical surface after 8-bit tint quantisation.
            const float polishX=r.getCentreX()+(pass==0?-8.0f:-6.0f)+float((serial*7)%(pass==0?17:13));
            const float polishY=r.getBottom()-(pass==0?65.0f:48.0f)+float((serial*13)%(pass==0?41:30));
            g.setColour(juce::Colours::white.withAlpha((pass==0?.24f:.055f)*gloss));g.fillEllipse(polishX,polishY,.85f,1.5f);
            keySurfaceWear(g,r,serial,pass!=0,age);
            const float crackOnset=n==60?.56f:n==65?.68f:n==74?.79f:2.0f;
            if(age>=crackOnset) {
                const float progress=juce::jlimit(0.0f,1.0f,(age-crackOnset)/(.90f-crackOnset));
                const float width=r.getWidth()*.20f,height=r.getHeight()*(n==65?.18f:.36f);
                const auto trace=juce::Rectangle<float>(r.getX()+r.getWidth()*.32f,r.getBottom()-height-17,width,height);
                juce::Graphics::ScopedSaveState save(g);
                const float revealed=trace.getHeight()*(n==60?(.12f+.88f*progress):(.04f+.96f*progress));
                g.reduceClipRegion(trace.withTrimmedTop(trace.getHeight()-revealed).toNearestInt());
                // Trace the owner's gently wandering hairline as a native
                // path: subpixel width at M, sharp at 4K, no bitmap dirt halo.
                juce::Path hairline;hairline.startNewSubPath(.56f,0);
                hairline.cubicTo(.58f,.12f,.32f,.19f,.39f,.29f);
                hairline.cubicTo(.41f,.38f,.65f,.44f,.59f,.53f);
                hairline.cubicTo(.57f,.65f,.40f,.71f,.45f,.80f);
                hairline.cubicTo(.46f,.87f,.22f,.95f,.15f,1);
                hairline.startNewSubPath(.59f,.53f);hairline.lineTo(.84f,.58f);
                if(n==65) {
                    hairline.clear();hairline.startNewSubPath(.8f,0);hairline.cubicTo(.72f,.18f,.27f,.19f,.43f,.38f);
                    hairline.cubicTo(.66f,.55f,.28f,.68f,.36f,.82f);hairline.lineTo(.19f,1);
                } else if(n==74) {
                    hairline.clear();hairline.startNewSubPath(.22f,0);hairline.cubicTo(.32f,.21f,.69f,.25f,.54f,.42f);
                    hairline.cubicTo(.39f,.6f,.76f,.74f,.63f,1);
                }
                const auto transform=juce::AffineTransform::scale(trace.getWidth(),trace.getHeight()).translated(trace.getX(),trace.getY());
                hairline.applyTransform(transform);
                g.setColour(juce::Colour(0xff554835).withAlpha(.78f));
                g.strokePath(hairline,juce::PathStrokeType(n==60?.32f:n==65?.34f:.33f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
            }
            if(pass==0&&n%12==0) {
                const int octave=n/12-1+juce::roundToInt(processor.state.getRawParameterValue("range")->load())-2;
                caption(g,"C"+juce::String(octave),{r.getX()+2,r.getBottom()-25,r.getWidth()-4,15},10.5f,
                    juce::Justification::centred,false,juce::Colour(0xff514d43).withAlpha(.87f));
            }
            if(down) {
                g.setColour(juce::Colours::black.withAlpha(pass==0?.14f:.18f));g.fillRoundedRectangle(r.reduced(1),1.5f);
                if(effectiveNote==n) {
                    g.setColour(amber);g.fillRoundedRectangle(r.getX()+9,r.getBottom()-7,r.getWidth()-18,3,1.0f);
                }
            }
            };
            const auto region=keyRect(n).expanded(5).getSmallestIntegerContainer();
            auto& image=keyImages[size_t(n-53)][size_t(down?(effectiveNote==n?2:1):0)];
            if(!image.isValid()) {
                image=juce::Image(juce::Image::ARGB,juce::roundToInt(region.getWidth()*scale),juce::roundToInt(region.getHeight()*scale),true,juce::SoftwareImageType());
                juce::Graphics raster(image);
                raster.addTransform(juce::AffineTransform::scale(scale).translated(-region.getX()*scale,-region.getY()*scale));
                paintKey(raster);
            }
            g.drawImage(image,region.toFloat());
        }
        g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.68f),0,0,juce::Colours::transparentBlack,0,20,false));
        g.fillRect(0,0,getWidth(),20);
    }
    void mouseDown(const juce::MouseEvent& e) override {changeNote(noteAt(e.position));}
    void mouseDrag(const juce::MouseEvent& e) override {changeNote(noteAt(e.position));}
    void mouseUp(const juce::MouseEvent&) override {release();}
    void focusLost(FocusChangeType) override {release();}
    void visibilityChanged() override {if(!isShowing())release();}
private:
    SH09Processor& processor;int mouseNote=-1;std::array<bool,128> computerNotes{};
    std::array<std::array<juce::Image,3>,32> keyImages;
    juce::Image whiteMaterial;float whiteMaterialAge=-1;
    float keyCacheAge=-1,keyCacheScale=0;int keyCacheRange=-1;juce::Point<int> keyCacheSize;

    juce::Rectangle<float> keyRect(int n) const {
        int whites=0;for(int i=53;i<n;++i)if(!isBlack(i))++whites;
        const float w=float(getWidth())/19;
        if(isBlack(n))return {float(whites)*w-w*.31f,0,w*.62f,float(getHeight())*.64f};
        const auto* material=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=material&&material->renderingPreview?material->age:appearanceAge(processor.state.getRawParameterValue("age")->load());
        const float shift=(n==64?-1.25f:n==65?1.15f:(surfaceNoise(uint32_t(n)*7919u)-.5f)*.75f)*age;
        const float narrowing=(n==71?.85f:.18f)*age;
        return {float(whites)*w+shift+narrowing*.5f,0,w-narrowing,float(getHeight())};
    }
    int noteAt(juce::Point<float> p) const {
        for(int pass=1;pass>=0;--pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass&&keyRect(n).contains(p))return n;
        return -1;
    }
    void changeNote(int n) {
        if(n==mouseNote)return;const int previous=mouseNote;mouseNote=n;
        if(n>=0)processor.queueKeyboard(n,true);if(previous>=0)processor.queueKeyboard(previous,false);repaint();
    }
    void release() {if(mouseNote>=0){processor.queueKeyboard(mouseNote,false);mouseNote=-1;repaint();}}
};
class Meter final : public juce::Component, private juce::Timer {
    class Indicator final : public juce::Component, public juce::SettableTooltipClient {
    public:
        juce::String label;float level=0;bool midi=false;
        void paint(juce::Graphics& g) override {
            caption(g,label,{0,0,midi?31.0f:25.0f,20},9,juce::Justification::centredLeft);
            if(midi){g.setColour(level>0?amber:juce::Colour(0xff373b39));g.fillEllipse(34,6,8,8);}
            else for(int i=0;i<8;++i){g.setColour(level>float(i)/8?amber:juce::Colour(0xff373b39));g.fillRect(28.0f+float(i*6),6.0f,4.0f,8.0f);}
        }
    };
public:
    explicit Meter(SH09Processor& p):processor(p) {
        const char* labels[]={"ENV","EXT","IN","MIDI"};
        const char* tips[]={"ENV: the synth's ADSR envelope level.","EXT: external audio envelope follower level, measured before the EXT fader.","IN: incoming audio level before the EXT fader and stereo-to-mono conversion.","MIDI: incoming notes, pitch bend and controller activity. This is not an audio meter."};
        for(int i=0;i<4;++i){auto& lamp=indicators[size_t(i)];lamp.label=labels[i];lamp.midi=i==3;lamp.setTooltip(tips[i]);lamp.setComponentID(juce::String("indicator")+labels[i]);addAndMakeVisible(lamp);lamp.setBounds(i*83,0,i==3?44:78,20);}
        startTimerHz(120);
    }
    ~Meter() override {stopTimer();}
    void setLevels(float e,float f,float input,unsigned serial) {
        const auto now=juce::Time::getMillisecondCounter();
        if(serial!=lastSerial){lastSerial=serial;lastActivity=now;hasActivity=true;}
        const float levels[]={e,f,input,hasActivity&&juce::uint32(now-lastActivity)<90?1.0f:0.0f};
        for(int i=0;i<4;++i){auto& lamp=indicators[size_t(i)];if(std::abs(lamp.level-levels[i])>.005f){lamp.level=levels[i];lamp.repaint();}}
    }
private:
    SH09Processor& processor;
    void timerCallback() override {
        setLevels(processor.displayedEnvelope.load(std::memory_order_relaxed),processor.displayedFollower.load(std::memory_order_relaxed),
                  processor.displayedInput.load(std::memory_order_relaxed),processor.midiActivitySerial.load(std::memory_order_relaxed));
    }
    std::array<Indicator,4> indicators;
    unsigned lastSerial=0;juce::uint32 lastActivity=0;bool hasActivity=false;
};
class RateLamp final : public juce::Component, public juce::SettableTooltipClient {
public:
    RateLamp(){setTooltip("Modulator rate light: follows the LFO cycle and RATE setting.");}
    void setLevel(float value) {if(std::abs(on-value)>.005f){on=value;repaint();}}
    void paint(juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().withSizeKeepingCentre(8,8).reduced(1);
        g.setColour(juce::Colour(0xff05090b));g.fillEllipse(r);
        g.setColour(juce::Colour(0xff471c17).interpolatedWith(juce::Colour(0xffed5035),on));g.fillEllipse(r.reduced(1.1f));
        g.setColour(juce::Colour(0xffffc493).withAlpha(.12f+on*.53f));g.fillEllipse(r.getX()+2,r.getY()+1.8f,1.6f,1.6f);
    }
private:float on=0;
};
class PowerButton final : public juce::Button {
public:
    explicit PowerButton(SH09Processor& p):Button("Power"),processor(p) {
        setComponentID("power");setTooltip("Instrument audio on/off. This setting is not stored in patches.");
        onClick=[this]{processor.setPowerEnabled(!processor.isPowerEnabled());repaint();if(onPowerChanged)onPowerChanged();};
    }
    std::function<void()> onPowerChanged;
    void paintButton(juce::Graphics& g,bool over,bool down) override {
        const auto* appearance=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=appearance&&appearance->renderingPreview?appearance->age:appearanceAge(processor.state.getRawParameterValue("age")->load());
        const bool pressed=processor.isPowerEnabled()||down;
        const auto recess=getLocalBounds().toFloat().withTrimmedTop(10).reduced(.7f);
        // The grey pushbutton stands tall when OFF. Project its estimated
        // physical height through the same 11-degree view as the other hardware;
        // ON lowers the same cap without shrinking its top or changing camera.
        g.setColour(juce::Colour(0xff050605));g.fillRoundedRectangle(recess,1.3f);
        g.setColour(juce::Colour(0xff43433b).withAlpha(.36f));g.drawRoundedRectangle(recess,1.3f,.45f);
        const auto footprint=recess.reduced(1.35f,1.2f).withTrimmedTop(3.0f);
        const float wallHeight=(pressed?31.0f:60.0f)*panelViewSin;
        const auto top=footprint.translated(0,-wallHeight);
        g.setColour(juce::Colours::black.withAlpha(pressed?.35f:.47f));
        g.fillRoundedRectangle(footprint.translated(.45f,.25f+wallHeight*.34f),1.1f);
        const auto grey=juce::Colour(0xff959b98).interpolatedWith(juce::Colour(0xff85877e),age*.65f);
        const auto wall=juce::Rectangle<float>(top.getX(),top.getBottom()-2,top.getWidth(),wallHeight+2);
        g.setGradientFill(juce::ColourGradient(grey.darker(.13f),0,wall.getY(),grey.darker(.23f),0,wall.getBottom(),false));
        // Retain straight skirt faces, with only a small molded edge radius.
        // A narrow light roll describes the rounded vertical edge rather than
        // rounding the entire bottom into a second pill-shaped cap.
        juce::Path skirt;skirt.addRoundedRectangle(wall,.85f);
        g.fillPath(skirt);
        {
            juce::Graphics::ScopedSaveState edge(g);g.reduceClipRegion(skirt);
            // Edge normals turn away from the upper-left light. Keep the
            // broad front wall readable, with a stronger falloff on the right.
            juce::ColourGradient roll(juce::Colours::black.withAlpha(.43f),wall.getX(),0,
                juce::Colours::black.withAlpha(.60f),wall.getRight(),0,false);
            roll.addColour(.032,juce::Colours::black.withAlpha(.23f));
            roll.addColour(.08,juce::Colours::black.withAlpha(.08f));
            roll.addColour(.14,juce::Colours::transparentBlack);
            roll.addColour(.80,juce::Colours::black.withAlpha(.025f));
            roll.addColour(.92,juce::Colours::black.withAlpha(.12f));
            roll.addColour(.968,juce::Colours::black.withAlpha(.32f));
            g.setGradientFill(roll);g.fillRect(wall);
            g.setGradientFill(juce::ColourGradient(juce::Colours::transparentBlack,0,wall.getBottom()-.8f,
                juce::Colours::black.withAlpha(.16f),0,wall.getBottom(),false));
            g.fillRect(wall.withTop(wall.getBottom()-.8f));
        }
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        g.drawImage(capSurface(top.getWidth(),top.getHeight(),age,over),top,juce::RectanglePlacement::stretchToFit);
    }
private:
    const juce::Image& capSurface(float width,float height,float age,bool over) {
        // A rounded moulded bevel, rather than a thin outlined inset. Surface
        // normals blend into the flat centre and give the front corner a soft
        // diffuse highlight. Dimensions are photo-informed visual estimates.
        const auto* look=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float scale=look!=nullptr&&look->conditionDragging?2.0f:6.0f;
        constexpr float bevel=1.5f,radius=3.0f;
        const int w=juce::roundToInt(width*scale),h=juce::roundToInt(height*scale);
        if(cap.isValid()&&cap.getWidth()==w&&cap.getHeight()==h&&capAge==age&&capOver==over)return cap;
        cap=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType());capAge=age;capOver=over;
        const auto base=juce::Colour(0xff959b98).interpolatedWith(juce::Colour(0xff85877e),age*.65f);
        juce::Image::BitmapData pixels(cap,juce::Image::BitmapData::writeOnly);
        for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
            const float px=(x+.5f)*width/w-width*.5f,py=(y+.5f)*height/h-height*.5f;
            const float qx=std::abs(px)-(width*.5f-radius),qy=std::abs(py)-(height*.5f-radius);
            const float dx=juce::jmax(qx,0.0f),dy=juce::jmax(qy,0.0f),length=std::sqrt(dx*dx+dy*dy);
            const float distance=length+juce::jmin(juce::jmax(qx,qy),0.0f)-radius;
            const float alpha=juce::jlimit(0.0f,1.0f,.5f-distance*scale);
            if(alpha<=0)continue;
            float gx=0,gy=0;
            if(length>0){gx=dx/length;gy=dy/length;}else if(qx>qy)gx=1;else gy=1;
            gx*=px<0?-1.0f:1.0f;gy*=py<0?-1.0f:1.0f;
            const float t=juce::jlimit(0.0f,1.0f,-distance/bevel);
            const float angle=(1-t)*juce::MathConstants<float>::halfPi;
            const float nx=gx*std::sin(angle),ny=gy*std::sin(angle),nz=std::cos(angle);
            const float diffuse=juce::jmax(0.0f,-nx*.35f-ny*.42f+nz*.84f);
            const float specular=std::pow(juce::jmax(0.0f,-nx*.19f-ny*.23f+nz*.953f),16)*(.075f-age*.035f);
            const float grain=(kb09_gui::power_detail::smoothNoise((px+width)*7,(py+height)*7,9457u)-.5f)*(.045f+age*.015f);
            const float edgeReflection=(1-age)*(
                .22f*std::exp(-std::pow((px+width*.5f-1.55f)/.65f,2.f))
                    *std::exp(-std::pow((py+height*.20f)/(height*.35f),4.f))
                +.12f*std::exp(-std::pow((py+height*.5f-1.25f)/.55f,2.f))
                    *std::exp(-std::pow((px+width*.20f)/(width*.42f),4.f)));
            const float cloud=.014f*std::sin(px*.17f+std::sin(py*.23f))-.009f*age*(1-t)
                +.035f*std::exp(-std::pow((px+width*.20f)/(width*.55f),2.f)-std::pow((py+height*.22f)/(height*.4f),2.f));
            const float shade=.73f+.28f*diffuse+specular+grain+cloud+edgeReflection+(over?.009f:0);
            pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                juce::jlimit(0.0f,1.0f,base.getFloatRed()*shade),
                juce::jlimit(0.0f,1.0f,base.getFloatGreen()*shade),
                juce::jlimit(0.0f,1.0f,base.getFloatBlue()*shade),alpha));
        }
        return cap;
    }
    SH09Processor& processor;
    juce::Image cap;
    float capAge=-1;
    bool capOver=false;
};
class Header final : public juce::Component {
public:
    explicit Header(sh09::WindowSizing& s):sizing(s) {setComponentID("windowHeader");setTitle("Window header");setMouseCursor(juce::MouseCursor::DraggingHandCursor);}
    void paint(juce::Graphics& g) override {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff25292b),0,0,juce::Colour(0xff111517),0,float(getHeight()),false));g.fillAll();
        if(!bool(getProperties().getWithDefault("standalone",false)))caption(g,"MONOPHONIC",{241,0,140,float(getHeight())},9,juce::Justification::centredLeft,false);
        if(sizing.isSnapArmed()){g.setColour(amber);g.drawRect(getLocalBounds(),2);}
    }
    void mouseDown(const juce::MouseEvent& e) override {sizing.beginHeaderDrag(e);}
    void mouseDrag(const juce::MouseEvent& e) override {sizing.dragHeader(e);}
    void mouseUp(const juce::MouseEvent& e) override {sizing.endHeaderDrag(e);}
    void mouseDoubleClick(const juce::MouseEvent&) override {sizing.doubleClickHeader();}
private:sh09::WindowSizing& sizing;
};
class MaximiseButton final : public juce::Button {
public:
    explicit MaximiseButton(sh09::WindowSizing& s):Button("Maximise"),sizing(s) {
        setTooltip("Maximise / restore the instrument editor");onClick=[this]{sizing.toggleMaximise();};
    }
    void paintButton(juce::Graphics& g,bool over,bool down) override {
        if(over||down){g.setColour(juce::Colour(0xff5d6465).withAlpha(down?.65f:.4f));g.fillAll();}
#if JUCE_MAC
        g.setColour(juce::Colour(0xff29c941));g.fillEllipse(float(getWidth())*.5f-7,float(getHeight())*.5f-7,14,14);
#else
        const float x=float(getWidth())*.5f-5,y=float(getHeight())*.5f-5;
        g.setColour(ink);
        if(sizing.isLarge()) {g.drawRect(x+2,y-2,9.0f,9.0f,1.0f);g.setColour(juce::Colour(0xff1c2022));g.fillRect(x-1,y+1,9.0f,9.0f);g.setColour(ink);g.drawRect(x-1,y+1,9.0f,9.0f,1.0f);}
        else {g.drawRect(x,y,10.0f,10.0f,1.0f);g.fillRect(x,y,10.0f,1.8f);}
#endif
    }
private:sh09::WindowSizing& sizing;
};
void module(juce::Graphics& g,float x,float w,const char* title,float age=0) {
    const auto ink=agedInk(age);
    g.setColour(ink.withAlpha(.84f));g.drawRoundedRectangle(x,57.0f,w,182.0f,2.2f,.85f);
    if(juce::String(title)=="VCO") {
        // The original legend interrupts the rule on its axis, beside TUNE.
        g.drawLine(x+1,83,313,83,.85f);g.drawLine(369,83,x+w-1,83,.85f);
    } else g.drawLine(x+1,83,x+w-1,83,.85f);
    caption(g,title,{x+3,59,w-6,22},13,juce::Justification::centred,true,ink);
}
void arcLabels(juce::Graphics& g,juce::Point<float> centre,
               const std::vector<std::pair<juce::String,float>>& labels,float radius,float age=0) {
    const auto ink=agedInk(age);
    for(const auto& [text,angle]:labels) {
        const auto p=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*radius;
        const auto q=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*(radius-4);
        g.setColour(ink);g.drawLine({p,q},.75f);
        auto label=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*(radius+7);
        // The neighbouring 2-foot and NOISE detents share the space between
        // selectors. Place NOISE above its knob while preserving the tick.
        if(text=="NOISE")label.y-=6;
        if(text=="SAW"||text=="SQR"||text=="PULSE")
            panelGlyph(g,text=="SAW"?PanelGlyph::saw:text=="SQR"?PanelGlyph::square:PanelGlyph::pulse,
                       {label.x-7.5f,label.y-4,15,8},ink,.95f);
        else caption(g,text,{label.x-21,label.y-7,42,14},text=="NOISE"?7.4f:8.4f,juce::Justification::centred,true,ink);
    }
}
}

class SH09Editor::Panel final : public juce::Component, private juce::Timer, private juce::KeyListener {
public:
    void bufferUnchangedHardware() {
        bufferInSoftware(keyboard,true);bufferInSoftware(bender,true);bufferInSoftware(power,true);
        for(auto& c:controls) {
            // Cache expensive knob material; lightweight fader caps stay live.
            if(c->slider)bufferInSoftware(*c->slider,c->slider->isRotary());
            if(c->selector)bufferInSoftware(*c->selector,true);
        }
    }
    void cacheForResize() {
        if(!resizeBackdrop.isValid()) {
            resizeBackdrop=juce::Image(juce::Image::RGB,designW,designH,true,juce::SoftwareImageType());
            renderingCondition=true;{juce::Graphics frame(resizeBackdrop);paint(frame);}renderingCondition=false;
        }
        resizeCacheUntil=juce::Time::getMillisecondCounterHiRes()+180;
        bufferInSoftware(keyboard,true);bufferInSoftware(bender,true);bufferInSoftware(power,true);
        for(auto& c:controls) {if(c->slider)bufferInSoftware(*c->slider,true);if(c->selector)bufferInSoftware(*c->selector,true);}
    }
    Panel(SH09Processor& p,sh09::WindowSizing& s)
        :processor(p),sizing(s),presetFiles(p,*this),keyboard(p),header(s),maximise(s),power(p),meter(p) {
        setLookAndFeel(&look);setSize(designW,designH);
        addAndMakeVisible(header);header.setBounds(0,0,designW,headerH);
        addAndMakeVisible(updates);updates.setBounds(25,5,132,24);updates.setButtonText("TOYLAND / KB-09");updates.setComponentID("updates");updates.onClick=[this]{kb09::Updates::instance().showMenu(&updates);};
        addAndMakeVisible(keyboard);keyboard.setBounds(326,headerH+370,1102,337);
        for(int i=0;i<processor.getNumPrograms();++i)presets.addItem(processor.getProgramName(i),i+1);
        addAndMakeVisible(presets);presets.setBounds(441,5,336,24);presets.setComponentID("factoryPreset");
        presets.setTooltip("Factory patch. Loading replaces current sound settings.");
        presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);
        presets.onChange=[this]{if(presets.getSelectedId()>0){userPreset=false;processor.setCurrentProgram(presets.getSelectedId()-1);}};
        addAndMakeVisible(previous);previous.setButtonText("<");previous.setBounds(410,5,25,24);
        previous.onClick=[this]{userPreset=false;processor.setCurrentProgram((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms());};
        addAndMakeVisible(next);next.setButtonText(">");next.setBounds(783,5,25,24);
        next.onClick=[this]{userPreset=false;processor.setCurrentProgram((processor.getCurrentProgram()+1)%processor.getNumPrograms());};
        addAndMakeVisible(savePreset);savePreset.setButtonText("SAVE");savePreset.setBounds(820,5,56,24);savePreset.setComponentID("savePreset");
        savePreset.setTooltip("Save the complete sound as a reusable .kb09 preset.");savePreset.onClick=[this]{presetFiles.savePreset();};
        addAndMakeVisible(loadPreset);loadPreset.setButtonText("LOAD");loadPreset.setBounds(882,5,56,24);loadPreset.setComponentID("loadPreset");
        loadPreset.setTooltip("Load a saved .kb09 preset.");loadPreset.onClick=[this]{presetFiles.loadPreset();};
        presetFiles.onCompleted=[this](const juce::Result& result,const juce::File& file,bool wasLoad) {
            if(result.wasOk()) {
                if(!wasLoad)savePreset.setTooltip("Saved preset: "+file.getFileName());
                if(wasLoad){userPreset=true;lastProgramRevision=processor.getProgramRevision();lastProgram=processor.getCurrentProgram();presets.setSelectedId(0,juce::dontSendNotification);presets.setText(file.getFileNameWithoutExtension(),juce::dontSendNotification);}
            }
        };
        addAndMakeVisible(meter);meter.setBounds(956,7,293,20);
        addAndMakeVisible(small);small.setButtonText("S");small.setBounds(1299,5,35,24);small.setComponentID("sizeS");
        small.setTooltip("Small: fit within 65% of this screen's work-area width and height, with the complete instrument visible.");small.onClick=[this]{sizing.chooseSmall();};
        addAndMakeVisible(medium);medium.setButtonText("M");medium.setBounds(1340,5,35,24);medium.setComponentID("sizeM");
        medium.setTooltip("Medium: fit within 90% of this screen's work-area width and height.");medium.onClick=[this]{sizing.chooseMedium();};
        addAndMakeVisible(large);large.setButtonText("L");large.setBounds(1381,5,35,24);large.setComponentID("sizeL");
        large.setTooltip("Large: fill this screen's available work area. For sharpness in Live, disable Auto-Scale Plug-in Window if available, then create a fresh instance.");large.onClick=[this]{sizing.chooseLarge();};
        if(processor.wrapperType==juce::AudioProcessor::wrapperType_Standalone) {
            // The native caption owns maximise; end the visible row at the
            // same insets as the footer controls.
            small.setTopLeftPosition(1358,5);
            medium.setTopLeftPosition(1399,5);
            large.setTopLeftPosition(1440,5);
            header.getProperties().set("standalone",true);
            addAndMakeVisible(audioMidi);audioMidi.setComponentID("audioMidiSettings");
            audioMidi.setButtonText("AUDIO / MIDI");audioMidi.setBounds(289,5,115,24);
            setWantsKeyboardFocus(true);addKeyListener(this);
            addAndMakeVisible(computerKeyboard);computerKeyboard.setComponentID("computerKeyboard");
            computerKeyboard.setBounds(161,5,122,24);computerKeyboard.setClickingTogglesState(true);
            computerKeyboard.setWantsKeyboardFocus(false);
            computerKeyboard.setColour(juce::TextButton::buttonOnColourId,amber);
            computerKeyboard.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff151a1d));
            computerKeyboard.setTooltip("Computer keyboard: A W S E D F T G Y H U J K play C to C. Z / X shift octave. Standalone only.");
            computerKeyboard.onClick=[this]{releaseComputerNotes();updateComputerLabel();if(computerKeyboard.getToggleState())grabKeyboardFocus();};
            updateComputerLabel();
            audioMidi.setTooltip("Choose your audio output device and enable your MIDI controller.");
            audioMidi.onClick=[this] {
                if(auto* holder=juce::StandalonePluginHolder::getInstance())
                    kb09_gui::AudioSettingsDialog::show(holder->deviceManager,this,
                        holder->getProcessorHasPotentialFeedbackLoop()?&holder->shouldMuteInput:nullptr);
            };
        }
        if(processor.wrapperType!=juce::AudioProcessor::wrapperType_Standalone)addAndMakeVisible(maximise);
        maximise.setBounds(1428,0,47,34);maximise.setComponentID("maximise");
        sizing.onChange=[this]{header.repaint();maximise.repaint();};
        addSwitch("lfoWave",106,104,49,121,{0,1,2},{"SINE","SQUARE","RANDOM\n(S/H)"},"Modulator: sine, square or random sample-and-hold.");
        addFader("lfoRate","RATE",178,96,33,129);controls.back()->label->setVisible(false);addFader("lfoDelay","DELAY",214,96,33,129);
        controls.back()->label->setVisible(false); // Sine + DELAY TIME is printed by the panel.
        addAndMakeVisible(rateLamp);rateLamp.setBounds(126,headerH+98,9,8);rateLamp.setInterceptsMouseClicks(false,false);
        addFader("vcoMod","MOD",284,96,45,129);
        controls.back()->slider->getProperties().set("scaleLeft",true);
        addKnob("range",356,169,87);addKnob("wave",456,169,87);addKnob("tune",356,86,22);
        addFader("pulseWidth","PULSE\nWIDTH",538,96,37,129);
        controls.back()->label->setVisible(false); // Shared PULSE WIDTH MOD bracket replaces this caption.
        addSwitch("pwmMode",561,104,51,121,{1,0,2},{"LFO","MANUAL","ENV"},"Pulse width: LFO, manual or envelope. LFO uses the undelayed sine.");
        addSwitch("subMode",633,104,60,121,{0,1,2},{"1 OCT\nDOWN","2 OCT\nDOWN","2 OCT\nDOWN"},"Sub oscillator: one octave down square, two octaves down square, or two octaves down pulse.");
        addFader("vcoLevel","VCO",732,96,37,129);addFader("subLevel","SUB\nOSC",770,96,37,129);addFader("extLevel","EXT",808,96,37,129);
        addFader("cutoff","CUTOFF\nFREQ",884,96,33,129);
        controls.back()->label->setBounds(866,headerH+89,36,20);
        controls.back()->label->setFont(font(8.8f,true));
        addFader("resonance","RESONANCE",919,96,33,129);
        controls.back()->label->setBounds(903,headerH+95,52,20);
        controls.back()->label->setFont(font(9.2f,true));
        addSwitch("vcfEnvMode",950,104,61,121,{2,0,1},{"ENV\nFOL'R","ENV +","ENV -"},"VCF envelope follower, positive envelope, or inverted envelope.",true);
        addFader("vcfEnv","ENV",1035,96,35,129);controls.back()->label->setVisible(false);addFader("vcfMod","MOD",1071,96,35,129);
        addSwitch("vcaMode",1110,104,56,121,{0,1,2},{"HOLD","ENV","GATE"},"HOLD passes audio without MIDI; ENV uses ADSR; GATE follows held keys.");
        // Balance hardware margins: left edge of switch track to divider,
        // right edge of RELEASE track to divider. Move labels/hit areas too.
        constexpr int envContentOffset=-7;
        addSwitch("envMode",1175+envContentOffset,104,55,121,{0,1,2},{"GATE +\nTRIG","GATE","LFO"},"Gate+Trigger re-attacks on low-note changes. Gate preserves legato. LFO repeats while held.",true);
        addFader("attack","A",1260+envContentOffset,96,31,129,true);addFader("decay","D",1292+envContentOffset,96,31,129,true);
        addFader("sustain","S",1324+envContentOffset,96,31,129,true);addFader("release","R",1356+envContentOffset,96,31,129,true);
        addKnob("volume",140,442,88);
        addAndMakeVisible(power);power.setBounds(231,headerH+398,50,44);power.onPowerChanged=[this]{repaint();};
        addFader("bendVco","VCO",128,496,35,96,true);addFader("bendVcf","VCF",164,496,35,96,true);
        addFader("portamento","PORTAMENTO",256,496,43,112,true);
        // The original has continuous shared rules between adjacent tracks,
        // with numbers at the outside edges of each group only.
        const auto sharedScale=[this](std::initializer_list<const char*> ids) {
            std::vector<juce::Slider*> group;
            for(const auto* id:ids)for(auto& c:controls)
                if(c->slider&&c->slider->getComponentID()==id)group.push_back(c->slider.get());
            for(size_t i=0;i<group.size();++i) {
                auto& slider=*group[i];auto& p=slider.getProperties();
                p.set("numbersLeft",i==0);p.set("numbersRight",i+1==group.size());
                if(i>0)p.set("tickLeftSpan",float(slider.getBounds().getCentreX()-group[i-1]->getBounds().getCentreX())*.5f);
                if(i+1<group.size())p.set("tickRightSpan",float(group[i+1]->getBounds().getCentreX()-slider.getBounds().getCentreX())*.5f);
            }
        };
        sharedScale({"lfoRate","lfoDelay"});sharedScale({"vcoLevel","subLevel","extLevel"});
        sharedScale({"cutoff","resonance"});sharedScale({"vcfEnv","vcfMod"});
        sharedScale({"attack","decay","sustain","release"});sharedScale({"bendVco","bendVcf"});
        addAndMakeVisible(bender);bender.setBounds(95,headerH+638,179,48);bender.setComponentID("bender");
        bender.setRange(-1,1,.001);
        lastBendSerial=processor.displayedPitchBendSerial.load(std::memory_order_acquire);
        bender.setValue(processor.displayedPitchBend.load(std::memory_order_relaxed),juce::dontSendNotification);
        bender.setSliderStyle(juce::Slider::LinearHorizontal);
        bender.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);bender.setScrollWheelEnabled(false);bender.setWantsKeyboardFocus(false);
        bender.setTooltip("Spring-return bender. VCO / VCF sliders set the depth.");
        bender.setTitle("Pitch bender");bender.setPopupDisplayEnabled(true,true,this);
        bender.textFromValueFunction=[](double v){return (v>0?juce::String("+"):juce::String())+juce::String(v*100,1)+"%";};
        bender.onValueChange=[this]{bendReturnStarted=0;bendActive=std::abs(bender.getValue())>.0001;processor.queuePitchBend(float(bender.getValue()));};
        bender.onDragStart=[this]{benderDragging=true;bendReturnStarted=0;};
        bender.onDragEnd=[this]{
            benderDragging=false;bendActive=false;
            lastBendSerial=processor.displayedPitchBendSerial.load(std::memory_order_acquire);
            bendReturnFrom=bender.getValue();
            const bool modern=processor.state.getRawParameterValue("modern")->load()>=.5f;
            bendReturnStarted=modern?0:juce::Time::getMillisecondCounterHiRes();
            if(modern)bender.setValue(0,juce::dontSendNotification);
            processor.queuePitchBendReturn();
        };
        addSwitch("modern",25,designH-headerH-44,131,42,{0,1},{"AUTHENTIC","MODERN"},"Authentic ignores velocity, aftertouch, tempo sync and input trim.");
        // Keep a consistent 52-unit gap between bottom-row control groups.
        addUtility("age","CONDITION / AGE",290,170);
        controls.back()->slider->setRange(0,1,.25);
        controls.back()->slider->onDragStart=[this]{
            look.conditionDragging=true;
            // Only hide hardware when a complete replacement can be drawn.
            // Automation may keep the first preview bank from warming at all.
            if(agePreviews.front().isValid()&&agePreviews.back().isValid()) {
                for(auto* c:getChildren())if(c->isVisible()&&c->getY()>=headerH&&c->getBottom()<=headerH+visibleBodyH) {
                    hiddenForCondition.emplace_back(c);c->setVisible(false);
                }
            }
        };
        controls.back()->slider->onDragEnd=[this]{
            look.conditionDragging=false;conditionBackdrop={};previousCondition={};
            look.age=appearanceAge(processor.state.getRawParameterValue("age")->load());
            for(auto& c:hiddenForCondition)if(c!=nullptr)c->setVisible(true);
            hiddenForCondition.clear();repaint();keyboard.repaint();
        };
        addUtility("velocity","VELOCITY",495,136);addUtility("aftertouch","AFTERTOUCH",683,136);
        addSwitch("lfoSync",831,designH-headerH-44,52,42,{0,1},{"FREE","SYNC"},"Modern mode: SYNC makes RATE select straight, dotted and triplet grid divisions.");
        addUtility("extGain","INPUT TRIM",1034,150);
        addAndMakeVisible(descriptions);descriptions.setComponentID("descriptions");
        descriptions.setButtonText("DESCRIPTION");descriptions.setClickingTogglesState(true);
        descriptions.setBounds(1161,designH-35,139,24);
        descriptions.setColour(juce::TextButton::buttonOnColourId,amber);
        descriptions.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff151a1d));
        descriptions.onClick=[this]{descriptions.setButtonText(descriptions.getToggleState()?"DESCRIPTION ON":"DESCRIPTION");};
        tooltip=std::make_unique<DescriptionTips>(this,descriptions);
        addAndMakeVisible(help);help.setButtonText("EXT AUDIO");help.setBounds(1355,designH-35,120,24);help.setComponentID("externalAudioHelp");
        help.onClick=[this] {
            externalHelp.close();
            externalHelp=juce::AlertWindow::showScopedAsync(
                juce::MessageBoxOptions{}.withIconType(juce::MessageBoxIconType::InfoIcon)
                    .withTitle("External audio + envelope follower")
                    .withMessage(kb09_gui::externalAudioMessage)
                    .withButton("OK").withAssociatedComponent(this),[](int){});
        };
        // Shift the physical controls and their interaction areas together.
        for(auto& c:controls)for(auto* child:{static_cast<juce::Component*>(c->slider.get()),
                static_cast<juce::Component*>(c->selector.get()),static_cast<juce::Component*>(c->label.get())}) {
            if(child==nullptr)continue;
            if(child->getY()>=headerH&&child->getBottom()<headerH+360)child->setTopLeftPosition(child->getX()-13,child->getY());
            else if(child->getY()>=headerH+360&&child->getBottom()<headerH+708)child->setTopLeftPosition(child->getX()+8,child->getY());
        }
        rateLamp.setTopLeftPosition(rateLamp.getX()-13,rateLamp.getY());
        power.setTopLeftPosition(power.getX()+8,power.getY());
        bender.setTopLeftPosition(bender.getX()+8,bender.getY());
        for(auto* child:{static_cast<juce::Component*>(&small),static_cast<juce::Component*>(&medium),
                static_cast<juce::Component*>(&large),static_cast<juce::Component*>(&maximise),static_cast<juce::Component*>(&help)})
            child->setTopLeftPosition(child->getX()-17,child->getY());
        updateModern();
        // Have one complete fallback before the editor can be displayed or
        // dragged. Until idle work builds all five ages, share this snapshot
        // across the bank so a first gesture never renders a bare chassis or
        // forces detailed hardware painting onto every mouse event.
        renderingCondition=true;look.conditionDragging=true;look.renderingPreview=true;
        const auto initialPreview=createComponentSnapshot({0,headerH,designW,visibleBodyH},true,900.0f/designW,juce::SoftwareImageType());
        renderingCondition=false;look.conditionDragging=false;look.renderingPreview=false;
        agePreviews.fill(initialPreview);
        bufferUnchangedHardware();
        startTimerHz(60);
    }
    ~Panel() override {
        externalHelp.close();stopTimer();releaseComputerNotes();removeKeyListener(this);sizing.onChange=nullptr;if(bendActive||benderDragging)processor.queuePitchBend(0);setLookAndFeel(nullptr);
    }
    void paint(juce::Graphics& g) override {
        if(!renderingCondition&&resizeBackdrop.isValid()&&resizeCacheUntil>0){g.drawImage(resizeBackdrop,getLocalBounds().toFloat());return;}
        if(look.conditionDragging&&!renderingCondition&&agePreviews.front().isValid()&&agePreviews.back().isValid()) {
            const int stage=kb09_gui::appearanceStage(processor.state.getRawParameterValue("age")->load());
            const juce::Rectangle<float> body(0,float(headerH),float(designW),float(visibleBodyH));
            g.setOpacity(1);g.drawImage(agePreviews[size_t(stage)],body);
            paintFooter(g);return;
        }
        if(renderingCondition){paintChassis(g);return;}
        // Hardware movement only exposes the same stationary panel beneath it.
        // Rasterise that material once at the actual display resolution, rather
        // than rebuilding grain, corrosion and photographic patches per repaint.
        const float age=appearanceAge(processor.state.getRawParameterValue("age")->load());
        const bool powered=processor.isPowerEnabled();
        const float scale=juce::jlimit(1.0f,4.0f,g.getInternalContext().getPhysicalPixelScaleFactor());
        if(!staticPanel.isValid()||staticPanelAge!=age||staticPanelPower!=powered||std::abs(staticPanelScale-scale)>.01f) {
            staticPanel=juce::Image(juce::Image::ARGB,juce::roundToInt(designW*scale),juce::roundToInt(designH*scale),true,juce::SoftwareImageType());
            juce::Graphics raster(staticPanel);raster.addTransform(juce::AffineTransform::scale(scale));
            paintChassis(raster);staticPanelAge=age;staticPanelPower=powered;staticPanelScale=scale;
            getProperties().set("chassisRasterCount",int(getProperties().getWithDefault("chassisRasterCount",0))+1);
        }
        g.drawImage(staticPanel,getLocalBounds().toFloat());
    }
    void paintChassis(juce::Graphics& g) {
        look.age=previewAge>=0?previewAge:appearanceAge(processor.state.getRawParameterValue("age")->load());
        const float age=look.age;
        const auto ink=agedInk(age);
        const auto& art=artwork();g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        {
            juce::Graphics::ScopedSaveState save(g);
            juce::Path outline;outline.addRoundedRectangle(0,float(headerH),float(designW),float(visibleBodyH),14);
            g.reduceClipRegion(outline);
            // Crop the empty photo border; the rounded chassis sits against the editor's dark surround.
            const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
                juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),
                juce::roundToInt(art.chassis.getHeight()*873.0/887));
            g.drawImage(art.chassis,0,headerH,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            if(age>0) {
                // Registered photographic rust sites appear and spread. Only metal ages;
                // rust does not spill onto the molded side cheeks or empty keybed.
                juce::Path metal;
                metal.addRectangle(55.0f,float(headerH),1389.0f,365.0f);
                metal.addRectangle(65.0f,float(headerH+366),257.0f,335.0f);
                g.reduceClipRegion(metal);g.setOpacity(1);
                g.drawImage(art.panelWear[size_t(kb09_gui::appearanceStage(age))],0,headerH,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            }
        }
        // Rebuild the widened performance bay from unoccupied photographic
        // paint, rather than stretching baked screws or leaving the old seam.
        {
            juce::Graphics::ScopedSaveState bay(g);
            g.addTransform(juce::AffineTransform::translation(0,float(headerH)));
            const juce::Rectangle<float> outer(55,364,273,344),face(55,364,273,344);
            g.setColour(juce::Colour(0xff030505));g.fillRect(outer.expanded(1,0));
            // Painted bay extends behind the outline-to-key margin; no grey rim.
            g.setColour(juce::Colour(0xff101413));g.fillRoundedRectangle(outer,3);
            juce::Path clip;clip.addRoundedRectangle(face,3);g.reduceClipRegion(clip);
            g.drawImage(art.chassis,face.getX(),face.getY(),face.getWidth(),face.getHeight(),105,530,158,220);
            g.setOpacity(1);
            g.drawImage(art.panelWear[size_t(kb09_gui::appearanceStage(age))],face.getX(),face.getY(),face.getWidth(),face.getHeight(),105,530,158,220);
            g.setOpacity(1);
        }
        for(int side=0;side<2;++side)kb09_gui::cheekMaterial().paint(g,side,age,float(headerH),float(designW));
        caseStains(g,age);
        paintPanelDust(g,age);
        // The narrow rail below the keys has a fine pebble finish. It is a
        // different material from the smooth painted main panel.
        {
            juce::Graphics::ScopedSaveState save(g);
            // End at the visible outer edge of the last ivory key.
            const auto rail=juce::Rectangle<float>(65.0f,float(headerH+708),1370.0f,24.0f)
                .withRight(float(keyboard.getRight())-.65f);
            // Unlit case below the rail must not read as a tall front face.
            g.setColour(juce::Colour(0xff070909));
            g.fillRect(59.0f,rail.getY(),1365.0f,float(headerH+visibleBodyH)-rail.getY());
            g.reduceClipRegion(rail.toNearestInt());
            g.drawImage(art.rail,rail.getX(),rail.getY(),rail.getWidth(),rail.getHeight(),0,0,art.rail.getWidth(),70);
            g.setColour(juce::Colours::black.withAlpha(.27f+age*.08f));g.fillRect(rail);
            g.setOpacity(std::pow(age,1.25f));g.drawImage(railDust(),rail);g.setOpacity(1);
            // A shallow key-front shadow meets the textured surface without
            // the old gap left by the exaggerated vertical key faces.
            g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.42f),0,rail.getY(),juce::Colours::transparentBlack,0,rail.getY()+3,false));
            g.fillRect(rail.withHeight(3));
            juce::ColourGradient fold(juce::Colours::transparentWhite,0,rail.getBottom()-1.7f,
                juce::Colour(0xffbfc2b9).withAlpha(.25f-age*.05f),0,rail.getBottom()-.3f,false);
            fold.addColour(.72,juce::Colour(0xffbfc2b9).withAlpha(.08f));
            g.setGradientFill(fold);g.fillRect(rail.withTop(rail.getBottom()-1.7f));
        }
        {
            juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::translation(0,float(headerH)));
            {juce::Graphics::ScopedSaveState topPrint(g);g.addTransform(juce::AffineTransform::translation(-13,0));
            module(g,107,141,"MODULATOR",age);module(g,252,365,"VCO",age);module(g,621,72,"SUB OSC",age);
            module(g,697,146,"AUDIO MIXER",age);module(g,847,251,"VCF",age);module(g,1102,64,"VCA",age);module(g,1170,221,"ENV",age);
            caption(g,juce::String::fromUTF8("UNOFFICIAL PERSONAL PROJECT \xe2\x80\x94 NOT AFFILIATED WITH ROLAND CORPORATION \xe2\x80\x94 NOT FOR SALE"),
                    {107,24,1284,15},10.0f,juce::Justification::centred,true,ink);
            g.setColour(ink);g.drawLine(135,102,160,102,.85f);
            caption(g,"RATE",{163.5f,96,29,12},8.4f,juce::Justification::centred,true,ink);
            panelGlyph(g,PanelGlyph::sine,{193,99,12,7},ink,.85f);
            caption(g,"DELAY\nTIME",{206,92,29,22},7.6f,juce::Justification::centred,true,ink);
            caption(g,"RANGE",{319,96,74,12},9,juce::Justification::centred,true,ink);
            caption(g,"WAVEFORM",{414,96,84,12},9,juce::Justification::centred,true,ink);
            caption(g,"TUNE",{315,75.5f,30,15},8.4f,juce::Justification::centred,true,ink);
            arcLabels(g,{356,163},{{"32'",-1.134464f},{"16'",-.567232f},{"8'",0.0f},{"4'",.567232f},{"2'",1.134464f}},40,age);
            arcLabels(g,{456,163},{{"NOISE",-1.047198f},{"SAW",-.349066f},{"SQR",.349066f},{"PULSE",1.047198f}},40,age);
            caption(g,"PULSE WIDTH MOD",{514,96,83,12},7.6f,juce::Justification::centred,true,ink);
            const float pwmCaptionLeft=555.5f-juce::GlyphArrangement::getStringWidth(font(7.6f,true),"PULSE WIDTH MOD")*.5f;
            juce::Path pwmLink;pwmLink.startNewSubPath(499,131);pwmLink.lineTo(499,106);pwmLink.quadraticTo(499,102,503,102);pwmLink.lineTo(pwmCaptionLeft-.8f,102);
            g.setColour(ink);g.strokePath(pwmLink,juce::PathStrokeType(.9f));
            juce::Path vcfLink;vcfLink.startNewSubPath(999,116);vcfLink.lineTo(999,106);vcfLink.quadraticTo(999,103,1002,103);vcfLink.lineTo(1027,103);vcfLink.quadraticTo(1030,103,1030,106);vcfLink.lineTo(1030,115);
            g.strokePath(vcfLink,juce::PathStrokeType(.85f));
            
            }
            // Original user marks: split into manufacturer on right and instrument on left.
            const float printWear=age*age;
            for(int i=0;i<2;++i) {
                g.setColour(ink.withAlpha(i==0?1-printWear:printWear));
                const auto& printed=i==0?art.marks:art.marksAged;
                g.drawImage(printed,94,260,410,52,846,308,1253,158,true);
                g.drawImage(printed,1143,258,222,51,85,308,722,158,true);
            }
            g.setColour(ink.withAlpha(.94f));g.fillRect(60.0f,313.0f,1364.0f,3.6f);
            // A shallow raised lip catches diffuse light below the printed
            // stripe. It fades into the same continuous metal face, without
            // reinstating the old hard divider or crossing the keyboard edge.
            juce::ColourGradient raisedLip(juce::Colours::transparentWhite,0,317,
                juce::Colours::transparentWhite,0,364,false);
            raisedLip.addColour(.23,juce::Colour(0xffd0d0cc).withAlpha(.09f-.02f*age));
            raisedLip.addColour(.47,juce::Colour(0xffbfc1bd).withAlpha(.065f-.01f*age));
            g.setGradientFill(raisedLip);g.fillRect(60.0f,317.0f,1364.0f,47.0f);
            {juce::Graphics::ScopedSaveState bayPrint(g);g.addTransform(juce::AffineTransform::translation(8,0));
            g.setColour(ink.withAlpha(.82f));
            juce::Path bayRule;bayRule.startNewSubPath(69,370);bayRule.lineTo(69,702);
            bayRule.quadraticTo(69,705,72,705);bayRule.lineTo(299,705);
            bayRule.quadraticTo(302,705,302,702);bayRule.lineTo(302,370);
            g.strokePath(bayRule,juce::PathStrokeType(1));
            g.drawLine(210,370,210,484,.85f);
            caption(g,"VOLUME",{96,376,88,13},10,juce::Justification::centred,true,ink);
            for(int i=0;i<=10;++i) {
                const float a=juce::MathConstants<float>::pi*(1.25f+1.5f*float(i)/10);
                const juce::Point<float> c(140,442),direction(std::sin(a),-std::cos(a));
                g.setColour(ink);g.drawLine({c+direction*41,c+direction*47},1.0f);
                if(i==0||i==10) {const auto p=c+direction*53;caption(g,juce::String(i),{p.x-10,p.y-5,20,10},7.5f,juce::Justification::centred,true,ink);}
            }
            {juce::Graphics::ScopedSaveState powerPrint(g);g.addTransform(juce::AffineTransform::translation(51,0));kb09_gui::drawPowerLegend(g,ink,age);}
            g.setColour(juce::Colour(0xff080b0e));g.fillEllipse(251.5f,475,9,9);
            g.setColour(juce::Colour(processor.isPowerEnabled()?0xffe54c35:0xff491c16));g.fillEllipse(253.5f,477,5,5);
            if(processor.isPowerEnabled()) {g.setColour(juce::Colour(0xffffaa6f).withAlpha(.65f));g.fillEllipse(254.0f,477.5f,1.8f,1.8f);}
            g.setColour(ink.withAlpha(.76f));g.drawRoundedRectangle(214.0f,489.0f,81.0f,134.0f,3.0f,.8f);
            juce::Path bendOutline;bendOutline.startNewSubPath(75,489);bendOutline.lineTo(209,489);
            bendOutline.lineTo(209,627);bendOutline.lineTo(295,627);bendOutline.lineTo(295,699);
            bendOutline.lineTo(75,699);bendOutline.closeSubPath();
            g.strokePath(bendOutline.createPathWithRoundedCorners(2.5f),juce::PathStrokeType(.8f));
            juce::Path bendLink;bendLink.startNewSubPath(128,583);bendLink.lineTo(128,608);bendLink.quadraticTo(128,611,131,611);bendLink.lineTo(182,611);bendLink.quadraticTo(185,611,185,614);bendLink.lineTo(185,624);
            bendLink.startNewSubPath(164,583);bendLink.lineTo(164,611);
            g.setColour(ink);g.strokePath(bendLink,juce::PathStrokeType(.9f));
            caption(g,"BENDER",{133,628,103,13},10,juce::Justification::centred,true,ink);
            g.setColour(ink);g.drawLine(132,682,178,682,.9f);g.drawLine(191,682,237,682,.9f);
            juce::Path arrows;arrows.startNewSubPath(127,682);arrows.lineTo(133,679.5f);arrows.lineTo(133,684.5f);arrows.closeSubPath();arrows.startNewSubPath(242,682);arrows.lineTo(236,679.5f);arrows.lineTo(236,684.5f);arrows.closeSubPath();g.fillPath(arrows);
            caption(g,"-",{114,676,12,12},10,juce::Justification::centred,true,ink);
            caption(g,"0",{178,676,13,12},8,juce::Justification::centred,true,ink);
            caption(g,"+",{243,676,12,12},10,juce::Justification::centred,true,ink);
            kb09_gui::drawPowerScrew(g,{98,642},age,false,1);
            kb09_gui::drawPowerScrew(g,{272,642},age,true,1);
            }
            uint32_t screwSeed=5011;
            for(const auto p:{juce::Point<float>(88,37),{1686,37},
                              {88,328},{1686,328},{83,399},{1692,399}}) {
                screwOxidation(g,{(p.x-2)*float(designW)/1770,(p.y-7)*float(bodyH)/873+(p.y>790?-40.0f:0.0f)},age,screwSeed);
                screwSeed+=7717;
            }
            kb09_gui::drawPowerScrew(g,{264,401},age,false);
            kb09_gui::drawPowerScrew(g,{264,449},age,true);
        }
        for(auto& c:controls)if(c->slider&&c->slider->getSliderStyle()==juce::Slider::LinearVertical)
            look.drawFaderScale(g,*c->slider);
        paintFooter(g);
    }
private:
    void paintFooter(juce::Graphics& g) {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff222729),0,float(headerH+visibleBodyH),
                                            juce::Colour(0xff121618),0,float(designH),false));
        g.fillRect(0,headerH+visibleBodyH,designW,designH-headerH-visibleBodyH);
        g.setColour(juce::Colour(0xff58605f));g.drawLine(0,float(headerH+visibleBodyH),float(designW),float(headerH+visibleBodyH),.65f);
        for(const auto& c:controls)if(c->slider&&c->slider->getSliderStyle()==juce::Slider::LinearHorizontal) {
            const auto id=c->slider->getComponentID();
            if(id!="age"&&id!="velocity"&&id!="aftertouch"&&id!="extGain")continue;
            const auto r=look.getSliderLayout(*c->slider).sliderBounds.translated(c->slider->getX(),c->slider->getY());
            const int steps=id=="age"?4:10;
            const auto colour=ink.withAlpha(c->slider->isEnabled()?.83f:.28f);
            for(int i=0;i<=steps;++i) {
                const float x=r.getX()+float(r.getWidth())*i/steps;
                const bool major=id=="age"||i%5==0;
                g.setColour(colour);g.drawLine(x,802,x,major?807.f:805.f,major?1.05f:.65f);
                if(id!="age"&&i%5==0)caption(g,juce::String(i),{x-7,807,14,8},6.f,juce::Justification::centred,false,colour);
            }
        }
    }
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    struct Control {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<SlideSwitch> selector;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<SliderAttachment> attachment;
    };
    SH09Processor& processor;sh09::WindowSizing& sizing;sh09::PresetFiles presetFiles;HardwareLook look;
    Keyboard keyboard;Header header;MaximiseButton maximise;PowerButton power;
    juce::ComboBox presets;juce::TextButton previous,next,small,medium,large,help,savePreset,loadPreset,descriptions,audioMidi,computerKeyboard,updates;bool updateOpened=false;
    std::unique_ptr<juce::TooltipWindow> tooltip;juce::Image conditionBackdrop,previousCondition,resizeBackdrop;std::array<juce::Image,5> agePreviews,pendingAgePreviews;std::vector<juce::Component::SafePointer<juce::Component>> hiddenForCondition;uint64_t previewParameters=0;double previewIdleAfter=0;float previewAge=-1;bool renderingCondition=false;double resizeCacheUntil=0;
    juce::Image staticPanel;float staticPanelAge=-1,staticPanelScale=0;bool staticPanelPower=false;
    double performanceIdleAfter=0;unsigned lastPerformanceMidiSerial=0;
    FineSlider bender;juce::ScopedMessageBox externalHelp;Meter meter;RateLamp rateLamp;
    unsigned lastBendSerial=0;bool benderDragging=false;double bendReturnStarted=0,bendReturnFrom=0;
    std::vector<std::unique_ptr<Control>> controls;
    std::array<int,128> lastKeys{};float lastAge=-1;int lastLabelRange=-1;bool lastModern=false,bendActive=false,userPreset=false,lastHoldMode=false;int lastProgram=-1,lastDisplayedNote=-1,lastDisplayedKeyboardNote=-1,lastHeldNote=-1;unsigned lastProgramRevision=0;double lastConditionPaint=0;
    std::array<bool,13> computerHeld{};
    int computerOctave=4;
    static constexpr const char* computerKeys="AWSEDFTGYHUJK";
    void updateComputerLabel() {computerKeyboard.setButtonText(computerKeyboard.getToggleState()?"KEYBOARD C"+juce::String(computerOctave):"KEYBOARD");}
    void releaseComputerNotes() {
        for(size_t i=0;i<computerHeld.size();++i)if(computerHeld[i]) {
            keyboard.setComputerNote((computerOctave+1)*12+int(i),false);computerHeld[i]=false;
        }
    }
    bool computerReady() const {
        if(!computerKeyboard.getToggleState()||!hasKeyboardFocus(true)||!isShowing())return false;
        for(auto* c=juce::Component::getCurrentlyFocusedComponent();c&&c!=this;c=c->getParentComponent())
            if(dynamic_cast<juce::TextEditor*>(c)||dynamic_cast<juce::ComboBox*>(c))return false;
        return !juce::ModifierKeys::getCurrentModifiersRealtime().isAnyModifierKeyDown();
    }
    bool keyStateChanged(bool,juce::Component*) override {
        if(!computerReady()){releaseComputerNotes();return false;}
        bool consumed=false;
        for(size_t i=0;i<computerHeld.size();++i) {
            const bool down=juce::KeyPress::isKeyCurrentlyDown(computerKeys[i]);
            consumed|=down||computerHeld[i];
            if(down!=computerHeld[i]){computerHeld[i]=down;keyboard.setComputerNote((computerOctave+1)*12+int(i),down);}
        }
        return consumed;
    }
    bool keyPressed(const juce::KeyPress& key,juce::Component*) override {
        if(!computerReady())return false;
        const auto c=juce::CharacterFunctions::toUpperCase(key.getTextCharacter());
        if(c=='Z'||c=='X') {releaseComputerNotes();computerOctave=juce::jlimit(1,6,computerOctave+(c=='X'?1:-1));updateComputerLabel();return true;}
        return juce::String(computerKeys).containsChar(c);
    }
    void timerCallback() override {
        const auto bendSerial=processor.displayedPitchBendSerial.load(std::memory_order_acquire);
        if(!benderDragging&&bendSerial!=lastBendSerial) {
            lastBendSerial=bendSerial;bendActive=false;
            const auto bend=processor.displayedPitchBend.load(std::memory_order_relaxed);
            // The audio spring and its visual animation share a duration. External
            // MIDI takes over immediately; display changes never feed back to audio.
            if(bendReturnStarted==0||(!processor.displayedPitchBendReturning.load(std::memory_order_relaxed)&&std::abs(bend)>.0001f)) {
                bendReturnStarted=0;bender.setValue(bend,juce::dontSendNotification);
            }
        }
        if(!benderDragging&&bendReturnStarted>0) {
            if(processor.state.getRawParameterValue("modern")->load()>=.5f) {
                bendReturnStarted=0;bender.setValue(0,juce::dontSendNotification);
            } else {
            const auto progress=juce::jlimit(0.0,1.0,(juce::Time::getMillisecondCounterHiRes()-bendReturnStarted)/(SH09Processor::benderReturnSeconds*1000));
            bender.setValue(bendReturnFrom*(1-progress)*(1-progress),juce::dontSendNotification);
            if(progress>=1)bendReturnStarted=0;
            }
        }
        if(processor.wrapperType!=juce::AudioProcessor::wrapperType_Undefined&&isShowing()&&getPeer()){if(!updateOpened){updateOpened=true;kb09::Updates::instance().opened(this);}kb09::Updates::instance().deliver(this);}
        if(!computerReady())releaseComputerNotes();
        else for(size_t i=0;i<computerHeld.size();++i)if(computerHeld[i]&&!juce::KeyPress::isKeyCurrentlyDown(computerKeys[i])) {
            computerHeld[i]=false;keyboard.setComputerNote((computerOctave+1)*12+int(i),false);
        }
        const auto programRevision=processor.getProgramRevision();
        const int program=processor.getCurrentProgram();
        if(!userPreset||program!=lastProgram||programRevision!=lastProgramRevision){presets.setSelectedId(program+1,juce::dontSendNotification);userPreset=false;}
        lastProgram=program;lastProgramRevision=programRevision;
        const double stamp=processor.displayedModulatorTime.load(std::memory_order_acquire);
        const double elapsed=juce::jmax(0.0,(juce::Time::getMillisecondCounterHiRes()-stamp)*.001);
        const float hz=processor.displayedModulatorFrequency.load(std::memory_order_relaxed);
        const double phase=processor.displayedModulatorPhase.load(std::memory_order_relaxed)+elapsed*hz;
        const float strobe=float(phase-std::floor(phase)<.5);
        const float blend=juce::jlimit(0.f,1.f,(hz-12)/4);
        const float averaged=.48f+.52f*juce::jlimit(0.0f,1.0f,(hz-16)/9);
        const float lamp=strobe+(averaged-strobe)*blend;
        rateLamp.setLevel(processor.isPowerEnabled()&&hz>0?lamp:0);
        bool keysChanged=false;
        for(size_t i=0;i<lastKeys.size();++i){const int n=processor.keyDisplay[i].load(std::memory_order_relaxed);if(lastKeys[i]!=n){lastKeys[i]=n;keyboard.repaintNote(int(i));keysChanged=true;}}
        const int displayedNote=processor.displayedNote.load(std::memory_order_relaxed);
        if(displayedNote!=lastDisplayedNote){keyboard.repaintNote(lastDisplayedNote);keyboard.repaintNote(displayedNote);lastDisplayedNote=displayedNote;keysChanged=true;}
        const int displayedKeyboardNote=processor.displayedKeyboardNote.load(std::memory_order_relaxed);
        if(displayedKeyboardNote!=lastDisplayedKeyboardNote){keyboard.repaintNote(lastDisplayedKeyboardNote);keyboard.repaintNote(displayedKeyboardNote);lastDisplayedKeyboardNote=displayedKeyboardNote;keysChanged=true;}
        const int heldNote=processor.displayedHeldNote.load(std::memory_order_relaxed);
        if(heldNote!=lastHeldNote){keyboard.repaintNote(lastHeldNote);keyboard.repaintNote(heldNote);lastHeldNote=heldNote;keysChanged=true;}
        const bool holdMode=processor.state.getRawParameterValue("vcaMode")->load()<.5f;
        if(holdMode!=lastHoldMode){lastHoldMode=holdMode;keyboard.repaint();}
        const float age=appearanceAge(processor.state.getRawParameterValue("age")->load());
        // Coalesce full-instrument wear redraws while the AGE thumb remains live.
        const auto now=juce::Time::getMillisecondCounterHiRes();
        if(age!=lastAge && (!look.conditionDragging||now-lastConditionPaint>=110)){
            lastConditionPaint=now;lastAge=age;look.age=age;
            for(auto& c:controls)if(c->label&&bool(c->label->getProperties().getWithDefault("hardwarePrint",false)))
                c->label->setColour(juce::Label::textColourId,agedInk(age));
            bender.repaint();power.repaint();
            for(auto& c:controls){if(c->slider)c->slider->repaint();if(c->selector)c->selector->repaint();}
            repaint();if(!look.conditionDragging)keyboard.repaint();
        }
        // Only idle work builds material frames. A drag performs two image
        // blits and a tiny footer repaint, never a full keyboard/knob render.
        const auto activity=processor.midiActivitySerial.load(std::memory_order_relaxed);
        if(activity!=lastPerformanceMidiSerial||keysChanged||benderDragging||displayedNote>=0||std::any_of(lastKeys.begin(),lastKeys.end(),[](int n){return n>0;}))
            performanceIdleAfter=now+750;
        lastPerformanceMidiSerial=activity;
        if(!look.conditionDragging && ((isShowing()&&getPeer()!=nullptr&&now>=performanceIdleAfter)||bool(getProperties().getWithDefault("renderConditionPreviews",false)))) {
            uint64_t stateHash=1469598103934665603ull;
            for(const auto& spec:sh09::specs)if(juce::String(spec.id)!="age") {
                const float v=processor.state.getRawParameterValue(spec.id)->load();
                stateHash=(stateHash^uint64_t(juce::roundToInt(v*100000)))*1099511628211ull;
            }
            // Build a separate bank. Keep the last complete instrument visible
            // while automation invalidates or interrupts pending material work.
            if(stateHash!=previewParameters){previewParameters=stateHash;previewIdleAfter=now+250;getProperties().set("conditionPreviewReady",false);pendingAgePreviews={};}
            if(!bool(getProperties().getWithDefault("conditionPreviewReady",false))&&now>=previewIdleAfter&&!juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())for(size_t i=0;i<pendingAgePreviews.size();++i)if(!pendingAgePreviews[i].isValid()) {
                const float originalAge=look.age;
                previewAge=float(i)/4;renderingCondition=true;look.age=previewAge;look.renderingPreview=true;
                look.conditionDragging=true; // Low-cost material rasterisation while caching.
                bufferInSoftware(keyboard,false);bufferInSoftware(bender,false);bufferInSoftware(power,false);
                for(auto& c:controls){if(c->slider)bufferInSoftware(*c->slider,false);if(c->selector)bufferInSoftware(*c->selector,false);}
                pendingAgePreviews[i]=createComponentSnapshot({0,headerH,designW,visibleBodyH},true,900.0f/designW,juce::SoftwareImageType());
                look.conditionDragging=false;look.renderingPreview=false;
                previewAge=-1;renderingCondition=false;look.age=originalAge;
                bufferUnchangedHardware();
                if(std::all_of(pendingAgePreviews.begin(),pendingAgePreviews.end(),[](const auto& image){return image.isValid();})) {
                    agePreviews=std::move(pendingAgePreviews);pendingAgePreviews={};
                    getProperties().set("conditionPreviewReady",true);
                }
                break;
            }
        }
        if(resizeCacheUntil>0&&now>=resizeCacheUntil) {
            resizeCacheUntil=0;resizeBackdrop={};bufferUnchangedHardware();repaint();keyboard.repaint();
        }
        const bool modern=processor.state.getRawParameterValue("modern")->load()>.5f;
        if(modern!=lastModern)updateModern();
        const int range=juce::roundToInt(processor.state.getRawParameterValue("range")->load());
        if(range!=lastLabelRange){lastLabelRange=range;keyboard.repaint();}
    }
    void updateModern() {
        keyboard.repaint();
        lastModern=processor.state.getRawParameterValue("modern")->load()>.5f;
        for(auto& c:controls) {
            juce::Component* component=c->slider?static_cast<juce::Component*>(c->slider.get()):static_cast<juce::Component*>(c->selector.get());
            const auto id=component->getComponentID();
            if(id=="velocity"||id=="aftertouch"||id=="lfoSync"||id=="extGain") {
                component->setEnabled(lastModern);component->setAlpha(lastModern?1.0f:.4f);
                if(c->label)c->label->setAlpha(lastModern?1.0f:.4f);
            }
        }
    }
    void addSwitch(const char* id,int x,int y,int w,int h,std::vector<int> order,
                   juce::StringArray labels,const juce::String& tip,bool left=false) {
        auto c=std::make_unique<Control>();auto* p=processor.state.getParameter(id);jassert(p);
        c->selector=std::make_unique<SlideSwitch>(*p,std::move(order),std::move(labels),left);
        c->selector->setComponentID(id);c->selector->setTitle(p->getName(100));c->selector->setTooltip(tip);
        c->selector->setBounds(x,headerH+y,w,h);addAndMakeVisible(*c->selector);controls.push_back(std::move(c));
    }
    void addSlider(const char* id,const char* name,int x,int y,int w,int h,juce::Slider::SliderStyle style) {
        auto c=std::make_unique<Control>();c->slider=std::make_unique<FineSlider>();auto& slider=*c->slider;
        slider.setComponentID(id);slider.setTitle(name);slider.setBounds(x,headerH+y,w,h);slider.setSliderStyle(style);
        slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);slider.setPopupDisplayEnabled(true,true,this);
        slider.setScrollWheelEnabled(false);slider.setMouseDragSensitivity(190);
        if(juce::String(id)=="age"||juce::String(id)=="velocity"||juce::String(id)=="aftertouch"||juce::String(id)=="extGain")slider.setSliderSnapsToMousePosition(true);
        const float twoPi=juce::MathConstants<float>::twoPi;
        if(juce::String(id)=="range")slider.setRotaryParameters(twoPi-1.134464f,twoPi+1.134464f,true);
        else if(juce::String(id)=="wave")slider.setRotaryParameters(twoPi-1.047198f,twoPi+1.047198f,true);
        else slider.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id) {slider.setTooltip(spec.name);
        if(juce::String(id)=="tune")slider.setTooltip("Tune oscillator pitch: +/-65 cents in Authentic mode; +/-12 semitones (one octave each way) in Modern mode. Centre is concert pitch.");
        if(juce::String(id)=="range")slider.setTooltip("Choose the oscillator octave range.");
        if(juce::String(id)=="wave")slider.setTooltip("Choose noise, saw, square or pulse as the main oscillator source.");
        if(juce::String(id)=="pulseWidth")slider.setTooltip("Set pulse width manually, or the amount of pulse-width modulation.");
        if(juce::String(id)=="vcoLevel")slider.setTooltip("Mix the main oscillator into the filter.");
        if(juce::String(id)=="subLevel")slider.setTooltip("Mix the sub-oscillator into the filter.");
        if(juce::String(id)=="extLevel")slider.setTooltip("Mix incoming external audio into the filter; watch the IN indicator.");
        if(juce::String(id)=="lfoRate")slider.setTooltip("Set modulation speed: Authentic 0.2–25 Hz; Modern FREE 0.05–25 Hz. Modern SYNC chooses a musical division of the host tempo.");
        if(juce::String(id)=="lfoDelay")slider.setTooltip("Delay the onset of modulation after a note starts.");
        if(juce::String(id)=="vcoMod")slider.setTooltip("Set the amount of LFO pitch modulation. MIDI mod wheel (CC1) controls this fader after reaching or crossing its current position. Manual edits re-arm pickup.");
        if(juce::String(id)=="cutoff")slider.setTooltip("Set filter cutoff frequency; lower it to remove upper harmonics.");
        if(juce::String(id)=="resonance")slider.setTooltip("Emphasise the cutoff frequency; high settings make the filter self-oscillate.");
        if(juce::String(id)=="vcfMod")slider.setTooltip("Set the amount of LFO modulation applied to filter cutoff.");
        if(juce::String(id)=="vcfEnv")slider.setTooltip("Set the amount of envelope or external envelope-follower modulation of cutoff.");
        if(juce::String(id)=="attack")slider.setTooltip("Set how quickly the envelope rises after a note starts.");
        if(juce::String(id)=="decay")slider.setTooltip("Set how quickly the envelope falls to sustain.");
        if(juce::String(id)=="sustain")slider.setTooltip("Set the envelope level while a note is held.");
        if(juce::String(id)=="release")slider.setTooltip("Set how slowly the envelope falls after a note ends.");
        if(juce::String(id)=="portamento")slider.setTooltip("Set the time taken to glide between note pitches.");
        if(juce::String(id)=="bendVco")slider.setTooltip("Set how much the spring-return bender changes oscillator pitch.");
        if(juce::String(id)=="bendVcf")slider.setTooltip("Set how much the bender changes filter cutoff.");
        if(juce::String(id)=="volume")slider.setTooltip("Set instrument output volume, from silence to full level.");
        if(juce::String(id)=="age")slider.setTooltip("Choose one of five named condition stages. Click a tick or drag to select; existing preset and host-automation AGE values remain compatible. Selection updates while dragging; release restores full rendering detail.");
        if(juce::String(id)=="velocity")slider.setTooltip("In Modern mode, set how strongly playing velocity affects the sound.");
        if(juce::String(id)=="aftertouch")slider.setTooltip("In Modern mode, channel aftertouch opens the filter. Set the pressure amount here; maximum pressure and amount raise cutoff by up to three octaves.");
        if(juce::String(id)=="extGain")slider.setTooltip("In Modern mode, adjust external input gain before the follower and mixer.");
slider.setDoubleClickReturnValue(true,spec.initial);}
        addAndMakeVisible(slider);c->attachment=std::make_unique<SliderAttachment>(processor.state,id,slider);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id&&spec.choices[0]==0) {
            if(juce::String(id)=="age")slider.textFromValueFunction=[](double v){
                return juce::String(kb09_gui::ageNames[size_t(kb09_gui::appearanceStage(float(v)))]);
            };
            else if(juce::String(id)=="tune")slider.textFromValueFunction=[this](double v){
                const bool modern=processor.state.getRawParameterValue("modern")->load()>.5f;
                const double cents=sh09::oscillatorTuneCents(v,modern);
                return juce::String(modern?cents/100.0:cents,modern?2:1)+(modern?" st":" ct");
            };
            else if(juce::String(id)=="lfoRate")slider.textFromValueFunction=[this](double v){
                return processor.state.getRawParameterValue("modern")->load()>.5f&&processor.state.getRawParameterValue("lfoSync")->load()>.5f
                    ?juce::String(sh09::rateDivision(v).label):juce::String(v*10,2);
            };
            else slider.textFromValueFunction=[](double v){return juce::String(v*10,2);};
        }
        controls.push_back(std::move(c));
    }
    void addLabel(Control& c,const juce::String& text,int x,int y,int w,int h,float size) {
        c.label=std::make_unique<juce::Label>();c.label->setText(text,juce::dontSendNotification);
        c.label->setBounds(x,headerH+y,w,h);c.label->setFont(font(size,true));c.label->setJustificationType(juce::Justification::centred);
        c.label->setBorderSize(juce::BorderSize<int>(0));c.label->setInterceptsMouseClicks(false,false);addAndMakeVisible(*c.label);
        const bool hardware=headerH+y<headerH+visibleBodyH;
        c.label->getProperties().set("hardwarePrint",hardware);
        if(hardware)c.label->setColour(juce::Label::textColourId,agedInk(appearanceAge(processor.state.getRawParameterValue("age")->load())));
    }
    void addFader(const char* id,const char* name,int centre,int y,int w,int h,bool compact=false) {
        addSlider(id,name,centre-w/2,y+20,w,h-11,juce::Slider::LinearVertical);
        auto& c=*controls.back();c.slider->getProperties().set("compact",compact);
        // Printed scale numerals extend past the narrow hit area, without
        // overlapping the neighbouring fader's interaction bounds.
        c.slider->setPaintingIsUnclipped(true);
        addLabel(c,name,centre-w/2-8,y-1,w+16,20,8.4f);
    }
    void addKnob(const char* id,int cx,int cy,int d) {
        addSlider(id,id,cx-d/2,cy-d/2,d,d,juce::Slider::RotaryHorizontalVerticalDrag);
    }
    void addUtility(const char* id,const char* name,int centre,int width) {
        addSlider(id,name,centre-width/2,designH-headerH-34,width,30,juce::Slider::LinearHorizontal);
        addLabel(*controls.back(),name,centre-width/2-8,designH-headerH-46,width+16,18,9);
    }
};
SH09Editor::SH09Editor(SH09Processor& p)
    :AudioProcessorEditor(p),sizing(*this,designW,designH),panel(std::make_unique<Panel>(p,sizing)) {
    setResizable(true,false);addAndMakeVisible(*panel);setSize(designW,designH);sizing.chooseMedium();
}
void SH09Editor::prepareStandaloneWindow() { sizing.prepareStandaloneWindow(); }
void SH09Editor::visibilityChanged() {sizing.peerAttached();}
SH09Editor::~SH09Editor()=default;
void SH09Editor::parentHierarchyChanged() {
#if JUCE_WINDOWS
    // This is our JUCE peer (the VST3 wrapper child or standalone window),
    // never the DAW's native frame. Software rendering avoids graphics-driver
    // shutdown waits when a host repeatedly creates and unloads this editor.
    if(auto* peer=getPeer()) {
        const auto engines=peer->getAvailableRenderingEngines();
        const auto software=engines.indexOf("Software Renderer");
        if(software>=0&&peer->getCurrentRenderingEngine()!=software)peer->setCurrentRenderingEngine(software);
    }
#endif
    sizing.peerAttached();
}
void SH09Editor::paint(juce::Graphics& g) {g.fillAll(juce::Colour(0xff101416));}
void SH09Editor::resized() {
    if(panel) {
        panel->cacheForResize();
        const auto bounds=sizing.contentBounds(getLocalBounds().toFloat());
        const float scale=bounds.getWidth()/float(designW);
        panel->setTransform(juce::AffineTransform::scale(scale).translated(bounds.getX(),bounds.getY()));
        panel->setTopLeftPosition(0,0);
    }
}
