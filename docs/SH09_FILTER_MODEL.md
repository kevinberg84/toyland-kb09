# SH-09 filter model: evidence, assumptions and verification

> Historical candidate document, **not a description of the released 1.5.2
> engine**. That engine uses the restored normalized four-stage model; see
> `Source/DSP/Engine.cpp` and `VCF_STEPPING_INVESTIGATION_2026_10_13.md`.
> Schematic reinspection correction (13 October): C17–20 are marked 1500 pF
> (1.5 nF). Further path tracing: R79 (58 kOhm) belongs to the cutoff-control
> circuit, **not the first audio-input attenuation network**. Audio reaches
> IC11 through C16 from R93/R95/R96; resonance returns to the opposite input
> through R109. Prior first-stage attenuation assignments are obsolete.
> The candidate's chosen device scales, bias and
> aging parameters are unmeasured assumptions and must not be reused as
> hardware-calibrated facts in new modeling work or manuals.

10 October 2026. DSP evaluation candidate 1.2.0; preceding released DSP 1.1.3 is preserved in `Tests/ReferenceDSP113`. This document describes a reduced, circuit-informed model, not a transistor-level recreation or a measured match to Kevin's SH-09.

## What the original circuit establishes

The [Roland service notes, 27 March 1980](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf), printed pages 5–6, show four selected BA662 stages (IC11–14), transistor buffers Q15–18, and four 150 pF integrating capacitors. The first input uses 100 kΩ/1 kΩ attenuation; subsequent inputs use 68 kΩ/1 kΩ networks, with 100 kΩ local feedback resistors. The resonance return includes VR111, R119 (56 kΩ), opposing D12/D13, R118 (1.5 kΩ) and R109 (12 kΩ). This is an OTA filter with a nonlinear resonance return, not a transistor ladder or an independently mixed oscillator.

IC15 is another BA662 in the VCA. Coupling capacitors and the subsequent volume/output circuitry also matter. The headphone amplifier is a separate path: feeding its output back to EXT is an additional external routing technique, not the normal filter circuit. This revision does not add that routing.

The service procedure uses no-input self-oscillation, including a roughly 50 Hz check when selecting replacement filter OTAs, and octave tracking adjustment. Those procedures support testing low-frequency startup and tracking; they do not specify a universal distortion percentage or recording level.

## Model and explicit approximations

The original software already oscillated through its four-stage feedback loop. There was no hidden sine generator to replace. Its common stage saturation scale and single feedback soft clip were replaced with separately parameterized OTA and diode behavior.

The OTA differential-pair law is based on `Iout = Iabc * tanh(Vdiff / (2 VT))`; see [Bogason and Werner, DAFx 2017](https://www.dafx.de/paper-archive/2017/papers/DAFx17_paper_89.pdf). This is a general OTA result, not a measured BA662 transfer curve. The software retains the existing normalized, unity small-signal pole responses and cutoff control law. It uses the documented input attenuation to give the first stage a different nonlinear scale from the following stages. It does not solve every resistor, buffer impedance, capacitor tolerance, OTA bias mirror, parasitic capacitance or transistor in the schematic.

For stage `j`, the trapezoidal residual is:

```
r[j] = y[j] - state[j]
       - g * scale[j] * (tanh((input[j]-y[j])/scale[j] + bias[j])
                        - tanh(bias[j])) / (1-tanh(bias[j])^2)
g = tan(pi * cutoff / internalSampleRate)
```

Subtracting the bias operating point avoids an artificial DC current; normalizing the local slope preserves the original low-level gain. The biases provide bounded even-harmonic asymmetry. The normalization and retained pole gains are reductions, not a complete netlist solution.

The opposing diodes are a fifth unknown in the same Newton solve:

```
rD = v + Rth*IsPositive*(exp(v/nVT)-1)
       - Rth*IsNegative*(exp(-v/nVT)-1) - drive
```

`Rth ≈ 1.302 kΩ` represents the maximum-resonance resistance network. The return normalization uses `100/12`; the pre-existing small-signal loop gain is retained at `4.65 * resonance`. Pot taper and its small resistance variation are not a full component simulation. The selected `Is = 2.5 nA`, `nVT = 45 mV`, and `1.5 circuit volts / engine unit` are disclosed engineering assumptions. They have **not** been fitted to the original diodes or hardware voltage measurements. The [ROHM 1S2473 datasheet](https://datasheet4u.com/pdf-down/1/S/2/1S2473_Rohm.pdf) documents a current-dependent silicon-diode response; it does not justify treating a diode as an ideal fixed 0.6 V clipper or claiming our chosen parameters are an original-part fit.

The downstream VCA/output remains a lumped saturating, AC-coupled model, with normalized offset and headroom. It is not a BA662 transistor macro-model or a fitted simulation of the output buffer/headphone stage. Existing low-level filter seed noise is unchanged; no new noise layer, waveshaper effect or separate oscillator was added.

## AGE and real-time implementation

AGE is normalized internally to 0–1 (the GUI displays 0–10). The new condition terms use `wear = AGE^4`, layered smoothly onto existing tuning/resonance/envelope changes. At maximum condition, OTA scale decreases 14%, the chosen diode strength increases 40%, diode mismatch increases from 1.2% to 13.2%, and lumped VCA headroom decreases 18%. Small deterministic offsets increase continuously. These magnitudes are creative condition assumptions, not a claim that surviving SH-09 components inevitably drift this way. AGE 0 already uses nonlinear OTA and diode behavior.

All five unknowns are solved together with analytic derivatives, at most 12 damped Newton iterations per internal sample. The audio path uses fixed storage, no file I/O, locks or dynamic allocation. Nonfinite/excessive states are guarded. Oversampling increased from 4× to 8× after high-cutoff stress exposed unwanted nonharmonic energy at 4×. FIR length and control alignment scale with the oversampling factor, preserving the reported **128 host-sample latency**. No parameter IDs, order, ranges, state schema, preset format, GUI or routing changed.

Saved sounds remain loadable, but the intended resonance correction changes their sound: stronger self-oscillation and a different large-signal pitch at identical cutoff settings. This is not bit-identical sonic compatibility. Absolute cutoff calibration still needs a controlled hardware comparison; octave tracking alone does not prove it.

## Reproduction and evidence

Build `SH09FilterCharacter` and `SH09FilterNonlinearTests` with the existing Release configuration. The reference implementation is linked only into the offline measurement executable, never the plug-in.

```
SH09FilterCharacter baseline artifacts/FilterCharacter/final full
SH09FilterCharacter current artifacts/FilterCharacter/final full
SH09FilterCharacter current artifacts/FilterCharacter/limits limits
python tools/analyze_filter_character.py artifacts/FilterCharacter/final
python tools/make_filter_auditions.py artifacts/FilterCharacter/final
```

The full sweep covers 44.1/48/96/192 kHz; cutoff, resonance onset and AGE 0/5/10; low-frequency startup; octave tracking; and partitions of 1/32/64/127/512/1,024 samples. The limits sweep adds near-maximum cutoff at every rate. Captures settle for at least 3 seconds and 320 nominal cycles, then measure one second. Measurements and validation counts are recorded in `artifacts/FilterCharacter/RESULTS.md` and its referenced logs. CPU figures are wall-render time divided by audio duration on this workstation, not a DAW-wide CPU percentage or a guarantee on another machine.

Harmonics use frequency-refined projections with a four-term Blackman–Harris window. The reported nonharmonic residual excludes DC and harmonic bins; it includes aliasing, other nonharmonic energy and the analysis floor. It is an artifact indicator, **not an isolated alias-power measurement**. Harmonic percentages near Nyquist omit out-of-band harmonics and must not be compared directly with low-frequency THD.

## Hardware comparison and remaining work

An author-published original SH-09 filter sweep is available in [Schlüenzen/Geisel's Bonedo article](https://www.bonedo.de/artikel/vintage-synth-roland-sh-09/). Its mixer levels, resonance position, capture gain/load and servicing condition are undocumented. We measured a cycle-aligned late low-frequency portion for a rough harmonic-order check. A sweep can contain modulation and other sources; this does not establish the distortion of an isolated, stationary self-oscillation or calibrate our model. The downloaded recording is retained only as research provenance, not included in the product or our listening clips. No hardware-level listening match is claimed.

For a defensible fit, record Kevin's unit with VCO/SUB/EXT at zero, VCA HOLD, VCF MOD/ENV zero and known key-follow note, resonance and cutoff positions. Capture several steady pitches (including approximately 50/100/440/1,000/5,000 Hz), the resonance onset, known line-output loading and recording gain, plus a separate clean oscillator-through-filter sweep. Measure physical output voltage where possible. Those captures can constrain voltage normalization, diode parameters, OTA stage behavior and VCA/output headroom. Without them, “circuit-informed nonlinear feedback” is supportable manual wording; “indistinguishable,” “component accurate” or “hardware calibrated” is not.
