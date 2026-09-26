# Roland SH-09 primary-source investigation

Research date: 2026-09-26. This is a documentary and schematic investigation, not measurement of a physical instrument. Component values below were read from rendered original scans rather than trusting OCR. Page numbers are printed page numbers; the owner's PDF page is printed page + 1.

## Sources inspected

* **Roland SH-09 Owner's Manual**, 48-page scan, publication 10522: [download mirror](https://snw.lonningdal.no/sh09/Roland%20SH-09%20Owners%20Manual.pdf), [alternate scan](https://thismodernday.com/wp-content/uploads/2021/07/Roland-SH-09-Owners-Manual.pdf). Local copy: `Research/ROLAND_SH-09_OWNERS_MANUAL.pdf`. Read controls pp. 3-7, operation pp. 9-17, patches pp. 20-30, interfacing pp. 32-42 and block diagram p. 43.
* **Roland SH-09 Service Notes**, March 27, 1980, seven pages: [original document scan](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf). Local copy: `Research/ROLAND_SH-09_SERVICE_NOTES.pdf`. Examined specifications p. 1, semiconductor/potentiometer list p. 3, entire two-sheet schematic pp. 4-5, adjustment procedure p. 6 and PCB annotations p. 7. Rendered whole pages and enlarged schematic sections remain in `Research/rendered`.
* **Fairchild uA726 manufacturer data sheet**: [scan](https://radio-hobby.org/uploads/datasheet/7/ua72/ua726.pdf). This confirms a temperature-controlled monolithic matched transistor pair suitable for exponential current generation; it does not establish the measured SH-09 oscillator performance.
* **Coolaudio V662A manufacturer data sheet**, [searchable PDF](https://www.coolaudio.com/docs/Datasheet_CA_V662A_2020-10-12_Rev.0_Signed.pdf). Search extraction contains the replacement OTA's gain/control-current and low-pass curves, but direct fetch returned 404. This replacement is contextual information, not evidence that its parameters equal original BA662 devices. No complete original ROHM BA662 data sheet was obtained.
* First-hand photographs: [Still Not Working SH-09 repair](https://snw.lonningdal.no/sh09.php), [Studio Repair 24 photographs](https://www.studiorepair.com/gallery/Roland/SH-09/index.html), [C. Y. Kong's personal SH-09](https://www.cykong.com/Synths/Roland%20SH-09/RolandSH09.htm). These document separate units, including considerable rust on Kong's unit. They are reference images, not included licensed product textures.
* First-hand recordings: [AnalogAudio1 hardware demonstration](https://www.youtube.com/watch?v=QQEdaviFPeQ), [Jexus hardware demonstration](https://www.youtube.com/watch?v=rw2JZ8YVERI), and Kong's linked waveform/VCF/S&H MP3 examples. AnalogAudio1 states that its left channel is dry, with reverb in the introduction and delay in the right channel. These are listening-reference candidates; no measurement or completed listening comparison is claimed here.

## Verified panel and behavior

| Section | Original controls and behavior | Evidence |
|---|---|---|
| Modulator | Sine, square, random sample-and-hold; rate; sine delay | Owner pp. 3,16 |
| VCO | Range 32/16/8/4/2 feet; fine tuning; modulation amount; one four-position selector: noise, saw, square, variable pulse | Owner pp. 3-4,11 |
| PWM | Manual, LFO, envelope; one pulse-width/depth fader | Owner pp. 4,11 |
| Sub | Square one octave down; square two octaves down; fixed pulse two octaves down | Owner pp. 4,11 |
| Mixer | VCO, sub, external level; no independent noise fader | Owner p. 4, schematic p. 5 |
| VCF | Cutoff, resonance, envelope amount, LFO amount; positive envelope / negative envelope / envelope follower selector | Owner pp. 4-5 |
| VCA | Hold / Envelope / Gate | Owner pp. 5,13 |
| Envelope | Attack, decay, sustain, release; Gate+Trigger / Gate / LFO | Owner pp. 5-6,14-15 |
| Performance | Portamento, independent VCO/VCF bender sensitivities, spring-centered bender, output volume | Owner pp. 7,16-17 |
| Keyboard | 32 keys F3-C6 at 8 feet; monophonic low-note priority; permanent VCO and VCF keyboard CV | Service p. 1; Owner pp. 9,13-14 |

Noise replaces the main oscillator waveform in the VCO mixer channel. The sub remains derived from the continuously running VCO. An independent noise mixer fader would change the original architecture.

Sine delay follows the effective keyboard trigger, including a newly played lower note during a legato passage. Higher notes do not trigger while lower notes remain down. Delay does not affect square or random modulation. PWM receives the modulator's dedicated smooth waveform independently of the selected sine/square/random mode and independently of delay. All these modulation paths share one underlying LFO rate (owner p. 16).

Gate is high while any key is held. Effective low-note changes generate trigger events. In Gate+Trigger mode, the envelope re-attacks from its current level, rather than being reset to zero. Gate mode continues the existing envelope through legato changes. LFO trigger mode repeats within the keyboard gate. Release begins immediately when all keys are released, including during attack or decay (owner pp. 14-15). The original keyboard cannot generate MIDI duplicate-note events; software handling of duplicate MIDI notes is an engineering decision.

The manual explicitly states that portamento stops when a key is released (p. 16). The pitch control remains sample-held. A digital implementation should avoid silently replacing this with always-completing pitch glide.

There is **no documented SH-09 auto-bend control or route** in the panel, block diagram, or operation text. Do not import SH-2 or SH-101 features based on family resemblance.

## Published ranges

Service p. 1 gives attack 1 ms-2.5 s, decay 2 ms-10 s, release 2 ms-10 s, sustain 0-100%, LFO 0.2-25 Hz, LFO delay 0-1.5 s, portamento 0-5 s, tuning +/-65 cents, VCF cutoff 10 Hz-20 kHz and resonance through self-oscillation. External signal input sensitivity is 0.5 Vpp or less. Main output is -10 dBm; CV is 1 V/oct; gate output 0/+14 V and input threshold +7.5 V.

These endpoints do not specify every fader taper, attack/decay measurement convention, bender maximum interval or DAW dBFS equivalence. A digital range map between endpoints must be identified as a model assumption.

## Schematic signal path

### VCO, dividers, PWM

Service p. 4 shows IC5 uA726 heated matched-pair exponential converter, op-amp integration around IC6 with C9 1000 pF, reset circuitry Q7/IC7 and separate comparator/pulse circuitry IC6. This is a discrete/op-amp oscillator with analog current conversion, not the SH-101 CEM3340. The range switch uses four selected equal 1 kOhm resistors R186-189 (p. 7) and range adjustment. The square-duty trim VR2 is adjusted to 1:1 mark/space on p. 6. No measured saw ramp curvature or comparator slewing data is given.

IC8 and IC9 are cascaded DN819 dividers. Their outputs feed the sub selector, and D21/D22 combine division states for the narrow two-octave pulse. A modulo-four counter clocked by the main oscillator is the justified digital architecture. A 25% pulse follows from the logic combination, but startup polarity/phase and real transition asymmetry remain unmeasured.

### Noise

Q34 is the selected 2SC828NZ noise transistor identified in the parts list, used as a reverse-biased junction. Q35-Q37 amplify it. The circuit contains AC coupling, emitter bypass and frequency-dependent feedback/shunt components including C38 .033 uF, C39 1 uF, C40 100 pF, C41 .01 uF and resistors R165-175. Thus the output is amplified and spectrally shaped junction noise, not a mathematically flat spectrum extending to Nyquist. A colored-noise approximation is justified; precise power spectral density and RMS gain require physical measurements or a complete transistor simulation with a noise model.

### Mixer and filter

Service p. 5 shows VCO/sub/ext logarithmic faders and summing resistors (VCO approximately 390 kOhm, sub 1 MOhm, external 330 kOhm). Therefore unity-normalized equal sources are not an established hardware calibration.

The VCF comprises four **selected BA662 OTAs IC11-14**, 1.5 nF capacitors C17-20, discrete buffers Q15-18 and local feedback. The service note explicitly requires transconductance matching across the four OTAs. First-stage input attenuation differs from later stages: R79 58 kOhm, later R98/R104/R111 68 kOhm, with 1 kOhm input shunts. Local feedback resistors R97/R102/R108/R115 are 100 kOhm. This is a four-pole OTA low-pass with local feedback and global resonance, not a transistor ladder. Nominal DC section gain cannot simply be assumed equal to one without normalization.

Q19 buffers the VCF output; VR111 100 kOhm A-taper controls feedback. Its wiper connects to R118 1.5 kOhm and antiparallel D12/D13 to ground, then R109 12 kOhm returns to the first stage. This explicit diode clamp limits resonance-feedback amplitude; a purely linear four-pole resonator misses that behavior. Q20/Q21 and IC17 form the cutoff exponential-control path; permanent keyboard CV is summed alongside panel cutoff, modulation and bender.

Calibration p. 6 adjusts VCF pitch tracking by requiring F3 to double F2, with self-oscillation around 1 kHz; maximum cutoff is trimmed to 20 kHz with the lowest key F1 in that procedure. Those F1/F2/F3 labels are explicitly **local service key designations**, not ordinary MIDI octave labels. Replacement test p. 5 expects about 50 Hz self-oscillation with its specified C2 key, cutoff minimum and resonance maximum. These are useful later calibration anchors, not measurements of this model.

### VCA/output

IC15 is a fifth BA662 with buffer; Q25 and related circuitry supply control current selected from hold, gate or envelope. Volume and output buffer Q38 follow. The headphone amplifier is separate. Original mono output must not acquire stereo effects in Authentic mode. OTA gain linearity, residual feedthrough, bias distortion and output headroom are unmeasured.

### External signal and follower

Input enters Q32/Q33 discrete preamplifier through C30 .033 uF and bias/feedback network. Its output splits into C33 .033 uF leading to the external mixer fader, and C34 .033 uF leading to the follower rectifier. The follower is therefore tapped **before the external mixer fader**, not from the filtered/VCA audio and not from the final output.

D20 and R159 3.3 kOhm clamp one polarity; D19 rectifies positively. R160 100 kOhm loads the rectified node. R161/R162/R163 are each 100 kOhm; C35 .1 uF is grounded; C36 .1 uF connects its intermediate node to the follower output; C37 .033 uF is grounded at the op-amp input. IC1 buffers the result with R164 390 kOhm feedback. This is a fixed rectifier and active multipole smoothing network, **not adjustable attack/release ADSR**, and not three independent passive RC stages. Component RC products of 10 ms, 10 ms and 3.3 ms describe scale only; active feedback/loading changes the actual transfer response.

Follower control is selected in the VCF envelope-source switch. It positively opens the VCF as external amplitude increases; the envelope-depth fader controls amount. It does not automatically trigger the synth's ADSR or open the original VCA. External operation without MIDI therefore requires **VCA Hold**, low VCO/sub mixer levels and sufficient EXT level. Owner pp. 32-34 demonstrate this. External audio with MIDI/gate can instead use VCA Gate/Envelope and the usual ADSR. An external audio sidechain is the software equivalent of the jack; MIDI gate remains a separate signal.

## Evidence boundaries

The primary documents establish topology, route selection, performance logic and the published endpoints. They do not establish digital saturation scales, complete control tapers, noise spectrum, oscillator shape error, absolute mixer gain, envelope/follower waveforms, temperature drift rates, aging behavior or exact self-oscillation amplitude. Those require circuit simulation with validated models and/or recordings from a calibrated physical SH-09. Internet demos are not substitutes for those measurements. Any condition/age system and modern modulation additions must be disclosed as optional engineering models.
