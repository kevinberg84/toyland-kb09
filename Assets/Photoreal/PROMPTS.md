# Production artwork provenance

The chassis, rotary knob, fader cap and white/black key-pair assets were created with the built-in image-generation tool for this project. They are original generated material reconstructions guided by the SH-09 reference research. The corrected `KB-09.png` supplied by the user is embedded as `marks.png`. No Universal Audio artwork or third-party instrument photograph is copied into the runtime interface.

| Runtime file | Actual source pixels | Saved production prompt |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | `chassis-black-prompt.txt`, neutral black edit of the historical chassis |
| `chassis-patina.png` | 1774 × 887 | `chassis-patina-prompt.txt`, gradual dulling and existing-screw rust without stains |
| `chassis-stains.png` | 1774 × 887 | `chassis-stains-prompt.txt`, six case-only marks on a transparent atlas |
| `knob.png` | 1254 × 1254 | `knob-prompt.txt` |
| `fader.png` | 1536 × 1024 | `fader-prompt.txt` |
| `keys.png` | 1254 × 1254 | `keys-prompt.txt` |
| `wear-atlas.png` | 1448 × 1086, RGBA | `wear-atlas-prompt.txt`, 32 separate key stains, two independent cracks and three separate rotary corrosion patches |
| `marks.png` | 2172 × 724 | User-supplied corrected artwork; no generation prompt |
| `marks-aged.png` | 2172 × 724 | `marks-aged-prompt.txt`, aged ink edit of supplied marks |

The prompts describe the requested output and production intent. Requested dimensions inside a prompt are not evidence that the image generator returned that resolution; the actual embedded dimensions are listed above.

The October 6 chassis revision changes the metal faces to **neutral black smooth satin painted steel**. Patina/rust crossfades gradually. Six separate case stains appear at distinct Age thresholds with fixed geometry and opacity; each case region is used once and never reused on keys or knobs. The front rail retains its original geometry with a subtle dark overlay to reduce sheen. This is cosmetic reconstruction, not a calibrated elapsed-year simulation. Historical `chassis.png`, `chassis-aged.png`, `knob-aged.png`, `keys-aged.png`, `keys-cracked.png`, `keys-crack-trace.png` and their prompts remain provenance and are no longer embedded. The chassis source resolution remains 1774 × 887.

The renderer shares clean photographic key bases, with individual native tint and polish reflections. Each key owns a different stain from wear-atlas cells 0–31; notes 60 and 77 use separate fracture drawings from cells 32 and 33. Range, Wave and Volume own cells 34, 35 and 36 respectively. None of these 37 assigned decals is reused on another object. Stains appear individually at fixed appearance. The two cracks begin separately and extend slightly without fading in. The shared aged-key and aged-knob photographs are no longer drawn. These are common clean bases with separate wear decals, not 32 independently generated full-key photographs.

The exact saved prompt requested an eight-column/six-row atlas, preferably 3072 × 2304, but the returned file is **1448 × 1086**. Measured runtime crops are **177 × 181**, located at `x = (index % 8) * 181 + 2`, `y = floor(index / 8) * 181 + 36`. The offset and side gutters follow the returned image's actual transparent gaps; an unadjusted origin would cut across marks. Cells 37–47 are unused. These coordinates select existing image regions; the source image has not been resampled or rearranged to manufacture the requested resolution.

All fader caps remain fitted; the earlier unverified missing-cap/metal-post treatment is removed. Compact switches and the spring-return bender use native geometry. The bender rotates around a hidden axle beneath its slot; exact dimensions and travel are inferred, not measured original-part CAD. Text and vectors render at the output's actual pixel scale, while artwork downscaling uses high-quality resampling.

The supplied branding is cropped and placed as **KEVINBERG 09 / KB-09 on the left** and **Toyland on the right**. There is one current product identity, Toyland KB-09; there is no selectable alternate skin.

All nine runtime PNGs are embedded by the `KB09Artwork` CMake target. Native masks constrain aged brand ink to the supplied original mark shapes. The built-in image-generation tool produced the raster material edits; no fallback CLI was used. No network connection, image-generation service or external image file is needed when using the instrument.

The compiled editor can export a 3840 × 2160 preview with text and vector details rendered at that output resolution. The chassis itself remains a 1774 × 887 raster source; enlargement cannot recover additional photographic detail. The same finite-resolution limitation applies to each bitmap layer. A 4K preview therefore demonstrates editor output dimensions, not a native 4K photograph, physical-display validation or measured visual equivalence to an original instrument.

The independent-wear revision passed 280 GUI/asset checks, including 80 atlas assertions and 3 cap-opacity assertions; the release validators and 600-second actual-module host render also passed. Prompts establish production intent, not independent proof of uniqueness or a passed final build. The GUI fixture now tests source-alpha shapes independently of colour, position and final key tint; the release evidence belongs in [VALIDATION.md](../../docs/VALIDATION.md).
