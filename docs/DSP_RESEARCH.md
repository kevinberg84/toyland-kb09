# Virtual analog DSP research

Research date: 2026-09-26. This document evaluates methods; it is not a claim that any resulting executable has passed the proposed tests. SH-09 topology, component designators, gains, and timing values belong in `SH09_RESEARCH.md`. Implementation choices and deviations belong in `DSP_DESIGN.md`.

Evidence labels: **VERIFIED** means a directly inspected primary publication supports the stated fact. **SUPPORTED** means evidence supports a technique but does not establish SH-09 equivalence. **INFERRED** means a proposed engineering choice. **SPECULATIVE** means a hypothesis requiring additional documentation or measurement.

## Oscillator and divider

**VERIFIED:** Discontinuities in ideal saw/pulse waveforms require alias suppression. Bandlimited impulse/step methods, polynomial approximations, and minimum/nonlinear-phase basis functions are established approaches. Pekonen's thesis reviews their computational and perceptual tradeoffs. A short polynomial correction is an approximation, not an alias-free guarantee. [Pekonen, *Filter-Based Oscillator Algorithms for Virtual Analog Synthesis*, Aalto University, 2014](https://research.aalto.fi/en/publications/filter-based-oscillator-algorithms-for-virtual-analog-synthesis/), [Pekonen and Välimäki, DAFx 2012 companion research](https://pekonen.cc/p/dafx12-nlpo/).

**INFERRED implementation plan:** Maintain a double-precision free-running master phase. Locate the fractional time of each reset and pulse transition, and add a PolyBLEP or precomputed minBLEP residual there. When pulse width changes, the moving falling edge must be treated as an event; correcting only the phase-wrap edge leaves PWM aliasing. Keep the raw oscillator's documented geometry separate from antialiasing so later measurements can adjust ramp curvature, reset shape, pulse polarity, and attenuation.

A sub oscillator must be a logical divider of the same master cycle events. Use persistent divider bits/cycle count, producing modes from those bits according to the SH-09 schematic. Correct every sub transition at its own fractional event time. An independent phasor at f/2 can retain frequency but lose the required phase relationship during frequency/range changes. Never derive the divider from the *bandlimited audio output's* zero crossings: ringing may create false transitions. Divider mode, polarity, and initialization remain dependent on SH-09 evidence.

**INFERRED acceptance tests:** Measure frequency across all MIDI notes/ranges/sample rates; measure sub f/2 and f/4 relationships over millions of master cycles; change range and PWM during held notes; compare alias residual against a much higher rate reference after matched lowpass/downsampling. Test noncoherent frequencies as well as FFT-bin-centered tones. Report residual integrated energy and worst spurious tones, not only a pleasing spectrum image.

## OTA and filter models

**VERIFIED:** A differential OTA commonly admits a saturating current approximation `i = alpha * I_bias * tanh(beta * v_diff / (2*V_T))`. Measured output-rail behavior can make a substantial difference to a circuit's response. The Wasp study demonstrates this for CA3080 parts and cautions against extrapolation beyond the measured operating range. Those measured coefficients and its CMOS/state-variable topology are **not** SH-09 data. [Köper, Holters, Esqueda and Parker, *A Virtual Analog Model of the EDP Wasp VCF*, DAFx 2022, equations 35–41 and conclusions](https://www.dafx.de/paper-archive/2022/papers/DAFx20in22_paper_34.pdf).

**SUPPORTED:** TI's LM13700 documentation demonstrates why bias current, differential input attenuation, linearizing circuitry, and loading matter in OTA applications. It is a useful device-physics reference, not evidence that SH-09 uses LM13700 or that LM13700 is interchangeable with BA662. [TI LM13700 datasheet](https://www.ti.com/lit/ds/symlink/lm13700.pdf).

**INFERRED conditional candidate:** If the SH-09 trace establishes four cascaded OTA lowpass cells with global negative feedback, begin from their continuous-time nodal equations, retaining stage input attenuation and capacitor values. A normalized *approximation* worth testing has per-stage derivative `dy_i/dt = omega_i * V_i * tanh((u_i-y_i)/V_i)` with `u_1 = input-k*y_4` and later `u_i=y_(i-1)`. `V_i` is a fitted normalized saturation scale, not a fabricated semiconductor voltage. Do not call this a BA662 model until pin-level signal paths and relevant operating ranges support the reduction.

For a trapezoidal realization, solve the simultaneous residuals

```
R_i = y_i - s_i - g_i*V_i*tanh((u_i-y_i)/V_i) = 0
g_i = tan(pi*f_i/Fs_internal)
s_i_next = 2*y_i-s_i
```

The linear small-signal reference is `G_i=g_i/(1+g_i)`, `y_i=G_i*u_i+(1-G_i)*s_i`. With equal stages, write `y_4=G^4*u_1+S`, so `u_1=(input-k*S)/(1+k*G^4)`. This analytically solves instantaneous feedback; inserting yesterday's y4 changes the transfer function and self-oscillation tuning. These equations are an engineering derivation for the stated reduced model, not a transcription of the SH-09 schematic.

**VERIFIED method support:** Trapezoidal state-space integration with algebraically solved instantaneous loops is documented by Wishnick. The cited example is an SVF; only the discretization method transfers. Nonlinear delay-free networks can be solved while preserving their interconnection topology, with historical terms separated from instantaneous terms. [Wishnick, *Time-Varying Filters for Musical Applications*, DAFx 2014, section 3](https://www.dafx14.fau.de/papers/dafx14_aaron_wishnick_time_varying_filters_for_.pdf), [Fontana, Rocchesso and Avanzini, DAFx 2004](https://dafx.de/paper-archive/2004/P_113.PDF).

**INFERRED solver policy:** Use a small fixed-size Jacobian and bounded Newton iterations, initialized from the last accepted state or linear solution. Add damping when residual increases; reject nonfinite solutions. A fixed iteration budget bounds real-time work, but a convergence counter must expose failures to tests. Do not silently clamp an unstable filter and call that circuit behavior. Test both numerical residual and audible output under rapid cutoff/resonance automation. The exact Newton budget, error threshold, and fallback require measurement of the implemented model.

**SUPPORTED alternative:** Wave digital formulations and iterative solutions for grouped nonlinearities provide another way to preserve circuit interactions. They are useful if the actual circuit trace makes the direct state-space reduction fragile. [Werner et al., *Resolving Grouped Nonlinearities in Wave Digital Filters Using Iterative Techniques*, DAFx 2016](https://www.dafx.de/paper-archive/2016/dafxpapers/39-DAFx-16_paper_14-PN.pdf).

Huovilainen's Moog ladder paper is useful as an example of deriving embedded nonlinearities from an analog circuit and checking tuning error. Its transistor ladder and Euler implementation must not be substituted for a Roland OTA cascade. [Huovilainen, DAFx 2004](https://www.dafx.de/paper-archive/2004/P_061.PDF).

## Oversampling, saturation, and ADAA

**VERIFIED:** JUCE offers staged 2x/4x/8x/16x oversampling with FIR or polyphase IIR filters. FIR trades latency for linear phase; IIR minimizes latency with phase change near Nyquist. Processing buffers must be prepared, internal DSP must use the increased sample rate, and latency must be reported correctly. [JUCE Oversampling API](https://docs.juce.com/master/classjuce_1_1dsp_1_1Oversampling.html).

**INFERRED:** Oversample the interacting mixer/filter/VCA nonlinear chain together. Generate the oscillator at that internal rate and interpolate external input properly. A zero-order hold plus unfiltered sample decimation is not an adequate quality implementation. Begin evaluating 4x and 8x at 44.1/48 kHz, then use spectral/stability results and CPU measurements to choose the default. Keep quality independent from patch voicing. A real-time quality switch must not allocate/reinitialize while processing; use prepared pipelines or an explicit safe reprepare policy.

**VERIFIED:** ADAA integrates a memoryless nonlinearity over a reconstructed input interval. The near-equal-input case needs a stable limit rather than dividing by a tiny difference. Ordinary first-order ADAA adds a half-sample delay and high-frequency attenuation; blindly inserting it inside feedback changes the system. Stateful variants require deliberate reformulation. [Parker, Zavalishin and Le Bivic, DAFx 2016 author's companion material](https://github.com/julian-parker/DAFX-AntiAliasing), [Holters, *Antiderivative Antialiasing for Stateful Systems*, DAFx 2019](https://www.dafx.de/paper-archive/2019/DAFx2019_paper_4.pdf).

**INFERRED:** ADAA is initially more appropriate for isolated feed-forward saturation than for an unverified VCF feedback modification. Compare it with high-rate reference renders. Do not copy companion code without checking its license; a publicly accessible repository alone is not a license grant.

## Envelope, follower, noise, and sample-rate invariance

**VERIFIED:** Peak, RMS, linear-domain and logarithmic-domain envelope detectors have different behavior; detector attack/release topology matters. Compressor papers supply useful detector equations, but compressor gain-control recommendations are not automatically SH-09 follower designs. [Giannoulis, Massberg and Reiss, *Digital Dynamic Range Compressor Design—A Tutorial and Analysis*, JAES 2012](https://eecs.qmul.ac.uk/~josh/documents/2012/GiannoulisMassbergReiss-dynamicrangecompression-JAES2012.pdf).

**INFERRED starting equations:** For a documented rectifier/RC detector, use its actual rectifier topology and solve the charging/discharging path. A fallback peak follower is `p=abs(x)`, `a=exp(-1/(tau*Fs))`, `e=a*e_prev+(1-a)*p`, choosing distinct charging and discharge constants. This does not establish the original rectifier polarity, threshold, or timing. Test burst response at several frequencies because ripple depends on input period; test recovery from clipped input and silence. State whether displayed times mean a time constant or 10–90% rise/decay. Do not conflate follower modulation of the filter with a follower-controlled VCA unless SH-09 routing supports both.

An RC ADSR likewise needs comparator thresholds, target voltages, and switch logic. An exponential charging curve never reaches its asymptotic target, so crossing a physical threshold must terminate attack. Retigger from the current capacitor-equivalent state when supported; do not reset blindly to zero. Keep gate/trigger events separate from the envelope curve. All coefficients use seconds and the actual processing rate. Smoothing must not blur sample-offset MIDI gates or replace documented portamento.

For noise, use a seeded source followed by the documented shaping network; determine gain and spectrum from hardware before calling it authentic. A PRNG white source is a computational excitation, not a claim that the original output is spectrally white. Preserve seeded behavior for repeatable tests; choose a separate stream from condition variation.

## Condition and calibration

**VERIFIED general physics:** Aging is component-specific. Murata documents logarithmic capacitance reduction in high-dielectric-constant ceramics and explicitly distinguishes temperature-compensating ceramics, which do not share that aging behavior. It does not establish the dielectric types in an SH-09. [Murata capacitor aging FAQ](https://www.murata.com/support/faqs/capacitor/ceramiccapacitor/char/0013).

**INFERRED model:** Separate fixed calibration offsets from slow thermal-like variation. Save a deterministic unit seed; derive a small correlated latent vector for oscillator scale, filter scale, common envelope capacitor scale, resonance calibration and gain. An example family is `theta_j=theta_healthy + age*(b_j + c_j*z_slow)`, with bounded b/z and fixed signs per unit. Shared control-current or capacitor causes should move related stages together. Update slow variation using seconds, not host block counts, and interpolate smoothly. Age zero gives the healthy calibration exactly. Do not add arbitrary fast pitch wobble, dropouts, or extra distortion solely because a control is named Age.

All numerical ranges remain **SPECULATIVE** until tied to component tolerances, service calibration limits, or measured units. Age is an artistic condition continuum, not a prediction that every 45-year-old instrument has a specific drift. Visual wear must use the same saved seed with cached deterministic marks, independent of repaint frequency.

## Measurement workflow and remaining uncertainty

**INFERRED:** Preserve raw WAV references plus sample rate, interface gain, control settings, warmup time, unit serial/revision, and repeat takes. Align/normalize only when the particular comparison permits it; never hide gain errors in saturation tests. Use raw-waveform harmonic ratios, tuned frequency error, small-signal sweeps, stepped resonance/self-oscillation tests, envelope transients, follower bursts, and noise PSD. SPICE against traced components is a useful intermediate comparison but is not a measurement of an actual unit.

Neither the method literature nor another commercial synth establishes SH-09 waveform geometry, mixer headroom, BA662 parameters, exact self-oscillation level, envelope thresholds, or follower timing. Those remain explicit calibration targets. A finite output stress test establishes numerical robustness for the tested conditions, not sonic equivalence.
