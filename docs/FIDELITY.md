# Fidelity evidence classification

Current release 1.3.0 restores the preserved 1.1.3 four-times DSP core after listening feedback. The 1.2.0 coupled diode/OTA revision is superseded. The restored core is a circuit-informed nonlinear four-stage approximation, not a component-calibrated model. Modern FREE adds a 0.05 Hz minimum only within the bottom fifth of RATE travel; Authentic retains 0.2–25 Hz. Parameter IDs, state format and reported 128-sample latency are unchanged. Earlier evidence below is historical unless explicitly identified as current.


This describes **Toyland KB-09 0.11.0**. “Authentic” names the software's original-routing mode; it is not a claim that every control law or sound has been measured against hardware.

## Verified from primary SH-09 documentation

The owner/service documents establish the monophonic control layout and routing, low-note selection, selectable noise/saw/square/pulse, divider sub modes, LFO/PWM/envelope routing, external mixer input and follower-to-VCF behavior, VCA modes, and published endpoint ranges. The service schematic identifies four selected BA662 filter stages, a BA662 VCA and the VCO/sub/noise circuits. These facts guide the implementation; a correct schematic reading alone does not certify its digital approximation. See SH09_RESEARCH.md for page-level evidence and sources.

## Supported by other evidence

Real-instrument photographs support the panel section order, materials and proportions. Published virtual-analog research supports trapezoidal integration, bounded nonlinear feedback solving and band-limiting choices. Official JUCE/VST3/Ableton documentation supports the host adapter. Recorded compiled-module and Live tests demonstrate the implemented route only for the binaries and host sessions identified in VALIDATION.md and LIVE_TEST.md; they do not automatically validate a subsequent 0.11.0 build. Contemporary instrument documentation informs optional workflow features. None of these substitutes for measuring a physical SH-09.

## Engineering inferences

Normalized nonlinear OTA stages, saturation and resonance limiting, filter/follower equations, control tapers between published endpoints, source gain staging, anti-alias filters, output coupling, smoothing times, pitch-bend depth maps and the deterministic Condition profile are explicit implementation choices. The Authentic MOD maxima—±12 semitones for VCO pitch and ±5 octaves for VCF cutoff—are engineering choices, not measured hardware values or maxima established by the reviewed primary documents.

Modern velocity, aftertouch, tempo sync, input trim and the 0.11.0 extended MOD depths are software extensions. Modern VCO MOD reaches ±36 semitones (±3 octaves); Modern VCF MOD reaches ±8 octaves. Those spans are applied before the existing oscillator/filter frequency bounds. The original 35 parameter IDs/order and normalized values remain intact, but an old Modern patch with nonzero MOD intentionally gains depth; see DSP_DESIGN.md for the exact scaling and how to recover its previous depth. Authentic depth mappings are unchanged by this extension.

The current Toyland KB-09 panel combines embedded raster artwork and drawn interactive controls. It replaces the earlier vector-only interpretation; its visual treatment is separate from evidence of sonic fidelity.

## Bounded digital evidence for extended modulation

The standalone extended-depth core test log, `artifacts/ModDepth/DSPTests.txt`, records **92 passing assertions and zero failures**. It measures VCO endpoint frequencies and VCF transfer behavior, exercises maximum Modern depths at five sample rates with a 25 Hz LFO, and reports zero solver failures and zero tracked C++ allocation calls in the stress loops. These results establish the stated software behavior under the test conditions. They do not establish hardware equivalence, all-frequency alias limits or completion of the final 0.11.0 plug-in/host validation.

## Unverified without physical SH-09 measurements

Waveform curvature, PWM transitions, noise spectrum, exact stage drive, cutoff/resonance interaction and self-oscillation level, input sensitivity and volts-to-dBFS mapping, active follower timing, envelope/control-law curves, VCA/output coloration and real-unit aging/tolerance distributions have not been fitted to hardware captures. No measured error bounds, blind listening result, completed listening A/B or component-level SPICE equivalence is claimed. The calibration tools and generated stimuli prepare for that work.
