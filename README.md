# Toyland KB-09 — Windows VST3 and standalone instrument

**Toyland KB-09 0.12.0** is a playable monophonic instrument based on Roland SH-09 manuals and service schematics, with an interactive photographic-style panel informed by Kevin's own instrument. Its engine is a **circuit-informed approximation** with explicit unmeasured parameters. See the [working user manual](docs/manual/index.html), [known limitations](docs/KNOWN_LIMITATIONS.md), [fidelity evidence](docs/FIDELITY.md), [acceptance](docs/ACCEPTANCE.md) and [validation](docs/VALIDATION.md).

## Load and play

The Windows build produces `Toyland KB-09.vst3` and `Toyland KB-09.exe`. Development outputs are under:

- `C:\Dev\SH-09\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3`
- `C:\Dev\SH-09\build-kb09\SH09_artefacts\Release\Standalone\Toyland KB-09.exe`

The system installation path is `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; the 0.12 release staging path is `artifacts/Release/Toyland KB-09-0.12.0-Windows-x64`. For another installation, copy the entire VST3 bundle, including `Contents`, into a DAW-scanned folder. The build does not install automatically. Legacy SH-09 and Toyland KB-09 share plugin identifiers, so install one identity in scanned folders. [Build instructions](docs/BUILD.md) explain the outputs and dependencies; [release validation](docs/VALIDATION.md) identifies the installed/tested binary and its completed checks.

The `.exe` is a standalone virtual synthesizer: open its audio/MIDI settings, select your output device and MIDI keyboard, and play without a DAW. Current build targets are Windows VST3 and standalone. AU for Mac hosts and AAX for Pro Tools are planned after the VST3 is complete and can be tested on those platforms; they are not included in this build. The offline [HTML manual](docs/manual/index.html) has clickable DAW sections, installation guidance and a personal test checklist. Keep its HTML, stylesheet and assets together when copying it.

Load **Toyland KB-09** on a MIDI track, select **Calibrated Init**, and play MIDI or the onscreen keyboard. The lowest held note wins. Gate+Trigger re-attacks from the current envelope level when the effective note changes; Gate retains legato. Oscillator and sub-divider phases run continuously.

The main selector offers noise, saw, square and variable pulse. Noise replaces the main waveform; the sub remains available. PWM uses manual width, undelayed LFO sine or ADSR. The VCF tracks keyboard pitch and accepts positive ADSR, negative ADSR or external-follower modulation. VCA Hold sustains without MIDI; Envelope uses ADSR; Gate follows held notes. Double-click continuous sound controls to restore defaults.

**SAVE** stores the current sound as a `.kb09` preset. **LOAD** accepts `.kb09` and compatible earlier `.sh09state` files. Loading a user sound retains all sixteen factory programs; selecting a factory program recalls its original settings. Invalid files leave the current patch unchanged. Power, held notes and window placement are not preset contents.

## Panel and window sizes

The 0.12 panel uses **black painted metal with restrained reflections**, photo-informed material wear, 18 calibrated faders, six compact slide switches, upper-arc range/wave selectors and a 32-key F-to-C keyboard. Knob barrels/skirts, fader caps, exposed posts, switch grips and the projecting bender follow the new multi-angle photographs. The volume control follows stock SH-09 geometry rather than the owner's replacement knob. Dimensions remain image-derived rather than measured original-part CAD. A small panel label identifies this as an unofficial personal project, unaffiliated with Roland Corporation and not for sale.

The supplied **KEVINBERG 09 / KB-09** mark remains on the left and **Toyland** on the right. **AGE — fresh to well loved** is both a creative sound profile and a visual condition control. The owner's photographed synth guides the look around displayed AGE 7, with further wear available toward 10. Dust, yellowed ink/ivory, dulling, knob tarnish and corrosion around exposed panel paint develop gradually. Screw heads remain dull metallic rather than receiving red-rust stamps. Independent seeded microdetail gives each key its own dust and grime; the old wear/crack atlas is no longer used. The envelope coffee ring is retained and appears at AGE 6 with a fixed shape and opacity.

One photo-informed hairline crack appears on MIDI note 60 at AGE 5.6 and can lengthen slightly. The VCO mixer cap disappears at 6.4, RELEASE loses its cap at 8.8, and the bender tip breaks at 9.2. Cap loss and tip breakage appear as discrete events, revealing flat stepped metal blades or the broken bender sector. These controls remain fully functional. Lowering AGE reverses the appearance; wear is deterministic and illustrative, not a model of elapsed years. The owner's bunny sticker and taped waveform marks are excluded.

A separate header contains factory/user-preset controls, meters, S/M/L and maximise/restore. S fits within 65% of available monitor width and height, M within 90%, and L requests the available work area. The complete panel and footer scale uniformly with letterboxing. Restore returns to the preceding S, M or custom dimensions. Initial fitting waits for the attached peer scale, and Windows sizing measures actual child-client pixels separately from permission to move a host frame. Hosts retain control over their windows and DPI settings. Kevin confirmed that disabling **Auto-Scale Plug-in Window** fixed the Live display. If a setting change is not reflected, reopen the editor and optionally test a fresh instance; creating one was not part of that reported confirmation. See the [manual's Live instructions](docs/manual/index.html#live), [HiDPI notes](docs/HIDPI.md), [UI](docs/UI.md) and [window behaviour](docs/GUI_WINDOW_BEHAVIOR.md).

Power mutes output and clears voice history while callbacks continue consuming MIDI. Notes released while off cannot remain held; play a fresh note after switching on. External audio in VCA Hold can resume immediately. Power is transient and is not stored in patches.

## External audio

The VST3 instrument exposes an **External Input auxiliary bus**. Route a DAW audio source to it. **External Filter** and **External Follower** factory patches lower VCO/Sub and select VCA Hold for processing without MIDI. Raise EXT, then adjust cutoff and resonance. For follower filtering, choose ENV FOL'R in VCF and raise ENV depth. The detector listens before the EXT mixer slider, so reducing EXT can leave follower modulation active.

Stereo input is averaged to mono; opposite-polarity channels cancel. Mono output is duplicated for stereo output buses. The follower opens the filter; it does not trigger ADSR or open the VCA. [External architecture](docs/EXTERNAL_AUDIO_ARCHITECTURE.md) and the [Live report](docs/LIVE_TEST.md) describe routing and the host observations actually completed.

## Authentic, Modern and Age

Authentic ignores velocity, aftertouch, LFO tempo sync and input trim. Age remains independently adjustable; zero selects the healthy calibration profile.

Modern adds velocity-to-level, aftertouch-to-cutoff, tempo-synchronised LFO and ±24 dB input trim. Since **0.11**, maximum LFO MOD depth is **±36 semitones for VCO and ±8 octaves for VCF**. Authentic retains **±12 semitones and ±5 octaves**. These are engineering ranges, not hardware measurements. **Older Modern patches with nonzero MOD can sound deeper after upgrading**, although their saved values and identifiers remain compatible. These depth changes do not alter the separate pitch-bender sensitivity controls.

There is no polyphony, unison, sequencer, onboard effect, MIDI Learn or hardware Gate/CV interface. The DAW stores sound parameters and factory-program reference. The former branding parameter remains an inert compatibility slot; the visible identity is always Toyland KB-09.

## Evidence and current release status

The signal path uses fixed 4x oversampling and reports **128 samples of resampling latency**. The x64 plugin processes float samples with double-precision filter state. Continuous controls smooth over approximately 8 ms; ordinary host automation follows block updates, while MIDI note offsets are sample-positioned.

[Validation](docs/VALIDATION.md) records exact build results, module hashes, installed/staged identity and the bounded scope of completed checks. Historical 0.11 results belong to that binary; they do not validate changed 0.12 artwork or rendering. [The Live report](docs/LIVE_TEST.md) distinguishes native observations from the owner's confirmed Auto-Scale fix.

GUI assertions exercise button-selected S/M/L sizes and restoration, transformed footer bounds, held white/black key feedback, bender positions, configured value-popup contrast, and the individual aging events. They do not establish every host gesture or physical-display condition. The renderer combines native geometry with finite-resolution photographic sources: a 3840-pixel compiled-editor export does not create additional source detail. [Artwork inventory and provenance](Assets/Photoreal/README.md) records current asset sources and dimensions.

Primary-document facts and inferred DSP are distinguished in `docs/SH09_RESEARCH.md`, `docs/DSP_DESIGN.md` and `docs/DSP_DESIGN_DSP.md`. No physical SH-09 audio capture set or completed listening A/B is claimed. `tools/calibrate.py` supports future recording analysis; WAV files under `artifacts/Audio` are software renders.
