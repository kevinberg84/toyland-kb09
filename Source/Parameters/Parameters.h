#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
namespace sh09 {
struct ParameterSpec { const char* id; const char* name; float min,max,initial; const char* choices; };
inline constexpr auto specs=std::to_array<ParameterSpec>({
 {"tune","Tune (cents)",-65,65,0,""}, {"range","VCO Range",0,4,2,"32 ft|16 ft|8 ft|4 ft|2 ft"},
 {"wave","VCO Wave",0,3,1,"Noise|Saw|Square|Pulse"}, {"pulseWidth","Pulse Width",0,1,0,""},
 {"pwmMode","Pulse Width Mode",0,2,0,"Manual|LFO|Envelope"}, {"vcoLevel","VCO Level",0,1,.8f,""},
 {"subLevel","Sub Level",0,1,0,""}, {"extLevel","External Level",0,1,0,""},
 {"subMode","Sub Mode",0,2,0,"-1 octave|-2 octaves|-2 pulse"},
 {"lfoRate","LFO Rate",0,1,.3f,""}, {"lfoDelay","LFO Delay",0,1,0,""},
 {"lfoWave","LFO Wave",0,2,0,"Sine|Square|Random"}, {"vcoMod","VCO Modulation",0,1,0,""},
 {"cutoff","VCF Cutoff",0,1,.7f,""}, {"resonance","VCF Resonance",0,1,0,""},
 {"vcfMod","VCF LFO Modulation",0,1,0,""}, {"vcfEnv","VCF Envelope Depth",0,1,.4f,""},
 {"vcfEnvMode","VCF Envelope Source",0,2,0,"Positive|Negative|Follower"},
 {"vcaMode","VCA Mode",0,2,1,"Hold|Envelope|Gate"},
 {"attack","Attack",0,1,.01f,""}, {"decay","Decay",0,1,.3f,""},
 {"sustain","Sustain",0,1,.7f,""}, {"release","Release",0,1,.2f,""},
 {"envMode","Envelope Trigger",0,2,0,"Gate + Trigger|Gate|LFO"},
 {"portamento","Portamento",0,1,0,""}, {"bendVco","Bender VCO Depth",0,1,.166667f,""},
 {"bendVcf","Bender VCF Depth",0,1,0,""}, {"volume","Volume",0,1,.7f,""},
 {"age","Condition / Age",0,1,0,""}, {"modern","Modern Mode",0,1,0,"Authentic|Modern"},
 {"velocity","Modern Velocity",0,1,0,""}, {"aftertouch","Modern Aftertouch",0,1,0,""},
 {"lfoSync","Modern LFO Sync",0,1,0,"Free|Tempo"},
 {"extGain","Modern Input Trim",0,1,.5f,""}, {"branding","Panel Branding",0,1,0,"Roland SH-09|Private Mono 09"}
});
inline juce::AudioProcessorValueTreeState::ParameterLayout makeParameters(){
 juce::AudioProcessorValueTreeState::ParameterLayout layout;
 for(const auto& p:specs){
   if(p.choices[0]!=0)layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{p.id,1},p.name,juce::StringArray::fromTokens(p.choices,"|",""),int(p.initial)));
   else layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{p.id,1},p.name,juce::NormalisableRange<float>{p.min,p.max},p.initial));
 }
 return layout;
}
}
