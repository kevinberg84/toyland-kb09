# SH-09 amplitude and staircase interaction: tested proposal

13 October 2026. Offline research, not a production DSP change or release.

Subsequent implementation and audition: [integrated preview](SH09_RESONANCE_CORRECTION_PREVIEW.md).
This memo preserves the original six-prototype results; the integrated candidate
adds stage-dependent headroom, unit-gain normalization and tuning calibration.

## Recommendation

Rebalance the OTA stages' nonlinear headroom and the resonance feedback limiter
together. This is now supported by numerical prototypes, rather than a proposal
based only on the appearance of harmonic holds. Increased headroom alone is a
useful lower-complexity candidate. A loaded diode limiter combined with that
headroom correction is the most promising candidate tested at unchanged maximum
small-signal feedback gain.

In plain language, the incoming VCO and the filter's own oscillation currently
compete too strongly for the model's nonlinear operating range. Give the filter
stages more room to operate, while controlling the oscillation in the feedback
path. Recalibrate their relative levels and pitch afterward. This explanation is
a supported modeling hypothesis; it does not identify a measured faulty BA662
parameter or prove a particular physical locking width.

No output AGC, unfiltered VCO bypass, added ideal sine oscillator, or artificial
removal of harmonic interactions is proposed. Genuine beating and locking can
remain. A perfectly flat sweep would not necessarily reproduce the hardware.

## Established reading

- [Zavalishin, The Art of VA Filter Design, revision 2.1.0](https://www.discodsp.net/VAFilterDesign_2.1.0.pdf),
  sections 6.3 and 6.9. Explains how input and feedback sharing a saturator can
  suppress resonance, and how the position and scaling of nonlinearities change
  filter behavior. The OTA discussion connects differential-input saturation to
  slew limiting. This supplies a mechanism to test, not SH-09 component values.
- [Bogason and Werner, Modeling Circuits with Operational Transconductance Amplifiers Using Wave Digital Filters, DAFx 2017](https://www.dafx.de/paper-archive/2017/papers/DAFx17_paper_89.pdf).
  Derives OTA differential-pair behavior and discusses limitations of simplified
  models. A generic differential-pair law does not establish the BA662's actual
  input scaling or its surrounding resistor network.
- [Odame, thesis, chapter 6: OTA-C oscillators with native amplitude limiting](https://hasler.ece.gatech.edu/Courses/ECE6435/theses/odame_phd.pdf).
  Examines oscillator amplitude regulation through OTA nonlinearities. Its
  second-order CMOS examples differ from the SH-09; use the principles, not its
  numerical parameters.
- [Parker, Zavalishin and Le Bivic, Reducing the aliasing of nonlinear waveshaping using continuous-time convolution, DAFx 2016](https://dafx.de/paper-archive/2016/dafxpapers/20-DAFx-16_paper_41-PN.pdf),
  section 6.3. Demonstrates stepped resonant sweeps caused by aliasing. This is a
  real alternative explanation, but the KB-09's broad dips and harmonic holds
  persist in the earlier higher-oversampling tests. Oversampling alone is not
  the leading remedy for these recordings.
- [Razavi, A Study of Injection Locking and Pulling in Oscillators, 2004](https://www.seas.ucla.edu/brweb/papers/Journals/RSep04.pdf).
  Describes amplitude/phase interaction and pulling. Its RF oscillator formulas
  are not SH-09 calibration rules. A bright horizontal output harmonic can also
  be ordinary filtered VCO energy; it is not a unique locking detector.
- [Roland SH-09 service notes](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf),
  schematic on printed page 5. Constrains the actual topology and nominal
  calibration independently of the worn specimen's recordings.

## Evidence and controlled comparison

Kevin's controlled recording has three segments, VCO mixer 10, 5 and 0, with
maximum RES confirmed. The original 96 kHz float recording is preserved:

`C:/Users/kevin/OneDrive/Desktop/SH-09 Test #2.wav`

SHA-256:
`8fe188934d3cc8cf2e99d34a74609dfa200f7033592c03bfc41f62a12fe0163a`

The instrument is approximately 40 years old and poorly maintained. Its measured
behavior is specimen evidence, not a factory calibration target. Earlier paired
recordings independently show excessive KB-09 level variation across eight
sweep takes, with comparably steady plain VCO references.

Six prototypes are generated from copies of the shipping equations. Tests use
96 kHz output, 4x internal oversampling, Minty/Modern controls to isolate the
filter, a steady VCA, and a saw near 177.76 Hz. These are diagnostic model renders,
not a complete recreation of the AUTHENTIC-mode plugin session.

Each candidate is independently calibrated to the same empirical free-filter
sweep. Three static free-oscillation measurements invert its own pitch law.
One fixed output gain matches free-oscillation median level; input drive is fitted
only to the stationary VCO band when the filter is high. The time-varying output
envelope is not fitted or normalized. Residual carrier mismatch is below 0.07 dB.
The hardware's mixer-5 carrier amplitude is approximately 0.172 of mixer-10;
that measured ratio is used instead of assuming a linear half-level.

Level variation below means the 90th minus 10th percentile of windowed RMS in
decibels, excluding one second at each end of the 18-second comparison.

| Case | 50 ms | 250 ms | 500 ms |
| --- | ---: | ---: | ---: |
| SH-09 specimen, VCO 10 | 0.68 dB | 0.51 dB | 0.47 dB |
| Current equations, calibrated | 2.85 dB | 1.43 dB | 1.15 dB |
| Increased stage headroom | 0.67 dB | 0.36 dB | 0.51 dB |
| Loaded diode limiter alone | 4.21 dB | 2.12 dB | 1.69 dB |
| Headroom + loaded diode limiter | 0.47 dB | 0.34 dB | 0.29 dB |

The limiter-only result is important: a more detailed nonlinearity is not
automatically a better model. The combination matters. A stronger-loop variant
reduces variation further, but is not preferred simply for having the flattest
output; it risks excessive stabilization and a changed resonance onset.

Rare dips are checked separately so that the percentile metric cannot conceal
them. Worst 250 ms dips below each signal's median are 1.49 dB for the hardware,
8.72 dB for current equations, 1.34 dB for increased headroom, and 0.70 dB for the
combined candidate. At 50 ms, even the hardware has a 5.29 dB dip; the combined
candidate has a 6.04 dB dip versus 13.40 dB for current equations. Short beating
events are not equivalent to a sustained amplitude collapse.

## Prototype equations and limits

The stage nonlinearity is expressed as `S * sat((input - state) / S)`. Raising
`S` from 1.1 to 3.3 preserves its unit small-signal derivative while increasing
nonlinear headroom. A common factor of three is a sensitivity experiment, not
a measured physical voltage scale or a final per-stage circuit derivation.

The normalized loaded antiparallel-diode experiment uses:

```text
d + alpha*sinh(beta*d) = (1 + alpha*beta)*k*y3/L
feedback = L*d
alpha = 2e-6; beta = 12; L = 2.8; k = 4.65
```

Its small-signal feedback slope remains `k`. The diode unknown and four filter
stage unknowns are solved together with Newton iteration, without inserting an
artificial sample delay. Coefficients and voltage normalization are engineering
hypotheses. This is not yet the complete loaded SH-09 diode/first-stage network.
All recorded native solver runs report zero convergence failures.

The preferred combined candidate retains maximum `k = 4.65`; increasing it to
5.5 solely to flatten the sweep is not the recommendation.

## Additional checks and remaining risks

With calibration parameters frozen from the original saw comparison:

| Held-out model case | Current equations | Headroom | Headroom + diode |
| --- | ---: | ---: | ---: |
| Saw an octave higher, 50 ms variation | 1.98 dB | 0.65 dB | 0.37 dB |
| Square, 50 ms variation | 5.97 dB | 1.56 dB | 0.89 dB |

These are additional model stress tests, not newly recorded hardware validation.
Spectrograms show shorter harmonic holds and more continuous moving ridges in
the successful candidates. This supports likely staircase improvement but is
not a measurement of physical locking width or an assistant listening judgment.

Five ordinary-patch spot checks use identical commands, drive and original
output gain across candidates. Four have RMS differences under 0.03 dB. A
near-onset resonant square differs by about -0.62 dB with headroom and -0.94 dB
with the combined candidate; harmonic balance also changes. The user's otherwise
good sound must be protected through broader listening and regression checks.
Native wall times include startup and disk writes and are not real-time CPU
benchmarks. The diode solver's additional runtime cost remains to be measured.

The moving free-oscillation waveform still differs from the specimen. A simple
high-frequency harmonic descriptor is not an exact harmonic match or stationary
laboratory THD measurement. The prototype is evidence for direction, not proof
of complete analog fidelity.

## Schematic correction

High-resolution tracing corrected an earlier research annotation: **R79 58k is
in the cutoff-control path, not the first audio input path**. Audio mixer paths
R93/R95 (both 390k) and R96 (330k) feed through C16 to IC11 pin 2; global resonance
reaches the opposite differential input, pin 3, through R109 (12k). The 1k
shunts and local feedback must be included in the node equations. Do not derive
an audio attenuation ratio from R79 or collapse these two nodes into one.

The historical notes are corrected. No production change depended on that
annotation, and these numerical tests are based on the shipping equations.

## Next implementation and acceptance criteria

1. Derive consistent first-stage audio/feedback node scaling from the actual
   schematic, including the loaded limiter, and separate source gain from OTA
   headroom. Use the successful reduced prototype as a behavioral cross-check.
2. Jointly calibrate free-oscillation level, cutoff/pitch law, relative VCO level
   and resonance onset. Constrain nominal ranges using service information;
   retain the worn instrument as an independent specimen check.
3. Require meaningful improvement in the supplied sweep cases without lowering
   the VCO band to hide interaction. Evaluate both typical spread and worst
   sustained dips; a flatter-than-hardware output is not automatically better.
4. Compare the staircase behavior by listening and frequency/phase analysis;
   separate the filter ridge from stationary VCO harmonics. Preserve legitimate
   analog interaction rather than promising to eliminate every held region.
5. Check ordinary patches, low/high RES, waveform/drive variation, resonance
   onset, 44.1/48/96 kHz behavior, aliasing, solver stability and real-time CPU
   before merging into the production engine. Preserve the current baseline
   for direct comparison.

Subjective engineering confidence, not a statistical probability:

- **About 90%** that this approach can meaningfully improve the reported
  amplitude problem after integration and calibration.
- **About 75%** that it will also meaningfully reduce the audible staircase
  interaction. Spectral evidence is encouraging; perceptual confirmation and
  physical locking attribution remain incomplete.
- No claim of exact factory fidelity or complete elimination of the interaction.

## Reproduction and artifacts

Scripts: `Tests/Experiments/prototype_resonance_limiter.py` and
`Tests/Experiments/evaluate_limiter_prototypes.py`. They generate isolated engine
copies under `artifacts/AnalogInvestigation/LimiterPrototypes/`; they do not edit
production DSP. Run the first script before the second. `--analyze-only` reuses
existing native render files. Scripts require the recorded inputs, MSVC for new
native renders, and the local numerical-analysis Python dependencies.

Key artifacts there: `results.json`, `checks.json`, `level-tail-check.json`,
`prototype-levels.png`, `prototype-spectra.png`, `held-out-model-stress.png`,
`solver-check.txt`, and `checks-solver.txt`.

`audition-current-then-prototype.wav` contains six seconds of calibrated current
equations, half a second of silence, then six seconds of the combined candidate.
It uses the same sweep window, fixed calibration gains, and 5 ms splice fades;
there is no AGC or peak normalization. It is an offline diagnostic audition,
not a built plugin. `audition-notes.json` records its provenance.
