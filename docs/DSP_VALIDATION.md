> **Historical record.** Version numbers, proposals, pending statuses and test results below refer to the revision/date described, not the current release. See [1.9.0 validation](VALIDATION_1.9.0.md) and [current sound-model evidence](FIDELITY.md).

# Core DSP validation

Executed 2026-09-27 in the shared workspace with Visual Studio 2026 Developer Environment 18.10.2, native x64 `cl`, C++20, `/O2 /EHsc /W4`. Both standalone test and renderer builds completed without warnings. These are core tests; VST3 wrapper, GUI and Ableton results are recorded separately.

## Result

83 assertions passed, zero failed. The saved run took 5.66 seconds. Full output: `artifacts/Validation/DSPTests-release.txt`. The sources are `Tests/DSPTests.cpp` and `Source/DSP/Engine.h/.cpp`.

| Check | Evidence |
| --- | --- |
| Pitch/octave range | 110, 440 and 1760 Hz targets across 44.1, 48, 88.2, 96 and 192 kHz; relative error below 0.1% threshold. |
| Sub divider modes | f/2 square, f/4 square and f/4 pulse checked at all five rates. |
| Keyboard | Lowest-note selection, ignored higher note, note-off ordering, duplicate notes, velocity-zero note-off, legato and held-key release. |
| Envelope | Minimum/intermediate/maximum attack, finite minimum release, sustain retention under Gate legato and exact silence following release. |
| Portamento | Glide continuation to target; pitch stops changing when the last key is released, at 44.1 and 96 kHz. |
| LFO | Sine delay, ignored higher note not restarting delay, square bypass of sine delay and sample-and-hold events at 25 Hz. |
| External audio | Detects before external mixer fader, silent audio when fader zero, released follower on silence, audio passage through Hold/filter path. |
| DC and panic | Asymmetric pulse/sub steady DC below 0.0002; All Sound Off silences Hold until a subsequent note or VCA mode interaction. |
| Self-oscillation | No main/sub/external audio; high-resonance test produces RMS 0.2405 at approximately 827.33 Hz. This is model behavior, not a hardware target measurement. |
| Aliasing | Coherent high saw at 5921.63 Hz / 44.1 kHz: nonharmonic energy −57.82 dB relative to harmonic energy, versus −6.81 dB for a naive base-rate saw. Hann FFT, harmonic neighborhoods excluded. |
| Noise sample-rate level | RMS 0.04185–0.04246 across the five sample rates under a fixed patch; variation approximately 1.5% peak-to-peak. |
| Stress | All inputs/mixers high, resonance high, changing cutoff/source/pulse width/VCA mode/pitch bend, MIDI bursts, invalid external values; finite bounded output at all five rates. |
| Solver | Zero iteration-budget failures in every tested stress, self-oscillation, sustained processing and rendered demonstration. |
| Continuous processing | 15 seconds at 48 kHz/max resonance; finite output, nonzero energy and zero solver failures. |
| Latency | External impulse peak at sample 130, consistent with 128 samples of linear-phase resampling delay plus the voice filter's phase response. |
| Repeatability | Identical seeded condition/patch renders; nonfinite controls and input are rejected. |

## Corrections made during testing

1. Self-oscillation originally remained at zero because the solver's absolute residual tolerance erased its very quiet excitation. Tightening the residual tolerance and retaining a bounded low-level excitation restored self-oscillation. The complete suite passed afterward.
2. Code review identified asymmetric pulse DC. A 5 Hz output coupling stage was added and checked with pulse plus 25% sub. Its pole is an engineering approximation to hardware AC coupling.
3. All Sound Off initially cleared state but allowed Hold to resume sound. A mute latch now persists until a note or VCA mode interaction; regression tests pass.
4. Discrete controls initially changed before the fade. They now switch only after a 2 ms fade to zero and fade back over 2 ms; VCA and follower selections are carried with delayed control frames.
5. Noise excitation originally had rate-dependent integrated power. Exact one-pole variance normalization prevents level shrinking as sample rate rises; all five rate checks pass.
6. An LFO delay assertion originally sampled a sine zero crossing. The observation time was corrected; this was a test defect, not a DSP change.

## Performance and audio artifacts

The saved max-resonance benchmark rendered 15 seconds at 48 kHz in 1.56 seconds, roughly 10.4% of one real-time core on this host. This is offline wall time, not a host dropout measurement or a portable CPU guarantee. Other successful runs were faster; concurrent system load can alter the result.

`Tests/Render.cpp` wrote seven mono PCM16/48 kHz five-second WAVs under `artifacts/Audio`: bass, resonant lead, random modulation, self-oscillation, external follower, condition, and a pulsed 110 Hz external-input stimulus. The renderer reported zero solver failures. Log: `artifacts/Validation/Render-release.txt`. These generated demonstrations are not recordings of real SH-09 hardware and no listening comparison is claimed.

## Limits of this evidence

The alias metric covers one stationary high saw, not every PWM/frequency/automation condition. Resonance, internal gain, saturation, waveform geometry, precise envelope/follower curves and sound condition have not been fitted against physical SH-09 measurements. The BA662 model is a reduced nonlinear OTA-style approximation. No SPICE comparison or hardware A/B is claimed. A 15-second sustained test is useful regression coverage but not an hours-long endurance test. Wrapper tests must separately prove audio-thread allocation behavior, state, parameter automation, host bus routing and lifecycle safety.

API/features were frozen after the final core corrections. Any subsequent change requires relevant regression tests and refreshed logs before packaging.
