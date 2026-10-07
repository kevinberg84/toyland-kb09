# Toyland KB-09 0.11.0 user interface

The editor combines generated photographic material assets with native JUCE controls, text and markings. Its module order follows Modulator, VCO, Sub Oscillator, Audio Mixer, VCF, VCA and Envelope. The left bay contains volume, power, VCO/VCF bend sensitivities, portamento and a spring-return bender. The 32-key keyboard spans MIDI 53–84: F3–C6 when middle C is C4, with 19 white and 13 black keys.

The body is **1500 × 750 logical design units**, with a separate **34-unit header** and **50-unit footer**: **1500 × 834** overall. The header provides the factory menu, previous/next, SAVE/LOAD, ENV/EXT meters, M/L and maximise/restore. The footer contains optional sound controls and external-routing help.

## Controls and presets

- Drag faders vertically and knobs horizontally or vertically. Double-click continuous controls to restore their defaults. Tuning displays cents; panel controls use 0–10 value bubbles. Range and waveform are detented rotary selectors.
- Six compact slide switches have a 24-unit throw inside a 44-unit recess. Click/drag their ridged actuators or adjacent legends. Focused switches accept arrows, Home and End; double-click restores the default.
- SAVE opens a native asynchronous file dialog and writes the current sound to `.kb09`. LOAD accepts that format and compatible legacy `.sh09state` exports. Files contain the complete schema-1 parameter state and factory-program reference, not a replacement factory bank. Sixteen built-in programs remain available.
- Loading validates the complete file before changing the patch; cancellation or rejection leaves the current sound unchanged. Saving writes and checks a temporary file before replacing the destination. Dialog callbacks are guarded by weak lifetime/owner checks, and file I/O runs on the message thread. The loaded filename appears in the preset field. Selecting a factory program restores its label even when its index matches the user patch's stored reference.
- Authentic dims/disables velocity, aftertouch, LFO sync and input trim. Their values return when Modern is enabled. Age remains available in either mode.
- Power mutes output through a dedicated processor state. Callbacks continue consuming/discarding MIDI while off, and transitions clear voice, held-note and controller history. Play a fresh note after power-up; external audio in VCA Hold can resume without MIDI. Power is neither saved in patches nor an automatable parameter.
- Keyboard dragging presses the destination before releasing the previous note to retain legato. Focus loss, hiding and destruction release mouse-held notes. MIDI-held white and black keys change appearance; the effective lowest note has an amber marker. Note events use a fixed-capacity queue and display state is read from atomics.
- The bender depicts a curved PB-4-style sector beneath a horizontal slot. Its projecting tab rotates in a foreshortened vertical plane about an inferred front-to-back axle; it returns to centre after dragging. The ±35° drawing is an engineering estimate supported by another Roland instrument's PB-4 documentation, not measured SH-09 travel. Incoming MIDI pitch bend affects audio but is not mirrored by this mouse control.
- ENV and EXT meters show envelope/follower activity. EXT AUDIO displays routing instructions over the keyboard and intercepts clicks on the overlay.

The original-style switches use these top-to-bottom orders:

| Switch | Top | Middle | Bottom |
|---|---|---|---|
| Modulator | Sine | Square | Random / sample-and-hold |
| PWM | LFO | Manual | Envelope |
| Sub | −1 octave square | −2 octaves square | −2 octaves narrow pulse |
| VCF source | External follower | Positive envelope | Inverted envelope |
| VCA | Hold | Envelope | Gate |
| Envelope trigger | Gate + Trigger | Gate | LFO |

There are 18 vertical sound-control faders, two large rotary selectors, volume, fine tune and the bender. Faders carry 10/5/0 scales; volume has an arc. Range/waveform upper arcs span approximately ±65°/±60°. Dimensions, arcs and mechanisms are photographic reconstruction estimates. [GUI_RESEARCH.md](GUI_RESEARCH.md) records references and uncertainties.

Modern's LFO MOD maxima are ±36 semitones VCO and ±8 octaves VCF; Authentic retains ±12/±5. Old Modern patches with nonzero MOD may sound deeper in 0.11. These engineering maxima are separate from the bender sensitivity controls and are not physical-unit calibration claims.

## Window and display behaviour

M fits the whole canvas within 90% of monitor work-area width and height, about 20% larger in each dimension than the preceding M. L requests the available work area. Uniform scaling and letterboxing retain panel proportions and the complete footer at different aspect ratios. Windows sizing derives scale from the actual native child-client pixel rectangle, independently of whether the host frame can safely be moved, and subtracts measurable surrounding chrome. The first attached fit waits for stable scale measurements. It does not change process-wide DPI awareness or the DAW's renderer. [HIDPI.md](HIDPI.md) explains Live's separate Auto-Scale setting.

Standalone L maximises its own native window while preserving its normal size for Restore. Its custom header supports dragging, restore-under-pointer and top-edge snap. VST3 requests sizes through the host; native movement is limited to conservatively identified floating frames. A host can restrict or reject requests. [GUI_WINDOW_BEHAVIOR.md](GUI_WINDOW_BEHAVIOR.md) separates current implementation, older observations and remaining host checks.

## Materials and Age

The chassis is smooth **black painted steel with reduced sheen**, distinct from molded side cheeks and reflective knob skirts. Rust, yellowing and metal patina develop gradually with Age. The isolated chassis-patina layer blends at `0.68 × Age²` on the main face, fold and performance bay; registered aged screw patches blend at `0.94 × Age²`. Knobs tarnish and lose sheen. The clean front-rail geometry remains in place with native sheen reduction.

Six case stains appear separately at Age **0.25, 0.38, 0.49, 0.60, 0.72 and 0.86**, using fixed geometry and 22% drawing opacity. Each of the 32 keys has a distinct healthy appearance through subtle tint/polish differences, plus its own independently sourced photographic stain. No case mark is reused on a key. Key-stain onset is `0.18 + (MIDI note − 53) × 0.023`, from 0.18 to 0.893. White-key yellowing develops gradually; the old repeated aged-black-key image is no longer drawn. Once a stain appears, its size and opacity do not increase with Age. Common clean key materials remain, while the final composites and all stain shapes differ.

Separate transparent crack traces appear on **MIDI 60 at 0.82** and **MIDI 77 at 0.94**. Each starts at 78% of its eventual length and reveals the remaining 22% as Age reaches one. These are two independent fracture shapes, not a mirrored copy. The three large rotary controls each use a separate corrosion patch blended gradually at `0.74 × Age²`, plus native dulling. All fader caps remain fitted at every Age. Their underlying LFE9R-C16 shaft geometry above the assembled panel was not verified, so no exposed-post depiction is included. The same deterministic Age parameter also changes the engine's documented tolerance profile; it is not an elapsed-years simulation.

Aged branding removes coverage only within the exact original supplied letter masks. It preserves the **KEVINBERG 09 / KB-09** mark on the left and **Toyland** on the right. Native labels and calibration ink age with the panel. No reference photograph or Universal Audio graphic is used as runtime artwork.

| Embedded asset family | Source resolution |
|---|---:|
| Black chassis / source wear artwork | 1774 × 887 |
| Clean rotary knob | 1254 × 1254 |
| Fader cap | 1536 × 1024 |
| Clean key pair | 1254 × 1254 |
| 37 independent wear decals | 1448 × 1086 atlas; each assigned crop 177 × 181 |
| Supplied / aged branding masks | 2172 × 724 |

`KB09Artwork` embeds nine PNGs: `chassis-black`, `chassis-patina`, `chassis-stains`, `knob`, `fader`, `keys`, `wear-atlas`, `marks` and `marks-aged`. Runtime needs no external image files or network. `Assets/Photoreal/README.md` records provenance and derived-layer dimensions. Persistent images use CPU-backed storage and the Windows editor selects its own JUCE software renderer. System Arial is requested with fallback. `Assets/Branding.h` supplies the single public identity; the legacy branding parameter is an inert compatibility slot.

Text/vector details render at output scale. A 3840-pixel export does not turn the 1774-pixel chassis into a native 4K photograph. Physical 4K, mixed-monitor DPI and broad host/accessibility coverage remain unavailable or untested.

## Recorded GUI checks

The final October 7 fixture reports **280 assertions, zero failures** in `artifacts/Validation-KB09/gui-0.11-aging-events-final.txt`, with previews and metadata in `artifacts/GUI-0.11-final`. It covers original parameter IDs/indices, schema-1 round trips, physical choices, PWM keyboard selection/automation redraw, white/black held-note changes and release restoration, bender left/centre/right appearance, and transformed footer controls/labels at wide and tall sizes. Expanded checks cover six abrupt stain thresholds and their fixed subsequent appearance, separate crack onsets, and distinct healthy/aged samples from all 19 white and 13 black keys. Configured value-bubble foreground/background contrast measured **12.68:1**; actual native hover-popup placement was not observed, and this is not a complete accessibility audit.

Snapshots include 2880 × 1620 and 3840 × 2160 exports. They are compiled-editor renders, not observations of a physical 4K monitor. Final native standalone M/L observations recorded **1051 × 642 / 1280 × 720 logical pixels**, with the full footer visible, on a 1920 × 1080 physical display at 150% scaling. The earlier 0.10 fixture's 94 assertions in `artifacts/UI` are historical. Final binary hashes, validator results and installation are maintained in [VALIDATION.md](VALIDATION.md). Live 0.11 was not retested because desktop automation could not resolve its foreground process after three recovery attempts; [LIVE_TEST.md](LIVE_TEST.md) scopes earlier host observations separately.
