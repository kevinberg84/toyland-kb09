# First supplied SH-09 / KB-09 recordings

Status: measured recording differences and an offline control-matching check.
Production DSP is unchanged. No component-model candidate, compressor, level
compensation, or separate sine oscillator has been implemented or selected.

Follow-up: `SH09_DSP_RESEARCH_DECISION_2026_10_13.md` combines the primary-source
research with twenty new one-factor offline renders. The dips are present before
the final output shaper and persist at 16x internal processing; halving input
drive strongly reduces them in the approximate pair-1 case. This is a diagnostic
lead, not a selected gain change or a calibrated excess-lock-width measurement.

## Inputs and provenance

Kevin supplied 22 WAV files under `C:/Users/kevin/OneDrive/Desktop/KB-09 Test`:
pairs 1–8 named Sweep, pairs 9–11 named VCO. He reports trying to match pitch,
modulation depth and RATE. The baseline photos show VCO mixer maximum, SUB/EXT
zero and VCA ENV on both; KB-09 is Minty / **Modern**. He says VCF MOD, pitch,
FREQ and RES were changed for the recordings. Precise per-pair settings,
capture/host gain and note timing have not yet been supplied. Do not relabel
these as a VCO-mixer-level matrix or as an Authentic-mode hardware calibration.

All files decode as 96 kHz, 32-bit IEEE float, two near-identical channels.
Analysis averages those channels; measured correlations exceed .99999. All
samples are finite; none has absolute magnitude >= 1. This does not establish
the absence of upstream analog/host limiting. Source files were only read.
No normalization, resampling or edits were applied to the original recordings.
Analysis spectrograms use an explicitly downsampled 48 kHz copy.

`artifacts/AnalogInvestigation/HardwareComparisons/recording-manifest.json`
records original paths, SHA-256 hashes, formats, durations, channel handling,
levels and descriptive spectral peaks. Pair 3 has unequal lengths (16 s KB,
17.44 s SH). Pair 8 includes a note end / trailing quiet portion. Do not treat
trailing silence or attacks as continuous filter behavior.

## Confirmed output-level difference

The table reports P90 minus P10 of 50 ms RMS levels in dB: the range containing
the middle 80% of short-window levels. A fixed one second is excluded at each
file edge, while internal dips remain. This is neither peak-to-peak amplitude
nor perceptual loudness/LUFS. Constant recording gain changes do not alter this
within-file spread; other processing or different patches can.

| Pair | SH-09 level range, dB | KB-09 level range, dB |
|---:|---:|---:|
| 1 | 0.72 | 8.85 |
| 2 | 0.70 | 4.83 |
| 3 | 0.67 | 5.16 |
| 4 | 0.42 | 8.88 |
| 5 | 0.47 | 9.64 |
| 6 | 0.55 | 10.71 |
| 7 | 2.89 | 9.83 |
| 8 | 1.77 | 12.07 |
| 9, VCO | 0.22 | 0.22 |
| 10, VCO | 0.26 | 0.25 |
| 11, VCO | 0.34 | 0.35 |

The plain VCO references are steady in both instruments, whereas KB-09 sweeps
show deep recurring dips. A longer-window check preserves the difference:
pair 1 at 250 ms gives 0.57 dB SH versus 5.32 dB KB; at 500 ms, 0.36 versus
3.25 dB. Longer windows average away short dips. At very low sine frequencies,
50 ms contains few periods, so short-window RMS alone can overstate envelope
motion; the 250/500 ms crosscheck is especially relevant to pairs 7–8.

References 9–11 have close measured pitches:

| Pair | SH-09 fundamental, Hz | KB-09 fundamental, Hz |
|---:|---:|---:|
| 9 | 271.550 | 270.439 |
| 10 | 228.066 | 227.411 |
| 11 | 170.967 | 170.365 |

These are approximately 5–7 cents apart. They are adequate for useful spectral
comparison without pitch shifting. Their absolute recorded levels differ by
about 6.8 dB; that is not, by itself, a claim about internal filter input drive.
The references here appear to be different pitches, not different waveforms or
different mixer levels. Confirm the user's pair descriptions before assigning
specific key/RANGE/taper values or drawing waveform-to-waveform conclusions.

## Staircase and VCO-frequency content

The paired spectrograms clearly show a much more pronounced sequence of held
upper frequencies and transitions in KB-09, while SH-09's prominent swept
component is smoother. This is the user's clarified staircase, not merely the
earlier single-region zero-crossing illustration. Hardware also has short
near-harmonic regions and interaction products; it is not claimed perfectly
linear or free of input interaction.

Differences in sweep extent and rate remain important. The VCO-frequency band
gets quieter as the filter goes sufficiently low in **both** instruments.
That is compatible with ordinary low-pass attenuation. The hardware can retain
much steadier total output while its own oscillation sweeps. Avoid bypassing
the filter or adding inverse gain/AGC merely to keep the VCO audible.

`recording_carrier_levels.py` uses annotated stationary harmonic families and
a 10 Hz complex-demodulation band to quantify total signal near the VCO pitch.
This is **not isolated VCO volume**: self-oscillation and intermodulation can
enter the same band. Pair 5's actual family near 256 Hz is absent from the
three VCO references, so it must not be forced onto a reference pitch or a loud
subharmonic. Carrier-level results are descriptive, not lock classifications.

## Approximate range-matching diagnostic

For pair 1, a robust sinusoid fit in log-frequency to the **dominant upper
spectral component** estimates:

| Recording | Lower fitted frequency | Upper fitted frequency | Fitted rate |
|---|---:|---:|---:|
| SH-09 | 440.7 Hz | 868.9 Hz | 0.17474 Hz |
| KB-09 | 207.2 Hz | 912.7 Hz | 0.15246 Hz |

These are descriptive fits to a mixed output, not measurements of cutoff CV
or a separately captured free-running hardware filter. Harmonic capture and
transitions distort the KB fit more strongly. In particular, its wider low
excursion must not be ignored when comparing amplitude/locking behavior.

`match_recorded_sweep.py` runs the actual unchanged Engine.cpp at 96 kHz with
Minty, Modern, saw, sustained ENV .7, VCO maximum or zero, SUB/EXT/VCF ENV/VCO
MOD zero. It chooses FREQ/MOD/RATE/TUNE from that SH-09 fit, assuming maximum
model RES and an initial .866 self-oscillation-to-command scaling. Warm-up also
advances the LFO to approximately the fitted phase. This is a diagnostic,
not a claim to have reconstructed the exact physical patch.

The **measured** VCO-off model sweep covers 440.26–867.96 Hz by interpolated
zero-crossing periods, verifying the intended approximate band. Over the same
20-second duration, middle-80% 50 ms level ranges are:

- SH-09 recording: **0.72 dB**.
- Unchanged model, approximately matched band, VCO off: **0.03 dB**.
- Same model/settings with VCO full: **3.44 dB**.

Both offline renders are finite and report zero solver failures. Narrowing the
band reduces modulation markedly relative to the original KB recording's
8.85 dB, but does not remove the residual difference under these assumptions.
That is stronger evidence for investigating the modeled input/filter balance
than the unmatched recordings alone. It is **not yet a calibrated measurement
of excess hardware/model lock width**, because actual resonance positions,
internal drive, isolated hardware oscillation level and device scaling remain
unidentified. A fixed source/feedback-level fit must precede a definitive
component replacement or a global claim about all SH-09s.

## Next focused work

1. Get the per-pair FREQ/RES/MOD descriptions if available; preserve the exact
   bad software preset. A baseline photo alone does not encode all eight cases.
2. At one documented representative patch, capture SH-09 with VCO 0, a middle
   level, and 10 without changing FREQ/RES/MOD or capture gain. Existing VCO-10
   data need not be discarded. The isolated take constrains the filter's own
   amplitude and sweep law; the middle level constrains input sensitivity.
3. Fit control span/rate and relative VCO/self-oscillation level separately.
   Then test the existing first-stage scaling, OTA saturation and resonance
   limiter against hardware, keeping the current engine as baseline.
4. Evaluate explicit lock/phase-coherence behavior only where components can
   be resolved. Do not substitute the loudest FFT peak for a unique filter
   frequency. Use other pitches/sweep directions as independent checks.

No compressor, automatic gain compensation, unfiltered VCO feed, added sine
oscillator or production filter rewrite follows automatically from these data.
The recordings substantiate a concrete behavioral mismatch and provide a
useful target for a controlled offline candidate.

## Reproduction and figures

Run the following with Python plus NumPy/SciPy/Matplotlib, and MSVC for the last
script. Optional analysis libraries are currently isolated in
`.tmp/dsp-analysis-packages`.

```
python Tests/Experiments/analyze_recorded_comparisons.py --input "C:/Users/kevin/OneDrive/Desktop/KB-09 Test"
python Tests/Experiments/recording_carrier_levels.py --input "C:/Users/kevin/OneDrive/Desktop/KB-09 Test"
python Tests/Experiments/rms_window_crosscheck.py
python Tests/Experiments/match_recorded_sweep.py --input "C:/Users/kevin/OneDrive/Desktop/KB-09 Test"
```

Output folder: `artifacts/AnalogInvestigation/HardwareComparisons`.
`pair-01-level-comparison.png` is a readable first figure: separate panels,
shared level-change scale, per-file median centering for display only.
`pair-01-spectra.png` shows the actual frequency content; its brightest ridge
is not labeled as an isolated oscillator. `matched-band-level-check.png` is
explicitly labeled diagnostic, with model assumptions. These are draft
research figures, not yet edits to the published manual or an update notice.
