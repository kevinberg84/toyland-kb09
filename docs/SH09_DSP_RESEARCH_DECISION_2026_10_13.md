# SH-09 DSP research: next approach after the first recordings

13 October 2026. Research memo, not a release specification. No production DSP
change is selected. The approved key-crack change remains separate and uncommitted.

Later tested proposal: `SH09_AMPLITUDE_AND_STAIRCASE_SOLUTION.md` records six
offline limiter/headroom prototypes, held-out model checks, rare-dip checks,
the corrected first-stage schematic annotation, and confidence estimates.
It supersedes the research priorities below without changing production DSP.

## Decision

Start with **measured input-drive / self-oscillation balance and resonance
calibration in the existing model**. Investigate a circuit-derived resonance
limiter and stage scaling if the calibrated existing equations still fail.
Do not start by replacing the entire filter, raising oversampling globally,
compressing its output, or forcing its oscillation to ignore the VCO.

This recommendation is stronger than before the recordings. It is still not
proof of a particular BA662 or diode parameter error, nor a calibrated
measurement of excess injection-locking width. The next controlled hardware
recordings distinguish a scale error from a limitation of the model equations.

Subsequent controlled recording: `SH09_LEVEL_MATRIX_SECOND_PASS.md` analyzes
Kevin's SH-09 Test #2 at VCO 10/5/0. Matching the measured high-filter VCO band
leaves 2.73 dB model level spread versus .68 dB hardware. Kevin subsequently
confirmed maximum hardware RES, matching the model's maximum panel setting;
actual loop-gain calibration remains unidentified. This supports jointly
testing resonance/limiting rather than choosing
the half-input diagnostic by RMS alone. Kevin explicitly identifies this poorly
maintained, approximately 40-year-old hardware as potentially outside factory
specifications; its measured settings are specimen data, not clean-mode targets.

## What is established, and what is not

The supplied recordings establish a repeatable behavioral difference: all eight
KB-09 sweep takes have more short-window level variation than their hardware
partners, while the three plain VCO references are comparably steady. Recorded
KB-09 spectra also show conspicuous held harmonic regions across whole sweeps.
Hardware has shorter harmonic regions too; its sweep is not completely immune
to the input. See `SH09_RECORDED_COMPARISONS_FIRST_PASS.md` for all measurements,
source hashes, patch limitations, and the longer-window RMS crosscheck.

The original pairs do not have identical sweep spans, rates or documented
resonance operating points. Baseline photos show Modern / Minty on KB-09 and
VCA ENV on both instruments. They do not establish internal signal voltages.
The roughly 6.8 dB absolute recording-level difference is not evidence that
KB-09's filter input is 6.8 dB too hot: output VOLUME and capture gain intervene.

An approximately matched-band test already narrowed that confound. Current
equations with VCO off cover approximately 440–868 Hz smoothly; with the VCO
added, the output varies much more. Controls were inferred from the mixed
hardware recording and maximum model RES was assumed. We still need the
hardware's VCO-off take at the same controls to identify its free sweep and level.

## New experiment: locate the behavior before choosing a fix

`Tests/Experiments/recorded_case_ablation.py` generated isolated copies of the
current engine and changed one factor at a time. Ten variants, each with VCO
off and full, produced twenty 20-second renders at 96 kHz after the same
6.05-second warm-up. Controls are the previous approximate pair-1 fit. The
shipping 4x engine runs internally at 384 kHz; a rate-control copy runs at
16x / 1.536 MHz with equivalent FIR bandwidth and host latency.

All renders are finite and report zero Newton solver failures. The instrumented
baseline output is bit-identical to both previous unchanged-engine renders.
This checks instrumentation, not circuit fidelity. Filter/source diagnostics
sample the last internal value rather than the FIR-decimated output, so these
traces are not suitable for measuring alias energy.

Level spread is P90 minus P10 of 50 ms RMS in seconds 1–19. The 250 ms column
crosschecks the same effect with more time averaging. Figures center each trace
at its own median for display; no audio AGC or normalization is applied.

| Diagnostic | 50 ms output spread | 250 ms output spread | Interpretation |
|---|---:|---:|---|
| Hardware pair 1 | 0.72 dB | 0.57 dB | Recorded target; not an exact patch reconstruction |
| Current model, VCO full | 3.44 dB | 2.86 dB | Baseline in the narrower band |
| Remove output saturation only | 3.49 dB | 2.90 dB | Final output waveshaper does not cause the main dips here |
| Remove input mixer saturation only | 3.78 dB | 3.07 dB | That saturation is not the sole cause either |
| Half filter input, before mixer saturation | 0.94 dB | 0.72 dB | Strong sensitivity to input drive; not a selected gain change |
| Same baseline equations at 16x | 3.73 dB | 2.86 dB | Broad dips persist at four times the internal rate |

The baseline **filter output itself** has a 3.53 dB spread before final VCA/output
saturation. Its driving source is steady. This locates the main model-level
effect inside the driven resonant filter in this operating band, rather than
in an unstable standalone VCO or the last output clipper.

The spectral crosscheck is equally important: harmonic holds remain conspicuous
at 16x, whereas the half-input diagnostic has shorter holds and a trajectory
qualitatively closer to the hardware recording. This is visual evidence of
input sensitivity, not a formal classification of locking or an audible test.

Changing the internal limiting scales also changes the dips, but changes the
free oscillator at the same time. For example, doubling the feedback saturation
scale gives a 0.94 dB output spread but moves free oscillation to approximately
405–799 Hz and raises its recorded model median level by 2.38 dB. Removing
feedback saturation gives only 0.67 dB spread but moves free oscillation to
392–773 Hz and raises its median by 3.12 dB. These are **not winners** simply
because their level variation is small. They demonstrate why frequency,
oscillation amplitude, harmonic balance and driven response must be calibrated
together.

Artifacts: `artifacts/AnalogInvestigation/RecordedCaseAblation/measurements.json`,
`summary.csv`, `solver-check.csv`, `ablation-levels.png`, `ablation-spectra.png`.
JSON preserves production source hashes, controls and generated-copy edits.

## Research that changes the diagnosis

### 1. There are different ways to get a staircase

A swept resonator can emphasize successive harmonics already in a stationary
saw. A self-oscillating nonlinear filter can also synchronize with that drive.
These processes can coexist. A bright horizontal FFT ridge alone does not
separate them, and a mixed waveform does not have a unique instantaneous pitch.

[Razavi's original injection-locking/pulling analysis](https://www.seas.ucla.edu/brweb/papers/Journals/RSep04.pdf)
connects synchronization, amplitude behavior, near-lock phase slips and injected
signal strength. Its RF tank formulas are not SH-09 coefficients. It supports
examining phase, sidebands and drive dependence, rather than equating every
harmonic ridge with a lock. Our hardware/model classification must use the free
sweep as a reference and explicitly mark unresolved crossing regions.

### 2. Aliasing can produce the same symptom, but is not the leading explanation here

[Parker, Zavalishin and Le Bivic, DAFx 2016, section 6.3](https://dafx.de/paper-archive/2016/dafxpapers/20-DAFx-16_paper_41-PN.pdf)
demonstrate steppy resonant sweeps caused by nonlinear alias components in a
Moog-like filter. Their example uses a 5 kHz sine input and a 1 Hz–21 kHz
cutoff sweep, not our low-pitched SH-09 case. Their antialiasing method also
requires care with feedback delay and topology. The new 384 kHz versus
1.536 MHz comparison retains both harmonic holds and nearly identical 250 ms
level spread. This weakens aliasing as the principal explanation in this
particular band. It does not establish alias-free audio at every setting or
justify copying an antialiasing block into the feedback loop without derivation.

### 3. The actual SH-09 topology must constrain the model

The original [Roland service notes, filter schematic](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf)
show four selected BA662 stages, discrete buffers, local feedback, and a
separate antiparallel-diode resonance return. Reinspection confirms C17–20 are
1500 pF, subsequent audio-stage input resistors are 68 kOhm, input shunts
are 1 kOhm, and local feedback resistors are 100 kOhm. Further path tracing
corrects a prior annotation: R79 (58 kOhm) is in the cutoff-control circuit,
not the first audio-input attenuation path. IC11's audio arrives through C16
from the mixer summing resistors; resonance reaches its opposite input through
R109. The earlier abandoned candidate's first-input assignments and 150 pF
capacitor reading are obsolete. Its hand-chosen
device scales are not calibration data. Unity-normalized software stages must
preserve the physical network's gain and nonlinear operating levels under that
normalization; matching four poles alone is insufficient.

The current generic stage law and feedback saturator have plausible behavior,
but their normalized scales are not fitted to that resistor/diode network.
A future circuit-informed candidate should solve the loaded resonance return
with its surrounding resistance and pot position, not substitute a fixed
0.6 V hard clip or reuse a guessed volts-per-engine-unit constant.

### 4. A generic OTA curve is a starting point, not a BA662 measurement

[Bogason and Werner, DAFx 2017](https://www.dafx.de/paper-archive/2017/papers/DAFx17_paper_89.pdf)
derive the familiar bipolar differential-pair current law and distinguish ideal
OTAs from nonideal macromodels. It explains why differential input scaling is
central, but does not supply a measured BA662 transfer curve. [Open Music Labs'
own BA662 measurements](https://www.openmusiclabs.com/wiki/BA662/)
provide useful device-specific evidence and explicitly disclose reverse-engineering
uncertainty and small sample size. They do not characterize Kevin's filter or
justify transferring clone/replacement parameters wholesale.

[Köper et al., DAFx 2022](https://dafx.de/paper-archive/2022/papers/DAFx20in22_paper_34.pdf)
show how measured component behavior and output-voltage dependence matter in
an OTA-based Wasp filter. That is a different topology, CA3080 device and supply
configuration. The lesson is to add nonideal effects when measurements require
them, not import Wasp rail-limiting constants into a BA662 filter.

### 5. Better fitting must not disguise wrong equations

[Esqueda et al., DAFx 2021](https://dafx.de/paper-archive/2021/proceedings/papers/DAFx20in21_paper_39.pdf)
show circuit-parameter fitting against hardware, including control laws and
diode behavior. They also warn that fitted parameters may be non-unique or
compensate for discretization errors. We should constrain schematic values,
identify a few effective gain/limiting quantities, and reserve independent
conditions for validation. A good fit to one take does not recover unique
internal voltages, diode parameters or capacitor values.

[Oyama, DAFx 2026](https://www.dafx.de/paper-archive/2026/papers/DAFx26_paper_29.pdf)
provides a useful controlled-ablation methodology: similar small-signal responses
can conceal nonlinear differences. Its reference is an idealized Moog circuit,
not a measured SH-09. We use the experimental discipline, not its topology or
rankings, and keep aliasing convergence a separate comparison.

## Next experiment and candidate-selection order

1. **Identify the free sweep.** Use VCO/SUB/EXT zero with the exact same
   FREQ/RES/MOD/RATE, note and output/capture settings as the driven takes.
   Measure free pitch trajectory, cycle rate, level versus frequency and
   harmonic content. Do not infer those exclusively from the loudest component
   of a mixed recording.
2. **Identify the drive relationship.** Compare VCO 0/5/10 without moving those
   filter controls, plus the plain VCO references. Fader 5 is not assumed to be
   half the physical signal: hardware pot taper and loading matter. Match
   relative output references within each device, not absolute dBFS across
   two differently gained recordings.
3. **Test the least invasive explanation first.** Constrain input scaling,
   RES onset/above-onset behavior and free-oscillation level together in the
   existing equations. The half-input diagnostic is a sensitivity experiment,
   not permission to turn every VCO down by 6 dB. A global gain change would
   alter ordinary patches and cannot be selected from this table alone.
4. **If calibrated scaling cannot explain the data, test structure.** Derive
   the SH-09's first/later stage attenuation and loaded diode return, normalize
   consistently, and solve coupled nonlinear states without invented feedback
   delays. Keep the existing engine and simpler fitted model as references.
5. **Validate independently.** Predict a held-out mixer level, another VCO pitch,
   rising/falling sweeps and a lower-RES patch. Compare free/driven spectra,
   level envelopes, onset and harmonic-dwell duration. Separate an unresolved
   spectral crossing from an asserted lock interval. Repeat suspicious cases at
   higher rates/tighter solver settings; compare audible-band behavior rather
   than requiring phase-sensitive sample subtraction of autonomous oscillators
   to remain small indefinitely.
6. **Protect normal sounds.** Before production selection, compare ordinary
   low/moderate resonance bass and lead patches, saw/square/PWM, SUB/EXT mixing,
   envelopes, tuning/tracking, several host rates and actual CPU cost. Match
   audition loudness with a fixed comparison gain only. No automatic compensation
   during playback and no published fidelity claim based on one RMS score.

If the VCO-level matrix leaves the mechanism ambiguous, a later optional
external pure-sine drive with a simultaneously recorded dry reference could
make synchronization easier to distinguish from changing saw-harmonic dominance.
It is not a prerequisite for today's work, and its input path must be calibrated
separately. No opening, probing or servicing of the user's SH-09 is requested.

## Confidence and limits

High confidence: the recordings reveal a meaningful difference; in the tested
model case the main dips arise in the driven resonant filter; simply changing
the output saturation or internal processing rate does not remove them.

Good evidence: relative drive and the mechanism limiting oscillation amplitude
are promising levers. The half-input diagnostic improves both envelope behavior
and spectral trajectory in this one approximately matched case.

Not established: the exact hardware/model lock-width difference, a unique
BA662/diode parameter error, the correct global input gain, or a production-ready
replacement. We can now choose an informative next test with confidence without
pretending the final coefficients are known.
