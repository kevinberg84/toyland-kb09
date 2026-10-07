# KB-09 material aging research and implementation guide

Current source: **Toyland KB-09 0.12.0, 2026-10-07**. The user's personal SH-09 photographs now guide the chassis, controls and wear. Gradual material changes are separate from a retained coffee-ring event, individual native key flecks, one native key crack, two lost fader caps and a broken bender tip. The corrected fader artwork is integrated in source; the changes described here do **not** establish a validated new build. [VALIDATION.md](VALIDATION.md) owns build-specific results; the 0.11 evidence below is historical.

## Integrated runtime images

The current CMake source list embeds **eight PNG files**. Dimensions below were checked from PNG headers; they are source dimensions, not generation-request targets. Neither rejected crack PNG is embedded.

| Runtime file | Source pixels | Role |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | Clean neutral-black chassis, replacing the earlier blue-grey plate |
| `chassis-personal-aged.png` | 1774 × 887 | New registered material endpoint informed by the user's unit; used only on metal regions and matching screw heads |
| `chassis-stains.png` | 1774 × 887, RGBA | Only the existing coffee-ring crop `(1420, 0, 300, 320)` is drawn; the other marks are unused |
| `knob-personal.png` | 1254 × 1254 | Revised rotary control, with a separate native black top index |
| `fader-personal-v2.png` | 1536 × 1024 | Corrected top-and-side stripe in the earlier compact footprint |
| `keys.png` | 1254 × 1254 | Clean white/black key pair |
| `marks.png` | 2172 × 724 | User-supplied corrected KEVINBERG 09 / KB-09 and Toyland letterforms |
| `marks-aged.png` | 2172 × 724 | Registered wear source constrained to the original mark coverage at runtime |

The old `wear-atlas.png` is **entirely removed from runtime**, including its key stains, cracks and rotary patches. The previous patina, aged-knob and aged-key sources are also not embedded by this source plan. Retained historical files do not imply runtime use. Registration means matched source canvases and explicit rendering crops/placements, not measured pixel-perfect correspondence to a real instrument. See [the artwork inventory](../Assets/Photoreal/README.md) for provenance and generation records.

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

The user's additional close-ups are recorded in [PERSONAL_SH09_CONTROL_REFERENCE.md](PERSONAL_SH09_CONTROL_REFERENCE.md). They resolve the raised ribbed switch grips, curved bender wheel, flat metal fader blades and cap strip crossing the top and descending the sides. Their uncalibrated lighting does not establish alloys or chemical composition. The replacement Volume knob is excluded as an original-part reference.

The blending curves, damage placement and warm printing colours below are **art direction and implementation choices**. They are not material measurements, an elapsed-years timeline, or a chemical model. Fine warm stipple on the knob rims is a stylized tarnish/soiling treatment, not proof of iron or a claim that aluminium develops red rust. Screw heads use a dull silver appearance; no ferrous-alloy or red-rust claim is made for them.

## Current 0.12 material treatment

The normalized Age value is `a`. Appearance is deterministic: lowering Age reverses the corresponding cosmetic event. These changes do not remove control hit targets or disable sound control.

| Surface | Source implementation / planned final treatment |
|---|---|
| Black painted case | The registered `chassis-personal-aged.png` endpoint blends at `0.90 * a^0.85` on the upper face/fold and performance bay only. Molded cheeks, the open keybed and bottom rail are excluded. The clean bottom rail keeps its geometry with a native `0.12 * a` black pass. |
| Screws | Matching endpoint pixels blend at `0.94 * a`. Four performance-bay heads are restored through tight circular clips above the perimeter rules. The intended finish is dull silver, without a red-alloy inference. |
| Coffee ring | The sole retained case-stain crop is `(1420, 0, 300, 320)` from `chassis-stains.png`. It appears at `a = 0.60` with fixed placement, size and 0.22 drawing opacity. The other five former case events are removed. |
| Printing and supplied marks | Labels, rules and scales shift toward warm aged ink using `0.86 * a`. The supplied KEVINBERG 09 / KB-09 and Toyland marks blend toward worn alpha coverage using `a²`; the source letter masks constrain coverage. Native labels receive colour aging, not individually chipped-letter masks. |
| Rotary controls | `knob-personal.png` provides the revised form. A separate black top index turns around the normalized image centre `(0.495, 0.300)`. Each knob has a separate deterministic seed for fine rim stipple and dust in the lower grip. Their opacity increases gradually; no old corrosion decal or copied rust blob is used. |
| White keys | Common clean material is combined with per-note ivory tint, a distinct small polish reflection and independent native wear geometry. Yellowing increases gradually. These are 32 distinct compositions across the whole keyboard, not 32 separate photographs. |
| Black keys | Common clean material, per-note tint/polish and independent native wear. The old repeated aged-key crop and atlas stamps are not drawn. |
| Fader caps | `fader-personal-v2.png` uses the older compact footprint. The light strip crosses the top and descends the sides; the broad front remains black. The obsolete front/shoulder belt is rejected. |
| Exposed fader blades | VCO mixer cap loss appears at `a = 0.64`; Release cap loss at `a = 0.88`. Native flat, stepped blades follow the user's uncapped-unit photographs. Exact dimensions and alloy remain unmeasured. Sliders retain their range, interaction and parameter attachments. |
| Mode switches | Raised black ribbed grips with native shading. Pale ridges are restrained highlights/wear, not an inserted white strip. |
| Bender | A raised curved wheel and tip use native geometry with an estimated ±35° projection. At `a = 0.92` the tip becomes a short broken stump. Spring return and bend functionality remain active. |

### Independent key wear and one crack

`keySurfaceWear` generates independent fixed microgeometry from each key's serial number, `MIDI note - 53`. Thirty-eight tiny dust particles concentrate near the back and side edges; their geometry stays fixed while opacity grows gradually with Age. At `a = 0.18 + serial * 0.023`, seven small dirt flecks appear for that key. Each fleck's placement, shape and size come from that key's seed. Those discrete flecks keep their geometry and intensity after onset; the separate fine-dust layer continues its gradual change. No photographic stain stamp is shared or transformed between keys.

Per-note tint and polish also distinguish clean key appearances. This is a source-level construction of distinct composites; final pixel uniqueness and visibility at each output size still require the rebuilt fixture and visual review. Common clean source materials remain in use.

Only MIDI 60 receives a native curved hairline, beginning at `a = 0.56`. Its width is 0.32 design pixels and drawing opacity is fixed at 0.55. It appears at 78% of its placed length, then reveals the remaining 22% toward maximum Age. `Assets/Photoreal/key-hairline-personal.svg` records the matching path; it is a reference for native drawing, not another embedded bitmap. Both generated crack PNGs were rejected because they retained a broad halo. No second key crack is drawn. Placement, onset and progression are artistic choices, not measured polymer-aging behavior.

### Layering, source limits and interaction

The case order is clean chassis, gradually blended personal-unit material endpoint, the discrete coffee ring, and native printing/perimeter rules. Four registered screw heads are then composited through 23-design-unit circular clips above the performance-bay outline. This preserves the overlap without adding rectangular panel patches.

Knob lighting remains baked into the image while the black indicator rotates separately. Native stipple varies between knobs. These are layered 2D materials, without runtime normal maps, relighting or a physical corrosion simulation. Native darkening does not by itself broaden a baked highlight.

Persistent images load from compiled BinaryData into JUCE CPU-backed `SoftwareImageType` storage; brand masks are prepared once. The common white-key crop is `(296, 60, 218, 1130)` and black-key crop is `(763, 68, 159, 695)`. High-quality resampling handles scaling, and labels/glyphs render at the output scale. Neither technique adds photographic detail beyond the finite source images.

The software project notice occupies the unused top body area. The full editor has S/M/L sizing: S fits within 65% of the monitor work area, M within 90%, and L requests the available work area, accounting for measurable frame chrome. See [UI.md](UI.md) and [GUI_WINDOW_BEHAVIOR.md](GUI_WINDOW_BEHAVIOR.md).

No online generation service, external image folder or image-file I/O is needed during playback. Cosmetic damage leaves keyboard, fader and bender interaction active. Audio Age behavior is separately documented and does not establish a measured link between visible damage and sound.

## Validation boundary and historical evidence

**The revised 0.12 material/control source and corrected cap asset have not been validated by a completed new build in this document.** Fixture code or inspected source alone is not a passing render, host test or visual acceptance result. [VALIDATION.md](VALIDATION.md) records the final build and evidence when available.

The historical 0.11 checkpoint recorded 280 GUI/asset, 92 DSP, 20 processor, 58 preset-file, 47 Steinberg and 75 actual-module host assertions passing, Pluginval strictness 10 SUCCESS/exit 0, and 600 seconds of module rendering. Its hash, logs and installation records belong to that checkpoint. Atlas-specific shape checks describe the old asset pipeline and do not validate the replacement native dust/stipple or new hairline path.

Historical native standalone observations were M 1051 × 642 and L 1280 × 720 logical pixels with complete footers. Those observations do not establish the new S control or the revised 0.12 materials. Offline 3840 × 2160 and 2880 × 1620 images establish their export dimensions only. A physical 4K display was unavailable; the 1774-pixel chassis and other raster assets remain finite-resolution sources.

Exact alloy/plastic/ink chemistry, physical aging calibration, measured sonic equivalence and an exhaustive manual sweep of every intermediate Age value remain outside the verified evidence. Cosmetic uniqueness does not establish those properties.
