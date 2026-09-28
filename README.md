# SH-09 — private Windows VST3 instrument

A playable monophonic instrument based on the original Roland SH-09 manuals and service schematic, delivered as **0.9.0 release candidate**. The engine is a **circuit-informed approximation**, with explicit unmeasured parameters. It is not certified circuit-accurate or equivalent to a physical SH-09. See [known limitations](docs/KNOWN_LIMITATIONS.md), [fidelity evidence](docs/FIDELITY.md), [acceptance checklist](docs/ACCEPTANCE.md) and [validation](docs/VALIDATION.md).

## Load and play

The Windows x64 VST3 bundle is `artifacts/Release/SH-09.vst3`. Copy the entire bundle to a VST3 folder scanned by your DAW. With permission, the tested build was installed at `C:\Program Files\Common Files\VST3\SH-09.vst3` on this workstation. Alternatively, point Live's VST3 custom folder at the release directory and rescan. JUCE 9.0.2 and MSVC 19.51 build instructions are in [BUILD.md](docs/BUILD.md).

Load SH-09 on a MIDI track. Select **Calibrated Init** and play MIDI or the onscreen keyboard. The lowest held note wins. Higher held notes become audible when lower notes are released. Gate+Trigger re-attacks from the current envelope level when the effective note changes; Gate retains legato. The LFO retrigger option runs while keys are held. Oscillator and sub-divider phases run continuously.

The main wave selector includes noise, saw, square and variable pulse. Noise replaces the main waveform; the sub oscillator remains available. PWM can use manual width, the undelayed LFO sine, or ADSR. The VCF always tracks keyboard pitch and offers positive ADSR, negative ADSR or the external follower. VCA Hold sustains without MIDI, Envelope uses ADSR, and Gate follows held notes. Double-click controls to restore their initial values. Pitch bend has separate VCO and VCF depth controls.

## External audio

The single VST3 instrument exposes an **External Input auxiliary bus**. Route a DAW audio source to that bus. For audio processing without MIDI, choose **External Filter** or **External Follower** from the factory bank. These lower VCO/Sub and select VCA Hold. Raise EXT, then adjust cutoff and resonance. For follower filtering, select Follower in VCF and raise ENV depth. The detector listens before the EXT mixer slider, so turning EXT down can leave follower modulation active.

Stereo input is averaged to mono before the hardware-style path; opposite-polarity stereo channels cancel. Output is mono duplicated to stereo when a stereo bus is requested. The follower opens the filter; it does not itself trigger ADSR or open the VCA. See [architecture](docs/EXTERNAL_AUDIO_ARCHITECTURE.md) and the [observed Ableton Live test](docs/LIVE_TEST.md) for routing, saved-Set recall and remaining acceptance checks.

## Optional controls

The separate lower strip contains Authentic/Modern, Condition/Age, velocity response, aftertouch, LFO sync and input trim. Authentic ignores velocity, aftertouch, sync and trim. Age is explicit and independently adjustable; zero selects the healthy calibration profile. Age applies a bounded deterministic correlated calibration variation, modest scratches and key discoloration. It is not a literal simulation of elapsed years or an empirically measured aging curve.

Modern adds velocity-to-level, aftertouch-to-cutoff, a host-tempo LFO and +/-24 dB input trim. There is no polyphony, unison, sequencer, onboard effect, MIDI Learn or Gate/CV hardware interface. Branding can switch between the private Roland skin and neutral Private/Mono 09 assets without changing DSP or layout.

Sixteen original factory patches demonstrate bass, lead, PWM, resonance, percussion, noise, random modulation, self-oscillation, drones, external filtering, following, condition and modern expression. The DAW stores all parameters and the program selection. Factory selection replaces sound controls while preserving the visual branding preference.

## Quality and evidence

The full connected signal path runs at fixed 4x oversampling, with FIR input interpolation/output decimation and **128 samples of reported resampling latency**. Float samples are processed by an x64 plug-in with double-precision internal filter state. Continuous controls smooth over approximately 8 ms; host parameter automation follows JUCE's block processing, while MIDI note offsets are handled at their sample positions.

Primary-document routing, controls, divider relationships and published endpoint ranges are distinguished from inferred waveform shape, gain/saturation, filter/follower equations, control tapers and condition offsets. Research and evidence are in `docs/SH09_RESEARCH.md`, `DSP_DESIGN.md`, `DSP_DESIGN_DSP.md`, and the cited research documents. The functional scalable vector panel is an original reconstruction; it is not a photorealistic reproduction of the enclosure.

Tests load the actual VST3, exercise core DSP and processor behavior, and run pluginval plus Steinberg's validator. Results apply only to the recorded binary and tested conditions. `tools/calibrate.py` imports hardware recordings for future frequency, harmonics, THD, level, envelope and transfer comparisons. Generated WAV examples under `artifacts/Audio` are software renders, not hardware references.
