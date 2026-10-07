# 0.11.0 — Toyland KB-09

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

## Current validation record

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
