> **Historical geometry research.** Source versions and pending statuses below describe the earlier revision under study. Use the [current manual](manual/index.html) and [approved interaction contracts](APPROVED_INTERACTIONS.md) for the released appearance.

# SH-09 control geometry and printed legends

Research inspection: 2026-10-07. Scope: physical references for the Toyland KB-09 interface, not a claim of measured replica geometry. Coordinates below refer to the 1500 × 750 instrument body; the software toolbar adds 34 pixels above it. Photographs are research references, not proposed runtime artwork.

Later the same day, the user's close-ups resolved additional details. See [personal control reference](PERSONAL_SH09_CONTROL_REFERENCE.md): GATE + TRIG has a separate printed plus sign; uncapped faders expose flat metal blades with wider shoulders; and cap strips cross the top and descend the sides, leaving the broad front black. These observations supersede the earlier shoulder/front-band interpretation. Exact blade dimensions and alloy remain unmeasured. The 0.12 source now incorporates personal-photo guidance; a completed new build or visual acceptance is not claimed by this research document.

## Evidence actually inspected

- Roland **SH-09 owner's manual**, printed pp. 2–6 and 11: panel drawing, control names, waveform and envelope symbols. Local `Research/ROLAND_SH-09_OWNERS_MANUAL.pdf` and rendered pages; [original-document scan](https://thismodernday.com/wp-content/uploads/2021/07/Roland-SH-09-Owners-Manual.pdf). Printed page 2 is PDF page 3. The scan is not a dimensioned panel drawing.
- Roland **SH09 Service Notes**, March 27, 1980, pp. 1 and 3: original front view, part identities and external dimensions. Local `Research/ROLAND_SH-09_SERVICE_NOTES.pdf`; [original-document scan](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf). Part labels include “Knob no.57 rotary” (`016-057`), “Knob no.33 slider” (`016-033`), and the separately listed “Strip no.12” (`063-012`). The notes do not specify the strip material, top-disc alloy, or complete control dimensions.
- Repairer's **two original units**, photographed at two oblique angles in one frame. Visually inspected `Research/SH09_front_reference.jpg`: [repair page](https://snw.lonningdal.no/sh09.php), [original photograph](https://snw.lonningdal.no/sh09/Roland_SH-09_repair_01.jpg). These informed the initial stepped-knob and panel-symbol study; the later personal close-ups resolve finer cap construction more reliably.
- **C. Y. Kong's own unit**, including a near-top view and close panel detail. Visually inspected `Research/SH09_Kong_top_reference.jpg` and `Research/SH09_Kong_panel_reference.jpg`: [owner's account](https://www.cykong.com/Synths/Roland%20SH-09/RolandSH09.htm), [near-top image](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Top.jpg), [panel detail](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Front%20Panel1.JPG). The unusual surface damage belongs to this particular neglected unit; it is not a factory finish specification.
- Syntaur's own **replacement-part descriptions**, supplemental only: [rotary replica](https://syntaur.com/Part-3224-Knob-rotary-Roland) states a 25 mm base and explicitly warns that the indicator alignment is inaccurate; [slider replica](https://syntaur.com/Part-3225-Slider-knob-Roland) identifies replacement of `016-033`. Their enlarged product images were unavailable through the browser fetch and were **not** visually inspected. Replica dimensions are corroboration, not measurements of an original SH-09.

The original-unit photographs, manual renderings, and then-current `Assets/Photoreal/knob.png` / `fader.png` were displayed and compared directly in the initial pass. The subsequent personal-photo inspection is documented separately. No new remote photographs were downloaded. No claim is made about unseen repair images, calibrated material colour, or physical measurement.

## Large knobs: observed construction and rendering guide

The Range, Waveform and Volume knobs share a stepped appearance: a broad flared silver skirt at panel level, a narrower black grip with fine vertical ribs, and a flat silver top disc. The index is a short dark radial mark near the top-disc rim. It is not a long needle beginning at the centre. Lighting changes the apparent silver colour between the units; the photos do not establish a specific alloy.

The initial generated knob had a top disc almost as wide as the outer skirt and a narrow base ring. The reference showed a more pronounced width change between the grip and flared skirt. This was the reason for revising the profile. The current 0.12 source uses `knob-personal.png` (1254 × 1254), with a separate native black top indicator centred at normalized image coordinates `(0.495, 0.300)`. The user explicitly requested that black tick remain. The personal unit's replacement Volume knob is excluded as an original-part reference.

Suggested **estimated** rendering proportions, relative to outer skirt diameter `D`:

| Feature | Practical target | Confidence |
|---|---|---|
| Outer silhouette | `D = 62–70` body pixels, excluding soft shadow | 25 mm replica converts to 62 px at 1500/605; photograph/camera allowance, not a measured original |
| Ribbed grip width | `0.72–0.80 D` | Visible in both original units; approximate projection |
| Silver top width | `0.68–0.74 D` | Photo estimate |
| Projected black front sidewall height | `0.12–0.20 D` | Depends on camera elevation; not physical cylinder height |
| Exposed skirt beside grip | approximately `0.10–0.14 D` each side | Photo estimate; smooth outward flare, not another ribbed tier |
| Index | about `0.11–0.16 D` long and `0.02 D` wide; outer end near disc rim | Photo estimate; rotate the mark around the projected disc centre |

Use a modestly flattened top ellipse and visible front sidewall consistently with the panel's camera angle. Keep studio illumination fixed while turning the index; neither the whole lighting pattern nor the cast shadow should orbit. Do not infer a precise rib count from the available pixels. The small Tune control is a separate inconspicuous black control immediately above Range; it should not inherit the large silver-knob artwork.

The original rotary marks occupy an upper arc. Current approximate detents are reasonable: Range `−65, −32.5, 0, 32.5, 65°`; Waveform `−60, −20, 20, 60°`, measured from vertical. These are **engineering angles**, not service-manual specifications. Preserve parameter ordering and align the rotating index to the matching printed mark.

## Fader caps and short slide switches

The personal cap photographs `130925/130929/130934/130940/130953/130955` resolve compact black molded parts with gently rounded edges and a thin light strip **across the top and down the sides**. The broad front face stays black. The earlier interpretation of a horizontal strip around the front/shoulder was incorrect. The first asset's inset capsule was also inaccurate, although the user preferred its compact screen footprint to the subsequently generated tall cap. The corrected `fader-personal-v2.png` (1536 × 1024) follows that older footprint with the corrected strip path; source integration and rebuilt visual review remain separate steps.

The source's main-cap drawing rectangle is **30 × 23 body pixels** (compact lower-bay controls use 25 × 23), with image padding affecting the actual visible silhouette. Preserve the user's preferred earlier apparent footprint when changing the bitmap; do not equate the full texture rectangle with the physical cap. A thin top strip around **1–1.6 projected pixels** is a starting estimate, with visible continuity down the sides rather than rounded capsule ends on the front. These are screen-space choices, not millimetre measurements. The service parts list identifies the cap and a separate strip but does **not** prove ivory plastic versus painted or bare metal.

The earlier generic metal rectangle was unsupported, and the previous checkpoint therefore retained all caps. The user's `131114/131131/131137` photographs now establish the original-unit exposed actuator's flat blade, narrow bright cut edge and wider stepped shoulders. Current 0.12 source replaces the VCO mixer cap at Age 0.64 and Release cap at 0.88 with native photo-informed blades. Exact protrusion, dimensions and alloy remain estimates. The event is cosmetic; slider geometry, ranges and operation stay active. A replica or another model's actuator is not used to establish those missing dimensions.

The six mode switches are short vertical mechanisms with black ribbed actuators. Their printed legends spread farther apart than their physical throw. Current 24 px total throw inside a roughly 44 px mechanism is a workable photographic estimate; it must not become a full-length fader. Preserve the existing parameter orders shown below. The source switches' top/bottom captions sit close to the actuator axis, with middle captions to one side; placing every legend as one text column is less faithful.

## Exact legend and symbol corrections

The table transcribes short functional panel labels and describes native-symbol corrections incorporated into the 0.12 source plan. Its action wording records the original implementation guidance; it is not a list of verified failures in a new compiled build. Personal close-ups resolve details missed by the small web photographs. Native stroked paths keep symbols clear when resized.

| Location | Original visible print / symbol | Current-source difference and action |
|---|---|---|
| MODULATOR source | Sine pictogram; square pictogram; `RANDOM` / `(S/H)` | Retain textual accessible names; paint actual waveform paths. Current LFO painter already has a basic glyph implementation. |
| MODULATOR faders | `RATE`; sine pictogram plus `DELAY` / `TIME` | Replace the short `DELAY` caption. The icon indicates that delay applies to sine. |
| VCO tuning | `TUNE`, with thin leader; `RANGE` below | Current Tune centre at x356 above Range is correct. |
| Range arc | `32′`, `16′`, `8′`, `4′`, `2′` | Preserve clockwise order. Prime typography is preferable to an apostrophe. |
| Waveform arc | `NOISE`, sawtooth pictogram, square pictogram, narrow-pulse pictogram | Replace `SAW`, `SQR`, `PULSE`. Keep their selection semantics and the collision-avoiding NOISE placement. |
| Pulse-width group | `PULSE WIDTH MOD` across a bracket; source sine pictogram + `LFO`, `MANUAL`, `ENV` with positive envelope contour | Remove the isolated `PULSE` / `WIDTH` fader label; add shared group print without moving controls. |
| SUB OSC | Square above `1 OCT` / `DOWN`; `2 OCT` / `DOWN` plus square; `2 OCT` / `DOWN` plus narrow pulse | Bottom caption is not `2 OCT PULSE`. Orders top/middle/bottom remain parameter values `0,1,2`. |
| AUDIO MIXER | `VCO`, `SUB` / `OSC`, `EXT` | Current wording matches. |
| VCF faders | `CUTOFF` / `FREQ`, `RESONANCE`, `ENV`, `MOD` | Current wording matches. |
| VCF source | `ENV` / `FOL'R`, then `ENV` with positive envelope contour, then `ENV` with inverted contour | Replace printed plus/minus abbreviations with contour paths. Existing displayed parameter order `2,0,1` is correct. |
| VCA source | `HOLD`, `ENV` with positive contour, `GATE` with rectangular gate pulse | Preserve displayed parameter order `0,1,2`. |
| ENV source / faders | Stacked `GATE` / `+` / `TRIG`, then `GATE`, then `LFO`; `A D S R` | Personal photo `131048` clearly resolves the plus sign; preserve all three printed rows. |
| POWER | `POWER`, beside small button-state symbols with `ON` / `OFF` | Add native state pictograms and the missing labels; do not relocate the button or lamp. |

At body scale 1500, waveform glyphs around **13–18 × 7–9 px**, with **0.8–1.0 px strokes**, are appropriate. Draw sawtooth as a rising ramp and vertical reset; square as an even high/low step; narrow pulse with a short high plateau. Envelope contours need an attack peak, decay to a sustain level, sustain segment and release; inverted contour mirrors vertically. These pictograms identify functions; they are not oscilloscope measurements.

## Age and yellow keys

Both units in the repair photograph show uneven warm key colour, dark side gaps and localized marks. Rear key portions and sidewalls can appear warmer than front playing surfaces. Some adjacent keys remain relatively clean. Kong's whole-unit photograph is substantially warmer overall, but its illumination and white balance prevent calibrated colour comparisons.

Use a gradual, restrained yellow/cream material shift with distinct per-key variation; let rear/side shading and surface polish remain visible. Separate this from fixed local damage events. A repeated rectangle over every key, identical stain placement, or uniformly saturated yellow obscures the physical construction. Dirt and yellowing should not change key boundaries or turn white-key highlights grey. Exact Age-to-colour curves, event onsets and damage placement are artistic design choices; the reference images establish plausible appearance only. No chemical aging model or original-unit ageing rate has been measured.

The current 0.12 source removes the old wear atlas entirely. Each key uses independent native microdust and seven small fixed dirt flecks at its own onset, together with individual tint/polish. Only MIDI 60 receives the native 0.32-design-pixel curved hairline at Age 0.56, fixed opacity 0.55 and 78–100% revealed length. No crack PNG is embedded. The case keeps only the original coffee-ring crop at Age 0.60; knob-rim stipple and lower-grip dust use independent seeds. [GUI_MATERIAL_AGING.md](GUI_MATERIAL_AGING.md) records the complete source plan and its pending validation boundary.

## Implementation boundary

This research informs native silkscreen corrections and separately created artwork. It does not establish CAD-level dimensions, factory colour values, exact cap-strip material, measured blade dimensions, selector angles or a calibrated aging chronology. Toyland branding, the project notice in the top body area and software utility strips remain product additions. Source sizing is S at 65%, M at 90% and L at the available monitor work area. Rendering should be reviewed at all three sizes and in 4K output after the changes. Finite raster sources remain finite, and no physical 4K-display validation is claimed. Inspecting the references alone is not visual acceptance of the completed plugin.
