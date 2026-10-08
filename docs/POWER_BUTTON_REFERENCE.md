# POWER button height correction — 7 October 2026

The owner clarified that the POWER cap **should stand tall in OFF**. The earlier reduction to a shallow OFF cap was a misinterpretation. This correction retains the shared near-overhead camera and grey material, and raises the same cap in OFF without stretching its face or changing its viewing angle.

## Evidence

- Roland's [SH-09 Owner's Manual, printed page 7](https://thismodernday.com/wp-content/uploads/2021/07/Roland-SH-09-Owners-Manual.pdf#page=8) identifies a push-on/push-off power switch with a separate LED indicator. The existing local page render was visually inspected.
- Roland's [27 March 1980 Service Notes](https://el34world.com/charts/Schematics/files/Roland/Roland_sh09_service_manual.pdf) identify grey Button no. 8, part 016-008, in the first-page product photograph and third-page parts list. The existing local first-page render was visually inspected. Neither manual supplies a cap-height or latch-travel dimension.
- The owner's `20261007_131020.jpg` shows the broad flat top, softened edges and substantial vertical body in an oblique view of the performance bay. `20261007_131236.jpg` and `20261007_131314.jpg` provide overhead context. Photo colour is not calibrated, and the photographs alone do not establish switch state or travel. The owner's explicit clarification establishes the raised OFF state requested here.

## Rendering

The top has identical dimensions in both states. Estimated physical depths of 18 design units OFF and 7 ON project through the common 9° camera to approximately 2.82 and 1.10 design units of visible front wall. The top moves approximately 1.72 design units between states. These are visual reconstruction estimates, not measured hardware dimensions. The deeper body has edge shading, bevels and a height-dependent contact shadow; grey material, fine moulding grain and age-related dirt are retained.

Actual editor exports are in `artifacts/GUI-power-tall-off-preview/`: `power_age0_off.png`, `power_age0_on.png`, `power_age7_off.png` and `power_age7_on.png`. The four crops and complete aged performance bay were visually inspected for grey material, increased OFF height, consistent top dimensions and clipping. `GUIRender.log` reports **347 checks, zero failures**, including the real POWER callback states. This establishes preview behaviour, not measured hardware fidelity or owner aesthetic acceptance.

Only `SH09GUIRender` was built. The installed 0.13.1 VST3, standalone and package remain unchanged.
