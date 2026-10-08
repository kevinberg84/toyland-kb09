# Production artwork provenance — 0.13.0

The raster material sources were produced with the built-in image-generation tool, guided by original-unit research and the user's personal photographs. The user's corrected `KB-09.png` supplies `marks.png`. Reference photographs and Universal Audio artwork are not embedded in the runtime. Current controls combine generated materials with native geometry; a generation prompt is production intent, not proof of exact hardware dimensions or final rendering quality.

## Current embedded sources

| Runtime file | Actual source pixels | Production record / current use |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | [chassis-black-prompt.txt](chassis-black-prompt.txt); neutral-black clean case |
| `chassis-personal-aged.png` | 1774 × 887 | [chassis-personal-aged-prompt.txt](chassis-personal-aged-prompt.txt); registered personal-unit wear endpoint |
| `chassis-stains.png` | 1774 × 887 | [chassis-stains-prompt.txt](chassis-stains-prompt.txt); only the existing coffee-ring crop is currently used |
| `knob-personal.png` | 1254 × 1254 | [knob-personal-prompt.txt](knob-personal-prompt.txt); approved material source mapped onto native knob geometry |
| `fader-personal-overhead.png` | **1484 × 1060** | [fader-personal-overhead-prompt.txt](fader-personal-overhead-prompt.txt); near-top cap with central top-and-side stripe |
| `front-rail-grain.png` | 2172 × 724 | [front-rail-grain-prompt.txt](front-rail-grain-prompt.txt); selected fine matte lower-rail texture |
| `keys.png` | 1254 × 1254 | [keys-prompt.txt](keys-prompt.txt); clean materials over individual native key surfaces |
| `marks.png` | 2172 × 724 | User-supplied corrected artwork; no generation prompt |
| `marks-aged.png` | 2172 × 724 | [marks-aged-prompt.txt](marks-aged-prompt.txt); worn coverage constrained to original letter masks |

The current CMake `KB09Artwork` list embeds these **nine PNGs**. Header-checked dimensions above take precedence over requested dimensions inside prompts. In particular, the overhead cap is 1484 × 1060; 1536 × 1024 describes earlier cap files, not this selected file.

## Selected material mapping

The approved knob prompt describes an oblique original material image, with no baked black indicator. The current renderer does not use that oblique silhouette. It samples top material from `(283, 101, 674, 550)` and rib material from **x230–1010 / y715–945**. Native geometry supplies a `0.50d` barrel height, `0.34d` body radius, `0.425d` skirt radius and `0.30d` top radius, projected at **18° off normal**. The top centre is `(0.50d, 0.40d)`; the separate moving black tick uses radii `0.175d–0.287d` and width `0.026d`. The material lighting remains fixed. Later attempts to generate a revised camera flattened the barrel and were rejected; those new knob candidates are not runtime inputs.

The overhead cap prompt requests an almost direct top view, preserving the physical cap rather than squashing its walls. The current renderer takes source crop `(125, 242, 1240, 565)` and draws it at **28 × 18 body units** over an opaque solid core. The index crosses the centre of the top surface and descends the narrow sides; the small vertical front lip remains black. Prompt camera language and the native knob camera are separate production records, not calibrated estimates of the user's camera.

The selected lower-rail texture came from `exec-e284c8b1-b01a-45b5-bcc1-5bd7ee2a219c.png`. Its saved prompt names the owner's `20261007_131213.jpg` and the discarded coarse candidate `exec-f8f3f01a-d393-4c33-9eac-13e3d695a36b.png` as references. The final instruction calls for fine, low-contrast matte grain; its dimensional/grain counts are requested art direction, not measured texture dimensions. Runtime samples the top 70 source rows for the narrow rail and leaves the smoother main painted panel distinct.

The aged chassis blends at `0.90 × a^0.85` on metal regions. Four registered performance-bay screw patches are composited above the perimeter rules; their aged source blend is `0.94 × a`. Native white/grey deposits are then drawn on all 13 screw heads after the ink. Neither the generation prompts nor this pale-deposit treatment identifies their alloy or chemistry.

Only source crop `(1420, 0, 300, 320)` from the old stain image is retained: the coffee ring, at fixed 22% opacity after `a = 0.60`. Its target moves +40 right and −45 up in body units to clear the envelope-mode control. GATE / + / TRIG is above the axis, GATE left of the middle, and LFO below.

## Native work and rejected inputs

The following details are native drawing, not additional embedded bitmap assets:

- Independent per-key tint, microdust and seven fixed flecks, with per-key onset `0.18 + (note − 53) × 0.023`. No old wear-atlas cell is sampled.
- Reduced key gloss. At display AGE 7, the clean black and white bitmap layers contribute 0.412 and 0.552 respectively over opaque native surfaces; highlights also fade. This is surface compositing, not key transparency.
- One curved key hairline on MIDI 60 from display AGE 5.6, width 0.32 body pixels and fixed 55% opacity. [key-hairline-personal.svg](key-hairline-personal.svg) records the path; its revealed length grows from 78% to 100%.
- Native knob cylinder/skirt, the moving black tick, independent fine rim stipple and grip dust.
- Raised ribbed slide switches, thin stepped exposed blades after cap-loss events at display AGE 6.4/8.8, and the bender's cosmetic broken tip at 9.2. Interactions and spring return remain active.
- Pale ivory POWER-button bevels, stable molding grain, recess and state movement.
- White/grey screw deposits, panel glyphs, scales and software controls.

`wear-atlas.png` is entirely removed from runtime. Its old independent-cell checks belong to historical releases. The former six-stain case sequence and two-crack sequence are also historical. Both generated crack PNGs were rejected for broad shaded halos; no crack PNG is embedded. Older cap files (`fader.png`, `fader-personal.png`, `fader-personal-v2.png`) and earlier aged-key/knob/patina files remain provenance only. Retaining a file or prompt does not make it a runtime dependency.

The former 0.12 eight-PNG inventory is superseded by the table above. Generated camera candidates that flattened the knob, and the coarse rail texture candidate, were not integrated. Original material files and prompts are retained to explain what was selected or rejected, not to silently substitute old artwork in a build.

## Validation boundary

The supplied branding remains KEVINBERG 09 / KB-09 on the left and Toyland on the right. Aged masks only remove coverage within the supplied letterforms. Playback needs no network, generation service or external image files.

Native text, paths and geometry render at the output pixel scale, while the photographic materials retain their finite source detail. A 3840 × 2160 compiled export establishes that output size, not native 4K photography, physical 4K-display validation or measured equivalence to a real instrument. Prompts and source integration do not establish a passing final build; [VALIDATION.md](../../docs/VALIDATION.md) owns the release-specific test, hash and installation evidence.
