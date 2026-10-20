> **Historical record.** Version numbers, proposals, pending statuses and test results below refer to the revision/date described, not the current release. See [1.9.0 validation](VALIDATION_1.9.0.md) and [current sound-model evidence](FIDELITY.md).

# Analog character investigation — October 11, 2026

Status: research and offline diagnosis; no published DSP or release changes.

The user reports thin, harsh, gargling saw/pulse output with both the restored
1.1.3 engine and the newer filter model. This is not yet reproduced conclusively.
Switching between those filters again without testing their shared paths is not
justified by the evidence. Preserve public 1.3.0 while investigating.

## Research read

* Pekonen et al., DAFx 2010, *On Minimizing the Look-up Table Size in
  Quasi-Bandlimited Classical Waveform Oscillators*: fractional positioning and
  interpolation of discontinuity corrections matter. Short correction functions
  trade cost against audible alias rejection. Ideal geometric waves and measured
  analog waveform shapes are separate calibration targets.
  https://www.dafx.de/paper-archive/2010/DAFx10/PekonenNamAbelSmithValimaki_DAFx10_P34.pdf
* Pekonen, 2014 thesis overview: includes measured analog oscillator waveshape
  modeling and perceptual aliasing criteria. The full thesis download was not
  accessible through the browsing tool in this session; do not claim it was read
  in full.
  https://research.aalto.fi/en/publications/filter-based-oscillator-algorithms-for-virtual-analog-synthesis/
* Holters, DAFx 2019, *Antiderivative Antialiasing for Stateful Systems*:
  nonlinear processing can create folded harmonics after oscillator generation.
  Naive insertion of ordinary ADAA into a feedback loop adds delay and changes
  dynamics. Use a derived stateful formulation rather than a drop-in shaper.
  https://www.dafx.de/paper-archive/2019/DAFx2019_paper_4.pdf
* Köper et al., DAFx 2022, *A Virtual Analog Model of the EDP Wasp VCF*:
  measured OTA rail behavior changes high-resonance trajectories. Device models
  must be used within the measured operating regime. This supports the method,
  NOT transferring Wasp/CA3080 coefficients to an SH-09/BA662 circuit.
  https://www.dafx.de/paper-archive/2022/papers/DAFx20in22_paper_34.pdf
* Reinspected local SH-09 VCO schematic enlargement and existing primary-source
  trace in Research/SH09_RESEARCH.md. Ramp integration/reset, comparator, divider,
  and mixer gains still require calibrated waveform/level measurements.

The original Native Instruments URL for Zavalishin's filter book returned 404.
Do not claim the book was read from that URL.

## Completed experiment

`Tests/Experiments/rate_convergence.py` generates isolated copies of the released
engine at 4/8/16/32 times host rate. FIR lengths, normalized cutoff, alignment,
and external-input polyphase interpolation all scale consistently. Host latency
remains 128 samples. These copies are offline diagnostics, not shipping options.

72 two-second renders: 44.1/48/96 kHz; steady saw, steady square, 25 Hz PWM,
resonant bass with sub, strong filter modulation, strong pitch modulation.
All outputs finite; zero solver failures in the logged runs. A separate baseline
check compared the generated 4x copy with actual released Engine.cpp:
432,000 samples, zero mismatches, including external input.

54 comparisons against 32x are saved in
`artifacts/AnalogInvestigation/RateConvergence/convergence.json`.
For example, steady saw at 44.1 kHz differs from 32x by -32.21 dB at 4x,
-43.23 dB at 8x, and -55.18 dB at 16x. These are total waveform differences,
including phase/desired harmonic changes; they are NOT aliasing measurements.
Equal latency does not remove differences in frequency-dependent group delay.

Static nonharmonic spectral residuals use a Hann window and exclude eight FFT
bins around each expected harmonic. At 44.1 kHz, 4x saw/square residuals were
-62.94/-64.51 dB; 32x yielded -69.83/-71.25 dB. This measurement includes window
leakage and noise; harmonic exclusions can conceal coincident aliases. It neither
establishes perceptual transparency nor reproduces the user's severe complaint.
Do not use this metric for modulated cases: modulation creates legitimate sidebands.

Performance.csv records wall processing time, not DAW CPU load. Some 32x/96 kHz
runs exceed one CPU-second per audio second on this workstation; 32x is a costly
offline reference, not a proposed realtime default. No physical hardware A/B yet.

Five WAV comparisons have identical gain and no normalization: current 4x first,
250 ms silence, then 32x, each 1.7 seconds. These assess numerical differences,
not competing hardware-calibrated voices or a claimed fix.

## Plugindoctor

DDMF documents harmonic analysis, IMD, response, scope, and performance tools,
plus a free demo. Its published loading support specifies effect plugins. Check
instrument compatibility before buying; alternatively analyze the external-input
path or route the instrument's output appropriately. Oscillator captures and
offline tests remain necessary.

On 2026-10-11, downloaded the official Windows demo 2.7.2 and ran its extracted
64-bit standalone. Its camera resource had to be placed in the expected
ProgramData/DDMF/PluginDoctor directory before startup succeeded. The full
installer was not completed. Disabled background scanning in the demo session
and selected the installed release 1.3.0 VST3 binary through its file chooser.
Plugindoctor explicitly rejected it: "Instrument plugin" / "Can only process
effect plugins". No KB-09 measurement was obtained in Plugindoctor, and no
purchase or shipping plug-in/DSP change was made. Demo files remain isolated
under artifacts/AnalogInvestigation/PluginDoctor and must not enter release
packages. Offline render measurements above are independent of Plugindoctor.
https://www.ddmf.eu/plugindoctor/
https://ddmf.eu/pdfmanuals/PlugindoctorManual.pdf

## Outstanding diagnosis

### Stage isolation and first candidate (2026-10-11)

`Tests/Experiments/stage_isolation.py` creates diagnostic copies with separate
post-decimation source, mixer, filter and output taps. 112 steady renders cover
seven pitches (approximately 65–4187 Hz), saw/square, 4x/32x internal rates,
48 kHz host rate, AGE zero, open cutoff, zero resonance and modulation. All
were finite; the original run reported zero solver failures. Source hashes
match the production checkout at C:/Dev/SH-09. No production source was edited.

The current memoryless saturation is monotonic (its analytic derivative is
nonnegative); it is not literally a wavefolder. It can still create aliasing.
The mixer tap has more nonharmonic energy than the source tap in these tests.
The filter adds some, while the final amplifier adds little at this patch level.
This is evidence to investigate shared nonlinear paths, not proof of the cause
of the user's severe perceived gargling.

An additional 28 steady renders compare a linear mixer diagnostic and a
first-order antiderivative antialiasing (ADAA) mixer candidate. The candidate
retains the same static saturation curve and uses its analytically derived
primitive; it is applied only outside the filter feedback loop.

| Saw pitch (MIDI) | Current residual dB | Candidate residual dB | RMS change dB |
|---|---:|---:|---:|
| 60 | -70.76 | -81.28 | -0.0015 |
| 84 | -64.32 | -74.58 | -0.0068 |
| 96 | -60.96 | -71.25 | -0.0145 |

Metric: Hann-windowed nonharmonic-bin power / harmonic-bin power, excluding
DC and three bins on either side of each harmonic. It includes leakage and
can hide aliases coinciding with harmonics. It is not a direct perceptual score.
ADAA also adds approximately half an internal sample of delay and changes HF
response; similar RMS does not prove identical desired harmonic amplitudes.
No automatic inverse EQ has been added. Character must be assessed separately.

`Tests/Experiments/mixer_candidate.py` exercised baseline/candidate at
44.1/48/96 kHz, eight cases, three seconds each (48 renders): saw, square,
fast PWM, sub/resonant bass, strong VCF modulation, strong VCO modulation,
maximum AGE/resonance, and self-oscillation. All finite, zero reported solver
failures. A numerical derivative check of the primitive had maximum absolute
error 7.19e-11. Median offline processing ratios were 0.08778 baseline and
0.08964 candidate (about 2.1% relative increase); this is a sequential single
run, not a rigorous benchmark or a DAW CPU guarantee. The experiment does not
exercise every automation, buffer-size, external-input or note-release case.

The separately loaded released VST3 passed 73 host assertions, including the
existing rate/buffer matrix. Its log is
`artifacts/AnalogInvestigation/released-host-check.txt`. These host tests do
not validate the experimental candidate and do not establish sound quality.

Artifacts under `artifacts/AnalogInvestigation/StageIsolation`: measurements.json,
candidate-stability.json, candidate-performance.csv, primitive-check.json and
six unnormalized same-gain `*-current-then-candidate.wav` comparisons (current
first, 250 ms silence, candidate second). No human listening result is claimed.
The recording protocol is docs/SH09_RECORDING_GUIDE.md. Hardware recordings and
reproduction of the user's specific bad patch are still outstanding.

### Additional engineering practice research

u-he's *RePro Filters Unveiled* describes measuring hardware bandwidth,
modulation, harmonic distortion, DC offset, cutoff bleed and self-oscillation
level/spectrum, then comparing five numerical methods for the same equations.
It documents why delayed/approximate nonlinear feedback can sound different,
and why louder/more distorted is not necessarily more faithful. This is a
methodological reference, not permission to import CEM3320 coefficients into
the SH-09 OTA filter.
https://www.u-he.com/downloads/UrsBlog/RePro_Filters_Unveiled.pdf

La Pastina and D'Angelo (DAFx 2022), *Antiderivative Antialiasing with Frequency
Compensation for Stateful Systems*, sections 1–3: ordinary ADAA has a non-flat
linearized response and delay; inserting it into feedback can affect stability.
The paper derives a compensated AA-IIR approach with minimum-phase constraints.
This reinforces testing the isolated feed-forward mixer first; no AA-IIR
feedback implementation or complete-paper implementation is claimed.
https://www.dafx.de/paper-archive/2022/papers/DAFx20in22_paper_4.pdf

Other tools considered: REW's spectrum/RTA and harmonic measurements are useful
for controlled input/output hardware tests; Sonic Visualiser provides detailed
spectrograms of recordings. Neither was installed or used for measurements in
this session. Current quantitative work uses the reproducible C++ renders and
NumPy FFT analysis. Plug-in validators cannot establish analog fidelity.
https://www.roomeqwizard.com/help/help_en-GB/html/spectrum.html
https://www.sonicvisualiser.org/doc/reference/5.0.1/en/

There is no single subjective "analog sound" pass/fail standard. Acceptance
should combine measured waveform/level/transfer/oscillation behavior, bounded
numerical error, low aliasing, real-time robustness and controlled listening.
Preserve actual harmonic character rather than removing all distortion.

User was asked which preset, AGE, notes, and whether SUB/EXT zero reproduces the
artifact. Await this to reproduce actual conditions. Inspect actual plugin/host
output alongside offline engine output; distinguish aliasing from expected FM,
nonlinear resonance intermodulation, duplicate monitoring, and output clipping.
The SH-09 has one master VCO and a locked sub divider, not two detuned VCOs.

For hardware calibration, request dry steady saw/square with SUB/EXT zero,
resonance minimum, filter fully open, no modulation/effects/normalization, several
low/mid/high notes, and documented interface gain. Separate level sweeps,
resonance sweeps and self-oscillation recordings then calibrate mixer headroom,
OTA nonlinearities and the diode limiter. Keep raw recordings and exact settings.
Do not substitute arbitrary warmth EQ, added noise or generic distortion for
physical calibration. No shipping model has been selected or changed yet.


## Slow VCO / self-oscillation interaction — 11 October 2026

Kevin narrowed the reported bubbling to relative pitch motion between a steady VCO and the self-oscillating filter, or the reverse. `Tests/Experiments/slow_pitch_interaction.py` exercises both directions with saw and square, resonance at maximum, VCO level 0.5, filter control 0.449, A4, a 0.2 Hz sine LFO, and approximately ±0.96-semitone motion. AGE 0 and 10 are tested at 48/96 kHz through independent 4/8/16/32× laboratory engines. Each render lasts 12 seconds. The copies retain equal host latency and reconstruction-filter bandwidth; the shipping audio path is untouched by this experiment.

All 64 renders were finite, with zero solver failures and peak magnitude at most 0.23955. At AGE 0, the 4× vs 32× time-domain difference is −76.7 to −63.5 dB relative to output RMS. The amplitude-envelope P90/P10 ratio is approximately 1.32–1.35 in the 48 kHz healthy cases and remains effectively unchanged with oversampling. This supports genuine modulation/intermodulation in this model as the dominant interaction in these settings, rather than a large foldback artifact cured simply by increasing the internal sample rate. It does not establish that the hardware behaves identically, and does not rule out aliasing in other patches.

At maximum AGE, saw interactions have greater phase-trajectory sensitivity: time-domain differences can reach −16.3 dB despite nearly identical RMS and envelope distributions. This must not be reported as an alias-only result. Square interactions converge more closely. Further calibration needs the same slow-relative-pitch experiments recorded from the physical SH-09; a recording from a badly drifting instrument is useful evidence but cannot establish factory tuning.

Median offline CPU ratios were 0.127, 0.243, 0.653 and 0.973 for 4/8/16/32× across this matrix. These are workload timings on this workstation, not cross-machine requirements or clean real-time CPU benchmarks. Four same-gain auditions under `artifacts/AnalogInvestigation/SlowInteraction` place 4× first and 32× second, separated by half a second. No listening approval or hardware match is claimed.
