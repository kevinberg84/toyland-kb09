# Known limitations and remaining fidelity work

There is no physical SH-09 capture set or validated SPICE reference for this build. No measured hardware equivalence, blind listening test or completed listening A/B is claimed. The public real-unit demonstrations and photographs in SH09_RESEARCH are references, not calibration data.

* The four nonlinear OTA cells and bounded instantaneous feedback solver are informed by the four-BA662 schematic. They normalize stages and approximate saturation/diode feedback; they do not solve the full transistor/op-amp network. Exact self-oscillation level, tuning, stage interaction and drive require measurement.
* The follower preserves the pre-mixer tap and VCF route but uses a positive rectifier plus three smoothing sections. The original active RC feedback network, diode threshold, input sensitivity and absolute volts-to-dBFS relationship are not exactly modeled.
* Oscillator waveform curvature, PWM edge behavior, mixer source levels, noise spectrum, VCA/output coloration and control tapers have not been fitted to real hardware. Published endpoint ranges guide the control maps. The VCO/VCF bender maxima are engineering choices.
* Condition/Age is an optional deterministic tolerance profile, not a measured age model. Default zero is calibrated and stable; no claim is made about a particular historical unit.
* The GUI is a functional scalable vector interpretation with dimensional shading and deterministic wear. It does not meet a strict photorealistic reproduction requirement, and selector rendering differs from physical switch hardware.
* Fixed 4x oversampling reports 128 samples of latency. CPU/latency tradeoffs are fixed; there is no quality selector. MIDI is sample-offset aware; ordinary host parameter changes are smoothed from JUCE block updates.
* The external bus accepts mono/stereo and averages stereo to mono. This matches a mono instrument architecture but is not a stereo effects processor. External-input trim operates only in Modern mode.
* Duplicate MIDI note-ons use reference counts. MIDI sustain pedal, per-note expressive protocols, polyphony, unison, MIDI Learn, alternate tunings and effects are not implemented. These are optional extensions, not hidden Authentic changes.
* State stores patch/branding/program parameters, not live held keys, oscillator phase, filter capacitor state or the current position in an envelope. Reopening a project restores settings; it does not resume an interrupted analog waveform sample-for-sample.
* The alias regression measures one high stationary saw against a naive reference; it is not a complete PWM/modulation/frequency grid. Automated stress and endurance results are limited to the rates, durations and controls listed in VALIDATION.md.

Consult VALIDATION.md for host tests actually performed and any incomplete Live checks. A validator pass is not proof of sonic fidelity or zero defects in every host. The tested bundle was installed in the standard Windows VST3 folder with permission on 2026-09-27.
