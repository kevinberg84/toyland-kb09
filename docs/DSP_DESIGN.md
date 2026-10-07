# DSP design evidence and implementation boundaries

Applies to **Toyland KB-09 0.11.0**, including its extended Modern MOD ranges. The standalone core evidence below is distinct from validation of the final 0.11.0 plug-in binary.

The primary evidence is recorded in `Research/SH09_RESEARCH.md` with original documents and schematic renders. The numerical DSP implementation and tests are described by `DSP_DESIGN_DSP.md` when present. This document records the hardware constraints that the implementation must preserve; it does not imply every proposed approximation is a measured circuit match.

## Required routing

Main oscillator selector (noise/saw/square/pulse) and phase-derived sub divider feed individual VCO/sub mixer levels. External input is preamplified, then splits into (a) external mixer fader and (b) fixed positive rectifier/follower. The three audio signals sum before the four-OTA low-pass. Its output passes through the Hold/Envelope/Gate VCA and mono volume stage. Filter envelope source selects positive ADSR, negative ADSR or follower. Keyboard CV and bender feed cutoff independently of oscillator range.

PWM receives an undelayed sine derived from the common LFO even when the pitch/filter LFO selection is square or random. Sine pitch/filter delay follows effective low-note trigger logic. The LFO and oscillator run continuously, avoiding artificial per-note phase resets. The sub divider must be clocked by main-oscillator cycles, maintaining integer octave relationships.

## Numerical approach

Use oversampling for oscillator discontinuities, nonlinear filter and input/VCA saturation. PolyBLEP or equivalent bandlimited discontinuity correction is required for oscillator and derived sub transitions. A shared phase/cycle counter ensures coherent sub modes. Filter states use double precision where useful and finite-value checking in tests. No allocation or lock is permissible in the render path; all queues and DSP storage have fixed/prepared capacity.

A suitable reduced OTA model uses four local-feedback integrator stages and a global resonance path solved at the current sample by a bounded zero-delay solver. Differential-pair saturation and the explicit antiparallel resonance diodes should be represented separately. Derive stage normalization from the documented 58/68 kOhm input and 100 kOhm local-feedback network rather than claiming a generic equal-gain ladder is the original. Exact transistor bias, diode voltage mapping, OTA gm tolerances and op-amp slew are not presently calibrated. If using a normalized nonlinear four-pole proxy, label it **circuit-informed**, never circuit accurate.

The follower should preserve the pre-EXT-fader tap and positive amplitude action. The actual network has 100 kOhm resistors, .1/.1/.033 uF capacitors and one capacitor returned to the buffer output. A three-pole digital smoothing proxy can approximate response scale but is not the exact active-network transfer. Its constants must remain distinct from ADSR settings. Do not make silence open the VCA in Authentic mode.

## Control mapping

| Control | Documented endpoints | Digital mapping status |
|---|---|---|
| Tune | -65 to +65 cents | Linear cents faithful to endpoint specification |
| Oscillator range | 32,16,8,4,2 feet | Exact powers of two |
| LFO | .2 to 25 Hz | Exponential interpolation is an engineering approximation |
| Sine delay | 0 to 1.5 s | Response shape and fader taper unmeasured |
| Attack | .001 to 2.5 s | Exponential time map; attack curvature requires verification |
| Decay/release | .002 to 10 s | Exponential time map; define test endpoint tolerance explicitly |
| Sustain | 0 to 1 | Normalized envelope level |
| Portamento | 0 to 5 s | Control law and interval dependence unmeasured; freeze when gate drops |
| Cutoff | 10 to 20,000 Hz | Log-frequency map at reference key; fixed one-octave/octave tracking |
| VCO MOD | Maximum interval not established by the reviewed primary documents | Authentic: up to ±12 semitones. Modern: up to ±36 semitones (±3 octaves), before the oscillator frequency bound. |
| VCF MOD | Maximum interval not established by the reviewed primary documents | Authentic: up to ±5 octaves. Modern: up to ±8 octaves, before the filter frequency bounds. |
| Pulse width | Square at zero, progressively narrower | Exact minimum duty unmeasured; retain stable lower bound in software |
| Bender VCO/VCF | Independent amounts, bipolar spring-centered control | Maximum interval is not published; disclose chosen range |
| External gain | <= .5 Vpp sensitivity | dBFS conversion and clipping scale require hardware calibration |

Sliders in schematics include A- and B-tapers; smooth/logarithmic numerical maps are not evidence of the exact original taper. Smooth continuous automation over a short sample-rate-independent interval. Discrete selector changes should not allocate or reset unrelated voices.

## Keyboard and envelope

Use low-note priority across held keys; note-off reveals the next-lowest remaining key. Gate is high for any held key. Gate+Trigger re-attacks from the current envelope value when the effective note changes, with Gate mode retriggering only on a fresh gate. LFO mode retriggers within the gate. Sustain-pedal and duplicate-MIDI handling are software extensions to a physical keyboard and should be consistent, bounded and tested. All-notes-off and all-sound-off must safely terminate stuck notes.

## Authentic and optional extensions

Authentic mode has no velocity/aftertouch response, tempo-locked LFO, stereo effects, independent noise mix, polyphony or autobend. Input trim is a software calibration aid; if presented as a Modern extension it must bypass to the defined nominal input scale in Authentic mode. Age/Condition is an optional model and visual treatment, not a historically measured calibration. Preserve the original controls and avoid adding random pitch wander so large that the well-stabilized uA726 VCO becomes misleading.

Version 0.11.0 extends the two MOD faders in Modern mode without changing their IDs (`vcoMod`, `vcfMod`), indices, normalized 0–1 ranges or state schema. At full depth and an LFO value of ±1, VCO pitch spans one eighth to eight times its unmodulated frequency (±36 semitones); VCF cutoff spans one 256th to 256 times its unmodulated frequency (±8 octaves). Other pitch/cutoff sources still sum normally. The existing oscillator range of 0.01 Hz to 0.2 times the internal sample rate and filter range of 5–22,000 Hz remain enforced; extreme settings reach these bounds. PWM and ADSR/follower depth are unchanged.

Authentic retains the previous ±12-semitone VCO and ±5-octave VCF engineering mappings. These are not claimed as measured or documented hardware depth maxima. This MOD extension leaves Authentic depth behavior unchanged. Saved Modern patches from versions through 0.10 with a nonzero MOD value intentionally produce deeper modulation at the same stored value: pitch depth triples and VCF depth increases by 1.6. To recover a prior Modern patch's depth, divide its VCO MOD value by 3 and multiply its VCF MOD value by 0.625. There is no automatic state migration for these values. Switching mode uses the existing brief discrete-control fade; continuous depth changes retain the existing smoothing.

## Validation and later hardware calibration

Measure oscillator/sub frequency and harmonics, phase relationships, pulse duty and alias residue; envelope crossing times and retrigger level; filter low-level response and self-oscillation pitch/level; detector attack/decay and external transfer gain. Cover 44.1/48/88.2/96/192 kHz and non-power-of-two blocks. Compare finite peak/RMS/DC statistics during extreme controls. Render matched, explicitly documented test patches for later physical-unit capture. Retain raw recordings and calibration metadata. A passing digital stability suite proves stable software, not sonic equivalence to a real SH-09.

The extended MOD core was compiled with MSVC x64, C++20, `/O2 /W4` and tested independently of the plug-in package. `artifacts/ModDepth/DSPTests.txt` records **92 passed, zero failed**. Audio measurements at 44.1 and 192 kHz verified full VCO modulation at approximately 55/3520 Hz in Modern versus 220/880 Hz in Authentic for a 440 Hz starting pitch. A 220 Hz external tone measured the deeper Modern negative VCF sweep and confirmed that Modern depth 0.625 reproduces the prior full-depth transfer. Existing five-rate stress now runs the maximum Modern depths at a 25 Hz LFO; all runs remained bounded with zero nonlinear-solver failures. Tracked C++ `new`/`new[]` calls were zero during those processing/control-update loops. This instrumentation does not cover every allocator or the host wrapper. These core results do not establish a frozen 0.11.0 module hash, a new host/validator pass, or listening approval; those belong in the separate final validation record.
