# Hardware visual preview — 7 October 2026

This is an **unreleased source and GUI-preview revision** following the owner's overhead SH-09 screenshot. The installed Windows VST3, standalone and 0.13.1 package remain unchanged. Only the `SH09GUIRender` target is compiled for this review; these results do not validate a new plugin binary or release.

## Knob proportions

The metal top now measures approximately 59% of the silver skirt's outside width, following the personal overhead photograph, compared with approximately 77% in the previous GUI. The black barrel narrows with the insert, retaining the grouped narrow ribs alternating with broad smooth facets, the approved height and the near-overhead camera. The owner's replacement Volume knob is excluded as a shape reference.

The clean runtime asset is [knob-personal-overhead-v3.png](../Assets/Photoreal/knob-personal-overhead-v3.png), 1254 × 1254 RGBA. A native `(70,70,1114,1114)` source crop normalizes the generated padding to keep the existing silver skirt's on-panel width. The [ticked preview](../Assets/Photoreal/knob-personal-overhead-v3-preview.png) is not embedded. [Exact prompts and reference provenance](../Assets/Photoreal/knob-personal-overhead-v3-prompt.txt) record built-in image generation in edit mode.

The opaque black tick is drawn natively at `(0.50d,0.420d)`, spanning `0.16d–0.260d`, width `0.020d`; its vertical direction retains `cos(9°)`. Only the indicator rotates; the photographed lighting stays fixed. Tarnish and dust placement follow the smaller top and grip. The approved fader asset and rendering are unchanged.

## Bender and POWER

The bender disk and pointer are one rigid extruded outline, with estimated depth increased from 5.5 to 7.5 design units. All object vertices rotate through the existing ±35° travel before the common 9° camera projection. A fixed fracture outline replaces the intact tip at internal AGE 0.92. The curved rim uses continuous sampled normals, a small rounded axial bevel and narrow fixed-light reflections on mostly black plastic, avoiding flat polygon seams. Surface shading is native; geometry, dimensions and material response remain an artistic reconstruction rather than measured CAD or a measured optical model. Its callbacks and spring return are retained.

POWER now has a neutral grey cap (`#a5a6a0`, ageing toward `#959487`) with reduced highlight contrast, a shorter contact shadow and less projected height. The visible front wall is 1.0 design unit OFF / 0.45 ON, with 0.45-unit ON displacement. Hit area, callback and separate lamp are retained.

## Preview evidence

Actual editor exports are in `artifacts/GUI-hardware-v3-final-preview/`. The fixture exports 4K and 2880 × 1620 panels, knob details at AGE 0/7, POWER ON/OFF at AGE 0/7, and bender left/neutral/right before and after the fracture. The updated fixture checks the moving indicator, fixed knob lighting, real POWER callback state, bender spring return and pitch callbacks, existing cap-loss behaviour and layout.

The final GUI-only build succeeds and `GUIRender.log` reports **310 checks, zero failures**. The actual exports were visually inspected: knob top and tick registration, grouped grip ribs, grey POWER ON/OFF, smooth bender rim at left/neutral/right, intact and fractured tips, and the complete panel at AGE 7. Automated checks establish behaviour and exported pixels, not aesthetic acceptance or hardware fidelity.

The installed VST3 SHA-256 remains `2338DE51AE7B010DDEB5E69C343EA8A8BF28146483BDC61B7140140166413D8B`. No new VST3, standalone, installer or ZIP is produced by this visual iteration. Owner visual acceptance remains separate from automated checks.
