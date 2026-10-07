# KB-09 material aging research and implementation guide

Validated checkpoint: **Toyland KB-09 0.11.0, 2026-10-07**. The editor separates gradual patina from discrete stains and crack traces. Each key stain, each crack and each rotary corrosion patch now has an independent source mark assigned to one object; every fader cap is retained. The revised source, asset extraction and test additions are documented below. Fresh results are280 GUI/asset checks, 92 DSP, 20 processor, 58 preset-file, 47 Steinberg and 75 actual-module host checks, all without failures. GUI-enabled Pluginval strictness 10 completed SUCCESS and exited0; this revision is installed with the hash recorded below.

## Integrated runtime images

The `KB09Artwork` CMake target embeds **nine PNG files**. Dimensions below were checked directly from their PNG headers; they are actual source dimensions, not requested generation sizes.

| Runtime file | Source pixels | Role |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | Clean neutral-black chassis, replacing the earlier blue-grey plate |
| `chassis-patina.png` | 1774 × 887, RGB | Registered gradual dulling, edge wear and screw-corrosion endpoint; localized stains are separate |
| `chassis-stains.png` | 1774 × 887, RGBA | Transparent six-region case-only stain atlas; each region drawn once on the case |
| `knob.png` | 1254 × 1254 | Clean rotary control |
| `fader.png` | 1536 × 1024 | Photographic fader cap; additional age behaviour is native drawing |
| `keys.png` | 1254 × 1254 | Clean white/black key pair |
| `wear-atlas.png` | 1448 × 1086, RGBA | 32 separate key stains, two independently drawn fracture traces and three separate rotary corrosion patches |
| `marks.png` | 2172 × 724 | User-supplied corrected KEVINBERG 09 / KB-09 and Toyland letterforms |
| `marks-aged.png` | 2172 × 724 | Registered wear source constrained to the original mark coverage at runtime |

The old `chassis.png`, `chassis-aged.png`, `knob-aged.png`, `keys-aged.png`, `keys-cracked.png` and `keys-crack-trace.png` remain historical source material but are **not embedded** by the current CMake list. Registration means matched source canvases and explicit rendering crops/placements, not measured pixel-perfect correspondence to a real instrument. Current production prompts include `chassis-patina-prompt.txt`, `chassis-stains-prompt.txt` and the exact saved `wear-atlas-prompt.txt`. See [the artwork inventory](../Assets/Photoreal/README.md).

The body remains a 1500 × 750 design canvas, with native software header/footer and vector labels, scales, switch geometry and indicators. A 3840 × 2160 export has that output resolution; the 1774-pixel chassis and other finite raster sources do not become native 4K photography when enlarged. Neither this inventory nor an offline export establishes validation on a physical 4K display.

## Evidence and direct photograph references

The two existing local Kong photographs were inspected for this investigation. Their owner's account identifies the instrument as a heavily deteriorated example found in a cellar. The photos show concentrated damage around the top rail and panel fold, irregular affected patches, and considerable intact printing between them. They also show warm, uneven key colour. This is evidence for one damaged unit, not the inevitable appearance of every old SH-09; lighting and white balance prevent calibrated colour measurements. [C. Y. Kong's first-hand SH-09 account](https://www.cykong.com/Synths/Roland%20SH-09/RolandSH09.htm).

| Primary photo reference | What to study |
|---|---|
| [Kong: complete original instrument](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Top.jpg), existing `Research/SH09_Kong_top_reference.jpg` | Uneven distribution across the whole instrument; substantial clean areas remain. |
| [Kong: original front panel, detail 1](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Front%20Panel1.JPG), existing `Research/SH09_Kong_panel_reference.jpg` | Dark screw heads, worn border/printing, local patches near the fold; knob geometry remains recognisable. |
| [Kong: original front panel, detail 2](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Front%20Panel2.JPG) | Additional original-unit reference linked by its owner; described there as scraped paint. |
| [CCI figure 11b: worn painted/plated musical box](https://www.canada.ca/content/dam/cci-icc/images/services/preventive-conservation/guidelines-collections/caring-for-metals/figure_11b.jpg) | Local coating loss and underlying rust. |
| [CCI figure 13: aluminium corrosion](https://www.canada.ca/content/dam/cci-icc/images/services/preventive-conservation/guidelines-collections/caring-for-metals/figure_13.jpg) | White deposits and pitting, distinct from iron rust. |
| [CCI figure 22: steel corrosion](https://www.canada.ca/content/dam/cci-icc/images/services/preventive-conservation/guidelines-collections/caring-for-metals/figure_22.jpg) | Restrained red-brown corrosion colour. |
| [CCI figure 30b: worn plating over iron](https://www.canada.ca/content/dam/cci-icc/images/services/preventive-conservation/guidelines-collections/caring-for-metals/figure_30b.jpg) | Adjacent surviving finish and corroded exposed substrate. |
| [CCI figure 42: fingerprint on brass](https://www.canada.ca/content/dam/cci-icc/images/services/preventive-conservation/guidelines-collections/caring-for-metals/figure_42.jpg) | A material-specific fingerprint reference; not proof that painted steel develops the same etched stain. |

The museum photos are identified by their captions in the [Canadian Conservation Institute's metal guide](https://www.canada.ca/en/conservation-institute/services/preventive-conservation/guidelines-collections/metal-objects.html). These are reference links, not licensed runtime textures. The remote image fetcher did not render several instrument JPEGs during this pass; visual observations above rely on the two existing local photographs. Other useful first-hand instrument photo pages are [Syntaur's SH-09 gallery](https://syntaur.com/gallery.php?keyboard=699) and the [Still Not Working repair record](https://snw.lonningdal.no/sh09.php). Syntaur expressly reserves permission to reuse its photographs.

## Material facts and limits

CCI distinguishes red-brown/black iron corrosion from white aluminium corrosion. Coating damage can expose underlying iron; fingerprints can stain metals. These distinctions support different treatments for the panel, screws and knob caps. They do **not** establish the exact alloy, plating or finish of every SH-09 component. [CCI: metal objects](https://www.canada.ca/en/conservation-institute/services/preventive-conservation/guidelines-collections/metal-objects.html).

Plastic discoloration, loss of strength and cracking can follow different processes and rates. A yellow key need not be cracked; identical ageing across all keys is not a physical requirement. The exact original key polymer/additives have not been identified here, so do not label this reconstruction an experimentally verified ABS-aging model. [CCI: plastics and rubbers](https://www.canada.ca/en/conservation-institute/services/preventive-conservation/guidelines-collections/caring-plastics-rubbers.html).

Surface roughness broadens reflections. In a metal/roughness workflow, paint, dirt and oxide cover the metal with a non-metallic surface; intact paint should not reflect like bare polished metal. Roughness changes should be distinguishable from changes in base colour. [Adobe PBR guide, part 1](https://www.adobe.com/learn/substance-3d-designer/web/the-pbr-guide-part-1), [part 2](https://www.adobe.com/learn/substance-3d-designer/web/the-pbr-guide-part-2).

The following blending curves, damage placement and warm printing colours are **art direction and implementation choices**. They are not material measurements, an aging timeline, or a chemical model of the original screen-print ink. In particular, visible cracks and stronger corrosion at maximum Age are the user's requested appearance, rather than features verified on all photographed units. The generated knob image includes grey dulling and brown speckling; the brown marks are a stylized soiling treatment, not evidence that the original cap was iron or that aluminium produces brown rust.

## Integrated material treatment

| Surface | Current renderer behaviour | Material/validation boundary |
|---|---|---|
| Black painted case | Blends the patina-only image gradually over the upper face/fold and performance bay. Six case-only marks appear at distinct Age thresholds with fixed placement, size and opacity. Side cheeks, the open keybed and bottom front rail are excluded from these photographic overlays. The front rail receives a native translucent black pass. | Patina and mark appearance are separate effects. Keeping the clean bottom rail preserves its original hardware geometry. The final compiled fixture passed and the worn preview was reviewed; the materials remain an artistic reconstruction. |
| Ivory printing and border | Native labels, ticks and rules shift from clean ivory toward warm aged ink. The two supplied brand marks additionally blend toward a worn alpha mask derived from `marks-aged.png`, multiplied by the original mark coverage. | Generated wear can remove ink only within the original mark shapes. Native labels/rules currently receive colour aging, not individual chipped-letter masks. |
| Silver-coloured rotary controls | Draws the common clean knob with silhouette darkening. Range, Wave and Volume each receive a different localized corrosion patch; the old aged-knob photograph is not drawn. Native indicators remain separate. | The artwork depicts stylized wear/soiling. It does not identify or scientifically reproduce the original alloy, plating or oxidation. |
| White keys | Uses the common clean white-key crop with native per-note tint and polish reflection. Tint increases with Age. Every key owns a separate stain source and fixed placement, size, rotation, opacity and onset. Two keys additionally receive independently drawn fracture shapes. | These are shared clean bases with separate wear decals, not 32 independently generated full-key photographs. No aged-key photograph is drawn. |
| Black keys | Uses the common clean black-key crop with a per-note native tint, polish reflection and independent stain source. Native geometry/shadows sit beneath; active-key feedback sits above. | The repeated aged-black-key scuff crop has been removed. Uniqueness must be assessed from the marks themselves, not inferred from different tint or location. |
| Faders, switches and bender | Retains every fader cap at every Age, using the photograph with native wear accents; switch/bender shading ages through native drawing. | The earlier missing-cap/unverified metal-post treatment has been removed. These controls do not each have a separate aged raster image. |
| Screws | Rust and adjoining brown staining come from the patina-only endpoint. Four performance-bay heads are re-composited through tight circular clips after its rules/outline. | The source places these heads over the ink. Treating them as ferrous fasteners is a visual choice, not a verified parts assay or measured corrosion match. |

### Stains and reflections

The new `chassis-patina.png` carries the gradual case treatment; `chassis-stains.png` supplies localized fingerprints, smears and beverage residue on transparency. The case draws six separate source regions once each, at fixed 22% opacity after their individual thresholds. These regions are not reused on the keys or knobs. The marks neither grow nor fade in as Age increases. Only the separate patina layer continues its gradual blend. Moving Age backwards removes marks below their threshold, so this is deterministic preset rendering rather than a persistent simulation of elapsed wear.

All 32 keys receive separate stain drawings from `wear-atlas.png` cells 0–31. Notes 53–84 map directly to those cells, so no stain is substituted by a rotated, mirrored or recoloured copy of another key's stain. Per-note placement, size, rotation, opacity and onset are additional composition choices. Native tints and tiny individually positioned polish reflections distinguish clean key appearances, but those differences alone cannot establish unique wear shapes. Once a stain appears, its geometry and opacity remain fixed. Cells 32 and 33 contain independent fractures for notes 60 and 77; each appears mostly complete, with its remaining length revealed gradually. Cells 34–36 belong individually to Range, Wave and Volume. Each of these 37 assigned decals is used on one object.

The actual atlas returned at 1448 × 1086, below the requested 3072 × 2304. Its measured crop formula is `x = (index % 8) * 181 + 2`, `y = floor(index / 8) * 181 + 36`, `width = 177`, `height = 181`. The 36-pixel vertical offset follows transparent gaps between the real mark rows; the two-pixel side inset excludes neighbouring alpha flecks. A nominal grid beginning at y=0 would truncate marks. Cells 37–47 are unused. The source was not rearranged or upscaled to manufacture the requested dimensions.

Clean and aged endpoints are drawn with shared geometry and baked lighting. This permits different material appearance without moving the control outlines. It remains a layered 2D approximation: no runtime roughness/normal maps, relighting or corrosion simulation are claimed. A native darkening pass reduces brightness but does not by itself broaden a baked highlight. Judge the actual highlight shape in the final composite rather than inferring it from opacity settings.

## Screw composition over the perimeter line

The implemented order is clean chassis, gradual patina, separately triggered case stains, then native printing and the performance-bay perimeter line. The renderer draws four registered screw-head patches through 23-design-unit circular clips, restoring clean head pixels and overlaying the matching patina pixels. Thus the heads are a foreground layer above the line rather than rectangular patches over the surrounding panel. Other screws remain part of the case/patina images. The final compiled output was visually reviewed; no measured physical-screw match is claimed.

## Age control and renderer integration

For the current source checkpoint, `a` is the normalized Condition/Age value. These are implementation curves, not measured physical laws:

| Layer | Implemented blend/threshold |
|---|---|
| Upper metal face/fold and performance bay | Patina-only endpoint opacity `0.68 * a²`, clipped to two metal regions |
| Six case stain regions | Appear at `a = 0.25 / 0.38 / 0.49 / 0.60 / 0.72 / 0.86`; fixed 0.22 opacity and fixed geometry |
| Bottom front rail | Clean geometry retained; native black overlay with opacity `0.12 * a`, with no aged-photo overlay |
| Re-composited screw heads | Patina endpoint opacity `0.94 * a²` |
| Rotary artwork | Separate patch per Range/Wave/Volume at opacity `0.74 * a²`, with `0.12 * a` silhouette darkening |
| Brand-mark wear | Clean/worn alpha treatments blended using `a²` |
| Native ink colour | Clean ivory interpolated toward warm aged ink using `0.86 * a` |
| White-key native tint | With `v = (note - 53) / 31`, opacity is `0.008 + 0.035*v + a*(0.16 + 0.085*v)` over the clean white-key image |
| Black-key native tint | Opacity `0.004 + (note - 53)*0.0012` over the clean black-key artwork |
| Individual polish reflections | Fixed per-note positions; 0.85 × 1.5 design-unit highlight, opacity 0.24 on white keys and 0.055 on black keys |
| Individual key stains | Unique atlas cell `s = note - 53`; onset `0.18 + s*0.023`; fixed opacity after onset `0.13 + ((s*5) mod 13)*0.009`; fixed per-note placement/size/rotation |
| Two key cracks | Separate cells 32/33: note 60 begins at 0.82 and note 77 at 0.94. Both use fixed 0.86 opacity; 78% of the placed trace height is revealed at onset, then the remaining 22% extends toward maximum Age |
| Fader caps | Retained throughout the complete Age range |

The pattern is deterministic because textures are fixed and per-key variation depends on the note. Knob/key layers stay within their own local placements; case damage remains on the body. Persistent artwork is loaded from compiled BinaryData into JUCE `SoftwareImageType` images; brand masks are prepared once. The common white-key crop is `(296, 60, 218, 1130)` and the black-key crop is `(763, 68, 159, 695)`. The independent crack cells are placed at different scales on opposite sides of notes 60/77 without reusing or mirroring one source trace. The case/patina pair shares the border crop. High-quality raster resampling handles downscaling; native labels and geometry render at the output pixel scale. Neither technique increases the photographic source detail.

No online generation service, external texture folder or file I/O is needed during playback. The software header, preset feedback, parameter values and active-note highlights remain native UI. Visual wear does not break keys, remove their hit targets or disable controls. The existing Age parameter's audio behaviour is separate from these GUI material layers; cosmetic appearance is not evidence of measured hardware aging or audio-reference fidelity.

## Validation checkpoint and remaining limits

Final checks: 280 GUI/asset, 92 DSP, 20 processor, 58 preset-file, 47 Steinberg and 75 actual-module host assertions passed. Pluginval strictness 10 with GUI completed SUCCESS and process exit 0; actual-module rendering covered 600 seconds. Build/staged/installed VST3 SHA-256: `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`. Exact logs and artifact paths are indexed in [VALIDATION.md](VALIDATION.md).

The new GUI-fixture checks read the compiled wear atlas directly. They verify the registered dimensions and alpha channel; visible isolated content in each of the 37 assigned crops; and pairwise distinct normalized alpha shapes for all key stains, the two cracks, the three rotary patches and all groups together. Bounding-box normalization removes location/scale, RGB is ignored and opacity is normalized. Eight mirror/quarter-turn variants are compared, with a deliberate duplicate negative control. The detector tolerates small resampling differences in a copied mark; it is a bounded test, not proof against every arbitrary affine transformation or photographic similarity.

The existing actual-editor checks remain relevant for discrete case-stain events, separate crack onsets, healthy/aged sampled key appearance, configured value-bubble contrast, keyboard feedback, physical switches and transformed footer geometry. Final pixels being different is insufficient by itself: a tint or polish reflection can distinguish otherwise identical stains. Source-shape checks therefore supplement actual render checks and visual review.

Final actual-editor output was reviewed, and native standalone M1051x642/L1280x720 logical-pixel windows were observed with complete footers and return from L to the enlarged M. Offline exports at 3840 × 2160 and 2880 × 1620 establish those image dimensions only. A physical 4K monitor was unavailable, so no physical 4K-display validation is claimed. Native vectors can stay sharp at the output scale, while the 1774-pixel chassis and 177 × 181 wear crops retain finite photographic detail.

Exact alloy/plastic/ink chemistry, physical aging calibration, measured sonic equivalence and an exhaustive manual sweep of every intermediate Age value remain outside the verified evidence. Cosmetic uniqueness does not establish any of those properties.
