#pragma once
#include <algorithm>
#include <cmath>

namespace kb09_gui {
// Once running, the lamp follows elapsed time and RATE alone. Waveform edits
// and fresh audio-display snapshots cannot re-anchor its visual phase.
class RateLampClock {
public:
    float level(bool power,double hz,double nowMs,double initialPhase=0) {
        if(!power||hz<=0) {running=false;return 0;}
        if(!running) {phase=initialPhase;running=true;}
        else phase+=std::max(0.,nowMs-lastMs)*.001*frequency;
        phase-=std::floor(phase);lastMs=nowMs;frequency=hz;
        const float strobe=phase<.5?1.f:0.f;
        const float blend=float(std::clamp((hz-12)/4,0.,1.));
        const float averaged=.48f+.52f*float(std::clamp((hz-16)/9,0.,1.));
        return strobe+(averaged-strobe)*blend;
    }
private:
    double phase=0,lastMs=0,frequency=0;bool running=false;
};
}
