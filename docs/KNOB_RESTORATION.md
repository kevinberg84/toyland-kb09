# Knob selection and camera correction — 0.13.1 source

**Latest source follow-up:** the owner's overhead screenshot led to the unreleased [v3 hardware preview](HARDWARE_PREVIEW_2026-10-07.md), with a smaller upper knob and unchanged installed 0.13.1 binaries. The v2 asset and validation described below belong to the installed 0.13.1 release.

Updated 2026-10-07. The user's latest exact-image selection **supersedes the interim pre-photo restoration**. The 0.13.1 source references a camera-edited complete photographic sprite. The compiled editor was inspected and all 302 editor checks pass; the tested Windows VST3 is installed and packaged. [VALIDATION.md](VALIDATION.md) records exact hashes and test scope. Owner acceptance of the final rendering remains separate.

## Current selected object and correction

The user explicitly selected `exec-8bcd38b0-7cb9-4ce3-99bc-80b29603b296.png`, describing that object as almost perfect and requesting approximately **90% overhead**, a black top tick and **no shortening of its physical barrel**. Preserve its silver skirt and groups of narrow ribs alternating with wider smooth black panels. Changing apparent sidewall visibility by moving the camera is not permission to flatten the physical part.

The current runtime source is [knob-personal-overhead-v2.png](../Assets/Photoreal/knob-personal-overhead-v2.png), **1254 × 1254 RGBA**, drawn as the complete sprite rather than a native cylinder assembled from material crops. [The ticked preview](../Assets/Photoreal/knob-personal-overhead-v2-preview.png), also 1254 × 1254, is not embedded. The [saved prompt file](../Assets/Photoreal/knob-personal-overhead-v2-prompt.txt) records the selected input, two camera/tick edits and final baked-tick removal. The clean runtime output is `exec-01df3e3b-48f0-4629-967c-49300119b0a3.png`.

For local drawing size `d`, the native black tick is centred at `(0.50d, 0.435d)`, spans radii `0.20d–0.329d`, and has width `0.026d`, in colour `#141718`. Its vertical direction uses the shared **9°** target, `sin = 0.156434` / `cos = 0.987688`, also used by the bender and exposed blades. Only the tick rotates; photographic lighting stays fixed. The 18° native-cylinder renderer from 0.13 is historical.

The grouped grip pattern is supported by [20261007_130820.jpg](<C:/Users/kevin/OneDrive/Desktop/SH-09/SH-09/20261007_130820.jpg>), which was visually inspected. Its original Wave knob shows grouped ribs separated by wider smooth panels. The photograph does not establish a complete circumference count, calibrated camera or measured dimensions. The requested height preservation and camera target are editing constraints; neither establishes measured CAD or sonic fidelity. The owner's replacement Volume knob remains excluded.

The approved overhead fader remains **unchanged**: [fader-personal-overhead.png](../Assets/Photoreal/fader-personal-overhead.png), 1484 × 1060, source crop `(125, 242, 1240, 565)`, drawn at 28 × 18 design units. Its light stripe crosses the top and descends the sides; the front lip stays black. The new knob does not restore the discarded shared wear atlas. Source textures retain finite photographic detail at larger output sizes.

## Historical pre-photo identity

The pre-personal-photo runtime knob investigated during the earlier restoration request was [Assets/Photoreal/knob.png](../Assets/Photoreal/knob.png), **1254 × 1254 RGBA**. Its Git blob is `2413d7fb1a80681ad7c34a67d0fb80794d463b09`, matching the file embedded by commit `4cf10ea` in `C:\Dev\SH-09`. It is not the newly selected 0.13.1 runtime asset.

That renderer used `BinaryData::knob_png` and drew the **whole sprite without a crop** into the rotary drawing area. Range and Wave component bounds were 87 × 87 design units; Volume was 79 × 79. The image has a broad satin-silver top, fine black ribbed grip, narrow lower silver collar and fixed photographic lighting. This was the interim restoration baseline, now superseded by the exact-image selection above. `knob-sh09.png` has a materially different, narrower top and wider flared skirt; it was not the runtime knob in that commit. `knob-aged.png` was also not rendered there.

The black pointer was separate native geometry. For drawing-area diameter `d`, its centre was `(0.50d, 0.428d)`, inner radius `0.20d`, outer radius `0.31d` and width `0.018d`, in colour `#141718`. A 0.5-design-unit white line offset right by 0.5 added a 13% highlight. Only the pointer rotated; the image and lighting stayed fixed. Range used an upper ±65° sweep, Wave ±60°, and Volume 225°–495°.

