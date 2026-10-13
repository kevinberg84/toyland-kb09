# SH-09 Test #2: VCO mixer 10 / 5 / 0

13 October 2026. Measured specimen behavior and offline diagnostics. Production
DSP remains unchanged; no proposed sonic change is selected or committed.

## Scope and the hardware's condition

Kevin identifies three clips in `C:/Users/kevin/OneDrive/Desktop/SH-09 Test #2.wav`:
first VCO 10, second 5, third 0. He also explicitly says this approximately
40-year-old SH-09 has not been well maintained and may differ considerably
from its intended specifications. **These measurements describe that specimen,
not a factory-calibrated SH-09 or a universal target for the clean software mode.**

Keep nominal schematic/service constraints separate from observed tuning, sweep
rate, level, control taper and component-condition deviations. The empirical
trajectory below is used only to control a comparison; it is not a proposal to
replace the software's intended LFO range or copy the unit's calibration drift.
Model changes require circuit support and additional operating conditions.

The recording follows a request to leave filter controls, key, VOLUME and
capture gain fixed while moving only the VCO mixer. That is assumed here.
Kevin subsequently confirmed that **RES was maximum (10)**. Both hardware and
model therefore use maximum panel settings. This does not establish identical
physical loop gain, resonance calibration or component condition. Saw waveform
is inferred from the preceding baseline photos
and the stationary harmonic family, not newly documented by a patch photo.

## Source and segmentation

The original is read only. It is 70 seconds, 96 kHz, 32-bit IEEE float, stereo;
channels are near-identical (correlation >.99999) and averaged for analysis.
All samples are finite. Peak absolute magnitude is .429593; none reaches 1.
That alone does not exclude upstream processing or analog limiting.

SHA-256: `8fe188934d3cc8cf2e99d34a74609dfa200f7033592c03bfc41f62a12fe0163a`.

| User label | Annotated original interval | Analysis interior |
|---|---|---|
| VCO 10 | 0–18 s | 1–17 s |
| VCO 5 | 26–44 s | 27–43 s |
| VCO 0 | 54–70 s | 55–69 s |

Silent gaps separate the clips. The second onset is slightly inside its
annotated interval; the one-second exclusion avoids this boundary. LFO phases
differ. Measurements are not sample-synchronous; unequal numbers of partial
cycles can affect percentiles slightly. Originals are neither normalized nor
time-stretched.

## Hardware results

The output stays fairly steady in all three takes. Spread is P90 minus P10 of
windowed RMS in dB, containing the middle 80% of measured levels. It is not a
peak-to-peak excursion, loudness score or a direct isolated-VCO amplitude.

| Mixer position | 50 ms spread | 250 ms spread | 500 ms spread | 50 ms median |
|---|---:|---:|---:|---:|
| 10 | 0.68 dB | 0.51 dB | 0.47 dB | -13.19 dBFS |
| 5 | 0.36 dB | 0.35 dB | 0.34 dB | -13.19 dBFS |
| 0 | 0.34 dB | 0.35 dB | 0.33 dB | -13.25 dBFS |

At 10 the spectra show short harmonic regions and extra components near the
steady VCO; at 5 these are substantially reduced. At 0 the isolated filter
provides a smooth frequency trace. Its observed interpolated positive-crossing
periods span approximately **118–2,734 Hz** in the retained portion.

A sinusoid in log-frequency is only an approximate description: its fit has
about .0865 log-frequency RMS error. A five-harmonic periodic description
reduces that to .01036 and estimates a cycle rate of **.18632 Hz** (about
5.37 seconds per cycle). Its fitted extrema, 121.6–2,655.6 Hz, smooth the
observed maxima/minima and must not be reported as exact measured endpoints.
This fit describes the combined modulation/control/free-oscillation trajectory;
it does not identify which physical block causes its shape. It also does not
establish a factory rate or diagnose a fault.

The near-VCO frequency is approximately 178 Hz. A whole-take PSD peak at mixer
5 was contaminated by the sweeping tone/sidebands. The final analysis instead
checks the demodulated phase while the filter is high, yielding approximately
177.76 Hz at 10 and 177.86 Hz at 5. This small difference is not claimed as
precise oscillator drift: the filtered carrier's phase can vary with cutoff.

During high-filter portions (estimated above 1,800 Hz), an 8 Hz complex-demodulation
lowpass measures the total near-VCO-frequency band:

| Mixer position | Median carrier-band RMS |
|---|---:|
| 10 | -31.53 dBFS |
| 5 | -46.83 dBFS |
| 0 | -90.62 dBFS |

Position 5 is roughly **15.30 dB below 10**, or .172 of its amplitude in this
output band. It must not automatically be represented as a linear .5 input.
This is an output observation with a frequency-selection mask, not direct
measurement of pot resistance, filter-input voltage or an exact taper law.

## Current-model comparison using the isolated sweep

`Tests/Experiments/match_hardware_level_matrix.py` uses generated engine copies.
The current filter equations, stage/feedback saturation and output path remain
intact. The diagnostic overrides cutoff with the empirical free-frequency
trajectory, deliberately separating the filter interaction from LFO-shape
differences. It is not a shipping LFO implementation or a full patch reconstruction.

Four steady VCO-off calibrations invert command frequency to model oscillation
frequency. At 100/500/2,000/4,000 Hz commands the model oscillates near
86.507/432.536/1,730.173/3,460.477 Hz. The generated trajectory follows the
empirical periodic fit. Its VCO-off measured frequency agrees with that supplied
fit to about .029 cents RMS after accounting for output FIR delay. This is
accuracy against the fitted trajectory, **not** .029-cent agreement with the
raw hardware trace, whose fitting error is much larger.

The model uses 96 kHz / 4x, maximum RES, saw at approximately 178 Hz, Minty,
Modern and steady ENV sustain .7. A **single fixed -0.947 dB output gain** is
chosen from the hardware VCO-off median and then held for every driven render.
This corrects overall recording/model gain; it does not compensate dips.

Initial input scales 1, .75 and .5 show:

| Input scale at full VCO | 50 ms output spread | High-filter carrier median |
|---|---:|---:|
| 1 | 4.71 dB | -29.93 dBFS |
| .75 | 1.78 dB | -32.35 dBFS |
| .5 | 0.82 dB | -35.82 dBFS |
| Hardware 10 | 0.68 dB | -31.53 dBFS |

Thus halving input can make the envelope look close while leaving the VCO band
about **4.29 dB too quiet** relative to the calibrated filter tone. This refines
the earlier half-input lead: a good RMS score alone is insufficient.

Interpolating input scale from the **high-filter carrier level only** yields
.82707. A new render confirms -31.528 dBFS versus hardware -31.530 dBFS;
output envelope variation was not a fitting target. Despite the carrier match,
the model retains **2.73 dB** 50 ms spread, **1.43 dB** at 250 ms and **1.13 dB**
at 500 ms, versus hardware .68/.51/.47 dB. The corresponding middle-level
prediction, using .17179 of the full drive, gives -46.715 dBFS versus hardware
-46.830 dBFS in the high-filter carrier band. The source family is therefore
reasonably aligned by this particular output observable, while the driven
envelope still differs at full level.

Sixteen driven/free trajectory renders plus four static frequency calibrations
are finite and report zero solver failures. Original production-source hashes
are recorded in the comparison JSON. No production source changes are required.

## Interpretation and next decision

This strengthens evidence that **input scaling alone is insufficient in this
maximum-RES comparison**. The next useful offline comparison should
jointly constrain resonance operating point, self-oscillation limiting and
stage gain/attenuation using the actual SH-09 circuit. The confirmed matching
panel positions remove a setting mismatch as an explanation; the actual
above-onset loop gain and limiter behavior remain quantities to investigate.

The model's stronger harmonic holds are visible after matching this relative
carrier level. They substantiate a behavioral mismatch in this comparison,
but do not uniquely establish injection-locking width: changing harmonic
dominance, forced response and nonlinear synchronization can coexist in a mixed
output. No formal lock detector or factory-calibrated hardware reference has
been established. Keep this distinction in any manual figures or explanations.

The user's maintenance warning prevents promoting the specimen's numerical
settings to universal SH-09 targets. Schematic-derived structure, nominal
service checks, parameter sensitivity and held-out ordinary patches remain
necessary. A separate healthy-unit comparison would strengthen a later claim
about the intended factory behavior; it is not required to demonstrate that the
present KB-09 equations can exhibit unusually strong interaction in these tests.

## Reproduction and artifacts

```
python Tests/Experiments/analyze_sh09_level_matrix.py --input "C:/Users/kevin/OneDrive/Desktop/SH-09 Test #2.wav"
python Tests/Experiments/match_hardware_level_matrix.py
python Tests/Experiments/match_hardware_level_matrix.py --fit-carrier
```

Analysis dependencies are isolated in `.tmp/dsp-analysis-packages`; MSVC builds
only the offline native diagnostic. Output directory:
`artifacts/AnalogInvestigation/HardwareLevelMatrix`.

- `analysis.json`: source hash, annotations, hardware metrics and fit limitations.
- `hardware-level-stability.png`: shared dBFS axis with unchanged recorded gain.
- `hardware-three-sweeps.png`: actual output spectra at all three positions.
- `isolated-filter-fit.png`: measured free trajectory and two descriptions.
- `CurrentModel/model-comparison.json` and `model-comparison-fitted.json`: all
  calibrations, source hashes, gain/drive fits, metrics and assumptions.
- `CurrentModel/hardware-model-spectra.png`: hardware versus original and
  relative-level-matched model. Approximate phase alignment; dotted harmonic
  guides are not asserted locking intervals.

Figures are draft research artifacts. No published manual, sonic release,
installation, update feed or distribution folder is changed.
