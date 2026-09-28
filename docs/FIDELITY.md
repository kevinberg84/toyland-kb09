# Fidelity evidence classification

## Verified from primary SH-09 documentation

The owner/service documents establish the monophonic control layout and routing, low-note selection, selectable noise/saw/square/pulse, divider sub modes, LFO/PWM/envelope routing, external mixer input and follower-to-VCF behavior, VCA modes, and published endpoint ranges. The service schematic identifies four selected BA662 filter stages, a BA662 VCA and the VCO/sub/noise circuits. These facts guide the implementation; a correct schematic reading alone does not certify its digital approximation. See SH09_RESEARCH.md for page-level evidence and sources.

## Supported by other evidence

Real-instrument photographs support the panel section order, materials and proportions. Published virtual-analog research supports trapezoidal integration, bounded nonlinear feedback solving and band-limiting choices. Official JUCE/VST3/Ableton documentation supports the host adapter; the compiled-module and Live tests now independently demonstrate the particular implemented route. Contemporary instrument documentation informs optional workflow features. None of these substitutes for measuring a physical SH-09.

## Engineering inferences

Normalized nonlinear OTA stages, saturation and resonance limiting, filter/follower equations, control tapers between published endpoints, source gain staging, anti-alias filters, output coupling, smoothing times, pitch-bend depth maps and the deterministic Condition profile are explicit implementation choices. Modern velocity, aftertouch, tempo sync and input trim are extensions. The panel is an original vector interpretation.

## Unverified without physical SH-09 measurements

Waveform curvature, PWM transitions, noise spectrum, exact stage drive, cutoff/resonance interaction and self-oscillation level, input sensitivity and volts-to-dBFS mapping, active follower timing, envelope/control-law curves, VCA/output coloration and real-unit aging/tolerance distributions have not been fitted to hardware captures. No measured error bounds, blind listening result, completed listening A/B or component-level SPICE equivalence is claimed. The calibration tools and generated stimuli prepare for that work.
