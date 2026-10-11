#pragma once
#include <array>
#include <vector>
#include <utility>
namespace sh09 {
struct Preset { const char* name; std::vector<std::pair<const char*,float>> values; };
// Append new sounds: saved states and hosts retain the original program indices.
inline const std::array<Preset,24>& factoryPresets(){
 static const std::array<Preset,24> bank{{
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
 {"16 / Modern Clockwork",{{"modern",1.0f},{"lfoSync",1.0f},{"lfoRate",.55f},{"lfoWave",1.0f},{"envMode",2.0f},{"decay",.25f},{"sustain",0.0f},{"cutoff",.5f},{"vcfEnv",.55f}}},
 {"17 / Iron Foundation",{{"range",1.0f},{"vcoLevel",.85f},{"subLevel",.75f},{"cutoff",.4f},{"resonance",.18f},{"vcfEnv",.35f},{"attack",0.0f},{"decay",.56f},{"sustain",.55f},{"release",.12f},{"volume",.82f}}},
 {"18 / PWM Heavyweight",{{"age",.2f},{"range",1.0f},{"wave",3.0f},{"pwmMode",1.0f},{"pulseWidth",.35f},{"lfoRate",.18f},{"vcoLevel",.85f},{"subLevel",.65f},{"cutoff",.48f},{"resonance",.28f},{"vcfEnv",.22f},{"decay",.58f},{"sustain",.7f},{"release",.18f},{"volume",.8f}}},
 {"19 / Roadworn Growl",{{"age",.6f},{"range",1.0f},{"wave",2.0f},{"vcoLevel",.8f},{"subLevel",.7f},{"subMode",2.0f},{"cutoff",.37f},{"resonance",.65f},{"vcfEnv",.48f},{"decay",.58f},{"sustain",.42f},{"release",.13f},{"portamento",.045f},{"volume",.82f}}},
 {"20 / Resonant Rubber",{{"age",.4f},{"range",1.0f},{"vcoLevel",.7f},{"subLevel",.45f},{"cutoff",.28f},{"resonance",.83f},{"vcfEnv",.65f},{"decay",.6f},{"sustain",.3f},{"release",.12f},{"portamento",.07f},{"volume",.82f}}},
 {"21 / Pure Feedback",{{"vcoLevel",0.0f},{"subLevel",0.0f},{"cutoff",.43f},{"resonance",1.0f},{"vcfEnv",0.0f},{"attack",.02f},{"decay",.25f},{"sustain",.85f},{"release",.3f},{"portamento",.055f},{"bendVco",0.0f},{"bendVcf",.166667f},{"volume",.75f}}},
 {"22 / Hot Feedback",{{"age",.8f},{"vcoLevel",0.0f},{"subLevel",0.0f},{"cutoff",.52f},{"resonance",1.0f},{"vcfEnv",.15f},{"attack",.01f},{"decay",.48f},{"sustain",.65f},{"release",.25f},{"lfoRate",.22f},{"vcfMod",.035f},{"bendVco",0.0f},{"bendVcf",.166667f},{"volume",.7f}}},
 {"23 / Sold As-Is Transmission",{{"age",1.0f},{"range",2.0f},{"wave",3.0f},{"pulseWidth",.42f},{"vcoLevel",.65f},{"subLevel",.35f},{"lfoWave",2.0f},{"lfoRate",.52f},{"vcfMod",.32f},{"cutoff",.42f},{"resonance",.77f},{"vcfEnv",.2f},{"decay",.4f},{"sustain",.55f},{"release",.22f},{"volume",.72f}}},
 {"24 / Aftertouch Brass",{{"age",.3f},{"modern",1.0f},{"velocity",.65f},{"aftertouch",.65f},{"wave",3.0f},{"pulseWidth",.32f},{"pwmMode",2.0f},{"vcoLevel",.85f},{"subLevel",.25f},{"cutoff",.48f},{"resonance",.25f},{"vcfEnv",.4f},{"attack",.16f},{"decay",.4f},{"sustain",.65f},{"release",.28f},{"portamento",.035f},{"volume",.82f}}}
 }};
 return bank;
}
}
