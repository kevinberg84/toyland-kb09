#include "Editor.h"
#include "../Parameters/ControlResponse.h"
#include "KeyboardFeedback.h"
#include "PresetFiles.h"
#include "PowerDetails.h"
#include "ScrewSeatRepair.h"
#include "KnobLighting.h"
#include "ReferenceKnobModel.h"
#include "RateLampClock.h"
#include "PowerLampFade.h"
#include "InsetRedLamp.h"
#include "WindowsRateLamp.h"
#include "PowerButtonTravel.h"
#include "CompactPanelLayout.h"
#include "AgeRasterCache.h"
#include "CheekMaterial.h"
#include "CollarDustGeometry.h"
#include "BenderMaterial.h"
#include "BenderBreakGeometry.h"
#include "BenderReturnMotion.h"
#include "FaderDetails.h"
#include "FaderCapModel.h"
#include "HardwareShadows.h"
#include "RustMaterial.h"
#include "AgeAppearance.h"
#include "UpdateChecker.h"
#include "AudioSettingsDialog.h"
#include "DialogMessages.h"
#include "MidiMapOverlay.h"
#include "KnobGripMotion.h"
#include "../Plugin/Processor.h"
#include "BinaryData.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#include <cmath>
#include <cstdio>
#include <vector>
#include <future>
#include <mutex>
#include <atomic>
#include <array>

namespace {
using kb09_gui::appearanceAge;
constexpr int designW=1483, bodyH=750, visibleBodyH=732, headerH=34, designH=816;
constexpr int topStripReduction=KB09_APPROVED_HARDWARE?kb09_gui::CompactPanelLayout::reduction:0;
constexpr int canvasH=designH-topStripReduction;
// Equal printed-outline margins between the bay face and first key.
constexpr int performanceBayOffsetX=5;
// One distance from each section rule to its border-facing fader axis;
// uniform tick spans and numeral boxes give equal number-column margins.
constexpr int topScaleBorderDistance=35;
// Center the complete 1284-unit section group on the 1483-unit panel.
constexpr float topPanelOffsetX=-7.5f;
constexpr int topPanelBoundsOffsetX=-8;
constexpr float topPanelFractionX=topPanelOffsetX-topPanelBoundsOffsetX;
constexpr int pulseWidthCentre=538,pwmSwitchX=561;
constexpr float pwmCaptionCentre=((pulseWidthCentre+.5f)+(pwmSwitchX+12.f))*.5f;
constexpr int volumeOffsetY=-5, powerOffsetY=-12, modulatorOffsetX=4;
constexpr int modulatorSwitchX=94+modulatorOffsetX,modulatorSwitchWidth=49;
constexpr int volumeKnobLift=KB09_APPROVED_HARDWARE?6:0;
constexpr int powerHardwareLift=5, powerLegendLift=0, powerLedDrop=6;
// Reduce proposal 197 by ten percent about the pair's shared midpoint.
// Diameter, centre spacing and clear gap all scale together.
constexpr float mainKnobScale=KB09_APPROVED_HARDWARE?1.06f*.9f:1.f;
constexpr float rangeKnobCentre=KB09_APPROVED_HARDWARE?355.55f:356.f;
constexpr float waveKnobCentre=KB09_APPROVED_HARDWARE?455.45f:456.f;
constexpr float tuneKnobShift=rangeKnobCentre-356.f;
constexpr float rangeDetentScale=KB09_APPROVED_HARDWARE?.93f:1.f;
constexpr float waveDetentScale=KB09_APPROVED_HARDWARE?.82f:1.f;
constexpr float panelScrewHeadRadius=10.f, performanceScrewRadius=panelScrewHeadRadius*.8f;
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
        const auto envPoint=[&](float x,float y){return point(x,inverted?1.f-y:y);};
        path.startNewSubPath(envPoint(0,.94f));
        path.lineTo(envPoint(.30f,.04f));path.lineTo(envPoint(.41f,.46f));
        path.lineTo(envPoint(.61f,.46f));
        path.cubicTo(envPoint(.69f,.77f),envPoint(.82f,.96f),envPoint(1,1));
    } else if(shape==PanelGlyph::gate) {
        start(0,.9f);line(.275f,.9f);line(.275f,.1f);line(.725f,.1f);line(.725f,.9f);line(1,.9f);
    } else {
        const float end=shape==PanelGlyph::pulse?.27f:.46f;
        start(0,.9f);line(0,.1f);line(end,.1f);line(end,.9f);line(1,.9f);if(shape!=PanelGlyph::gate)line(1,.1f);
    }
    // Equal visible symbol heights, independent of each path's internal margins.
    const auto symbolBounds=path.getBounds();
    if(symbolBounds.getHeight()>0) {
        constexpr float symbolHeight=6.4f;
        path.applyTransform(juce::AffineTransform::translation(0,-symbolBounds.getCentreY())
            .followedBy(juce::AffineTransform::scale(1,symbolHeight/symbolBounds.getHeight()))
            .followedBy(juce::AffineTransform::translation(0,box.getCentreY())));
    }
    g.setColour(colour);g.strokePath(path,juce::PathStrokeType(stroke,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
}
// Optional native-renderer diagnostics; disabled in normal launches.
struct StartupScope {
    const char* name;double start=juce::Time::getMillisecondCounterHiRes();
    ~StartupScope(){if(juce::SystemStats::getEnvironmentVariable("KB09_PROFILE_STARTUP","")=="1")
        std::fprintf(stderr,"STARTUP_MATERIAL %s %.3f ms\n",name,juce::Time::getMillisecondCounterHiRes()-start);}
};
struct LivePaintScope {
    const char* name;double began=juce::Time::getMillisecondCounterHiRes();
    ~LivePaintScope(){if(juce::SystemStats::getEnvironmentVariable("KB09_PROFILE_UI","")=="1")std::fprintf(stderr,"UI %s %.3f\n",name,juce::Time::getMillisecondCounterHiRes()-began);}
};
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
    juce::Image knob=load(BinaryData::knobgripphotov6_png,BinaryData::knobgripphotov6_pngSize)
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
const Artwork& artwork() {static const Artwork a=[] {StartupScope timing{"artwork"};return Artwork();}();return a;}
// Keep component caches in CPU memory too, including offscreen host editors.
// A host's temporary graphics context must not make our caches retain GPU
// resources across plug-in unload. Repaints invalidate only the affected area.
void drawNativeRaster(juce::Graphics& g,const juce::Image& image,float scale,juce::Point<float> origin={}) {
    juce::Graphics::ScopedSaveState native(g);
    // These bitmaps are already rasterised at the destination pixel density.
    // Filtering them again costs a full-frame resample and softens the result.
    g.setImageResamplingQuality(juce::Graphics::lowResamplingQuality);
    g.drawImageTransformed(image,juce::AffineTransform::scale(1.f/scale).translated(origin.x,origin.y));
}
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
        g.setOpacity(owner.getAlpha());drawNativeRaster(g,image,scale);
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
float collarDustRadius(float angle) {
    static const auto edges=[] {
        std::array<float,360> radii{};
        const auto photo=Artwork::load(BinaryData::knobtoprightsquarerimv95_png,BinaryData::knobtoprightsquarerimv95_pngSize).getClippedImage({70,70,1114,1114});
        juce::Image::BitmapData pixels(photo,juce::Image::BitmapData::readOnly);
        for(size_t i=0;i<radii.size();++i) {
            const float a=float(i)*juce::MathConstants<float>::twoPi/float(radii.size());
            float r=.56f;
            for(;r>.3f;r-=.0005f) {
                const int x=juce::jlimit(0,pixels.width-1,int((.497f+r*std::cos(a))*pixels.width));
                const int y=juce::jlimit(0,pixels.height-1,int((.505f+r*std::sin(a))*pixels.height));
                if(pixels.getPixelColour(x,y).getFloatAlpha()>=.85f)break;
            }
            radii[i]=r;
        }
        return radii;
    }();
    return kb09_gui::collarRadiusAt(edges,angle);
}
float collarDustFade(float angle,float distanceFromEdge) {
    // Broad, smooth tails toward the exposed front of the collar. Both the
    // grain density and each grain's opacity vanish, avoiding a cut-off arc.
    const auto smooth=[](float t){t=juce::jlimit(0.f,1.f,t);return t*t*(3.f-2.f*t);};
    return smooth((.98f-std::sin(angle))/.72f)*smooth((8.f-distanceFromEdge)/7.f);
}
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
            float x=0,y=0,grainFade=1.f;
            if(i%4<2) {
                // Narrow, uneven deposits hugging the two collars and the
                // sheltered bridge between them, never a cloud below a knob.
                const float cx=i%2?rangeKnobCentre+.25f:waveKnobCentre+.25f,cy=197.91f;
                const float a=surfaceNoise(seed+17)*juce::MathConstants<float>::twoPi;
                const float distance=.15f+7.f*std::pow(surfaceNoise(seed+31),2.4f);
                const float radius=collarDustRadius(a)*(87.f*.95f*mainKnobScale)+distance;
                x=cx+std::cos(a)*radius;y=cy+std::sin(a)*radius;
                grainFade=collarDustFade(a,distance);
                if(surfaceNoise(seed+53)>std::sqrt(grainFade))continue;
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
                x+=performanceBayOffsetX;
            }
            if(i%4!=3)x+=topPanelOffsetX;
            // A fine powder, occasional lint, and sparse brighter grains.
            const float size=.27f+.45f*std::pow(surfaceNoise(seed+89),2.0f);
            const float tone=surfaceNoise(seed+101);
            g.setColour(juce::Colour(0xffc2c3b9).interpolatedWith(juce::Colour(0xff797f7b),tone)
                        .withAlpha((.32f+.36f*surfaceNoise(seed+113))*grainFade));
            g.fillEllipse(x,y,size*(i%79==0?2.7f:1.0f),size);
        }
        // Independent deposits around the VOLUME collar, in the sheltered
        // paint beside its base. These stay on the panel as the knob turns.
        for(uint32_t i=0;i<4800;++i) {
            const uint32_t seed=surfaceHash(0x4c21a9u+i*104729u);
            const float a=surfaceNoise(seed+17)*juce::MathConstants<float>::twoPi;
            const float distance=.15f+7.f*std::pow(surfaceNoise(seed+31),2.4f);
            const float radius=collarDustRadius(a)*(88.f*.95f*mainKnobScale)+distance;
            const float x=139.75f+performanceBayOffsetX+std::cos(a)*radius;
            const float y=float(headerH+442+volumeOffsetY-volumeKnobLift)+.418f+std::sin(a)*radius;
            const float grainFade=collarDustFade(a,distance);
            if(surfaceNoise(seed+53)>std::sqrt(grainFade))continue;
            const float size=.27f+.45f*std::pow(surfaceNoise(seed+89),2.f);
            g.setColour(juce::Colour(0xffc2c3b9).interpolatedWith(juce::Colour(0xff797f7b),surfaceNoise(seed+101))
                .withAlpha((.32f+.36f*surfaceNoise(seed+113))*grainFade));
            g.fillEllipse(x,y,size*(i%83==0?2.3f:1.f),size);
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
void paintExtraDust135(juce::Graphics& target,float age) {
    if(age<=0)return;
    static const auto image=[] {
        juce::Image result(juce::Image::ARGB,designW*2,designH*2,true,juce::SoftwareImageType());
        juce::Graphics g(result);g.addTransform(juce::AffineTransform::scale(2.f));
        juce::Path paint;paint.addRectangle(60.f,65.f,1380.f,326.f);paint.addRoundedRectangle(55.f,398.f,273.f,344.f,3.f);g.reduceClipRegion(paint);
        const auto grain=[&](float x,float y,float fade,uint32_t seed) {
            const float size=.24f+.43f*std::pow(surfaceNoise(seed+89),2.f);
            g.setColour(juce::Colour(0xffc2c3b9).interpolatedWith(juce::Colour(0xff85877c),surfaceNoise(seed+101))
                .withAlpha((.29f+.35f*surfaceNoise(seed+113))*fade));
            g.fillEllipse(x,y,size*(seed%83==0?1.8f:1.f),size);
        };
        // Opposed crescents hug the inward-facing sides of the two collars.
        // Their feathered tips approach across the gap; every mark is grain.
        for(uint32_t i=0;i<24000;++i) {
            const auto seed=surfaceHash(0x135dd1u+i*7919u);const bool left=i%2==0;
            const float local=(surfaceNoise(seed+17)-.5f)*3.35f;
            const float a=local+(left?0.f:juce::MathConstants<float>::pi);
            const float distance=.2f+8.f*std::pow(surfaceNoise(seed+31),2.5f);
            const float radius=collarDustRadius(a)*(87.f*.95f*mainKnobScale)+distance;
            const float fade=std::pow(juce::jmax(0.f,std::cos(local*1.5707963f/1.675f)),.7f)*(1-distance/9.f);
            grain((left?rangeKnobCentre+.25f+topPanelOffsetX:waveKnobCentre+.25f+topPanelOffsetX)+std::cos(a)*radius,197.91f+std::sin(a)*radius,fade,seed);
        }
        // Tight, complete powder at the VOLUME collar, fading radially outward.
        for(uint32_t i=0;i<10500;++i) {
            const auto seed=surfaceHash(0x135ac2u+i*104729u);
            const float a=surfaceNoise(seed+17)*juce::MathConstants<float>::twoPi;
            const float distance=.12f+5.f*std::pow(surfaceNoise(seed+31),2.8f);
            const float radius=collarDustRadius(a)*(88.f*.95f*mainKnobScale)+distance;
            const float fade=(1-distance/5.5f)*(.65f+.35f*collarDustFade(a,distance));
            grain(139.75f+performanceBayOffsetX+std::cos(a)*radius,float(headerH+442+volumeOffsetY-volumeKnobLift)+.418f+std::sin(a)*radius,fade,seed);
        }
        // Sheltered upper-left performance-bay corner and its two seams.
        for(uint32_t i=0;i<16000;++i) {
            const auto seed=surfaceHash(0x135cb3u+i*65537u);
            const float t=surfaceNoise(seed+17),distance=14.f*std::pow(surfaceNoise(seed+31),1.6f);
            const float x=i%2?55.f+distance:55.f+105.f*t;
            const float y=i%2?398.f+108.f*t:398.f+distance;
            grain(x,y,(1-distance/15.f)*std::pow(1-t,.65f)*(.75f+.25f*surfaceNoise(seed+47)),seed);
        }
        return result;
    }();
    juce::Graphics::ScopedSaveState save(target);target.setOpacity(std::pow(age,1.35f));target.drawImage(image,juce::Rectangle<float>(0,0,designW,designH));
}
void paintPanelDust(juce::Graphics& g,float age) {
    if(age<=0)return;
    juce::Graphics::ScopedSaveState save(g);
    g.setOpacity(std::pow(juce::jlimit(0.0f,1.0f,age),1.35f));
    g.drawImage(panelDust(),juce::Rectangle<float>(0,0,designW,designH));
}
void paintCloseKnobRings144(juce::Graphics& target,float age) {
    if(age<=0)return;
    static const auto image=[] {
        juce::Image result(juce::Image::ARGB,designW*2,designH*2,true,juce::SoftwareImageType());
        juce::Graphics g(result);g.addTransform(juce::AffineTransform::scale(2.f));
        for(uint32_t i=0;i<14000;++i) {
            const auto seed=surfaceHash(0x144cd1u+i*104729u);
            const float a=surfaceNoise(seed+17)*juce::MathConstants<float>::twoPi;
            const float distance=.25f+3.2f*std::pow(surfaceNoise(seed+31),2.6f);
            const float radius=collarDustRadius(a)*(87.f*.95f*mainKnobScale)+distance;
            const float x=(i%2?rangeKnobCentre+.25f+topPanelOffsetX:waveKnobCentre+.25f+topPanelOffsetX)+std::cos(a)*radius,y=197.91f+std::sin(a)*radius;
            const float fade=(1-distance/3.8f)*(.8f+.2f*surfaceNoise(seed+53));
            const float size=.24f+.36f*std::pow(surfaceNoise(seed+71),2.f);
            g.setColour(juce::Colour(0xffb8b8a9).interpolatedWith(juce::Colour(0xff81867d),surfaceNoise(seed+89))
                .withAlpha((.32f+.33f*surfaceNoise(seed+101))*fade));
            g.fillEllipse(x,y,size,size);
        }
        return result;
    }();
    juce::Graphics::ScopedSaveState save(target);target.setOpacity(std::pow(age,1.35f));target.drawImage(image,juce::Rectangle<float>(0,0,designW,designH));
}
struct KnobMetalWear {
    juce::Image matte;
    std::array<juce::Image,3> pits;
    std::array<juce::Image,3> edgeGrime;
    KnobMetalWear(const juce::Image& photo=artwork().knob,bool largerFace=false) {
        StartupScope timing{"knob wear"};
        const float faceX=largerFace?.283f:.248f,faceY=largerFace?.273f:.258f,faceCentreY=largerFace?.422f:.410f;
        const int width=photo.getWidth(),height=photo.getHeight();
        matte=juce::Image(juce::Image::ARGB,width,height,true,juce::SoftwareImageType());
        for(auto& image:pits)image=juce::Image(juce::Image::ARGB,width,height,true,juce::SoftwareImageType());
        const juce::Image::BitmapData input(photo,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData dull(matte,juce::Image::BitmapData::readWrite);
        for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
            const float u=(float(x)+.5f)/width,v=(float(y)+.5f)/height;
            const auto colour=input.getPixelColour(x,y);
            const float tx=(u-.497f)/faceX,ty=(v-faceCentreY)/faceY;
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
                const float topX=((float(x)+.5f)/width-.497f)/faceX;
                const float topY=((float(y)+.5f)/height-faceCentreY)/faceY;
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
        for(size_t index=0;index<edgeGrime.size();++index) {
            auto& image=edgeGrime[index];image=juce::Image(juce::Image::ARGB,384,384,true,juce::SoftwareImageType());
            juce::Image::BitmapData output(image,juce::Image::BitmapData::writeOnly);
            for(int y=0;y<384;++y)for(int x=0;x<384;++x) {
                const float tx=((x+.5f)/384.f-.497f)/faceX,ty=((y+.5f)/384.f-faceCentreY)/faceY;
                const float radius=std::hypot(tx,ty);if(radius>1.005f||radius<.80f)continue;
                float deposit=0;
                for(uint32_t clump=0;clump<9;++clump) {
                    const uint32_t key=seeds[index]+clump*104729u+539;
                    const float angle=juce::MathConstants<float>::twoPi*(float(clump)+surfaceNoise(key+17)*.72f)/9.f;
                    const float centre=.855f+.125f*surfaceNoise(key+31);
                    const float radial=(tx*std::cos(angle)+ty*std::sin(angle)-centre)/(.040f+.065f*surfaceNoise(key+47));
                    const float tangent=(-tx*std::sin(angle)+ty*std::cos(angle))/(.07f+.12f*surfaceNoise(key+59));
                    const float shape=1.f-kb09_gui::power_detail::smoothRange(.45f,1.15f,std::hypot(radial,tangent));
                    deposit=juce::jmax(deposit,shape*(.45f+.35f*surfaceNoise(key+71)));
                }
                const float mottling=kb09_gui::power_detail::smoothNoise(tx*53.f,ty*53.f,seeds[index]+793);
                const float grain=surfaceNoise(seeds[index]+uint32_t(x)*7919u+uint32_t(y)*104729u);
                const float coverage=deposit*kb09_gui::power_detail::smoothRange(.25f,.66f,mottling)*(.40f+.60f*grain);
                const float tone=grain>.91f?.40f:.11f+.14f*grain;
                output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(tone,tone*.95f,tone*.81f,coverage));
            }
        }
    }
};
const KnobMetalWear& knobMetalWear() {static const KnobMetalWear material;return material;}
void keySurfaceWear(juce::Graphics& g,juce::Rectangle<float> r,int serial,bool black,float age,bool fineScuffs=false) {
    juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(r.reduced(.8f).toNearestInt());
    const auto seed=uint32_t(serial+1)*7919u;
    if(!black&&fineScuffs&&age>0) {
        juce::Graphics::ScopedSaveState rubbed(g);
        g.reduceClipRegion(r.withTrimmedBottom(8).reduced(2).toNearestInt());
        // Finger-contact patches: irregular loss of polish, with short,
        // overlapping abrasion inside the patch rather than scattered hairs.
        for(int patch=0;patch<4;++patch) {
            const uint32_t key=seed+uint32_t(patch)*104729u+4387u;
            const float strength=juce::jlimit(0.f,1.f,(age-(.08f+patch*.14f))/.38f);
            if(strength<=0)continue;
            const float cx=r.getX()+r.getWidth()*(.25f+.50f*surfaceNoise(key+11));
            const float cy=r.getY()+r.getHeight()*(.40f+.48f*surfaceNoise(key+23));
            const float rx=r.getWidth()*(.15f+.14f*surfaceNoise(key+37));
            const float ry=8.f+15.f*surfaceNoise(key+51);
            for(float y=-ry;y<=ry;y+=.65f)for(float x=-rx;x<=rx;x+=.65f) {
                const float radial=std::hypot(x/rx,y/ry);
                const float noise=kb09_gui::power_detail::smoothNoise((cx+x)*.38f,(cy+y)*.38f,key);
                const float mask=juce::jlimit(0.f,1.f,(1.f-radial+(.5f-noise)*.40f)/.45f);
                if(mask<=0)continue;
                const float grain=surfaceNoise(key+uint32_t((x+rx)*19)*7919u+uint32_t((y+ry)*19)*317u);
                g.setColour(juce::Colour(grain>.88f?0xffeee7cf:0xff9d9887).withAlpha(strength*mask*(.025f+.045f*grain)));
                g.fillRect(cx+x,cy+y,.72f,.72f);
            }
            for(uint32_t mark=0;mark<24;++mark) {
                const float a=surfaceNoise(key+mark*127)*juce::MathConstants<float>::twoPi;
                const float rad=std::sqrt(surfaceNoise(key+mark*317));
                const float x=cx+std::cos(a)*rx*rad*.85f,y=cy+std::sin(a)*ry*rad*.85f;
                const float length=.6f+2.8f*surfaceNoise(key+mark*701);
                const float slant=(surfaceNoise(key+mark*107)-.5f)*.65f;
                g.setColour(juce::Colour(0xfff8f1dc).withAlpha(strength*(.24f+.14f*surfaceNoise(key+mark*97))));
                g.drawLine(x,y,x+length,y+slant,.20f+.13f*surfaceNoise(key+mark*59));
                g.setColour(juce::Colour(0xfff4edda).withAlpha(strength*.20f));
                g.drawLine(x,y-.20f,x+length*.85f,y+slant-.20f,.15f);
            }
        }
    }
    if(black&&fineScuffs&&age>0) {
        juce::Graphics::ScopedSaveState topOnly(g);
        g.reduceClipRegion(r.withTrimmedBottom(11).reduced(1.8f).toNearestInt());
        // Independent shallow abrasion, with paired edges rather than deep
        // cavities. Fixed paths gain contrast with age; no repeated stamps.
        for(uint32_t mark=0;mark<138;++mark) {
            const uint32_t key=seed+0x18539u+mark*104729u;
            const float onset=.04f+.78f*surfaceNoise(key+17);
            const float strength=juce::jlimit(0.f,1.f,(age-onset)/.24f);
            if(strength<=0)continue;
            const float x=r.getX()+r.getWidth()*(.08f+.84f*surfaceNoise(key+31));
            const float y=r.getY()+r.getHeight()*(.05f+.85f*surfaceNoise(key+47));
            const float angle=surfaceNoise(key+59)*juce::MathConstants<float>::twoPi;
            const float length=1.4f+7.5f*std::pow(surfaceNoise(key+71),1.6f);
            const juce::Point<float> along(std::cos(angle),std::sin(angle)),across(-along.y,along.x);
            const int strands=mark%7==0?(black?6:3):1;
            for(int strand=0;strand<strands;++strand) {
                const auto start=juce::Point<float>(x,y)+across*(strand*.31f);
                const auto end=start+along*(length*(1.f-.10f*strand));
                const auto mid=(start+end)*.5f+across*(surfaceNoise(key+83)-.5f)*1.4f;
                juce::Path trace;trace.startNewSubPath(start);trace.quadraticTo(mid,end);
                const float width=.12f+.13f*surfaceNoise(key+97);
                g.setColour(juce::Colour(0xff090b0a).withAlpha(strength*(black?.27f:.09f)));
                g.strokePath(trace,juce::PathStrokeType(width+.12f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                g.setColour(juce::Colour(black?0xffb8bdb7:0xffeee9d7).withAlpha(strength*(.13f+.17f*surfaceNoise(key+113))));
                g.strokePath(trace,juce::PathStrokeType(width,juce::PathStrokeType::curved,juce::PathStrokeType::rounded),juce::AffineTransform::translation(-.10f,-.13f));
            }
        }
    }
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
        for(uint32_t scratch=0;scratch<uint32_t((black?54.f:fineScuffs?18.f:55.f)*age);++scratch) {
            const float x=r.getX()+r.getWidth()*(.14f+.68f*surfaceNoise(seed+scratch*127));
            const float y=r.getY()+r.getHeight()*(black?(.12f+.68f*surfaceNoise(seed+scratch*419)):(.68f+.25f*surfaceNoise(seed+scratch*419)));
            const float dx=(surfaceNoise(seed+scratch*199)-.5f)*16,dy=1+(9+2*ivoryWear)*surfaceNoise(seed+scratch*733);
            juce::Path trace;trace.startNewSubPath(x,y);trace.quadraticTo(x+dx*.2f+1,y+dy*.5f,x+dx,y+dy);
            g.setColour(juce::Colour(black?0xffb7b9ae:fineScuffs?0xfff8f1dc:0xff696e68).withAlpha((1+.65f*ivoryWear)*(.12f+.12f*surfaceNoise(seed+scratch*37))));
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
    if constexpr(KB09_APPROVED_HARDWARE) {
        // Reference blades are stockier: preserve their position and material,
        // Keep the approved width; use 75% of proposal 265's exposed height.
        g.addTransform(juce::AffineTransform::scale(1.20f,.51f,cx,capBounds.getCentreY()));
    }
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

void cap(juce::Graphics& g,juce::Rectangle<float> r,bool small=false,bool indexed=true,float age=0,uint32_t wearSeed=0,bool grooveDust=false,bool heavyDust=false) {
    const auto ink=agedInk(age);
    if constexpr(KB09_APPROVED_HARDWARE)if(!small) {
        // The model's silver-band centre is the value indicator, independent
        // of the asymmetric projected plastic silhouette.
        static kb09_gui::FaderCapModel model;
        juce::Graphics::ScopedSaveState save(g);
        model.draw(g,{r.getCentreX(),r.getCentreY()-.675f},28.f,18.f,age,wearSeed);
        return;
    }
    if(!small) {
        juce::Graphics::ScopedSaveState save(g);
        g.setOpacity(1.0f);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        // The cap's broad TOP now faces the camera. The central top index
        // continues over the two narrow sides, never over the front lip.
        // Owner's annotated measurement: 19/32-inch width and 3/8-inch
        // TOTAL visible cap height, including the front lip, against the
        // 1-5/8-inch visible main-panel slot (98 design units).
        const float width=KB09_APPROVED_HARDWARE?98.f*(19.f/52.f):28.f;
        constexpr float frontLip=2.f;
        const float topDepth=KB09_APPROVED_HARDWARE?98.f*(3.f/13.f)-frontLip:16.f;
        const auto photo=juce::Rectangle<float>(r.getCentreX()-width*.5f,r.getCentreY()-(topDepth+frontLip)*.5f,width,topDepth+frontLip);
        const auto source=juce::Rectangle<int>(125,242,1240,565);
        const auto drawBody=[&](const juce::Image& image) {
            if constexpr(KB09_APPROVED_HARDWARE) {
                // Stretch the photographed top face to its measured footprint;
                // preserve the front lip's existing projected thickness.
                constexpr int faceRows=502;
                g.drawImage(image,photo.getX(),photo.getY(),width,topDepth,
                    source.getX(),source.getY(),source.getWidth(),faceRows);
                g.drawImage(image,photo.getX(),photo.getY()+topDepth,width,frontLip,
                    source.getX(),source.getY()+faceRows,source.getWidth(),source.getHeight()-faceRows);
            } else g.drawImage(image,photo.getX(),photo.getY(),photo.getWidth(),photo.getHeight(),
                source.getX(),source.getY(),source.getWidth(),source.getHeight());
        };
        juce::Path solid;solid.addRoundedRectangle(photo.reduced(.7f,.5f),2.0f);
        g.setColour(juce::Colour(0xff171719));g.fillPath(solid);
        drawBody(artwork().fader);
        if(age>0) {
            g.setColour(juce::Colours::black.withAlpha(age*.10f));
            g.fillPath(solid);
            // Silver loses its narrow photographic shine gradually; it neither
            // yellows with the printing nor acquires the exposed posts' rust.
            g.setOpacity(age*.90f);
            drawBody(artwork().faderDull);
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
            const float x=left+3+(photo.getWidth()-7)*surfaceNoise(seed),y=cy+(i%2?-6.9f:2.f)+3.8f*surfaceNoise(seed+13);
            juce::Path scuff;scuff.startNewSubPath(x,y);scuff.quadraticTo(x+.3f,y+.7f,x+.4f+1.5f*surfaceNoise(seed+31),y+1.3f);
            g.setColour(juce::Colour(0xffa0a6a0).withAlpha(.19f));
            g.strokePath(scuff,juce::PathStrokeType(.11f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
        return;
    }
    if(!indexed) {
        // Straight rectangular molded switch, with narrow polished edges.
        // The cap fits inside the recessed slot at every switch position.
        juce::Path tongue;tongue.addRectangle(r.reduced(.35f,.2f));
        if constexpr(KB09_APPROVED_HARDWARE)kb09_gui::roundedHardwareShadow(g,r.reduced(.35f,.2f),.4f,.28f);
        else {g.setColour(juce::Colours::black.withAlpha(.52f));g.fillPath(tongue,juce::AffineTransform::translation(1.1f,1.3f));}
        juce::ColourGradient plastic(juce::Colour(0xff373a3a),r.getCentreX(),r.getY(),
                                    juce::Colour(0xff202526),r.getCentreX(),r.getBottom(),false);
        plastic.addColour(.48,juce::Colour(0xff292d2e));plastic.addColour(.69,juce::Colour(0xff24292a));
        g.setGradientFill(plastic);g.fillPath(tongue);
        g.setColour(juce::Colour(0xff030405));g.strokePath(tongue,juce::PathStrokeType(.65f));
        for(int i=0;i<6;++i) {
            const float yy=r.getY()+r.getHeight()*(.13f+.148f*float(i));
            const float inset=r.getWidth()*.12f;
            g.setColour(juce::Colour(0xff050708));g.drawLine(r.getX()+inset,yy+.6f,r.getRight()-inset,yy+.6f,.85f);
            g.setColour(juce::Colour(0xffa0a09a).withAlpha(.34f+age*.16f));
            g.drawLine(r.getX()+inset,yy,r.getRight()-inset,yy,.65f);
        }
        if(grooveDust&&age>0) {
            // Sheltered grooves accumulate fine powder, with a stable pattern
            // unique to each switch and each gap. Keep rib crests uncovered.
            juce::Graphics::ScopedSaveState dustClip(g);g.reduceClipRegion(tongue);
            const float left=r.getX()+r.getWidth()*.13f,width=r.getWidth()*.74f;
            const float step=r.getHeight()*.148f,gapHeight=step-1.55f;
            for(uint32_t gap=0;gap<5;++gap) {
                const uint32_t seed=surfaceHash(wearSeed+gap*104729u+0x6d317u);
                const float top=r.getY()+r.getHeight()*(.13f+.148f*float(gap))+1.05f;
                for(uint32_t patch=0;patch<38;++patch) {
                    const float u=(float(patch)+.5f)/38.f;
                    const float density=.25f+.75f*surfaceNoise(seed+patch*113u);
                    const float height=gapHeight*(heavyDust?.50f+.48f*density:.22f+.58f*density);
                    g.setColour(juce::Colour(0xff938d7d).withAlpha(std::pow(age,heavyDust?1.45f:2.f)*(heavyDust?.42f:.16f)*density));
                    g.fillRect(left+width*u,top+gapHeight-height,width/38.f+.025f,height);
                }
                for(uint32_t grain=0;grain<uint32_t((heavyDust?230.f:96.f)*age);++grain) {
                    const uint32_t key=seed+grain*7919u;
                    const float x=left+width*surfaceNoise(key+13);
                    const float y=top+gapHeight*(.1f+.82f*std::sqrt(surfaceNoise(key+31)));
                    const float size=.065f+.15f*surfaceNoise(key+47);
                    g.setColour(juce::Colour(0xffc0b9a5).interpolatedWith(juce::Colour(0xff686b61),surfaceNoise(key+59))
                        .withAlpha((.18f+.48f*surfaceNoise(key+71))*age*(heavyDust?1.35f:1.f)));
                    g.fillEllipse(x,y,size*(grain%29==0?2.f:1.f),size*.65f);
                }
            }
        }
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
    if(small && indexed) {
        // Same cool silver stops as the synth caps, independent of print ink.
        juce::ColourGradient silver(juce::Colour(0xffa2abad),r.getX(),r.getCentreY()-1,
            juce::Colour(0xffbdc5c4),r.getRight(),r.getCentreY()+1,false);
        silver.addColour(.18,juce::Colour(0xffe8eeee));silver.addColour(.40,juce::Colour(0xffa9b3b4));
        silver.addColour(.68,juce::Colour(0xffd5dcda));
        g.setGradientFill(silver);g.fillRect(r.getX()+2,r.getCentreY()-.65f,r.getWidth()-4,1.3f);
        g.setColour(juce::Colour(0xffe8eeee).withAlpha(.65f));
        g.drawLine(r.getX()+2,r.getCentreY()-.65f,r.getRight()-2,r.getCentreY()-.65f,.2f);
    }
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
void projectedKnob(juce::Graphics& g,juce::Rectangle<float> area,float angle,float age,uint32_t seed,bool dullGrips=false,bool gentleDulling=false,bool innerCollar=false,bool thickTicks=false,bool shorterCollar=false,bool topRightLight=false,bool rimShine=false,bool refined132=false,bool detail135=false,bool inner143=false,bool fixed152=false,bool fill161=false,bool broad165=false,bool dust166=false,bool glint167=false,bool shadow172=false,float assemblyScale=mainKnobScale) {
    // Final small inset correction retains the approved body height/camera.
    juce::Graphics::ScopedSaveState save(g);
    const float d=juce::jmin(area.getWidth(),area.getHeight())*.95f;
    const auto sprite=area.withSizeKeepingCentre(d,d);
    // One transform preserves every internal dimension, tick, wear mark and
    // shadow. Keep the actual collar centre fixed; hit areas remain unchanged.
    g.addTransform(juce::AffineTransform::scale(assemblyScale,assemblyScale,
        sprite.getX()+d*.497f,sprite.getY()+d*.505f));
    if(shadow172) {
        // Same five-layer penumbra, opacity and down-right source as black keys.
        // The collar sits on the panel, so its stationary footprint casts it.
        const juce::Rectangle<float> footprint(sprite.getX()+d*(.497f-.44f),sprite.getY()+d*(.505f-.445f),d*.88f,d*.89f);
        const float scale=d/(88.f*.95f);
        for(int i=5;i>0;--i) {
            g.setColour(juce::Colours::black.withAlpha(.07f));
            g.fillEllipse(footprint.expanded(float(i)*.45f*scale).translated(2.f*scale,3.f*scale));
        }
    }
    const auto top=juce::Point<float>(sprite.getX()+d*.497f,sprite.getY()+d*(topRightLight?.422f:.410f));
    const float faceScale=fixed152?kb09_gui::KnobPlasticLighting::thickerInnerRadius/kb09_gui::KnobPlasticLighting::innerRadius:1.f;
    static std::unique_ptr<kb09_gui::ReferenceKnobModel> referenceModel;
    if(topRightLight&&!referenceModel)referenceModel=std::make_unique<kb09_gui::ReferenceKnobModel>(
        Artwork::load(BinaryData::knobtoprightsquarerimv95_png,BinaryData::knobtoprightsquarerimv95_pngSize).getClippedImage({70,70,1114,1114}),artwork().knob);
    const auto& knobPhoto=topRightLight?referenceModel->frame(0,3,0,false,false,false,false,fixed152):artwork().knob;
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    static kb09_gui::KnobGripMotion originalMotion(artwork().knob);
    const size_t index=seed==12011u?0:seed==41117u?1:2;
    // Make the reflection reduction linear in AGE, ending at the installed
    // Vintage finish (.375 wear) rather than its Fire Hazard finish (.75).
    const float gripWear=gentleDulling?.375f*std::pow(age,1.f/1.05f):age*.75f;
    g.setOpacity(1.f);
    if(topRightLight)g.drawImage(referenceModel->frame(angle,index,age,rimShine,refined132,detail135,inner143,fixed152,fill161,broad165,dust166,shadow172),sprite,juce::RectanglePlacement::stretchToFit);
    else {g.drawImage(knobPhoto,sprite,juce::RectanglePlacement::stretchToFit);g.drawImage(originalMotion.frame(angle,index,dullGrips?gripWear:0.f),sprite,juce::RectanglePlacement::stretchToFit);}
    if(age>0) {
        juce::Path body;
        if(topRightLight) {
            const auto point=[&](float a,float y){return juce::Point<float>(sprite.getX()+d*(.497f+.326f*std::cos(a)),sprite.getY()+d*(y+.315f*std::sin(a)));};
            body.startNewSubPath(point(juce::MathConstants<float>::pi,.410f));
            for(int i=49;i<=96;++i)body.lineTo(point(float(i)*juce::MathConstants<float>::twoPi/96,.410f));
            for(int i=0;i<=48;++i)body.lineTo(point(float(i)*juce::MathConstants<float>::twoPi/96,.488f));
            body.closeSubPath();
        }
        const auto ageLayer=[&](const juce::Image& image,float opacity,bool mask) {
            for(int part=0;part<(topRightLight?2:1);++part) {
                juce::Graphics::ScopedSaveState clip(g);
                if(topRightLight) {
                    juce::Path region;
                    if(part==1){region.addRectangle(sprite);region.addPath(body);region.setUsingNonZeroWinding(false);}
                    else region=body;
                    g.reduceClipRegion(region);
                }
                const float amount=opacity*(part==1?.5f:1.f);
                if(mask){g.setOpacity(1);g.setColour(juce::Colours::black.withAlpha(amount));}
                else g.setOpacity(amount);
                g.drawImage(image,sprite,juce::RectanglePlacement::stretchToFit,mask);
            }
        };
        ageLayer(knobPhoto,age*.12f,true);
        const auto& material=[&]() -> const KnobMetalWear& {
            if(topRightLight){static std::array<std::unique_ptr<KnobMetalWear>,2> relit;
                auto& finish=relit[fixed152?1:0];if(!finish)finish=std::make_unique<KnobMetalWear>(knobPhoto,true);return *finish;}
            return knobMetalWear();
        }();
        ageLayer(material.matte,age*.84f,false);
        {
            juce::Graphics::ScopedSaveState movingWear(g);
            juce::Path face;face.addEllipse(top.x-d*(topRightLight?.283f:.248f)*faceScale,top.y-d*(topRightLight?.273f:.258f)*faceScale,d*(topRightLight?.566f:.496f)*faceScale,d*(topRightLight?.546f:.516f)*faceScale);g.reduceClipRegion(face);
            const float aspect=topRightLight?.273f/.283f:.258f/.248f;
            g.addTransform(juce::AffineTransform::translation(-top.x,-top.y).scaled(1,1/aspect)
                .rotated(angle).scaled(1,aspect).translated(top.x,top.y));
            g.setOpacity(std::pow(age,1.35f)*(thickTicks?.78f:1.f));g.drawImage(material.pits[index],sprite,juce::RectanglePlacement::stretchToFit);
            if(thickTicks) {
                g.setOpacity(std::pow(age,1.2f)*.90f);
                g.drawImage(material.edgeGrime[index],sprite,juce::RectanglePlacement::stretchToFit);
            }
        }
        g.setOpacity(1);
    }
    if(topRightLight&&dust166&&age>.20f) {
        // Larger deposits sit above the fine photographic pitting and matte
        // film, rather than being washed out by those ageing layers.
        g.setOpacity(1);g.drawImage(referenceModel->grimeFrame(index,age),sprite,juce::RectanglePlacement::stretchToFit);
    }
    for(uint32_t i=0;i<116;++i) {
        const float a=surfaceNoise(seed+i*71)*juce::MathConstants<float>::twoPi-angle;
        if(std::cos(a)<0)continue;
        const auto p=top+juce::Point<float>(std::sin(a)*d*.335f,
            std::cos(a)*d*.335f*panelViewCos+d*(.025f+.050f*surfaceNoise(seed+i*79)));
        const float size=d*(.002f+.003f*surfaceNoise(seed+i*37));
        g.setColour(juce::Colour(0xffc6beaa).withAlpha(age*.35f));g.fillEllipse(p.x,p.y,size,size*.70f);
    }
    // The native opaque black tick rotates; photographic lighting stays fixed.
    const auto direction=juce::Point<float>(std::sin(angle),-std::cos(angle)*panelViewCos);
    g.setColour(juce::Colour(0xff141718));
    // One third of the metal top's diameter, with a clear square-ended mark.
    const float indexThickness=d*(thickTicks?.040f:.028f);
    if(thickTicks) {
        // Slide the whole printed mark out to the photographed metal rim.
        // The top is elliptical in this view, so a fixed radial endpoint
        // leaves different gaps at different pointer angles.
        const float rx=(topRightLight?.285f:.250f)*faceScale,ry=(topRightLight?.275f:.260f)*faceScale;
        const float rim=1.f/std::sqrt(direction.x*direction.x/(rx*rx)+direction.y*direction.y/(ry*ry));
        juce::Graphics::ScopedSaveState topTick(g);
        juce::Path face;face.addEllipse(top.x-d*rx,top.y-d*ry,d*rx*2,d*ry*2);g.reduceClipRegion(face);
        g.drawLine({top+direction*d*(rim-.166f),top+direction*d*rim},indexThickness);
        if(rimShine) {
            const auto start=top+direction*d*(rim-.166f),end=top+direction*d*rim;
            const float gloss=1-age*.55f;
            juce::ColourGradient plastic(juce::Colour(0xff41474a).interpolatedWith(juce::Colour(0xff252a2c),1-gloss),
                top.x-d*.24f,top.y-d*.24f,juce::Colour(0xff111516),top.x+d*.24f,top.y+d*.24f,false);
            if(fixed152)g.setColour(juce::Colour(0xff070909));
            else g.setGradientFill(plastic);
            g.drawLine({start,end},indexThickness);
            if(fixed152) {
                // The same molded plastic as the rim: a visible narrow bevel
                // catches the fixed upper-left source, with a moving glint.
                const auto across=juce::Point<float>(std::cos(angle),std::sin(angle)*panelViewCos);
                const float edgeFacing=juce::jmax(0.f,.70710678f*(std::cos(angle)+std::sin(angle)));
                const float glint=kb09_gui::KnobPlasticLighting::tickGlint(angle,age,glint167);
                const auto shift=across*(-indexThickness*.36f)+juce::Point<float>(0,-d*.001f);
                if(glint167) {
                    // A broad reflection lightens the entire molded tick while
                    // preserving its square silhouette and apparent width.
                    g.setColour(juce::Colour(0xff070909).interpolatedWith(juce::Colour(0xffaab4ae),glint*.32f));
                    g.drawLine({start,end},indexThickness);
                } else {
                const float gain=glint167?.92f:.66f;
                g.setColour(juce::Colour::fromFloatRGBA(.035f+gain*glint+.035f*edgeFacing,.040f+gain*glint+.035f*edgeFacing,.040f+gain*.985f*glint+.035f*edgeFacing,1));
                g.drawLine({start+shift+direction*d*.004f,end+shift-direction*d*.003f},juce::jmax(.30f,indexThickness*(glint167?.29f:.17f)));
                g.setColour(juce::Colour(glint167?0xffedf2ee:0xffdce1dd).withAlpha(glint*(glint167?.96f:.76f)));
                g.drawLine({end-direction*d*.030f+shift,end-direction*d*.003f+shift},juce::jmax(.22f,indexThickness*(glint167?.24f:.13f)));
                }
            }
        }
    } else g.drawLine({top+direction*d*.079f,top+direction*d*.245f},indexThickness);
    // Matching radial index on the flared metal collar; body lighting stays fixed.
    const auto collar=juce::Point<float>(top.x,sprite.getY()+d*.505f);
    if(rimShine&&fixed152&&glint167) {
        // The collar index keeps its full silhouette while sharing the molded
        // top tick's gradual loss of gloss, with its own unoccluded light angle.
        const float glint=kb09_gui::KnobPlasticLighting::collarTickGlint(angle,age);
        g.setColour(juce::Colour(0xff141718).interpolatedWith(juce::Colour(0xffaab4ae),glint*.32f));
    }

    if(innerCollar) {
        // Intersect the same cylinder used by the rendered grip texture:
        // rear rim y=.410, front rim y=.410+.078, collar center y=.505.
        const float vx=direction.x,vy=direction.y;
        const auto ellipseRadius=[&](float centreY) {
            const float aa=vx*vx/(.326f*.326f)+vy*vy/(.315f*.315f);
            const float bb=-2.f*centreY*vy/(.315f*.315f);
            const float cc=centreY*centreY/(.315f*.315f)-1.f;
            return (-bb+std::sqrt(bb*bb-4.f*aa*cc))/(2.f*aa);
        };
        float innerRadius=0;
        const float front=ellipseRadius(-.017f),rear=ellipseRadius(-.095f);
        if(front*vy>=-.017f)innerRadius=front;
        if(rear*vy<=-.095f)innerRadius=juce::jmax(innerRadius,rear);
        if(std::abs(vx)>.00001f) {
            const float side=.326f/std::abs(vx),y=side*vy;
            if(y>=-.095f&&y<=-.017f)innerRadius=juce::jmax(innerRadius,side);
        }
        // A short overlap meets the grip's irregular edge cleanly. Leave
        // 1.5 index strokes of metal before the visible outside edge.
        const float directionLength=direction.getDistanceFromOrigin();
        // Measure the actual photographic silhouette once. Transparent image
        // padding and an estimated ellipse must not set the visible margin.
        static const auto collarEdges=[] {
            std::array<float,720> radii{};
            const auto& photo=artwork().knob;
            juce::Image::BitmapData pixels(photo,juce::Image::BitmapData::readOnly);
            for(size_t i=0;i<radii.size();++i) {
                const float a=float(i)*juce::MathConstants<float>::twoPi/float(radii.size());
                auto ray=juce::Point<float>(std::sin(a),-std::cos(a)*panelViewCos);
                ray/=ray.getDistanceFromOrigin();
                float radius=.56f;
                for(;radius>.25f;radius-=.0005f) {
                    const int x=juce::jlimit(0,photo.getWidth()-1,int((.497f+ray.x*radius)*photo.getWidth()));
                    const int y=juce::jlimit(0,photo.getHeight()-1,int((.505f+ray.y*radius)*photo.getHeight()));
                    if(pixels.getPixelColour(x,y).getFloatAlpha()>=.5f)break;
                }
                radii[i]=radius;
            }
            return radii;
        }();
        float phase=std::fmod(angle,juce::MathConstants<float>::twoPi);
        if(phase<0)phase+=juce::MathConstants<float>::twoPi;
        const float tableAt=phase*float(collarEdges.size())/juce::MathConstants<float>::twoPi;
        const int edgeIndex=int(tableAt)%int(collarEdges.size());
        const float outside=juce::jmap(tableAt-std::floor(tableAt),collarEdges[size_t(edgeIndex)],collarEdges[size_t((edgeIndex+1)%int(collarEdges.size()))]);
        const float begin=innerRadius-.012f;
        // Preserve the approved endpoint while changing the printed stroke width.
        // Exclude the rolled outer rim from the printable silver segment.
        const float exposedEnd=(outside-.022f)/directionLength;
        const float approvedEnd=(outside-1.5f*.028f)/directionLength;
        const float end=shorterCollar?juce::jmin(approvedEnd,innerRadius+(exposedEnd-innerRadius)*(2.f/3.f))
            :approvedEnd;
        if(end>begin) {
            const auto inner=collar+direction*d*begin,outer=collar+direction*d*end;
            const auto across=juce::Point<float>(-direction.y,direction.x)/directionLength;
            const float length=inner.getDistanceFrom(outer);
            const float tipWidth=juce::jmax(indexThickness*.5f,indexThickness-2.f*length*std::tan(juce::degreesToRadians(shorterCollar?2.f:3.f)));
            juce::Path mark;mark.startNewSubPath(inner+across*(indexThickness*.5f));
            mark.lineTo(outer+across*(tipWidth*.5f));mark.lineTo(outer-across*(tipWidth*.5f));
            mark.lineTo(inner-across*(indexThickness*.5f));mark.closeSubPath();g.fillPath(mark);
        }
    } else g.drawLine({collar+direction*d*.390f,collar+direction*d*.440f},indexThickness);
}
// The six cavity masks come from the clean chassis photograph, registered
// through the same crop/scale as the panel. The measured centers and axes bound
// each original recess; its actual pixel shape supplies the mask and texture.
const std::array<juce::Image,6>& screwCavityMasks() {
    static const auto masks=[] {
        std::array<juce::Image,6> result;
        const auto& art=artwork();
        const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
            juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),
            juce::roundToInt(art.chassis.getHeight()*873.0/887));
        const std::array<juce::Point<float>,6> heads{{{88,37},{1686,37},{88,328},{1686,328},{83,399},{1692,399}}};
        const std::array<juce::Point<float>,6> cavities{{{85.885f,34.985f},{1686,34.835f},{84.915f,326.965f},
            {1686.065f,326.815f},{79.335f,397.415f},{1691.85f,396.835f}}};
        const std::array<float,6> degrees{{6.967f,-6.440f,7.884f,-7.013f,15.332f,5.592f}};
        for(size_t i=0;i<heads.size();++i) {
            const auto centre=juce::Point<float>((heads[i].x-2)*float(designW)/1770,(heads[i].y-7)*float(bodyH)/873);
            juce::Image photograph(juce::Image::ARGB,96,96,true,juce::SoftwareImageType());
            {
                juce::Graphics pg(photograph);pg.addTransform(juce::AffineTransform::scale(4.f));
                pg.drawImage(art.chassis.getClippedImage(source),{12-centre.x,12-centre.y,float(designW),float(bodyH)},juce::RectanglePlacement::stretchToFit);
            }
            auto& mask=result[i];mask=juce::Image(juce::Image::ARGB,96,96,true,juce::SoftwareImageType());
            const float angle=juce::degreesToRadians(degrees[i]),ca=std::cos(angle),sa=std::sin(angle);
            juce::Image::BitmapData pixels(photograph,juce::Image::BitmapData::readOnly),output(mask,juce::Image::BitmapData::writeOnly);
            for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                const float sx=heads[i].x+((x+.5f)/4.f-12)*1770.f/float(designW)-cavities[i].x;
                const float sy=heads[i].y+((y+.5f)/4.f-12)*873.f/float(bodyH)-cavities[i].y;
                const float u=sx*ca+sy*sa,v=sy*ca-sx*sa;
                const float edge=1.f-kb09_gui::power_detail::smoothRange(1.65f,2.25f,juce::jmin(std::abs(u),std::abs(v)));
                const float tip=1.f-kb09_gui::power_detail::smoothRange(4.4f,5.2f,juce::jmax(std::abs(u),std::abs(v)));
                const auto colour=pixels.getPixelColour(x,y);
                const float alpha=edge*tip*(1.f-kb09_gui::power_detail::smoothRange(.10f,.34f,colour.getBrightness()));
                if(alpha>0)output.setPixelColour(x,y,colour.withMultipliedBrightness(.65f).withAlpha(alpha));
            }
        }
        return result;
    }();
    return masks;
}
void paintRepairedScrewSeat(juce::Graphics& g,juce::Point<float> c,float age,int index) {
    LivePaintScope timing{"repair"};
    static std::array<juce::Image,30> patches;
    static std::mutex patchesMutex;
    std::unique_lock<std::mutex> patchLock(patchesMutex);
    const int stage=kb09_gui::appearanceStage(age);
    auto& patch=patches[size_t(stage*6+index)];
    if(!patch.isValid()) {
        const auto& art=artwork();
        const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
            juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),juce::roundToInt(art.chassis.getHeight()*873.0/887));
        auto sample=[&](float y) {
            juce::Image result(juce::Image::ARGB,257,257,true,juce::SoftwareImageType());
            juce::Graphics paint(result);paint.addTransform(juce::AffineTransform::scale(4.f));
            paint.drawImage(art.chassis,32-c.x,32-y,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            if(age>0)paint.drawImage(art.panelWear[size_t(stage)],32-c.x,32-y,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            return result;
        };
        patch=kb09_gui::repairScrewSeat(sample(c.y),sample(c.y<80?140.f:200.f),index%2!=0);
    }
    const auto readyPatch=patch;patchLock.unlock();
    juce::Graphics::ScopedSaveState clip(g);g.setOpacity(1.f);
    g.reduceClipRegion(juce::Rectangle<int>(58,5,1367,360));
    g.drawImage(readyPatch,{c.x-32,c.y-32,64,64});
}

void screwOxidation(juce::Graphics& g,juce::Point<float> c,float age,uint32_t seed,bool previewRotation=false,bool socketPreview=false,bool lighterGrey=false,bool revised73=false,bool bay73=false,bool gentler77=false,bool proposal84=false,bool reposition131=false) {
    if(gentler77&&age>=.999f) {
        // Endpoint uses the complete approved Sold As-Is material path.
        screwOxidation(g,c,.75f,seed,previewRotation,socketPreview,lighterGrey,revised73,bay73,false,proposal84,reposition131);
        return;
    }
    if(age<=0&&!previewRotation&&!socketPreview)return;
    // One material progression for every condition. A broad photographic light
    // field replaces specular detail, while original rim/contact shadows and
    // black cavities are retained. There is no Fire-Hazard-only brightness cap.
    static const auto finishes=[] {
        StartupScope timing{"legacy screw materials"};
        std::array<std::array<juce::Image,6>,5> result;
        const auto& art=artwork();
        const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
            juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),
            juce::roundToInt(art.chassis.getHeight()*873.0/887));
        const std::array<juce::Point<float>,6> heads{{{88,37},{1686,37},{88,328},{1686,328},{83,399},{1692,399}}};
        const std::array<juce::Point<float>,6> cavities{{{85.885f,34.985f},{1686,34.835f},{84.915f,326.965f},
            {1686.065f,326.815f},{79.335f,397.415f},{1691.85f,396.835f}}};
        const std::array<float,6> degrees{{6.967f,-6.440f,7.884f,-7.013f,15.332f,5.592f}};
        for(size_t i=0;i<heads.size();++i) {
            const auto centre=juce::Point<float>((heads[i].x-2)*float(designW)/1770,(heads[i].y-7)*float(bodyH)/873);
            juce::Image photo(juce::Image::ARGB,96,96,true,juce::SoftwareImageType());
            {
                juce::Graphics pg(photo);pg.addTransform(juce::AffineTransform::scale(4.f));
                pg.drawImage(art.chassis.getClippedImage(source),{12-centre.x,12-centre.y,float(designW),float(bodyH)},juce::RectanglePlacement::stretchToFit);
            }
            std::array<float,96*96> field{};
            juce::Image::BitmapData original(photo,juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData cavity(screwCavityMasks()[i],juce::Image::BitmapData::readOnly);
            result[0][i]=photo.createCopy();
            {
                juce::Image::BitmapData clean(result[0][i],juce::Image::BitmapData::readWrite);
                for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                    const float dx=(x+.5f)/4.f-12,dy=(y+.5f)/4.f-12;
                    const float alpha=1.f-kb09_gui::power_detail::smoothRange(9.5f,10.8f,std::sqrt(dx*dx+dy*dy));
                    clean.setPixelColour(x,y,clean.getPixelColour(x,y).withAlpha(alpha));
                }
            }
            // Blur only the illumination used by the matte material. The
            // original photograph still supplies the silhouette and fine grain.
            for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                float sum=0,weights=0;
                for(int yy=-10;yy<=10;yy+=2)for(int xx=-10;xx<=10;xx+=2) {
                    const float weight=1.f/(1.f+float(xx*xx+yy*yy)*.022f);
                    sum+=weight*original.getPixelColour(juce::jlimit(0,95,x+xx),juce::jlimit(0,95,y+yy)).getBrightness();weights+=weight;
                }
                field[size_t(y*96+x)]=sum/weights;
            }
            const float angle=juce::degreesToRadians(degrees[i]),ca=std::cos(angle),sa=std::sin(angle);
            for(int stage=1;stage<5;++stage) {
                const float condition=float(stage)*.25f,finish=std::pow(condition,.75f);
                // Stripped sockets are a discrete damage event at Sold As-Is.
                const float worn=stage>=3?1.f:0.f;
                auto& image=result[size_t(stage)][i];image=juce::Image(juce::Image::ARGB,96,96,true,juce::SoftwareImageType());
                juce::Image::BitmapData output(image,juce::Image::BitmapData::writeOnly);
                for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                    const float dx=(x+.5f)/4.f-12,dy=(y+.5f)/4.f-12;
                    const float distance=std::sqrt(dx*dx+dy*dy);
                    const float coverage=1.f-kb09_gui::power_detail::smoothRange(9.5f,10.8f,distance);
                    if(coverage<=0)continue;
                    const auto fresh=original.getPixelColour(x,y);const float bright=fresh.getBrightness();
                    const float low=field[size_t(y*96+x)];
                    const auto key=5011u+uint32_t(i)*7717u;
                    const float grain=surfaceNoise(key+uint32_t(x)*7919u+uint32_t(y)*104729u);
                    const float detail=juce::jlimit(-.022f,.022f,(bright-low)*.08f);
                    // Deep photographic shadows stay deep; dulling does not
                    // replace the dome or the contact shadow with a flat disk.
                    const float shadow=kb09_gui::power_detail::smoothRange(.025f,.225f,bright);
                    const float value=juce::jlimit(0.f,1.f,(.07f+.58f*std::pow(low,.85f)+detail)*(.30f+.70f*shadow)*(.96f+.04f*grain));
                    auto matte=juce::Colour::fromFloatRGBA(value,value*.965f,value*.875f,1.f);
                    // Rust patches grow from a fixed field and retain their hue.
                    const float rustField=.56f*kb09_gui::power_detail::smoothNoise((dx+dy*.37f)*.39f,(dy-dx*.24f)*.41f,key+53)
                        +.29f*kb09_gui::power_detail::smoothNoise(dx*1.17f+2.8f,dy*.91f-4.2f,key+79)
                        +.15f*kb09_gui::power_detail::smoothNoise((dx-dy)*2.13f,(dx+dy)*1.91f,key+107);
                    const float rust=kb09_gui::power_detail::smoothRange(.80f-.20f*condition,.87f-.20f*condition,rustField);
                    const float red=value*(.85f+.20f*grain);
                    matte=matte.interpolatedWith(juce::Colour::fromFloatRGBA(red,red*.46f,red*.24f,1),rust*.85f);
                    auto colour=fresh.interpolatedWith(matte,finish);
                    // Cavity coverage is independent of surface patina. Its
                    // interior remains near-black, never gray oxide paint.
                    float hole=kb09_gui::power_detail::smoothRange(.02f,.40f,cavity.getPixelColour(x,y).getFloatAlpha());
                    if(i==2||i==4) {
                        const float sx=heads[i].x+dx*1770.f/float(designW)-cavities[i].x;
                        const float sy=heads[i].y+dy*873.f/float(bodyH)-cavities[i].y;
                        const float u=sx*ca+sy*sa,v=sy*ca-sx*sa;
                        const float a=std::atan2(v,u);
                        const float jagged=1.f+.10f*std::sin(a*3.f+float(i))+.055f*std::cos(a*7.f-.8f);
                        const float radius=(1.05f+3.05f*worn)*jagged;
                        const float r=std::sqrt(u*u+(i==2?1.16f:.86f)*v*v);
                        const float stripped=worn>0?1.f-kb09_gui::power_detail::smoothRange(radius-.25f,radius+.20f,r):0.f;
                        // Uneven wall and torn lip around a widened, rounded-out
                        // driver socket. Both remain aligned to the original.
                        const float lip=kb09_gui::power_detail::smoothRange(radius-.1f,radius+.2f,r)
                            *(1.f-kb09_gui::power_detail::smoothRange(radius+.2f,radius+.65f,r))*worn;
                        const float wall=value*(.35f+.22f*juce::jlimit(0.f,1.f,-std::sin(a)-std::cos(a)));
                        colour=colour.interpolatedWith(juce::Colour::fromFloatRGBA(wall,wall*.94f,wall*.81f,1),lip*.8f);
                        hole=juce::jmax(hole,stripped);
                    }
                    const float dark=.009f+.009f*grain;
                    colour=colour.interpolatedWith(juce::Colour::fromFloatRGBA(dark,dark*.98f,dark*.88f,1),hole);
                    output.setPixelColour(x,y,colour.withAlpha(coverage));
                }
            }
        }
        return result;
    }();
    // Review only: photographic macro material, registered individually so
    // every age has the same head diameter. No procedural cross or painted rust.
    const auto* selected=&finishes;
    if(socketPreview) {
        const auto makePhotographic=[&](bool gentle,int wantedStage) {
            StartupScope timing{gentle?"gentler photographic screws":"old photographic screws"};
            std::array<std::array<juce::Image,6>,5> result{};
            const auto atlas=juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(
                BinaryData::panelscrewsphotov65_png,BinaryData::panelscrewsphotov65_pngSize));
            const std::array<juce::Rectangle<int>,30> crops{{
                {28,29,194,192},{255,30,193,192},{477,30,193,192},{701,30,192,192},{926,29,193,193},{1153,28,194,194},
                {29,248,193,193},{254,249,194,193},{476,249,194,193},{701,249,193,194},{925,248,194,194},{1154,248,192,194},
                {29,469,198,197},{252,469,198,197},{474,469,197,197},{701,470,194,196},{924,469,197,197},{1150,469,196,196},
                {28,691,198,197},{252,690,198,198},{473,690,199,200},{699,691,197,199},{921,690,199,198},{1149,691,198,198},
                {24,911,201,199},{251,910,199,199},{473,911,198,198},{697,911,197,199},{921,910,199,199},{1150,911,197,199}
            }};
            for(int stage=0;stage<5;++stage)for(int head=0;head<6;++head) {
                if(stage!=wantedStage)continue;
                const int wearStage=juce::jmin(stage,3);
                auto image=finishes[size_t(wearStage)][size_t(head)].createCopy();
                juce::Graphics photo(image);photo.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                juce::Path face;face.addEllipse(8,8,80,80);photo.reduceClipRegion(face);
                photo.drawImage(atlas.getClippedImage(crops[size_t(wearStage*6+head)]),{4,4,88,88},juce::RectanglePlacement::stretchToFit);
                if(stage>0) {
                    // Re-tone only warm corrosion. Keep the photographed
                    // relief, pores, slots and surviving steel unchanged.
                    juce::Image::BitmapData pixels(image,juce::Image::BitmapData::readWrite);
                    for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                        const auto original=pixels.getPixelColour(x,y);
                        const float r=original.getFloatRed(),green=original.getFloatGreen(),b=original.getFloatBlue();
                        const float warm=(r-green)/juce::jmax(.025f,r);
                        const float corrosion=kb09_gui::power_detail::smoothRange(.08f,.30f,warm)
                            *kb09_gui::power_detail::smoothRange(.06f,.20f,r);
                        if(corrosion<=0)continue;
                        const float field=kb09_gui::power_detail::smoothNoise(x*.11f,y*.11f,11279u+uint32_t(head)*7919u);
                        const float retainRed=kb09_gui::power_detail::smoothRange(.67f,.77f,field)*.72f;
                        const float luminance=.2126f*r+.7152f*green+.0722f*b;
                        const float oxide=.045f+luminance*.57f;
                        const auto dark=juce::Colour::fromFloatRGBA(oxide*.96f,oxide*.99f,oxide,original.getFloatAlpha());
                        pixels.setPixelColour(x,y,original.interpolatedWith(dark,corrosion*(1-retainRed)));
                    }
                }
                // Crop off the thin rolled metal lip, retaining only a compact
                // contact shadow outside the rimless photographic face.
                const auto source=image.createCopy();
                juce::Image::BitmapData input(source,juce::Image::BitmapData::readOnly);
                juce::Image::BitmapData finish(image,juce::Image::BitmapData::writeOnly);
                const float tarnish=gentle?float(stage)*.25f*std::pow(.75f,1.35f):std::pow(float(wearStage)*.25f,1.35f);
                for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                    auto original=input.getPixelColour(x,y);
                    const float radius=std::hypot(x+.5f-48,y+.5f-48);
                    if(radius>=40.f) {
                        // The source photograph contains a larger old head here.
                        // Replace it with a clean contact shadow; never scale its rim.
                        const float alpha=.24f*(1.f-kb09_gui::power_detail::smoothRange(40.f,44.f,radius));
                        finish.setPixelColour(x,y,juce::Colours::black.withAlpha(alpha));continue;
                    }
                    if(radius>36.f) {
                        const float inward=(radius-5.f)/radius;
                        const int sx=juce::jlimit(0,95,int(48+(x+.5f-48)*inward));
                        const int sy=juce::jlimit(0,95,int(48+(y+.5f-48)*inward));
                        original=original.interpolatedWith(input.getPixelColour(sx,sy),kb09_gui::power_detail::smoothRange(36.f,38.5f,radius));
                    }
                    const float light=original.getBrightness();
                    float local=0;
                    for(int yy:{-2,0,2})for(int xx:{-2,0,2})local+=input.getPixelColour(juce::jlimit(0,95,x+xx),juce::jlimit(0,95,y+yy)).getBrightness()/9.f;
                    const float grain=juce::jlimit(-.010f,.010f,(light-local)*.12f);
                    const float shadow=kb09_gui::power_detail::smoothRange(.015f,.18f,light);
                    const float value=juce::jlimit(0.f,1.f,(.10f+.13f*light+grain+.009f*(96-x-y)/48.f)*shadow);
                    const float redRemnant=juce::jlimit(0.f,.12f,original.getFloatRed()-original.getFloatGreen());
                    const auto matte=juce::Colour::fromFloatRGBA(value*(.96f+redRemnant),value*.98f,value,original.getFloatAlpha());
                    auto aged=original.interpolatedWith(matte,tarnish);
                    if(stage==4&&!gentle) {
                        // Fire Hazard inherits the previous Sold As-Is surface
                        // and sockets. Compress metallic highlights into diffuse
                        // charcoal without pushing it to the former black endpoint.
                        const float brightness=aged.getBrightness();
                        const float diffuse=brightness<=.22f?brightness:
                            .22f+.055f*(1.f-std::exp(-(brightness-.22f)/.09f));
                        if(brightness>.0001f)aged=aged.withMultipliedBrightness(diffuse/brightness);
                    }
                    finish.setPixelColour(x,y,aged);
                }
                result[size_t(stage)][size_t(head)]=image;
            }
            return result;
        };
        // Only the active age/finish is needed on first paint. Legacy review
        // variants and other age rows are prepared only if actually requested.
        using ScrewStages=std::array<std::array<juce::Image,6>,5>;
        static std::array<ScrewStages,2> photographic;
        auto& active=photographic[gentler77?1:0];
        const int requested=kb09_gui::appearanceStage(age);
        if(!active[size_t(requested)][0].isValid()) {
            auto prepared=makePhotographic(gentler77,requested);
            active[size_t(requested)]=std::move(prepared[size_t(requested)]);
        }
        selected=&active;
    }
    const auto index=juce::jlimit(0,5,int((seed-5011u)/7717u));
    const auto stage=juce::jlimit(0,4,kb09_gui::appearanceStage(age));
    const juce::Image* finalImage=&(*selected)[size_t(stage)][size_t(index)];
    if(socketPreview&&lighterGrey) {
        const auto makeLowerPower=[&] {
            std::array<juce::Image,5> result;
            for(int condition=0;condition<5;++condition) {
                if(condition!=stage)continue;
                auto image=(*selected)[size_t(condition)][3].createCopy();
                juce::Image::BitmapData pixels(image,juce::Image::BitmapData::readWrite);
                for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                    if(std::hypot(x+.5f-48,y+.5f-48)>=39.5f)continue;
                    const auto colour=pixels.getPixelColour(x,y);const float light=colour.getBrightness();
                    const float face=kb09_gui::power_detail::smoothRange(.09f,.22f,light);
                    const float grey=(colour.getFloatRed()+colour.getFloatGreen()+colour.getFloatBlue())/3;
                    const auto neutral=juce::Colour::fromFloatRGBA(grey,grey,grey,colour.getFloatAlpha());
                    const auto base=colour.interpolatedWith(neutral,.45f*face);
                    const float lift=.16f*(1.f-light)*face;
                    pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(juce::jmin(1.f,base.getFloatRed()+lift),
                        juce::jmin(1.f,base.getFloatGreen()+lift),juce::jmin(1.f,base.getFloatBlue()+lift),colour.getFloatAlpha()));
                }
                result[size_t(condition)]=image;
            }
            return result;
        };
        static std::array<std::array<juce::Image,5>,2> lowerPowerSets;
        auto& lowerPower=lowerPowerSets[gentler77?1:0];
        if(!lowerPower[size_t(stage)].isValid()) {
            auto prepared=makeLowerPower();lowerPower[size_t(stage)]=std::move(prepared[size_t(stage)]);
        }
        finalImage=&lowerPower[size_t(stage)];
    }
    if(revised73) {
        // Flatten the broad light falloff while preserving photographic grain
        // and the deep Phillips slots. Bay heads have a shallower crown.
        static std::array<juce::Image,240> flatter;
        auto& cached=flatter[size_t((gentler77?120:0)+(bay73?60:0)+(lighterGrey?30:0)+stage*6+index)];
        if(!cached.isValid()) {
            cached=finalImage->createCopy();
            juce::Image::BitmapData input(*finalImage,juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData output(cached,juce::Image::BitmapData::writeOnly);
            for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                auto colour=input.getPixelColour(x,y);
                const float b=colour.getBrightness(),r=std::hypot(x+.5f-48,y+.5f-48);
                if(r>=39.f||b<.055f)continue;
                float broad=0;
                for(int yy:{-8,-4,0,4,8})for(int xx:{-8,-4,0,4,8})
                    broad+=input.getPixelColour(juce::jlimit(0,95,x+xx),juce::jlimit(0,95,y+yy)).getBrightness()/25.f;
                const float mask=kb09_gui::power_detail::smoothRange(.055f,.23f,b)
                    *(1.f-kb09_gui::power_detail::smoothRange(35.f,39.f,r));
                const float amount=(bay73?.48f:.27f)*(1.f-(gentler77?stage*.75f:float(stage))*.22f);
                float value=b+amount*(.40f-broad)*mask;
                if(stage==0) {
                    // Mint steel approaches Seen Some Shit's restrained shine.
                    value*=.94f;
                    if(value>.58f)value=.58f+.13f*(1.f-std::exp(-(value-.58f)/.15f));
                }
                output.setPixelColour(x,y,colour.withMultipliedBrightness(juce::jmax(0.f,value)/b));
            }
        }
        finalImage=&cached;
        if(!bay73&&!proposal84) {
            static std::array<juce::Image,30> cleanPaint;
            auto& patch=cleanPaint[size_t(stage*6+index)];
            if(!patch.isValid()) {
                const auto& art=artwork();
                const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
                    juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),juce::roundToInt(art.chassis.getHeight()*873.0/887));
                juce::Image nearby(juce::Image::ARGB,160,160,true,juce::SoftwareImageType());
                {juce::Graphics pg(nearby);pg.addTransform(juce::AffineTransform::scale(4.f));
                 pg.drawImage(art.chassis,20-c.x,20-c.y,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
                 if(age>0)pg.drawImage(art.panelWear[size_t(stage)],20-c.x,20-c.y,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());}
                patch=juce::Image(juce::Image::ARGB,112,112,true,juce::SoftwareImageType());
                juce::Image::BitmapData input(nearby,juce::Image::BitmapData::readOnly),output(patch,juce::Image::BitmapData::writeOnly);
                for(int y=0;y<112;++y)for(int x=0;x<112;++x) {
                    const float dx=(x+.5f)/4.f-14,dy=(y+.5f)/4.f-14,r=std::hypot(dx,dy);
                    const float angle=std::atan2(dy,dx);
                    float red=0,green=0,blue=0;
                    for(int offset=-2;offset<=2;++offset) {
                        const float theta=angle+offset*.14f;
                        const int sx=juce::jlimit(0,159,int(80+std::cos(theta)*62)),sy=juce::jlimit(0,159,int(80+std::sin(theta)*62));
                        const auto col=input.getPixelColour(sx,sy);red+=col.getFloatRed()/5;green+=col.getFloatGreen()/5;blue+=col.getFloatBlue()/5;
                    }
                    const float alpha=1.f-kb09_gui::power_detail::smoothRange(12.f,14.f,r);
                    output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(red,green,blue,alpha));
                }
            }
            g.drawImage(patch,{c.x-14,c.y-14,28,28});
        }
    }
    if(proposal84) {
        // Reshape the photographed socket, retaining its bevels and tool marks.
        // Only the socket region turns; the face lighting and aged finish stay put.
        static std::array<juce::Image,240> sockets;
        auto& socket=sockets[size_t((gentler77?120:0)+(bay73?60:0)+(lighterGrey?30:0)+stage*6+index)];
        if(!socket.isValid()) {
            socket=finalImage->createCopy();
            constexpr std::array<float,6> angles{{-28.f,11.f,42.f,-9.f,27.f,-39.f}};
            const float turn=juce::degreesToRadians(angles[size_t(index)]-(index%2?45.f:0.f));
            const float ca=std::cos(turn),sa=std::sin(turn);
            juce::Image::BitmapData input(*finalImage,juce::Image::BitmapData::readOnly),out(socket,juce::Image::BitmapData::writeOnly);
            for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                const float dx=x+.5f-48,dy=y+.5f-48,r=std::hypot(dx,dy);
                if(r>=32.f)continue;
                const float widen=1.f+.46f*std::exp(-r*r/150.f);
                const float u=(dx*ca+dy*sa)/widen+47.5f,v=(dy*ca-dx*sa)/widen+47.5f;
                const int ix=juce::jlimit(0,94,int(std::floor(u))),iy=juce::jlimit(0,94,int(std::floor(v)));
                const float fx=juce::jlimit(0.f,1.f,u-ix),fy=juce::jlimit(0.f,1.f,v-iy);
                const auto a=input.getPixelColour(ix,iy).interpolatedWith(input.getPixelColour(ix+1,iy),fx);
                const auto b=input.getPixelColour(ix,iy+1).interpolatedWith(input.getPixelColour(ix+1,iy+1),fx);
                out.setPixelColour(x,y,input.getPixelColour(x,y).interpolatedWith(a.interpolatedWith(b,fy),1.f-kb09_gui::power_detail::smoothRange(26.f,32.f,r)));
            }
        }
        finalImage=&socket;
    }
    if(proposal84&&!bay73&&index==0) {
        // This head sits in the bright upper-left portion of the panel.
        // Lift its diffuse illumination without painting over the socket,
        // changing the silhouette, or restoring gloss to oxidised steel.
        static std::array<juce::Image,20> brighter;
        auto& lit=brighter[size_t((revised73?10:0)+(gentler77?5:0)+stage)];
        if(!lit.isValid()) {
            lit=finalImage->createCopy();juce::Image::BitmapData pixels(lit,juce::Image::BitmapData::readWrite);
            for(int y=0;y<96;++y)for(int x=0;x<96;++x) {
                const auto colour=pixels.getPixelColour(x,y);const float b=colour.getBrightness();
                const float dx=x+.5f-48.f,dy=y+.5f-48.f,r=std::hypot(dx,dy);
                const float mask=kb09_gui::power_detail::smoothRange(.06f,.20f,b)*(1.f-kb09_gui::power_detail::smoothRange(35.f,39.f,r));
                if(mask<=0||b<=0)continue;
                const float key=juce::jlimit(0.f,1.f,(-dx-dy)/55.f);
                const float value=b+(b*(.18f+.10f*key)+.012f)*mask;
                pixels.setPixelColour(x,y,colour.withMultipliedBrightness(value/b));
            }
        }
        finalImage=&lit;
    }
    juce::Graphics::ScopedSaveState material(g);g.setOpacity(1.f);
    if(proposal84&&!bay73) {
        // Clean paint above was reconstructed at the ORIGINAL baked-head site.
        // Move/resize only the new head, so neither its old rim nor a scaled
        // circle of surrounding paint can survive at the original position.
        const float moveX=index==4?5.f*float(designW)/1770.f:index==5?-6.f*float(designW)/1770.f:0.f;
        // Keep the upper pair wholly inside the surviving 54..77 strip.
        // Its former centre left the heads touching the compacted top edge.
        const float moveY=reposition131?(index<2?8.f:index<4?-24.f:6.f):0.f;
        g.addTransform(juce::AffineTransform::translation(-c.x,-c.y)
            .scaled(.8f,.8f*panelViewCos).translated(c.x+moveX,c.y+moveY));
    }
    if(previewRotation&&!socketPreview) {
        constexpr std::array<float,6> rotations{{-16.f,0.f,-16.f,-4.f,-28.f,-2.f}};
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rotations[size_t(index)]),c.x,c.y));
    }
    const float half=revised73?10.8f:12.f;
    g.drawImage(*finalImage,{c.x-half,c.y-half,half*2,half*2},juce::RectanglePlacement::stretchToFit);
}

class ChoiceKnobDisplay {
public:
    virtual ~ChoiceKnobDisplay()=default;
    virtual double displayedChoiceProportion()=0;
};
class HardwareLook final : public kb09_gui::DialogTheme {
public:
    bool printWearEnabled() const override {return false;}
    bool conditionDragging=false,renderingPreview=false,ageIsDeferred=false;
    kb09_gui::AgeRasterCache faderAgeCache{128u*1024u*1024u};
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
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override {
        auto layout=juce::LookAndFeel_V4::getSliderLayout(slider);
        if constexpr(KB09_APPROVED_HARDWARE)if(slider.getSliderStyle()==juce::Slider::LinearVertical) {
            // The physical slot continues past the numbered scale at each end.
            // Cap travel and its input mapping follow these inset scale ends.
            const int inset=juce::roundToInt(layout.sliderBounds.getHeight()*.09f);
            layout.sliderBounds.reduce(0,inset);
        }
        return layout;
    }
    // Printed scales belong to the panel, so their outer numerals and
    // shared strokes are neither clipped by hit areas nor redrawn with caps.
    void drawFaderScale(juce::Graphics& g,juce::Slider& slider) {
        const auto bounds=getSliderLayout(slider).sliderBounds.translated(slider.getX(),slider.getY());
        // Printing is compacted separately in Y; retain the component's
        // fractional X translation so ticks and values follow the cap axis.
        const float cx=float(bounds.getCentreX())+slider.getTransform().mat02;
        const int y=bounds.getY(),h=bounds.getHeight();
        // Every unshared outer tick uses the same span and numeral gap,
        // including ADSR and the performance-bay faders.
        constexpr float shortSpan=18.f,numberShift=3.5f;
        const auto ink=agedInk(age);
            for(int t=0;t<=10;++t) {
                const float yy=float(y)+float(h)*float(t)/10;
                const bool major=t%5==0;
                const float left=float(slider.getProperties().getWithDefault("tickLeftSpan",shortSpan))+(major&&bool(slider.getProperties().getWithDefault("numbersLeft",true))?2.f:0.f);
                const float right=float(slider.getProperties().getWithDefault("tickRightSpan",shortSpan))+(major&&bool(slider.getProperties().getWithDefault("numbersRight",true))?2.f:0.f);
                g.setColour(ink.withAlpha(major?.90f:.73f));
                g.drawLine(cx-left,yy,cx-4.8f,yy,major?1.25f:.80f);
                g.drawLine(cx+4.8f,yy,cx+right,yy,major?1.25f:.80f);
            }
            for(int i=0;i<3;++i) {
                const float yy=float(y)+float(h)*float(i)*.5f;
                if(bool(slider.getProperties().getWithDefault("numbersLeft",true)))
                    caption(g,juce::String(10-i*5),{cx-29.0f-numberShift,yy-4,11,8},6.6f,juce::Justification::centredRight,false,ink);
                if(bool(slider.getProperties().getWithDefault("numbersRight",true)))
                    caption(g,juce::String(10-i*5),{cx+18.0f+numberShift,yy-4,11,8},6.6f,juce::Justification::centredLeft,false,ink);
            }
    }
    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,
                          const juce::Slider::SliderStyle style,juce::Slider& slider) override {
        LivePaintScope timing{"fader"};
        if(style!=juce::Slider::LinearVertical||bool(slider.getProperties().getWithDefault("disableFaderAgeCache186",false))) {
            paintLinearSlider(g,x,y,w,h,pos,style,slider);return;
        }
        const juce::Rectangle<int> region(-30,-14,slider.getWidth()+60,slider.getHeight()+28);
        if(!g.clipRegionIntersects(region))return;
        const float scale=juce::jlimit(1.f,4.f,g.getInternalContext().getPhysicalPixelScaleFactor());
        auto key=slider.getComponentID()+":"+juce::String(age,7)+":"+juce::String(scale,4)+":";
        for(const int value:{x,y,w,h,slider.getWidth(),slider.getHeight(),int(slider.getProperties().getWithDefault("compact",false))})key+=juce::String(value)+":";
        key+=juce::String(pos,5);
        auto image=faderAgeCache.find(key);
        if(!image.isValid()) {
            image=juce::Image(juce::Image::ARGB,juce::roundToInt(region.getWidth()*scale),juce::roundToInt(region.getHeight()*scale),true,juce::SoftwareImageType());
            {juce::Graphics raster(image);raster.addTransform(juce::AffineTransform::scale(scale).translated(-region.getX()*scale,-region.getY()*scale));paintLinearSlider(raster,x,y,w,h,pos,style,slider);}
            faderAgeCache.put(key,image);
            slider.getProperties().set("faderRasterCount",int(slider.getProperties().getWithDefault("faderRasterCount",0))+1);
        }
        drawNativeRaster(g,image,scale,region.getTopLeft().toFloat());
    }
    void paintLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,
                          const juce::Slider::SliderStyle style,juce::Slider& slider) {
        if(!g.clipRegionIntersects(juce::Rectangle<int>(-30,-14,slider.getWidth()+60,slider.getHeight()+28)))return;
        const bool utility=slider.getY()>=headerH+visibleBodyH;
        const juce::ScopedValueSetter<float> materialAge(age,utility?0.f:age);
        const auto ink=agedInk(age);
        const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f-((style==juce::Slider::LinearHorizontal&&utility)?4.f:0.f);
        if(style==juce::Slider::LinearVertical) {
            const bool compact=bool(slider.getProperties().getWithDefault("compact",false));
            // Use the original slot bounds: only the scale and cap travel
            // are inset, not the physical opening or its dust cover.
            const auto slot=juce::LookAndFeel_V4::getSliderLayout(slider).sliderBounds;
            const auto rim=juce::Rectangle<float>(cx-4.8f,float(slot.getY())-2,9.6f,float(slot.getHeight())+4);
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
            g.drawLine(cx-4.5f,float(slot.getY())+4,cx-4.5f,float(slot.getBottom())-4,.45f);
            // The silver index spans centreY-1.8 .. centreY+.45. Align its
            // midpoint, rather than the plastic silhouette, to the value tick.
            const float capY=KB09_APPROVED_HARDWARE?pos-10.825f:pos-10.f;
            const auto capBounds=juce::Rectangle<float>(cx-(compact?12.5f:15),capY,compact?25.0f:30.0f,23);
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
                constexpr float radius=60.0f;
                const bool fitBender=bool(slider.getProperties().getWithDefault("previewBenderFit19",true));
                const bool showUnderside=bool(slider.getProperties().getWithDefault("previewBenderRib21",true));
                const bool concentricRib=bool(slider.getProperties().getWithDefault("previewConcentricRib22",true));
                const bool parallelRib=bool(slider.getProperties().getWithDefault("previewParallelRib23",true));
                const bool bevelBreak=bool(slider.getProperties().getWithDefault("previewBrokenBevel26",true));
                const bool continuousFront=bool(slider.getProperties().getWithDefault("previewContinuousFront27",true));
                const bool continuousFinish=bool(slider.getProperties().getWithDefault("previewContinuousBenderFinish29",true));
                const bool joinedPointer=bool(slider.getProperties().getWithDefault("previewBenderJoinedRoot30",true));
                const bool raisedSeam=bool(slider.getProperties().getWithDefault("previewBenderSeam31",true));
                const bool gentleBenderFinish=continuousFinish||bool(slider.getProperties().getWithDefault("previewBenderSheen28",false));
                const float ageGloss=gentleBenderFinish?1.f-.18f*age:1.f;
                const bool curvedBreak=bevelBreak||bool(slider.getProperties().getWithDefault("previewBrokenContour25",true));
                const float rootWidth=fitBender?8.f:7.f;
                constexpr float rearRound=.40f,frontRound=1.15f;
                const bool broken=age>=1.f;
                const bool matchedDisc=bool(slider.getProperties().getWithDefault("previewMatchedDisc17",true));
                const bool recessedBreak=broken&&bool(slider.getProperties().getWithDefault("previewDiscBreak",true));
                const float seamWidth=(recessedBreak?.28f:.48f)*(raisedSeam?1.125f:1.f);
                const float depth=fitBender?16.f:matchedDisc?17.f:recessedBreak?kb09_gui::benderPreviewDepth:13.5f;
                // A thin central web carries a substantially thicker molded rim.
                const auto frontDepth=[recessedBreak,depth](juce::Point<float> p) {
                    if(recessedBreak)return kb09_gui::benderDiscAxle(p,1.f,depth);
                    const float t=juce::jlimit(0.0f,1.0f,(p.getDistanceFromOrigin()-51.0f)/5.0f);
                    return depth*(.4f+.6f*t*t*(3-2*t));
                };
                const float wheelBaseline=13.0f+radius*panelViewSin-(matchedDisc?2.6f:recessedBreak?5.2f:0.f);
                const bool previewDust=broken&&bool(slider.getProperties().getWithDefault("previewFractureDust",true));
                const auto project=[centre,wheelBaseline,sa,ca](juce::Point<float> p,float axleDepth) {
                    const float xx=p.x*ca+p.y*sa,zz=p.y*ca-p.x*sa;
                    return juce::Point<float>(centre+xx,wheelBaseline-zz*panelViewSin+axleDepth*panelViewCos);
                };
                constexpr int circleSegments=256,vertexCapacity=circleSegments+64;
                std::array<juce::Point<float>,vertexCapacity> outline{};
                int count=0;
                // The healthy disc continues beneath the molded pointer.
                // Cutting out a wedge as wide as the pointer leaves a hole
                // beside its rounded root, especially at either bend extreme.
                const float rootAngle=std::asin((broken?21.0f:joinedPointer?0.f:rootWidth)/radius),rootHeight=std::sqrt(radius*radius-rootWidth*rootWidth);
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
                    if(recessedBreak)for(const auto p:kb09_gui::recessedBenderBreak())outline[size_t(count++)]=p;
                    else for(const auto p:tab)outline[size_t(count++)]=p;
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
                    return juce::Point<float>((surfaceNoise(seed)-.5f)*(recessedBreak?1.2f:4.f)*taper,
                                              (surfaceNoise(seed+137u)-.5f)*1.7f*taper);
                };
                const auto surfacePoint=[&](int i,float t) {
                    auto local=outline[size_t(i)];
                    if(recessedBreak) {
                        local=i<=circleSegments?kb09_gui::benderBrokenArc(i,circleSegments,t,t>0?frontRound:rearRound):
                            kb09_gui::benderFractureLocal(local,i-circleSegments,t,t>0?frontRound:rearRound,depth);
                        const float axle=kb09_gui::benderDiscAxle(local,t,depth);
                        if(curvedBreak&&i>circleSegments)local=kb09_gui::constrainBenderFractureToRim(local,axle,depth,angle);
                        return project(local,axle);
                    }
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
                // The wheel projects in front of the upper aperture edge. Clip
                // to the component above it, not to the opening's back plane.
                const int clipTop=0;
                g.reduceClipRegion(juce::Rectangle<int>(int(well.getX()),clipTop,int(well.getWidth()),int(well.getBottom())-clipTop));
                {
                    juce::Graphics::ScopedSaveState shadowLimit(g);
                    if(bevelBreak&&recessedBreak) {
                        // The fracture's translated cast shadow used to hang
                        // over the front rim even after its mesh was trimmed.
                        // End that shadow at the same continuous visible curve.
                        const float r=radius-frontRound;
                        juce::Path limit;limit.startNewSubPath(centre-r,0);limit.lineTo(centre+r,0);
                        for(int j=0;j<=256;++j) {
                            const float x=r-2*r*float(j)/256.f;
                            limit.lineTo(centre+x,wheelBaseline+depth*panelViewCos-std::sqrt(juce::jmax(0.f,r*r-x*x))*panelViewSin);
                        }
                        limit.closeSubPath();g.reduceClipRegion(limit);
                    }
                    if constexpr(KB09_APPROVED_HARDWARE) {
                        // Both the wheel and raised pointer are in these rigid
                        // silhouettes, so their cast shadow follows every pose.
                        juce::Path caster=rear;caster.addPath(front);caster.setUsingNonZeroWinding(true);
                        kb09_gui::hardwarePathShadow(g,caster,.6f);
                    } else {
                        g.setColour(juce::Colours::black.withAlpha(.40f));g.fillPath(rear,juce::AffineTransform::translation(.7f,1.3f));
                        g.fillPath(front,juce::AffineTransform::translation(.7f,1.3f));
                    }
                }
                g.setColour(juce::Colour(0xff080b0b));g.fillPath(rear);
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff141615).darker(age*.20f),centre-45,16,
                                                     juce::Colour(0xff050606),centre+45,well.getBottom(),false));g.fillPath(front);
                // Smooth cylinder shading is evaluated per sample, rather than
                // giving each mesh strip its own flat colour and antialias seam.
                // The inverse projection uses the same rigid axle/camera above.
                const auto plasticLight=[age=age,recessedBreak,matchedDisc,ageGloss,continuousFinish](float nx,float ny,float nz,float axial) {
                    const float diffuse=juce::jmax(0.0f,-.270f*nx+.090f*ny+.959f*nz);
                    const float half=juce::jlimit(0.0f,1.0f,-.137f*nx+.125f*ny+.982f*nz);
                    const float fill=std::pow(juce::jmax(0.0f,.62f*std::abs(nx)+.78f*nz),7.0f);
                    // Keep the existing broad highlight in the same place. Shape
                    // its boundary instead of adding another reflected light.
                    const float specular=kb09_gui::polishedBenderHighlight(half,recessedBreak&&!continuousFinish?age*.22f:age);
                    const float lightShape=.70f+.30f*std::exp(-((axial-1.2f)*(axial-1.2f))/1.8f);
                    const float rightReflection=(recessedBreak||matchedDisc)?.24f*std::exp(-std::pow((nx-.53f)/.12f,8.f))
                        *std::exp(-std::pow((axial-.95f)/.55f,8.f)):0.f;
                    return .068f+.064f*diffuse+.090f*fill+ageGloss*(.95f*specular*lightShape*(1-age*(recessedBreak&&!continuousFinish?.12f:.42f))
                        +(recessedBreak&&!matchedDisc?.42f*kb09_gui::benderHardReflection(nx,ny,nz,age*.2f):0.f)+rightReflection)+age*.008f;
                };
                const int samples=conditionDragging?1:juce::jlimit(1,4,int(std::ceil(g.getInternalContext().getPhysicalPixelScaleFactor())));
                // Include the whole projected crown when the wheel rises above
                // the aperture. A fixed y=12 texture boundary cuts the material
                // off horizontally while its underlying geometry continues.
                const float rimTop=juce::jmin(12.f,wheelBaseline-radius*panelViewSin-.30f);
                const juce::Rectangle<float> rimBounds(centre-radius-1,rimTop,2*radius+2,
                                                       wheelBaseline+depth*panelViewCos+1.f-rimTop);
                juce::Image rim(juce::Image::ARGB,juce::roundToInt(rimBounds.getWidth()*samples),
                                juce::roundToInt(rimBounds.getHeight()*samples),true);
                {
                    juce::Image::BitmapData pixels(rim,juce::Image::BitmapData::writeOnly);
                    const auto& scuffs=kb09_gui::benderScuffs();
                    const juce::Image::BitmapData scuffPixels((fitBender?scuffs.keyRim:scuffs.rim)[size_t(kb09_gui::appearanceStage(age))],juce::Image::BitmapData::readOnly);
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
                            if(raisedSeam)r+=.075f*std::exp(-std::pow((axial-depth*.5f)/seamWidth,2.f));
                            if(std::abs(px)>=r)break;
                            z=std::sqrt(r*r-px*px);
                            axial=(py-wheelBaseline+z*panelViewSin)/panelViewCos;
                        }
                        if(std::abs(px)>=r||axial<-.18f||axial>depth+.18f)continue;
                        const float localAngle=std::atan2(px*ca-z*sa,z*ca+px*sa);
                        if(recessedBreak) {
                            const float boundary=std::asin(kb09_gui::benderBrokenShoulder(localAngle<0,axial/depth)/radius);
                            if(std::abs(localAngle)<boundary)continue;
                        } else if(std::abs(localAngle)<rootAngle)continue;
                        float radial=1,ny=0;
                        if(axial<rearRound)ny=juce::jlimit(-1.0f,0.0f,(axial-rearRound)/rearRound);
                        else if(axial>depth-frontRound)ny=juce::jlimit(0.0f,1.0f,(axial-(depth-frontRound))/frontRound);
                        if(raisedSeam) {
                            const float offset=axial-depth*.5f;
                            ny=juce::jlimit(-.99f,.99f,ny+.15f*offset/(seamWidth*seamWidth)*std::exp(-std::pow(offset/seamWidth,2.f)));
                        }
                        radial=std::sqrt(juce::jmax(0.0f,1-ny*ny));
                        float value=plasticLight(px/r*radial,ny,z/r*radial,axial);
                        if(age>0) {
                            const float scratch=kb09_gui::sampleBenderScuff(scuffPixels,(localAngle+juce::MathConstants<float>::pi)/juce::MathConstants<float>::twoPi,axial/depth);
                            const float grazingLight=juce::jmax(0.f,-.270f*px/r*radial+.090f*ny+.959f*z/r*radial);
                            value=value*(1-.75f*scratch)+(.18f+.25f*grazingLight)*scratch;
                        }
                        // Fine raised parting seam follows the molded disc around its circumference.
                        const float seam=(axial-depth*.50f)/seamWidth;
                        const float litArc=std::pow(juce::jmax(0.0f,-.30f*px/r+.954f*z/r),10.0f);
                        value+=((recessedBreak?.21f:.105f)*(raisedSeam?1.04f:1.f)*std::exp(-seam*seam)
                            -(recessedBreak?.085f:.045f)*(raisedSeam?1.12f:1.f)*std::exp(-(seam-1.15f)*(seam-1.15f)))
                            *(1-age*(recessedBreak?.10f:.38f))*(recessedBreak?juce::jmax(.35f,litArc):litArc);
                        const auto grainKey=uint32_t(int((localAngle+juce::MathConstants<float>::pi)*380))
                                           +uint32_t(juce::jmax(0,int(axial*6)))*1177u;
                        value+=(surfaceNoise(0x3a71u+grainKey)-.5f)*(.005f+age*.007f);
                        const float coverage=juce::jlimit(0.0f,1.0f,(juce::jmin(axial,depth-axial)+.15f)/.30f);
                        value=juce::jlimit(0.f,1.f,value);
                        pixels.setPixelColour(xx,yy,juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,coverage));
                    }
                }
                if(recessedBreak) {
                    // The photograph shows the recessed body beneath the web,
                    // with a dusty shelf between them, rather than a black void
                    // or a separate round raised rib.
                    const auto lowerArc=[&](float r,float axle) {
                        juce::Path path;
                        for(int i=0;i<=96;++i) {
                            const float x=-27.f+54.f*float(i)/96.f;
                            const auto p=project({x,std::sqrt(r*r-x*x)},axle);
                            if(i==0)path.startNewSubPath(p);else path.lineTo(p);
                        }
                        return path;
                    };
                    const float webCentre=depth*.5f-1.70f;
                    const float backingEnd=webCentre-depth*.125f+.6f;
                    const float shelfAxle=webCentre+depth*.125f+1.5f;
                    // The inner cylinder is set below the outer crown. Its own
                    // curved top edge and the shadow behind the raised web make
                    // the setback visible; it must not bridge flush to the rim.
                    juce::Path backing=lowerArc(53.f,.30f);
                    for(int i=96;i>=0;--i) {
                        const float x=-27.f+54.f*float(i)/96.f;
                        backing.lineTo(project({x,std::sqrt(53.f*53.f-x*x)},backingEnd));
                    }
                    backing.closeSubPath();
                    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff303730),centre-15,15,
                        juce::Colour(0xff171d18),centre+18,19,false));g.fillPath(backing);
                    g.setColour(juce::Colour(0xff727b70).withAlpha(.17f));
                    g.strokePath(lowerArc(53.f,.30f),juce::PathStrokeType(.16f));
                    {
                        juce::Graphics::ScopedSaveState grainClip(g);g.reduceClipRegion(backing);
                        for(uint32_t grain=0;grain<(previewDust?3600u:240u);++grain) {
                            const uint32_t key=0x47b13u+grain*7919u;
                            const float x=-24.f+48.f*surfaceNoise(key);
                            // More deposits toward the sheltered foot of the
                            // raised broken web; sparse grains remain farther back.
                            const float t=previewDust?1.f-std::pow(1.f-surfaceNoise(key+117),2.2f):surfaceNoise(key+117);
                            const auto p=project({x,std::sqrt(53.f*53.f-x*x)},.30f+(backingEnd-.30f)*t);
                            const float size=.08f+(previewDust?.27f:.12f)*surfaceNoise(key+59);
                            g.setColour(juce::Colour(previewDust?(grain%5?0xffa99e82:0xff4d4d3f):(grain%4?0xff878b80:0xff101513))
                                .withAlpha(previewDust?(.28f+.46f*t):(.08f+age*.08f)));
                            g.fillEllipse(p.x,p.y,size,size*.7f);
                        }
                        // The web casts a narrow contact shadow onto this
                        // backing. The web itself is painted later, unchanged.
                        for(int i=circleSegments+1;i+1<count;++i) {
                            const auto a=outline[size_t(i)],b=outline[size_t(i+1)];
                            if(std::abs((a.x+b.x)*.5f)>11.5f||juce::jmax(a.y,b.y)>54.f)continue;
                            const auto p=surfacePoint(i,0),q=surfacePoint(i+1,0);
                            juce::Path shadow;shadow.startNewSubPath(p);shadow.lineTo(q);
                            shadow.lineTo(q.translated(-.10f,-.75f));shadow.lineTo(p.translated(-.10f,-.75f));shadow.closeSubPath();
                            g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.64f),p.x,p.y,
                                juce::Colours::black.withAlpha(.05f),p.x-.10f,p.y-.75f,false));g.fillPath(shadow);
                        }
                    }
                    juce::Path body=lowerArc(49.5f,shelfAxle);
                    for(int i=96;i>=0;--i) {
                        const float x=-27.f+54.f*float(i)/96.f;
                        // This is the exact outer-rim front contour. Sharing
                        // these vertices closes the join in every bend pose.
                        body.lineTo(project({x,std::sqrt((radius-frontRound)*(radius-frontRound)-x*x)},depth));
                    }
                    body.closeSubPath();
                    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff303531),centre-11,24,
                        juce::Colour(0xff242826),centre+18,33,false));g.fillPath(body);
                    g.setColour(juce::Colour(0xff838b85).withAlpha(.32f));g.strokePath(lowerArc(49.5f,shelfAxle+.05f),juce::PathStrokeType(.22f));
                    juce::Path shelf=lowerArc(54.5f,shelfAxle);
                    for(int i=96;i>=0;--i) {
                        const float x=-27.f+54.f*float(i)/96.f;
                        shelf.lineTo(project({x,std::sqrt(49.5f*49.5f-x*x)},shelfAxle));
                    }
                    shelf.closeSubPath();
                    g.setColour(juce::Colour(0xff262823));g.fillPath(shelf);
                    if(previewDust) {
                        juce::Graphics::ScopedSaveState deposit(g);g.reduceClipRegion(shelf);
                        for(uint32_t grain=0;grain<1400;++grain) {
                            const uint32_t key=0x9137u+grain*7919u;
                            const float x=-23.f+46.f*surfaceNoise(key);
                            const float r=49.5f+5.f*surfaceNoise(key+139);
                            const auto p=project({x,std::sqrt(r*r-x*x)},shelfAxle);
                            const float size=.10f+.30f*surfaceNoise(key+317);
                            g.setColour(juce::Colour(grain%4?0xffaaa087:0xff4c4a3f).withAlpha(.42f+.36f*surfaceNoise(key+59)));
                            g.fillEllipse(p.x,p.y,size,size*.65f);
                        }
                    }
                    {
                        // The raised rib shades the trough and body below it.
                        // Dust belongs on this receiver, not as a beige coating
                        // over the broken black plastic.
                        auto receiver=body;receiver.addPath(shelf);
                        juce::Graphics::ScopedSaveState shadowClip(g);g.reduceClipRegion(receiver);
                        for(int i=circleSegments+1;i+1<count;++i) {
                            const auto a=outline[size_t(i)],b=outline[size_t(i+1)];
                            if(std::abs((a.x+b.x)*.5f)>11.5f||juce::jmax(a.y,b.y)>54.f)continue;
                            const auto p=surfacePoint(i,1),q=surfacePoint(i+1,1);
                            juce::Path shadow;shadow.startNewSubPath(p.translated(.20f,.05f));shadow.lineTo(q.translated(.20f,.05f));
                            shadow.lineTo(q.translated(.45f,1.75f));shadow.lineTo(p.translated(.45f,1.75f));shadow.closeSubPath();
                            g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.58f),p.x+.20f,p.y+.05f,
                                juce::Colours::black.withAlpha(.04f),p.x+.45f,p.y+1.75f,false));g.fillPath(shadow);
                        }
                    }
                }
                g.drawImage(rim,rimBounds);
                if(recessedBreak) {
                    // Short, irregular facets across both dimensions replace
                    // full-depth vertical bands. The polished molded seam is
                    // drawn only on the intact curved rim above, never as a
                    // fracture stripe across that surface.
                    struct FractureVertex {juce::Point<float> local;float axle;};
                    const auto vertex=[&](int i,float t) {
                        auto p=i<=circleSegments?kb09_gui::benderBrokenArc(i,circleSegments,t,t>0?frontRound:rearRound):
                            kb09_gui::benderFractureLocal(outline[size_t(i)],i-circleSegments,t,t>0?frontRound:rearRound,depth);
                        const float axle=kb09_gui::benderDiscAxle(p,t,depth);
                        if(curvedBreak&&i>circleSegments)p=kb09_gui::constrainBenderFractureToRim(p,axle,depth,angle);
                        return FractureVertex{p,axle};
                    };
                    const auto paintFacet=[&](FractureVertex a,FractureVertex b,FractureVertex c,uint32_t key) {
                        const auto u=b.local-a.local,v=c.local-a.local;
                        const float ua=b.axle-a.axle,va=c.axle-a.axle;
                        float nx=ua*v.y-u.y*va,nz=u.x*va-ua*v.x,ny=u.y*v.x-u.x*v.y;
                        const float length=std::sqrt(nx*nx+nz*nz+ny*ny);if(length<.00001f)return;
                        nx/=length;nz/=length;ny/=length;
                        const float wx=nx*ca+nz*sa,wz=nz*ca-nx*sa;
                        if(wz*panelViewCos+ny*panelViewSin<=0)return;
                        const auto middle=(a.local+b.local+c.local)/3.f;
                        const bool inner=std::abs(middle.x)<11.5f&&middle.y<54.f;
                        const float diffuse=juce::jmax(0.f,-.27f*wx+.09f*ny+.959f*wz);
                        const float alignment=juce::jmax(0.f,-.137f*wx+.125f*ny+.982f*wz);
                        const float sheen=inner?.15f*kb09_gui::polishedBenderHighlight(alignment,.10f)
                            +.15f*kb09_gui::benderHardReflection(wx,ny,wz,.10f):.08f*std::pow(alignment,32.f);
                        const float underside=inner?juce::jlimit(0.f,1.f,((a.axle+b.axle+c.axle)/3.f-kb09_gui::benderDiscRear(middle,depth))/kb09_gui::benderDiscDepth(middle,depth)):0.f;
                        const float shadow=inner?1.f-.62f*kb09_gui::power_detail::smoothRange(.58f,.98f,underside):1.f;
                        const float ribLight=inner?.065f*diffuse*(1.f-kb09_gui::power_detail::smoothRange(.40f,.85f,underside)):0.f;
                        const float value=(.045f+.12f*diffuse+sheen*ageGloss+ribLight+.012f*surfaceNoise(key))*shadow;
                        auto colour=juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,1);
                        const auto pa=project(a.local,a.axle),pb=project(b.local,b.axle),pc=project(c.local,c.axle);
                        juce::Path face;face.startNewSubPath(pa);face.lineTo(pb);face.lineTo(pc);face.closeSubPath();
                        g.setColour(colour);g.fillPath(face);g.strokePath(face,juce::PathStrokeType(.08f));
                        juce::Graphics::ScopedSaveState material(g);g.reduceClipRegion(face);
                        for(uint32_t grain=0;grain<9u;++grain) {
                            const float s=std::sqrt(surfaceNoise(key+grain*7919u+13)),r=surfaceNoise(key+grain*113u+31);
                            const auto point=pa*(1-s)+pb*(s*(1-r))+pc*(s*r);
                            const float size=.08f+.19f*surfaceNoise(key+grain*317u);
                            const bool strayDust=previewDust&&inner&&grain==0;
                            g.setColour(juce::Colour(strayDust?0xffa69e89:0xff010202).withAlpha(strayDust?.10f:.25f));
                            g.fillEllipse(point.x,point.y,size,size*.7f);
                        }
                    };
                    for(int i=circleSegments;i<count;++i) {
                        const int next=(i+1)%count;
                        for(int band=0;band<6;++band) {
                            const float t0=float(band)/6.f,t1=float(band+1)/6.f;
                            const auto a=vertex(i,t0),b=vertex(next,t0),c=vertex(next,t1),d=vertex(i,t1);
                            const auto key=uint32_t(i)*104729u+uint32_t(band)*7919u;
                            paintFacet(a,b,c,key);paintFacet(a,c,d,key+391u);
                        }
                    }
                    // Tiny polished lips on individual chips catch the same
                    // upper-left light as the intact wheel. Keep them broken
                    // into short edge glints, not a stripe across the web.
                    for(int i=circleSegments+1;i+1<count;++i) {
                        const auto a=outline[size_t(i)],b=outline[size_t(i+1)],middle=(a+b)*.5f;
                        if(std::abs(middle.x)>=10.f||middle.y>=51.f)continue;
                        const auto delta=b-a;const float length=delta.getDistanceFromOrigin();
                        if(length<.0001f)continue;
                        const float nx=(-delta.y*ca+delta.x*sa)/length,nz=(delta.x*ca+delta.y*sa)/length;
                        const float glint=std::pow(juce::jmax(0.f,-.45f*nx+.89f*nz),8.f);
                        if(glint<.035f)continue;
                        const auto p=surfacePoint(i,.12f),q=surfacePoint(i+1,.12f);
                        const float dustAttenuation=(previewDust?.85f:1.f)*ageGloss;
                        g.setColour(juce::Colour(0xffedeee8).withAlpha(juce::jmin(.90f,glint*1.25f)*dustAttenuation));
                        g.drawLine({p+(q-p)*.10f,p+(q-p)*.79f},.25f);
                    }
                }
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
                    if(i<circleSegments||!broken||recessedBreak)continue;
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
                        // The exposed broken plastic catches the same studio
                        // light as the disc; varying normals reveal the jagged cut.
                        const float diffuse=juce::jmax(0.0f,-.54f*nx+.842f*nz);
                        const float value=fracture?.029f+.185f*diffuse*diffuse
                            +.024f*surfaceNoise(uint32_t(i)*917u)
                            +.52f*kb09_gui::polishedBenderHighlight(juce::jmax(0.f,-.137f*nx+.982f*nz),age*.25f)
                            +.45f*kb09_gui::benderHardReflection(nx,0,nz,age*.25f):plasticLight(nx,0,nz,axial)*(1+(!broken&&a.y>radius?.22f*(1-age):0.f));
                        auto colour=juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,1);
                        if(fracture)colour=colour.interpolatedWith(juce::Colour(0xff777777),.08f);
                        const bool insideBreak=fracture&&std::abs((a.x+b.x)*.5f)<11.5f&&juce::jmax(a.y,b.y)<54.f;
                        if(insideBreak&&previewDust) {
                            const float sheltered=1.f-juce::jlimit(0.f,1.f,std::abs((a.x+b.x)*.5f)/22.f);
                            const float deposit=(.28f+.34f*kb09_gui::power_detail::smoothNoise((a.x+b.x)*.20f,axial*.31f,0x78139u))*(.75f+.25f*sheltered);
                            const float dustLight=.23f+.20f*diffuse;
                            colour=colour.interpolatedWith(juce::Colour::fromFloatRGBA(dustLight,dustLight*.93f,dustLight*.80f,1),deposit);
                        }
                        return colour;
                    };
                    const auto middle=(a+b)*.5f,backPoint=project(middle,0),frontPoint=project(middle,depth);
                    juce::ColourGradient reflection(shade(0),backPoint.x,backPoint.y,shade(depth),frontPoint.x,frontPoint.y,false);
                    reflection.addColour(.22,shade(depth*.22f));reflection.addColour(.70,shade(depth*.70f));

                    g.setGradientFill(reflection);
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
                        if(previewDust&&std::abs((a.x+b.x)*.5f)<11.5f&&juce::jmax(a.y,b.y)<54.f)for(uint32_t grain=0;grain<76;++grain) {
                            const auto key=uint32_t(i)*104729u+grain*7919u+0x8913u;
                            const float along=surfaceNoise(key+13),across=std::pow(surfaceNoise(key+31),1.35f);
                            const auto local=a+(b-a)*along;
                            const auto shift=fractureOffset(i)+(fractureOffset(next)-fractureOffset(i))*along;
                            const auto point=project(local+shift*across,frontDepth(local)*across);
                            const float size=.10f+.30f*std::pow(surfaceNoise(key+47),1.7f);
                            g.setColour(juce::Colour(grain%4?0xffa69e89:0xff69685b).withAlpha(.35f+.48f*surfaceNoise(key+61)));
                            g.fillEllipse(point.x-size*.5f,point.y-size*.3f,size,size*.65f);
                            if(grain%23==0) {
                                g.setColour(juce::Colour(0xffaaa492).withAlpha(.45f));
                                g.drawLine(point.x,point.y,point.x+.30f+.55f*surfaceNoise(key+79),point.y+.20f,.065f);
                            }
                        }
                    }
                    // Overlap the antialias coverage with the identical material.
                    g.setGradientFill(reflection);
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
                if(continuousFront&&recessedBreak) {
                    // The front face is in front of the jagged cut surfaces.
                    // Paint it after those surfaces, with ONE ellipse for the
                    // entire disc. Fracture triangles must not overpaint this
                    // face and interrupt its smooth outer silhouette.
                    g.setColour(juce::Colour(0xff080b0b));
                    g.fillPath(channel(radius-frontRound,depth-.40f));
                }
                {
                    juce::Graphics::ScopedSaveState surface(g);g.reduceClipRegion(front);
                    // Recessed web and the rounded step under the thick rim.
                    if(!recessedBreak) {
                        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff343735).darker(age*.4f),centre,21,juce::Colour(0xff080b0b),centre,well.getBottom(),false));
                        g.fillPath(channel(54,frontDepth({0,54})));
                        g.setColour(juce::Colour(0xff090c0c));g.fillPath(channel(51.5f,frontDepth({0,51.5f})));
                    }
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
                    if(!recessedBreak||showUnderside) {
                        // Raised concentric moulding under the outer rim, with a
                        // circular cross-section and continuous surface normals.
                        juce::Graphics::ScopedSaveState ringClip(g);
                        if(!parallelRib)g.reduceClipRegion(channel(54,showUnderside?depth-(concentricRib?0.f:.6f):frontDepth({0,54})));
                        // Put both concentric circles on the same axial plane.
                        // Offsetting only the rib toward the back displaces its
                        // projected center and pinches the gap near the crown.
                        const float ringShift=showUnderside?(depth-(concentricRib?0.f:1.f)-10.8f):(depth-13.5f)*.8f;
                        if(parallelRib)kb09_gui::paintParallelBenderRib(g,centre,wheelBaseline,radius-frontRound,depth,age,samples,
                            bool(slider.getProperties().getWithDefault("previewThickRib24",true))?3.8f:1.3f,continuousFront);
                        else kb09_gui::paintBenderUndersideRing(g,centre,wheelBaseline+ringShift*panelViewCos,age,samples);
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
                if(!broken)kb09_gui::paintRoundedBenderTab(g,centre,wheelBaseline,angle,age,samples,depth,fitBender,fitBender,gentleBenderFinish,bool(KB09_APPROVED_HARDWARE));
            } else {
                // The track spans the cap's full travel envelope. Its visible
                // endpoints stay fixed even when a cap moves away from them.
                const float trackLeft=0,trackWidth=float(slider.getWidth());
                g.setColour(juce::Colour(0xff05080a));g.fillRoundedRectangle(trackLeft,cy-3,trackWidth,6,3);
                g.setColour(juce::Colours::white.withAlpha(.18f));g.drawLine(trackLeft,cy+3,trackLeft+trackWidth,cy+3,.6f);
                g.setColour(ink.withAlpha(.35f));g.fillRect(float(x),cy-1,juce::jmax(0.0f,pos-float(x)),1.5f);
                cap(g,{pos-8,cy-8,16,17},true);
            }
        }
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,
                          float start,float end,juce::Slider& slider) override {
        LivePaintScope timing{slider.getComponentID()=="range"?"range":slider.getComponentID()=="wave"?"wave":"other"};
        juce::Graphics::ScopedSaveState position(g);
        g.addTransform(juce::AffineTransform::translation(float(slider.getProperties().getWithDefault("knobOffsetX198",0.f)),0));
        if(!g.clipRegionIntersects(slider.getLocalBounds().expanded(5)))return;
        const auto area=juce::Rectangle<float>(float(x),float(y),float(w),float(h));
        const auto ink=agedInk(age);
        const auto c=area.getCentre();const float d=juce::jmin(float(w),float(h));
        // Choice knobs draw exact detents. No fractional display or animation.
        const bool discrete=slider.getComponentID()=="range"||slider.getComponentID()=="wave";
        auto* choiceDisplay=discrete?dynamic_cast<ChoiceKnobDisplay*>(&slider):nullptr;
        const float visualValue=choiceDisplay?float(choiceDisplay->displayedChoiceProportion()):discrete?float(slider.valueToProportionOfLength(std::round(slider.getValue()))):value;
        const float a=start+(end-start)*visualValue;
        if(slider.getComponentID()=="tune") {
            auto disc=juce::Rectangle<float>(c.x-d*.38f,c.y-d*.38f,d*.76f,d*.76f);
            if constexpr(KB09_APPROVED_HARDWARE) {
                juce::Graphics::ScopedSaveState shadow(g);
                g.setColour(juce::Colours::black.withAlpha(.07f));
                for(int i=5;i>0;--i)g.fillEllipse(disc.expanded(i*.45f*.6f).translated(1.2f,1.8f));
            } else {g.setColour(juce::Colours::black.withAlpha(.5f));g.fillEllipse(disc.expanded(1).translated(1,2));}
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff565b5c),disc.getX(),disc.getY(),
                                                juce::Colour(0xff141719),disc.getRight(),disc.getBottom(),false));
            g.fillEllipse(disc);
            g.setColour(juce::Colour(0xff0b0d0e));g.drawEllipse(disc,.8f);
            if(bool(slider.getProperties().getWithDefault("previewTuneRim163",bool(KB09_APPROVED_HARDWARE)))) {
                // A fixed top-left reflection on the existing rim, without
                // changing its shape or the dust inside its recessed center.
                for(int segment=0;segment<96;++segment) {
                    const float theta=juce::MathConstants<float>::twoPi*float(segment)/96.f;
                    const float next=juce::MathConstants<float>::twoPi*float(segment+1)/96.f;
                    const float middle=(theta+next)*.5f;
                    const float key=juce::jmax(0.f,-(std::cos(middle)+std::sin(middle))*.70710678f);
                    const float sheen=std::pow(key,10.f)*(1-age*.65f);
                    if(sheen<.002f)continue;
                    const auto point=[&](float angle,float radius){return c+juce::Point<float>(std::cos(angle),std::sin(angle))*d*radius;};
                    juce::Path edge;edge.startNewSubPath(point(theta,.347f));edge.lineTo(point(theta,.369f));edge.lineTo(point(next,.369f));edge.lineTo(point(next,.347f));edge.closeSubPath();
                    g.setColour(juce::Colour(0xffd4dcd9).withAlpha(sheen*.90f));g.fillPath(edge);
                }
            }
            const auto bowl=disc.reduced(d*.09f);
            const bool packedTuneDust=bool(slider.getProperties().getWithDefault("previewTuneDust155",bool(KB09_APPROVED_HARDWARE)));
            // Fire Hazard reaches the former Sold As-Is dust amount, with
            // proportionally lighter buildup at each preceding condition.
            const float dustAge=packedTuneDust?age*.75f:age;
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff080b0c),bowl.getCentreX(),bowl.getY(),
                juce::Colour(0xff3e4242),bowl.getCentreX(),bowl.getBottom(),false));g.fillEllipse(bowl);
            const bool buriedTuneTick=packedTuneDust&&bool(slider.getProperties().getWithDefault("previewTuneTickDust160",bool(KB09_APPROVED_HARDWARE)));
            const auto tunePointer=c+juce::Point<float>(std::sin(a),-std::cos(a))*d*.26f;
            if(buriedTuneTick) {
                // Printing sits beneath the settled coating and becomes
                // occluded as fine powder fills the recess.
                g.setColour(ink.withAlpha(1.f-.82f*std::pow(age,1.3f)));
                g.drawLine({c,tunePointer},1.4f);
            }
            for(uint32_t grain=0;grain<180;++grain) {
                const float theta=surfaceNoise(grain*7919u+57)*juce::MathConstants<float>::twoPi;
                const float radius=std::sqrt(surfaceNoise(grain*113u+91))*bowl.getWidth()*.47f;
                const auto q=bowl.getCentre()+juce::Point<float>(std::cos(theta),std::sin(theta))*radius;
                g.setColour(juce::Colour(0xffb2ac98).withAlpha(dustAge*(.23f+.30f*surfaceNoise(grain+53))));
                g.fillEllipse(q.x,q.y,.22f,.18f);
            }
            if(dustAge>0&&packedTuneDust) {
                juce::Graphics::ScopedSaveState recess(g);juce::Path clip;clip.addEllipse(bowl);g.reduceClipRegion(clip);
                const float buildup=std::pow(dustAge,1.55f);
                // Settled powder fills the recess, including its wall contact;
                // it does not spill onto the outer cap or surrounding panel.
                g.setColour(juce::Colour(0xffa59d87).withAlpha(buildup*.57f));g.fillEllipse(bowl);
                for(uint32_t grain=0;grain<720;++grain) {
                    const float theta=surfaceNoise(grain*7919u+1955u)*juce::MathConstants<float>::twoPi;
                    const float radial=std::sqrt(surfaceNoise(grain*113u+319u));
                    const auto q=bowl.getCentre()+juce::Point<float>(std::cos(theta),std::sin(theta))*radial*bowl.getWidth()*.5f;
                    const float speck=d*(.006f+.009f*surfaceNoise(grain*791u+717u));
                    const auto dust=juce::Colour(0xffbdb49b).interpolatedWith(juce::Colour(0xff746e5e),surfaceNoise(grain*31u+119u));
                    g.setColour(dust.withAlpha(buildup*(.32f+.42f*surfaceNoise(grain+811u))));
                    g.fillEllipse(q.x-speck*.5f,q.y-speck*.5f,speck,speck*.80f);
                }
            }
            if(!buriedTuneTick) {g.setColour(ink);g.drawLine({c,tunePointer},1.4f);}
        } else {
            const uint32_t seed=slider.getComponentID()=="range"?12011u:slider.getComponentID()=="wave"?41117u:93973u;
            projectedKnob(g,KB09_APPROVED_HARDWARE?area.reduced(5.f):area,a,age,seed,bool(slider.getProperties().getWithDefault("previewGripDulling",true)),
                bool(slider.getProperties().getWithDefault("previewGentleGrip17",true)),
                bool(slider.getProperties().getWithDefault("previewInnerCollar35",true)),
                bool(slider.getProperties().getWithDefault("previewThickTicks65",true)),
                bool(slider.getProperties().getWithDefault("previewCollarFraction71",true)),
                bool(slider.getProperties().getWithDefault("previewTopRightKnobs91",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewRimShine123",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewGripLight132",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewDetail135",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewInnerRim143",false)),
                bool(slider.getProperties().getWithDefault("previewFixedLight152",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewDetailFill161",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewBroadFill165",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewTroughDust166",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewTickGlimmer167",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewKnobShadow172",bool(KB09_APPROVED_HARDWARE))),
                bool(slider.getProperties().getWithDefault("previewOriginalKnobSize192",false))?1.f:mainKnobScale);
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
class FineSlider final : public juce::Slider,public ChoiceKnobDisplay,private juce::Timer {
public:
 static constexpr double choiceSnapDurationMs=25.;
 FineSlider() {setSliderSnapsToMousePosition(false);}
 double valueToProportionOfLength(double value) override {
  return sh09::controlResponse::position(getComponentID().toRawUTF8(),juce::Slider::valueToProportionOfLength(value));
 }
 double proportionOfLengthToValue(double proportion) override {
  return juce::Slider::proportionOfLengthToValue(sh09::controlResponse::effect(getComponentID().toRawUTF8(),proportion));
 }
 void mouseEnter(const juce::MouseEvent& e) override {getProperties().set("hardwareHoverActive",true);juce::Slider::mouseEnter(e);}
 void mouseExit(const juce::MouseEvent& e) override {getProperties().set("hardwareHoverActive",false);juce::Slider::mouseExit(e);}
 double displayedChoiceProportion() override {
  const double now=juce::Time::getMillisecondCounterHiRes();
  const double value=animationEnabled()?displayAt(now):valueToProportionOfLength(std::round(getValue()));
  if(bool(getProperties().getWithDefault("recordChoiceSnap",false))) {
   auto records=getProperties()["choiceSnapFrames"];
   if(!records.isArray())records=juce::var(juce::Array<juce::var>());
   juce::DynamicObject::Ptr frame=new juce::DynamicObject;
   frame->setProperty("time",now);frame->setProperty("start",snapStarted);frame->setProperty("serial",snapSerial);
   frame->setProperty("value",value);frame->setProperty("target",valueToProportionOfLength(snapTarget));
   if(records.getArray()->size()<2000)records.getArray()->add(juce::var(frame.get()));
   getProperties().set("choiceSnapFrames",records);
  }
  return value;
 }
 void valueChanged() override {
  if(!animationEnabled())return;
  const int next=juce::roundToInt(getValue());
  if(snapTarget<0){snapTarget=next;snapFrom=valueToProportionOfLength(next);return;}
  if(next==snapTarget)return;
  const double now=juce::Time::getMillisecondCounterHiRes();
  snapFrom=displayAt(now);snapTarget=next;snapStarted=now;++snapSerial;snapActive=true;
  getProperties().set("choiceSnapEndMs",now+choiceSnapDurationMs);
  getProperties().set("choiceSnapActive",true);
  getProperties().set("choiceSnapDurationMs",choiceSnapDurationMs);
  // Never let a cached intermediate raster become the displayed endpoint.
  setCachedComponentImage(nullptr);startTimer(4);repaint();
 }
 double snapValue(double attempted,DragMode mode) override {
  return choice()?std::round(attempted):juce::Slider::snapValue(attempted,mode);
 }
 void mouseDown(const juce::MouseEvent& e) override {
  getProperties().set("hardwareGestureActive",true);
  juce::Slider::mouseDown(e);last=e.position;position=valueToProportionOfLength(getValue());
 }
 void mouseUp(const juce::MouseEvent& e) override {
  getProperties().set("hardwareGestureActive",false);juce::Slider::mouseUp(e);
 }
 void mouseDrag(const juce::MouseEvent& e) override {
  LivePaintScope timing{getComponentID()=="range"?"rangeDrag":"otherDrag"};
  const auto delta=e.position-last;last=e.position;
  const bool rotary=getSliderStyle()==RotaryHorizontalVerticalDrag;
  const double distance=rotary?delta.x-delta.y:isVertical()?-delta.y:delta.x;
  const double extent=rotary?190.0:juce::jmax(1,isVertical()?getHeight():getWidth());
  position=juce::jlimit(0.0,1.0,position+distance/extent*(e.mods.isShiftDown()?.1:1.0));
  const double attempted=proportionOfLengthToValue(position);
  const bool discrete=getComponentID()=="range"||getComponentID()=="wave";
  setValue(discrete?std::round(attempted):attempted,juce::sendNotificationSync);repaint();
 }
private:
 bool choice() const {return getComponentID()=="range"||getComponentID()=="wave";}
 bool animationEnabled() const {return choice()&&bool(getProperties().getWithDefault("previewChoiceSnap35",bool(KB09_APPROVED_HARDWARE)));}
 double displayAt(double now) {
  if(snapTarget<0)return valueToProportionOfLength(std::round(getValue()));
  const double target=valueToProportionOfLength(snapTarget);
  if(!snapActive)return target;
  return snapFrom+(target-snapFrom)*juce::jlimit(0.,1.,(now-snapStarted)/choiceSnapDurationMs);
 }
 void timerCallback() override {
  if(juce::Time::getMillisecondCounterHiRes()-snapStarted>=choiceSnapDurationMs) {
   snapActive=false;getProperties().set("choiceSnapActive",false);stopTimer();
  }
  repaint();
 }
 double snapFrom=0,snapStarted=0;int snapTarget=-1,snapSerial=0;bool snapActive=false;
 juce::Point<float> last;double position=0;
};

class SlideSwitch final : public juce::Component, public juce::SettableTooltipClient {
public:
    SlideSwitch(juce::RangedAudioParameter& p,std::vector<int> positions,juce::StringArray names,bool left=false)
        : order(std::move(positions)),labels(std::move(names)),labelsLeft(left),
          attachment(p,[this](float v){value=juce::roundToInt(v);repaint();},nullptr) {
        initial=juce::roundToInt(p.convertFrom0to1(p.getDefaultValue()));
        setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::NormalCursor);
        attachment.sendInitialUpdate();
    }
    ~SlideSwitch() override {if(dragging)attachment.endGesture();}
    void paint(juce::Graphics& g) override {
        const auto* look=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=getY()<headerH+visibleBodyH&&look!=nullptr?look->age:0;
        const auto ink=agedInk(getY()<headerH+visibleBodyH?age:0);
        const float xx=actuatorX();
        const auto id=getComponentID();
        const bool middleOnLeft=labelsLeft||id=="lfoWave"||id=="vcaMode";
        const float middle=float(getHeight())*.5f,throwStep=order.size()==3?12.0f:10.0f;
        const float top=middle-throwStep*float(order.size()-1)*.5f,bottom=middle+throwStep*float(order.size()-1)*.5f;
        const float labelStep=order.size()==3?36.0f:20.0f;
        if(bool(getProperties().getWithDefault("previewSwitchTrack",true))) {
            // Preview only: a punched opening with a dark rolled edge rather
            // than a bright, uniformly outlined rectangular frame.
            // Center on the rendered actuator face, including its subtly
            // asymmetric dark bevel. Equal vertical margins at the middle stop.
            const juce::Rectangle<float> slot(xx-8.25f,top-11,16,bottom-top+22);
            // Match the fader slots' left rolled edge and narrow reflection.
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff363937),slot.getX(),slot.getY(),
                juce::Colour(0xff101211),slot.getRight(),slot.getY(),false));g.fillRect(slot);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff010202),slot.getX(),slot.getY(),
                juce::Colour(0xff080a09),slot.getRight(),slot.getBottom(),false));
            g.fillRect(slot.reduced(.9f,1.f));
            g.setColour(juce::Colour(0xff96998f).withAlpha(.08f*(1-age*.6f)));
            g.drawLine(slot.getX()+.3f,slot.getY()+1,slot.getX()+.3f,slot.getBottom()-.8f,.45f);
            g.setColour(juce::Colour(0xff666c62).withAlpha(.15f));
            g.drawLine(slot.getX()+.5f,slot.getY()+.2f,slot.getRight()-.7f,slot.getY()+.2f,.4f);
            g.setColour(juce::Colours::black.withAlpha(.75f));
            g.drawLine(slot.getRight()-.3f,slot.getY()+.5f,slot.getRight()-.3f,slot.getBottom(),.6f);
            g.drawLine(slot.getX(),slot.getBottom()-.3f,slot.getRight(),slot.getBottom()-.3f,.6f);
        } else {
            g.setColour(juce::Colour(0xff555c5e));g.fillRoundedRectangle(xx-8,top-11,16,bottom-top+23,.65f);
            g.setColour(juce::Colour(0xff020405));g.fillRoundedRectangle(xx-7.1f,top-10,14.2f,bottom-top+21,.35f);
            g.setColour(juce::Colour(0xff181d1e));g.drawLine(xx+6.7f,top-9.5f,xx+6.7f,bottom+10.5f,.35f);
        }
        if(age>0&&bool(getProperties().getWithDefault("previewSwitchDust135",bool(KB09_APPROVED_HARDWARE)))) {
            juce::Graphics::ScopedSaveState recessedDust(g);
            const juce::Rectangle<float> slot(xx-7.3f,top-10.f,14.2f,bottom-top+20.f);juce::Path opening;opening.addRectangle(slot);g.reduceClipRegion(opening);
            const uint32_t seed=uint32_t(id.hashCode())^0x135ff1u;
            for(uint32_t grain=0;grain<1400;++grain) {
                const auto key=surfaceHash(seed+grain*7919u);
                const float u=surfaceNoise(key+17),v=surfaceNoise(key+31);
                // Most powder settles into the side channels and slot floor.
                const float x=grain%3==0?slot.getX()+slot.getWidth()*u:
                    (grain%2?slot.getX()+1.4f*u*u:slot.getRight()-1.4f*u*u);
                const float y=grain%3==0?slot.getBottom()-2.f*v*v:slot.getY()+slot.getHeight()*v;
                const float size=.13f+.25f*surfaceNoise(key+47);
                g.setColour(juce::Colour(0xffa9a591).interpolatedWith(juce::Colour(0xff62675d),surfaceNoise(key+59))
                    .withAlpha(std::pow(age,1.4f)*(.19f+.39f*surfaceNoise(key+71))));
                g.fillEllipse(x,y,size,size*.75f);
            }
        }
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
                                 {i==0?xx-7:xx-25,labelY-4,16,8},ink);
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
                    panelGlyph(g,PanelGlyph::envelope,{xx-7,labelY+2,15,8},ink);
                }
            } else if(id=="vcfEnvMode"||id=="vcaMode") {
                if(i==0)caption(g,labels[0],axialText,7.3f,juce::Justification::centred,true,ink);
                else {
                    // Match VCA's approved middle ENV-to-slot spacing in VCF.
                    const auto words=i==1?juce::Rectangle<float>(xx-28,labelY-13,19,26):axialText;
                    caption(g,id=="vcaMode"&&i==2?"GATE":"ENV",words.translated(0,-5),7.6f,juce::Justification::centred,true,ink);
                    const auto glyph=i==1?PanelGlyph::envelope:id=="vcaMode"?PanelGlyph::gate:PanelGlyph::invertedEnvelope;
                    panelGlyph(g,glyph,{words.getCentreX()-8,labelY,16,8},ink);
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
            if(value==order[i]) {
                juce::Graphics::ScopedSaveState actuator(g);
                if constexpr(KB09_APPROVED_HARDWARE)g.reduceClipRegion(juce::Rectangle<float>(xx-7.35f,top-10,14.2f,bottom-top+20).toNearestInt());
                cap(g,{xx-5.2f,yy-9.6f,10.4f,19.2f},true,false,age,uint32_t(id.hashCode()),
                    bool(getProperties().getWithDefault("previewSwitchGrooveDust28",true)),bool(getProperties().getWithDefault("previewSwitchDust135",bool(KB09_APPROVED_HARDWARE))));
            }
        }
        
    }
    void mouseDown(const juce::MouseEvent& e) override {
        dragging=true;pointerDownX=e.position.x;
        draggingSlot=isActuatorGesture();attachment.beginGesture();select(e.position.y,false);showValue(0);
    }
    void mouseDrag(const juce::MouseEvent& e) override {select(e.position.y,false);showValue(0);}
    void mouseEnter(const juce::MouseEvent&) override {getProperties().set("hardwareHoverActive",true);showValue(0);}
    void mouseExit(const juce::MouseEvent&) override {getProperties().set("hardwareHoverActive",false);if(!dragging)popup.setVisible(false);}
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
            if(getComponentID()=="lfoWave")popup.setAllowedPlacement(juce::BubbleComponent::right);
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
        setMouseCursor(juce::MouseCursor::NormalCursor);setWantsKeyboardFocus(true);
        setTitle("32-key low-note-priority keyboard");
    }
    ~Keyboard() override {release();}
    void repaintNote(int note) {
        if(note>=53&&note<=84)repaint(keyRect(note).expanded(5).toNearestInt());
    }
    void repaintOctaveLabels() {
        for(int n=53;n<=84;++n)if(n%12==0) {
            const auto r=keyRect(n);
            repaint(juce::Rectangle<float>(r.getX()+1,r.getBottom()-26,r.getWidth()-2,18).getSmallestIntegerContainer());
        }
    }
    void setComputerNote(int note,bool down) override {
        if(note<0||note>=128||computerNotes[size_t(note)]==down)return;
        computerNotes[size_t(note)]=down;
        processor.queueKeyboard(note,down,false);
        repaint();
    }
    void paint(juce::Graphics& g) override {
        LivePaintScope timing{"keyboard"};
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
        const bool blackKeyProposal=bool(getProperties().getWithDefault("previewBlackKeyWear108",bool(KB09_APPROVED_HARDWARE)));
        const bool whiteKeyProposal=bool(getProperties().getWithDefault("previewWhiteKeyWear111",bool(KB09_APPROVED_HARDWARE)));
        const bool indicatorBacking=bool(getProperties().getWithDefault("previewKeyIndicator253",bool(KB09_APPROVED_HARDWARE)));
        const juce::Point<int> size(getWidth(),getHeight());
        if(keyCacheSize!=size||std::abs(keyCacheScale-scale)>.01f||keyCacheBlackProposal!=blackKeyProposal||keyCacheWhiteProposal!=whiteKeyProposal||keyCacheIndicatorBacking!=indicatorBacking)
            ageKeyCache.clear();
        if(keyCacheAge!=age||keyCacheSize!=size||std::abs(keyCacheScale-scale)>.01f||keyCacheBlackProposal!=blackKeyProposal||keyCacheWhiteProposal!=whiteKeyProposal||keyCacheIndicatorBacking!=indicatorBacking) {
            keyImages={};keyCacheAge=age;keyCacheSize=size;keyCacheScale=scale;
            keyCacheBlackProposal=blackKeyProposal;
            keyCacheWhiteProposal=whiteKeyProposal;keyCacheIndicatorBacking=indicatorBacking;
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
                for(int i=5;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.07f));g.fillRoundedRectangle(r.expanded(float(i)*.45f).translated(2,3),KB09_APPROVED_HARDWARE?3.5f:1.8f);}
                juce::Path outline;outline.addRoundedRectangle(r,KB09_APPROVED_HARDWARE?3.5f:2.5f);g.reduceClipRegion(outline);
                auto face=r.withTrimmedTop(r.getHeight()-3),top=r.withTrimmedBottom(3);
                const auto matte=[age](juce::Colour fresh,juce::Colour worn) {return fresh.interpolatedWith(worn,age);};
                if constexpr(KB09_APPROVED_HARDWARE) {
                    // The reference's raised glossy plateau occupies about
                    // two thirds of the overall key width. Broad planar sides
                    // and the front slope share a quiet, uniform plastic tone.
                    // Use the photographic plastic's fine surface variation,
                    // with its broad lighting removed, across each flat plane.
                    // Only the narrow rolled perimeter changes light rapidly.
                    constexpr float density=4.f;
                    const int mw=juce::jmax(1,juce::roundToInt(r.getWidth()*density)),mh=juce::jmax(1,juce::roundToInt(r.getHeight()*density));
                    juce::Image material(juce::Image::ARGB,mw,mh,true,juce::SoftwareImageType());
                    const auto& specimen=artwork().blackKey;
                    const juce::Image::BitmapData sample(specimen,juce::Image::BitmapData::readOnly);
                    {
                        juce::Image::BitmapData pixels(material,juce::Image::BitmapData::writeOnly);
                        const float slope=r.getHeight()*.075f,start=r.getHeight()-slope;
                        for(int py=0;py<mh;++py)for(int px=0;px<mw;++px) {
                            const float x=(px+.5f)/density,y=(py+.5f)/density;
                            const int sx=40+(px+int(n)*7)%72,sy=90+(py+int(n)*13)%400;
                            const auto luminance=[&](int xx,int yy){auto c=sample.getPixelColour(xx,yy);return (c.getFloatRed()+c.getFloatGreen()+c.getFloatBlue())/3.f;};
                            const float fine=luminance(sx,sy)-(luminance(sx-3,sy)+luminance(sx+3,sy)+luminance(sx,sy-3)+luminance(sx,sy+3))*.25f;
                            const float progress=juce::jlimit(0.f,1.f,(y-start)/slope);
                            const float join=r.getWidth()*.16f*(1-progress)+progress;
                            const bool front=y>=start&&x>=join&&x<=r.getWidth()-join;
                            const float tone=front?.151f:(x<r.getWidth()*.5f?.139f:.118f);
                            const float edge=juce::jmin(x,r.getWidth()-x,r.getHeight()-y);
                            // Retain a restrained edge reflection in the oldest condition.
                            const float finish=1.f-.86f*juce::jmin(age,.78f);
                            const auto beam=[](float distance,float centre,float width){return std::exp(-std::pow((distance-centre)/width,2.f));};
                            // Narrow glints follow the rolled edges, leaving
                            // the broad planar faces evenly lit. The left rim
                            // catches the upper-left source most strongly.
                            const float rim=((.42f+.28f*(1-age))*beam(x,.75f,.50f)
                                +(.16f+.12f*(1-age))*beam(r.getWidth()-x,.75f,.45f)
                                +(.25f+.20f*(1-age))*beam(r.getHeight()-y,.75f,.50f))*finish;
                            const float crease=front?((.24f+.28f*(1-age))*beam(x-join,.25f,.50f)
                                +(.08f+.12f*(1-age))*beam(r.getWidth()-join-x,.25f,.44f))*finish:0.f;
                            const float roll=.022f*beam(edge,.75f,.35f)-.038f*std::exp(-edge/.32f)+rim+crease;
                            const float grain=(surfaceNoise(uint32_t(n)*7919u+uint32_t(px)*317u+uint32_t(py)*113u)-.5f)*.006f;
                            const float value=juce::jlimit(0.f,1.f,tone+fine*.85f+grain+roll+age*.009f);
                            pixels.setPixelColour(px,py,juce::Colour::fromFloatRGBA(value,value*1.025f,value*.975f,1));
                        }
                    }
                    g.setOpacity(1.f);g.drawImage(material,r);
                    top=r.reduced(r.getWidth()*.16f,0).withTrimmedTop(.6f).withTrimmedBottom(r.getHeight()*.075f);
                    juce::Graphics::ScopedSaveState raised(g);juce::Path plateau;plateau.addRoundedRectangle(top,2.7f);g.reduceClipRegion(plateau);
                    g.setColour(matte(juce::Colour(0xff111517),juce::Colour(0xff191a17)));g.fillRect(top);
                    const auto& photo=artwork().blackKey;
                    const auto source=juce::Rectangle<int>(juce::roundToInt(photo.getWidth()*.18f),2,
                        juce::roundToInt(photo.getWidth()*.64f),juce::roundToInt(photo.getHeight()*.94f));
                    const float reflected=.57f+.06f*surfaceNoise(uint32_t(n)*6197u);
                    g.setOpacity(1-age*(1-reflected));g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                    g.drawImage(photo.getClippedImage(source),top,juce::RectanglePlacement::stretchToFit);g.setOpacity(1);
                } else {
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
                const float reflected=(blackKeyProposal?.57f:.34f)+.06f*surfaceNoise(uint32_t(n)*6197u);
                g.setOpacity(1.0f-age*(1.0f-reflected));g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(artwork().blackKey,r,juce::RectanglePlacement::stretchToFit);
                g.setOpacity(1.0f);
                }
                // Reference photo: a longer front slope (about 7.5% of the
                // visible black-key length) with gently softened transitions.
                const float chamfer=KB09_APPROVED_HARDWARE?r.getHeight()*.075f:10.f;
                const float shoulder=r.getBottom()-chamfer;
                juce::Path bevel;bevel.startNewSubPath(r.getX()+4,shoulder);
                bevel.lineTo(r.getRight()-4,shoulder);bevel.lineTo(r.getRight()-1,r.getBottom()-1);
                bevel.lineTo(r.getX()+1,r.getBottom()-1);bevel.closeSubPath();
                if constexpr(KB09_APPROVED_HARDWARE)bevel=bevel.createPathWithRoundedCorners(3.5f);
                if constexpr(KB09_APPROVED_HARDWARE) {
                    // A flat face has one tone across its plane. Only its
                    // rounded outline and real wear interrupt that tone.
                    // The material above already covers this planar face.
                    // Do not paint over its texture with a solid decal.
                } else {
                    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff555953),0,shoulder,
                        juce::Colour(0xff0b0d0c),0,r.getBottom()-1,false));g.fillPath(bevel);
                    g.setColour(juce::Colour(0xffaeb6b2).withAlpha(.24f*(1-age*.5f)));
                    g.drawLine(r.getX()+4.5f,shoulder,r.getRight()-4.5f,shoulder,.65f);
                }
                if constexpr(KB09_APPROVED_HARDWARE)if(blackKeyProposal&&age>0) {
                    juce::Graphics::ScopedSaveState wornFace(g);g.reduceClipRegion(bevel);
                    const uint32_t seed=uint32_t(n)*104729u+0x213faceu;
                    // Fingers also rub the sloped front. Shallow abrasion and
                    // broken polish follow that plane, without gouging it.
                    for(uint32_t patch=0;patch<4;++patch) {
                        const auto key=seed+patch*7919u;
                        const float x=r.getX()+r.getWidth()*(.18f+.64f*surfaceNoise(key+17));
                        const float y=shoulder+chamfer*(.22f+.56f*surfaceNoise(key+31));
                        const float radius=2.f+3.f*surfaceNoise(key+47);
                        juce::ColourGradient rub(juce::Colour(patch%2?0xffa5a69b:0xff020403).withAlpha(age*.16f),x,y,
                            juce::Colours::transparentBlack,x+radius,y+radius*.65f,true);
                        g.setGradientFill(rub);g.fillEllipse(x-radius,y-radius*.65f,radius*2,radius*1.3f);
                    }
                    for(uint32_t mark=0;mark<32;++mark) {
                        const auto key=seed+mark*65537u;
                        const float strength=juce::jlimit(0.f,1.f,(age-(.08f+.68f*surfaceNoise(key+11)))/.24f);
                        if(strength<=0)continue;
                        const float x=r.getX()+r.getWidth()*(.08f+.84f*surfaceNoise(key+23));
                        const float y=shoulder+chamfer*(.08f+.82f*surfaceNoise(key+37));
                        const float dx=(surfaceNoise(key+53)-.5f)*6.f,dy=(surfaceNoise(key+67)-.5f)*2.8f;
                        juce::Path scuff;scuff.startNewSubPath(x,y);scuff.quadraticTo(x+dx*.5f,y+dy*.4f-.2f,x+dx,y+dy);
                        const float width=.12f+.10f*surfaceNoise(key+79);
                        g.setColour(juce::Colour(0xff080a09).withAlpha(strength*.30f));
                        g.strokePath(scuff,juce::PathStrokeType(width+.12f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                        g.setColour(juce::Colour(0xffbec0b5).withAlpha(strength*(.18f+.14f*surfaceNoise(key+97))));
                        g.strokePath(scuff,juce::PathStrokeType(width,juce::PathStrokeType::curved,juce::PathStrokeType::rounded),juce::AffineTransform::translation(-.1f,-.1f));
                    }
                }
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
            keySurfaceWear(g,r,serial,pass!=0,age,pass!=0?blackKeyProposal:whiteKeyProposal);
            const float crackOnset=n==60?.50f:n==65?.68f:n==74?.70f:2.0f;
            if(age>=crackOnset) {
                const float progress=juce::jlimit(0.0f,1.0f,(age-crackOnset)/(.90f-crackOnset));
                const float width=r.getWidth()*.20f,height=r.getHeight()*(n==65?.18f:whiteKeyProposal&&n==74?.26f:.36f);
                const auto trace=juce::Rectangle<float>(r.getX()+r.getWidth()*(n==74?.18f:.32f),r.getBottom()-height-17,width,height);
                juce::Graphics::ScopedSaveState save(g);
                // Vintage begins at 40% of the Sold As-Is crack length.
                // Preserve the reviewed Sold As-Is and fully aged endpoints.
                constexpr float soldCrack=.64f,vintageCrack=soldCrack*.40f;
                const float firstCrack=age<=.75f
                    ?vintageCrack+(soldCrack-vintageCrack)*juce::jlimit(0.f,1.f,(age-.5f)/.25f)
                    :soldCrack+(1-soldCrack)*juce::jlimit(0.f,1.f,(age-.75f)/.15f);
                const float revealed=trace.getHeight()*(n==60?firstCrack:(.04f+.96f*progress));
                g.reduceClipRegion(trace.withTrimmedTop(trace.getHeight()-revealed).toNearestInt());
                // Trace the owner's gently wandering hairline as a native
                // path: subpixel width at M, sharp at 4K, no bitmap dirt halo.
                juce::Path hairline;hairline.startNewSubPath(.56f,0);
                hairline.cubicTo(.58f,.12f,.32f,.19f,.39f,.29f);
                hairline.cubicTo(.41f,.38f,.65f,.44f,.59f,.53f);
                hairline.cubicTo(.57f,.65f,.40f,.71f,.45f,.80f);
                hairline.cubicTo(.46f,.87f,.22f,.95f,.15f,1);
                if(!whiteKeyProposal){hairline.startNewSubPath(.59f,.53f);hairline.lineTo(.84f,.58f);}
                if(n==65) {
                    hairline.clear();hairline.startNewSubPath(.8f,0);hairline.cubicTo(.72f,.18f,.27f,.19f,.43f,.38f);
                    hairline.cubicTo(.66f,.55f,.28f,.68f,.36f,.82f);hairline.lineTo(.19f,1);
                } else if(n==74) {
                    hairline.clear();hairline.startNewSubPath(.22f,0);hairline.cubicTo(.32f,.21f,.69f,.25f,.54f,.42f);
                    hairline.cubicTo(.39f,.6f,.76f,.74f,.63f,1);
                }
                const auto transform=juce::AffineTransform::scale(trace.getWidth(),trace.getHeight()).translated(trace.getX(),trace.getY());
                hairline.applyTransform(transform);
                const float crackWidth=n==60?.32f:n==65?.34f:.33f;
                // Fade every ink layer at the currently revealed ends, not
                // the full hidden path: age clipping must not leave a blunt tip.
                const float visibleTop=trace.getBottom()-revealed;
                const auto feather=[&](juce::Colour colour,float top,float bottom,bool fadeStart=true) {
                    const float fraction=juce::jmin(.45f,juce::jmax(.32f,16.f/juce::jmax(1.f,bottom-top)));
                    juce::ColourGradient ink(fadeStart?colour.withAlpha(0.f):colour,0,top,
                        colour.withAlpha(0.f),0,bottom,false);
                    if(fadeStart) {
                        ink.addColour(fraction*.20f,colour.withMultipliedAlpha(.025f));
                        ink.addColour(fraction*.50f,colour.withMultipliedAlpha(.18f));
                        ink.addColour(fraction*.80f,colour.withMultipliedAlpha(.60f));
                        ink.addColour(fraction,colour);
                    }
                    ink.addColour(1-fraction,colour);
                    ink.addColour(1-fraction*.80f,colour.withMultipliedAlpha(.60f));
                    ink.addColour(1-fraction*.50f,colour.withMultipliedAlpha(.18f));
                    ink.addColour(1-fraction*.20f,colour.withMultipliedAlpha(.025f));
                    g.setGradientFill(ink);
                };
                if(whiteKeyProposal) {
                    feather(juce::Colour(0xff6e634e).withAlpha(.07f),visibleTop,trace.getBottom());
                    g.strokePath(hairline,juce::PathStrokeType(crackWidth+.60f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                    feather(juce::Colour(0xff554835).withAlpha(.16f),visibleTop,trace.getBottom());
                    g.strokePath(hairline,juce::PathStrokeType(crackWidth+.25f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                }
                feather(juce::Colour(0xff554835).withAlpha(.78f),visibleTop,trace.getBottom());
                g.strokePath(hairline,juce::PathStrokeType(crackWidth,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                if(whiteKeyProposal&&n==60) {
                    juce::Path branch;branch.startNewSubPath(.59f,.53f);
                    branch.cubicTo(.62f,.535f,.655f,.554f,.68f,.571f);
                    branch.cubicTo(.705f,.586f,.735f,.580f,.752f,.601f);
                    branch.applyTransform(transform);
                    feather(juce::Colour(0xff6e634e).withAlpha(.08f),trace.getY()+trace.getHeight()*.53f,trace.getY()+trace.getHeight()*.632f,false);
                    g.strokePath(branch,juce::PathStrokeType(.72f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                    feather(juce::Colour(0xff554835).withAlpha(.66f),trace.getY()+trace.getHeight()*.53f,trace.getY()+trace.getHeight()*.632f,false);
                    g.strokePath(branch,juce::PathStrokeType(.23f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                    juce::Path tip;tip.startNewSubPath(.752f,.601f);tip.cubicTo(.766f,.615f,.783f,.616f,.793f,.632f);tip.applyTransform(transform);
                    feather(juce::Colour(0xff554835).withAlpha(.35f),trace.getY()+trace.getHeight()*.53f,trace.getY()+trace.getHeight()*.632f,false);
                    g.strokePath(tip,juce::PathStrokeType(.13f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                }
            }
            if(down) {
                g.setColour(juce::Colours::black.withAlpha(pass==0?.14f:.18f));g.fillRoundedRectangle(r.reduced(1),1.5f);
                if(effectiveNote==n) {
                    const auto marker=juce::Rectangle<float>(r.getX()+9,r.getBottom()-7,r.getWidth()-18,3);
                    if(pass==0&&indicatorBacking) {
                        g.setColour(juce::Colour(0xff392511).withAlpha(.12f));g.fillRoundedRectangle(marker.expanded(1.15f,.95f),1.8f);
                        g.setColour(juce::Colour(0xff392511).withAlpha(.32f));g.fillRoundedRectangle(marker.expanded(.90f,.75f),1.6f);
                        g.setColour(juce::Colour(0xff392511).withAlpha(.92f));g.fillRoundedRectangle(marker.expanded(.65f,.55f),1.4f);
                    }
                    g.setColour(amber);g.fillRoundedRectangle(marker,1.0f);
                }
            }
            };
            const auto region=keyRect(n).expanded(5).getSmallestIntegerContainer();
            auto& image=keyImages[size_t(n-53)][size_t(down?(effectiveNote==n?2:1):0)];
            const auto ageKey=juce::String(age,7)+":"+juce::String(n)+":"+juce::String(down?(effectiveNote==n?2:1):0);
            if(!image.isValid())image=ageKeyCache.find(ageKey);
            if(!image.isValid()) {
                image=juce::Image(juce::Image::ARGB,juce::roundToInt(region.getWidth()*scale),juce::roundToInt(region.getHeight()*scale),true,juce::SoftwareImageType());
                juce::Graphics raster(image);
                raster.addTransform(juce::AffineTransform::scale(scale).translated(-region.getX()*scale,-region.getY()*scale));
                paintKey(raster);
                ageKeyCache.put(ageKey,image);
                getProperties().set("keyRasterCount",int(getProperties().getWithDefault("keyRasterCount",0))+1);
            }
            getProperties().set("ageKeyCacheBytes",double(ageKeyCache.sizeBytes()));
            // Octave captions use translucent ink. Never carry that opacity
            // into subsequent white/black key image blits.
            g.setOpacity(1.f);drawNativeRaster(g,image,scale,region.getTopLeft().toFloat());
            // RANGE changes only the octave printing, not the key material.
            // Keep lettering live so changing range never rebuilds 32 keys
            // on the message thread during a 35 ms knob transition.
            if(pass==0&&n%12==0&&!bool(getProperties().getWithDefault("externalOctaveLabels",false))) {
                const int octave=n/12-1+range-2;
                caption(g,"C"+juce::String(octave),{r.getX()+2,r.getBottom()-25,r.getWidth()-4,15},10.5f,
                    juce::Justification::centred,false,juce::Colour(0xff514d43).withAlpha(down?.7482f:.87f));
            }
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
    kb09_gui::AgeRasterCache ageKeyCache{128u*1024u*1024u};
    std::array<std::array<juce::Image,3>,32> keyImages;
    juce::Image whiteMaterial;float whiteMaterialAge=-1;
    float keyCacheAge=-1,keyCacheScale=0;juce::Point<int> keyCacheSize;
    bool keyCacheBlackProposal=false;
    bool keyCacheWhiteProposal=false,keyCacheIndicatorBacking=false;

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
        juce::String label;float level=0,displayFloor=0;bool midi=false;
        int litSegments() const noexcept {
            if(midi)return level>0?1:0;
            // Hardware inputs never reach digital zero. Do not present their
            // inaudible noise floor as a permanently active first segment.
            if(level<=displayFloor)return 0;
            int count=0;
            for(int i=0;i<8;++i)if(level>float(i)/8)++count;
            return count;
        }
        void paint(juce::Graphics& g) override {
            caption(g,label,{0,0,midi?31.0f:25.0f,20},9,juce::Justification::centredLeft);
            const int lit=litSegments();
            if(midi){g.setColour(lit?amber:juce::Colour(0xff373b39));g.fillEllipse(34,6,8,8);}
            else for(int i=0;i<8;++i){g.setColour(i<lit?amber:juce::Colour(0xff373b39));g.fillRect(28.0f+float(i*6),6.0f,4.0f,8.0f);}
        }
    };
public:
    explicit Meter(SH09Processor& p):processor(p) {
        const char* labels[]={"ENV","EXT","IN","MIDI"};
        const char* tips[]={"ENV: the synth's ADSR envelope level.","EXT: external audio envelope follower level, measured before the EXT fader.","IN: incoming audio level before the EXT fader and stereo-to-mono conversion.","MIDI: incoming notes, pitch bend and controller activity. This is not an audio meter."};
        for(int i=0;i<4;++i){auto& lamp=indicators[size_t(i)];lamp.label=labels[i];lamp.midi=i==3;
            lamp.displayFloor=(i==1||i==2)?.0001f:0.f; // -80 dBFS, display only.
            lamp.setTooltip(tips[i]);lamp.setComponentID(juce::String("indicator")+labels[i]);addAndMakeVisible(lamp);lamp.setBounds(i*83,0,i==3?44:78,20);}
        startTimerHz(120);
    }
    ~Meter() override {stopTimer();}
    void setLevels(float e,float f,float input,unsigned serial) {
        const auto now=juce::Time::getMillisecondCounter();
        if(serial!=lastSerial){lastSerial=serial;lastActivity=now;hasActivity=true;}
        const float levels[]={e,f,input,hasActivity&&juce::uint32(now-lastActivity)<90?1.0f:0.0f};
        for(int i=0;i<4;++i){
            auto& lamp=indicators[size_t(i)];const int previous=lamp.litSegments();
            // Never retain a tiny positive release tail when the real level is
            // zero. Repaint on visible segment transitions, not a level delta.
            lamp.level=levels[i];if(lamp.litSegments()!=previous)lamp.repaint();
        }
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
class RateLamp final : public juce::Component, public juce::SettableTooltipClient, private juce::Timer {
public:
    explicit RateLamp(SH09Processor& p):processor(p) {
        setTooltip("Modulator rate light: follows the LFO cycle and RATE setting.");startTimerHz(120);
    }
    ~RateLamp() override {stopTimer();
#if JUCE_WINDOWS
        independent.reset();
#endif
    }
    bool appearanceSnapshot=false;
    void paint(juce::Graphics& g) override {
        if(appearanceSnapshot)return;
        const float level=float(getProperties().getWithDefault("previewLedLevel",on));
        if constexpr(KB09_APPROVED_HARDWARE)kb09_gui::drawInsetRedLamp(g,getLocalBounds().toFloat().getCentre(),level,true);
        else {
            auto r=getLocalBounds().toFloat().withSizeKeepingCentre(8,8).reduced(1);
            g.setColour(juce::Colour(0xff05090b));g.fillEllipse(r);
            g.setColour(juce::Colour(0xff471c17).interpolatedWith(juce::Colour(0xffed5035),level));g.fillEllipse(r.reduced(1.1f));
            g.setColour(juce::Colour(0xffffc493).withAlpha(.12f+level*.53f));g.fillEllipse(r.getX()+2,r.getY()+1.8f,1.6f,1.6f);
        }
    }
private:
    SH09Processor& processor;kb09_gui::RateLampClock clock;float on=0;
#if JUCE_WINDOWS
    std::unique_ptr<kb09_gui::WindowsRateLamp> independent;
    void updateIndependent() {
        if constexpr(!KB09_APPROVED_HARDWARE)return;
        auto* peer=getPeer();
        getProperties().set("ratePeer245",peer!=nullptr);getProperties().set("rateShowing245",isShowing());
        if(peer&&isShowing()) {
            if(!independent)independent=std::make_unique<kb09_gui::WindowsRateLamp>([this] {
                return kb09_gui::WindowsRateLamp::Input{processor.isPowerEnabled(),
                    processor.rateLampFrequency(),
                    processor.displayedModulatorPhase.load(std::memory_order_relaxed),
                    processor.displayedModulatorTime.load(std::memory_order_acquire)};
            });
            auto& owner=peer->getComponent();RECT client{};GetClientRect(static_cast<HWND>(peer->getNativeHandle()),&client);
            const float sx=float(client.right-client.left)/owner.getWidth(),sy=float(client.bottom-client.top)/owner.getHeight();
            const auto origin=owner.getLocalPoint(this,juce::Point<float>{0,0});
            const auto end=owner.getLocalPoint(this,juce::Point<float>{float(getWidth()),float(getHeight())});
            const auto bounds=juce::Rectangle<int>::leftTopRightBottom(juce::roundToInt(origin.x*sx),juce::roundToInt(origin.y*sy),
                juce::roundToInt(end.x*sx),juce::roundToInt(end.y*sy));
            getProperties().set("rateBounds245",bounds.toString());
            independent->place(peer->getNativeHandle(),bounds,true);
        }else if(independent)independent->place(nullptr,{},false);
        if(independent) {
            getProperties().set("rateNativeFrames245",int(independent->frames()));
            getProperties().set("rateNativeTransitions245",int(independent->transitions()));
            getProperties().set("rateNativeGap245",independent->largestGap());
            getProperties().set("rateNativeErrors245",int(independent->errors()));
            getProperties().set("rateNativeErrorCode245",int(independent->lastError()));
            getProperties().set("rateNativeHandle245",juce::int64(independent->handle()));
        }
    }
#endif
    void timerCallback() override {
#if JUCE_WINDOWS
        updateIndependent();
#endif
        const double now=juce::Time::getMillisecondCounterHiRes();
        const double stamp=processor.displayedModulatorTime.load(std::memory_order_acquire);
        const double hz=processor.rateLampFrequency();
        const double phase=processor.displayedModulatorPhase.load(std::memory_order_relaxed)+juce::jmax(0.0,(now-stamp)*.001)*hz;
        const float value=clock.level(processor.isPowerEnabled(),hz,now,phase);
        if(std::abs(on-value)>.005f){on=value;
            // Keep the underlying renderer current too. A host may temporarily
            // cover or detach the native overlay; it must not expose a stale LED.
            repaint();
        }
        if(auto* parent=getParentComponent();parent&&bool(parent->getProperties().getWithDefault("rateTimingProbe186",false)))
            parent->getProperties().set("rateUpdateTime186",now);
    }
};
class PowerButton final : public juce::Button, private juce::Timer {
public:
    explicit PowerButton(SH09Processor& p):Button("Power"),processor(p) {
        setComponentID("power");setTooltip("Instrument audio on/off. This setting is not stored in patches.");
        onClick=[this]{
            const auto now=juce::Time::getMillisecondCounterHiRes();
            if(!travel.hasCycle()){travel.press(now,restHeight());travel.release(now);}
            // Preserve the released button's entire spring/repaint path.
            // Only skip the audio toggle when mouse-down already turned ON.
            if(!(mouseGesture&&poweredOnPress))processor.setPowerEnabled(!processor.isPowerEnabled());
            startTimer(4);repaint();if(onPowerChanged)onPowerChanged();
        };
    }
    ~PowerButton() override {stopTimer();}
    std::function<void()> onPowerChanged;
    void mouseDown(const juce::MouseEvent& e) override {
        if(!isEnabled())return;
        mouseGesture=true;mouseHeld=true;
        travel.press(juce::Time::getMillisecondCounterHiRes(),restHeight());startTimer(4);
        poweredOnPress=!processor.isPowerEnabled();
        if(poweredOnPress) {
            processor.setPowerEnabled(true);
            if(onPowerChanged)onPowerChanged();
        }
        juce::Button::mouseDown(e);repaint();
    }
    void mouseUp(const juce::MouseEvent& e) override {
        mouseHeld=false;travel.release(juce::Time::getMillisecondCounterHiRes());startTimer(4);
        // Button::mouseUp may flash buttonDown for 100 ms if the down frame
        // was not painted. That accessibility feedback must not re-press the cap.
        juce::Button::mouseUp(e);mouseGesture=false;repaint();
    }
    void buttonStateChanged() override {
        const auto now=juce::Time::getMillisecondCounterHiRes();
        const bool physicallyDown=mouseGesture?mouseHeld:isDown();
        if(physicallyDown&&!travel.isHeld())travel.press(now,restHeight());
        else if(!physicallyDown&&travel.isHeld())travel.release(now);
        if(travel.hasCycle())startTimer(4);
        repaint();
    }
    bool hitTest(int x,int y) override {
        if constexpr(KB09_APPROVED_HARDWARE)return x>=0&&x<50&&y>=0&&y<44;
        return juce::Button::hitTest(x,y);
    }
    void paintButton(juce::Graphics& g,bool over,bool down) override {
        juce::Graphics::ScopedSaveState buttonSurface(g);
        if constexpr(KB09_APPROVED_HARDWARE)g.addTransform(juce::AffineTransform::scale(.94f,1.f,25.f,0.f));
        const auto* appearance=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=appearance&&appearance->renderingPreview?appearance->age:appearanceAge(processor.state.getRawParameterValue("age")->load());
        // Fire Hazard retains the former Vintage finish. Only the button's
        // plastic ages more slowly; surrounding hardware uses full AGE.
        const float finishAge=age*.5f;
        const bool proposed=bool(getProperties().getWithDefault("previewPowerCrown",true));
        const bool solidBase=bool(getProperties().getWithDefault("previewSolidPower17",true));
        const float heightUnits=float(getProperties().getWithDefault("previewPowerHeightUnits",travel.sample(juce::Time::getMillisecondCounterHiRes(),restHeight())));
        const bool pressed=heightUnits<1.5f;
        getProperties().set("powerTravelUnits",heightUnits);
        const auto faceBounds=KB09_APPROVED_HARDWARE?juce::Rectangle<int>(0,0,50,44):getLocalBounds();
        const auto recess=faceBounds.toFloat().withTrimmedTop(10).reduced(.7f);
        // The grey pushbutton stands tall when OFF. Project its estimated
        // physical height through the same 11-degree view as the other hardware;
        // ON lowers the same cap without shrinking its top or changing camera.
        const bool lowerMount=bool(getProperties().getWithDefault("previewPowerMount20",true));
        const bool centeredMount=bool(getProperties().getWithDefault("previewPowerNudge24",true));
        const float mountNudge=(centeredMount?.65f:0.f)+(bool(getProperties().getWithDefault("previewContourMount25",true))?1.45f:0.f);
        const auto visibleRecess=lowerMount?recess.withTrimmedTop(15.5f*panelViewSin+mountNudge):recess;
        g.setColour(juce::Colour(0xff050605));g.fillRoundedRectangle(visibleRecess,1.3f);
        g.setColour(juce::Colour(0xff43433b).withAlpha(.36f));g.drawRoundedRectangle(visibleRecess,1.3f,.45f);
        const auto footprint=recess.reduced(1.35f,1.2f).withTrimmedTop(3.0f);
        const bool lowerHeight=bool(getProperties().getWithDefault("previewPowerHeight18",true));
        const bool halfAgain=bool(getProperties().getWithDefault("previewPowerHalf23",true));
        // The visible knob grip side is .078 of its projected sprite diameter.
        // OFF = 2/3 of that height; ON = 1/3, with the same existing press cycle.
        const float knobSideHeight=88.f*.95f*.078f;
        const float wallHeight=bool(getProperties().getWithDefault("previewPowerProportion89",bool(KB09_APPROVED_HARDWARE)))
            ?knobSideHeight*heightUnits/3.f
            :(lowerHeight?15.5f:30.f)*heightUnits*panelViewSin*(halfAgain?.5f:1.f);
        getProperties().set("powerVisibleSideHeight",wallHeight);
        const auto top=footprint.translated(0,-wallHeight);
        if constexpr(KB09_APPROVED_HARDWARE)
            // The tall OFF cap needs a visible throw past its skirt and recess.
            // Shorten continuously through ON to the fully depressed contact.
            kb09_gui::roundedHardwareShadow(g,footprint,1.1f,juce::jlimit(.08f,1.2f,heightUnits*.6f));
        else {g.setColour(juce::Colours::black.withAlpha(pressed?.35f:.47f));
            g.fillRoundedRectangle(footprint.translated(centeredMount?0.f:.45f,.25f+wallHeight*.34f),1.1f);}
        const auto grey=juce::Colour(0xff959b98).interpolatedWith(juce::Colour(0xffb1a774),finishAge*(KB09_APPROVED_HARDWARE?.50f:.65f));
        // Extend the skirt behind the rounded cap so its front corners meet
        // solid plastic rather than exposing a waist-shaped notch.
        const float overlap=proposed?5.5f:2.f;
        const auto wall=juce::Rectangle<float>(top.getX(),top.getBottom()-overlap,top.getWidth(),wallHeight+overlap);
        const auto wallTop=proposed?grey.withMultipliedBrightness(.74f):grey.darker(.13f);
        const auto wallBottom=proposed?grey.withMultipliedBrightness(.71f):grey.darker(.23f);
        const bool lighterUnderside=bool(getProperties().getWithDefault("previewPowerUnderside104",bool(KB09_APPROVED_HARDWARE)));
        if(solidBase)g.setColour(grey.withMultipliedBrightness(lighterUnderside?.43f:.38f));
        else g.setGradientFill(juce::ColourGradient(wallTop,0,wall.getY(),wallBottom,0,wall.getBottom(),false));
        // Retain straight skirt faces, with only a small molded edge radius.
        // A narrow light roll describes the rounded vertical edge rather than
        // rounding the entire bottom into a second pill-shaped cap.
        juce::Path skirt;skirt.addRoundedRectangle(wall,.85f);
        g.fillPath(skirt);
        if(bool(getProperties().getWithDefault("previewPowerCorners18",true))) {
            juce::Graphics::ScopedSaveState corners(g);g.reduceClipRegion(skirt);
            // Small, symmetric contact shadows at the lower corners only;
            // retain the approved uniform color across the broad front wall.
            for(bool right:{false,true}) {
                const float x=right?wall.getRight()-.20f:wall.getX()+.20f;
                const float y=wall.getBottom()-.20f;
                g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.16f),x,y,
                    juce::Colours::transparentBlack,x+(right?-4.f:4.f),y,true));
                g.fillRect(right?wall.getRight()-4.f:wall.getX(),wall.getBottom()-3.4f,4.f,3.4f);
            }
        }
        if(!solidBase) {
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
        g.drawImage(capSurface(top.getWidth(),top.getHeight(),finishAge,over),top,juce::RectanglePlacement::stretchToFit);

    }
private:
    float restHeight() const{return processor.isPowerEnabled()?1.f:2.f;}
    void timerCallback() override {
        travel.sample(juce::Time::getMillisecondCounterHiRes(),restHeight());repaint();
        if(!travel.animating())stopTimer();
    }
    kb09_gui::PowerButtonTravel travel;
    bool mouseGesture=false,mouseHeld=false,poweredOnPress=false;
    const juce::Image& capSurface(float width,float height,float age,bool over) {
        // A rounded moulded bevel, rather than a thin outlined inset. Surface
        // normals blend into the flat centre and give the front corner a soft
        // diffuse highlight. Dimensions are photo-informed visual estimates.
        const auto* look=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float scale=look!=nullptr&&look->conditionDragging?2.0f:6.0f;
        const bool proposed=bool(getProperties().getWithDefault("previewPowerCrown",true));
        const bool balancedCorners=bool(getProperties().getWithDefault("previewPowerNudge24",true));
        const bool shoulderProposal=bool(getProperties().getWithDefault("previewPowerShoulder32",true));
        const bool sliverOnly=bool(getProperties().getWithDefault("previewPowerSliver33",true));
        const bool refinedFinish=shoulderProposal||bool(getProperties().getWithDefault("previewPowerFinish31",false));
        // Tighten the top-to-side roll, retaining the approved plan outline.
        const float bevel=proposed?(shoulderProposal?(sliverOnly?2.45f:2.65f):3.2f):1.5f,radius=KB09_APPROVED_HARDWARE?2.7f:proposed?(shoulderProposal?3.5f:refinedFinish?4.2f:4.6f):3.f;
        const int w=juce::roundToInt(width*scale),h=juce::roundToInt(height*scale);
        if(cap.isValid()&&cap.getWidth()==w&&cap.getHeight()==h&&capAge==age&&capOver==over&&capProposed==proposed&&capBalancedCorners==balancedCorners&&capRefinedFinish==refinedFinish&&capShoulder==shoulderProposal&&capSliverOnly==sliverOnly)return cap;
        cap=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType());capAge=age;capOver=over;capProposed=proposed;capBalancedCorners=balancedCorners;capRefinedFinish=refinedFinish;capShoulder=shoulderProposal;capSliverOnly=sliverOnly;
        const auto base=juce::Colour(0xff959b98).interpolatedWith(juce::Colour(0xffb1a774),age*(KB09_APPROVED_HARDWARE?.50f:.65f));
        // Keep the installed Vintage sheen as the new Fire Hazard endpoint.
        // Interpolate reflection strengths evenly, without changing yellowing.
        const float fullAge=juce::jlimit(0.f,1.f,age*2.f);
        const float shineProgress=.5f*fullAge;
        const float specularProgress=std::pow(.5f,.95f)*fullAge;
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
            float nx=gx*std::sin(angle),ny=gy*std::sin(angle),nz=std::cos(angle);
            if(proposed) {
                // A softly crowned top turns through both directions at the
                // corners, rather than leaving a flat face inside a thin bevel.
                nx+=.10f*px/(width*.5f);ny+=.12f*py/(height*.5f);
                const float normal=std::sqrt(nx*nx+ny*ny+nz*nz);nx/=normal;ny/=normal;nz/=normal;
            }
            // Both lateral bevels must remain distinguishable from the top.
            // A strong left-only edge reflection previously erased that side
            // and made the centered cap appear to lean toward the right.
            const float cornerBlend=balancedCorners?
                kb09_gui::power_detail::smoothRange(width*.28f,width*.40f,std::abs(px)):0.f;
            const float litNx=nx+(std::abs(nx)-nx)*cornerBlend;
            const float litPx=px+(-std::abs(px)-px)*cornerBlend;
            const float diffuse=juce::jmax(0.0f,-litNx*.35f-ny*.42f+nz*.84f);
            const float specularAlignment=shoulderProposal?-.32f*nx-.35f*ny+.88f*nz:-litNx*.19f-ny*.23f+nz*.953f;
            const float specular=std::pow(juce::jlimit(0.0f,1.0f,specularAlignment),proposed?48.f:16.f)
                *(proposed?(refinedFinish?.27f:.20f)*(1.f-(refinedFinish?.88f:.78f)*specularProgress):(.075f-age*.035f));
            const float grain=(kb09_gui::power_detail::smoothNoise((px+width)*7,(py+height)*7,9457u)-.5f)*(.045f+age*.015f);
            const float reflectionPx=shoulderProposal?px:litPx;
            const float edgeReflection=(sliverOnly?1.2f:1.f)*(proposed?(refinedFinish?1.55f:1.35f)*(1.f-(refinedFinish?.86f:.74f)*shineProgress):1-age)*(
                (balancedCorners?(shoulderProposal?.055f:.035f):.22f)*std::exp(-std::pow((reflectionPx+width*.5f-1.55f)/.65f,2.f))
                    *std::exp(-std::pow((py+height*.20f)/(height*.35f),4.f))
                +.12f*std::exp(-std::pow((py+height*.5f-1.25f)/.55f,2.f))
                    *std::exp(-std::pow((reflectionPx+width*.20f)/(width*.42f),4.f)));
            const float cloud=.014f*std::sin(px*.17f+std::sin(py*.23f))-.009f*age*(1-t)
                +(sliverOnly?0.f:.035f*std::exp(-std::pow((reflectionPx+width*.20f)/(width*.55f),2.f)-std::pow((py+height*.22f)/(height*.4f),2.f)));
            const float topLeftReflection=shoulderProposal&&!sliverOnly?.07f*(1.f-.88f*shineProgress)
                *std::exp(-std::pow((px+width*.22f)/(width*.18f),4.f)-std::pow((py+height*.30f)/(height*.17f),4.f)):0.f;
            const float dotAge=shineProgress;
            const float reflectionDot=.70f*(1.f-.88f*dotAge)
                *std::exp(-std::pow((px+width*.5f-2.7f)/1.10f,2.f)-std::pow((py+height*.5f-1.6f)/.75f,2.f));
            const float shade=(.73f+.28f*diffuse+specular+grain+cloud+edgeReflection+topLeftReflection+reflectionDot+(over?.009f:0))
                *(1.f-.36f*juce::jmax(0.f,ny));
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
    bool capProposed=false;
    bool capBalancedCorners=false;
    bool capRefinedFinish=false;
    bool capShoulder=false;
    bool capSliverOnly=false;
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
        g.drawLine(x+1,83,313+tuneKnobShift,83,.85f);g.drawLine(369+tuneKnobShift,83,x+w-1,83,.85f);
    } else g.drawLine(x+1,83,x+w-1,83,.85f);
    caption(g,title,{x+3,59,w-6,22},13,juce::Justification::centred,true,ink);
}
void arcLabels(juce::Graphics& g,juce::Point<float> centre,
               const std::vector<std::pair<juce::String,float>>& labels,float radius,float age=0,float angularScale=1.f) {
    const auto ink=agedInk(age);
    for(const auto& [text,rawAngle]:labels) {
        const float angle=rawAngle*angularScale;
        const auto p=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*radius;
        const auto q=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*(radius-4);
        g.setColour(ink);g.drawLine({p,q},.75f);
        if constexpr(KB09_APPROVED_HARDWARE)if(text=="NOISE") {
            // The S/E ink gap sits vertically above the OUTER tick end,
            // as in the reference, rather than along an extrapolated ray.
            juce::GlyphArrangement glyphs;glyphs.addLineOfText(font(7.4f,true),text,0,0);
            juce::Path sInk,eInk,allInk;
            glyphs.getGlyph(3).createPath(sInk);glyphs.getGlyph(4).createPath(eInk);
            for(int i=0;i<glyphs.getNumGlyphs();++i)glyphs.getGlyph(i).createPath(allInk);
            const float gap=(sInk.getBounds().getRight()+eInk.getBounds().getX())*.5f;
            g.setColour(ink);glyphs.draw(g,juce::AffineTransform::translation(p.x-gap,p.y-2.5f-allInk.getBounds().getBottom()));
            continue;
        }
        const float labelGap=KB09_APPROVED_HARDWARE&&text.endsWith("'")?4.f:7.f;
        auto label=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*(radius+labelGap);
        // The neighbouring 2-foot and NOISE detents share the space between
        // selectors. Place NOISE above its knob while preserving the tick.
        if(text=="NOISE") {
            label.y-=KB09_APPROVED_HARDWARE?5.f:6.f;
            if constexpr(KB09_APPROVED_HARDWARE)label.x+=4.f;
        }
        if(text=="SAW"||text=="SQR"||text=="PULSE")
            panelGlyph(g,text=="SAW"?PanelGlyph::saw:text=="SQR"?PanelGlyph::square:PanelGlyph::pulse,
                       {label.x-7.5f,label.y-4,15,8},ink,.95f);
        else caption(g,text,{label.x-21,label.y-7,42,14},text=="NOISE"?7.4f:8.4f,juce::Justification::centred,true,ink);
    }
}
}

void paintPanelMaterial(juce::Graphics& g,float age,const juce::NamedValueSet& properties,int keyboardRight) {
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
        if(bool(properties.getWithDefault("previewScrews84",true))) {
            juce::Graphics::ScopedSaveState repair(g);g.addTransform(juce::AffineTransform::translation(0,float(headerH)));
            int index=0;
            for(const auto p:{juce::Point<float>(88,37),{1686,37},{88,328},{1686,328},{83,399},{1692,399}})
                paintRepairedScrewSeat(g,{(p.x-2)*float(designW)/1770,(p.y-7)*float(bodyH)/873},age,index++);
        }
        for(int side=0;side<2;++side)(bool(properties.getWithDefault("previewCheekEdges79",false))?kb09_gui::cheekEdgeProposal():kb09_gui::cheekMaterial(bool(properties.getWithDefault("previewCornerGouges64",true)))).paint(g,side,age,float(headerH),float(designW),
            bool(properties.getWithDefault("previewCheekScuffs17",true)),bool(properties.getWithDefault("previewCheekContrast82",false)),KB09_APPROVED_HARDWARE?6.f:0.f);
        {LivePaintScope t{"stains"};caseStains(g,age);}
        {LivePaintScope t{"panelDust"};paintPanelDust(g,age);}
        if(bool(properties.getWithDefault("previewDetail135",bool(KB09_APPROVED_HARDWARE)))){LivePaintScope t{"extraDust"};paintExtraDust135(g,age);}
        if(bool(properties.getWithDefault("previewFullRings144",bool(KB09_APPROVED_HARDWARE)))){LivePaintScope t{"closeRings"};paintCloseKnobRings144(g,age);}
        // The narrow rail below the keys has a fine pebble finish. It is a
        // different material from the smooth painted main panel.
        {
            juce::Graphics::ScopedSaveState save(g);
            // End at the visible outer edge of the last ivory key.
            const auto rail=juce::Rectangle<float>(59.25f,float(headerH+708),1375.75f,24.0f)
                .withRight(juce::jmin(float(keyboardRight)-.65f,float(designW)-59.25f));
            // Unlit case below the rail must not read as a tall front face.
            g.setColour(juce::Colour(0xff070909));
            g.fillRect(59.0f,rail.getY(),1365.0f,float(headerH+visibleBodyH)-rail.getY());
            g.reduceClipRegion(rail.toNearestInt());
            g.drawImage(art.rail,rail.getX(),rail.getY(),rail.getWidth(),rail.getHeight(),0,0,art.rail.getWidth(),70);
            g.setColour(juce::Colours::black.withAlpha(.27f+age*.08f));g.fillRect(rail);
            g.setOpacity(std::pow(age,1.25f));g.drawImage(railDust(),rail);g.setOpacity(1);
            // A shallow key-front shadow meets the textured surface without
            // the old gap left by the exaggerated vertical key faces.
            if constexpr(KB09_APPROVED_HARDWARE) {
                // A visible cast shadow extends beyond the immediate contact
                // seam. Keep rail texture and dust beneath the translucent shade.
                constexpr float shadowDepth=9.f;
                juce::ColourGradient seam(juce::Colours::black.withAlpha(.72f),0,rail.getY(),
                    juce::Colours::transparentBlack,0,rail.getY()+shadowDepth,false);
                seam.addColour(.34,juce::Colours::black.withAlpha(.62f));
                seam.addColour(.70,juce::Colours::black.withAlpha(.28f));
                g.setGradientFill(seam);g.fillRect(rail.withHeight(shadowDepth));
            } else {
                g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.42f),0,rail.getY(),juce::Colours::transparentBlack,0,rail.getY()+3,false));
                g.fillRect(rail.withHeight(3));
            }
            juce::ColourGradient fold(juce::Colours::transparentWhite,0,rail.getBottom()-1.7f,
                juce::Colour(0xffbfc2b9).withAlpha(.25f-age*.05f),0,rail.getBottom()-.3f,false);
            fold.addColour(.72,juce::Colour(0xffbfc2b9).withAlpha(.08f));
            g.setGradientFill(fold);g.fillRect(rail.withTop(rail.getBottom()-1.7f));
        }
        // The joint is above the rail in the paint order; no pebble texture or
        // rail dust may bridge the recessed cheek-to-case separation.
        if(bool(properties.getWithDefault("previewCheekJoint74",true)))
            kb09_gui::paintCheekJoint(g,age,float(headerH),float(designW));
        if(bool(properties.getWithDefault("previewGapDust136",bool(KB09_APPROVED_HARDWARE))))kb09_gui::paintLowerBayJointDust(g,age,float(headerH),float(designW));
}
struct PanelMaterialBank {
    std::mutex mutex;
    std::array<juce::Image,5> images;
    std::array<bool,5> ready{};
    std::atomic<bool> cancelled{false},failed{false};
    std::atomic<int> requested{0};
    juce::String key;
    float scale=1;
};
class SH09Editor::Panel final : public juce::Component, private juce::Timer, private juce::KeyListener {
public:
    std::function<void(bool)> audioSettingsObserver;
    std::function<void(int)> midiChannelObserver;
    void bufferUnchangedHardware() {
        bufferInSoftware(keyboard,true);bufferInSoftware(bender,true);bufferInSoftware(power,true);
        for(auto& c:controls) {
            // Cache expensive knob material; lightweight fader caps stay live.
            if(c->slider)bufferInSoftware(*c->slider,c->slider->isRotary()&&!bool(c->slider->getProperties().getWithDefault("choiceSnapActive",false)));
            if(c->selector)bufferInSoftware(*c->selector,true);
        }
    }
    void cacheForResize() {
        if(!resizeBackdrop.isValid()) {
            resizeBackdrop=juce::Image(juce::Image::RGB,designW,canvasH,true,juce::SoftwareImageType());
            renderingCondition=true;{juce::Graphics frame(resizeBackdrop);paint(frame);}renderingCondition=false;
        }
        resizeCacheUntil=juce::Time::getMillisecondCounterHiRes()+180;
        bufferInSoftware(keyboard,true);bufferInSoftware(bender,true);bufferInSoftware(power,true);
        for(auto& c:controls) {if(c->slider)bufferInSoftware(*c->slider,true);if(c->selector)bufferInSoftware(*c->selector,true);}
    }
    Panel(SH09Processor& p,sh09::WindowSizing& s)
        :processor(p),sizing(s),presetFiles(p,*this),keyboard(p),header(s),maximise(s),power(p),meter(p),rateLamp(p) {
        setLookAndFeel(&look);setSize(designW,canvasH);
        getProperties().set("topStripReduction",topStripReduction);
        addAndMakeVisible(header);header.setBounds(0,0,designW,headerH);
        addAndMakeVisible(updates);updates.setBounds(25,5,132,24);updates.setButtonText("TOYLAND / KB-09");updates.setComponentID("updates");updates.setTooltip("Check for updates.");updates.onClick=[this]{kb09::Updates::instance().showMenu(&updates);};
        addAndMakeVisible(keyboard);keyboard.setComponentID("keyboard");keyboard.setBounds(326,headerH+370,1102,337);
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
        large.setTooltip("Large: fill this screen's available work area.");large.onClick=[this]{sizing.chooseLarge();};
        if(processor.wrapperType==juce::AudioProcessor::wrapperType_Standalone) {
            addAndMakeVisible(midiMapButton);midiMapButton.setComponentID("midiMap");
            midiMapButton.setButtonText("MIDI MAP");midiMapButton.setBounds(1255,5,80,24);
            midiMapButton.setClickingTogglesState(true);midiMapButton.setWantsKeyboardFocus(false);
            midiMapButton.setColour(juce::TextButton::buttonOnColourId,amber);
            midiMapButton.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff151a1d));
            midiMapButton.setTooltip("Click MIDI MAP, select a highlighted control, tap a button or move a knob/fader, then click DONE. Mappings are remembered. CLEAR ALL resets them.");
            midiMapButton.onClick=[this]{setMapping(midiMapButton.getToggleState());};
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
                if(auto* holder=juce::StandalonePluginHolder::getInstance()) {
                    if(audioSettingsObserver)audioSettingsObserver(true);
                    const auto observer=audioSettingsObserver;
                    kb09_gui::AudioSettingsDialog::show(holder->deviceManager,this,
                        holder->getProcessorHasPotentialFeedbackLoop()?&holder->shouldMuteInput:nullptr,
                        [observer]{if(observer)observer(false);},processor.standaloneMidiChannel(),
                        [safe=juce::Component::SafePointer<Panel>(this)](int channel) {
                            if(safe==nullptr)return;
                            safe->processor.setStandaloneMidiChannel(channel);
                            if(safe->midiChannelObserver)safe->midiChannelObserver(channel);
                        });
                }
            };
        }
        if(processor.wrapperType!=juce::AudioProcessor::wrapperType_Standalone)addAndMakeVisible(maximise);
        maximise.setBounds(1428,0,47,34);maximise.setComponentID("maximise");
        sizing.onChange=[this]{header.repaint();maximise.repaint();};
        addSwitch("lfoWave",modulatorSwitchX,104,modulatorSwitchWidth,121,{0,1,2},{"SINE","SQUARE","RANDOM\n(S/H)"},"Modulator: sine, square or random sample-and-hold.");
        addFader("lfoRate","RATE",240-topScaleBorderDistance-36,96,33,129);controls.back()->label->setVisible(false);addFader("lfoDelay","DELAY",240-topScaleBorderDistance,96,33,129);
        controls.back()->label->setVisible(false); // Sine + DELAY TIME is printed by the panel.
        addAndMakeVisible(rateLamp);rateLamp.setComponentID("rateLamp");if constexpr(KB09_APPROVED_HARDWARE)rateLamp.setBounds(modulatorSwitchX+(modulatorSwitchWidth-11)/2,headerH+100,11,10);
        else rateLamp.setBounds(modulatorSwitchX+(modulatorSwitchWidth-9)/2,headerH+101,9,8);rateLamp.setInterceptsMouseClicks(false,false);
        addFader("vcoMod","MOD",244+topScaleBorderDistance,96,45,129);
        controls.back()->slider->getProperties().set("scaleLeft",true);
        addKnob("range",rangeKnobCentre,163,87);addKnob("wave",waveKnobCentre,163,87);addKnob("tune",rangeKnobCentre,86,22);
        addFader("pulseWidth","PULSE\nWIDTH",pulseWidthCentre-2,96,37,129);
        controls.back()->label->setVisible(false); // Shared PULSE WIDTH MOD bracket replaces this caption.
        addSwitch("pwmMode",pwmSwitchX+2,104,51,121,{1,0,2},{"LFO","MANUAL","ENV"},"Pulse width: LFO, manual or envelope. LFO uses the undelayed sine.");
        addSwitch("subMode",633,104,60,121,{0,1,2},{"1 OCT\nDOWN","2 OCT\nDOWN","2 OCT\nDOWN"},"Sub oscillator: one octave down square, two octaves down square, or two octaves down pulse.");
        addFader("vcoLevel","VCO",697+topScaleBorderDistance,96,37,129);addFader("subLevel","SUB\nOSC",770,96,37,129);addFader("extLevel","EXT",843-topScaleBorderDistance,96,37,129);
        addFader("cutoff","CUTOFF\nFREQ",KB09_APPROVED_HARDWARE?882:884,96,33,129);
        controls.back()->label->setBounds(866,headerH+89,36,20);
        controls.back()->label->setFont(font(8.8f,true));
        addFader("resonance","RESONANCE",KB09_APPROVED_HARDWARE?921:919,96,33,129);
        controls.back()->label->setBounds(903,headerH+95,52,20);
        controls.back()->label->setFont(font(9.2f,true));
        addSwitch("vcfEnvMode",938,104,61,121,{2,0,1},{"ENV\nFOL'R","ENV +","ENV -"},"ENV + and ENV - use the synth's envelope. ENV FOL'R uses external input level for an auto-wah effect.",true);
        addFader("vcfEnv","ENV",1098-topScaleBorderDistance-36,96,35,129);controls.back()->label->setVisible(false);addFader("vcfMod","MOD",1098-topScaleBorderDistance,96,35,129);
        addSwitch("vcaMode",1110,104,56,121,{0,1,2},{"HOLD","ENV","GATE"},"HOLD passes audio without MIDI; ENV uses ADSR; GATE follows held keys.");
        // Balance hardware margins: left edge of switch track to divider,
        // right edge of RELEASE track to divider. Move labels/hit areas too.
        constexpr int envContentOffset=-7;
        addSwitch("envMode",1170+envContentOffset,104,55,121,{0,1,2},{"GATE +\nTRIG","GATE","LFO"},"Gate+Trigger re-attacks on low-note changes. Gate preserves legato. LFO repeats while held.",true);
        addFader("attack","A",1403-topScaleBorderDistance-114,96,31,129,true);addFader("decay","D",1403-topScaleBorderDistance-76,96,31,129,true);
        addFader("sustain","S",1403-topScaleBorderDistance-38,96,31,129,true);addFader("release","R",1403-topScaleBorderDistance,96,31,129,true);
        // One native printed row keeps single- and two-line control names on
        // the same baseline without Label fitting shrinking longer captions.
        for(auto& c:controls)if(c->label&&c->slider) {
            const auto id=c->slider->getComponentID();
            if(juce::StringArray{"vcoMod","vcoLevel","subLevel","extLevel","cutoff","resonance","vcfMod","attack","decay","sustain","release"}.contains(id))
                c->label->setVisible(false);
        }
        addKnob("volume",140,442+volumeOffsetY-volumeKnobLift,88);
        addAndMakeVisible(power);power.setBounds(231,headerH+398+powerOffsetY-powerHardwareLift,KB09_APPROVED_HARDWARE?56:50,KB09_APPROVED_HARDWARE?52:44);power.onPowerChanged=[this]{
            powerLampFade.update(processor.isPowerEnabled(),juce::Time::getMillisecondCounterHiRes());
            // Let the 20 ms return finish before any idle material-bank work.
            performanceIdleAfter=juce::Time::getMillisecondCounterHiRes()+200;
            repaint(251+performanceBayOffsetX,headerH-topStripReduction+475+powerOffsetY+powerLedDrop,11,11);
            keyboard.repaint();
        };
        addFader("bendVco","VCO",128,496,35,96,true);addFader("bendVcf","VCF",164,496,35,96,true);
        addFader("portamento","PORTAMENTO",256,496,43,96,true);
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
        springAnimation.tick=[this]{advanceBenderReturn();};
        bender.onValueChange=[this]{springAnimation.stopTimer();bendReturnStarted=0;bendActive=std::abs(bender.getValue())>.0001;processor.queuePitchBend(float(bender.getValue()));};
        bender.onDragStart=[this]{springAnimation.stopTimer();benderDragging=true;bendReturnStarted=0;};
        bender.onDragEnd=[this]{
            benderDragging=false;bendActive=false;
            lastBendSerial=processor.displayedPitchBendSerial.load(std::memory_order_acquire);
            bendReturnFrom=bender.getValue();
            const bool modern=processor.state.getRawParameterValue("modern")->load()>=.5f;
            bendReturnStarted=modern?0:juce::Time::getMillisecondCounterHiRes();
            if(modern){springAnimation.stopTimer();bender.setValue(0,juce::dontSendNotification);}
            else springAnimation.startTimer(4);
            processor.queuePitchBendReturn();
        };
        addSwitch("modern",25,designH-headerH-44,80,42,{0,1},{"AUTHENTIC","MODERN"},"Authentic ignores velocity, aftertouch, tempo sync and input trim.");
        // Initial positions; layoutFooterArtwork finalizes all eight groups
        // from their painted extents after both action buttons exist.
        constexpr int footerGap=52;
        int footerX=25+80+footerGap;
        auto footerUtility=[&](const char* id,const char* label,int width) {
            addUtility(id,label,footerX+8+width/2,width);
            footerX+=width+16+footerGap;
        };
        footerUtility("age","CONDITION / AGE",170);
        controls.back()->slider->setRange(0,1,.25);
        controls.back()->slider->onDragStart=[this]{
            look.conditionDragging=true;look.ageIsDeferred=true;look.renderingPreview=true;
            getProperties().set("conditionRenderDeferred",true);
            // Only hide hardware when a complete replacement can be drawn.
            // Automation may keep the first preview bank from warming at all.
            if(bool(getProperties().getWithDefault("conditionPreviewReady",false))&&agePreviews.front().isValid()&&agePreviews.back().isValid()) {
                for(auto* c:getChildren())if(c!=&rateLamp&&c->isVisible()&&c->getY()>=headerH&&c->getBottom()<=headerH+visibleBodyH) {
                    hiddenForCondition.emplace_back(c);c->setVisible(false);
                }
            }
        };
        controls.back()->slider->onDragEnd=[this]{
            look.conditionDragging=false;conditionBackdrop={};previousCondition={};
            ageCommitAfter=juce::Time::getMillisecondCounterHiRes()+80;
            for(auto& c:hiddenForCondition)if(c!=nullptr)c->setVisible(true);
            hiddenForCondition.clear();repaint(0,headerH+visibleBodyH-topStripReduction,designW,50);
        };
        controls.back()->slider->onValueChange=[this]{
            // The parameter and thumb update now; expensive material work waits
            // until the gesture ends, including a short gap for its final paint.
            look.ageIsDeferred=true;look.renderingPreview=true;
            ageCommitAfter=juce::Time::getMillisecondCounterHiRes()+80;
            getProperties().set("conditionRenderDeferred",true);
        };
        footerUtility("velocity","VELOCITY",136);footerUtility("aftertouch","AFTERTOUCH",136);
        addSwitch("lfoSync",footerX,designH-headerH-44,52,42,{0,1},{"FREE","SYNC"},"Modern mode: SYNC makes RATE select straight, dotted and triplet grid divisions.");
        footerX+=52+footerGap;
        footerUtility("extGain","INPUT TRIM",150);
        addAndMakeVisible(descriptions);descriptions.setComponentID("descriptions");
        descriptions.setButtonText("DESCRIPTION");descriptions.setClickingTogglesState(true);
        descriptions.setBounds(footerX,designH-35,139,24);footerX+=139+footerGap;
        descriptions.setColour(juce::TextButton::buttonOnColourId,amber);
        descriptions.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff151a1d));
        descriptions.onClick=[this]{descriptions.setButtonText(descriptions.getToggleState()?"DESCRIPTION ON":"DESCRIPTION");};
        tooltip=std::make_unique<DescriptionTips>(this,descriptions);
        addAndMakeVisible(help);help.setButtonText("EXT AUDIO");help.setBounds(footerX,designH-35,120,24);help.setComponentID("externalAudioHelp");
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
            if(child->getY()>=headerH&&child->getBottom()<headerH+360) {
                child->setTopLeftPosition(child->getX()+topPanelBoundsOffsetX,child->getY());
                child->setTransform(juce::AffineTransform::translation(topPanelFractionX,0));
            }
            else if(child->getY()>=headerH+360&&child->getBottom()<headerH+708)child->setTopLeftPosition(child->getX()+performanceBayOffsetX,child->getY());
        }
        // SH-09 reference: bring each main-panel fader assembly closer to
        // its caption. Printed numerals/scales follow the slider bounds;
        // switches and all approved caption positions remain unchanged.
        for(auto& c:controls)if(c->slider&&c->slider->getSliderStyle()==juce::Slider::LinearVertical
                &&c->slider->getY()<headerH+360)
            c->slider->setTopLeftPosition(c->slider->getX(),c->slider->getY()-10);
        // Match the main-panel caption-to-track spacing while preserving
        // the three performance labels' approved printed positions.
        for(auto& c:controls)if(c->slider) {
            const auto id=c->slider->getComponentID();
            if(id=="bendVco"||id=="bendVcf"||id=="portamento")
                c->slider->setTopLeftPosition(c->slider->getX(),c->slider->getY()-10);
            if(id=="bendVco"||id=="bendVcf") {
                c->slider->setTopLeftPosition(c->slider->getX()-4,c->slider->getY());
                if(c->label)c->label->setTopLeftPosition(c->label->getX()-4,c->label->getY());
            }
        }
        rateLamp.setTopLeftPosition(rateLamp.getX()+topPanelBoundsOffsetX,rateLamp.getY());
        rateLamp.setTransform(juce::AffineTransform::translation(topPanelFractionX,0));
        power.setTopLeftPosition(power.getX()+performanceBayOffsetX,power.getY());
        bender.setTopLeftPosition(bender.getX()+performanceBayOffsetX,bender.getY());
        for(auto* child:{static_cast<juce::Component*>(&small),static_cast<juce::Component*>(&medium),
                static_cast<juce::Component*>(&large),static_cast<juce::Component*>(&maximise)})
            child->setTopLeftPosition(child->getX()-17,child->getY());
        layoutFooterArtwork();
        if constexpr(topStripReduction>0)
            for(auto* child:getChildren())if(child->getY()>=kb09_gui::CompactPanelLayout::secondEnd)
                child->setTransform(child->getTransform().translated(0,-float(topStripReduction)));
        updateModern();
        // Have one complete fallback before the editor can be displayed or
        // dragged. Until idle work builds all five ages, share this snapshot
        // across the bank so a first gesture never renders a bare chassis or
        // forces detailed hardware painting onto every mouse event.
        renderingCondition=true;look.conditionDragging=true;look.renderingPreview=true;
        const auto initialPreview=captureAppearancePreview({0,headerH,designW,visibleBodyH-topStripReduction},900.0f/designW);
        renderingCondition=false;look.conditionDragging=false;look.renderingPreview=false;
        agePreviews.fill(initialPreview);
       #if KB09_APPROVED_HARDWARE
        // Keep tiny changing labels outside the large keyboard texture. A
        // dirty label otherwise forces a full Direct2D keyboard-image upload.
        keyboard.getProperties().set("externalOctaveLabels",true);keyboard.repaint();
        for(size_t i=0;i<octaveLabels.size();++i) {
            auto& label=octaveLabels[i];const float width=float(keyboard.getWidth())/19.f;
            label.setBounds(juce::Rectangle<float>(keyboard.getX()+(4.f+7.f*float(i))*width+2.f,float(keyboard.getBottom()-25),width-4.f,15.f).toNearestInt());
            if constexpr(topStripReduction>0)label.setTransform(juce::AffineTransform::translation(0,-float(topStripReduction)));
            label.setFont(font(10.5f));label.setBorderSize(juce::BorderSize<int>(0));label.setJustificationType(juce::Justification::centred);
            label.setColour(juce::Label::textColourId,juce::Colour(0xff514d43).withAlpha(.87f));label.setInterceptsMouseClicks(false,false);addAndMakeVisible(label);
        }
       #endif
        bufferUnchangedHardware();
        dustWarmup=std::async(std::launch::async,[] {
            (void)panelDust();
            juce::Image scratch(juce::Image::ARGB,1,1,true,juce::SoftwareImageType());juce::Graphics graphics(scratch);
            paintExtraDust135(graphics,1.f);paintCloseKnobRings144(graphics,1.f);
        });
        startTimerHz(60);
    }
    ~Panel() override {
        if(materialBank)materialBank->cancelled=true;
        if(processor.midiMap)processor.midiMap->cancelLearn();
        springAnimation.stopTimer();externalHelp.close();stopTimer();releaseComputerNotes();removeKeyListener(this);sizing.onChange=nullptr;if(bendActive||benderDragging)processor.queuePitchBend(0);setLookAndFeel(nullptr);
    }
    void paint(juce::Graphics& g) override {
        if(!renderingCondition&&resizeBackdrop.isValid()&&resizeCacheUntil>0){g.drawImage(resizeBackdrop,getLocalBounds().toFloat());return;}
        LivePaintScope timing{"panel"};
        if(look.conditionDragging&&!renderingCondition&&bool(getProperties().getWithDefault("conditionPreviewReady",false))&&agePreviews.front().isValid()&&agePreviews.back().isValid()) {
            const int stage=kb09_gui::appearanceStage(processor.state.getRawParameterValue("age")->load());
            const juce::Rectangle<float> body(0,float(headerH),float(designW),float(visibleBodyH-topStripReduction));
            g.setOpacity(1);g.drawImage(agePreviews[size_t(stage)],body);
            {juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::translation(0,-float(topStripReduction)));paintFooter(g);}return;
        }
        if(renderingCondition){
            if constexpr(topStripReduction>0) {
                juce::Image original(juce::Image::ARGB,designW,designH,true,juce::SoftwareImageType());
                {juce::Graphics raster(original);paintChassis(raster);}
                kb09_gui::CompactPanelLayout::draw(g,original);
            } else paintChassis(g);
            return;
        }
        // Hardware movement only exposes the same stationary panel beneath it.
        // Rasterise that material once at the actual display resolution, rather
        // than rebuilding grain, corrosion and photographic patches per repaint.
        const float age=look.ageIsDeferred?look.age:appearanceAge(processor.state.getRawParameterValue("age")->load());
        look.age=age;
        const float scale=juce::jlimit(1.0f,4.0f,g.getInternalContext().getPhysicalPixelScaleFactor());
        const bool screwPreview=bool(getProperties().getWithDefault("previewScrewRotation",true)) != bool(getProperties().getWithDefault("previewScrews73",true));
        const bool cornerPreview=bool(getProperties().getWithDefault("previewCornerGouges64",true));
        const bool socketPreview=bool(getProperties().getWithDefault("previewPanelSockets64",true));
        const bool cheekPreview=bool(getProperties().getWithDefault("previewCheekScuffs17",true));
        const bool jointPreview=bool(getProperties().getWithDefault("previewCheekJoint74",true));
        const bool cheekEdgePreview=bool(getProperties().getWithDefault("previewCheekEdges79",false));
        const bool cheekContrastPreview=bool(getProperties().getWithDefault("previewCheekContrast82",false));
        const bool screwAgingPreview=bool(getProperties().getWithDefault("previewScrewAging77",true));
        const bool powerMountPreview=bool(getProperties().getWithDefault("previewPowerMount20",true));
        const bool powerNudgePreview=bool(getProperties().getWithDefault("previewPowerNudge24",true));
        const bool powerContourPreview=bool(getProperties().getWithDefault("previewContourMount25",true));
        const bool printPreview=bool(getProperties().getWithDefault("previewPrintAlignment35",true));
        const bool screws84=bool(getProperties().getWithDefault("previewScrews84",true));
        if(staticPanelScrews84!=screws84){staticPanel=juce::Image{};staticPanelScrews84=screws84;}
        const bool gap136=bool(getProperties().getWithDefault("previewGapDust136",bool(KB09_APPROVED_HARDWARE)));
        if(staticPanelGapDust136!=gap136){staticPanel={};staticPanelGapDust136=gap136;}
        const bool rings144=bool(getProperties().getWithDefault("previewFullRings144",bool(KB09_APPROVED_HARDWARE)));
        if(staticPanelFullRings144!=rings144){staticPanel={};staticPanelFullRings144=rings144;}
        const bool detail135=bool(getProperties().getWithDefault("previewDetail135",bool(KB09_APPROVED_HARDWARE)));
        if(staticPanelDetail135!=detail135){staticPanel={};staticPanelDetail135=detail135;}
        const bool position131=bool(getProperties().getWithDefault("previewScrewPosition131",bool(KB09_APPROVED_HARDWARE)));
        if(staticPanelPosition131!=position131){staticPanel={};staticPanelPosition131=position131;}
        const bool powerScrewSpacing102=bool(getProperties().getWithDefault("previewPowerScrewSpacing102",bool(KB09_APPROVED_HARDWARE)));
        if(staticPanelPowerScrewSpacing102!=powerScrewSpacing102){staticPanel=juce::Image{};staticPanelPowerScrewSpacing102=powerScrewSpacing102;}
        const bool powerScrewSize105=bool(getProperties().getWithDefault("previewPowerScrewSize105",bool(KB09_APPROVED_HARDWARE)));
        if(staticPanelPowerScrewSize105!=powerScrewSize105){staticPanel=juce::Image{};staticPanelPowerScrewSize105=powerScrewSize105;}
        if(!staticPanel.isValid()||staticPanelAge!=age||staticPanelCornerPreview!=cornerPreview||staticPanelSocketPreview!=socketPreview||staticPanelScrewPreview!=screwPreview||staticPanelCheekPreview!=cheekPreview||staticPanelJointPreview!=jointPreview||staticPanelCheekEdgePreview!=cheekEdgePreview||staticPanelCheekContrastPreview!=cheekContrastPreview||staticPanelScrewAgingPreview!=screwAgingPreview||staticPanelPowerMountPreview!=powerMountPreview||staticPanelPowerNudgePreview!=powerNudgePreview||staticPanelPowerContourPreview!=powerContourPreview||staticPanelPrintPreview!=printPreview||std::abs(staticPanelScale-scale)>.01f) {
            auto rasterKey=juce::String(age,7)+":"+juce::String(scale,4)+":";
            for(const bool flag:{screwPreview,cornerPreview,socketPreview,cheekPreview,jointPreview,cheekEdgePreview,cheekContrastPreview,screwAgingPreview,powerMountPreview,powerNudgePreview,powerContourPreview,printPreview,screws84,gap136,rings144,detail135,position131,powerScrewSpacing102,powerScrewSize105})rasterKey+=flag?"1":"0";
            rasterKey+=processor.state.getRawParameterValue("modern")->load()>.5f?"M":"A";
            const size_t frameBytes=size_t(juce::roundToInt(designW*scale))*size_t(juce::roundToInt(designH*scale))*4u;
            agePanelCache.setBudget(juce::jlimit(size_t(64u*1024u*1024u),size_t(256u*1024u*1024u),frameBytes*5u));
            staticPanel=agePanelCache.find(rasterKey);
            if(!staticPanel.isValid()) {
                const auto bankKey=rasterKey.fromFirstOccurrenceOf(":",false,false);
                const int stage=kb09_gui::appearanceStage(age);
                staticPanel=takePreparedMaterial(bankKey,stage);
                if(!staticPanel.isValid()) {
                    juce::Image original(juce::Image::ARGB,juce::roundToInt(designW*scale),juce::roundToInt(designH*scale),true,juce::SoftwareImageType());
                    {juce::Graphics raster(original);raster.addTransform(juce::AffineTransform::scale(scale));paintPanelMaterial(raster,age,getProperties(),keyboard.getRight());}
                    if constexpr(topStripReduction>0) {
                        staticPanel=juce::Image(juce::Image::ARGB,juce::roundToInt(designW*scale),juce::roundToInt(canvasH*scale),true,juce::SoftwareImageType());
                        juce::Graphics compact(staticPanel);compact.addTransform(juce::AffineTransform::scale(scale));kb09_gui::CompactPanelLayout::draw(compact,original);
                    } else staticPanel=original;
                }
                {juce::Graphics print(staticPanel);print.addTransform(juce::AffineTransform::scale(scale));paintCompactPrinting(print,age);}
                agePanelCache.put(rasterKey,staticPanel);
                beginMaterialBank(bankKey,scale,stage);
                getProperties().set("chassisRasterCount",int(getProperties().getWithDefault("chassisRasterCount",0))+1);
            }
            getProperties().set("agePanelCacheBytes",double(agePanelCache.sizeBytes()));
            staticPanelCornerPreview=cornerPreview;staticPanelSocketPreview=socketPreview;staticPanelAge=age;staticPanelScale=scale;staticPanelScrewPreview=screwPreview;staticPanelCheekPreview=cheekPreview;staticPanelJointPreview=jointPreview;staticPanelCheekEdgePreview=cheekEdgePreview;staticPanelCheekContrastPreview=cheekContrastPreview;staticPanelScrewAgingPreview=screwAgingPreview;staticPanelPowerMountPreview=powerMountPreview;staticPanelPowerNudgePreview=powerNudgePreview;staticPanelPowerContourPreview=powerContourPreview;staticPanelPrintPreview=printPreview;
        }
        drawNativeRaster(g,staticPanel,scale);
        {juce::Graphics::ScopedSaveState lamp(g);
         g.addTransform(juce::AffineTransform::translation(float(performanceBayOffsetX),float(headerH-topStripReduction)));
         paintPowerLamp(g);}
    }
    void beginMaterialBank(const juce::String& key,float scale,int initialStage) {
        const bool enabled=bool(getProperties().getWithDefault("backgroundMaterialWarmup186",isShowing()&&getPeer()!=nullptr));
        if(!enabled||(materialBank&&materialBank->key==key))return;
        if(materialBank)materialBank->cancelled=true;
        auto bank=std::make_shared<PanelMaterialBank>();bank->key=key;bank->scale=scale;bank->ready[size_t(initialStage)]=true;
        materialBank=bank;
        const auto properties=getProperties();const int right=keyboard.getRight();
        materialJobs.erase(std::remove_if(materialJobs.begin(),materialJobs.end(),[](auto& job){
            if(job.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return false;
            job.get();return true;
        }),materialJobs.end());
        materialJobs.push_back(std::async(std::launch::async,[bank,properties,right] {
            try {
                for(int count=0;count<4&&!bank->cancelled;++count) {
                    int stage=bank->requested.load();
                    {std::lock_guard<std::mutex> guard(bank->mutex);if(bank->ready[size_t(stage)]) {
                        stage=0;while(stage<5&&bank->ready[size_t(stage)])++stage;
                    }}
                    if(stage>=5)break;
                    juce::Image image(juce::Image::ARGB,juce::roundToInt(designW*bank->scale),juce::roundToInt(designH*bank->scale),true,juce::SoftwareImageType());
                    {juce::Graphics raster(image);raster.addTransform(juce::AffineTransform::scale(bank->scale));paintPanelMaterial(raster,stage*.25f,properties,right);}
                    if constexpr(topStripReduction>0) {
                        juce::Image compact(juce::Image::ARGB,juce::roundToInt(designW*bank->scale),juce::roundToInt(canvasH*bank->scale),true,juce::SoftwareImageType());
                        {juce::Graphics destination(compact);destination.addTransform(juce::AffineTransform::scale(bank->scale));kb09_gui::CompactPanelLayout::draw(destination,image);}
                        image=compact;
                    }
                    {std::lock_guard<std::mutex> guard(bank->mutex);bank->images[size_t(stage)]=image;bank->ready[size_t(stage)]=true;}
                }
            } catch(...) {bank->failed=true;}
        }));
    }
    juce::Image takePreparedMaterial(const juce::String& key,int stage) {
        if(!materialBank||materialBank->key!=key)return {};
        std::lock_guard<std::mutex> guard(materialBank->mutex);
        auto image=materialBank->images[size_t(stage)];materialBank->images[size_t(stage)]={};return image;
    }
    juce::Image captureAppearancePreview(juce::Rectangle<int> region,float scale) {
        juce::ScopedValueSetter<bool> omitLamp(rateLamp.appearanceSnapshot,true);
        return createComponentSnapshot(region,true,scale,juce::SoftwareImageType());
    }
    void paintPowerLamp(juce::Graphics& g) {
        if constexpr(KB09_APPROVED_HARDWARE) {
            const float level=float(getProperties().getWithDefault("previewPowerLedLevel",
                powerLampFade.level(juce::Time::getMillisecondCounterHiRes())));
            kb09_gui::drawInsetRedLamp(g,{256.f,479.5f+powerOffsetY+powerLedDrop},level);return;
        }
        g.setColour(juce::Colour(0xff080b0e));g.fillEllipse(251.5f,475+powerOffsetY+powerLedDrop,9,9);
        const float level=float(getProperties().getWithDefault("previewPowerLedLevel",
            bool(getProperties().getWithDefault("previewPowerLedFade88",true))?powerLampFade.level(juce::Time::getMillisecondCounterHiRes()):(processor.isPowerEnabled()?1.f:0.f)));
        g.setColour(juce::Colour(0xff491c16).interpolatedWith(juce::Colour(0xffe54c35),level));g.fillEllipse(253.5f,477+powerOffsetY+powerLedDrop,5,5);
        if(level>0) {g.setColour(juce::Colour(0xffffaa6f).withAlpha(.65f*level));g.fillEllipse(254.0f,477.5f+powerOffsetY+powerLedDrop,1.8f,1.8f);}
    }
    void paintChassis(juce::Graphics& g) {
        look.age=previewAge>=0?previewAge:look.ageIsDeferred?look.age:appearanceAge(processor.state.getRawParameterValue("age")->load());
        const float age=look.age;
        paintPanelMaterial(g,age,getProperties(),keyboard.getRight());
        paintChassisPrinting(g,age);
    }
    void paintChassisPrinting(juce::Graphics& g,float age) {
        const auto drawBayScrew=[&](juce::Graphics& target,juce::Point<float> centre,float materialAge,bool lower,int pair,float radius) {
            if(bool(getProperties().getWithDefault("previewPanelSockets64",true))) {
                juce::Graphics::ScopedSaveState save(target);
                target.addTransform(juce::AffineTransform::translation(-centre.x,-centre.y)
                    .scaled(radius/10.f,radius*panelViewCos/10.f).translated(centre.x,centre.y));
                const int head=pair==1?(lower?5:0):(lower?3:1);
                screwOxidation(target,centre,materialAge,5011u+uint32_t(head)*7717u,false,true,pair==0&&lower,bool(getProperties().getWithDefault("previewScrews73",true)),true,bool(getProperties().getWithDefault("previewScrewAging77",true)),bool(getProperties().getWithDefault("previewScrews84",true)));
            } else kb09_gui::drawPowerScrew(target,centre,materialAge,lower,pair,radius);
        };
        const auto ink=agedInk(age);const auto& art=artwork();
        {
            juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::translation(0,float(headerH)));
            {juce::Graphics::ScopedSaveState topPrint(g);g.addTransform(juce::AffineTransform::translation(topPanelOffsetX,0));
            module(g,95,145,"MODULATOR",age);module(g,244,373,"VCO",age);module(g,621,72,"SUB OSC",age);
            module(g,697,146,"AUDIO MIXER",age);module(g,847,251,"VCF",age);module(g,1102,64,"VCA",age);module(g,1170,233,"ENV",age);
            caption(g,juce::String::fromUTF8("UNOFFICIAL PERSONAL PROJECT \xe2\x80\x94 NOT AFFILIATED WITH ROLAND CORPORATION \xe2\x80\x94 NOT FOR SALE"),
                    {107,24,1284,15},10.0f,juce::Justification::centred,true,ink);
            constexpr float controlCaptionSize=8.4f;
            const auto rowCaption=[&](const char* text,float centre,float width=60.f,float y=99.f) {
                caption(g,text,{centre-width*.5f,y,width,12},controlCaptionSize,juce::Justification::centred,true,ink);
            };
            // Preserve the existing component label centers, including the
            // integer component bounds offset and odd-width half pixels.
            const auto labelCentre=[&](const char* id) {
                for(const auto& c:controls)if(c->label&&c->slider&&c->slider->getComponentID()==id)
                    return float(c->label->getX())+float(c->label->getWidth())*.5f-float(topPanelBoundsOffsetX);
                jassertfalse;return 0.f;
            };
            rowCaption("RATE",169,29);
            juce::GlyphArrangement rateInk;
            const auto rateBox=juce::Rectangle<float>(154.5f,99,29,12).toNearestInt();
            rateInk.addFittedText(font(controlCaptionSize,true),"RATE",float(rateBox.getX()),float(rateBox.getY()),
                                 float(rateBox.getWidth()),float(rateBox.getHeight()),juce::Justification::centred,1,.8f);
            // The lamp's painted ellipse is six units across, inside its hit box.
            const float lampRight=float(rateLamp.getX())+float(rateLamp.getWidth())*.5f+(KB09_APPROVED_HARDWARE?4.5f:3.f)-float(topPanelBoundsOffsetX);
            const float linkCentre=(lampRight+rateInk.getBoundingBox(0,-1,true).getX())*.5f;
            g.setColour(ink);g.drawLine(linkCentre-10.5f,105,linkCentre+10.5f,105,.85f);
            caption(g,"DELAY",{197,87,29,12},controlCaptionSize,juce::Justification::centred,true,ink);
            rowCaption("TIME",211.5f,29);
            panelGlyph(g,PanelGlyph::sine,{184,95.5f,12,7},ink,.85f);
            rowCaption("MOD",labelCentre("vcoMod"));
            rowCaption("RANGE",rangeKnobCentre,74);
            rowCaption("WAVEFORM",waveKnobCentre,84);
            rowCaption("VCO",labelCentre("vcoLevel"));
            caption(g,"SUB",{labelCentre("subLevel")-25.f,89,50,12},controlCaptionSize,juce::Justification::centred,true,ink);
            rowCaption("OSC",labelCentre("subLevel"));
            rowCaption("EXT",labelCentre("extLevel"));
            if constexpr(KB09_APPROVED_HARDWARE) {
                // Centre the visible lettering on the slot, rather than the
                // older manually positioned label box (1.5 units to its right).
                float centre=labelCentre("cutoff");
                for(const auto& c:controls)if(c->slider&&c->slider->getComponentID()=="cutoff")
                    centre=c->slider->getX()+c->slider->getWidth()*.5f-float(topPanelBoundsOffsetX);
                const auto printCutoffLine=[&](const char* text,float y) {
                    juce::GlyphArrangement glyphs;
                    glyphs.addFittedText(font(controlCaptionSize,true),text,0,0,36,12,juce::Justification::centred,1,.8f);
                    juce::Graphics::ScopedSaveState alignment(g);
                    g.addTransform(juce::AffineTransform::translation(centre-glyphs.getBoundingBox(0,-1,true).getCentreX(),y));
                    g.setColour(ink);glyphs.draw(g);
                };
                printCutoffLine("CUTOFF",89);printCutoffLine("FREQ",99);
            } else {
                caption(g,"CUTOFF",{labelCentre("cutoff")-18.f,89,36,12},controlCaptionSize,juce::Justification::centred,true,ink);
                rowCaption("FREQ",labelCentre("cutoff"),36);
            }
            rowCaption("RESONANCE",labelCentre("resonance"),64);
            rowCaption("MOD",labelCentre("vcfMod"));
            rowCaption("A",labelCentre("attack"),31);rowCaption("D",labelCentre("decay"),31);
            rowCaption("S",labelCentre("sustain"),31);rowCaption("R",labelCentre("release"),31);
            caption(g,"TUNE",{315.f+tuneKnobShift,75.5f,30,15},8.4f,juce::Justification::centred,true,ink);
            arcLabels(g,{float(rangeKnobCentre),163},{{"32'",-1.134464f},{"16'",-.567232f},{"8'",0.0f},{"4'",.567232f},{"2'",1.134464f}},KB09_APPROVED_HARDWARE?40.5f:40.f,age,rangeDetentScale);
            arcLabels(g,{float(waveKnobCentre),163},{{"NOISE",-1.047198f},{"SAW",-.349066f},{"SQR",.349066f},{"PULSE",1.047198f}},KB09_APPROVED_HARDWARE?40.5f:40.f,age,waveDetentScale);
            const bool alignedPrint=bool(getProperties().getWithDefault("previewPrintAlignment35",true));
            auto printedSliderAxis=[&](const char* id,float fallback) {
                for(const auto& c:controls)if(c->slider&&c->slider->getComponentID()==id) {
                    const auto r=look.getSliderLayout(*c->slider).sliderBounds;
                    return float(c->slider->getX()+r.getX())+float(r.getWidth())*.5f-float(topPanelBoundsOffsetX);
                }
                return fallback;
            };
            float pwmCentre=pwmCaptionCentre;
            if(alignedPrint) {
                juce::GlyphArrangement digits,lfo;
                const auto axis=printedSliderAxis("pulseWidth",pulseWidthCentre+.5f);
                // Match the actual scale numeral and switch-label glyph ink,
                // including the odd-width fader and clamped axial label box.
                digits.addFittedText(font(6.6f),"10",std::floor(axis)-29.f,0,11,8,juce::Justification::centredRight,1,.8f);
                lfo.addFittedText(font(7.6f,true),"LFO",pwmSwitchX,0,28,22,juce::Justification::centred,1,.8f);
                pwmCentre=(digits.getBoundingBox(0,-1,true).getX()+lfo.getBoundingBox(0,-1,true).getRight())*.5f;
            }
            caption(g,"PULSE WIDTH MOD",{pwmCentre-41.5f,99,83,12},controlCaptionSize,juce::Justification::centred,true,ink);
            const float pwmCaptionLeft=pwmCentre-juce::GlyphArrangement::getStringWidth(font(controlCaptionSize,true),"PULSE WIDTH MOD")*.5f;
            constexpr float pwmRuleY=105.f;
            const float pulseAngle=1.047198f*waveDetentScale;
            const float pulseSymbolX=waveKnobCentre+std::sin(pulseAngle)*47.5f;
            const float pulseSymbolTop=163.f-std::cos(pulseAngle)*47.5f-4.f*.72f;
            const float linkX=KB09_APPROVED_HARDWARE?pulseSymbolX:499.f;
            const float linkBottom=KB09_APPROVED_HARDWARE?pulseSymbolTop-1.8f:131.f;
            juce::Path pwmLink;pwmLink.startNewSubPath(linkX,linkBottom);pwmLink.lineTo(linkX,pwmRuleY+4);pwmLink.quadraticTo(linkX,pwmRuleY,linkX+4,pwmRuleY);pwmLink.lineTo(pwmCaptionLeft-.8f,pwmRuleY);
            g.setColour(ink);g.strokePath(pwmLink,juce::PathStrokeType(.9f));
            const float vcfSliderAxis=alignedPrint?printedSliderAxis("vcfEnv",1035.5f):1030.f;
            juce::Path vcfLink;vcfLink.startNewSubPath(987,116);vcfLink.lineTo(987,106);vcfLink.quadraticTo(987,103,990,103);vcfLink.lineTo(vcfSliderAxis-3,103);vcfLink.quadraticTo(vcfSliderAxis,103,vcfSliderAxis,106);vcfLink.lineTo(vcfSliderAxis,115);
            g.strokePath(vcfLink,juce::PathStrokeType(.85f));
            
            }
            // Original user marks: split into manufacturer on right and instrument on left.
            const float printWear=age*age;
            for(int i=0;i<2;++i) {
                g.setColour(ink.withAlpha(i==0?1-printWear:printWear));
                const auto& printed=i==0?art.marks:art.marksAged;
                g.drawImage(printed,95+topPanelOffsetX,270.4f,328,41.6f,846,308,1253,158,true);
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
            {juce::Graphics::ScopedSaveState bayPrint(g);g.addTransform(juce::AffineTransform::translation(float(performanceBayOffsetX),0));
            g.setColour(ink.withAlpha(.82f));
            // SH-09 outer side rules are open at the bottom; the inner bender
            // outline below supplies the single horizontal printed boundary.
            g.drawLine(69,370,69,702,1);g.drawLine(302,370,302,702,1);
            g.drawLine(211.5f,370,211.5f,484,.85f);
            caption(g,"VOLUME",{96,376+volumeOffsetY,88,13},8.4f,juce::Justification::centred,true,ink);
            for(int i=0;i<=10;++i) {
                const float a=juce::MathConstants<float>::pi*(1.25f+1.5f*float(i)/10);
                const juce::Point<float> c(140,442+volumeOffsetY-volumeKnobLift),direction(std::sin(a),-std::cos(a));
                const float scale=KB09_APPROVED_HARDWARE?.9f:1.f;
                g.setColour(ink);g.drawLine({c+direction*(41*scale),c+direction*(47*scale)},1.0f);
                if(i==0||i==10) {const auto p=c+direction*(53*scale);caption(g,juce::String(i),{p.x-10,p.y-5,20,10},7.5f,juce::Justification::centred,true,ink);}
            }
            {juce::Graphics::ScopedSaveState powerPrint(g);g.addTransform(juce::AffineTransform::translation(51,float(powerOffsetY-powerLegendLift)));
             const float gapCentre=((449-powerHardwareLift+5)+performanceScrewRadius*.981627f+475+powerLedDrop)*.5f;
             kb09_gui::drawPowerLegend(g,ink,age,gapCentre);}
            if(renderingCondition)paintPowerLamp(g);
            g.setColour(ink.withAlpha(.76f));g.drawRoundedRectangle(214.0f,489.0f,81.0f,103.0f,3.0f,.8f);
            juce::Path bendOutline;bendOutline.startNewSubPath(75,489);bendOutline.lineTo(209,489);
            bendOutline.lineTo(209,597);bendOutline.lineTo(295,597);bendOutline.lineTo(295,699);
            bendOutline.lineTo(75,699);bendOutline.closeSubPath();
            g.strokePath(bendOutline.createPathWithRoundedCorners(2.5f),juce::PathStrokeType(.8f));
            juce::Path bendLink;bendLink.startNewSubPath(124,573);bendLink.lineTo(124,598);bendLink.quadraticTo(124,601,127,601);bendLink.lineTo(182,601);bendLink.quadraticTo(185,601,185,604);bendLink.lineTo(185,624);
            bendLink.startNewSubPath(160,573);bendLink.lineTo(160,601);
            g.setColour(ink);g.strokePath(bendLink,juce::PathStrokeType(.9f));
            caption(g,"BENDER",{133,628,103,13},10,juce::Justification::centred,true,ink);
            g.setColour(ink);g.drawLine(113,682,178,682,.9f);g.drawLine(191,682,256,682,.9f);
            juce::Path arrows;arrows.startNewSubPath(108,682);arrows.lineTo(114,679.5f);arrows.lineTo(114,684.5f);arrows.closeSubPath();arrows.startNewSubPath(261,682);arrows.lineTo(255,679.5f);arrows.lineTo(255,684.5f);arrows.closeSubPath();g.fillPath(arrows);
            // Both signs share the very same glyph crossbar, so width and
            // thickness agree exactly without relying on the shorter hyphen.
            juce::GlyphArrangement plusGlyph;plusGlyph.addFittedText(font(12,true),"+",262,675,14,14,juce::Justification::centred,1,.8f);
            juce::Path plusPath;plusGlyph.createPath(plusPath);
            const auto plusBounds=plusPath.getBounds();
            const juce::Rectangle<float> crossbar(plusBounds.getX(),plusBounds.getCentreY()-.7f,plusBounds.getWidth(),1.4f);
            g.fillRect(crossbar.translated(-168,0));
            caption(g,"0",{178,676,13,12},8,juce::Justification::centred,true,ink);
            g.fillRect(crossbar);
            g.fillRect(juce::Rectangle<float>(plusBounds.getCentreX()-.7f,plusBounds.getY(),1.4f,plusBounds.getHeight()));
            drawBayScrew(g,{108,637},age,false,1,performanceScrewRadius);
            drawBayScrew(g,{262,637},age,true,1,performanceScrewRadius);
            }
            uint32_t screwSeed=5011;
            for(const auto p:{juce::Point<float>(88,37),{1686,37},
                              {88,328},{1686,328},{83,399},{1692,399}}) {
                screwOxidation(g,{(p.x-2)*float(designW)/1770,(p.y-7)*float(bodyH)/873+(p.y>790?-40.0f:0.0f)},age,screwSeed,
                    bool(getProperties().getWithDefault("previewScrewRotation",true)),
                    bool(getProperties().getWithDefault("previewPanelSockets64",true)),false,bool(getProperties().getWithDefault("previewScrews73",true)),false,bool(getProperties().getWithDefault("previewScrewAging77",true)),bool(getProperties().getWithDefault("previewScrews84",true)),bool(getProperties().getWithDefault("previewScrewPosition131",bool(KB09_APPROVED_HARDWARE))));
                screwSeed+=7717;
            }
            const float powerMountDrop=bool(getProperties().getWithDefault("previewPowerMount20",true))?15.5f*panelViewSin:0.f;
            const float powerMountNudge=(bool(getProperties().getWithDefault("previewPowerNudge24",true))?.65f:0.f)
                +(bool(getProperties().getWithDefault("previewContourMount25",true))?1.45f:0.f);
            const float powerScrewInset=KB09_APPROVED_HARDWARE?-.6f:
                bool(getProperties().getWithDefault("previewPowerScrewSpacing102",false))?3.4f:0.f;
            // Match the bender screw heads and move both power seats outward
            // equally, preserving the pair's existing centre.
            const float powerRadius=KB09_APPROVED_HARDWARE?performanceScrewRadius:
                performanceScrewRadius*(bool(getProperties().getWithDefault("previewPowerScrewSize105",false))?.9f:1.f);
            drawBayScrew(g,{float(256+performanceBayOffsetX),401+powerOffsetY-powerHardwareLift+powerMountDrop+powerMountNudge-4.f+powerScrewInset},age,false,0,powerRadius);
            drawBayScrew(g,{float(256+performanceBayOffsetX),449+powerOffsetY-powerHardwareLift+5.f-powerScrewInset},age,true,0,powerRadius);
        }
        for(auto& c:controls)if(c->slider&&c->slider->getSliderStyle()==juce::Slider::LinearVertical)
            look.drawFaderScale(g,*c->slider);
        paintFooter(g);
    }
    void paintCompactPrinting(juce::Graphics& g,float age) {
        if constexpr(topStripReduction>0) {
            int destination=0;
            for(const auto segment:{juce::Range<int>(0,kb09_gui::CompactPanelLayout::firstCut),juce::Range<int>(kb09_gui::CompactPanelLayout::firstEnd,kb09_gui::CompactPanelLayout::secondCut),juce::Range<int>(kb09_gui::CompactPanelLayout::secondEnd,designH)}) {
                juce::Graphics::ScopedSaveState band(g);
                g.reduceClipRegion(juce::Rectangle<int>(0,destination,designW,segment.getLength()));
                g.addTransform(juce::AffineTransform::translation(0,float(destination-segment.getStart())));
                paintChassisPrinting(g,age);destination+=segment.getLength();
            }
        } else paintChassisPrinting(g,age);
    }
private:
    void layoutFooterArtwork() {
        // Measure actual paint on transparent rasters, including the cap at
        // BOTH travel limits. Empty label and hit-area padding cannot add gaps.
        // Run once before disabling Modern-only controls, so both modes share
        // the same layout. No parameter or slider value is changed to measure it.
        struct Group {std::vector<juce::Component*> members;int left=designW*4,right=0;};
        std::array<Group,8> groups;
        const std::array<const char*,6> ids{"modern","age","velocity","aftertouch","lfoSync","extGain"};
        for(size_t i=0;i<ids.size();++i)for(auto& c:controls) {
            auto* control=c->slider?static_cast<juce::Component*>(c->slider.get()):static_cast<juce::Component*>(c->selector.get());
            if(control->getComponentID()==ids[i]) {
                groups[i].members.push_back(control);
                if(c->label)groups[i].members.push_back(c->label.get());
            }
        }
        groups[6].members.push_back(&descriptions);groups[7].members.push_back(&help);
        for(auto& group:groups)for(auto* member:group.members) {
            auto image=member->createComponentSnapshot(member->getLocalBounds(),true,4.f,juce::SoftwareImageType());
            if(auto* slider=dynamic_cast<juce::Slider*>(member)) {
                juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale(4.f));
                const auto r=look.getSliderLayout(*slider).sliderBounds;
                for(const float pos:{float(r.getX()),float(r.getRight())})
                    look.drawLinearSlider(g,r.getX(),r.getY(),r.getWidth(),r.getHeight(),pos,0,0,slider->getSliderStyle(),*slider);
            }
            const juce::Image::BitmapData pixels(image,juce::Image::BitmapData::readOnly);
            for(int x=0;x<image.getWidth();++x)for(int y=0;y<image.getHeight();++y)
                if(pixels.getPixelColour(x,y).getAlpha()>=64) {
                    group.left=juce::jmin(group.left,member->getX()*4+x);
                    group.right=juce::jmax(group.right,member->getX()*4+x+1);break;
                }
        }
        float total=0;for(const auto& group:groups)total+=float(group.right-group.left)/4;
        constexpr float inset=25;
        const float gap=(designW-2*inset-total)/float(groups.size()-1);
        float x=inset;
        for(const auto& group:groups) {
            const int shift=juce::roundToInt(x-float(group.left)/4);
            for(auto* member:group.members)member->setTopLeftPosition(member->getX()+shift,member->getY());
            x+=float(group.right-group.left)/4+gap;
        }
    }
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
    std::array<juce::Label,3> octaveLabels;
    kb09_gui::PowerLampFade powerLampFade{processor.isPowerEnabled()};
    bool powerLampWasFading=false;
    juce::ComboBox presets;juce::TextButton previous,next,small,medium,large,help,savePreset,loadPreset,descriptions,audioMidi,computerKeyboard,updates,midiMapButton;bool updateOpened=false;
    std::unique_ptr<kb09_gui::MidiMapOverlay> mappingOverlay;unsigned lastMappingRevision=0;bool lastMappingLearn=false;std::uint64_t lastLearnProgress=0;
public:
    void layoutMapping() {
        if(!mappingOverlay)return;
        auto* editor=findParentComponentOfClass<SH09Editor>();if(!editor)return;
        // Dialog UI lives in the unscaled editor, alongside the scaled canvas.
        // Its text therefore follows the same desktop DPI as native alerts.
        if(mappingOverlay->getParentComponent()!=editor)editor->addChildComponent(*mappingOverlay);
        mappingOverlay->setBounds(editor->getLocalBounds());
        std::vector<kb09_gui::MidiMapOverlay::Target> targets;
        for(auto& c:controls) {
            auto* component=c->slider?static_cast<juce::Component*>(c->slider.get()):static_cast<juce::Component*>(c->selector.get());
            targets.push_back({mappingOverlay->getLocalArea(component,component->getLocalBounds()),sh09::StandaloneMidiMap::indexFor(component->getComponentID())});
        }
        mappingOverlay->setTargets(std::move(targets));
        mappingOverlay->setPassThrough(mappingOverlay->getLocalArea(&header,header.getLocalBounds()),
            mappingOverlay->getLocalArea(&descriptions,descriptions.getLocalBounds()));
        mappingOverlay->setMessageArea(mappingOverlay->getLocalArea(&keyboard,keyboard.getLocalBounds()));
        mappingOverlay->setCondition(look.age);
    }
private:
    void setMapping(bool enabled) {
        if(!processor.midiMap)return;
        if(enabled)processor.midiMap->cancelLearn();else processor.midiMap->finishLearn();
        midiMapButton.setToggleState(enabled,juce::dontSendNotification);
        if(!enabled){if(mappingOverlay)mappingOverlay->setVisible(false);return;}
        if(!mappingOverlay) {
            mappingOverlay=std::make_unique<kb09_gui::MidiMapOverlay>(*processor.midiMap);
            mappingOverlay->onDone=[this]{setMapping(false);};
        }
        layoutMapping();
        mappingOverlay->setVisible(true);mappingOverlay->refresh();mappingOverlay->toFront(true);
    }
    std::unique_ptr<juce::TooltipWindow> tooltip;juce::Image conditionBackdrop,previousCondition,resizeBackdrop;std::array<juce::Image,5> agePreviews,pendingAgePreviews;std::vector<juce::Component::SafePointer<juce::Component>> hiddenForCondition;uint64_t previewParameters=0;double previewIdleAfter=0;float previewAge=-1;bool renderingCondition=false;double resizeCacheUntil=0;
    bool staticPanelGapDust136=false;
    bool staticPanelFullRings144=false;
    bool staticPanelDetail135=false;
    bool staticPanelScrews84=false;bool staticPanelPosition131=false;
    bool staticPanelPowerScrewSpacing102=false;
    bool staticPanelPowerScrewSize105=false;
    std::shared_ptr<PanelMaterialBank> materialBank;
    std::vector<std::future<void>> materialJobs;
    std::future<void> dustWarmup;
    kb09_gui::AgeRasterCache agePanelCache;
    juce::Image staticPanel;float staticPanelAge=-1,staticPanelScale=0;bool staticPanelCornerPreview=false,staticPanelSocketPreview=false,staticPanelScrewPreview=false,staticPanelCheekPreview=false,staticPanelJointPreview=false,staticPanelCheekEdgePreview=false,staticPanelCheekContrastPreview=false,staticPanelScrewAgingPreview=false,staticPanelPowerMountPreview=false,staticPanelPowerNudgePreview=false,staticPanelPowerContourPreview=false,staticPanelPrintPreview=false;
    double performanceIdleAfter=0;unsigned lastPerformanceMidiSerial=0;
    FineSlider bender;juce::ScopedMessageBox externalHelp;Meter meter;RateLamp rateLamp;
    unsigned lastBendSerial=0;bool benderDragging=false;double bendReturnStarted=0,bendReturnFrom=0;
    struct SpringAnimation final:juce::Timer {
        std::function<void()> tick;
        void timerCallback() override {if(tick)tick();}
    } springAnimation;
    void advanceBenderReturn() {
        if(benderDragging||bendReturnStarted<=0){springAnimation.stopTimer();return;}
        const auto elapsed=juce::Time::getMillisecondCounterHiRes()-bendReturnStarted;
        const bool modern=processor.state.getRawParameterValue("modern")->load()>=.5f;
        bender.setValue(modern?0:kb09_gui::benderReturnPosition(bendReturnFrom,elapsed),juce::dontSendNotification);
        if(modern||elapsed>=55.0){bendReturnStarted=0;springAnimation.stopTimer();}
    }
    std::vector<std::unique_ptr<Control>> controls;
    std::array<int,128> lastKeys{};float lastAge=-1;int lastLabelRange=-1;bool lastModern=false,bendActive=false,userPreset=false,lastHoldMode=false;int lastProgram=-1,lastDisplayedNote=-1,lastDisplayedKeyboardNote=-1,lastHeldNote=-1;unsigned lastProgramRevision=0;double lastConditionPaint=0,ageCommitAfter=0;
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
        LivePaintScope timing{"panelTimer"};
        const auto lampNow=juce::Time::getMillisecondCounterHiRes();
        const bool lampChanged=powerLampFade.update(processor.isPowerEnabled(),lampNow);
        if(bool(getProperties().getWithDefault("previewPowerLedFade88",true))&&(lampChanged||powerLampFade.active(lampNow)||powerLampWasFading))
            repaint(251+performanceBayOffsetX,headerH-topStripReduction+475+powerOffsetY+powerLedDrop,11,11);
        powerLampWasFading=powerLampFade.active(lampNow);

        if(mappingOverlay&&mappingOverlay->isVisible()) {
            mappingOverlay->setCondition(look.age);
            const auto revision=processor.midiMap->getRevision();const auto learning=processor.midiMap->isLearning();const auto progress=processor.midiMap->learnProgress();
            if(revision!=lastMappingRevision||learning!=lastMappingLearn||progress!=lastLearnProgress){lastMappingRevision=revision;lastMappingLearn=learning;lastLearnProgress=progress;mappingOverlay->refresh();}
        }
        const auto bendSerial=processor.displayedPitchBendSerial.load(std::memory_order_acquire);
        if(!benderDragging&&bendSerial!=lastBendSerial) {
            lastBendSerial=bendSerial;bendActive=false;
            const auto bend=processor.displayedPitchBend.load(std::memory_order_relaxed);
            // Audio reaches neutral in 35 ms; the visual spring adds a 20 ms recoil.
            // External MIDI takes over immediately; recoil never feeds back to audio.
            if(bendReturnStarted==0||(!processor.displayedPitchBendReturning.load(std::memory_order_relaxed)&&std::abs(bend)>.0001f)) {
                bendReturnStarted=0;bender.setValue(bend,juce::dontSendNotification);
            }
        }
        advanceBenderReturn();
        if(processor.wrapperType!=juce::AudioProcessor::wrapperType_Undefined&&isShowing()&&getPeer()){if(!updateOpened)updateOpened=kb09::Updates::instance().opened(this);kb09::Updates::instance().deliver(this);}
        if(!computerReady())releaseComputerNotes();
        else for(size_t i=0;i<computerHeld.size();++i)if(computerHeld[i]&&!juce::KeyPress::isKeyCurrentlyDown(computerKeys[i])) {
            computerHeld[i]=false;keyboard.setComputerNote((computerOctave+1)*12+int(i),false);
        }
        const auto programRevision=processor.getProgramRevision();
        const int program=processor.getCurrentProgram();
        if(!userPreset||program!=lastProgram||programRevision!=lastProgramRevision){presets.setSelectedId(program+1,juce::dontSendNotification);userPreset=false;}
        lastProgram=program;lastProgramRevision=programRevision;
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
        bool selectedMaterialReady=true;
        if(materialBank&&!materialBank->failed) {
            const int stage=kb09_gui::appearanceStage(age);materialBank->requested=stage;
            std::lock_guard<std::mutex> guard(materialBank->mutex);
            selectedMaterialReady=materialBank->ready[size_t(stage)];
            getProperties().set("preparedMaterialStages",int(std::count(materialBank->ready.begin(),materialBank->ready.end(),true)));
        }
        if(look.ageIsDeferred&&!look.conditionDragging&&now>=ageCommitAfter&&selectedMaterialReady) {
            look.ageIsDeferred=false;look.renderingPreview=false;
            getProperties().set("conditionRenderDeferred",false);
        }
        if(age!=lastAge&&!look.ageIsDeferred){
            lastConditionPaint=now;lastAge=age;look.age=age;
            getProperties().set("displayedConditionAge",age);
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
        // Never run a multi-millisecond appearance-bank render between motion
        // frames. JUCE's synthetic/native drags and automation both participate.
        for(const auto& control:controls)if(control->slider||control->selector) {
            const auto& flags=(control->slider?static_cast<juce::Component*>(control->slider.get()):static_cast<juce::Component*>(control->selector.get()))->getProperties();
            if(bool(flags.getWithDefault("hardwareHoverActive",false))||bool(flags.getWithDefault("hardwareGestureActive",false))||bool(flags.getWithDefault("choiceSnapActive",false))) {
                performanceIdleAfter=now+500;previewIdleAfter=now+500;break;
            }
        }
        if(!look.conditionDragging && !(mappingOverlay&&mappingOverlay->isVisible()) && (bool(getProperties().getWithDefault("renderConditionPreviews",false))&&now>=performanceIdleAfter)) {
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
                pendingAgePreviews[i]=captureAppearancePreview({0,headerH,designW,visibleBodyH-topStripReduction},900.0f/designW);
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
        bool labelsCanUpdate=true;
        for(const auto& control:controls)if(control->slider&&control->slider->getComponentID()=="range") {
            const auto& flags=control->slider->getProperties();
            labelsCanUpdate=!bool(flags.getWithDefault("hardwareGestureActive",false))
                &&now>=double(flags.getWithDefault("choiceSnapEndMs",0.))+80.;
        }
        if(range!=lastLabelRange&&labelsCanUpdate){
            lastLabelRange=range;
            if(bool(keyboard.getProperties().getWithDefault("externalOctaveLabels",false))) {
                for(size_t i=0;i<octaveLabels.size();++i)octaveLabels[i].setText("C"+juce::String(4+int(i)+range-2),juce::dontSendNotification);
            } else keyboard.repaintOctaveLabels();
        }
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
        if(juce::String(id)=="range")slider.setRotaryParameters(twoPi-1.134464f*rangeDetentScale,twoPi+1.134464f*rangeDetentScale,true);
        else if(juce::String(id)=="wave")slider.setRotaryParameters(twoPi-1.047198f*waveDetentScale,twoPi+1.047198f*waveDetentScale,true);
        else slider.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id) {slider.setTooltip(spec.name);
        if(juce::String(id)=="tune")slider.setTooltip("Tune oscillator pitch: +/-65 cents in Authentic mode; +/-12 semitones (one octave each way) in Modern mode. Centre is concert pitch.");
        if(juce::String(id)=="range")slider.setTooltip("Choose the oscillator octave range.");
        if(juce::String(id)=="wave")slider.setTooltip("Choose noise, saw, square or pulse as the main oscillator source.");
        if(juce::String(id)=="pulseWidth")slider.setTooltip("Set pulse width manually, or the amount of pulse-width modulation.");
        if(juce::String(id)=="vcoLevel")slider.setTooltip("Mix the main oscillator into the filter.");
        if(juce::String(id)=="subLevel")slider.setTooltip("Mix the sub-oscillator into the filter.");
        if(juce::String(id)=="extLevel")slider.setTooltip("Mix incoming external audio into the filter; watch the IN indicator.");
        if(juce::String(id)=="lfoRate")slider.setTooltip("Set modulation speed: Authentic 0.2 to 25 Hz; Modern FREE 0.05 to 25 Hz. Modern SYNC chooses a musical division of the host tempo.");
        if(juce::String(id)=="lfoDelay")slider.setTooltip("Delay the onset of modulation after a note starts.");
        if(juce::String(id)=="vcoMod")slider.setTooltip(processor.wrapperType==juce::AudioProcessor::wrapperType_VST3
            ?"Set the amount of LFO pitch modulation. MIDI mod wheel (CC1) controls this fader immediately on its next movement. DAW mappings and automation remain available."
            :"Set the amount of LFO pitch modulation. Unless reassigned with MIDI MAP, MIDI mod wheel (CC1) controls this fader after reaching or crossing its current position. Manual edits re-arm pickup.");
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
        if(juce::String(id)=="age")slider.setTooltip("Click a tick or drag to choose a condition, from Minty to Fire Hazard.");
        if(juce::String(id)=="velocity")slider.setTooltip("In Modern mode, set how strongly playing velocity affects the sound.");
        if(juce::String(id)=="aftertouch")slider.setTooltip("In Modern mode, channel aftertouch opens the filter. Set the pressure amount here; maximum pressure and amount raise cutoff by up to three octaves.");
        if(juce::String(id)=="extGain")slider.setTooltip("In Modern mode, adjust external input gain before the follower and mixer.");
slider.setDoubleClickReturnValue(true,spec.initial);}
        addAndMakeVisible(slider);slider.resized();c->attachment=std::make_unique<SliderAttachment>(processor.state,id,slider);
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
            else if(sh09::controlResponse::squared(id)) {
                slider.textFromValueFunction=[name=std::string(id)](double v){return juce::String(sh09::controlResponse::position(name,v)*10,2);};
                slider.valueFromTextFunction=[name=std::string(id)](const juce::String& text){return sh09::controlResponse::effect(name,text.getDoubleValue()/10);};
            }
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
        if constexpr(KB09_APPROVED_HARDWARE)w=juce::jmax(w,compact?35:37); // Keep adjacent 36-unit bay centres separate.
        addSlider(id,name,centre-w/2,y+20,w,h-11,juce::Slider::LinearVertical);
        auto& c=*controls.back();c.slider->getProperties().set("compact",compact);
        // Printed scale numerals extend past the narrow hit area, without
        // overlapping the neighbouring fader's interaction bounds.
        c.slider->setPaintingIsUnclipped(true);
        addLabel(c,name,centre-w/2-8,y-1,w+16,20,8.4f);
    }
    void addKnob(const char* id,float cx,int cy,int d) {
        // Leave room in the component raster for the collar's shadow.
        // Keep fractional centres so pair spacing scales by exactly ten percent.
        if constexpr(KB09_APPROVED_HARDWARE)if(juce::String(id)!="tune")d+=10;
        const int snapped=juce::roundToInt(cx);
        addSlider(id,id,snapped-d/2,cy-d/2,d,d,juce::Slider::RotaryHorizontalVerticalDrag);
        controls.back()->slider->getProperties().set("knobOffsetX198",cx-float(snapped));
    }
    void addUtility(const char* id,const char* name,int centre,int width) {
        addSlider(id,name,centre-width/2,designH-headerH-34,width,30,juce::Slider::LinearHorizontal);
        addLabel(*controls.back(),name,centre-width/2-8,designH-headerH-46,width+16,18,9);
    }
};
SH09Editor::SH09Editor(SH09Processor& p)
    :AudioProcessorEditor(p),sizing(*this,designW,canvasH),panel(std::make_unique<Panel>(p,sizing)) {
    setResizable(true,false);addAndMakeVisible(*panel);setSize(designW,canvasH);sizing.chooseMedium();
}
void SH09Editor::prepareStandaloneWindow() { sizing.prepareStandaloneWindow(); }
void SH09Editor::setAudioSettingsObserver(std::function<void(bool)> observer) {panel->audioSettingsObserver=std::move(observer);}
void SH09Editor::setStandaloneMidiChannelObserver(std::function<void(int)> observer) {panel->midiChannelObserver=std::move(observer);}
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
        panel->layoutMapping();
    }
}
