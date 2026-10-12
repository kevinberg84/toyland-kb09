# Current 1.5.1 GUI

Approved revisions 4–12 refine panel spacing, the performance bay, keyboard seams, photographic cheeks, spatial rust growth and bender geometry/material. The final pointer uses the disc's polished highlight; tapered roughness scuffs replace uniform light dashes on its rotating plastic. Key/cap scuff counts and twist/lean progress at 0/25/50/75/100%. Every cap has a unique fixed seeded angle, with FREQ/RES restrained. PULSE WIDTH replaces VCF MOD in the final cap-loss event. The audio engine and parameter identities are unchanged.

# Current 1.5.0 GUI

The approved five-stage GUI, footer scales, performance-bay spacing and material changes are described in `GUI_REVIEW_2026_10_12_REV3.md`. The release adds lighter white-key scuffs at Sold As-Is while retaining the approved Fire Hazard endpoint. Current user instructions are in `manual/index.html`; material details and image provenance are in `Assets/Photoreal`. The historical UI notes below do not define the current controls.

# Toyland KB-09 0.12.0 user interface

This describes the current 0.12 source and artwork integration plan. A completed new build, final visual acceptance and host validation are not claimed here; [VALIDATION.md](VALIDATION.md) records build-specific evidence. The 0.11 observations below are historical.

The editor combines generated photographic material assets with native JUCE controls, text and markings. Its module order follows Modulator, VCO, Sub Oscillator, Audio Mixer, VCF, VCA and Envelope. The left bay contains volume, power, VCO/VCF bend sensitivities, portamento and a spring-return bender. The 32-key keyboard spans MIDI 53–84: F3–C6 when middle C is C4, with 19 white and 13 black keys.

The body is **1500 × 750 logical design units**, with a separate **34-unit header** and **50-unit footer**: **1500 × 834** overall. The header provides the factory menu, previous/next, SAVE/LOAD, ENV/EXT meters, S/M/L and maximise/restore. The footer contains optional sound controls and external-routing help. The project notice sits in the unused top body area.

## Controls and presets

- Drag faders vertically and knobs horizontally or vertically. Double-click continuous controls to restore their defaults. Tuning displays cents; panel controls use 0–10 value bubbles. Range and waveform are detented rotary selectors.
- Six compact slide switches have a 24-unit throw inside a 44-unit recess. Click/drag their ridged actuators or adjacent legends. Focused switches accept arrows, Home and End; double-click restores the default.
- SAVE opens a native asynchronous file dialog and writes the current sound to `.kb09`. LOAD accepts that format and compatible legacy `.sh09state` exports. Files contain the complete schema-1 parameter state and factory-program reference, not a replacement factory bank. Sixteen built-in programs remain available.
- Loading validates the complete file before changing the patch; cancellation or rejection leaves the current sound unchanged. Saving writes and checks a temporary file before replacing the destination. Dialog callbacks are guarded by weak lifetime/owner checks, and file I/O runs on the message thread. The loaded filename appears in the preset field. Selecting a factory program restores its label even when its index matches the user patch's stored reference.
- Authentic dims/disables velocity, aftertouch, LFO sync and input trim. Their values return when Modern is enabled. Age remains available in either mode.
- Power mutes output through a dedicated processor state. Callbacks continue consuming/discarding MIDI while off, and transitions clear voice, held-note and controller history. Play a fresh note after power-up; external audio in VCA Hold can resume without MIDI. Power is neither saved in patches nor an automatable parameter.
- Keyboard dragging presses the destination before releasing the previous note to retain legato. Focus loss, hiding and destruction release mouse-held notes. Mouse-held and MIDI-held white and black keys have blue highlights; a seven-unit amber bar at the lower edge identifies the effective lowest note. A mouse press shows its amber bar immediately when it wins priority; audio-thread selection changes also request a repaint independently of held-key counts. Note events use a fixed-capacity queue and display state is read from atomics.
- The bender depicts the raised curved wheel visible in the user's original-unit close-up, with an intact tip reconstructed from additional original-unit references. Its projecting tip rotates in a foreshortened vertical plane about an inferred front-to-back axle and returns to centre after dragging. The ±35° drawing is an engineering estimate, not measured SH-09 travel. At Age 0.92 its tip breaks cosmetically; bend operation and spring return remain active. Incoming MIDI pitch bend affects audio but is not mirrored by this mouse control.
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

Modern's LFO MOD maxima are ±36 semitones VCO and ±8 octaves VCF; Authentic retains ±12/±5. These maxima were introduced in 0.11, so older Modern patches with nonzero MOD may sound deeper. They are separate from the bender sensitivity controls and are not physical-unit calibration claims.

## Window and display behaviour

S fits the whole canvas within 65% of monitor work-area width and height; M uses 90%; L requests the available work area. Uniform scaling and letterboxing retain panel proportions and the complete footer at different aspect ratios. Windows sizing derives scale from the actual native child-client pixel rectangle, independently of whether the host frame can safely be moved, and subtracts measurable surrounding chrome. The first attached fit waits for stable scale measurements. It does not change process-wide DPI awareness or the DAW's renderer. [HIDPI.md](HIDPI.md) explains Live's separate Auto-Scale setting.

Standalone L maximises its own native window while preserving its normal size for Restore. Its custom header supports dragging, restore-under-pointer and top-edge snap. VST3 requests sizes through the host; native movement is limited to conservatively identified floating frames. A host can restrict or reject requests. [GUI_WINDOW_BEHAVIOR.md](GUI_WINDOW_BEHAVIOR.md) separates current implementation, older observations and remaining host checks.

## Materials and Age

The chassis depicts smooth black painted metal with reduced sheen, distinct from molded side cheeks and reflective knob skirts. The new personal-unit material endpoint blends at `0.90 × Age^0.85` on the main face, fold and performance bay. Registered screw-head patches use `0.94 × Age` and a dull silver appearance; the photographs do not establish their alloy. The clean front rail retains its geometry with native sheen reduction.

The original coffee-ring crop `(1420, 0, 300, 320)` is the only retained case-stain event. It appears at Age **0.60** with fixed geometry and 22% opacity. The other former case-stain events and the entire old wear atlas are removed from runtime.

Each of the 32 keys combines shared clean material with its own tint, polish reflection and independently seeded native microdust. Dust opacity grows gradually. Seven tiny dirt flecks appear per key at `0.18 + (MIDI note − 53) × 0.023`, keeping fixed positions, shapes, sizes and intensity afterward. No photographic stain stamp is repeated across keys or copied from the case. White-key yellowing grows gradually; final rendered uniqueness and visibility require the rebuilt fixture.

Only **MIDI 60 at Age 0.56** receives a native curved hairline: 0.32 design pixels wide at fixed 0.55 opacity. It starts at 78% of its placed length and reveals the remaining 22% toward maximum Age. The matching path is recorded in `Assets/Photoreal/key-hairline-personal.svg`; the two generated crack PNGs were rejected for their broad halos and are not embedded. No second key cracks.

The revised knob image has a separate black top index centred at normalized image coordinates `(0.495, 0.300)`. Fine rim stipple and lower-grip dust use independent seeds for each knob and increase gradually. The former copied corrosion patches are gone. The user's replacement Volume knob is not used as an original-part geometry reference.

Corrected fader caps retain the older compact footprint. Their light strip runs across the **top and down the sides**, leaving the broad front black. The VCO mixer cap is lost at Age **0.64**, and the Release cap at **0.88**; exposed flat, stepped blades follow the user's uncapped SH-09 photographs. Their dimensions and alloy remain estimates. Both faders retain their normal hit targets, range and audio operation. The bender-tip break at **0.92** is also cosmetic. These deterministic event thresholds are artistic choices rather than an elapsed-years simulation.

Aged branding removes coverage only inside the exact supplied KEVINBERG 09 / KB-09 and Toyland letter masks. Native labels and calibration ink warm with Age. Personal-unit and other primary photographs inform reconstruction; they are not pasted into the panel, and no Universal Audio graphic is runtime artwork. [PERSONAL_SH09_CONTROL_REFERENCE.md](PERSONAL_SH09_CONTROL_REFERENCE.md) and [GUI_MATERIAL_AGING.md](GUI_MATERIAL_AGING.md) give evidence and limits.

| Runtime asset family | Source resolution |
|---|---:|
| Clean/personal aged chassis and coffee-ring source | 1774 × 887 |
| Revised rotary knob | 1254 × 1254 |
| Corrected fader cap | 1536 × 1024 |
| Clean key pair | 1254 × 1254 |
| Supplied / aged branding masks | 2172 × 724 |

The final source plan embeds **eight PNGs**: `chassis-black`, `chassis-personal-aged`, `chassis-stains` (coffee-ring crop only), `knob-personal`, `fader-personal-v2`, `keys`, `marks` and `marks-aged`. Runtime needs no external image files or network. [The artwork inventory](../Assets/Photoreal/README.md) records provenance. Persistent images use CPU-backed storage and the Windows editor selects its own JUCE software renderer. System Arial is requested with fallback. `Assets/Branding.h` supplies the public identity; the legacy branding parameter is an inert compatibility slot.

Text and vector details render at output scale. A 3840-pixel export does not turn the 1774-pixel chassis into a native 4K photograph. Physical 4K, mixed-monitor DPI and broad host/accessibility coverage remain unavailable or untested.

## Historical GUI evidence and current validation boundary

The **0.11** fixture recorded 280 assertions and zero failures in `artifacts/Validation-KB09/gui-0.11-aging-events-final.txt`, with previews in `artifacts/GUI-0.11-final`. It covered parameter/state mappings, switches, keyboard feedback, bender positions and transformed footer geometry. Its old atlas/event checks do not validate the replacement native wear, new event thresholds, revised cap/knob images or new S sizing. Configured value-bubble contrast was 12.68:1; native hover-popup placement was not observed and this was not a complete accessibility audit.

Historical exports included 2880 × 1620 and 3840 × 2160 images. Native standalone observations for that checkpoint recorded M 1051 × 642 and L 1280 × 720 logical pixels with complete footers on a 1920 × 1080 physical display at 150% scaling. These are earlier-build observations, not a physical 4K test or a completed 0.12 review.

Current build hashes, validator results and installation belong in [VALIDATION.md](VALIDATION.md). A new build and visual review are still required for the source/artwork described here. [LIVE_TEST.md](LIVE_TEST.md) separates the earlier Ableton host observations; Live 0.11 was not retested after three unsuccessful foreground-process recovery attempts.
