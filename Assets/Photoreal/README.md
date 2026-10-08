# KB-09 artwork inventory — 0.13.1 source

Kevin's personal SH-09 photographs guide the current geometry and wear. The reference photographs themselves are not embedded. The replacement Volume knob, bunny sticker and waveform tape are excluded; the original Range/Wave construction also guides the software Volume knob. This inventory describes the 0.13.1 integration. [VALIDATION.md](../../docs/VALIDATION.md) records the installed build's hashes and passing checks; owner visual acceptance remains separate.

## Runtime inputs

The current CMake list embeds **nine PNGs**. Actual source dimensions below were read from their PNG headers. Persistent artwork uses CPU-backed JUCE software storage and high-quality resampling. Playback requires no external image folder or network.

| File | Actual source pixels | Runtime role |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | Clean neutral-black case |
| `chassis-personal-aged.png` | 1774 × 887 | Registered dust, dulling and damaged paint informed by the user's unit |
| `chassis-stains.png` | 1774 × 887, RGBA | Only the original envelope coffee-ring crop is sampled |
| `knob-personal-overhead-v2.png` | 1254 × 1254, RGBA | Complete near-overhead photographic knob sprite derived from the owner's expressly selected object; no baked tick |
| `fader-personal-overhead.png` | **1484 × 1060**, RGBA | Overhead cap, with its index crossing the centre of the top and continuing down the sides |
| `front-rail-grain.png` | 2172 × 724 | Fine matte texture for the narrow rail below the keys |
| `keys.png` | 1254 × 1254, RGBA | Shared clean key materials, individually composited over native key geometry |
| `marks.png` | 2172 × 724 | User-supplied Toyland and KEVINBERG 09 / KB-09 letterforms |
| `marks-aged.png` | 2172 × 724 | Worn coverage constrained to those supplied letterforms |

The overhead cap's actual dimensions differ from the earlier 1536 × 1024 cap assets. Its current source crop is `(125, 242, 1240, 565)`, drawn into a centred **28 × 18 body-unit** target. An opaque rounded backing covers the solid core so the slot cannot show through it. The top strip continues down the narrow sides; the vertical front lip stays black.

## Materials and projection

The case crop `(2, 7, 1770, 873)` occupies a 1500 × 750 body. The personal aged endpoint blends over the metal face and performance bay at `0.90 × a^0.85`, where `a` is normalized Age. The open keybed and molded cheeks are excluded. Four performance-bay screw patches are composited above the perimeter rules, with their aged source blended at `0.94 × a`. Native pale white/grey deposits then cover selected areas of all **13 screw heads**, using separate seeds. Recesses and dull exposed metal remain visible. This depicts oxidation-like wear without identifying the fasteners' alloy or chemistry.

The coffee-ring source crop `(1420, 0, 300, 320)` appears once at normalized Age 0.60 (display 6.0), at fixed 22% opacity. Its placement is shifted **+40 right and −45 up** in body design units to clear the envelope-source switch. GATE / + / TRIG is above that switch's axis, GATE is left of its middle, and LFO is below. The other former generic case-stain events are removed.

The lower rail has a distinct fine matte grain, following photo `20261007_131213.jpg`; the main painted panel remains smoother. The renderer selects the texture's top 70 source rows and places them in the 1382 × 43 body-unit rail region, preserving its outline and upper joint. The selected generator output is `exec-e284c8b1-b01a-45b5-bcc1-5bd7ee2a219c.png`; the exact production record is [front-rail-grain-prompt.txt](front-rail-grain-prompt.txt). The earlier coarse, rock-like candidate was rejected.

The owner subsequently selected the exact generated object `exec-8bcd38b0-7cb9-4ce3-99bc-80b29603b296.png`, superseding the interim return to the pre-photo knob. The current large control draws the **complete photographic sprite**, not a native cylinder reconstructed from texture fragments. The edit request preserves that object's physical barrel height, silver skirt and grouped ribs alternating with wider smooth panels, moving the camera toward approximately 90% overhead. It must not shorten the physical barrel to obtain a smaller visible wall. These are art-direction constraints; the generated image is not measured original-part CAD.

The native projection target is now **9° off the panel normal**, sine 0.156434 and cosine 0.987688, shared by the bender, exposed blades and knob overlays. For square drawing size `d`, the top-tick centre is `(0.50d, 0.435d)`, with radii `0.20d–0.329d` and width `0.026d`. Only this opaque black tick rotates; the whole-image photographic lighting stays fixed. Independent rim stipple and grip dust remain native. The clean sprite and non-runtime ticked preview, `knob-personal-overhead-v2-preview.png`, are both 1254 × 1254. [The saved prompt](knob-personal-overhead-v2-prompt.txt) records the selection, camera edits and final tick removal. The preview is not an additional runtime input. The approved overhead fader is unchanged.

## Native wear and controls

Each of the 32 keys combines shared clean material with an individual tint, polish highlight, seeded microdust and small soil inclusions. Seven fixed grime flecks appear at `a = 0.18 + (note − 53) × 0.023`; their geometry and intensity stay fixed after onset. The fine-dust layer grows gradually in opacity. No photographic dirt stamp is repeated across objects.

Keys become less glossy with Age: the black-key bitmap opacity is `1 − 0.84a` and the white-key bitmap opacity is `1 − 0.64a`, revealing opaque native matte key surfaces beneath. At display AGE 7, those bitmap contributions are **0.412 black / 0.552 white**. Native highlight strength also decreases. This changes surface appearance without making the keys transparent or removing their hit targets.

MIDI 60 alone develops a native hairline at display AGE 5.6: width 0.32 body pixels, opacity 55%, initially 78% of its placed length and extending to 100%. [key-hairline-personal.svg](key-hairline-personal.svg) records the matching path. Neither rejected crack PNG is embedded.

The VCO mixer cap disappears at display AGE 6.4 and Release loses its cap at 8.8. Native thin longitudinal metal edges with small stepped shoulders follow the uncapped-unit photographs; the broad blade face is not presented to the near-top camera. Original slider ranges, hit targets and parameter attachments stay active. Blade dimensions and alloy remain estimates.

Small switches use raised ribbed black tongues. The bender uses the shared camera for a curved wheel and estimated intact tip; its tip becomes a short broken stump at display AGE 9.2. Bend range and spring return remain active. POWER uses native pale ivory bevels, fine stable molding grain, a dark recess and state movement, with a separate lamp.

Brand wear removes coverage only within the exact supplied shapes. Native labels, scales and outlines warm with Age; software readouts remain separate. The panel notice identifies an unofficial personal project, unaffiliated with Roland and not for sale.

## Provenance, history and limits

[Production records](PROMPTS.md) distinguish generated source materials from native reconstruction. Version 0.13's 18° native knob cylinder/skirt used `knob-personal.png` as material; that renderer is historical. The older eight-PNG 0.12 inventory, pre-photo `knob.png`, older cap variants, old wear atlas and generated crack images are also historical. The current nine-PNG source list uses the newly selected complete knob sprite; this does not restore the old wear atlas.

See [the personal control reference](../../docs/PERSONAL_SH09_CONTROL_REFERENCE.md) for inspected photographs and [material research](../../docs/GUI_MATERIAL_AGING.md) for evidence limits. Warm source lighting is not calibrated colour evidence. Source assets remain finite-resolution: a 3840 × 2160 export does not turn the 1774-pixel case or smaller control crops into native 4K photography. No exact chemistry, measured visual equivalence or physical 4K-display test is claimed here. Build-specific results belong in [VALIDATION.md](../../docs/VALIDATION.md).
