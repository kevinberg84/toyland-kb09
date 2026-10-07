# Toyland KB-09 — Windows VST3 instrument

**Toyland KB-09 0.11.0** is a playable monophonic instrument based on Roland SH-09 manuals and service schematics, with an interactive photographic-style panel. Its engine is a **circuit-informed approximation** with explicit unmeasured parameters. See [known limitations](docs/KNOWN_LIMITATIONS.md), [fidelity evidence](docs/FIDELITY.md), [acceptance](docs/ACCEPTANCE.md) and [validation](docs/VALIDATION.md).

## Load and play

The Windows build produces `Toyland KB-09.vst3` and `Toyland KB-09.exe`. Development outputs are under:

- `C:\Dev\SH-09\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3`
- `C:\Dev\SH-09\build-kb09\SH09_artefacts\Release\Standalone\Toyland KB-09.exe`

The final 0.11 bundle is installed at `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; matching release artifacts are staged in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`. For another installation, copy the entire VST3 bundle, including `Contents`, into a DAW-scanned folder. The build does not install automatically. Legacy SH-09 and Toyland KB-09 share plugin identifiers, so install one identity in scanned folders. [Build instructions](docs/BUILD.md) explain the outputs and dependencies; [release validation](docs/VALIDATION.md) identifies the installed/tested binary.

Load **Toyland KB-09** on a MIDI track, select **Calibrated Init**, and play MIDI or the onscreen keyboard. The lowest held note wins. Gate+Trigger re-attacks from the current envelope level when the effective note changes; Gate retains legato. Oscillator and sub-divider phases run continuously.

The main selector offers noise, saw, square and variable pulse. Noise replaces the main waveform; the sub remains available. PWM uses manual width, undelayed LFO sine or ADSR. The VCF tracks keyboard pitch and accepts positive ADSR, negative ADSR or external-follower modulation. VCA Hold sustains without MIDI; Envelope uses ADSR; Gate follows held notes. Double-click continuous sound controls to restore defaults.

**SAVE** stores the current sound as a `.kb09` preset. **LOAD** accepts `.kb09` and compatible earlier `.sh09state` files. Loading a user sound retains all sixteen factory programs; selecting a factory program recalls its original settings. Invalid files leave the current patch unchanged. Power, held notes and window placement are not preset contents.

## Panel and window sizes

The 0.11 panel uses smooth **black painted metal with restrained reflections**, photographic material wear, 18 calibrated faders, six compact slide switches, upper-arc range/wave selectors and a 32-key F-to-C keyboard. The bender depicts a curved PB-4-style sector beneath a horizontal opening, with a projecting tab and spring return. Its geometry is an engineering reconstruction, not measured original-part CAD.

The supplied **KEVINBERG 09 / KB-09** mark remains on the left and **Toyland** on the right. Condition/Age gradually develops rust, metal patina and yellowed ink/keys, chips the original brand masks and reduces knob sheen. Each key owns a separate photographic stain, with no repeated/mirrored stain stamps. Two independent crack traces appear separately and extend slightly afterward. Three rotary controls have separate corrosion patches. All fader caps remain fitted. Wear is deterministic and illustrative, not a model of elapsed years.

A separate header contains factory/user-preset controls, meters, M/L and maximise/restore. M now fits within 90% of available monitor width and height, about 20% larger in each dimension than the preceding M; L requests the available work area. The complete panel and footer scale uniformly with letterboxing. Initial fitting waits for the attached peer scale, and Windows sizing measures actual child-client pixels separately from permission to move a host frame. Hosts retain control over their windows and DPI settings. [Sharp rendering in Live](docs/HIDPI.md) explains its Auto-Scale option and fresh-instance requirement. See [UI](docs/UI.md) and [window behaviour](docs/GUI_WINDOW_BEHAVIOR.md).

Power mutes output and clears voice history while callbacks continue consuming MIDI. Notes released while off cannot remain held; play a fresh note after switching on. External audio in VCA Hold can resume immediately. Power is transient and is not stored in patches.

## External audio

The VST3 instrument exposes an **External Input auxiliary bus**. Route a DAW audio source to it. **External Filter** and **External Follower** factory patches lower VCO/Sub and select VCA Hold for processing without MIDI. Raise EXT, then adjust cutoff and resonance. For follower filtering, choose ENV FOL'R in VCF and raise ENV depth. The detector listens before the EXT mixer slider, so reducing EXT can leave follower modulation active.

Stereo input is averaged to mono; opposite-polarity channels cancel. Mono output is duplicated for stereo output buses. The follower opens the filter; it does not trigger ADSR or open the VCA. [External architecture](docs/EXTERNAL_AUDIO_ARCHITECTURE.md) and the [Live report](docs/LIVE_TEST.md) describe routing and the host observations actually completed.

## Authentic, Modern and Age

Authentic ignores velocity, aftertouch, LFO tempo sync and input trim. Age remains independently adjustable; zero selects the healthy calibration profile.

Modern adds velocity-to-level, aftertouch-to-cutoff, tempo-synchronised LFO and ±24 dB input trim. In **0.11**, maximum LFO MOD depth is **±36 semitones for VCO and ±8 octaves for VCF**. Authentic retains **±12 semitones and ±5 octaves**. These are engineering ranges, not hardware measurements. **Older Modern patches with nonzero MOD can sound deeper after upgrading**, although their saved values and identifiers remain compatible. These depth changes do not alter the separate pitch-bender sensitivity controls.

There is no polyphony, unison, sequencer, onboard effect, MIDI Learn or hardware Gate/CV interface. The DAW stores sound parameters and factory-program reference. The former branding parameter remains an inert compatibility slot; the visible identity is always Toyland KB-09.

## Evidence and current release status

The signal path uses fixed 4x oversampling and reports **128 samples of resampling latency**. The x64 plugin processes float samples with double-precision filter state. Continuous controls smooth over approximately 8 ms; ordinary host automation follows block updates, while MIDI note offsets are sample-positioned.

Final October 7 checks passed: **92 DSP, 20 processor, 58 preset-file, 280 GUI, 47 Steinberg and 75 actual-VST3 host checks**, all with zero failures. GUI-enabled pluginval strictness 10 reported SUCCESS and exited 0. Processor checks recorded zero tracked C++ allocations in covered callbacks. The actual VST3 rendered 600 seconds; these are bounded software tests, not hardware-equivalence results. The build, staged bundle and installed module share SHA-256 `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`. [Validation](docs/VALIDATION.md) records the exact logs and standalone hash.

Final standalone M/L were observed at 1051x642/1280x720 logical pixels with the complete footer on this workstation. Live-specific 0.11 checks were not repeated because desktop automation could not resolve Live's foreground process after three recovery attempts; [the Live report](docs/LIVE_TEST.md) keeps earlier observations separate.

GUI assertions cover transformed footer bounds at wide/tall sizes, held white/black key feedback, bender positions and configured value-popup contrast. They do not establish every host gesture or physical-display condition. A physical 4K display was unavailable. The chassis source remains **1774 × 887**; 3840-pixel compiled-editor exports do not create native 4K photographic detail. [Artwork provenance](Assets/Photoreal/PROMPTS.md) records asset sources.

Primary-document facts and inferred DSP are distinguished in `docs/SH09_RESEARCH.md`, `docs/DSP_DESIGN.md` and `docs/DSP_DESIGN_DSP.md`. No physical SH-09 capture set or completed listening A/B is claimed. `tools/calibrate.py` supports future recording analysis; WAV files under `artifacts/Audio` are software renders.
