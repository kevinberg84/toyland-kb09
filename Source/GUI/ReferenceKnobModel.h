#pragma once
#include <juce_graphics/juce_graphics.h>
#include <array>
#include <cmath>
#include "KnobPlasticLighting.h"
#include "KnobGrooveDust.h"
namespace kb09_gui {
// Right cylindrical grip with rounded trapezoid ribs and a square annular lip.
// The inner wall has CONSTANT radius at both heights: exactly 90 degrees.
// No generated photograph supplies this geometry.
class ReferenceKnobModel {
public:
    explicit ReferenceKnobModel(const juce::Image& reference,const juce::Image& originalPlastic={}):photo(reference),legacyRim(originalPlastic),collar(reference.rescaled(size,size,juce::Graphics::highResamplingQuality)) {
        // Continue the real silver collar below the new cylindrical geometry.
        juce::Image::BitmapData pixels(collar,juce::Image::BitmapData::readWrite),source(photo,juce::Image::BitmapData::readOnly);
        for(int y=0;y<size;++y)for(int x=0;x<size;++x) {
            const float u=(x+.5f)/size,v=(y+.5f)/size,dx=u-cx;
            const float a=(v-topY)/aspect,b=(v-baseY)/aspect;
            if(dx*dx+a*a>(maxR+.012f)*(maxR+.012f)&&dx*dx+b*b>(maxR+.012f)*(maxR+.012f)&&!(std::abs(dx)<maxR+.012f&&v>=topY&&v<=baseY))continue;
            const float angle=std::atan2(v-.505f,dx);
            const int sx=juce::jlimit(0,source.width-1,int((cx+.432f*std::cos(angle))*source.width));
            const int sy=juce::jlimit(0,source.height-1,int((.505f+.432f*std::sin(angle))*source.height));
            pixels.setPixelColour(x,y,source.getPixelColour(sx,sy).withAlpha(juce::uint8(255)));
        }
    }
    const juce::Image& frame(float rotation,size_t slot,float age=0,bool rimShine=false,bool refined132=false,bool detail135=false,bool inner143=false,bool fixed152=false,bool fill161=false,bool broad165=false,bool dust166=false,bool shadow172=false) {
        auto& cache=caches[slot][size_t(juce::jlimit(0,4,juce::roundToInt(age*4)))];if(cache.image.isValid()&&cache.angle==rotation&&cache.age==age&&cache.rimShine==rimShine&&cache.refined132==refined132&&cache.detail135==detail135&&cache.inner143==inner143&&cache.fixed152==fixed152&&cache.fill161==fill161&&cache.broad165==broad165&&cache.dust166==dust166&&cache.shadow172==shadow172)return cache.image;
        cache.angle=rotation;cache.age=age;cache.rimShine=rimShine;cache.refined132=refined132;cache.detail135=detail135;cache.inner143=inner143;cache.fixed152=fixed152;cache.fill161=fill161;cache.broad165=broad165;cache.dust166=dust166;cache.shadow172=shadow172;cache.image=collar.createCopy();juce::Graphics g(cache.image);
        // Short, shallow scuffs on the stationary metal collar. Deterministic
        // per control, with staggered age onset; no gouges or moving wear.
        if(dust166&&age>0) {
            juce::Graphics::ScopedSaveState clipped(g);
            juce::Path band;band.addEllipse((cx-.421f)*size,(.505f-.421f)*size,.842f*size,.842f*size);
            band.addEllipse((cx-.347f)*size,(.505f-.347f)*size,.694f*size,.694f*size);
            band.setUsingNonZeroWinding(false);g.reduceClipRegion(band);
            juce::Random random(170013+juce::int64(slot)*7919);
            for(int j=0;j<64;++j) {
                const float a=random.nextFloat()*tau,r=.350f+.066f*random.nextFloat();
                const float direction=a+.7f+(random.nextFloat()-.5f)*2.4f;
                const float length=7.f+random.nextFloat()*13.f;
                const float onset=.10f+.72f*random.nextFloat();
                const float strength=juce::jlimit(0.f,1.f,(age-onset)/.35f);
                const juce::Point<float> p((cx+r*std::cos(a))*size,(.505f+r*std::sin(a))*size);
                const juce::Point<float> delta(std::cos(direction)*length,std::sin(direction)*length);
                juce::Path scuff;scuff.startNewSubPath(p);scuff.quadraticTo(p+delta*.45f+juce::Point<float>(.4f,-.3f),p+delta);
                g.setColour(juce::Colour(0xff454843).withAlpha(strength*.52f));
                g.strokePath(scuff,juce::PathStrokeType(1.15f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                g.saveState();g.addTransform(juce::AffineTransform::translation(-.25f,-.4f));
                g.setColour(juce::Colour(0xffe4e5dd).withAlpha(strength*.72f));
                g.strokePath(scuff,juce::PathStrokeType(.85f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));g.restoreState();
            }
        }
        if(shadow172) {
            // The raised cylindrical grip casts a second, shorter shadow onto
            // the metal collar. Paint it before the opaque body, so only its
            // down-right overhang remains visible on the receiving surface.
            juce::Graphics::ScopedSaveState cast(g);
            const float scale=float(size)/(88.f*.95f);
            const juce::Rectangle<float> foot((cx-maxR)*size,(baseY-maxR*aspect)*size,2*maxR*size,2*maxR*aspect*size);
            juce::Path receiver;receiver.addEllipse((cx-.435f)*size,(.505f-.445f)*size,.870f*size,.890f*size);g.reduceClipRegion(receiver);
            for(int i=5;i>0;--i) {
                g.setColour(juce::Colours::black.withAlpha(.07f));
                g.fillEllipse(foot.expanded(float(i)*.45f*scale).translated(2.f*scale,3.f*scale));
            }
        }
        const float innerR=fixed152?KnobPlasticLighting::thickerInnerRadius:KnobPlasticLighting::innerRadius;
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        juce::Path silhouette;silhouette.startNewSubPath(ringPoint(juce::MathConstants<float>::pi,rotation,topY));
        for(int i=segments/2;i<=segments;++i)silhouette.lineTo(ringPoint(float(i)*tau/segments,rotation,topY));
        silhouette.lineTo(ringPoint(0,rotation,baseY));
        for(int i=0;i<=segments/2;++i)silhouette.lineTo(ringPoint(float(i)*tau/segments,rotation,baseY));
        silhouette.closeSubPath();g.setColour(colour(.03f));g.fillPath(silhouette);
        // Three rounded ribs per group, then a broad planar separator.
        // Section widths follow the close-up and earlier full-knob reference.
        for(int i=0;i<segments;++i) {
            const float a=float(i)*tau/segments,b=float(i+1)*tau/segments,m=(a+b)*.5f;
            if(std::sin(m)<=0)continue;
            const auto p=ringPoint(a,rotation,topY),q=ringPoint(b,rotation,topY);
            auto tangent=q-p;juce::Point<float> normal(tangent.y/aspect,-tangent.x);
            normal/=juce::jmax(.00001f,normal.getDistanceFromOrigin());
            const float diffuse=juce::jmax(0.f,-.65f*normal.x-.65f*normal.y);
            const float relief=juce::jlimit(0.f,1.f,((radius(a-rotation)+radius(b-rotation))*.5f-.320f)/.0105f);
            // Ambient occlusion distinguishes the raised triangular flanks
            // from their dark valleys without adding a lower light source.
            const float halfVector=juce::jmax(0.f,-.72f*normal.x+.69f*normal.y);
            const float reflection=std::pow(halfVector,rimShine?68.f:38.f)*(rimShine?.74f:.48f)*(1-age*(rimShine?.40f:.65f));
            const float oldTone=rimShine?.085f+.06f*relief+.25f*diffuse+reflection:.13f+.20f*std::sqrt(relief)+.15f*diffuse+reflection;
            const float crisp=.065f+.10f*diffuse+(.25f+.36f*diffuse)*std::pow(relief,1.6f)+reflection*relief;
            const float tone=fixed152?KnobPlasticLighting::facet(normal.x,normal.y,m,relief,age,fill161,broad165,dust166):refined132?crisp+(oldTone-crisp)*age:oldTone;
            juce::ColourGradient wall(colour(tone*1.10f),0,topY*size,colour(tone*.78f),0,baseY*size,false);
            g.setGradientFill(wall);quad(g,p,q,q+juce::Point<float>(0,(baseY-topY)*size),p+juce::Point<float>(0,(baseY-topY)*size));
            // Adjacent subpixel facets must overlap at their shared edges;
            // separate antialiased quads otherwise leave black coverage seams.
            const auto centre=(p+q)*.5f;
            g.drawLine({centre,centre+juce::Point<float>(0,(baseY-topY)*size)},1.1f);
        }
        if(rimShine&&(age>0||refined132)) {
            // Powder packs the narrow troughs along almost their full height.
            // The broad smooth separators and exposed rounded crests stay
            // dark, as in the user's low side-view reference photograph.
            juce::Image deposits(juce::Image::ARGB,size,size,true,juce::SoftwareImageType());
            {juce::Image::BitmapData pixels(deposits,juce::Image::BitmapData::readWrite);
             const auto noise=[](unsigned seed) {
                 seed^=seed>>16;seed*=0x7feb352du;seed^=seed>>15;seed*=0x846ca68bu;seed^=seed>>16;
                 return float(seed&65535u)/65535.f;
             };
             struct Column {bool visible=false;float a=0,upper=0,relief=0,key=0,lit=0;};
             std::array<Column,size> columns;
             for(int x=0;x<size;++x) {
                const float dx=(x+.5f)/size-cx;if(std::abs(dx)>=maxR)continue;
                auto& c=columns[size_t(x)];c.visible=true;
                float a=std::acos(juce::jlimit(-1.f,1.f,dx/.323f));
                for(int n=0;n<3;++n)a=std::acos(juce::jlimit(-1.f,1.f,dx/radius(a-rotation)));
                c.a=a;c.upper=topY+radius(a-rotation)*aspect*std::sin(a);
                c.relief=juce::jlimit(0.f,1.f,(radius(a-rotation)-.320f)/.0105f);
                c.key=juce::jlimit(0.f,1.f,-std::cos(a)*.8f+.2f);
                if(fixed152) {
                    const float before=a-.0002f,after=a+.0002f;
                    const auto tangent=juce::Point<float>(radius(after-rotation)*std::cos(after)-radius(before-rotation)*std::cos(before),radius(after-rotation)*std::sin(after)-radius(before-rotation)*std::sin(before));
                    auto normal=juce::Point<float>(tangent.y,-tangent.x);normal/=juce::jmax(.00001f,normal.getDistanceFromOrigin());
                    c.lit=KnobPlasticLighting::facet(normal.x,normal.y,a,c.relief,age,fill161,broad165,dust166);
                } else c.lit=juce::jmin(.98f,.055f+(detail135?.96f:.62f)*std::pow(c.relief,detail135?1.7f:1.3f)*(.45f+.55f*c.key));
             }
             auto dustColumns=columns;
             if(dust166) {
                for(auto& c:dustColumns)c.visible=false;
                // Cast each image column against the actual triangular front
                // facets. The old inverse-radius iteration could select a
                // hidden trough behind a visible crest near the sides.
                for(int i=0;i<segments;++i) {
                    const float a=float(i)*juce::MathConstants<float>::pi/segments,b=float(i+1)*juce::MathConstants<float>::pi/segments;
                    const float ra=radius(a-rotation),rb=radius(b-rotation);
                    const float xa=(cx+ra*std::cos(a))*size,xb=(cx+rb*std::cos(b))*size;
                    if(std::abs(xb-xa)<.00001f)continue;
                    const int first=juce::jmax(0,int(std::ceil(juce::jmin(xa,xb)-.5f))),last=juce::jmin(size-1,int(std::floor(juce::jmax(xa,xb)-.5f)));
                    for(int x=first;x<=last;++x) {
                        const float t=((x+.5f)-xa)/(xb-xa),depth=ra*std::sin(a)+(rb*std::sin(b)-ra*std::sin(a))*t;
                        auto& c=dustColumns[size_t(x)];if(c.visible&&c.key>=depth)continue;
                        c.visible=true;c.a=a+(b-a)*t;c.key=depth;c.upper=topY+depth*aspect;
                        c.relief=juce::jlimit(0.f,1.f,(ra+(rb-ra)*t-.320f)/.0105f);
                    }
                }
             }
             for(int y=0;y<size;++y)for(int x=0;x<size;++x) {
                const auto& c=columns[size_t(x)];if(!c.visible)continue;
                const float a=c.a,v=(y+.5f)/size;
                const float height=(v-c.upper)/(baseY-topY);if(height<0||height>1)continue;
                if(refined132&&age<1) {
                    const float value=fixed152?c.lit*(1-height*.12f):juce::jmin(.98f,(.055f+(detail135?.96f:.62f)*std::pow(c.relief,detail135?1.7f:1.3f)*(.45f+.55f*c.key))*(1-height*.12f));
                    pixels.setPixelColour(x,y,colour(value).withAlpha(1-age));
                }
                const auto& deposit=dust166?dustColumns[size_t(x)]:c;if(!deposit.visible)continue;
                const float dustHeight=(v-deposit.upper)/(baseY-topY);if(dustHeight<0||dustHeight>1)continue;
                float phase=std::fmod((deposit.a-rotation)/tau*40.f,5.f);if(phase<0)phase+=5;
                const float trough=std::round(phase);
                if(!dust166&&trough>3&&trough<5)continue;
                const float distance=std::abs(phase-trough);
                const float coverage=dust166?knobTroughDust(phase,deposit.relief):std::exp(-distance*distance/.065f);
                if(coverage<.02f)continue;
                const unsigned seed=unsigned((deposit.a-rotation+tau*8)*1300)+unsigned(dustHeight*97)*7919u+unsigned(slot)*31337u;
                const float grain=noise(seed),density=.80f+.20f*grain;
                const float opacity=std::pow(age,1.3f)*coverage*density*(dust166?.74f:.86f);
                const float value=.43f+.14f*grain;
                pixels.setPixelColour(x,y,pixels.getPixelColour(x,y).overlaidWith(juce::Colour::fromFloatRGBA(value,value*.98f,value*.88f,opacity)));
             }}
            g.drawImageAt(deposits,0,0);
        }
        // Photographic, flat machined silver floor, recessed below the lip.
        {juce::Graphics::ScopedSaveState face(g);juce::Path disk;disk.addEllipse((cx-innerR)*size,(faceY-innerR*aspect)*size,innerR*2*size,innerR*aspect*2*size);g.reduceClipRegion(disk);
         g.addTransform(juce::AffineTransform::translation(-cx*photo.getWidth(),-.407f*photo.getHeight())
             .scaled((innerR/.267f)*size/photo.getWidth(),(innerR*aspect/.277f)*size/photo.getHeight()).translated(cx*size,faceY*size));
         g.drawImageAt(photo,0,0);}
        if(dust166&&age>.20f) {
            cache.grime=juce::Image(juce::Image::ARGB,size,size,true,juce::SoftwareImageType());
            juce::Graphics grimeGraphics(cache.grime);
            juce::Graphics::ScopedSaveState faceGrime(grimeGraphics);
            juce::Path disk;disk.addEllipse((cx-innerR)*size,(faceY-innerR*aspect)*size,innerR*2*size,innerR*aspect*2*size);grimeGraphics.reduceClipRegion(disk);
            juce::Random grimeRandom(179901+juce::int64(slot)*4271);
            // Broad, ragged deposits grow inward from the face/rim seam.
            // Every patch touches the edge; the centre receives no new spots.
            for(int j=0;j<5;++j) {
                const float centreAngle=float(j)*2.399963f+.55f*grimeRandom.nextFloat()+rotation;
                const float halfArc=.23f+.42f*grimeRandom.nextFloat();
                const float depth=innerR*(.075f+.12f*grimeRandom.nextFloat());
                const float onset=.19f+.49f*grimeRandom.nextFloat();
                const float strength=juce::jlimit(0.f,1.f,(age-onset)/.37f);
                const float opacity=(.40f+.24f*grimeRandom.nextFloat())*strength;
                const auto point=[&](float angle,float r) {return juce::Point<float>((cx+r*std::cos(angle))*size,(faceY+r*aspect*std::sin(angle))*size);};
                juce::Path patch;patch.startNewSubPath(point(centreAngle-halfArc,innerR+.002f));
                for(int k=1;k<=28;++k)patch.lineTo(point(centreAngle-halfArc+2*halfArc*float(k)/28,innerR+.002f));
                for(int k=28;k>=0;--k) {
                    const float t=float(k)/28;
                    const float envelope=std::pow(juce::jmax(0.f,std::sin(t*juce::MathConstants<float>::pi)),.50f);
                    const float ragged=.55f+.65f*grimeRandom.nextFloat();
                    patch.lineTo(point(centreAngle-halfArc+2*halfArc*t,innerR-depth*envelope*ragged));
                }
                patch.closeSubPath();
                grimeGraphics.setColour(juce::Colour(0xff66553c).withAlpha(opacity*.20f));
                grimeGraphics.strokePath(patch,juce::PathStrokeType(1.1f));
                grimeGraphics.setColour(juce::Colour(0xff66553c).withAlpha(opacity));grimeGraphics.fillPath(patch);
            }
        } else cache.grime={};
        // Back half of the VERTICAL inner wall. Same innerR on top and bottom.
        for(int i=0;i<segments;++i) {
            const float a=float(i)*tau/segments,b=float(i+1)*tau/segments,m=(a+b)*.5f;
            if(std::sin(m)>=0)continue;
            const float diffuse=juce::jmax(0.f,.65f*std::cos(m)+.65f*std::sin(m));
            g.setColour(colour(.020f+.045f*diffuse));
            quad(g,innerPoint(a,topY,innerR),innerPoint(b,topY,innerR),innerPoint(b,faceY,innerR),innerPoint(a,faceY,innerR));
        }
        // Flat annular top: the approved rib crests stay exposed at the edge.
        juce::Path annulus;annulus.startNewSubPath(ringPoint(0,rotation,topY));
        for(int i=1;i<=segments;++i)annulus.lineTo(ringPoint(float(i)*tau/segments,rotation,topY));annulus.closeSubPath();
        annulus.addEllipse((cx-innerR)*size,(topY-innerR*aspect)*size,innerR*2*size,innerR*aspect*2*size);annulus.setUsingNonZeroWinding(false);
        g.setColour(colour(.065f));g.fillPath(annulus);
        for(int i=0;i<segments;++i) {
            const float a=float(i)*tau/segments,b=float(i+1)*tau/segments,m=(a+b)*.5f;
            // A restrained glint on the flat lip and sharp edge, from top left.
            const float key=juce::jmax(0.f,(-std::cos(m)-std::sin(m))*.70710678f);
            if(!rimShine&&key>.4f) {
                g.setColour(colour(.065f+.15f*std::pow(key,8.f)*(1-age*.72f)));
                quad(g,ringPoint(a,rotation,topY),ringPoint(b,rotation,topY),innerPoint(b,topY,innerR),innerPoint(a,topY,innerR));
                g.setColour(colour(.085f+.29f*std::pow(key,16.f)*(1-age*.72f)));
                g.drawLine({ringPoint(a,rotation,topY),ringPoint(b,rotation,topY)},.55f);
            }
        }
        if(rimShine) {
            // Shade the actual annular surface in one continuous image, rather
            // than independently antialiased facets. Each glint occupies only
            // the adjacent HALF of its radial width, with the whole arc intact.
            juce::Image light(juce::Image::ARGB,size,size,true,juce::SoftwareImageType());
            {juce::Image::BitmapData pixels(light,juce::Image::BitmapData::writeOnly);
             for(int y=0;y<size;++y)for(int x=0;x<size;++x) {
                const float dx=(x+.5f)/size-cx,dy=((y+.5f)/size-topY)/aspect;
                const float a=std::atan2(dy,dx),r=std::hypot(dx,dy),outer=radius(a-rotation);
                if(r<innerR||r>outer)continue;
                const float radial=(r-innerR)/(outer-innerR);
                const float key=radial>=.5f?juce::jmax(0.f,(-std::cos(a)-std::sin(a))*.70710678f):juce::jmax(0.f,(std::cos(a)+std::sin(a))*.70710678f);
                // Fire Hazard keeps the old shine at AGE=.625, midway
                // between Vintage and Sold As-Is, with a linear progression.
                const float shine=std::pow(key,6.f)*(1-age*.375f);
                float value=.065f+.54f*shine;
                if(refined132&&!fixed152&&legacyRim.isValid()) {
                    const float sampleR=.256f+.070f*radial;
                    const int sx=juce::jlimit(0,legacyRim.getWidth()-1,int((cx+sampleR*std::cos(a))*legacyRim.getWidth()));
                    const int sy=juce::jlimit(0,legacyRim.getHeight()-1,int((topY+sampleR*aspect*std::sin(a))*legacyRim.getHeight()));
                    const float photographed=legacyRim.getPixelAt(sx,sy).getBrightness();
                    const float detail=juce::jlimit(0.f,1.f,photographed*2.2f);
                    value=.035f+(.36f+.55f*detail)*shine;
                    if((inner143||fixed152)&&radial<.5f)value=.065f+(.80f+.12f*detail)*shine;
                }
                if(fixed152) {
                    // This is a flat, square-edged lip. Constant intensity
                    // ACROSS each narrow radial band avoids the photographic
                    // rounded/bevelled profile. Only its arc fades with light.
                    value=.045f;
                    if(radial>=.5f&&legacyRim.isValid()) {
                        // Restore proposal 152's upper-left outer reflection;
                        // retain proposal 154's square inner edge and geometry.
                        const float sampleR=.256f+.070f*radial;
                        const int sx=juce::jlimit(0,legacyRim.getWidth()-1,int((cx+sampleR*std::cos(a))*legacyRim.getWidth()));
                        const int sy=juce::jlimit(0,legacyRim.getHeight()-1,int((topY+sampleR*aspect*std::sin(a))*legacyRim.getHeight()));
                        const float detail=juce::jlimit(0.f,1.f,legacyRim.getPixelAt(sx,sy).getBrightness()*2.2f);
                        value+=(.36f+.55f*detail)*shine;
                    }
                    // A subdued bounce glint belongs on the outside edge of
                    // the lip. Leave the inner boundary dark and circular.
                    if(radial>=.78f) {
                        const float bounce=juce::jmax(0.f,(std::cos(a)+std::sin(a))*.70710678f);
                        value+=.38f*std::pow(bounce,6.f)*(1-age*.375f);
                    }
                    if(radial<.10f)value=.018f;
                }
                pixels.setPixelColour(x,y,colour(value));
             }}
            juce::Graphics::ScopedSaveState clipped(g);g.reduceClipRegion(annulus);g.drawImageAt(light,0,0);
        }
        if(age>.15f) {
            juce::Graphics::ScopedSaveState wear(g);juce::Path lip;lip.addEllipse((cx-maxR)*size,(topY-maxR*aspect)*size,maxR*2*size,maxR*aspect*2*size);
            juce::Path hole;hole.addEllipse((cx-innerR)*size,(topY-innerR*aspect)*size,innerR*2*size,innerR*aspect*2*size);lip.addPath(hole);lip.setUsingNonZeroWinding(false);g.reduceClipRegion(lip);
            for(int j=0;j<8;++j) {
                const float onset=.18f+j*.085f,strength=juce::jlimit(0.f,1.f,(age-onset)/.28f);if(strength<=0)continue;
                const float a=rotation+j*2.399963f+.37f*float(slot),r=innerR+.008f+.018f*float((j*7)%5)/4;
                const auto p=juce::Point<float>((cx+r*std::cos(a))*size,(topY+r*aspect*std::sin(a))*size);
                const auto direction=juce::Point<float>(std::cos(a),aspect*std::sin(a));
                g.setColour(juce::Colour(0xff060707).withAlpha(strength));g.drawLine({p-direction*2.1f,p+direction*1.1f},1.1f+.4f*(j%3));
                g.setColour(juce::Colour(0xff777b75).withAlpha(strength*.6f));g.drawLine({p-direction*1.6f+juce::Point<float>(.3f,-.4f),p+direction*.7f+juce::Point<float>(.3f,-.4f)},.48f);
            }
        }
        return cache.image;
    }
    const juce::Image& grimeFrame(size_t slot,float age)const {return caches[slot][size_t(juce::jlimit(0,4,juce::roundToInt(age*4)))].grime;}
private:
    static constexpr int size=512,segments=720;
    static constexpr float cx=.497f,topY=.410f,faceY=.422f,baseY=.488f,maxR=.3305f,innerR=.280f,aspect=.315f/.326f,tau=juce::MathConstants<float>::twoPi;
    float radius(float angle)const {
        float p=std::fmod(angle/tau*40.f,5.f);if(p<0)p+=5;
        float relief=0;
        for(int rib=0;rib<3;++rib) {
            // The close reference shows gently rounded shoulders, a short
            // nearly flat crest, and a smooth transition into each trough.
            const float shoulder=juce::jlimit(0.f,1.f,(std::abs(p-(rib+.5f))-.055f)/.225f);
            relief=juce::jmax(relief,1.f-shoulder*shoulder*(3.f-2.f*shoulder));
        }
        // Eight repeats: three rib pitches and a two-pitch flat section.
        // Its centre is a true chord plane, not an arc of the cylinder.
        const float local=p-4.f,pitch=tau/40.f;
        const float shoulder=juce::jlimit(0.f,1.f,(std::abs(local)-.80f)/.20f);
        const float join=1.f-shoulder*shoulder*(3.f-2.f*shoulder);
        const float planeRadius=maxR*std::cos(.80f*pitch)/std::cos(juce::jlimit(-.80f,.80f,local)*pitch);
        return juce::jmax(.320f+.0105f*relief,.320f+(planeRadius-.320f)*join);
    }
    juce::Point<float> ringPoint(float angle,float rotation,float y)const {const float r=radius(angle-rotation);return {(cx+r*std::cos(angle))*size,(y+r*aspect*std::sin(angle))*size};}
    juce::Point<float> innerPoint(float a,float y,float r)const{return {(cx+r*std::cos(a))*size,(y+r*aspect*std::sin(a))*size};}
    static juce::Colour colour(float v){return juce::Colour::fromFloatRGBA(v,v*.99f,v*.98f,1);}
    static void quad(juce::Graphics& g,juce::Point<float>a,juce::Point<float>b,juce::Point<float>c,juce::Point<float>d){juce::Path p;p.startNewSubPath(a);p.lineTo(b);p.lineTo(c);p.lineTo(d);p.closeSubPath();g.fillPath(p);}
    struct Cache {float angle=0,age=-1;bool rimShine=false,refined132=false,detail135=false,inner143=false,fixed152=false,fill161=false,broad165=false,dust166=false,shadow172=false;juce::Image image,grime;};std::array<std::array<Cache,5>,4> caches;
    juce::Image photo,legacyRim,collar;
};
}
