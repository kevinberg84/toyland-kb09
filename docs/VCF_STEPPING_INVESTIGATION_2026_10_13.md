# SH-09 VCF pitch stepping — 13 October 2026

Update: Kevin supplied 22 hardware/software recordings and baseline photos.
See `SH09_RECORDED_COMPARISONS_FIRST_PASS.md` for measured level differences,
the recorded staircase, and an approximate range-matching diagnostic. Those
findings supersede the earlier "hardware recordings pending" status below.
The subsequent `SH09_DSP_RESEARCH_DECISION_2026_10_13.md` records a primary-source
research pass and twenty one-factor diagnostic renders, with the evidence and
limits of the recommended input/resonance calibration approach.

Status: research and isolated offline measurements. Production DSP remains
1.5.2. No sonic candidate is selected, committed, installed, or published.
The separately requested GUI crack adjustment is an uncommitted preview.

## Subsequent clarification and decision

Kevin has approved the final GUI, including the Vintage crack at 40% of the
Sold As-Is length; it remains prepared for the next build. Sound is now the
priority. He clarified that the audible problem is a **staircase of successive
discrete pitches over the sweep**, not merely one flat region near the VCO.
The earlier statement that the offline experiment reproduced his symptom was
too strong: it reproduced a related input/filter interaction, not conclusively
this staircase. Interpret the findings below with that distinction.

The displayed graph measures combined filter-output positive zero-crossing
periods, not the frequency of an independently isolated self-oscillating
component. Strong VCO prominence, beating, changing waveform shape and locking
can all affect that estimate. Spikes are not automatically audible pitch jumps.
The graph supports further investigation but does not by itself prove either
the mechanism of Kevin's report or exaggerated injection locking versus SH-09.

Two distinct acoustic mechanisms must be separated: the moving filter may
highlight successive stationary VCO harmonics (a shift in which component is
heard most strongly), or nonlinear oscillator interaction may create successive
lock regions at frequency ratios. They can coexist. Actual control/frequency
quantization and numerical artifacts remain checks rather than being declared
impossible from one graph. The next analysis should track spectral components
and phase coherence over the **whole** audible upward sweep; one zero-crossing
estimate around 440 Hz is insufficient.

Kevin explicitly requires establishing exaggeration before implementing a fix
whose sole justification is excess locking. **No production DSP change on that
basis is authorized or justified yet.** A hardware/model comparison is needed;
software-only simulation cannot establish the physical instrument's behavior.
Capture the exact KB-09 symptom and its preset first, including actual waveform,
AGE, key and levels. Compare to SH-09 in a common Hz band, matching sweep rate,
isolated filter sweep endpoints, and measured VCO/self-oscillation level
relationships rather than simply matching fader positions. Record both sweep
directions and check whether steps follow harmonic ratios or another grid.

A component-aware model may still merit an isolated prototype for independently
demonstrated fidelity improvements (transfer, distortion, resonance onset,
amplitude limiting, control response). Extra component detail alone is not
evidence of better sound. Retain the current engine as baseline, and select a
replacement only after hardware measurements and controlled listening establish
an improvement without damaging the ordinary patches Kevin already likes.

Session preparation now lives in `SH09_SESSION_CHECKLIST.md`; draft figure and
analysis safeguards are in `manual/ANALOG_COMPARISON_FIGURE_PLAN.md`. The latter
records the executed known-signal sanity checks. It is preparatory material,
not a manual publication or a hardware/model validation result.

## Reproduction and findings

Kevin reports slow self-oscillation pitch motion driven by VCF MOD, Authentic
mode, RATE minimum. With VCO mixer zero the sweep is smooth; adding the steady,
unmodulated VCO produces perceived pitch steps. This substantially narrows the
problem to input/filter interaction. Exact waveform, AGE and levels are still
needed to reproduce his precise patch; these laboratory settings are explicit
test cases, not a claim to have captured his patch.

Code inspection establishes that the built-in sine LFO, cutoff calculation and
nonlinear filter integration run at every oversampled sample (192 kHz at a
48 kHz host rate). There is no MIDI quantization, note snapping, or coarse LFO
table on this route. Panel parameters are continuous floats; the normal 8 ms
parameter smoother is not the sine LFO's update interval. More smoothing is
therefore not the first proposed remedy.

`Tests/Experiments/low_cutoff_interaction.py` creates instrumented copies of the
unchanged production equations at 4x and 16x host rate. FIR bandwidth and host
latency are kept equivalent. At 48 kHz it renders two low-FREQ cases, four VCO
mixer levels (0, 1/3, 2/3, 1), and two integration rates: sixteen 16-second
renders. All use saw, MIDI 69 / 440 Hz, Minty, Authentic, sine LFO 0.2 Hz,
maximum RES, zero VCO MOD / VCF ENV / SUB / EXT, and VCA HOLD.

Case 0 uses FREQ 0 and VCF MOD 1; case 1 uses FREQ .2 and VCF MOD .6. These
normalized controls are model settings, not measured hardware taper positions.
All renders are finite, with zero reported Newton solver failures and output
peak at most .24934. A separate 384,000-sample check compares the instrumented
4x engine to the actual shipping Engine.cpp: **zero sample mismatches**.

The cutoff command is smooth above its lower bound in both cases. Maximum
host-sample change is .15727 cents (case 0) and .09442 cents (case 1); internal
updates are finer still. Case 0 hits the 5 Hz floor for 38.62% of the measured
15-second interval. Case 1 never hits that floor. The floor explains flattened
very-low-frequency motion, but cannot alone explain a VCO-dependent plateau
near 440 Hz, nor the interactions in case 1.

Spectrograms and interpolated positive zero-crossing periods reproduce the
reported distinction: filter alone sweeps continuously; increasing VCO level
introduces a plateau near the steady VCO frequency, with disturbances entering
and leaving it. The plateau and smooth isolated sweep persist at 16x. For
example, at VCO 1 the 4x output has 132 consecutive crossing periods within
0.1% of 440 Hz, spanning about .298 seconds, while its cutoff command ranges
504.81–538.17 Hz. The 16x case has 135 such periods over about .305 seconds,
with command 502.87–538.17 Hz. A mixed waveform does not have a unique
instantaneous pitch, so this is a period/coherence diagnostic, not a definitive
perceptual pitch estimator. It is stronger evidence than an FFT-bin staircase.

This supports input-induced locking/pulling in the model as the leading
explanation. It does **not** establish the real SH-09's locking strength. More
oversampling alone is not a convincing proposed cure for this interaction.
Full waveform differences across rates can be large because of phase/startup
sensitivity, especially through very low cutoff. They are not alias-only
measurements and must not be presented as such. The zero crossing curves also
show some trajectory differences outside the plateau; perfect numerical
equivalence is not claimed.

Artifacts: `artifacts/AnalogInvestigation/LowCutoffInteraction/` contains the
instrumented sources, CSV timings/failures, raw interleaved float traces,
measurements.json, period-measurements.json, baseline-check.txt, spectrograms,
period comparison and same-gain unnormalized audio. `--analyze-only` reuses
renders and regenerates analysis plus the baseline check. NumPy, SciPy and
Matplotlib are analysis dependencies only, not plug-in dependencies.

## What the circuit tells us to investigate

Reinspected the original service schematic, printed p. 5, including the local
render `Research/rendered/filter-5.png`. This SH-09 uses four selected **BA662
OTAs IC11–14**, not a Moog transistor ladder or an interchangeable family filter.
The 1.5 nF integrator capacitors, differing first/later input resistors, local
feedback, discrete buffers, resonance potentiometer and antiparallel D12/D13
limiter define its gain and nonlinear behavior. The service notes require OTA
matching. The normalized model currently uses four identical stage saturation
scales (1.1) and a generic feedback saturation scale (2.8); these are engineering
approximations, not a measured BA662/diode network identification.

The first focused candidate should derive/identify the input attenuations,
stage gains and resonance-limiter behavior together, preserving normal signal
level and frequency response. If recordings show excess model locking, reduce
the excess input forcing **relative to the real self-oscillation limiting**;
do not arbitrarily lower input gain, remove resonance, or bolt on a clean sine
oscillator. A component-aware diode-pair model and properly scaled OTA transfer
are promising candidates, not automatically more accurate without calibration.

Retain implicit nonlinear feedback solution. Adding a sample delay or ordinary
feed-forward ADAA directly inside the feedback loop can change its dynamics;
prior mixer antialiasing experiments address a different question. Keep those
investigations separate. No blanket oscillator replacement, random instability,
noise, or warmth EQ is proposed: the rest of the sound is accepted by Kevin.

Acceptance measurements: isolated self-oscillation frequency/amplitude versus
cutoff; onset and limiting versus RES; low-resonance transfer and mixer-level
response; then lock/pull intervals, phase slips and harmonic balance under slow
relative motion at several VCO levels, both sweep directions, saw and square.
Check 44.1/48/96 kHz and buffer/automation behavior after a candidate is chosen.
Matching one demonstration while damaging ordinary patches is not acceptance.

## Hardware recordings and the range mismatch

Start with the focused four-take protocol in `SH09_RECORDING_GUIDE.md`, using
the same slow sine-filter sweep at VCO 0, 2, 5 and 10. Fixed interface gain and
VOLUME are essential; do not normalize individual takes. A panel photo and
exact patch settings allow the next model experiment to target his sound.

We can compare within the instruments' common frequency range by measuring
the hardware's actual Hz and setting KB-09's note/range/TUNE accordingly. Match
the isolated filter sweep's measured endpoints and rate, not equal-looking
fader positions. No digital pitch shifting is needed. This remains useful even
if the SH-09 is out of calibration.

The range mismatch alone does not prove which instrument is correct. Roland
documents 10 Hz–20 kHz VCF and 0.2–25 Hz LFO endpoints, but not every fader taper
or maximum modulation span. KB-09's Authentic 5-octave VCF MOD span is explicitly
an unmeasured model mapping. Diagnose hardware offset, octave tracking and
modulation depth separately; do not tune the whole model to an assumed fault
in a single instrument. No internal hardware adjustment is requested.

## Primary references and limits

- [Roland SH-09 Service Notes (1980)](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf),
  pp. 1, 5–6: specifications, actual circuit and adjustment criteria.
- [Roland SH-09 Owner's Manual](https://thismodernday.com/wp-content/uploads/2021/07/Roland-SH-09-Owners-Manual.pdf),
  controls pp. 3–5: sine LFO and self-oscillating 24 dB/oct VCF.
- [Razavi, A Study of Injection Locking and Pulling in Oscillators (2004)](https://www.seas.ucla.edu/brweb/papers/Journals/RSep04.pdf),
  sections II–IV, particularly quasi-lock/phase slips, pp. 1419–20. This is
  general oscillator theory, not a measurement of the SH-09.
- [u-he, RePro Filters Unveiled](https://www.u-he.com/downloads/UrsBlog/RePro_Filters_Unveiled.pdf):
  measure hardware nonlinear behavior and compare solvers for the same model.
  CEM3320 parameters do not transfer to the SH-09.
- [Köper et al., A Virtual Analog Model of the EDP Wasp VCF (DAFx 2022)](https://www.dafx.de/paper-archive/2022/papers/DAFx20in22_paper_34.pdf):
  component-aware OTA nonlinear modeling and physical validation methodology.
  Wasp/CA3080 coefficients/topology do not transfer to BA662 stages.
