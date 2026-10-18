# Slider response review — October 18, 2026

Status: user authorized the four squared curves and Linux JACK on October 18, 2026. CurvesJack283 implements physical GUI/MIDI tapering; Engine.cpp and stored/automated effect parameter values remain unchanged. Approved282 remains the previous appearance-only baseline. The updated TS is an audition, not a public release.

## Findings from the current source

The stored continuous parameters use linear NormalisableRange values. The DSP then maps those values separately for each control; a single blanket audio taper would double-curve controls that are already curved.

| Control | Current mapping | First tick (10% travel), Authentic/Minty |
| --- | --- | --- |
| PORTAMENTO | 5 times parameter seconds, for 99.9% pitch-error settling | 500 ms |
| VCO MOD | 12 times parameter semitones peak excursion; 36 in Modern | 1.2 semitones |
| VCF MOD | 5 times parameter octaves peak excursion; 8 in Modern | 0.5 octaves |
| VCF ENV / FOL'R depth | 8 times parameter octaves at full envelope/follower | 0.8 octaves |
| LFO DELAY | 1.5 times parameter seconds, followed by its fade-in | 150 ms |
| CUTOFF | Exponential 10 Hz–20 kHz before tracking/modulation | Already curved |
| LFO RATE | Exponential 0.2–25 Hz; Modern extends the low end to 0.05 Hz | Already curved |
| ATTACK / DECAY / RELEASE | Exponential time maps, with existing age adjustment | Already curved |
| VOLUME | Cubic amplitude map | Already curved |

Sources: Source/Parameters/Parameters.h, Source/DSP/Engine.cpp, Source/DSP/LfoRate.h, Source/Plugin/Processor.cpp, Source/GUI/Editor.cpp. No claim is made that the current or proposed tapers are measured SH-09 potentiometer laws.

## Recommended first audition

Use a squared physical-position curve, effect = position squared, on PORTAMENTO, VCO MOD, VCF MOD and VCF ENV/FOL'R depth. At 10% travel it produces 1% of the existing maximum; at 50% it produces 25%; full travel still produces 100%. This gives the first tick approximately 50 ms glide, 0.12 semitones VCO modulation, 0.05 octaves VCF modulation and 0.08 octaves ENV/FOL'R depth in Authentic mode. These are calculated mapping examples, not listening-test results.

Keep cutoff, rate, envelope time and volume mappings unchanged for the first audition. Do not change all faders just because their parameters are stored linearly. Keep current maxima and Authentic/Modern differences.

## Compatibility requirements for an implementation

Prefer remapping physical control position rather than altering the sound of stored parameter values: GUI position maps to the existing effect value; inverse mapping places caps and readouts. Existing presets and recorded DAW automation retain their sound and parameter IDs/ranges. Cap positions for those existing sounds will change as expected under the new physical taper.

MIDI-learn faders and the default CC1 modulation wheel use the same forward mapping in CurvesJack283. Recorded MIDI CC performances therefore adopt the new response; compatibility preservation applies to stored parameter amounts and DAW parameter automation, not the old CC-to-amount curve. Soft takeover must compare the hardware position with the inverse-mapped current value; preserve existing pickup behavior and VST3 timed CC1 handling. Update displayed tick-position values and typed-value conversion consistently. Do not square the modulation wheel only after pickup while comparing pickup against uncurved hardware values.

Verify endpoints, monotonicity, forward/inverse mapping, fine adjustment, cap/readout alignment, host automation, preset sound preservation, MIDI mapping, mod-wheel pickup and parameter smoothing. Provide an audition before replacing the approved sound behavior. Refresh manual screenshots after the final response/layout decision.

## Audio backend review

Windows currently enables ASIO and JUCE's default WASAPI/DirectSound backends. The actual startup probe enumerates Windows Audio shared, exclusive and low-latency modes, DirectSound and ASIO. macOS uses Core Audio. Linux enables ALSA but currently leaves JUCE_JACK at JUCE's default 0, even though the CI installs JACK development headers. Enabling and testing JACK, including PipeWire's JACK compatibility layer, is the useful next Linux compatibility proposal. It is not implemented in Approved282; CurvesJack283 enables it for Linux and adds native backend-factory checks. Linux physical desktop audio/MIDI validation remains unverified. Plug-ins use their DAW's audio device/backend.

Primary references:
- https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/low-latency-audio
- https://developer.apple.com/documentation/CoreAudio
- https://docs.juce.com/master/classjuce_1_1AudioIODeviceType.html
- https://github.com/PipeWire/pipewire/blob/master/README.md
