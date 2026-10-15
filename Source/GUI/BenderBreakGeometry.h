#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>

namespace kb09_gui {
inline constexpr float benderPreviewDepth=20.5f;
inline constexpr float benderPreviewWebCentre=benderPreviewDepth*.5f-1.70f;
// Reconstruct the recessed bite visible in the owner's broken-bender close-up.
// Every tooth subtracts material from the circle; no shoulder stands proud.
inline std::array<juce::Point<float>,34> recessedBenderBreak() {
    constexpr std::array<float,20> x{{-20.8f,-18.6f,-17.3f,-15.1f,-13.7f,-11.8f,-9.1f,-7.8f,-5.4f,-3.1f,
                                     -.5f,2.6f,5.8f,8.7f,10.2f,12.7f,14.4f,16.8f,18.9f,20.8f}};
    constexpr std::array<float,20> removed{{.05f,.4f,1.1f,2.7f,2.1f,6.3f,8.5f,8.9f,8.6f,9.6f,
                                           9.3f,9.9f,9.4f,8.8f,6.1f,4.f,3.1f,1.2f,.45f,.05f}};
    std::array<juce::Point<float>,20> base{};
    for(size_t i=0;i<base.size();++i) {
        const float recess=2.f*juce::jlimit(0.f,1.f,(12.f-std::abs(x[i]))/3.f);
        base[i]={x[i],std::sqrt(3600.f-x[i]*x[i])-removed[i]-recess};
    }
    std::array<juce::Point<float>,34> points{};size_t n=0;
    for(size_t i=0;i<base.size();++i) {
        points[n++]=base[i];
        if(i+1<base.size()&&std::abs(base[i].x)<10.f&&std::abs(base[i+1].x)<10.f) {
            const auto span=base[i+1]-base[i];
            const float phase=float(i)*2.137f;
            // Unequal chips along the one inner edge, rather than a straight
            // bridge or a repeating row of teeth. All chips cut into the web.
            auto a=base[i]+span*(.28f+.08f*std::sin(phase));
            auto b=base[i]+span*(.66f+.07f*std::cos(phase));
            a.y-=.75f+.55f*(.5f+.5f*std::sin(phase+1.f));
            b.y-=.10f+.25f*(.5f+.5f*std::cos(phase+2.f));
            points[n++]=a;points[n++]=b;
        }
    }
    return points;
}
inline juce::Point<float> withinBenderDisc(juce::Point<float> p,float round) {
    const float r=p.getDistanceFromOrigin(),limit=60.f-round;
    return r>limit?p*(limit/r):p;
}
inline float benderWebThickness(float x,float depth=benderPreviewDepth) {
    const float chip=juce::jmax(0.f,std::sin(x*1.89f+.3f)*std::sin(x*.71f-1.1f));
    return depth*.25f-.70f*chip*chip;
}
inline float benderDiscDepth(juce::Point<float> p,float depth=benderPreviewDepth) {
    const float t=juce::jlimit(0.f,1.f,(p.getDistanceFromOrigin()-51.f)/5.f);
    // The exposed central web is a thin sheet, not the full-depth outer rim.
    const float web=benderWebThickness(p.x,depth);
    return web+(depth-web)*t*t*(3.f-2.f*t);
}
inline float benderDiscRear(juce::Point<float> p,float depth=benderPreviewDepth) {
    const float t=juce::jlimit(0.f,1.f,(p.getDistanceFromOrigin()-51.f)/5.f);
    // Position the thin web within the rim instead of on its rear/top edge.
    // Its projected center meets the visible seam at the broken shoulders.
    return (depth*.5f-1.70f-benderWebThickness(p.x,depth)*.5f)*(1.f-t*t*(3.f-2.f*t));
}
inline float benderDiscAxle(juce::Point<float> p,float t,float depth=benderPreviewDepth) {
    return benderDiscRear(p,depth)+benderDiscDepth(p,depth)*t;
}
inline float benderBrokenShoulder(bool left,float t) {
    constexpr std::array<float,9> depths{{0.f,.12f,.24f,.38f,.50f,.61f,.74f,.88f,1.f}};
    constexpr std::array<float,9> lhs{{15.2f,14.3f,16.5f,15.7f,18.9f,18.1f,22.4f,23.1f,25.f}};
    constexpr std::array<float,9> rhs{{14.3f,15.8f,14.9f,18.5f,17.7f,20.6f,21.4f,24.9f,26.f}};
    const auto& values=left?lhs:rhs;t=juce::jlimit(0.f,1.f,t);
    for(size_t i=1;i<depths.size();++i)if(t<=depths[i]) {
        const float f=(t-depths[i-1])/(depths[i]-depths[i-1]);
        return values[i-1]+f*(values[i]-values[i-1]);
    }
    return values.back();
}
inline juce::Point<float> benderBrokenArc(int i,int segments,float t,float round) {
    const float right=std::asin(benderBrokenShoulder(false,t)/60.f);
    const float left=std::asin(benderBrokenShoulder(true,t)/60.f);
    const float angle=right+(juce::MathConstants<float>::twoPi-left-right)*float(i)/float(segments);
    return {std::sin(angle)*(60.f-round),std::cos(angle)*(60.f-round)};
}
inline juce::Point<float> benderFractureLocal(juce::Point<float> p,int vertex,float t,float round,float depth=benderPreviewDepth) {
    const float axialFraction=benderDiscAxle(p,t,depth)/depth;
    const float originalX=p.x;
    const float cut=std::sqrt(juce::jmax(0.f,3600.f-p.x*p.x))-p.y;
    if(std::abs(p.x)>10.f) {
        const float shoulder=benderBrokenShoulder(p.x<0,axialFraction);
        p.x=std::copysign(10.f+(std::abs(p.x)-10.f)*(shoulder-10.f)/11.f,p.x);
        p.y=std::sqrt(juce::jmax(0.f,3600.f-p.x*p.x))-cut;
    }
    const float taper=juce::jlimit(0.f,1.f,(21.f-std::abs(originalX))/5.f);
    const float phase=float(vertex)*2.137f;
    // Fracture relief changes through the depth. It must not extrude a single
    // saw-tooth profile into continuous full-height flutes.
    const float envelope=std::sin(juce::MathConstants<float>::pi*t);
    p.x+=taper*(.35f*std::sin(phase)*t+.65f*envelope*std::sin(phase+9.1f*t));
    p.y+=taper*(.65f*std::cos(phase+1.f)*t+.85f*envelope*std::cos(phase+11.3f*t));
    return withinBenderDisc(p,round);
}
// Trim the broken shoulders against the same projected front contour as the
// intact rim. Radial cutbacks must not become extra forward-hanging material.
inline juce::Point<float> constrainBenderFractureToRim(juce::Point<float> p,float axle,float depth,float angle,float bevelInset=0.f) {
    constexpr float viewSin=.190809f,viewCos=.981627f;
    const float radius=58.85f+bevelInset,frontAxle=depth-bevelInset;
    const float s=std::sin(angle),c=std::cos(angle);
    const float x=p.x*c+p.y*s,z=p.y*c-p.x*s;
    if(std::abs(x)>=radius)return p;
    const float minimumZ=std::sqrt(radius*radius-x*x)-(frontAxle-axle)*viewCos/viewSin;
    const float boundedZ=juce::jmax(z,minimumZ);
    return {x*c-boundedZ*s,boundedZ*c+x*s};
}
}
