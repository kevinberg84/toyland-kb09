#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>

namespace kb09_gui {
// One highlight for the disc and finger tab: preserve its interior brightness,
// but replace the diffuse outer tail with a tight, continuous boundary.
inline float polishedBenderHighlight(float alignment,float age) {
    const float broad=std::pow(juce::jlimit(0.f,1.f,alignment),60.f-24.f*age);
    const float edge=juce::jlimit(0.f,1.f,(broad-.14f)/(.055f+.065f*age));
    return broad*edge*edge*(3.f-2.f*edge);
}
// A narrow rectangular studio-light reflection gives polished black plastic
// a crisp highlight. Its edges follow reflected rays and surface curvature,
// rather than painting a bright stripe directly on the screen.
inline float benderHardReflection(float nx,float ny,float nz,float age) {
    constexpr float viewSin=.190809f,viewCos=.981627f;
    const float facing=ny*viewSin+nz*viewCos;
    const float rx=2.f*facing*nx,ry=2.f*facing*ny-viewSin,rz=2.f*facing*nz-viewCos;
    const float softness=.010f+.014f*age;
    const auto rise=[&](float edge,float value) {
        const float t=juce::jlimit(0.f,1.f,(value-edge+softness)/(2*softness));return t*t*(3-2*t);
    };
    return rise(-.77f,rx)*(1-rise(-.68f,rx))*rise(-.36f,ry)*(1-rise(.52f,ry))*rise(.60f,rz);
}
// Fixed object-space handling scratches, with equal count increments across
// the five conditions. Texture construction happens once, never while dragging.
struct BenderScuffMaterial {
    std::array<juce::Image,5> rim,tab,keyRim,keyTab;
    static float random(unsigned value) {
        value^=value>>16;value*=0x7feb352du;value^=value>>15;value*=0x846ca68bu;value^=value>>16;
        return float(value&65535u)/65535.f;
    }
    BenderScuffMaterial() {
        for(int stage=0;stage<5;++stage)for(int surface=0;surface<2;++surface) {
            auto& image=(surface?tab:rim)[size_t(stage)];
            const float width=surface?14.f:juce::MathConstants<float>::twoPi*60.f;
            image=juce::Image(juce::Image::ARGB,int(std::ceil(width*4)),54,true,juce::SoftwareImageType());
            juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale(4.f));
            const unsigned count=unsigned((surface?16:56)*stage/4);
            for(unsigned i=0;i<count;++i) {
                const unsigned key=(surface?0x472a113u:0x83e61a9u)+i*7919u;
                const float x=width*(.06f+.86f*random(key)),y=1.f+10.8f*random(key+317);
                const float length=.25f+(surface?2.3f:6.4f)*std::pow(random(key+59),2.2f);
                const float direction=(random(key+97)-.5f)*(surface?3.5f:1.8f);
                const float dx=length*std::cos(direction),dy=length*std::sin(direction);
                const float bow=(random(key+241)-.5f)*length*.24f;
                const auto point=[&](float t){return juce::Point<float>(x+dx*t,y+dy*t+bow*4*t*(1-t));};
                // Taper each abrasion to fine ends. Fixed, irregular lengths
                // avoid repeated white dashes; alpha stores roughness coverage.
                for(int segment=0;segment<5;++segment) {
                    const float a=float(segment)/5,b=float(segment+1)/5;
                    const float taper=std::sin(juce::MathConstants<float>::pi*(a+b)*.5f);
                    juce::Path scratch;scratch.startNewSubPath(point(a));
                    scratch.quadraticTo(point((a+b)*.5f),point(b));
                    g.setColour(juce::Colours::white.withAlpha((.10f+.13f*random(key+131))*taper));
                    g.strokePath(scratch,juce::PathStrokeType((.07f+.07f*random(key+193))*(.35f+.65f*taper),juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                }
            }
        }
        for(int stage=0;stage<5;++stage)for(int surface=0;surface<2;++surface) {
            auto& image=(surface?keyTab:keyRim)[size_t(stage)];
            const float width=surface?14.f:juce::MathConstants<float>::twoPi*60.f;
            image=juce::Image(juce::Image::ARGB,int(std::ceil(width*4)),54,true,juce::SoftwareImageType());
            juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale(4.f));
            const unsigned seed=surface?0x781c31u:0xa63117u;
            for(unsigned scratch=0;scratch<unsigned((surface?64:176)*stage/4);++scratch) {
                const float x=width*(.06f+.88f*random(seed+scratch*127));
                const float y=.6f+9.3f*random(seed+scratch*419);
                const float dx=(random(seed+scratch*199)-.5f)*(surface?6.f:16.f);
                const float dy=.5f+2.8f*random(seed+scratch*733);
                juce::Path trace;trace.startNewSubPath(x,y);
                trace.quadraticTo(x+dx*.2f+.4f,y+dy*.5f,x+dx,y+dy);
                g.setColour(juce::Colours::white.withAlpha(.12f+.12f*random(seed+scratch*37)));
                g.strokePath(trace,juce::PathStrokeType(.11f+.08f*random(seed+scratch*59),juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
            }
        }
    }
};
inline const BenderScuffMaterial& benderScuffs() {static const BenderScuffMaterial material;return material;}
inline float sampleBenderScuff(const juce::Image::BitmapData& pixels,float u,float v) {
    const float x=juce::jlimit(0.f,float(pixels.width-1),u*pixels.width),y=juce::jlimit(0.f,float(pixels.height-1),v*pixels.height);
    const int x0=int(x),y0=int(y),x1=juce::jmin(x0+1,pixels.width-1),y1=juce::jmin(y0+1,pixels.height-1);
    const float fx=x-x0,fy=y-y0;
    const float a=pixels.getPixelColour(x0,y0).getFloatAlpha(),b=pixels.getPixelColour(x1,y0).getFloatAlpha();
    const float c=pixels.getPixelColour(x0,y1).getFloatAlpha(),d=pixels.getPixelColour(x1,y1).getFloatAlpha();
    return (a+(b-a)*fx)*(1-fy)+(c+(d-c)*fx)*fy;
}
// The finger tab is one rounded moulding, including the two axial ends.
// Shade its continuous surface instead of rounding each mesh facet separately.
inline void paintRoundedBenderTab(juce::Graphics& g,float centre,float baseline,
                                  float angle,float age,int samples,float depth=13.5f,bool proportional=false,bool keyScuffs=false,bool gentleAgeFinish=false,bool castShadow=false) {
    constexpr float viewSin=.190809f,viewCos=.981627f;
    constexpr float lower=58.3f,upper=87.f;
    const float halfWidth=proportional?depth*.5f:7.f;
    // Wider roll at the two sides, tighter rounds at the axial/top-bottom edges.
    // Elliptical corner sections keep a curved normal rather than a flat chamfer.
    const float roundX=5.2f*halfWidth/7.f;
    constexpr float roundA=3.6f,roundZ=6.0f;
    const float sa=std::sin(angle),ca=std::cos(angle),zc=(lower+upper)*.5f;
    const auto project=[&](float x,float z,float axle) {
        return juce::Point<float>(centre+x*ca+z*sa,baseline-(z*ca-x*sa)*viewSin+axle*viewCos);
    };
    juce::Rectangle<float> bounds;
    bool first=true;
    for(float x:{-halfWidth,halfWidth})for(float z:{lower,upper})for(float a:{0.f,depth}) {
        const auto p=project(x,z,a);
        if(first){bounds={p.x,p.y,.001f,.001f};first=false;}else bounds=bounds.getUnion({p.x,p.y,.001f,.001f});
    }
    bounds=bounds.expanded(1.f).getSmallestIntegerContainer().toFloat();
    juce::Image image(juce::Image::ARGB,int(bounds.getWidth()*samples),int(bounds.getHeight()*samples),true,juce::SoftwareImageType());
    juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
    const auto& scuffs=benderScuffs();
    const juce::Image::BitmapData scuffPixels((keyScuffs?scuffs.keyTab:scuffs.tab)[size_t(juce::jlimit(0,4,juce::roundToInt(age*4)))],juce::Image::BitmapData::readOnly);
    const auto distance=[&](float x,float a,float z) {
        const float qx=(std::abs(x)-(halfWidth-roundX))/roundX;
        const float qa=(std::abs(a-depth*.5f)-(depth*.5f-roundA))/roundA;
        // A shallow crown across the front avoids a flat central plaque with
        // a bright rectangular bevel around it. Its normals roll continuously.
        const float across=juce::jlimit(-1.f,1.f,x/halfWidth);
        const float crown=1.8f*across*across;
        const float qz=(std::abs(z-zc+crown)-((upper-lower)*.5f-roundZ))/roundZ;
        const float dx=juce::jmax(0.f,qx),da=juce::jmax(0.f,qa),dz=juce::jmax(0.f,qz);
        return (std::sqrt(dx*dx+da*da+dz*dz)+juce::jmin(juce::jmax(qx,qa,qz),0.f)-1.f)*.85f;
    };
    // Orthographic viewing ray, transformed into the tab's rigid local frame.
    const float vx=sa*viewCos,va=-viewSin,vz=-ca*viewCos;
    for(int y=0;y<image.getHeight();++y)for(int x=0;x<image.getWidth();++x) {
        const float px=bounds.getX()+(x+.5f)/samples-centre;
        const float py=bounds.getY()+(y+.5f)/samples;
        float a=depth+4.f,rotatedZ=(baseline+a*viewCos-py)/viewSin;
        float lx=px*ca-rotatedZ*sa,lz=rotatedZ*ca+px*sa,travel=0.f;
        bool hit=false;
        for(int step=0;step<96&&travel<160.f;++step) {
            const float d=distance(lx,a,lz);
            if(d<.018f){hit=true;break;}
            const float advance=juce::jmax(d,.018f);
            lx+=vx*advance;a+=va*advance;lz+=vz*advance;travel+=advance;
        }
        if(!hit)continue;
        constexpr float e=.035f;
        float nx=distance(lx+e,a,lz)-distance(lx-e,a,lz);
        float ny=distance(lx,a+e,lz)-distance(lx,a-e,lz);
        float nz=distance(lx,a,lz+e)-distance(lx,a,lz-e);
        const float length=std::sqrt(nx*nx+ny*ny+nz*nz)+1.e-8f;
        nx/=length;ny/=length;nz/=length;
        const float worldX=nx*ca+nz*sa,worldZ=nz*ca-nx*sa;
        const float diffuse=juce::jmax(0.f,-.27f*worldX+.09f*ny+.959f*worldZ);
        const float reflection=polishedBenderHighlight(-.137f*worldX+.125f*ny+.982f*worldZ,age);
        const float edgeLight=std::pow(juce::jmax(0.f,-.46f*worldX-.24f*ny+.855f*worldZ),24.f);
        const float gloss=gentleAgeFinish?1.f-.18f*age:1.f;
        float value=juce::jlimit(0.f,1.f,.066f+.089f*diffuse+(.95f*reflection+.12f*edgeLight)*(1-.45f*age)*gloss);
        if(age>0) {
            const float scratch=sampleBenderScuff(scuffPixels,(lx+halfWidth)/(2*halfWidth),a/depth)*juce::jlimit(0.f,1.f,nz*2.f);
            value=value*(1-.75f*scratch)+(.18f+.25f*diffuse)*scratch;
        }
        pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(value,value*.99f,value*.96f,1.f));
    }
    if(castShadow) {
        juce::Graphics::ScopedSaveState shadow(g);
        g.addTransform(juce::AffineTransform::scale(1.f/samples).translated(bounds.getX(),bounds.getY()));
        juce::DropShadow(juce::Colours::black.withAlpha(.25f),juce::jmax(1,juce::roundToInt(1.5f*samples)),{juce::roundToInt(1.2f*samples),juce::roundToInt(1.8f*samples)}).drawForImage(g,image);
    }
    g.setOpacity(1);g.drawImage(image,bounds);
}
// Rotational symmetry permits one cached raised rib per condition/resolution.
// Moving the bender only translates this image; no ray tracing occurs per frame.
inline void paintBenderUndersideRing(juce::Graphics& g,float centre,float baseline,float age,int samples) {
    constexpr float viewSin=.190809f,viewCos=.981627f,major=48.f,minor=1.9f,axle=10.8f;
    samples=juce::jlimit(1,4,samples);
    const int stage=juce::jlimit(0,4,juce::roundToInt(age*4));
    static std::array<std::array<juce::Image,4>,5> cache;
    auto& image=cache[size_t(stage)][size_t(samples-1)];
    const juce::Rectangle<float> bounds(-52.f,-4.f,104.f,28.f);
    if(image.isNull()) {
        image=juce::Image(juce::Image::ARGB,104*samples,28*samples,true,juce::SoftwareImageType());
        juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
        for(int y=0;y<image.getHeight();++y)for(int x=0;x<image.getWidth();++x) {
            const float px=bounds.getX()+(x+.5f)/samples,py=bounds.getY()+(y+.5f)/samples;
            float a=18.f,z=(a*viewCos-py)/viewSin,travel=0.f;
            bool hit=false;
            for(int step=0;step<84&&travel<190.f;++step) {
                const float radial=std::hypot(px,z)-major;
                const float d=std::hypot(radial,a-axle)-minor;
                if(d<.012f){hit=true;break;}
                const float advance=juce::jmax(.012f,d);
                z-=viewCos*advance;a-=viewSin*advance;travel+=advance;
            }
            if(!hit)continue;
            const float radius=std::hypot(px,z),radial=radius-major;
            const float length=std::hypot(radial,a-axle);
            const float nx=radial/length*px/radius,ny=(a-axle)/length,nz=radial/length*z/radius;
            const float diffuse=juce::jmax(0.f,-.27f*nx+.09f*ny+.959f*nz);
            const float shine=std::pow(juce::jmax(0.f,-.137f*nx+.125f*ny+.982f*nz),32.f);
            const float value=.055f+.075f*diffuse+.075f*juce::jmax(0.f,ny)+(.19f*shine+.35f*benderHardReflection(nx,ny,nz,age))*(1-.45f*age);
            pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(value,value*1.02f,value*.99f,1));
        }
    }
    g.setOpacity(1);g.drawImage(image,bounds.translated(centre,baseline));
}

// Follow the visible front lip with one constant normal inset. Deriving both
// edges from this outline avoids the changing gap of a separately projected
// torus, especially beside the central break.
inline void paintParallelBenderRib(juce::Graphics& g,float centre,float baseline,
                                   float radius,float depth,float age,int samples,float ribWidth=1.3f,bool noGlint=false) {
    constexpr float viewSin=.190809f,viewCos=.981627f,gap=2.4f;
    struct Entry {juce::Image image;float radius=0,depth=0,width=0;bool noGlint=false;};
    static std::array<std::array<Entry,4>,5> cache;
    samples=juce::jlimit(1,4,samples);
    const int stage=juce::jlimit(0,4,juce::roundToInt(age*4));
    auto& entry=cache[size_t(stage)][size_t(samples-1)];
    const juce::Rectangle<float> bounds(-radius-1,depth*viewCos-radius*viewSin-1,2*radius+2,radius*viewSin+2);
    if(entry.image.isNull()||entry.radius!=radius||entry.depth!=depth||entry.width!=ribWidth||entry.noGlint!=noGlint) {
        entry.radius=radius;entry.depth=depth;entry.width=ribWidth;entry.noGlint=noGlint;
        entry.image=juce::Image(juce::Image::ARGB,int(std::ceil(bounds.getWidth()*samples)),int(std::ceil(bounds.getHeight()*samples)),true,juce::SoftwareImageType());
        juce::Graphics material(entry.image);
        material.addTransform(juce::AffineTransform::translation(-bounds.getX(),-bounds.getY()).scaled(float(samples)));
        const auto point=[&](float theta,float inset) {
            const float s=std::sin(theta),c=std::cos(theta);
            float nx=s,ny=-c/viewSin;
            const float length=std::hypot(nx,ny);nx/=length;ny/=length;
            return juce::Point<float>(radius*s-nx*inset,depth*viewCos-radius*c*viewSin-ny*inset);
        };
        for(int segment=0;segment<256;++segment) {
            const float a=-juce::MathConstants<float>::halfPi+juce::MathConstants<float>::pi*float(segment)/256.f;
            const float b=-juce::MathConstants<float>::halfPi+juce::MathConstants<float>::pi*float(segment+1)/256.f;
            for(int band=0;band<12;++band) {
                const float t0=float(band)/12.f,t1=float(band+1)/12.f;
                const float cross=juce::MathConstants<float>::pi*(.5f-(t0+t1)*.5f);
                const float radial=std::sin(cross),ny=std::cos(cross),theta=(a+b)*.5f;
                const float nx=radial*std::sin(theta),nz=radial*std::cos(theta);
                const float diffuse=juce::jmax(0.f,-.27f*nx+.09f*ny+.959f*nz);
                const float shine=std::pow(juce::jmax(0.f,-.137f*nx+.125f*ny+.982f*nz),32.f);
                const float value=.055f+.075f*diffuse+.075f*ny+(noGlint?0.f:.19f*shine+.35f*benderHardReflection(nx,ny,nz,age))*(1-.45f*age);
                juce::Path face;face.startNewSubPath(point(a,gap+ribWidth*t0));
                face.lineTo(point(b,gap+ribWidth*t0));face.lineTo(point(b,gap+ribWidth*t1));face.lineTo(point(a,gap+ribWidth*t1));face.closeSubPath();
                material.setColour(juce::Colour::fromFloatRGBA(value,value*1.02f,value*.99f,1.f));
                material.fillPath(face);material.strokePath(face,juce::PathStrokeType(.04f));
            }
        }
    }
    g.setOpacity(1);g.drawImage(entry.image,bounds.translated(centre,baseline));
}

}
