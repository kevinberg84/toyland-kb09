# Toyland KB-09 physical GUI research

Research updated 2026-10-03. This document specifies the SH-09 geometry and mechanisms to reconstruct under the **Toyland KB-09** identity. It is reference research, not a claim that a completed screenshot has passed visual review. No third-party photograph, UAD texture, UAD control image, or original manufacturer logo is proposed as a runtime asset.

## Evidence inspected

1. **Original Roland owner's manual**, printed p. 2 front-panel drawing; pp. 3-7 control descriptions; p. 43 block diagram. [Original-document scan](https://snw.lonningdal.no/sh09/Roland%20SH-09%20Owners%20Manual.pdf), local `Research/ROLAND_SH-09_OWNERS_MANUAL.pdf`. Printed page 2 is PDF page 3. New inspection renders are `Research/rendered/gui-panel-03.png` and `gui-sub-03.png`.
2. **Original Roland service notes**, p. 1 dimensions, p. 4 switch/divider circuit. [Original-document scan](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf), local `Research/ROLAND_SH-09_SERVICE_NOTES.pdf`.
3. **Two actual units photographed by their repairer**, [Still Not Working](https://snw.lonningdal.no/sh09.php), [direct photograph](https://snw.lonningdal.no/sh09/Roland_SH-09_repair_01.jpg), local `Research/SH09_front_reference.jpg`. This is the clearest inspected evidence for caps, silver skirts, grooves, legends, switch positions and keyboard construction. `Research/rendered/gui-sub-photo-labels.png` is an enlarged inspection crop; the original remains unchanged.
4. **C. Y. Kong's own unit**, [owner's page](https://www.cykong.com/Synths/Roland%20SH-09/RolandSH09.htm), [near-top photograph](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Top.jpg), [close panel photograph](https://www.cykong.com/Synths/Roland%20SH-09/Images/Orig%20SH-09%20Front%20Panel1.JPG). Local files are `Research/SH09_Kong_top_reference.jpg` and `SH09_Kong_panel_reference.jpg`. The top photo is useful for proportions; this particular instrument is rusty and modified, so its damage is not a universal design specification.
5. **Universal Audio's own Distressor manual and interface illustration**, [manual](https://help.uaudio.com/hc/en-us/articles/18741515014676-Empirical-Labs-EL8-Distressor-Manual), [official interface illustration](https://help.uaudio.com/hc/article_attachments/20381267951252), local `Research/UAD_Distressor_official_reference.png`. Inspected for visual quality and interaction principles only.

## Proportions and confidence

The service specification is **605 mm wide × 305 mm deep × 100 mm high**. Width/depth is **1.9836:1**. At design width 1500, a true top-plan footprint is **1500 × 756.2**. The 100 mm height must not be added directly to the depth of a top-view drawing. A tilted camera changes the apparent depth, and the sloping control face and horizontal keyboard do not share one plane.

The photographs are not calibrated orthographic scans. The coordinate proposal below preserves the documented outer ratio and estimates internal geometry from Kong's near-top image, checked against the manual's module drawing and the sharper two-unit photo. It does **not** establish exact millimetre positions. Exact panel CAD or physical measurement remains unavailable. Expect roughly 2-3% uncertainty in major divisions and several design pixels in control centers.

Kong's 600 × 337 image has an approximately 577 × 286 instrument box at `(9,30)`. Its visible silhouette ratio is 2.02:1, close to the physical specification. The following rounded design coordinates use that silhouette as a guide, with the small vertical correction necessary to recover the official 1.984:1 footprint.

| Feature | Proposed coordinates at width 1500 | Fraction of body / interpretation |
|---|---|---|
| Outer chassis | `(0,0,1500,756)` | Fixed body aspect, excluding software toolbar |
| Side cheeks | approximately `x=0..55`, `x=1445..1500` | Approximately 3.7% width each; dark molded material |
| Upper metal face | approximately `x=60..1440`, `y=12..347` | Approximately 44% body depth including branding and lower bevel |
| Outlined control modules | `x=107..1391`, `y=53..235` | Approximately 85.6% body width; 24.1% body depth |
| Branding band | `x=94..1400`, `y=250..303` | Broad quiet band below modules, not a giant header above them |
| Lower panel fold / key rear shadow | `y=312..350` | Visible separation of the two physical planes |
| Left performance bay | approximately `x=75..285`, `y=350..714` | Approximately 14% body width inside side cheek |
| Keyboard playing surfaces | approximately `x=300..1435`, `y=350..715` | Approximately 75.7% body width and 48.3% body depth |
| Front rail / key-front lip | approximately `y=715..750` | Thin foreground thickness, not a deep extra panel |

A separate plugin strip can sit above or below this body. It must not compress the keyboard, stretch the instrument, or replace the branding band. Fit-to-screen may uniformly reduce the body; arbitrary width-only expansion destroys the physical proportions.

The manual's drawing supplies particularly consistent relative **module widths**. Use these boundaries rather than seven equal columns:

| Module | Left x | Right x | Width | Share of control-module row |
|---|---:|---:|---:|---:|
| MODULATOR | 107 | 252 | 145 | 11.3% |
| VCO | 252 | 621 | 369 | 28.7% |
| SUB OSC | 621 | 697 | 76 | 5.9% |
| AUDIO MIXER | 697 | 847 | 150 | 11.7% |
| VCF | 847 | 1102 | 255 | 19.9% |
| VCA | 1102 | 1170 | 68 | 5.3% |
| ENV | 1170 | 1391 | 221 | 17.2% |

The white frames have small gaps, narrow strokes, and modest corner radii. Section headings interrupt or sit immediately above an inner horizontal rule. They are printed markings on one continuous metal face, not floating rounded cards.

## Mechanisms, counts and arrangement

Inspection counts **18 vertical faders**: 2 Modulator, 2 VCO, 3 Mixer, 4 VCF, 4 ADSR, and 3 beside the keyboard. There are **six three-position short vertical slide switches**, **two large rotary selectors**, **one large volume potentiometer**, **one small fine-tune potentiometer**, **one horizontal spring-return bender**, **one gray push-on/push-off power button**, and **32 keys**. The manual identifies both the LFO-rate and power LEDs. A claim of 19 faders would be incorrect.

Main fader tracks share a common upper/lower range, approximately `y=120..219` in the proposed body. The following centers are rounded implementation guides, not CAD measurements:

| Section | Left-to-right control centers (x), with placement |
|---|---|
| Modulator | mode 135; rate 182; sine delay 224. Mode is left of both faders. Rate LED sits near the rate label. |
| VCO | MOD 284; RANGE 356; WAVEFORM 450; pulse-width 536; PWM mode 586. Small TUNE centered above RANGE, around `(356,86)`. Large rotary centers around `y=169`. |
| Sub | selector 653; middle-position legend to its right, upper and lower legends above/below. |
| Mixer | VCO 733; SUB OSC 777; EXT 819. These are three faders; there is no separate noise fader. |
| VCF | cutoff 880; resonance 923; source switch 974; ENV amount 1025; MOD amount 1072. The switch sits between resonance and envelope amount. |
| VCA | mode 1136. This is a narrow single-switch section. |
| ENV | trigger switch 1200; A 1250; D 1288; S 1326; R 1364. Trigger switch sits to the left of four equal faders. |
| Performance bay | volume around `(140,409)`; power around `(233,400)`; VCO bend 125, VCF bend 170, portamento 235 around `y=520`; horizontal bender around `(178,665)`. |

Original continuous faders show 0 at the bottom and 10 at the top. Their caps are broad black molded pieces with sloped faces, side thickness, fine grooves and a pale horizontal index near the top/front edge. A skinny white line or flat rounded thumb alone does not reproduce that construction. The slot is much narrower than the cap; the scale ticks sit outside it.

Range, waveform and volume use black ribbed cylindrical bodies, a reflective silver skirt/bezel, a metallic top disc and an indicator. Tune is a much smaller black knob. Range has five detents, clockwise 32′, 16′, 8′, 4′, 2′. Waveform has four detents, clockwise noise, saw, square, variable pulse. Original waveform symbols should remain visually distinct at small sizes. Use pointer alignment with the printed detents rather than a continuous numerical textbox.

The six mode controls are **slide switches**: a narrow recessed vertical channel and a short ridged black actuator moving among three stops. They are not long silver toggle levers, rotary knobs, drop-down menus, or three separate illuminated pushbuttons. Draw and hit-test the physical switch; clicking any adjacent legend can also select its position directly.

### Verified top-to-bottom switch order

| Parameter | Top | Middle | Bottom | Existing parameter values in visual order |
|---|---|---|---|---|
| `lfoWave` | Sine | Square | Random / S&H | `0,1,2` |
| `pwmMode` | LFO | Manual | ENV | `1,0,2` |
| `subMode` | -1 octave square | -2 octaves square | -2 octaves narrow pulse | `0,1,2` |
| `vcfEnvMode` | ENV follower | Positive ENV | Inverted ENV | `2,0,1` |
| `vcaMode` | Hold | ENV | Gate | `0,1,2` |
| `envMode` | Gate + Trigger | Gate | LFO | `0,1,2` |

**Sub-order correction:** an initial low-resolution inspection tentatively reversed the middle and bottom waveforms. The enlarged real-unit silkscreen makes the distinction visible: middle is approximately 50% duty square, bottom has a distinctly narrower high interval. The manual front-panel drawing agrees. Do not carry the early tentative reversal into implementation. Existing parameter values already have the correct order.

The bender is a horizontal lever projecting out of a dark rectangular recess; it returns to center. Its two depth faders are separate. Portamento has its own outlined area to their right. The power mechanism is a rectangular push button with ON/OFF markings, not a rotary control.

## Keyboard and material reconstruction

The 32-note F-to-C span contains **19 white and 13 black keys**. Start with F/G/A/B, then two complete C-through-B groups, then C. The resulting black-key groups are 3,2,3,2,3. At a 1135-wide keybed, white pitch is approximately 59.7 design units. An initial black-key width of 0.53-0.58 white pitch and visible length of 0.57-0.63 white-key depth matches the inspected photographs; refine against the rendered comparison. These ratios are image-derived estimates.

Key realism comes from geometry: rounded front corners, thin dark gaps, a visible white front lip, black-key side walls and bevel, soft reflections, and cast shadow onto the white key below. The black-key top is charcoal rather than absolute black. Keep surface lighting consistent with knobs and caps. A pressed key should change its position/shadow subtly; a small unobtrusive state marker may supplement it for MIDI visibility.

The main shell reads as dark charcoal with a slight cool cast; the side cheeks and molded controls have different roughness. Metal rotary skirts are significantly more reflective than paint. White panel ink is slightly softened, not luminous. Hardware seams and screws provide scale. Use one broad upper-left light source, restrained edge highlights and contact shadows. Avoid uniform glossy gradients across every material, bright halos around knobs, excessive scratches, or thick outlines around keys. These are reconstruction recommendations inferred from the inspected photographs, not manufacturer color specifications.

## What to learn from the official UAD Distressor reference

The official interface visibly separates matte panel paint, reflective knob skirts, molded switch faces and recessed LED lenses. Consistent highlights, contact shadows and small edge bevels convey depth without obscuring legends. Treat that as a quality target, using original KB-09 rendering and the SH-09 mechanisms. Do not copy the Distressor knob shape, logo, typography, button artwork or panel layout. This is visual analysis, not a description of UA's proprietary production process.

UA documents direct interaction with labels/LEDs as well as the physical switches, and identifies its added Mix/Headroom controls explicitly. Apply the same usability principle: direct mode-label selection and a separate place for plugin-only functions. UA's published [interaction manual](https://help.uaudio.com/hc/en-us/articles/5085501350932-Using-UAD-Plug-Ins-Manual) supports resizing and fine dragging. Its implementation does not establish how KB-09 currently behaves or guarantee its rendering quality.

## High-DPI / 4K implementation guidance

JUCE distinguishes image pixels from logical points: `ScaledImage` stores the image plus the pixel-to-point scale, and scale 2 means two pixels per point. This is confirmed in the installed JUCE header `juce_graphics/images/juce_ScaledImage.h` and the [official API documentation](https://docs.juce.com/master/classjuce_1_1ScaledImage.html). Therefore a 1500 × 756 logical body needs at least a 3000 × 1512 raster at 200% scale. Larger editor zoom multiplies that requirement again. Four-kilopixel display resolution alone does not specify the required source asset resolution.

Use vector text/markings and either sufficiently large original raster layers or geometry rendered at the current output scale. A 1500-wide flattened texture magnified to 3000 pixels cannot recover surface detail. Keep source artwork, control positions and mouse targets in the same logical coordinate system; device-scale conversion belongs at rendering boundaries.

JUCE's [Graphics API](https://docs.juce.com/master/classjuce_1_1Graphics.html) provides image resampling controls and warns that the current graphics opacity also affects images. Restore opaque image drawing after translucent shadows. The installed `juce_Component.h` documents that `setBufferedToImage` caches the component and children but repaint invalidates affected areas. Cache expensive static material work at an appropriate resolution; do not regenerate texture noise across the complete panel at meter refresh rate. This cache policy is a project recommendation, not a JUCE requirement.

Microsoft's [high-DPI guidance](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows) explains that DPI-unaware/system-DPI windows may be bitmap stretched and that assets/layout must react to DPI changes for crisp per-monitor rendering. A plugin is hosted inside another application's window hierarchy, so do not change the DAW's process-wide DPI state from plugin code. Validate the actual host behavior. UA separately documents [cropped, oversized or blurred plugin windows on Windows](https://help.uaudio.com/hc/en-us/articles/14996936104468-Native-UAD-Plug-in-GUI-Blurry-or-Too-Large-on-Windows), reinforcing that attractive source artwork alone is insufficient.

Suggested acceptance captures: 100%, 150% and 200% Windows scale where available; default and largest editor sizes; editor reopen and host reload; all six switch stops; each rotary detent; pressed white/black keys; mouse targets after resize. Inspect text, scale ticks, texture grain, diagonal edges, knob indicators and key gaps at native pixel size. A screenshot must be from the compiled editor, not only an offline rendering. Document unavailable display configurations instead of claiming them tested.

## Handoff boundaries

This research changes no DSP, parameters, editor code or runtime assets. Geometry is a reconstruction target; exact physical metrology, manufacturer paint specifications and a published UA rendering recipe were not obtained. The completed GUI should be judged against the cited photographs and actual hosted screenshots. `docs/UI.md` describes the earlier vector interface and will need revision by the implementation owner after the new editor behavior is final.
