# Hardware visual preview — 7 October 2026

**Release follow-up:** this historical preview is incorporated in 0.13.2, whose rebuilt VST3 passed final release checks and was installed. See [VALIDATION.md](VALIDATION.md) for the new binary identity; the unchanged-install statements below describe the preview stage.

This is the **unreleased v5 source and GUI preview** following the owner's overhead SH-09 screenshot and subsequent proportion and ageing feedback. The installed Windows VST3, standalone and 0.13.1 package remain unchanged. Only the `SH09GUIRender` target is compiled for this review; these results do not validate a new plugin binary or release.

## Knob proportions

The owner's overhead photograph remains the proportion reference. After the v3 correction, v4 reduced only the silver top inset by approximately 5%. The v5 edit retains that inset and thins the surrounding black top rim, targeting approximately 25% less radial rim thickness by bringing the black barrel's outer edge inward slightly. These are image-edit targets and approximate proportions, not dimensional measurements. The silver skirt, full physical barrel height, near-overhead camera and grouped narrow ribs alternating with broad smooth facets are retained. The owner's replacement Volume knob is excluded as a shape reference.

The clean runtime asset is [knob-personal-overhead-v5.png](../Assets/Photoreal/knob-personal-overhead-v5.png), 1254 × 1254 RGBA. A native `(70,70,1114,1114)` source crop normalizes the generated padding to keep the existing silver skirt's on-panel width. The [v5 prompt](../Assets/Photoreal/knob-personal-overhead-v5-prompt.txt) records built-in image generation in edit mode; the [v4 prompt](../Assets/Photoreal/knob-personal-overhead-v4-prompt.txt) records the preceding inset reduction. Neither asset has a baked indicator.

The opaque black top tick is drawn natively around `(0.497d,0.410d)`, spanning radii `0.079d–0.245d`, width `0.028d`. Its length is approximately one third of the silver top's diameter; its vertical direction retains `cos(9°)`. Only the indicator rotates; the photographed lighting stays fixed. The collar has no tick, following the owner's clarification that it would be hidden from this viewpoint. The approved fader asset and rendering are unchanged.

## Knob metal ageing

The runtime adds fine, irregular micro-pitting across the entire silver top and exposed silver skirt, including their interiors. Cached pit positions stay fixed as AGE changes, with independent deterministic patterns for Range, Waveform and Volume. Pit visibility builds continuously with AGE while the metal's bright reflections gradually dull. Returning AGE to zero restores the clean material. This replaces the earlier sparse wear confined to the top's outer edge, which left aged knobs looking too new against the panel.

The material is an artistic reconstruction informed by the owner's photographs and requested visual progression. Pit density, colouring and reflection response are not a measured corrosion law or evidence of a particular metal alloy.

## Keyboard feedback

An active mouse press shows the effective note's amber bar immediately at the key's lower edge, before the audio callback updates the display. The bar is now seven design pixels tall. The manual explains click-and-hold operation, the marker's location, and the distinction between amber for the effective lowest held note and blue for other held notes. MIDI and mouse notes share that priority rule. The editor also repaints when the selected effective note changes without a held-note-count change. Fixture checks cover mouse presses, dragging, release and mixed MIDI/mouse priority; separate white- and black-key screenshots show the immediate mouse feedback.

## Bender and POWER

The v3 bender changes are retained. The bender disk and pointer are one rigid extruded outline, with estimated depth increased from 5.5 to 7.5 design units. All object vertices rotate through the existing ±35° travel before the common 9° camera projection. A fixed fracture outline replaces the intact tip at internal AGE 0.92. The curved rim uses continuous sampled normals, a small rounded axial bevel and narrow fixed-light reflections on mostly black plastic, avoiding flat polygon seams. Surface shading is native; geometry, dimensions and material response remain an artistic reconstruction rather than measured CAD or a measured optical model. Its callbacks and spring return are retained.

POWER retains its neutral grey cap (`#a5a6a0`, ageing toward `#959487`). A [subsequent correction](POWER_BUTTON_REFERENCE.md) follows the owner's clarification that OFF should stand tall: estimated physical depths of 18 design units OFF / 7 ON project through the same 9° camera to approximately 2.82 / 1.10 units of visible front wall. ON lowers the same cap by approximately 1.72 units, preserving its top dimensions. The later owner close-up further resolves its broad rounded bevel and diffuse plastic, the two surrounding screws with different full-head oxidation, filled ON/OFF button-profile symbols, restrained print wear, and the lamp below the label. The earlier photographic upper-right screw is relocated to the cap axis. These are artistic reconstructions, not measured hardware dimensions. Hit area and POWER callback are retained. Height-only exports remain in `artifacts/GUI-power-tall-off-preview/`; the close-up exports are in `artifacts/GUI-power-closeup-preview/`. The final GUI build succeeds with all 347 checks passing; all four clean/aged ON/OFF details and the aged performance bay were visually inspected.

## Preview evidence

Actual editor exports are in `artifacts/GUI-hardware-v5-preview/`. The fixture exports 4K and 2880 × 1620 panels, knob details at AGE 0/7, isolated knob metal close-ups at AGE 0/3/7/10, POWER ON/OFF at AGE 0/7, and bender left/neutral/right before and after the fracture. `keyboard_mouse_white.png` and `keyboard_mouse_black.png` show the immediate mouse-held amber bar. The updated fixture checks progressive metal dulling and fine texture on both silver surfaces, continuous ageing and restoration, the moving indicator, fixed knob lighting, mouse/MIDI feedback and priority, real POWER callback state, bender spring return and pitch callbacks, existing cap-loss behaviour and layout.

The final GUI-only build succeeds and `GUIRender.log` reports **347 checks, zero failures**. The actual exports were visually inspected: narrower tops and thinner rims with preserved barrel depth, the longer moving tick, fine irregular pits and duller metal at AGE 7/10, white- and black-key amber mouse bars, and the complete AGE 7 panel. The standalone manual checker also passes: two HTML documents, 30 IDs, 38 fragment links and ten local references. Automated checks establish behaviour and exported pixels, not aesthetic acceptance or hardware fidelity.

The installed VST3 SHA-256 remains `2338DE51AE7B010DDEB5E69C343EA8A8BF28146483BDC61B7140140166413D8B`. No new VST3, standalone, installer or ZIP is produced by this visual iteration. Owner visual acceptance remains separate from automated checks.
