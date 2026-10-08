# Production artwork provenance — 0.13.1 source

The raster material sources were produced with the built-in image-generation tool, guided by original-unit research and the user's personal photographs. The user's corrected `KB-09.png` supplies `marks.png`. Reference photographs and Universal Audio artwork are not embedded in the runtime. Current controls combine complete generated sprites with native geometry and overlays. A generation prompt is production intent, not proof of exact hardware dimensions or final rendering quality. This describes 0.13.1 source; no new build/test pass is claimed.

## Current embedded sources

| Runtime file | Actual source pixels | Production record / current use |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | [chassis-black-prompt.txt](chassis-black-prompt.txt); neutral-black clean case |
| `chassis-personal-aged.png` | 1774 × 887 | [chassis-personal-aged-prompt.txt](chassis-personal-aged-prompt.txt); registered personal-unit wear endpoint |
| `chassis-stains.png` | 1774 × 887 | [chassis-stains-prompt.txt](chassis-stains-prompt.txt); only the existing coffee-ring crop is currently used |
| `knob-personal-overhead-v2.png` | 1254 × 1254 | [knob-personal-overhead-v2-prompt.txt](knob-personal-overhead-v2-prompt.txt); complete camera-edited sprite with a separate native moving tick |
| `fader-personal-overhead.png` | **1484 × 1060** | [fader-personal-overhead-prompt.txt](fader-personal-overhead-prompt.txt); near-top cap with central top-and-side stripe |
| `front-rail-grain.png` | 2172 × 724 | [front-rail-grain-prompt.txt](front-rail-grain-prompt.txt); selected fine matte lower-rail texture |
| `keys.png` | 1254 × 1254 | [keys-prompt.txt](keys-prompt.txt); clean materials over individual native key surfaces |
| `marks.png` | 2172 × 724 | User-supplied corrected artwork; no generation prompt |
| `marks-aged.png` | 2172 × 724 | [marks-aged-prompt.txt](marks-aged-prompt.txt); worn coverage constrained to original letter masks |

The current CMake `KB09Artwork` list embeds these **nine PNGs**. Header-checked dimensions above take precedence over requested dimensions inside prompts. In particular, the overhead cap is 1484 × 1060; 1536 × 1024 describes earlier cap files, not this selected file.

## Selected material mapping

The latest user selection supersedes the interim pre-photo restoration: the exact starting object is `exec-8bcd38b0-7cb9-4ce3-99bc-80b29603b296.png`. The user requested approximately 90% overhead and a black top tick, expressly preserving physical barrel height, silver skirt, and clustered ribs alternating with smooth panels. The [saved generation chain](knob-personal-overhead-v2-prompt.txt) records the first camera/tick edit `exec-6d78635e-3f6e-4178-b2ff-835b7ecab191.png`, the further overhead preview `exec-2defd67b-201c-4291-88aa-d7dce8425684.png`, and the clean runtime texture `exec-01df3e3b-48f0-4629-967c-49300119b0a3.png`. The last edit requests removal of only the baked tick so the application can draw a moving indicator; its registration and unchanged pixels are prompt intent, not a claim of pixel-identical generation.

The complete clean sprite is drawn without material sampling into a square control area. Both it and the non-runtime `knob-personal-overhead-v2-preview.png` are **1254 × 1254**. The source's shared camera target is **9° off normal**, sine 0.156434 and cosine 0.987688, also used for native bender/blade projection. The opaque native black tick is centred at `(0.50d, 0.435d)`, spans radii `0.20d–0.329d`, and has width `0.026d`. Only the tick rotates; image lighting stays fixed. Preserving physical height is an explicit editing constraint, not a dimensional measurement established by the generated bitmap.

The 0.13 renderer's **18° native cylinder/skirt/top** and cropped `knob-personal.png` material are historical. Its top crop `(283, 101, 674, 550)`, rib band x230–1010/y715–945 and native radii do not describe the new sprite renderer. Earlier flattened camera candidates remain rejected.

The approved overhead cap is unchanged. Its prompt requests an almost direct top view, preserving the physical cap rather than squashing its walls. The current renderer takes source crop `(125, 242, 1240, 565)` and draws it at **28 × 18 body units** over an opaque solid core. The index crosses the centre of the top surface and descends the narrow sides; the small vertical front lip remains black. Prompt camera language and the native projection target are separate production records, not calibrated estimates of the user's camera.

The selected lower-rail texture came from `exec-e284c8b1-b01a-45b5-bcc1-5bd7ee2a219c.png`. Its saved prompt names the owner's `20261007_131213.jpg` and the discarded coarse candidate `exec-f8f3f01a-d393-4c33-9eac-13e3d695a36b.png` as references. The final instruction calls for fine, low-contrast matte grain; its dimensional/grain counts are requested art direction, not measured texture dimensions. Runtime samples the top 70 source rows for the narrow rail and leaves the smoother main painted panel distinct.

The aged chassis blends at `0.90 × a^0.85` on metal regions. Four registered performance-bay screw patches are composited above the perimeter rules; their aged source blend is `0.94 × a`. Native white/grey deposits are then drawn on all 13 screw heads after the ink. Neither the generation prompts nor this pale-deposit treatment identifies their alloy or chemistry.

Only source crop `(1420, 0, 300, 320)` from the old stain image is retained: the coffee ring, at fixed 22% opacity after `a = 0.60`. Its target moves +40 right and −45 up in body units to clear the envelope-mode control. GATE / + / TRIG is above the axis, GATE left of the middle, and LFO below.

## Native work and rejected inputs

The following details are native drawing, not additional embedded bitmap assets:

- Independent per-key tint, microdust and seven fixed flecks, with per-key onset `0.18 + (note − 53) × 0.023`. No old wear-atlas cell is sampled.
- Reduced key gloss. At display AGE 7, the clean black and white bitmap layers contribute 0.412 and 0.552 respectively over opaque native surfaces; highlights also fade. This is surface compositing, not key transparency.
- One curved key hairline on MIDI 60 from display AGE 5.6, width 0.32 body pixels and fixed 55% opacity. [key-hairline-personal.svg](key-hairline-personal.svg) records the path; its revealed length grows from 78% to 100%.
- The moving black top tick over the complete knob sprite, independent fine rim stipple and grip dust.
- Raised ribbed slide switches, thin stepped exposed blades after cap-loss events at display AGE 6.4/8.8, and the bender's cosmetic broken tip at 9.2. Interactions and spring return remain active.
- Pale ivory POWER-button bevels, stable molding grain, recess and state movement.
- White/grey screw deposits, panel glyphs, scales and software controls.

`wear-atlas.png` is entirely removed from runtime. Its old independent-cell checks belong to historical releases. The former six-stain case sequence and two-crack sequence are also historical. Both generated crack PNGs were rejected for broad shaded halos; no crack PNG is embedded. Older cap files (`fader.png`, `fader-personal.png`, `fader-personal-v2.png`) and earlier aged-key/knob/patina files remain provenance only. Retaining a file or prompt does not make it a runtime dependency.

The former 0.12 eight-PNG inventory and 0.13 native-cylinder knob are superseded by the table above. Generated camera candidates that flattened the knob, and the coarse rail texture candidate, remain rejected. Original material files and prompts are retained to explain what was selected or rejected, not to silently substitute old artwork in a build. The current ticked knob preview is provenance only; the tick-free sprite is the runtime input.

## Validation boundary

The supplied branding remains KEVINBERG 09 / KB-09 on the left and Toyland on the right. Aged masks only remove coverage within the supplied letterforms. Playback needs no network, generation service or external image files.

Native text, paths and geometry render at the output pixel scale, while the photographic materials retain their finite source detail. A 3840 × 2160 compiled export establishes that output size, not native 4K photography, physical 4K-display validation or measured equivalence to a real instrument. Prompts and source integration do not establish a passing final build; [VALIDATION.md](../../docs/VALIDATION.md) owns the release-specific test, hash and installation evidence.
