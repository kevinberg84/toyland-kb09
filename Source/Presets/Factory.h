#pragma once
#include <array>
#include <vector>
#include <utility>
namespace sh09 {
struct Preset { const char* name; std::vector<std::pair<const char*,float>> values; };
inline const std::array<Preset,16>& factoryPresets(){
 static const std::array<Preset,16> bank{{
 {"01 / Calibrated Init",{}},
 {"02 / Fundamental Bass",{{"range",1.0f},{"cutoff",.45f},{"vcfEnv",.27f},{"decay",.46f},{"sustain",.15f},{"subLevel",.4f}}},
 {"03 / Hollow Pulse",{{"wave",3.0f},{"pulseWidth",.55f},{"subLevel",.2f},{"cutoff",.6f},{"vcfEnv",.15f}}},
 {"04 / Singing Lead",{{"portamento",.09f},{"lfoDelay",.35f},{"vcoMod",.025f},{"cutoff",.7f},{"resonance",.36f}}},
 {"05 / Resonant Steps",{{"cutoff",.34f},{"resonance",.8f},{"vcfEnv",.62f},{"decay",.4f},{"sustain",0.0f},{"release",.14f}}},
 {"06 / Soft PWM",{{"wave",3.0f},{"pwmMode",1.0f},{"pulseWidth",.55f},{"attack",.55f},{"release",.6f},{"cutoff",.6f},{"vcfEnv",.15f}}},
 {"07 / Low Drum",{{"range",0.0f},{"wave",2.0f},{"cutoff",.22f},{"resonance",.7f},{"vcfEnv",.4f},{"decay",.28f},{"sustain",0.0f},{"release",.1f}}},
 {"08 / Noise Snare",{{"wave",0.0f},{"cutoff",.72f},{"resonance",.35f},{"decay",.25f},{"sustain",0.0f},{"vcfEnv",.15f}}},
 {"09 / Random Circuit",{{"lfoWave",2.0f},{"lfoRate",.7f},{"vcfMod",.8f},{"cutoff",.5f},{"resonance",.65f},{"vcfEnv",0.0f},{"vcaMode",0.0f}}},
 {"10 / Filter Whistle",{{"vcoLevel",0.0f},{"cutoff",.48f},{"resonance",1.0f},{"vcfEnv",0.0f},{"sustain",1.0f}}},
 {"11 / Sub Drone",{{"vcoLevel",.15f},{"subLevel",.85f},{"subMode",2.0f},{"cutoff",.6f},{"vcaMode",0.0f},{"vcfEnv",0.0f}}},
 {"12 / External Filter",{{"vcoLevel",0.0f},{"extLevel",.8f},{"cutoff",.7f},{"resonance",.25f},{"vcfEnv",0.0f},{"vcaMode",0.0f}}},
 {"13 / External Follower",{{"vcoLevel",0.0f},{"extLevel",.8f},{"cutoff",.3f},{"resonance",.5f},{"vcfEnvMode",2.0f},{"vcfEnv",.8f},{"vcaMode",0.0f}}},
 {"14 / Played-in Bass",{{"age",.7f},{"range",1.0f},{"subLevel",.3f},{"cutoff",.45f},{"vcfEnv",.4f},{"sustain",.2f}}},
 {"15 / Modern Expression",{{"modern",1.0f},{"velocity",.8f},{"aftertouch",.6f},{"cutoff",.55f},{"lfoDelay",.4f}}},
 {"16 / Modern Clockwork",{{"modern",1.0f},{"lfoSync",1.0f},{"lfoRate",.55f},{"lfoWave",1.0f},{"envMode",2.0f},{"decay",.25f},{"sustain",0.0f},{"cutoff",.5f},{"vcfEnv",.55f}}}
 }};
 return bank;
}
}
