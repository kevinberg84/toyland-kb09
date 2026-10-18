#pragma once
#include <juce_graphics/juce_graphics.h>
#include <cmath>
namespace kb09_gui {
struct KnobPlasticLighting {
    static constexpr float innerRadius=.280f,outerRadius=.326f;
    // Reduce the metal floor; the outer grip and vertical inner wall stay square.
    static constexpr float thickerInnerRadius=innerRadius*.98f;
    static float facet(float nx,float ny,float azimuth,float relief,float age,bool extraFill=false,bool broad165=false,bool darkerCrests=false) {
        // World-space source and reflection direction: geometry moves through
        // them. Rib height contributes only modest ambient occlusion, never
        // a painted highlight carried around by a crest.
        const float diffuse=juce::jmax(0.f,-.70710678f*nx-.70710678f*ny);
        const float half=juce::jmax(0.f,-.72f*nx+.694f*ny);
        const float envelope=.35f+.65f*std::pow(juce::jmax(0.f,-.72f*std::cos(azimuth)+.694f*std::sin(azimuth)),3.f);
        const float gloss=(1-age*.40f)*(.10f*std::pow(half,88.f)+1.18f*std::pow(half,32.f))*envelope*std::pow(relief,3.8f);
        // The underside has no room-light sheen or broad fill. Keep only
        // its base material response and the existing narrow rib reflections.
        juce::ignoreUnused(extraFill,broad165);
        const float crest=juce::jlimit(0.f,1.f,(relief-.20f)/.65f);
        const float reflectionGain=darkerCrests?1.f-.55f*crest:1.f;
        return juce::jlimit(0.f,.98f,(.085f+.20f*diffuse+gloss*reflectionGain)*(.82f+.18f*relief));
    }
    static float collarTickGlint(float angle,float age) {
        // Only the exposed upper-left collar sector catches the light.
        // No secondary lobe: the raised grip occludes the lower sector.
        return std::pow(juce::jmax(0.f,std::cos(angle+juce::MathConstants<float>::pi*.25f)),16.f)*(1-age*.82f);
    }
    static float tickGlint(float angle,float age,bool prominent=false) {
        // Pointer angles are clockwise from twelve o'clock. Upper left=-45°.
        const float upperLeft=std::pow(juce::jmax(0.f,std::cos(angle+juce::MathConstants<float>::pi*.25f)),prominent?14.f:24.f);
        const float lowerRight=(prominent?.72f:.62f)*std::pow(juce::jmax(0.f,std::cos(angle-juce::MathConstants<float>::pi*.75f)),prominent?17.f:28.f);
        return juce::jmax(upperLeft,lowerRight)*(1-age*(prominent?.82f:.375f));
    }
};
}
