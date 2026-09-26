# DSP implementation design

Written before core implementation, 2026-09-26. This is a circuit-informed reduced model and not a measured hardware-equivalence claim.

The engine is allocation-free C++20 with fixed arrays, double state, four-times internal processing, and mono output. MIDI is handled externally at sample offsets. Lowest held note wins. Gate+Trigger restarts attack from the existing envelope on effective-note changes; Gate retains legato; LFO retriggers only with held keys. Oscillator and divider remain free-running. Portamento pitch freezes while no key is down.

Primary service-note ranges supplied by the hardware trace: attack 1 ms–2.5 s, decay/release 2 ms–10 s, LFO 0.2–25 Hz, delay 0–1.5 s, portamento 0–5 s, cutoff 10 Hz–20 kHz, tune ±65 cents. Logarithmic slider mappings between endpoints are engineering inferences. Attack is capacitor-style charging toward an over-target threshold; decay/release use exponential segments. Minimum times remain finite.

The main source selects shaped noise, saw, square or pulse. Saw/pulse edges use polynomial BLEP suppression at the internal rate. Sub phase is derived from a shared master modulo-four cycle state, giving f/2 square, f/4 square, or f/4 25% pulse. PWM uses the undelayed sine independently of selected LFO waveform. Audio noise is filtered excitation approximating the documented reverse-junction noise path; its exact spectrum awaits measurement.

External input uses a prepared interpolation FIR. The input trim is neutral in Authentic and optionally ±24 dB in Modern. Follower rectification precedes the external mixer fader, preserving detection when external audio is turned down. A positive rectifier and three smoothing sections approximate the documented active RC ladder (100k, .1uF/.1uF/.033uF); the active network is not claimed to be reproduced exactly. Follower modulation controls VCF only.

The four selected BA662 stages and discrete buffers motivate four nonlinear lowpass cells with instantaneous global feedback. The reduced cell law is tanh of differential input; feedback uses a smooth diode-clamp approximation. All four trapezoidal equations are solved together using a bounded Newton loop and analytic sparse Jacobian. Normalized saturation scales, resonance scaling, drive and output gain are engineering assumptions. No transistor-level BA662 accuracy is claimed. Nonconvergence is counted for tests; it is never silently presented as fidelity.

The resampling filters are symmetric, prepared windowed-sinc FIRs. Synthesis/control frames are delayed to align with interpolated external input. The output is decimated only after antialias filtering. Engine latency is an exact integer, reported by latencySamples(); all paths use the same delay. Quality is fixed to avoid dynamic allocation or latency changes during playback.

Continuous controls smooth over approximately 8 ms; first controls initialize immediately. Discrete source/mode transitions receive a brief bounded output dip. Condition uses a deterministic fixed calibration profile with correlated oscillator/filter/envelope offsets, not per-note random pitch or extra hiss. Age zero is the healthy reference. Numerical bounds, denormal flushing and finite input validation protect real-time operation without claiming that protection mechanisms are hardware behavior.

Tests will cover end-to-end oscillator pitch/ranges/sub, note ordering/repeated notes/legato, envelope behavior, external input and follower before the mixer, self-oscillation, rapid automation, long finite runs, multiple sample rates and resampling latency. Results are recorded separately after execution.
