# KB-09 photographic material assets

Source inventory: **Toyland KB-09 0.11.0, 2026-10-07**. The current `KB09Artwork` CMake target embeds the nine PNG files below. The new wear atlas gives each key stain, each crack and each rotary corrosion patch its own source mark. Inventory and extraction coordinates have been checked against the source; the revised executable passed 280 GUI/asset checks, and the tested/installed module hash is recorded below.

| Embedded file | Actual source pixels | Runtime purpose |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | Clean, smooth neutral-black case |
| `chassis-patina.png` | 1774 × 887, RGB | Registered gradual dulling, edge wear and screw corrosion, separate from localized stains |
| `chassis-stains.png` | 1774 × 887, RGBA | Six case-only stain regions, each drawn once on the case |
| `knob.png` | 1254 × 1254 | Clean rotary control |
| `fader.png` | 1536 × 1024 | Photographic fader-cap surface |
| `keys.png` | 1254 × 1254 | Clean white/black key pair |
| `wear-atlas.png` | 1448 × 1086, RGBA | 32 separate key stains, two independently drawn cracks and three separate rotary corrosion patches |
| `marks.png` | 2172 × 724 | User-supplied corrected branding artwork |
| `marks-aged.png` | 2172 × 724 | Wear source applied inside the original branding coverage |

The earlier `chassis.png`, `chassis-aged.png`, `knob-aged.png`, `keys-aged.png`, `keys-cracked.png` and `keys-crack-trace.png` are historical artwork and are not embedded. Their retained prompts document previous revisions. The exact current-layer prompts include `chassis-patina-prompt.txt`, `chassis-stains-prompt.txt` and `wear-atlas-prompt.txt`. Prompt requests for a size or effect do not establish the returned dimensions or prove the final rendered result.

## How the editor uses these files

The clean case and patina-only image share a 1774 × 887 canvas and border crop, placed on the 1500 × 750 body design canvas. Patina is blended at `0.68 * Age²` over the upper metal face/fold and performance bay, excluding the molded side cheeks, open keybed and bottom front rail. This gradual treatment carries dulling and restrained wear/rust; localized marks come from a separate transparent atlas.

Six fixed atlas regions appear on the case at Age 0.25, 0.38, 0.49, 0.60, 0.72 and 0.86. Each is drawn at fixed 22% opacity once its threshold is reached: the marks do not fade in or expand with Age. The front rail retains its original clean geometry and receives only a native black overlay at `0.12 * Age`. This preserves the hardware layout and reduces brightness/apparent sheen; it is not physical relighting.

The common white-key crop is `(296, 60, 218, 1130)`; the common black-key crop is `(763, 68, 159, 695)`. Both use clean base photographs and individual native tints and polish reflections. No shared aged-key or scuff photograph is drawn. Each of the 32 keys owns a different wear-atlas cell: cell `MIDI note - 53`, covering notes 53–84. A key's stain appears at `Age = 0.18 + (MIDI note - 53) * 0.023`, then keeps its fixed geometry and opacity. Per-note placement, size, rotation and opacity supplement independent source shapes; those transforms are not used to substitute for a distinct mark. These are native composites of common clean key bases and separate wear decals, **not 32 independently generated full-key photographs**.

The two cracks use separate atlas cells 32 and 33. Note 60 receives cell 32 on its left side beginning at Age 0.82; note 77 receives cell 33 on its right side beginning at 0.94. They are independently drawn fracture shapes, not mirrored copies. Each initially reveals 78% of its placed trace height; the remaining 22% extends toward maximum Age at fixed 86% opacity. Only the trace is added, preserving each key's tint and stain.

Range, Wave and Volume use independent corrosion patches from cells 34, 35 and 36 respectively over the common clean knob. Patch opacity is `0.74 * Age²`; silhouette darkening is `0.12 * Age`. The position indicator is rendered separately. Fader caps use `fader.png` over an opaque registered body backing, plus gradual index-yellowing accents and **all caps remain present across the full Age range**. The earlier unverified missing-cap/metal-post treatment was removed. Compact switches and the bender use native drawing.

The returned wear atlas is not perfectly aligned to the requested grid origin. Runtime cells are 177 × 181 pixels, cropped at `x = (index % 8) * 181 + 2`, `y = floor(index / 8) * 181 + 36`. The vertical offset follows the actual transparent row gaps; the two-pixel side inset excludes neighbouring alpha flecks. Cells 0–36 are each assigned once; 37–47 are unused. The six case marks remain in their original case atlas and are never sampled for keys or knobs.

The supplied branding retains **KEVINBERG 09 / KB-09 on the left** and **Toyland on the right**. Runtime code converts the original marks to coverage, then multiplies that coverage by the aged source's wear mask. The generated wear can remove ink within the supplied letterforms but cannot add new lettering outside them. Native labels, ticks and panel rules receive warm ink colour aging; they do not each use the worn brand-image mask.

Four performance-bay screw heads are drawn again through tight circular clips after the white/ivory perimeter line. Their clean and patina pixels therefore sit above that line; the local patina pass uses `0.94 * Age²`. Brown screw staining and duller corroded heads come from the patina-only image. The final compiled worn preview was visually reviewed, including foreground screw heads, solid caps, separate cracks and complete footer.

All artwork is loaded from compiled BinaryData and retained as CPU-backed JUCE software images. Artwork downscaling uses high-quality image resampling. Runtime use requires no image-generation service, network request or external texture files. Native text, scale marks, indicators, popup values and software header/footer remain separate from these bitmap layers and render at the output scale.

## Provenance and fidelity limits

The case, knob, fader and key materials are original generated photographic-style reconstructions. The corrected clean marks are user-supplied; their aged variant supplies a constrained wear treatment. Research photographs inform the design and are not copied into the runtime interface. Prompt files document production intent; [the material research document](../../docs/GUI_MATERIAL_AGING.md) records primary references and distinguishes known material behaviour from art direction.

The depicted black coating, silver-coloured cap finish, brown fastener corrosion, key staining/cracking and ink fading are visual choices. The original instrument's exact cap alloy/plating, key polymer/additives and printing chemistry have not been established by this work. Brown speckling on the generated knob is stylized soiling, not a claim that aluminium rusts like steel. Cosmetic wear does not demonstrate measured physical or audio equivalence to a particular vintage unit.

The source dimensions in the table are finite. A 3840 × 2160 editor export does not create missing photographic detail or make the 1774-pixel-wide case a native 4K source. Each new wear mark comes from a 177 × 181 source crop. A physical 4K monitor was unavailable; offline export dimensions do not substitute for that test.

## Validation checkpoint

Final checks: 280 GUI/asset, 92 DSP, 20 processor (zero tracked allocations), 58 preset-file, 47 Steinberg and 75 actual-VST3 host assertions, all passing. The actual module rendered 600 seconds; GUI-enabled Pluginval strictness 10 reported SUCCESS and exited0. Build/staged/installed VST3 SHA-256: `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`. Compiled previews are in `artifacts/GUI-0.11-final`, and binaries in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`. [VALIDATION.md](../../docs/VALIDATION.md) indexes exact logs.

The added GUI tests inspect all 37 assigned alpha crops, check visible content and transparent isolation, and compare normalized shapes pairwise after ignoring RGB, position and overall opacity. Mirrors and quarter turns are also compared, with a duplicate negative control. Existing sampled-key pixel checks alone cannot establish independent wear shapes. These bounded checks do not prove uniqueness under every possible image transformation, replace visual review, or establish a physical 4K-display test or measured material/sonic equivalence.
