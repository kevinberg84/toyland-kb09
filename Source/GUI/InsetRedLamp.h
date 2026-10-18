#pragma once
#include <juce_graphics/juce_graphics.h>
#include <cmath>

namespace kb09_gui {
inline constexpr float insetRedLampDiameter=9.f;
inline void drawRedLampLens(juce::Graphics& g,juce::Point<float> centre,float level) {
    const auto lens=juce::Rectangle<float>(6.2f,6.2f).withCentre(centre);
    g.setColour(juce::Colour(0xff491c16).interpolatedWith(juce::Colour(0xffe54c35),level));g.fillEllipse(lens);
    // The lens catches a small reflection even when no current lights it.
    g.setColour(juce::Colour(0xffffc493).withAlpha(.12f+level*.53f));
    g.fillEllipse(juce::Rectangle<float>(1.6f,1.6f).withCentre(centre+juce::Point<float>(-.65f,-.75f)));
}

inline void drawInsetRedLamp(juce::Graphics& g,juce::Point<float> centre,float level,bool brightRateGlimmer=false,bool illuminateWell=true) {
    juce::Graphics::ScopedSaveState save(g);
    level=juce::jlimit(0.f,1.f,level);
    const auto well=juce::Rectangle<float>(insetRedLampDiameter,insetRedLampDiameter).withCentre(centre);
    // One dark well with a narrow panel reflection outside its lower-right edge.
    g.setColour(juce::Colour(0xff060a09));g.fillEllipse(well);
    // Illuminate the full recessed wall, clipped to the existing circular
    // opening. Keep its outside panel reflection independent of LED light.
    if(illuminateWell&&level>0.f) {
        const juce::Colour red(0xffdc3b20);
        juce::ColourGradient spill(red.withAlpha(.85f*level),centre.x,centre.y,
            red.withAlpha(.55f*level),centre.x+insetRedLampDiameter*.5f,centre.y,true);
        spill.addColour(.55,red.withAlpha(.80f*level));
        spill.addColour(.85,red.withAlpha(.65f*level));
        g.setGradientFill(spill);g.fillEllipse(well);
    }
    // The narrow glimmer sits just outside the opening, at bottom right.
    for(int i=0;i<32;++i) {
        const float a=juce::MathConstants<float>::halfPi*i/32.f;
        const float b=juce::MathConstants<float>::halfPi*(i+1)/32.f;
        const float strength=std::pow(juce::jmax(0.f,std::sin(a+b)),brightRateGlimmer?2.5f:4.f);
        const auto p=centre+juce::Point<float>(std::cos(a),std::sin(a))*4.78f;
        const auto q=centre+juce::Point<float>(std::cos(b),std::sin(b))*4.78f;
        g.setColour(juce::Colour(0xffc3c9be).withAlpha((brightRateGlimmer?.94f:.64f)*strength));
        g.drawLine({p,q},.36f);
    }
    drawRedLampLens(g,centre,level);
}
}

