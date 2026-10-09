#include "Editor.h"
#include "PresetFiles.h"
#include "PowerDetails.h"
#include "UpdateChecker.h"
#include "../Plugin/Processor.h"
#include "BinaryData.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#include <cmath>
#include <vector>
#include <array>

namespace {
constexpr int designW=1500, bodyH=750, headerH=34, designH=834;
constexpr float panelViewSin=.156434f,panelViewCos=.987688f;
const juce::Colour ink(0xffe9e7dd), muted(0xffb4b7b5), amber(0xffdbb572);
juce::Colour agedInk(float age) {
    return ink.interpolatedWith(juce::Colour(0xffd1bd86),juce::jlimit(0.0f,1.0f,age)*.86f);
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
    juce::Image stains=load(BinaryData::chassisstains_png,BinaryData::chassisstains_pngSize);
    // Normalize the new sprite's padding so its silver skirt keeps the previous
    // on-panel width. Only the upper knob is narrower, as in the owner's photo.
    juce::Image knob=load(BinaryData::knobpersonaloverheadv5_png,BinaryData::knobpersonaloverheadv5_pngSize)
        .getClippedImage({70,70,1114,1114});
    juce::Image fader=load(BinaryData::faderpersonaloverhead_png,BinaryData::faderpersonaloverhead_pngSize);
    juce::Image rail=load(BinaryData::frontrailgrain_png,BinaryData::frontrailgrain_pngSize);
    juce::Image keys=load(BinaryData::keys_png,BinaryData::keys_pngSize);
    juce::Image whiteKey=keys.getClippedImage({296,60,218,1130});
    juce::Image blackKey=keys.getClippedImage({763,68,159,695});
    juce::Image marks,marksAged;
    Artwork() {
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
void caseStains(juce::Graphics& g,float age) {
    if(age<.60f)return;
    juce::Graphics::ScopedSaveState save(g);
    juce::Path metal;metal.addRectangle(55.0f,float(headerH),1389.0f,365.0f);
    metal.addRectangle(65.0f,float(headerH+366),182.0f,335.0f);g.reduceClipRegion(metal);
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
        metal.addRectangle(69.0f,404.0f,176.0f,323.0f);g.reduceClipRegion(metal);
        // Density varies near controls; every visible deposit is a discrete
        // grain. No blurred gradients or translucent clouds cover the paint.
        for(uint32_t i=0;i<26000;++i) {
            const uint32_t seed=surfaceHash(880301u+i*7919u);
            float x=0,y=0;
            if(i%4<2) {
                // Narrow, uneven deposits hugging the two collars and the
                // sheltered bridge between them, never a cloud below a knob.
                const float cx=i%2?356.0f:468.0f,cy=203.4f;
                const float a=surfaceNoise(seed+17)*juce::MathConstants<float>::twoPi;
                const float radius=38.0f+7.0f*std::pow(surfaceNoise(seed+31),2.4f);
                x=cx+std::cos(a)*radius;y=cy+std::sin(a)*radius;
                if(y>cy+31)continue;
            } else if(i%4==2) {
                x=395+34*surfaceNoise(seed+17);y=183+43*surfaceNoise(seed+31);
                // Sparse bridge grains: no concentrated elliptical bank.
                if(surfaceNoise(seed+47)>.32f)continue;
            } else {
                // Performance bay perimeter, PORTAMENTO recess corners and
                // bender opening lips. Keep its broad open paint clear.
                const int edge=int(surfaceNoise(seed+17)*8);
                const float t=surfaceNoise(seed+31),inset=1+5*std::pow(surfaceNoise(seed+47),2);
                if(edge==0){x=70+inset;y=409+310*t;}
                else if(edge==1){x=244-inset;y=409+310*t;}
                else if(edge==2){x=72+170*t;y=726-inset;}
                else if(edge==3){x=175+inset;y=526+128*t;}
                else if(edge==4){x=234-inset;y=526+128*t;}
                else if(edge==5){x=177+55*t;y=654-inset;}
                else if(edge==6){x=95+123*t;y=headerH+643+inset*.45f;}
                else {x=95+123*t;y=headerH+672-inset*.45f;}
            }
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
        juce::Image image(juce::Image::ARGB,2764,86,true);juce::Graphics g(image);
        g.addTransform(juce::AffineTransform::scale(2.0f));
        for(uint32_t i=0;i<90000;++i) {
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

void sliderPost(juce::Graphics& g,juce::Rectangle<float> capBounds,float age,bool releasePost) {
    // The blade's broad plane follows the slot. Seen almost from above and
    // without camera yaw, its narrow longitudinal edge is the visible surface.
    // The owner's photos establish this orientation, not physical dimensions.
    juce::Graphics::ScopedSaveState save(g);g.setOpacity(1.0f);
    const float cx=capBounds.getCentreX();
    const float edgeLength=capBounds.getHeight()*.62f*panelViewCos;
    const float lift=7.0f*panelViewSin;
    const float top=capBounds.getCentreY()-edgeLength*.5f-lift;
    const float bottom=capBounds.getCentreY()+edgeLength*.5f;
    const float half=.68f,shoulder=1.18f,step=bottom-2.7f;
    juce::Path edge;edge.startNewSubPath(cx-half,top);
    edge.lineTo(cx+half,top+.20f);edge.lineTo(cx+half,step);
    edge.lineTo(cx+shoulder,step+.25f);edge.lineTo(cx+shoulder,bottom);
    edge.lineTo(cx-shoulder,bottom);edge.lineTo(cx-shoulder,step-.20f);
    edge.lineTo(cx-half,step-.20f);edge.closeSubPath();
    // A short contact shadow gives height without exposing the broad blade face.
    g.setColour(juce::Colours::black.withAlpha(.43f));
    g.fillPath(edge,juce::AffineTransform::translation(.55f,lift*.62f));
    juce::ColourGradient metal(juce::Colour(0xff393d3b),cx-half,top,
                               juce::Colour(0xffa9aaa1),cx+half,bottom,false);
    metal.addColour(releasePost?.36:.60,juce::Colour(0xff686e68));
    g.setGradientFill(metal);g.fillPath(edge);
    g.setColour(juce::Colour(0xff202523));
    g.drawLine(cx-half,top+.35f,cx-half,step-.35f,.42f);
    // The cut edge is a thin lengthwise glint, not a wide horizontal paddle.
    g.setColour(juce::Colour(0xffd4d6cb));
    g.drawLine(cx+.13f,top+.30f,cx+.13f,step-.45f,.45f);
    g.drawLine(cx-half+.12f,top+.18f,cx+half-.12f,top+.33f,.45f);
    g.setColour(juce::Colour(0xff92988f));
    g.drawLine(cx+half+.10f,step+.35f,cx+shoulder-.08f,step+.35f,.38f);
    g.setColour(juce::Colour(0xffb1b4a8).withAlpha(age*.20f));
    g.fillRect(cx-shoulder+.16f,step+.8f,shoulder*2-.32f,bottom-step-.9f);
    { juce::Graphics::ScopedSaveState rust(g);g.reduceClipRegion(edge);
      const uint32_t seed=releasePost?19331u:7717u;
      for(uint32_t i=0;i<43;++i) {
          const float yy=top+(bottom-top)*surfaceNoise(seed+i*37);
          const float xx=cx-half+half*2*surfaceNoise(seed+i*61);
          g.setColour(juce::Colour(i%3?0xff74472b:0xff352d23).withAlpha(age*.85f));
          g.fillEllipse(xx,yy,.35f+.46f*surfaceNoise(seed+i*19),.55f);
      }
    }
    // Small, distinct interruptions keep the two blades from sharing a stamp.
    const float markY=top+(releasePost?.64f:.38f)*(bottom-top);
    g.setColour(juce::Colour(0xff373e37).withAlpha(.65f));
    g.drawLine(cx-.36f,markY,cx+.34f,markY+.28f,.34f);
}

void cap(juce::Graphics& g,juce::Rectangle<float> r,bool small=false,bool indexed=true,float age=0) {
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
            // Registered top stripe warms gradually with the other printing.
            g.setColour(juce::Colour(0xffc2a15d).withAlpha(age*.46f));
            g.fillRect(photo.getX()+photo.getWidth()*.055f,photo.getY()+photo.getHeight()*.405f,photo.getWidth()*.89f,photo.getHeight()*.12f);
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
    g.drawLine({collar+direction*d*.390f,collar+direction*d*.455f},d*.014f);
}
void screwOxidation(juce::Graphics& g,juce::Point<float> c,float age,uint32_t seed) {
    if(age<=0)return;
    // Chalky white/gray deposits on the head, concentrated in the rim and
    // protected recess edges. Panel corrosion stays in the photographic layer.
    juce::Graphics::ScopedSaveState save(g);
    juce::Path head;head.addEllipse(c.x-7.5f,c.y-7.5f,15,15);g.reduceClipRegion(head);
    for(uint32_t i=0;i<4;++i) {
        const float a=surfaceNoise(seed+i*113)*juce::MathConstants<float>::twoPi;
        const auto p=c+juce::Point<float>(std::sin(a),std::cos(a))*5.7f;
        const float size=1.4f+surfaceNoise(seed+i*83);
        g.setColour(juce::Colour(0xffe3e4d8).withAlpha(age*.24f));
        g.fillEllipse(p.x-size*.5f,p.y-size*.3f,size,size*.6f);
    }
    for(uint32_t i=0;i<90;++i) {
        const float a=surfaceNoise(seed+i*23)*juce::MathConstants<float>::twoPi;
        const float radius=4.1f+2.9f*surfaceNoise(seed+i*31+7);
        const auto p=c+juce::Point<float>(std::sin(a),std::cos(a))*radius;
        const float size=.25f+.95f*surfaceNoise(seed+i*19+3);
        g.setColour(juce::Colour(i%3?0xffe6e3d6:0xffa5a99f).withAlpha(age*(.24f+.44f*surfaceNoise(seed+i*47))));
        g.fillEllipse(p.x,p.y,size,size*(.5f+surfaceNoise(seed+i*41)));
    }
}
class HardwareLook final : public juce::LookAndFeel_V4 {
public:
    float age=0;bool conditionDragging=false,renderingPreview=false;
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
    juce::Font getTextButtonFont(juce::TextButton&,int) override {return font(11,true);}
    juce::Font getSliderPopupFont(juce::Slider&) override {return font(15,true);}
    juce::Font getLabelFont(juce::Label& l) override {return l.getFont();}
    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,
                          const juce::Slider::SliderStyle style,juce::Slider& slider) override {
        const auto ink=agedInk(age);
        const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f;
        if(style==juce::Slider::LinearVertical) {
            const bool compact=bool(slider.getProperties().getWithDefault("compact",false));
            for(int t=0;t<=20;++t) {
                const float yy=float(y)+float(h)*float(t)/20;
                const float len=t%5==0?10.0f:7.2f;
                g.setColour(ink.withAlpha(t%5==0?.86f:.62f));
                const float arm=compact?5.5f:len;
                g.drawLine(cx-arm-5,yy,cx-5,yy,.65f);g.drawLine(cx+5,yy,cx+arm+5,yy,.65f);
            }
            for(int i=0;i<3;++i) {
                const float yy=float(y)+float(h)*float(i)*.5f;
                const bool scaleLeft=bool(slider.getProperties().getWithDefault("scaleLeft",false));
                caption(g,juce::String(10-i*5),{cx+(scaleLeft?-26.0f:14.5f),yy-4,11,8},6.6f,
                        scaleLeft?juce::Justification::centredRight:juce::Justification::centredLeft,false,ink);
            }
            g.setColour(juce::Colour(0xff62696a));g.fillRoundedRectangle(cx-4,float(y)-2,8,float(h)+4,2);
            g.setColour(juce::Colour(0xff06090b));g.fillRoundedRectangle(cx-3.2f,float(y)-1,6.4f,float(h)+2,1.5f);
            g.setColour(juce::Colour(0xff000204));g.drawLine(cx-2.5f,float(y),cx-2.5f,float(y+h),1);
            g.setColour(juce::Colours::white.withAlpha(.13f));g.drawLine(cx+3.6f,float(y),cx+3.6f,float(y+h),.55f);
            const auto capBounds=juce::Rectangle<float>(cx-(compact?12.5f:15),pos-10,compact?25.0f:30.0f,23);
            const bool releasePost=slider.getComponentID()=="release";
            const bool missing=(slider.getComponentID()=="vcoLevel"&&age>=.64f)||(releasePost&&age>=.88f);
            // Cap loss is a discrete visual event. The original slider, hit
            // area, attachment and value remain available beneath the drawing.
            if(missing)sliderPost(g,capBounds,age,releasePost);
            else cap(g,capBounds,false,true,age);
        } else {
            if(slider.getComponentID()=="bender") {
                const float amount=float(slider.getValue());
                const float centre=float(slider.getWidth())*.5f;
                auto well=juce::Rectangle<float>(8,13,float(slider.getWidth())-16,22);
                g.setColour(juce::Colour(0xff080909));g.fillRoundedRectangle(well.expanded(1.3f),1.1f);
                g.setColour(juce::Colour(0xff393b38).withAlpha(.55f));g.drawLine(well.getX(),well.getBottom()+.6f,well.getRight(),well.getBottom()+.6f,.65f);
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff030303),well.getX(),well.getY(),
                                                     juce::Colour(0xff202020),well.getX(),well.getBottom(),false));
                g.fillRoundedRectangle(well,.7f);
                // One rigid PB-4-like disk and finger tab, extruded along its
                // hidden axle. Rotate object vertices first; the shared 9-degree
                // camera then projects every face without screen-space reshaping.
                const float angle=amount*.610865f,sa=std::sin(angle),ca=std::cos(angle);
                constexpr float radius=60.0f,depth=12.8f,rootWidth=9.0f;
                // A thin central web carries a substantially thicker molded rim.
                const auto frontDepth=[](juce::Point<float> p) {
                    const float t=juce::jlimit(0.0f,1.0f,(p.getDistanceFromOrigin()-51.0f)/5.0f);
                    return 5.4f+7.4f*t*t*(3-2*t);
                };
                const float wheelBaseline=13.0f+radius*panelViewSin;
                const bool broken=age>=.92f;
                const auto project=[centre,wheelBaseline,sa,ca](juce::Point<float> p,float axleDepth) {
                    const float xx=p.x*ca+p.y*sa,zz=p.y*ca-p.x*sa;
                    return juce::Point<float>(centre+xx,wheelBaseline-zz*panelViewSin+axleDepth*panelViewCos);
                };
                constexpr int circleSegments=256,vertexCapacity=circleSegments+24;
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
                        {-13.8f,57.3f},{-14.6f,54.1f},{-11.7f,50.7f},{-8.9f,50.4f},
                        {-6.1f,50.9f},{-3.6f,50.1f},{-.8f,50.4f},{2.3f,49.8f},
                        {5.2f,50.5f},{8.6f,50.1f},{11.3f,51.6f},{13.4f,55.0f},
                        {12.1f,59.5f},{15.2f,62.9f},{17.0f,60.2f},{20.5f,56.6f}}};
                    for(const auto p:tab)outline[size_t(count++)]=p;
                } else {
                    outline[size_t(count++)]={-9.0f,rootHeight+1.2f};
                    for(int i=0;i<=5;++i){const float a=juce::MathConstants<float>::pi-float(i)*juce::MathConstants<float>::halfPi/5;outline[size_t(count++)]={-4.5f+4.5f*std::cos(a),80.5f+4.5f*std::sin(a)};}
                    for(int i=0;i<=5;++i){const float a=juce::MathConstants<float>::halfPi-float(i)*juce::MathConstants<float>::halfPi/5;outline[size_t(count++)]={4.5f+4.5f*std::cos(a),80.5f+4.5f*std::sin(a)};}
                    outline[size_t(count++)]={9.0f,rootHeight+1.2f};
                }
                const auto section=[&](float z) {
                    juce::Path p;p.startNewSubPath(project(outline[0],z==0?0:frontDepth(outline[0])));
                    for(int i=1;i<count;++i)p.lineTo(project(outline[size_t(i)],z==0?0:frontDepth(outline[size_t(i)])));
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
                    const float specular=std::pow(half,54.0f-28.0f*age);
                    const float lightShape=.70f+.30f*std::exp(-((axial-1.2f)*(axial-1.2f))/1.8f);
                    return .045f+.040f*diffuse+.11f*fill+1.40f*(1-age*.58f)*specular*lightShape+age*.008f;
                };
                const int samples=conditionDragging?1:4;
                const juce::Rectangle<float> rimBounds(centre-radius-1,12,2*radius+2,
                                                       wheelBaseline+depth*panelViewCos-11);
                juce::Image rim(juce::Image::ARGB,juce::roundToInt(rimBounds.getWidth()*samples),
                                juce::roundToInt(rimBounds.getHeight()*samples),true);
                {
                    juce::Image::BitmapData pixels(rim,juce::Image::BitmapData::writeOnly);
                    constexpr float bevel=.40f;
                    for(int yy=0;yy<rim.getHeight();++yy)for(int xx=0;xx<rim.getWidth();++xx) {
                        const float px=rimBounds.getX()+(float(xx)+.5f)/samples-centre;
                        const float py=rimBounds.getY()+(float(yy)+.5f)/samples;
                        if(std::abs(px)>=radius)continue;
                        float z=std::sqrt(radius*radius-px*px);
                        float axial=(py-wheelBaseline+z*panelViewSin)/panelViewCos;
                        if(axial<-.18f||axial>depth+.18f)continue;
                        float r=radius;
                        for(int iteration=0;iteration<2;++iteration) {
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
                        if(axial<bevel)ny=juce::jlimit(-1.0f,0.0f,(axial-bevel)/bevel);
                        else if(axial>depth-bevel)ny=juce::jlimit(0.0f,1.0f,(axial-(depth-bevel))/bevel);
                        radial=std::sqrt(juce::jmax(0.0f,1-ny*ny));
                        float value=plasticLight(px/r*radial,ny,z/r*radial,axial);
                        // Fine raised parting seam follows the molded disc around its circumference.
                        const float seam=(axial-depth*.50f)/.48f;
                        value+=(.24f*std::exp(-seam*seam)-.11f*std::exp(-(seam-1.15f)*(seam-1.15f)))*(1-age*.28f);
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
                    if(i<circleSegments)continue;
                    const auto a=outline[size_t(i)],b=outline[size_t((i+1)%count)];
                    const float dx=b.x-a.x,dz=b.y-a.y,length=std::sqrt(dx*dx+dz*dz);
                    if(length<.0001f)continue;
                    const float nx=(-dz*ca+dx*sa)/length,nz=(dx*ca+dz*sa)/length;
                    if(nz<=0)continue;
                    juce::Path wall;wall.startNewSubPath(project(a,0));wall.lineTo(project(b,0));
                    wall.lineTo(project(b,frontDepth(b)));wall.lineTo(project(a,frontDepth(a)));wall.closeSubPath();
                    const bool fracture=broken&&i>=circleSegments;
                    const auto shade=[&](float axial) {
                        // Fractured plastic scatters light; glossy face-by-face
                        // highlights made the old break look like metal teeth.
                        const float value=fracture?.10f+.035f*juce::jmax(0.0f,nz)+.018f*(1-axial/depth):plasticLight(nx,0,nz,axial);
                        auto colour=juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,1);
                        if(fracture)colour=colour.interpolatedWith(juce::Colour(0xff77786e),.11f);
                        return colour;
                    };
                    const auto middle=(a+b)*.5f,backPoint=project(middle,0),frontPoint=project(middle,depth);
                    juce::ColourGradient reflection(shade(0),backPoint.x,backPoint.y,shade(depth),frontPoint.x,frontPoint.y,false);
                    reflection.addColour(.22,shade(depth*.22f));reflection.addColour(.70,shade(depth*.70f));
                    if(!broken)wall=wall.createPathWithRoundedCorners(2.6f);
                    g.setGradientFill(reflection);g.fillPath(wall);
                    if(fracture) {
                        juce::Graphics::ScopedSaveState rough(g);g.reduceClipRegion(wall);
                        for(uint32_t grain=0;grain<18;++grain) {
                            const auto key=uint32_t(i)*7919u+grain*113u;
                            const auto local=a+(b-a)*surfaceNoise(key+13);
                            const auto point=project(local,frontDepth(local)*surfaceNoise(key+31));
                            g.setColour(juce::Colour(grain%3?0xffa9a593:0xff080a09).withAlpha(.20f));
                            g.fillEllipse(point.x,point.y,.27f,.20f);
                        }
                    }
                    // Overlap the antialias coverage with the identical material.
                    g.strokePath(wall,juce::PathStrokeType(.16f));
                    g.setColour(juce::Colour(0xff8c9087).withAlpha((fracture?.12f:.32f)*(1-age*.45f)));
                    g.drawLine({project(a,.15f),project(b,.15f)},.45f);
                    g.setColour(juce::Colour(0xff010202).withAlpha(.55f));
                    g.drawLine({project(a,depth),project(b,depth)},.35f);
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
                    g.setColour(juce::Colour(0xffb7bab0).withAlpha(.15f*(1-age*.45f)));
                    g.strokePath(channel(54,frontDepth({0,54})),juce::PathStrokeType(.50f));
                    g.setColour(juce::Colour(0xffa2a397).withAlpha(.07f*(1-age*.50f)));g.strokePath(channel(53.6f,frontDepth({0,53.6f})),juce::PathStrokeType(.30f));
                    g.setColour(juce::Colour(0xff020404));g.strokePath(channel(51,frontDepth({0,51})),juce::PathStrokeType(.45f));
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
        const auto ink=agedInk(getY()<headerH+bodyH?age:0);
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
class Keyboard final : public juce::Component {
public:
    explicit Keyboard(SH09Processor& p):processor(p) {
        setComponentID("keyboard");
        setMouseCursor(juce::MouseCursor::PointingHandCursor);setWantsKeyboardFocus(true);
        setTitle("32-key low-note-priority keyboard");
    }
    ~Keyboard() override {release();}
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff06090b));
        const auto* material=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=juce::jlimit(0.0f,1.0f,material!=nullptr&&material->renderingPreview?material->age:processor.state.getRawParameterValue("age")->load());
        const float gloss=1.0f-age*.90f;
        std::array<bool,128> held{};
        int effectiveNote=-1;
        const bool powered=processor.isPowerEnabled();
        const int consumedMouseNote=processor.displayedKeyboardNote.load(std::memory_order_relaxed);
        // Replace the audio thread's previous mouse contribution with the live
        // mouse position. Subtract only one count: MIDI on the same pitch stays
        // held. Upward drags and releases then respond before the next callback.
        for(int n=0;n<128;++n) {
            const int midiCount=processor.keyDisplay[size_t(n)].load(std::memory_order_relaxed)-(n==consumedMouseNote?1:0);
            held[size_t(n)]=powered&&(midiCount>0||n==mouseNote);
            if(effectiveNote<0&&held[size_t(n)])effectiveNote=n;
        }
        for(int pass=0;pass<2;++pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass) {
            auto r=keyRect(n);
            const bool down=held[size_t(n)];
            const float depressed=down?3.5f:0;
            r=r.withTrimmedTop(depressed);
            if(pass==0) {
                const auto ivory=juce::Colour(0xffefede6).interpolatedWith(juce::Colour(0xffd1bd86),age*.85f);
                auto key=r.reduced(.65f,0);
                auto face=key.withTrimmedTop(key.getHeight()-20);
                auto top=key.withTrimmedBottom(20);
                juce::ColourGradient enamel(ivory.darker(.23f),0,top.getY(),ivory,0,top.getBottom(),false);
                enamel.addColour(.07,ivory.darker(.09f));enamel.addColour(.55,ivory.brighter(.04f*gloss));
                enamel.addColour(.975,ivory.brighter(.09f*gloss));g.setGradientFill(enamel);g.fillRect(top);
                g.setGradientFill(juce::ColourGradient(ivory.darker(.19f),0,face.getY(),ivory.darker(.04f),0,face.getBottom(),false));
                g.fillRoundedRectangle(face,1.4f);
                g.setColour(juce::Colours::white.withAlpha(.55f*gloss));g.drawLine(key.getX()+1,top.getY()+2,key.getX()+1,top.getBottom()-2,.8f);
                g.setColour(juce::Colour(0xff635e54).withAlpha(.28f));g.drawLine(key.getRight()-1,top.getY(),key.getRight()-1,top.getBottom(),1.2f);
                g.setColour(juce::Colours::white.withAlpha(.62f*gloss));g.drawLine(key.getX()+2,face.getY(),key.getRight()-2,face.getY(),.8f);
                g.setColour(juce::Colours::black.withAlpha(.26f));g.fillRect(key.getX()+1,key.getBottom()-2,key.getWidth()-2,2.0f);
                // The photo contains fresh specular reflections. Reveal the
                // matte native enamel underneath as its surface loses gloss.
                g.setOpacity(1.0f-age*.64f);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(artwork().whiteKey,key,juce::RectanglePlacement::stretchToFit);
                g.setOpacity(1.0f);
                const float variation=float(n-53)/31;
                // Polymer and printing age separately. Strong warm ivory at the
                // endpoint follows the uneven cream keys in original-unit photos.
                g.setColour(juce::Colour(0xffb99d53).withAlpha(.008f+variation*.035f+age*(.28f+variation*.16f)));
                g.fillRoundedRectangle(key.reduced(.5f),1.4f);
            } else {
                for(int i=5;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.07f));g.fillRoundedRectangle(r.expanded(float(i)*.45f).translated(2,3),1.8f);}
                auto face=r.withTrimmedTop(r.getHeight()-17),top=r.withTrimmedBottom(17);
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
                // Black keys lose their broad photographic shine especially
                // strongly, while their silhouette remains fully opaque.
                g.setOpacity(1.0f-age*.40f);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(artwork().blackKey,r,juce::RectanglePlacement::stretchToFit);
                g.setOpacity(1.0f);
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
                g.strokePath(hairline,juce::PathStrokeType(n==60?.32f:n==65?.62f:.56f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
            }
            if(pass==0&&n%12==0&&processor.state.getRawParameterValue("modern")->load()>.5f) {
                const int octave=n/12-1+juce::roundToInt(processor.state.getRawParameterValue("range")->load())-2;
                caption(g,"C"+juce::String(octave),{r.getX()+2,r.getBottom()-15,r.getWidth()-4,10},7.0f,
                    juce::Justification::centred,false,juce::Colour(0xff645f53).withAlpha(.72f));
            }
            if(down) {
                g.setColour(juce::Colours::black.withAlpha(pass==0?.14f:.18f));g.fillRoundedRectangle(r.reduced(1),1.5f);
                if(effectiveNote==n) {
                    g.setColour(amber);g.fillRoundedRectangle(r.getX()+4,r.getBottom()-11,r.getWidth()-8,7,1.2f);
                }
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
    SH09Processor& processor;int mouseNote=-1;
    juce::Rectangle<float> keyRect(int n) const {
        int whites=0;for(int i=53;i<n;++i)if(!isBlack(i))++whites;
        const float w=float(getWidth())/19;
        if(isBlack(n))return {float(whites)*w-w*.31f,0,w*.62f,float(getHeight())*.64f};
        const auto* material=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=material?material->age:processor.state.getRawParameterValue("age")->load();
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
class Meter final : public juce::Component {
public:
    void setLevels(float e,float f,float input) {if(std::abs(e-env)>.005f||std::abs(f-ext)>.005f||std::abs(input-in)>.005f){env=e;ext=f;in=input;repaint();}}
    void paint(juce::Graphics& g) override {
        for(int k=0;k<3;++k) {
            const float xx=float(k*90);caption(g,k==2?"IN":k?"EXT":"ENV",{xx,0,25,20},9,juce::Justification::centredLeft);
            for(int i=0;i<8;++i) {
                const bool lit=(k==2?in:k?ext:env)>float(i)/8;
                g.setColour(lit?amber:juce::Colour(0xff373b39));g.fillRect(xx+28+float(i*6),6.0f,4.0f,8.0f);
            }
        }
    }
private:float env=0,ext=0,in=0;
};
class RateLamp final : public juce::Component {
public:
    void setLevel(float value) {if(std::abs(on-value)>.005f){on=value;repaint();}}
    void paint(juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().reduced(1);
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
        const float age=juce::jlimit(0.0f,1.0f,processor.state.getRawParameterValue("age")->load());
        const bool pressed=processor.isPowerEnabled()||down;
        const auto recess=getLocalBounds().toFloat().reduced(.7f);
        // The grey pushbutton stands tall when OFF. Project its estimated
        // physical height through the same 9-degree view as the other hardware;
        // ON lowers the same cap without shrinking its top or changing camera.
        g.setColour(juce::Colour(0xff050605));g.fillRoundedRectangle(recess,1.3f);
        g.setColour(juce::Colour(0xff43433b).withAlpha(.36f));g.drawRoundedRectangle(recess,1.3f,.45f);
        const auto footprint=recess.reduced(1.35f,2.1f).withTrimmedTop(5.0f);
        const float wallHeight=(pressed?7.0f:29.0f)*panelViewSin;
        const auto top=footprint.translated(0,-wallHeight);
        g.setColour(juce::Colours::black.withAlpha(pressed?.35f:.47f));
        g.fillRoundedRectangle(footprint.translated(.45f,.25f+wallHeight*.34f),1.1f);
        const auto grey=juce::Colour(0xff858780).interpolatedWith(juce::Colour(0xff78786c),age*.76f);
        const auto wall=juce::Rectangle<float>(top.getX(),top.getBottom()-2,top.getWidth(),wallHeight+2);
        g.setGradientFill(juce::ColourGradient(grey.darker(.13f),0,wall.getY(),grey.darker(.23f),0,wall.getBottom(),false));
        g.fillRoundedRectangle(wall,3.8f);
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
        constexpr float bevel=2.3f,radius=3.8f;
        const int w=juce::roundToInt(width*scale),h=juce::roundToInt(height*scale);
        if(cap.isValid()&&cap.getWidth()==w&&cap.getHeight()==h&&capAge==age&&capOver==over)return cap;
        cap=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType());capAge=age;capOver=over;
        const auto base=juce::Colour(0xff858780).interpolatedWith(juce::Colour(0xff78786c),age*.76f);
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
            const float diffuse=juce::jmax(0.0f,nx*.35f+ny*.42f+nz*.84f);
            const float specular=std::pow(juce::jmax(0.0f,nx*.19f+ny*.23f+nz*.953f),16)*(.075f-age*.035f);
            const float grain=(surfaceNoise(uint32_t(x/2)*7919u+uint32_t(y/2)*104729u+9457u)-.5f)*.012f;
            const float cloud=.008f*std::sin(px*.31f+std::sin(py*.47f))-.009f*age*(1-t);
            const float shade=.73f+.28f*diffuse+specular+grain+cloud+(over?.009f:0);
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
        caption(g,"TOYLAND / KB-09",{20,0,208,float(getHeight())},11,juce::Justification::centredLeft);
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
    g.setColour(ink.withAlpha(.84f));g.drawRect(x,57.0f,w,182.0f,.85f);
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
                       {label.x-9,label.y-5,18,10},ink,1.15f);
        else caption(g,text,{label.x-21,label.y-7,42,14},8.4f,juce::Justification::centred,true,ink);
    }
}
}

class SH09Editor::Panel final : public juce::Component, private juce::Timer, private juce::KeyListener {
public:
    void cacheForResize() {
        if(!resizeBackdrop.isValid()) {
            resizeBackdrop=juce::Image(juce::Image::RGB,designW,designH,true);
            renderingCondition=true;{juce::Graphics frame(resizeBackdrop);paint(frame);}renderingCondition=false;
        }
        resizeCacheUntil=juce::Time::getMillisecondCounterHiRes()+180;
        keyboard.setBufferedToImage(true);bender.setBufferedToImage(true);power.setBufferedToImage(true);
        for(auto& c:controls) {if(c->slider)c->slider->setBufferedToImage(true);if(c->selector)c->selector->setBufferedToImage(true);}
    }
    Panel(SH09Processor& p,sh09::WindowSizing& s)
        :processor(p),sizing(s),presetFiles(p,*this),keyboard(p),header(s),maximise(s),power(p) {
        setLookAndFeel(&look);setSize(designW,designH);
        addAndMakeVisible(header);header.setBounds(0,0,designW,headerH);
        addAndMakeVisible(updates);updates.setBounds(4,5,132,24);updates.setButtonText("TOYLAND / KB-09");updates.setComponentID("updates");updates.onClick=[this]{kb09::Updates::instance().showMenu(&updates);};
        addAndMakeVisible(keyboard);keyboard.setBounds(286,headerH+370,1142,325);
        for(int i=0;i<processor.getNumPrograms();++i)presets.addItem(processor.getProgramName(i),i+1);
        addAndMakeVisible(presets);presets.setBounds(438,5,336,24);presets.setComponentID("factoryPreset");
        presets.setTooltip("Factory patch. Loading replaces current sound settings.");
        presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);
        presets.onChange=[this]{if(presets.getSelectedId()>0){userPreset=false;processor.setCurrentProgram(presets.getSelectedId()-1);}};
        addAndMakeVisible(previous);previous.setButtonText("<");previous.setBounds(407,5,25,24);
        previous.onClick=[this]{userPreset=false;processor.setCurrentProgram((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms());};
        addAndMakeVisible(next);next.setButtonText(">");next.setBounds(780,5,25,24);
        next.onClick=[this]{userPreset=false;processor.setCurrentProgram((processor.getCurrentProgram()+1)%processor.getNumPrograms());};
        addAndMakeVisible(savePreset);savePreset.setButtonText("SAVE");savePreset.setBounds(817,5,56,24);savePreset.setComponentID("savePreset");
        savePreset.setTooltip("Save the complete sound as a reusable .kb09 preset.");savePreset.onClick=[this]{presetFiles.savePreset();};
        addAndMakeVisible(loadPreset);loadPreset.setButtonText("LOAD");loadPreset.setBounds(879,5,56,24);loadPreset.setComponentID("loadPreset");
        loadPreset.setTooltip("Load a saved .kb09 preset.");loadPreset.onClick=[this]{presetFiles.loadPreset();};
        presetFiles.onCompleted=[this](const juce::Result& result,const juce::File& file,bool wasLoad) {
            if(result.wasOk()) {
                if(!wasLoad)savePreset.setTooltip("Saved preset: "+file.getFileName());
                if(wasLoad){userPreset=true;lastProgramRevision=processor.getProgramRevision();lastProgram=processor.getCurrentProgram();presets.setSelectedId(0,juce::dontSendNotification);presets.setText(file.getFileNameWithoutExtension(),juce::dontSendNotification);}
            }
        };
        addAndMakeVisible(meter);meter.setBounds(1008,7,270,20);
        addAndMakeVisible(small);small.setButtonText("S");small.setBounds(1291,5,35,24);small.setComponentID("sizeS");
        small.setTooltip("Small: fit within 65% of this screen's work-area width and height, with the complete instrument visible.");small.onClick=[this]{sizing.chooseSmall();};
        addAndMakeVisible(medium);medium.setButtonText("M");medium.setBounds(1332,5,35,24);medium.setComponentID("sizeM");
        medium.setTooltip("Medium: fit within 90% of this screen's work-area width and height.");medium.onClick=[this]{sizing.chooseMedium();};
        addAndMakeVisible(large);large.setButtonText("L");large.setBounds(1373,5,35,24);large.setComponentID("sizeL");
        large.setTooltip("Large: fill this screen's available work area. For sharpness in Live, disable Auto-Scale Plug-in Window if available, then create a fresh instance.");large.onClick=[this]{sizing.chooseLarge();};
        if(processor.wrapperType==juce::AudioProcessor::wrapperType_Standalone) {
            header.getProperties().set("standalone",true);
            addAndMakeVisible(audioMidi);audioMidi.setComponentID("audioMidiSettings");
            audioMidi.setButtonText("AUDIO / MIDI");audioMidi.setBounds(268,5,115,24);
            setWantsKeyboardFocus(true);addKeyListener(this);
            addAndMakeVisible(computerKeyboard);computerKeyboard.setComponentID("computerKeyboard");
            computerKeyboard.setBounds(140,5,122,24);computerKeyboard.setClickingTogglesState(true);
            computerKeyboard.setWantsKeyboardFocus(false);
            computerKeyboard.setColour(juce::TextButton::buttonOnColourId,amber);
            computerKeyboard.setColour(juce::TextButton::textColourOnId,juce::Colour(0xff151a1d));
            computerKeyboard.setTooltip("Computer keyboard: A W S E D F T G Y H U J K play C to C. Z / X shift octave. Standalone only.");
            computerKeyboard.onClick=[this]{releaseComputerNotes();updateComputerLabel();if(computerKeyboard.getToggleState())grabKeyboardFocus();};
            updateComputerLabel();
            audioMidi.setTooltip("Choose your audio output device and enable your MIDI controller.");
            audioMidi.onClick=[] {
                if(auto* holder=juce::StandalonePluginHolder::getInstance())holder->showAudioSettingsDialog();
            };
        }
        if(processor.wrapperType!=juce::AudioProcessor::wrapperType_Standalone)addAndMakeVisible(maximise);
        maximise.setBounds(1453,0,47,34);maximise.setComponentID("maximise");
        sizing.onChange=[this]{header.repaint();maximise.repaint();};
        addSwitch("lfoWave",116,104,49,121,{0,1,2},{"SINE","SQUARE","RANDOM\n(S/H)"},"Modulator: sine, square or random sample-and-hold.");
        addFader("lfoRate","RATE",182,96,43,129);controls.back()->label->setVisible(false);addFader("lfoDelay","DELAY",224,96,43,129);
        controls.back()->label->setVisible(false); // Sine + DELAY TIME is printed by the panel.
        addAndMakeVisible(rateLamp);rateLamp.setBounds(122,headerH+98,8,8);rateLamp.setInterceptsMouseClicks(false,false);
        addFader("vcoMod","MOD",284,96,45,129);
        controls.back()->slider->getProperties().set("scaleLeft",true);
        addKnob("range",356,169,87);addKnob("wave",468,169,87);addKnob("tune",356,86,22);
        addFader("pulseWidth","PULSE\nWIDTH",536,96,45,129);
        controls.back()->label->setVisible(false); // Shared PULSE WIDTH MOD bracket replaces this caption.
        addSwitch("pwmMode",566,104,51,121,{1,0,2},{"LFO","MANUAL","ENV"},"Pulse width: LFO, manual or envelope. LFO uses the undelayed sine.");
        addSwitch("subMode",633,104,60,121,{0,1,2},{"1 OCT\nDOWN","2 OCT\nDOWN","2 OCT\nDOWN"},"Sub oscillator: one octave down square, two octaves down square, or two octaves down pulse.");
        addFader("vcoLevel","VCO",721,96,41,129);addFader("subLevel","SUB\nOSC",770,96,41,129);addFader("extLevel","EXT",819,96,41,129);
        addFader("cutoff","CUTOFF\nFREQ",880,96,43,129);addFader("resonance","RESONANCE",923,96,45,129);
        addSwitch("vcfEnvMode",950,104,61,121,{2,0,1},{"ENV\nFOL'R","ENV +","ENV -"},"VCF envelope follower, positive envelope, or inverted envelope.",true);
        addFader("vcfEnv","ENV",1030,96,40,129);controls.back()->label->setVisible(false);addFader("vcfMod","MOD",1075,96,40,129);
        addSwitch("vcaMode",1110,104,56,121,{0,1,2},{"HOLD","ENV","GATE"},"HOLD passes audio without MIDI; ENV uses ADSR; GATE follows held keys.");
        addSwitch("envMode",1175,104,55,121,{0,1,2},{"GATE +\nTRIG","GATE","LFO"},"Gate+Trigger re-attacks on low-note changes. Gate preserves legato. LFO repeats while held.",true);
        addFader("attack","A",1249,96,35,129,true);addFader("decay","D",1288,96,35,129,true);
        addFader("sustain","S",1327,96,35,129,true);addFader("release","R",1366,96,35,129,true);
        addKnob("volume",117,442,79);
        addAndMakeVisible(power);power.setBounds(184,headerH+410,42,32);power.onPowerChanged=[this]{repaint();};
        addFader("bendVco","VCO",98,496,37,112,true);addFader("bendVcf","VCF",142,496,37,112,true);
        addFader("portamento","PORTAMENTO",205,496,37,112,true);
        addAndMakeVisible(bender);bender.setBounds(85,headerH+638,143,48);bender.setComponentID("bender");
        bender.setRange(-1,1,.001);bender.setValue(0);bender.setSliderStyle(juce::Slider::LinearHorizontal);
        bender.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);bender.setScrollWheelEnabled(false);bender.setWantsKeyboardFocus(false);
        bender.setTooltip("Spring-return bender. VCO / VCF sliders set the depth.");
        bender.setTitle("Pitch bender");bender.setPopupDisplayEnabled(true,true,this);
        bender.textFromValueFunction=[](double v){return (v>0?juce::String("+"):juce::String())+juce::String(v*100,1)+"%";};
        bender.onValueChange=[this]{bendActive=std::abs(bender.getValue())>.0001;processor.queuePitchBend(float(bender.getValue()));};
        bender.onDragEnd=[this]{bender.setValue(0,juce::sendNotificationSync);};
        addSwitch("modern",22,designH-headerH-44,131,42,{0,1},{"AUTHENTIC","MODERN"},"Authentic ignores velocity, aftertouch, tempo sync and input trim.");
        // Keep a consistent 52-unit gap between bottom-row control groups.
        addUtility("age","CONDITION / AGE",290,170);
        controls.back()->slider->onDragStart=[this]{
            look.conditionDragging=true;
            // All expensive hardware is already in small pre-rendered frames.
            // Keep the footer and its AGE thumb live on every mouse event.
            {
                for(auto* c:getChildren())if(c->isVisible()&&c->getY()>=headerH&&c->getBottom()<=headerH+bodyH) {
                    hiddenForCondition.emplace_back(c);c->setVisible(false);
                }
            }
        };
        controls.back()->slider->onDragEnd=[this]{
            look.conditionDragging=false;conditionBackdrop={};previousCondition={};
            look.age=processor.state.getRawParameterValue("age")->load();
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
        addAndMakeVisible(help);help.setButtonText("EXT AUDIO");help.setBounds(1352,designH-35,120,24);
        help.onClick=[this]{externalHelp.setVisible(!externalHelp.isVisible());};
        addChildComponent(externalHelp);externalHelp.setBounds(390,headerH+418,900,225);
        externalHelp.setText("EXTERNAL AUDIO + ENVELOPE FOLLOWER\n\nRoute DAW audio to the instrument's External Input and raise EXT in the mixer.\nWithout MIDI, set VCA to HOLD and lower VCO / SUB.\nSelect ENV FOL'R in VCF, lower CUTOFF and raise ENV for touch-sensitive filtering.\nThe follower listens before the EXT fader. Click EXT AUDIO again to close.",juce::dontSendNotification);
        externalHelp.setFont(font(16));externalHelp.setJustificationType(juce::Justification::topLeft);
        externalHelp.setBorderSize(juce::BorderSize<int>(24));
        externalHelp.setColour(juce::Label::backgroundColourId,juce::Colour(0xf5101518));
        externalHelp.setColour(juce::Label::outlineColourId,amber);externalHelp.setInterceptsMouseClicks(true,true);
        updateModern();startTimerHz(60);
    }
    ~Panel() override {
        stopTimer();releaseComputerNotes();removeKeyListener(this);sizing.onChange=nullptr;if(bendActive)processor.queuePitchBend(0);setLookAndFeel(nullptr);
    }
    void paint(juce::Graphics& g) override {
        if(!renderingCondition&&resizeBackdrop.isValid()&&resizeCacheUntil>0){g.drawImage(resizeBackdrop,getLocalBounds().toFloat());return;}
        if(look.conditionDragging&&!renderingCondition) {
            const float amount=juce::jlimit(0.0f,1.0f,processor.state.getRawParameterValue("age")->load())*4;
            const int low=juce::jmin(3,int(amount));
            const juce::Rectangle<float> body(0,float(headerH),float(designW),float(bodyH));
            if(agePreviews[0].isValid()&&agePreviews[4].isValid()) {
                g.drawImage(agePreviews[size_t(low)],body);
                g.setOpacity(amount-low);g.drawImage(agePreviews[size_t(low+1)],body);g.setOpacity(1);
            } else {
                // Even a gesture immediately after opening cannot force a
                // detailed render onto the input path. Use a warmed frame if
                // available; otherwise show the embedded body until release.
                const juce::Image* available=nullptr;
                for(const auto& frame:agePreviews)if(frame.isValid()){available=&frame;break;}
                if(available!=nullptr)g.drawImage(*available,body);
                else {const auto& photo=artwork().chassis;g.drawImage(photo,0,headerH,designW,bodyH,2,7,1770,873);}
            }
            paintFooter(g);return;
        }
        look.age=previewAge>=0?previewAge:processor.state.getRawParameterValue("age")->load();
        const float age=look.age;
        const auto ink=agedInk(age);
        const auto& art=artwork();g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        {
            juce::Graphics::ScopedSaveState save(g);
            juce::Path outline;outline.addRoundedRectangle(0,float(headerH),float(designW),float(bodyH),14);
            g.reduceClipRegion(outline);
            // Crop the empty photo border; the rounded chassis sits against the editor's dark surround.
            const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
                juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),
                juce::roundToInt(art.chassis.getHeight()*873.0/887));
            g.drawImage(art.chassis,0,headerH,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            if(age>0) {
                // Registered photographic material endpoint. Only metal ages;
                // rust does not spill onto the molded side cheeks or empty keybed.
                juce::Path metal;
                metal.addRectangle(55.0f,float(headerH),1389.0f,365.0f);
                metal.addRectangle(65.0f,float(headerH+366),182.0f,335.0f);
                g.reduceClipRegion(metal);g.setOpacity(.90f*std::pow(age,.85f));
                g.drawImage(art.chassisAged,0,headerH,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            }
            // The close-up places the upper POWER screw on the cap axis.
            // Cover its earlier baked corner location with nearby registered
            // panel material before drawing the relocated head and panel ink.
            const float mountX=(277.0f-2)*designW/1770,mountY=headerH+(457.0f-7)*bodyH/873;
            const float materialShift=32.0f*designW/1770;
            for(int edge=0;edge<6;++edge) {
                juce::Graphics::ScopedSaveState patch(g);
                const float radius=12.5f-edge*.3f,opacity=float(edge+1)/6;
                juce::Path mask;mask.addEllipse(mountX-radius,mountY-radius,radius*2,radius*2);g.reduceClipRegion(mask);
                g.setOpacity(opacity);g.drawImage(art.chassis,juce::roundToInt(materialShift),headerH,designW,bodyH,
                    source.getX(),source.getY(),source.getWidth(),source.getHeight());
                g.setOpacity(opacity*.90f*std::pow(age,.85f));
                g.drawImage(art.chassisAged,juce::roundToInt(materialShift),headerH,designW,bodyH,
                    source.getX(),source.getY(),source.getWidth(),source.getHeight());
            }
        }
        {
            // Replace unwanted photographic heads with neighboring material
            // from the same horizontal rail, at both aging endpoints.
            for(const auto p:{juce::Point<int>(100,457),{602,30},{1175,30},{887,399}}) {
                const float xx=(p.x-2)*float(designW)/1770,yy=headerH+(p.y-7)*float(bodyH)/873;
                juce::Graphics::ScopedSaveState patch(g);
                if(p.y==457)g.reduceClipRegion(juce::Rectangle<int>(69,404,176,323));
                juce::Image material(juce::Image::RGB,100,80,true);
                {juce::Graphics fill(material);
                 fill.drawImage(art.chassis,0,0,100,80,p.x+35,p.y-23,59,47);
                 fill.setOpacity(.90f*std::pow(age,.85f));fill.drawImage(art.chassisAged,0,0,100,80,p.x+35,p.y-23,59,47);}
                for(int ring=0;ring<14;++ring) {
                    juce::Graphics::ScopedSaveState blend(g);
                    const float t=float(ring+1)/14,previous=float(ring)/14;
                    juce::Path mask;mask.addEllipse(xx-25+ring*.65f,yy-20+ring*.45f,50-ring*1.3f,40-ring*.9f);
                    g.reduceClipRegion(mask);g.setOpacity((t-previous)/(1-previous));
                    g.drawImage(material,{xx-25,yy-20,50,40});
                }
            }
        }
        // Independent short scuffs and nicks on the molded cheeks, not rust.
        if(age>0)for(int side=0;side<2;++side) {
            juce::Graphics::ScopedSaveState cheek(g);
            g.reduceClipRegion(juce::Rectangle<int>(side?1447:5,headerH+12,45,bodyH-24));
            for(uint32_t i=0;i<110;++i) {
                const uint32_t seed=surfaceHash(15077u+uint32_t(side)*191911u+i*7919u);
                const float x=(side?1447.0f:5.0f)+45*surfaceNoise(seed),y=headerH+20+705*surfaceNoise(seed+31);
                const float length=1+9*surfaceNoise(seed+53),dy=(surfaceNoise(seed+71)-.5f)*6;
                g.setColour(juce::Colour(i%4?0xff8b8b80:0xff111513).withAlpha(age*age*(i%4?.22f:.65f)));
                g.drawLine(x,y,x+length,y+dy,i%9==0?.85f:.28f);
            }
        }
        caseStains(g,age);
        paintPanelDust(g,age);
        // The narrow rail below the keys has a fine pebble finish. It is a
        // different material from the smooth painted main panel.
        {
            juce::Graphics::ScopedSaveState save(g);
            const juce::Rectangle<float> rail(59.0f,float(headerH+705),1382.0f,43.0f);
            g.reduceClipRegion(rail.toNearestInt());
            g.drawImage(art.rail,rail.getX(),rail.getY(),rail.getWidth(),rail.getHeight(),0,0,art.rail.getWidth(),70);
            g.setColour(juce::Colours::black.withAlpha(.27f+age*.18f));g.fillRect(rail);
            g.setOpacity(std::pow(age,1.25f));g.drawImage(railDust(),rail);g.setOpacity(1);
            g.setColour(juce::Colour(0xff9b9c91).withAlpha(.12f-age*.06f));g.drawLine(rail.getX(),rail.getY(),rail.getRight(),rail.getY(),.7f);
        }
        {
            juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::translation(0,float(headerH)));
            module(g,107,141,"MODULATOR",age);module(g,252,365,"VCO",age);module(g,621,72,"SUB OSC",age);
            module(g,697,146,"AUDIO MIXER",age);module(g,847,251,"VCF",age);module(g,1102,64,"VCA",age);module(g,1170,221,"ENV",age);
            caption(g,juce::String::fromUTF8("UNOFFICIAL PERSONAL PROJECT \xe2\x80\x94 NOT AFFILIATED WITH ROLAND CORPORATION \xe2\x80\x94 NOT FOR SALE"),
                    {107,24,1284,15},10.0f,juce::Justification::centred,true,ink);
            g.setColour(ink);g.drawLine(132,102,165,102,.85f);
            caption(g,"RATE",{167.5f,96,29,12},8.4f,juce::Justification::centred,true,ink);
            panelGlyph(g,PanelGlyph::sine,{205,99,12,7},ink,.85f);
            caption(g,"DELAY\nTIME",{218,92,29,22},7.6f,juce::Justification::centred,true,ink);
            caption(g,"RANGE",{319,96,74,12},9,juce::Justification::centred,true,ink);
            caption(g,"WAVEFORM",{426,96,84,12},9,juce::Justification::centred,true,ink);
            caption(g,"TUNE",{315,75.5f,30,15},8.4f,juce::Justification::centred,true,ink);
            arcLabels(g,{356,163},{{"32'",-1.134464f},{"16'",-.567232f},{"8'",0.0f},{"4'",.567232f},{"2'",1.134464f}},40,age);
            arcLabels(g,{468,163},{{"NOISE",-1.047198f},{"SAW",-.349066f},{"SQR",.349066f},{"PULSE",1.047198f}},40,age);
            caption(g,"PULSE WIDTH MOD",{515,96,100,12},7.6f,juce::Justification::centredLeft,true,ink);
            juce::Path pwmLink;pwmLink.startNewSubPath(509,130);pwmLink.lineTo(509,105);pwmLink.quadraticTo(509,102,512,102);pwmLink.lineTo(513,102);
            g.setColour(ink);g.strokePath(pwmLink,juce::PathStrokeType(.9f));
            juce::Path vcfLink;vcfLink.startNewSubPath(999,116);vcfLink.lineTo(999,106);vcfLink.quadraticTo(999,103,1002,103);vcfLink.lineTo(1027,103);vcfLink.quadraticTo(1030,103,1030,106);vcfLink.lineTo(1030,115);
            g.strokePath(vcfLink,juce::PathStrokeType(.85f));
            
            // Original user marks: split into manufacturer on right and instrument on left.
            const float printWear=age*age;
            for(int i=0;i<2;++i) {
                g.setColour(ink.withAlpha(i==0?1-printWear:printWear));
                const auto& printed=i==0?art.marks:art.marksAged;
                g.drawImage(printed,94,256,441,56,846,308,1253,158,true);
                g.drawImage(printed,1175,258,235,51,85,308,722,158,true);
            }
            g.setColour(ink.withAlpha(.94f));g.fillRect(60,312,1381,2);
            // Move the original photographic lower screw row above the bender,
            // replacing the old seats with neighbouring unoccupied panel material.
            for(const auto p:{juce::Point<int>(98,799),{277,799}}) {
                const float xx=float(p.x-2)*designW/1770,yy=float(p.y-7)*bodyH/873;
                g.setOpacity(1);g.drawImage(art.chassis,juce::roundToInt(xx-13),juce::roundToInt(yy-13),26,26,p.x-15,p.y-60,30,30);
                g.setOpacity(age*.94f);g.drawImage(art.chassisAged,juce::roundToInt(xx-13),juce::roundToInt(yy-13),26,26,p.x-15,p.y-60,30,30);
            }
            g.setOpacity(1);
            g.setColour(ink.withAlpha(.82f));g.drawRect(69,370,176,323,1);
            caption(g,"VOLUME",{80,376,75,13},10,juce::Justification::centred,true,ink);
            for(int i=0;i<=10;++i) {
                const float a=juce::MathConstants<float>::pi*(1.25f+1.5f*float(i)/10);
                const juce::Point<float> c(117,442),direction(std::sin(a),-std::cos(a));
                g.setColour(ink);g.drawLine({c+direction*41,c+direction*47},1.0f);
                if(i==0||i==10) {const auto p=c+direction*53;caption(g,juce::String(i),{p.x-10,p.y-5,20,10},7.5f,juce::Justification::centred,true,ink);}
            }
            kb09_gui::drawPowerLegend(g,ink,age);
            g.setColour(juce::Colour(0xff080b0e));g.fillEllipse(200.5f,475,9,9);
            g.setColour(juce::Colour(processor.isPowerEnabled()?0xffe54c35:0xff491c16));g.fillEllipse(202.5f,477,5,5);
            if(processor.isPowerEnabled()) {g.setColour(juce::Colour(0xffffaa6f).withAlpha(.65f));g.fillEllipse(203.0f,477.5f,1.8f,1.8f);}
            g.setColour(ink.withAlpha(.76f));g.drawRect(174.0f,489.0f,61.0f,134.0f,.8f);
            juce::Path bendLink;bendLink.startNewSubPath(98,600);bendLink.lineTo(98,614);bendLink.quadraticTo(98,617,101,617);bendLink.lineTo(153,617);bendLink.quadraticTo(156,617,156,620);bendLink.lineTo(156,624);
            bendLink.startNewSubPath(142,600);bendLink.lineTo(142,617);
            g.setColour(ink);g.strokePath(bendLink,juce::PathStrokeType(.9f));
            caption(g,"BENDER",{105,628,103,13},10,juce::Justification::centred,true,ink);
            g.setColour(ink);g.drawLine(104,682,150,682,.9f);g.drawLine(163,682,209,682,.9f);
            juce::Path arrows;arrows.startNewSubPath(99,682);arrows.lineTo(105,679.5f);arrows.lineTo(105,684.5f);arrows.closeSubPath();arrows.startNewSubPath(214,682);arrows.lineTo(208,679.5f);arrows.lineTo(208,684.5f);arrows.closeSubPath();g.fillPath(arrows);
            caption(g,"-",{86,676,12,12},10,juce::Justification::centred,true,ink);
            caption(g,"0",{150,676,13,12},8,juce::Justification::centred,true,ink);
            caption(g,"+",{215,676,12,12},10,juce::Justification::centred,true,ink);
            // Screw heads and contact shadows are a foreground layer over ink.
            // Repaint tightly clipped registered photographic heads after rules.
            for(const auto p: {juce::Point<float>(98,799),{277,799}}) {
                juce::Graphics::ScopedSaveState save(g);
                const float shift=p.y>790?-40.0f:0.0f;
                const float xx=(p.x-2)*float(designW)/1770,yy=(p.y-7)*float(bodyH)/873+shift;
                juce::Path head;head.addEllipse(xx-11.5f,yy-11.5f,23,23);g.reduceClipRegion(head);
                g.setOpacity(1);g.drawImage(art.chassis,0,juce::roundToInt(shift),designW,bodyH,2,7,1770,873);
                g.setOpacity(age*.94f);g.drawImage(art.chassisAged,0,juce::roundToInt(shift),designW,bodyH,2,7,1770,873);
            }
            uint32_t screwSeed=5011;
            for(const auto p:{juce::Point<float>(88,37),{602,30},{1175,30},{1686,37},
                              {88,328},{1686,328},{83,399},{1692,399},
                              {98,799},{277,799}}) {
                screwOxidation(g,{(p.x-2)*float(designW)/1770,(p.y-7)*float(bodyH)/873+(p.y>790?-40.0f:0.0f)},age,screwSeed);
                screwSeed+=7717;
            }
            kb09_gui::drawPowerScrew(g,{205,397},age,false);
            kb09_gui::drawPowerScrew(g,{205,451},age,true);
        }
        paintFooter(g);
    }
private:
    void paintFooter(juce::Graphics& g) {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff222729),0,float(headerH+bodyH),
                                            juce::Colour(0xff121618),0,float(designH),false));
        g.fillRect(0,headerH+bodyH,designW,designH-headerH-bodyH);
        g.setColour(juce::Colour(0xff58605f));g.drawLine(0,float(headerH+bodyH),float(designW),float(headerH+bodyH),.65f);
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
    std::unique_ptr<juce::TooltipWindow> tooltip;juce::Image conditionBackdrop,previousCondition,resizeBackdrop;std::array<juce::Image,5> agePreviews;std::vector<juce::Component::SafePointer<juce::Component>> hiddenForCondition;uint64_t previewParameters=0;double previewIdleAfter=0;float previewAge=-1;bool renderingCondition=false;double resizeCacheUntil=0;
    FineSlider bender;juce::Label externalHelp;Meter meter;RateLamp rateLamp;
    std::vector<std::unique_ptr<Control>> controls;
    std::array<int,128> lastKeys{};float lastAge=-1;bool lastModern=false,bendActive=false,userPreset=false;int lastProgram=-1,lastDisplayedNote=-1,lastDisplayedKeyboardNote=-1;unsigned lastProgramRevision=0;double lastConditionPaint=0;
    std::array<bool,13> computerHeld{};
    int computerOctave=4;
    static constexpr const char* computerKeys="AWSEDFTGYHUJK";
    void updateComputerLabel() {computerKeyboard.setButtonText(computerKeyboard.getToggleState()?"KEYBOARD C"+juce::String(computerOctave):"KEYBOARD");}
    void releaseComputerNotes() {
        for(size_t i=0;i<computerHeld.size();++i)if(computerHeld[i]) {
            processor.queueKeyboard((computerOctave+1)*12+int(i),false,false);computerHeld[i]=false;
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
            if(down!=computerHeld[i]){computerHeld[i]=down;processor.queueKeyboard((computerOctave+1)*12+int(i),down,false);}
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
        if(processor.wrapperType!=juce::AudioProcessor::wrapperType_Undefined&&isShowing()&&getPeer()){if(!updateOpened){updateOpened=true;kb09::Updates::instance().opened();}kb09::Updates::instance().deliver();}
        if(!computerReady())releaseComputerNotes();
        else for(size_t i=0;i<computerHeld.size();++i)if(computerHeld[i]&&!juce::KeyPress::isKeyCurrentlyDown(computerKeys[i])) {
            computerHeld[i]=false;processor.queueKeyboard((computerOctave+1)*12+int(i),false,false);
        }
        const auto programRevision=processor.getProgramRevision();
        const int program=processor.getCurrentProgram();
        if(!userPreset||program!=lastProgram||programRevision!=lastProgramRevision){presets.setSelectedId(program+1,juce::dontSendNotification);userPreset=false;}
        lastProgram=program;lastProgramRevision=programRevision;
        meter.setLevels(processor.displayedEnvelope.load(),processor.displayedFollower.load(),processor.displayedInput.load());
        const double stamp=processor.displayedModulatorTime.load(std::memory_order_acquire);
        const double elapsed=juce::jmax(0.0,(juce::Time::getMillisecondCounterHiRes()-stamp)*.001);
        const float hz=processor.displayedModulatorFrequency.load(std::memory_order_relaxed);
        const double phase=processor.displayedModulatorPhase.load(std::memory_order_relaxed)+elapsed*hz;
        const float lamp=hz>=8?.48f+.52f*juce::jlimit(0.0f,1.0f,(hz-8)/24):float(phase-std::floor(phase)<.5);
        rateLamp.setLevel(processor.isPowerEnabled()&&hz>0?lamp:0);
        bool keysChanged=false;
        for(size_t i=0;i<lastKeys.size();++i){const int n=processor.keyDisplay[i].load(std::memory_order_relaxed);if(lastKeys[i]!=n){lastKeys[i]=n;keysChanged=true;}}
        const int displayedNote=processor.displayedNote.load(std::memory_order_relaxed);
        if(displayedNote!=lastDisplayedNote){lastDisplayedNote=displayedNote;keysChanged=true;}
        const int displayedKeyboardNote=processor.displayedKeyboardNote.load(std::memory_order_relaxed);
        if(displayedKeyboardNote!=lastDisplayedKeyboardNote){lastDisplayedKeyboardNote=displayedKeyboardNote;keysChanged=true;}
        if(keysChanged)keyboard.repaint();
        const float age=processor.state.getRawParameterValue("age")->load();
        // Coalesce full-instrument wear redraws while the AGE thumb remains live.
        const auto now=juce::Time::getMillisecondCounterHiRes();
        if(age!=lastAge && (!look.conditionDragging||now-lastConditionPaint>=110)){
            lastConditionPaint=now;lastAge=age;look.age=age;
            for(auto& c:controls)if(c->label&&bool(c->label->getProperties().getWithDefault("hardwarePrint",false)))
                c->label->setColour(juce::Label::textColourId,agedInk(age));
            repaint();if(!look.conditionDragging)keyboard.repaint();
        }
        // Only idle work builds material frames. A drag performs two image
        // blits and a tiny footer repaint, never a full keyboard/knob render.
        if(!look.conditionDragging && ((isShowing()&&getPeer()!=nullptr)||bool(getProperties().getWithDefault("renderConditionPreviews",false)))) {
            uint64_t stateHash=1469598103934665603ull;
            for(const auto& spec:sh09::specs)if(juce::String(spec.id)!="age") {
                const float v=processor.state.getRawParameterValue(spec.id)->load();
                stateHash=(stateHash^uint64_t(juce::roundToInt(v*100000)))*1099511628211ull;
            }
            if(stateHash!=previewParameters){previewParameters=stateHash;previewIdleAfter=now+250;getProperties().set("conditionPreviewReady",false);for(auto& image:agePreviews)image={};}
            if(now>=previewIdleAfter&&!juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())for(size_t i=0;i<agePreviews.size();++i)if(!agePreviews[i].isValid()) {
                const float originalAge=look.age;
                previewAge=float(i)/4;renderingCondition=true;look.age=previewAge;look.renderingPreview=true;
                look.conditionDragging=true; // Low-cost material rasterisation while caching.
                keyboard.setBufferedToImage(false);bender.setBufferedToImage(false);power.setBufferedToImage(false);
                for(auto& c:controls)if(c->slider)c->slider->setBufferedToImage(false);
                agePreviews[i]=createComponentSnapshot({0,headerH,designW,bodyH},true,900.0f/designW);
                look.conditionDragging=false;look.renderingPreview=false;
                previewAge=-1;renderingCondition=false;look.age=originalAge;
                getProperties().set("conditionPreviewReady",agePreviews[0].isValid()&&agePreviews[4].isValid());
                break;
            }
        }
        if(resizeCacheUntil>0&&now>=resizeCacheUntil) {
            resizeCacheUntil=0;resizeBackdrop={};bender.setBufferedToImage(false);power.setBufferedToImage(false);
            for(auto& c:controls) {if(c->slider)c->slider->setBufferedToImage(false);if(c->selector)c->selector->setBufferedToImage(false);}
            keyboard.setBufferedToImage(false);repaint();keyboard.repaint();
        }
        const bool modern=processor.state.getRawParameterValue("modern")->load()>.5f;
        if(modern!=lastModern)updateModern();
    }
    void updateModern() {
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
        if(juce::String(id)=="tune")slider.setTooltip("Fine-tune oscillator pitch in cents.");
        if(juce::String(id)=="range")slider.setTooltip("Choose the oscillator octave range.");
        if(juce::String(id)=="wave")slider.setTooltip("Choose noise, saw, square or pulse as the main oscillator source.");
        if(juce::String(id)=="pulseWidth")slider.setTooltip("Set pulse width manually, or the amount of pulse-width modulation.");
        if(juce::String(id)=="vcoLevel")slider.setTooltip("Mix the main oscillator into the filter.");
        if(juce::String(id)=="subLevel")slider.setTooltip("Mix the sub-oscillator into the filter.");
        if(juce::String(id)=="extLevel")slider.setTooltip("Mix incoming external audio into the filter; watch the IN indicator.");
        if(juce::String(id)=="lfoRate")slider.setTooltip("Set modulation speed. In Modern SYNC, choose a musical division of the host tempo.");
        if(juce::String(id)=="lfoDelay")slider.setTooltip("Delay the onset of modulation after a note starts.");
        if(juce::String(id)=="vcoMod")slider.setTooltip("Set the amount of LFO pitch modulation.");
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
        if(juce::String(id)=="age")slider.setTooltip("Age the circuitry and appearance. Click anywhere on the track to jump there, or drag; SHIFT gives fine adjustment. Appearance previews update while dragging; full visual detail returns on release.");
        if(juce::String(id)=="velocity")slider.setTooltip("In Modern mode, set how strongly playing velocity affects the sound.");
        if(juce::String(id)=="aftertouch")slider.setTooltip("In Modern mode, set the amount of pressure-controlled expression.");
        if(juce::String(id)=="extGain")slider.setTooltip("In Modern mode, adjust external input gain before the follower and mixer.");
slider.setDoubleClickReturnValue(true,spec.initial);}
        addAndMakeVisible(slider);c->attachment=std::make_unique<SliderAttachment>(processor.state,id,slider);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id&&spec.choices[0]==0) {
            if(juce::String(id)=="age")slider.textFromValueFunction=[](double v){
                constexpr std::array<const char*,10> phrases{{"Nice 'n' New","Barely Broken In","Like New-ish",
                    "Lightly Loved","Seen Some Shit","Vintage, Baby!","Sold As-Is","Smells Like 1983",
                    "Fire Hazard","Who Let You Have This?"}};
                return juce::String(phrases[size_t(juce::jlimit(0,9,int(std::floor(v*10+1.0e-6))))]);
            };
            else if(juce::String(id)=="tune")slider.textFromValueFunction=[](double v){return juce::String(v,1)+" ct";};
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
        const bool hardware=headerH+y<headerH+bodyH;
        c.label->getProperties().set("hardwarePrint",hardware);
        if(hardware)c.label->setColour(juce::Label::textColourId,agedInk(processor.state.getRawParameterValue("age")->load()));
    }
    void addFader(const char* id,const char* name,int centre,int y,int w,int h,bool compact=false) {
        addSlider(id,name,centre-w/2,y+20,w,h-11,juce::Slider::LinearVertical);
        auto& c=*controls.back();c.slider->getProperties().set("compact",compact);
        // Printed scale numerals extend past the narrow hit area, without
        // overlapping the neighbouring fader's interaction bounds.
        c.slider->setPaintingIsUnclipped(true);
        addLabel(c,name,centre-w/2-8,y-1,w+16,20,juce::String(id)=="portamento"?7.1f:8.4f);
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
#if JUCE_WINDOWS
    if(p.wrapperType==juce::AudioProcessor::wrapperType_Standalone) {
        juce::Component::SafePointer<SH09Editor> editor(this);
        juce::MessageManager::callAsync([editor] {
            if(editor==nullptr)return;
            if(auto* window=editor->findParentComponentOfClass<juce::DocumentWindow>()) {
                if(!window->isOnDesktop())return;
                window->setName("Toyland KB-09 Standalone");
                window->setUsingNativeTitleBar(true);
                window->setTitleBarButtonsRequired(juce::DocumentWindow::allButtons,false);
                editor->sizing.peerAttached();
                window->toFront(true);
            }
        });
    }
#endif
}
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
        if(software>=0)peer->setCurrentRenderingEngine(software);
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


