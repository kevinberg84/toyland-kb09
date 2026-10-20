> **Historical record.** Version numbers, proposals, pending statuses and test results below refer to the revision/date described, not the current release. See [1.9.0 validation](VALIDATION_1.9.0.md) and [current sound-model evidence](FIDELITY.md).

# Resonance correction: integrated preview

13 October 2026. Implemented in the local source checkout for audition; no
installed application, public download or update feed has been replaced.

## What changed

The revised engine distributes nonlinear headroom across the four normalized
filter stages and solves a separate loaded diode resonance limiter in the same
instantaneous feedback system. This addresses the excessive competition between
the steady VCO and self-oscillation demonstrated by the supplied recordings and
the earlier sensitivity tests. It does not prove that every held output harmonic
is physical injection locking.

The three later stage gain ratios use the schematic's 100k/68k resistor ratio.
Changing state coordinates preserves small-signal input/output gain. The final
stage headroom is `2.2 * .6200804173378996`, with preceding normalized headrooms
scaled by successive powers of `100/68`. The diode voltage scale is
`2.8 * .6200804173378996`; its normalized law uses alpha `2e-6`, beta `12` and
maximum loop gain `4.65`. Absolute scales remain engineering parameters, not
measured BA662 or diode specifications or a complete SH-09 nodal derivation.

The five coupled unknowns are solved with Newton iteration and an analytic
Schur complement. A shared exponential supplies sinh/cosh without a lookup
table, artificial feedback delay, output AGC, added sine oscillator or VCO bypass.
The new diode state is cleared on reset/all-sound-off.

Changing the limiting also changed autonomous oscillation tuning. A separately
measured continuous RES-dependent correction to the integration coefficient
preserves the previous tuning. It is unity below self-oscillation onset and is
linearly interpolated above it. Cutoff remains continuous; no pitch quantization
is introduced. The correction is calibrated against the retained model to avoid
detuning existing sounds, not asserted to be a factory SH-09 CV calibration.

`FilterModel::legacy` retains the original equations for independent regression
and listening comparisons. The uncommitted default is `FilterModel::revised`.
This selector is internal and is not a new GUI control or saved parameter.

## Actual modulation-path checks

These tests use the real Authentic LFO/control path at 48 kHz: RATE minimum,
VCF MOD .45, CUTOFF .58, maximum RES, steady VCA and a stationary VCO. They do not
override cutoff or apply fitted gains. One second is excluded at each capture
edge. Spread is P90 minus P10 of 50 ms RMS; worst sustained dip uses 250 ms RMS
relative to that signal's median.

| Wave / VCO mixer | Original spread | Revised spread | Original worst sustained dip | Revised dip |
| --- | ---: | ---: | ---: | ---: |
| Saw / .5 | .89 dB | .25 dB | 2.14 dB | .16 dB |
| Saw / 1 | 5.32 dB | .70 dB | 12.97 dB | .32 dB |
| Square / .5 | 2.13 dB | .39 dB | 12.75 dB | 1.19 dB |
| Square / 1 | 8.30 dB | 1.62 dB | 14.82 dB | 2.26 dB |

The strong square-wave case is improved but retains more interaction. Do not
describe the staircase problem as completely solved. Kevin reported a slight
audible staircase improvement for the earlier proposal; the integrated audition
awaits his assessment.

Twelve nonresonant saw/square level checks at 44.1/48/96 kHz differ by at most
.0086 dB. Thirty-six self-oscillation tuning comparisons, including intermediate
RES values and three cutoff settings at those rates, differ by at most 1.11 cents.
These are bounded spot checks, not proof that every resonant patch is unchanged.

Native validation passes 202 existing DSP checks, 161 targeted integration checks,
and 153 independent legacy/reference scenarios. The latter remain sample for
sample identical. Existing stress, no-allocation and alias suppression checks
pass with the revised default. All six CTest suites also pass in the authoritative
checkout: DSP, resonance integration, legacy reference, processor, preset files
and the factory bank (123 seconds total). Logs are
`artifacts/resonance-integration-build.log` and
`artifacts/resonance-integration-ctest.log` in that checkout.

## Hardware-matched preview

The audition uses Kevin's SH-09 VCO-10 segment and the earlier comparison's
empirical free-filter sweep. Each model uses one fixed output gain from its free
oscillation and an independently fitted high-filter VCO carrier level. No
time-varying envelope fitting, AGC or peak normalization is applied. Diagnostic
cutoff inversion compensates the new tuning correction to retain the same free
frequency trajectory; the actual-control checks above have no such override.

The original model's 50 ms spread is 2.85 dB; the integrated revision is .45 dB;
the SH-09 specimen is .68 dB. Its worst 250 ms dip is 1.49 dB, versus 8.72 dB for
the original model and about .37 dB for the calibrated revision. Flatter than
this specimen is not automatically better; waveform shape, resonance onset,
frequency/phase behavior and listening remain important.

At equal six-second magnification, the SH-09 also has short dips and beating
that are less visible in the user's zoomed-out DAW screenshot. The major original
model amplitude collapse is substantially reduced. The worn hardware remains a
specimen comparison, not a clean factory calibration target.

## CPU and practical limits

The optimized integrated process loop costs roughly 8-19% more than the retained
legacy path in the final native spot checks. At 48 kHz, the measured revised
wall/audio-time ratios are about .10 for the resonant sweep and .088 for the
moderately resonant square patch, on this machine. Preparation, warmup and disk
I/O are excluded. These are average single-machine measurements, not real-time
DAW deadline or dropout guarantees. There is no additional reported latency,
oversampling factor or audio-thread allocation.

The altered saturation distribution can change the timbre of resonant patches.
The existing factory bank, external-input paths and normal presets must be
auditioned; the level and tuning checks do not certify identical harmonic balance.
The proposal improves the recorded problem without establishing complete
component accuracy or universal elimination of staircase interaction.

## Artifacts and reproduction

- `Source/DSP/ResonanceModel.h`: revised solver and tuning calibration.
- `Tests/ResonanceIntegrationTests.cpp`: real-path improvement, level, pitch and
  reset checks; registered in CMake and both native platform workflows.
- `Tests/ReferenceDSP152/`: verified frozen pre-correction sources. Historical
  analysis scripts now read these copies rather than the evolving production DSP.
- `Tests/Experiments/validate_resonance_integration.py`: isolated native suite.
- `Tests/Experiments/render_revised_resonance.py`: integrated-source audition,
  waveform figure and full-process timed-loop check.
- `artifacts/AnalogInvestigation/ResonanceIntegration/`: validation logs, raw
  pitch-calibration observations, source hashes, WAVs, waveform image and local
  preview `index.html`. Audio players expose the original and revision separately,
  the hardware reference, a sequential A/B, and identical-control renders.

The preview is comparison material, not a release approval. Do not install,
publish or announce a new release until the proposed sound has been reviewed.

## Manual and pending GUI change

The next-build manual source and self-contained HTML have more specific practical
hardware recommendations; general Arch/XWayland/PipeWire launch and audio help;
matching Windows/Mac security boxes; a 56-word AGE appearance explanation;
envelope-filter terminology; and swipe-up/restore instructions in the display
section. Historical KB-09 version discussions were removed. Hardware examples
are recommendations, not certified minimum benchmarks. The friend's exact Linux
specs are unknown; Kevin authorized general guidance using the existing notes.

The already approved Vintage key crack at 40% of Sold As-Is remains the separate
pending GUI edit. No additional GUI redesign is introduced here.
