# 1.2.1 — Toyland KB-09

Fixed the bare chassis flashing while dragging AGE during parameter automation. The editor keeps the last complete instrument preview while building a replacement bank; incomplete banks never replace it. A complete startup preview also keeps the first AGE drag responsive. Releasing AGE restores live control positions and keyboard feedback. The audio engine, parameter IDs and preset definitions are unchanged from 1.2.0.

# 1.2.0 — Toyland KB-09

The filter now models four coupled OTA stages and the opposing-diode resonance limiter described in the SH-09 service schematic. Self-oscillation comes from the filter feedback loop, with stronger output and restrained harmonic colour. Eight-times oversampling and continuous AGE changes support nonlinear headroom and asymmetry. This is a reduced circuit model; hardware-level accuracy has not been established.

The bank now contains 24 factory sounds, preserving the original sixteen definitions and order and appending eight bass, PWM, expression and filter-feedback patches across AGE 0–10. The manual covers the filter model, new sounds and system recommendations. The GUI and textured strip remain as released in 1.1.3.

Presets, parameter IDs and automation formats are preserved. Self-oscillating patches may need cutoff/level adjustment: the representative fresh case is 3.23 dB stronger and 145 cents higher at the same cutoff. The measured core costs about 2.4 times as much CPU; start at 44.1/48 kHz. Reported latency remains 128 samples.

Windows: six test suites, 548 GUI checks, 47 Steinberg VST3 tests, strictness-10 headless pluginval and isolated installer verification passed. The actual VST3 host passed 75 assertions but waited at shutdown; this is not a clean host exit. Similar GUI-validator shutdown waits were reproduced on released 1.1.3. Mac: universal standalone/AU/VST3 built; six suites, 548 GUI checks and Apple AU validation passed on the arm64 CI runner. Intel execution and comparison against Kevin's hardware remain to be evaluated. Full measurements and limitations are recorded in the filter validation artifacts and model notes.

Existing shared Drive folder and download file IDs are retained. Both current ZIP downloads must be hash-verified before publishing the update announcement.

# 0.13.2 — Toyland KB-09

This update brings the owner's final control/material refinements into the Windows VST3 and standalone build. A real Windows AAX Native development bundle also compiles from the shared source; it is **unsigned and untested in Pro Tools**, so it belongs in the package's clearly labeled Development area. Native AU configuration and Mac build instructions are supplied, but no AU binary has been built because no Mac/Xcode host is available. A Mac source kit is not an installable Audio Unit.

- The selected v5 photographic knobs retain their full physical barrel height, near-overhead view, silver skirt and grouped ribs alternating with smooth panels. Silver top insets and black rims are narrower. The native black top tick is approximately one third of the silver top's diameter; photographed lighting stays fixed as the tick rotates, and no collar tick is drawn.
- Fine irregular micro-pitting develops across each silver top and skirt with independent deterministic patterns. Reflections dull progressively with AGE so the knobs age with the panel; returning AGE to zero restores the clean metal. This is photo-informed illustrative wear, not a measured corrosion law.
- The POWER area follows the owner's close-up: diffuse grey plastic, a broad rounded bevel, substantial raised OFF position and lower ON position, filled button-profile ON/OFF symbols, restrained print wear and the lamp below the legend. Its two surrounding screws are aligned with the cap and age differently: a dark tarnished upper head and broader pale oxidation below. Height, travel, alloy and chemistry are not measured hardware claims.
- The approved top-down fader caps retain their stripe across the top and down the sides, with black front lips. Thin exposed fader blades and the rigid extruded bender retain the shared 9° native camera target; the bender's disk and pointer rotate together. Matte key aging, the almost hairline single crack, fine textured lower rail, moved coffee ring and corrected GATE / + / TRIG selector legends remain.
- Click-and-hold mouse playing now shows the effective note's amber bar immediately at the key's lower edge. Presses, upward/downward drags and release update before the next audio callback; MIDI and mouse retain the same lowest-held-note priority, with other held notes blue.
- The offline HTML guide and release image are updated for 0.13.2. The desktop package separates usable Windows VST3/standalone files from unsigned AAX development material and Mac build source, with explicit format status and preserved dependency notices. Exact assembled files and installation status are recorded separately.
- Windows file/product version resources now regenerate when the project version changes. Explorer, the plugin manifest and all three Windows binaries report 0.13.2 consistently after an incremental build.

The DSP and parameter/state identities remain unchanged from the preceding release. No additional age-dependent saturation was introduced: the [research](SH09_AGE_DISTORTION_RESEARCH.md) does not establish a universal distortion curve for aged SH-09 hardware. Older Modern patches retain the deeper MOD mapping introduced in 0.11; Authentic mappings are unchanged.

Release-specific test counts, binary hashes, package checks and installed identity belong in [VALIDATION.md](VALIDATION.md). Successful compilation alone does not validate the AAX wrapper or establish a new native DAW, Mac/AU, Pro Tools or physical-display test. [Format status](FORMAT_ROADMAP.md) and [known limitations](KNOWN_LIMITATIONS.md) preserve these boundaries.

# Historical 0.12.0 — Toyland KB-09

Photo-informed controls and AGE wear now follow Kevin's personal SH-09. This Windows VST3/standalone build includes corrected compact opaque caps, stripes across their top and down the sides, native black knob-top indicator ticks, stepped exposed fader blades, raised ribbed switches and an arched bender. The owner's replacement volume knob, sticker and waveform tape are excluded.

- The personal instrument represents AGE 7. Gradual dust collects at seams, slider openings, knob sides and key gaps; ink and individual keys yellow, metal dulls and knob rims tarnish finely. Gray screw heads replace red-rust stamps; brown corrosion follows exposed panel paint. The existing envelope coffee ring is retained. The old generic wear atlas is excluded from runtime.
- MIDI 60 alone receives a nearly hairline crack, with no broad shadow or halo. It appears at displayed AGE 5.6 and can lengthen slightly. The VCO mixer cap disappears at 6.4, RELEASE at 8.8, and the bender tip breaks at 9.2. These discrete damage events leave the controls functional and reverse when AGE is reduced.
- S joins M/L: approximately 65%, 90% and the full available work area. Complete header/panel/footer fitting remains uniform. A small unofficial/unaffiliated/not-for-sale notice is printed above the panel.
- The personal test ZIP includes VST3, standalone, 16 exported factory sounds and a branded offline HTML guide. It documents the owner's confirmed Live Auto-Scale remedy and individually sourced settings for other DAWs. AU/macOS and AAX remain future builds.

All current checks pass: DSP 92/0, processor 20/0 with zero tracked allocations, preset files 58/0, compiled GUI 278/0, Steinberg 47/0, actual-VST3 host 75/0 including a 600-second render, and GUI-enabled Pluginval strictness 10 with exit 0. The installed module matches the tested build. Exact hashes and evidence boundaries are in [VALIDATION.md](VALIDATION.md). Physical 4K/mixed-DPI and new-build owner Live recall remain separate checks.

DSP and parameter/state identities are unchanged from 0.11. Older Modern patches still have the deeper MOD mapping introduced in 0.11; Authentic mappings are unchanged.

# Historical 0.11.0 — Toyland KB-09

This release adds user-preset files, deeper Modern modulation, revised black-metal materials and Age wear, clearer keyboard/value feedback, and a correction to Windows editor sizing. The single Toyland KB-09 identity, VST3 identifiers, 35 parameter slots and schema-1 sound state remain compatible. **Older Modern patches with nonzero MOD can sound deeper** because the Modern depth mapping has changed.

## Changes

- The smooth black-painted chassis has restrained reflections. Age gradually develops rust/patina and yellowed ink/keys on the main face, fold and performance bay, tarnishes knobs and chips branding strictly inside the supplied original letter masks. Six case stains appear separately at 0.25/0.38/0.49/0.60/0.72/0.86 with fixed size and 22% drawing opacity. Registered aged screw patches preserve their panel positions; the clean front rail is dulled natively.
- Each key has a distinct tint and its own independent photographic stain, used once. Key stains appear individually from Age 0.18 to 0.893 and retain their size/opacity. Transparent cracks start on MIDI 60/77 at 0.82/0.94, initially showing 78% of their eventual length, then extending the remaining 22%. All fader caps remain fitted; the unverified exposed-post depiction has been removed. Nine runtime PNGs provide separate patina, case stains and a 37-mark atlas: 32 unique key stains, two independent fractures and three independent rotary corrosion patches. No mirrored fracture or repeated aged-key/knob stamp is drawn. Age remains an illustrative condition/tolerance profile, not a measured aging curve.
- The bender depicts a curved PB-4-style sector beneath a horizontal opening, with a projecting tab and spring return. Its inferred front-to-back axle and ±35° projected travel are engineering estimates supported by cross-model PB-4 documentation, not measured SH-09 mechanics. Compact physical switches, calibrated faders and detented upper rotary arcs remain.
- M now uses 90% of monitor work-area dimensions, about 20% larger linearly. M/L sizing uses actual native child-client pixels independently of host movement eligibility, optional measurable surrounding chrome, and a bounded initial fit after peer scale stabilizes. The complete footer scales within standard, wide and tall editor rectangles. The helper does not change process-wide DPI awareness or the DAW's renderer; host-owned frame restrictions still apply.
- SAVE/LOAD use native asynchronous dialogs for `.kb09` presets and compatible earlier `.sh09state` exports. Validated schema-1 current sound state is saved separately from the retained sixteen-program factory bank. Invalid loads leave the current patch unchanged. Checked temporary writes protect existing files; weak callback/owner lifetime checks cover editor closure.
- Loaded user filenames appear in the preset field. Recalling a factory program resets the displayed name even when its index matches the stored reference, using a program revision counter.
- Modern LFO MOD maxima increase to **±36 semitones VCO / ±8 octaves VCF**. Authentic retains **±12 semitones / ±5 octaves**. These engineering ranges are unmeasured; the separate bender depth controls are unchanged.
- Held white and black MIDI notes now have verified visible feedback. Value-bubble colours have a checked **12.68:1 configured contrast ratio**; this is not a full accessibility certification.

## 0.11 validation record

Final October 7 results apply to VST3 module SHA-256 `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`, identical in the build, staged bundle and installed location. The standalone SHA-256 is `93F1660185E8C67781D0118DAED461598080580EB30976AB6D5C11D227E6216A`.

| Check | Result | Evidence under `artifacts/Validation-KB09` |
|---|---|---|
| DSP | 92 passed, 0 failed | `dsp-0.11-materials-final.txt` |
| Processor | 20 passed, 0 failed; 0 tracked C++ allocations | `processor-0.11-aging-events-final.txt` |
| Preset files | 58 passed, 0 failed | `presets-0.11-aging-events-final.txt` |
| Compiled GUI | 280 assertions, 0 failures | `gui-0.11-aging-events-final.txt` |
| Steinberg validator | 47 passed, 0 failed; exit 0 | `steinberg-0.11-final.txt` |
| Pluginval strictness 10, GUI enabled | SUCCESS; confirmed exit 0 | `pluginval-0.11-final-level10.txt`, `pluginval-0.11-final-exit.txt` |
| Actual VST3 host | 75 passed, 0 failed; 600-second render | `host-0.11-final.txt` |

GUI exports and component metadata are in `artifacts/GUI-0.11-final`, including 2880 × 1620 and 3840 × 2160 images and bender positions. Tests cover footer bounds at wide/tall sizes, held-key feedback, configured contrast, fixed stain events, separate crack onsets and distinct healthy/aged samples of all 32 keys. Subtle individual polish reflections prevent duplicated healthy samples at 8-bit output precision. A physical 4K display was unavailable; the chassis remains a **1774 × 887 source raster**. Native hover-popup placement was not observed.

The final VST3 is installed at `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; both release formats are staged in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`. The actual VST3's 600-second offline render consumed **51.0971 seconds of callback time**, with a maximum measured 128-sample callback of **1.006 ms** at 48 kHz. This is a single offline measurement, not a DAW CPU guarantee. The preceding 0.11 standalone M/L were observed at **857 × 534 / 1280 × 720 logical units**, with the complete footer, on a 1920 × 1080 physical display at 150% scaling.

Live-specific 0.11 checks were not repeated: desktop automation could not resolve Live's foreground process after three recovery attempts. Earlier Live observations remain explicitly scoped in [LIVE_TEST.md](LIVE_TEST.md). [VALIDATION.md](VALIDATION.md) and [ACCEPTANCE.md](ACCEPTANCE.md) record current software-test scope and artifact identity; actual-module tests do not substitute for every Live workflow.

## Historical 0.10 checkpoint

Version 0.10 established the Toyland identity, photographic-style panel, power control and native window actions. Its October 5 records contain 83 DSP, 17 processor, 94 GUI, 75 actual-VST3 host and 47 Steinberg passes, plus GUI-enabled pluginval strictness 10 with confirmed exit 0. CPU-backed artwork and software rendering on the editor's own Windows peer avoided the observed NVIDIA-driver shutdown wait. Those results describe that binary and do not certify 0.11. Installation/recall status is recorded centrally rather than inferred from these older logs.

## Fidelity scope

The instrument remains a circuit-informed approximation. Physical SH-09 captures, a validated full-circuit SPICE reference and a completed hardware listening A/B are unavailable. Waveform shape, source gains, saturation, filter/follower behaviour, control tapers and condition offsets retain the explicit [known limitations](KNOWN_LIMITATIONS.md). Artwork is a finite-resolution reconstruction, not measured panel CAD. Generated assets and factory patches are original project content; branding marks were supplied by the user. No Universal Audio graphics or commercial presets were copied.
