# KB-09 sound model: evidence and limits

Release scope: **KB-09 1.9.0**. Reviewed 19 October 2026 against the released engine, current manual, recording-comparison provenance and release validation. This page describes the current model; older design and investigation records are historical evidence, not current release specifications.

## What comes from the SH-09 documentation

The original owner and service documentation establish the monophonic signal path, low-note selection, noise/saw/square/pulse sources, divider sub oscillator, mixer, envelope and LFO routing, external audio and envelope following, and VCA modes. The service schematic identifies four selected BA662 filter stages and a BA662 VCA. These relationships guide the software, but reading a schematic correctly does not establish that its digital approximation behaves identically.

See the [SH-09 documentation research](SH09_RESEARCH.md) for sources and circuit references.

## What the released sound engine does

The 1.9.0 engine uses four-times internal processing, band-limited oscillator edges and filtered downsampling. The filter is a reduced nonlinear four-stage model. Its stage states and a diode-like resonance limiter are solved together with instantaneous feedback. Later stages have different nonlinear headroom scales, informed by the schematic's interstage gain relationships. The filter generates its own self-oscillation through that feedback loop; it does not mix in a separate sine oscillator or use automatic level correction.

The model also applies a resonance-dependent pitch correction to compensate for the frequency shift introduced by its nonlinear equations. This is software calibration, not a component copied from the original circuit. Absolute stage headroom, limiter scales, source levels, control mappings and output coloration are engineering choices rather than measured values for every component in the hardware.

Implementation references: [Engine.cpp](../Source/DSP/Engine.cpp), [Engine.h](../Source/DSP/Engine.h) and [ResonanceModel.h](../Source/DSP/ResonanceModel.h). The old 1.3.0 restored-core description and the superseded 1.2.0 candidate do not describe the currently released filter.

## What the hardware recordings contributed

The crucial test put a steady VCO into a self-oscillating filter while its pitch swept slowly past the VCO’s harmonics. The two signals interacted inside the filter, revealing whether its tone stayed steady, changed level or was pulled toward the incoming oscillator.

We compared a collection of SH-09 and KB-09 recordings at different VCO mixer levels, alongside filter-only and oscillator-only recordings, captured at 96 kHz in 32-bit floating point. Listening, waveforms and frequency analysis guided the model toward the hardware’s balance of stable resonance and musical interaction. Matching pitch and relative signal levels made those comparisons more useful than copying slider positions alone.

The earlier approximation could be pulled too strongly toward the incoming oscillator's pitch during the sweep. The revised stage headroom and feedback limiter address that interaction. The improvement follows the hardware's behavior more closely while remaining a reduced mathematical model; it is not a simulation of every transistor and component.

The manual's overview uses common time and amplitude scales and fixed calibration gains, with no automatic leveling or peak normalization. The close-up uses excerpts from the same recordings, each starting at an upward zero crossing. The hardware and software recordings are not phase locked, and fine waveform differences remain. See [the recording and comparison explanation](manual/index.html#filter-craft) and [figure provenance](manual/assets/vcf-comparison-provenance.json).

## What those comparisons do not prove

- The examples compare one aged SH-09 and particular settings, not every surviving instrument or possible patch.
- The recording set is not a component-by-component calibration of BA662 behavior, diode characteristics, noise, oscillator curvature, envelope timing, VCA coloration or external-input sensitivity.
- No full-circuit SPICE equivalence, comprehensive measured error bounds or controlled blind-listening equivalence is claimed.
- Natural beating, signal interaction and unit-to-unit variation remain relevant; matching two pictures is not a complete sound-equivalence test.

## Software additions and compatibility

AUTHENTIC preserves original-style routing; its name does not mean that every control law has been measured against hardware. MODERN adds features including velocity, aftertouch, tempo sync and extended control ranges. AGE is a creative, deterministic condition model for sound and appearance, not a measurement of decades of component deterioration.

The 1.8.2 physical slider changes give PORTAMENTO and selected modulation-depth controls squared response for finer adjustment near the bottom. Stored preset and DAW automation effect amounts retain their meanings; MIDI CC response follows the revised physical taper. The 1.9.0 release adds standalone MIDI receive-channel selection and fixes visual/startup behavior without changing the released sound-engine equations, preset compatibility or reported 128-sample processing latency.

## Release validation and platform limits

The [1.9.0 validation record](VALIDATION_1.9.0.md) distinguishes automated checks, actual hosted tests and user observations. Windows standalone/VST3, universal Mac standalone/AU/VST3 and Linux x86-64 standalone were built and checked in their documented environments. These checks do not establish every DAW, device, desktop or sample-rate/buffer combination.

Linux physical desktop, audio and MIDI confirmation remains outstanding. Mac builds are ad-hoc signed, not Apple-notarized; Intel slices were architecture-checked but not run on a physical Intel Mac. These are current distribution limits, separate from evidence about the sound model.

## Reading older engineering notes

[DSP design notes](DSP_DESIGN.md), [the 1.2.0 filter candidate](SH09_FILTER_MODEL.md), dated investigations and versioned validation files document how the implementation developed. Their version-specific proposals, counts, measurements and pending statuses must not be read as current release claims. Use this page, the [current manual](manual/index.html) and [current validation](VALIDATION_1.9.0.md) for the released instrument.
