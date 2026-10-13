# Analog comparison figures — working plan, not published manual content

13 October 2026. Hardware recordings are pending. No hardware/model comparison
results or claims of exaggerated locking are established. Kevin approved the
GUI; sound work must preserve the current production DSP baseline until a
specific improvement is supported.

Update: 22 recordings and baseline photographs have now been supplied. See
`../SH09_RECORDED_COMPARISONS_FIRST_PASS.md` for the measured output-level
differences and the approximate range-matching diagnostic. Initial draft
measurement figures are under `artifacts/AnalogInvestigation/HardwareComparisons`.
Precise hardware/model lock-width calibration and a selected sonic fix remain
outstanding; the original planning text below describes the acceptance criteria.

Each manual figure should answer one plain-language question, label actual
measurements and uncertainty, and use an understandable caption. Keep the
solver/oversampling study in an optional technical appendix. Publication is
separate from this preparation; do not automatically add draft conclusions to
the shipped manuals or update feed.

## Candidate figures, selected only after the recordings

| Reader's question | Figure | Conditions for using it |
|---|---|---|
| Does the filter glide smoothly? | Two aligned pitch-versus-time panels: SH-09 and KB-09, filter alone | Use a validated single-component estimator; mark gaps/uncertainty; show one rising and falling cycle |
| Why do I hear steps when the VCO is added? | Two simple spectrograms with a few labeled fixed VCO harmonics and any independently visible moving component | Explain brightness = strength; do not present the loudest spectral ridge as the filter's pitch |
| Does turning up the VCO change the interaction? | A few matched panels or a level-versus-observed-interaction summary | Define an interaction metric only after the mechanism is established; do not call every near-harmonic interval a lock |
| How close are the tone shapes? | First six/eight harmonic bars relative to the fundamental, hardware beside model | Stationary references only; state any display gain adjustment; preserve raw level measurements separately |
| What changed after a verified fix? | Hardware / released baseline / candidate on identical axes, plus a short same-level listening example | Include only if the candidate improves hardware agreement and normal patches still pass listening/regression checks |

Avoid a dense multi-line chart with several DSP rates in the main explanation.
Directly label hardware/model lines, use distinct line styles as well as color,
large type, shared axes and Hz labels (note names optional). Include enough
context to prevent a cropped interval from implying the whole sweep behaves
the same way. Ratio-to-VCO-frequency plots can help the technical analysis when
tuning differs, but should not replace actual-Hz measurements silently.

Every measurement figure should retain source filenames/hashes, instrument
version/condition, sample rate, actual pitch/waveform, settings, analysis method
and display transformations in an accompanying provenance record. Original
audio is immutable. Align comparable LFO phases for display if necessary and
disclose that alignment; never silently time-stretch audio to force agreement.

## Analysis safeguards

The earlier zero-crossing chart concerns the combined filter output. It does
not establish the entire staircase or prove exaggerated injection locking.
For mixtures, preserve a time-frequency view of multiple components, examine
phase coherence where those components are separable, and mark intervals where
the sources cannot be distinguished. A component hidden under another cannot
be confidently assigned a continuous pitch curve.

Validate the workflow against known signals: a smooth isolated chirp, the same
chirp plus fixed harmonics whose strengths change, and deliberately stepped
tones. The first two must not be mislabeled as a quantized oscillator merely
because a dominant-frequency or zero-crossing estimate jumps. Synthetic
examples are labeled illustrations/self-tests, never measured synth evidence.

Executed preparation: `Tests/Experiments/analysis_sanity.py` constructs all three
known cases. At 48 kHz, refined dominant-peak errors at the 99th percentile are
about 0.00168% for the isolated chirp and 0.00123% for deliberately stepped
tones away from transitions. The unchanged chirp is recovered from the
constructed mixture to floating-point precision. The mixture's strongest peak
visibly hops between stationary harmonics even though the chirp is unchanged.
Artifacts and the clearly labeled illustrative figure are under
`artifacts/AnalogInvestigation/AnalysisSanity/`. This verifies those narrow
safeguards, not a general source separator, perceptual pitch estimator or
injection-locking detector. No synth/hardware claim follows from this example.

Do not fit the model and evaluate its success exclusively on the same patch.
Reserve other mixer levels, waveform choices, notes and opposite sweep
directions for checks. Distinguish current-unit calibration and wear from the
intended circuit; one old SH-09 is not a population sample.

## Research informing preparation

- [Holmes / van Walstijn, DAFx 2019](https://dafx.de/paper-archive/2019/DAFx2019_paper_35.pdf):
  ideal logarithmic/linear control laws can differ from manufactured tapers;
  input/output measurements can identify control behavior without disassembly.
  Application here is methodological, not an imported Big Muff control law.
- [Esqueda et al., DAFx 2021](https://dafx.de/paper-archive/2021/proceedings/papers/DAFx20in21_paper_39.pdf),
  sections 2–3: circuit parameters can be fit to measured behavior, but multiple
  component combinations can give the same response. Constrain the physical
  topology and use several informative operating conditions; do not claim
  unique component identification from one audio clip.
- [Köper et al., DAFx 2022](https://www.dafx.de/paper-archive/2022/papers/DAFx20in22_paper_34.pdf),
  sections 3–5: OTA output operating range can matter greatly at high resonance;
  simple tanh and rail-aware models differ in that particular Wasp circuit.
  This motivates checking operating range, not importing CA3080 rail models or
  assuming the SH-09 reaches its rails.
- [u-he, RePro Filters Unveiled](https://www.u-he.com/downloads/UrsBlog/RePro_Filters_Unveiled.pdf):
  distinguish equation/model error from numerical-solver error, and distinguish
  louder/more distorted from closer hardware agreement. Their CEM3320 results
  are not SH-09 coefficients or a prescribed oversampling rate for this project.

No model training, new component solver, hardware parameter fit, or production
sonic change is claimed by this preparation.
