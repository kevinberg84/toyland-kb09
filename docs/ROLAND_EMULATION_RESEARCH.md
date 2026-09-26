# Public Roland emulation research

Research date: 2026-09-26. Scope: public manufacturer descriptions, user manuals and research publications. No proprietary binaries, commercial code, graphical assets or preset banks were inspected or copied.

Labels: **VERIFIED** identifies an inspected source's documented statement; it does not independently verify a manufacturer's fidelity claim. **SUPPORTED** is evidence for an engineering approach. **INFERRED** is our interpretation. **SPECULATIVE** requires new evidence.

## Roland ACB

**VERIFIED:** Roland describes ACB as component-level behavioral work informed by design drawings, analysis of reference instruments and consultation with original engineers. Its explanation emphasizes reconstructing interactions among components and knowledge not captured by schematic analysis alone. Roland also attributes substantial CPU demand to component modeling. These are public process descriptions, not disclosed numerical algorithms. [Roland, *What is Analog Circuit Behavior?*](https://rolandcorp.com.au/blog/what-is-analog-circuit-behavior-acb), [Roland support, Legendary-series CPU use](https://support.roland.com/hc/en-us/articles/4412191951771-Roland-Cloud-Why-are-the-Legendary-series-synthesizers-so-intensive-on-my-CPU).

**SUPPORTED:** Schematic study, device behavior, calibration, signal levels and controlled comparisons should be considered together. **SPECULATIVE:** ACB's exact state equations, solver, oversampling factor, tolerance distribution and parameter-fitting method. We found no public primary disclosure justifying a claim that our code uses the same algorithms.

## SH-2, SH-101 and SYSTEM-100 software

**VERIFIED manufacturer statements:** Roland's SH-2 page specifically calls out circuit interaction, self-oscillating VCF behavior, envelope attack/decay, harmonic makeup and pitch variation. It also advertises a second envelope, expanded range and other additions. This establishes which behaviors Roland considered relevant, not that SH-2 and SH-09 have identical circuits. [Roland SH-2 software](https://www.roland.com/global/products/rc_sh-2/).

Roland's SH-101 software page describes involvement from original engineers and adds a dedicated filter envelope, extra LFO shapes, expanded oscillator range and tempo-related features. The published parameter ranges describe that software product; they must not become SH-09 specifications. [Roland SH-101 software](https://www.roland.com/global/products/rc_sh-101/).

Roland's SYSTEM-100 software combines two historical modules and offers visible routing, a routing matrix and effects. The useful transferable lesson is to make signal routing inspectable. Its filter, oscillator, ring modulator and control ranges are not SH-09 evidence. [Roland SYSTEM-100 software](https://www.roland.com/global/products/rc_system-100/).

## TAL, Softube and D16

**VERIFIED manufacturer statements:** TAL says BassLine-101 models envelope transitions between overlapping notes and generates oscillators in real time. Its hardware/software envelope demonstrations repeat notes because oscillator free-running phase changes the attack waveform. Its changelog includes sub-mode, gate-mode and oversampling corrections. These point directly to behavioral regression tests worth retaining. TAL's modern options include microtuning, modulation integration and a preset browser. The public page does not disclose a transferable SH-09 filter equation. [TAL-BassLine-101 product page and changelog](https://tal-software.com/products/tal-bassline-101).

Softube documents the Model 82's front-panel behavior, envelope modes and added output drive/doubling. It discusses self-oscillation and interaction between strong oscillator input and resonant filtering. Its manual provides a clear example of distinguishing added functions from the original voice architecture. Model 82 is SH-101-style research, not SH-09 topology verification. [Softube Model 82 manual](https://www.softube.com/user-manuals/model-82-sequencing-mono-synth).

D16's Lush 2 page emphasizes oscillator spectral behavior during pitch modulation, resonance across cutoff settings, self-oscillation and multilayer workflow. These are manufacturer claims and test suggestions, not independent measurements. The successor product is useful current context for LuSH-101's extended architecture. The original LuSH-101 manual remains a historical workflow reference. [D16 Lush 2](https://d16.pl/lush2), [D16 LuSH-101 manual](https://w-ww.d16.pl/pub/manuals/LuSH-101-manual-gb.pdf).

## Transfer decisions

| Technique or observation | Classification | Decision for this project |
| --- | --- | --- |
| Analyze real circuit stages plus their interaction | SUPPORTED | Trace SH-09 summing, attenuation and feedback; preserve levels between stages. |
| Model note priority and envelope retrigger separately from ADSR curves | SUPPORTED | Derive event rules from SH-09 documentation and test overlapping/repeated notes. |
| Repeated attack comparisons with free-running VCO | SUPPORTED | Compare several phase conditions; do not tune one transient to one video. |
| Test resonance over both frequency and drive | SUPPORTED | Include cutoff sweeps and self-oscillation with/without oscillator and external input. |
| Reuse SH-101 cutoff, envelope ranges or sub shape | SPECULATIVE | Reject unless SH-09 sources independently establish the same fact. |
| Reuse SH-2 filter because of release date or family name | SPECULATIVE | Reject; require schematic/device/operating-range comparison. |
| Claim our solver reproduces ACB | SPECULATIVE | Reject; no implementation disclosure supports it. |
| Add conveniences outside the original panel | INFERRED | Use a clearly separate Modern area with explicit opt-in and complete state recall. |

The transfer gate is always: matching topology, matching devices or a justified reduced model, matching operating regime, and supporting SH-09 evidence. Failure of any gate means the idea is at most an engineering experiment. These pages do not prove sonic equivalence of the resulting instrument.

## Public condition controls

**VERIFIED:** Roland publicly describes Circuit Mod and Condition in current ACB expansions as ways to vary vintage character. It does not publish an SH-09 tolerance distribution or a mapping from years of age to component values. [Roland JUNO-106 ACB Expansion](https://www.roland.com/mx/products/rc_juno-106_acb_expansion/).

**INFERRED:** A saved, bounded calibration profile is more defensible than per-note random detuning. Correlate parameters only where a shared physical cause is plausible. Keep a healthy reference state; report that the amount of wear/drift is an artistic extension unless measured population data becomes available. See `DSP_RESEARCH.md` for the proposed separation of fixed offsets and slow variation.

## Evidence limits

No SH-09 recording comparison, listening evaluation of our executable, or hardware measurement occurred during this literature task. Public promotional language is not a validation result. Future controlled recordings should include dry oscillator spectra, mixer drive, self-oscillation tuning/level, complete gate sequences, external-input sweeps and follower bursts; compression and unidentified settings in internet demonstrations prevent them from serving as laboratory standards.
