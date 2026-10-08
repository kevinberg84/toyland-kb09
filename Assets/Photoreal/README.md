# KB-09 artwork inventory — 0.13.2

Kevin's personal SH-09 photographs guide the current geometry and wear. The reference photographs themselves are not embedded. The replacement Volume knob, bunny sticker and waveform tape are excluded; the original Range/Wave construction also guides the software Volume knob. This inventory describes the 0.13.2 integration, including the v5 knobs and the later POWER close-up. [VALIDATION.md](../../docs/VALIDATION.md) records release-specific binary hashes, completed checks and installed identity; owner visual acceptance remains separate.

## Runtime inputs

The current CMake list embeds **nine PNGs**. Actual source dimensions below were read from their PNG headers. Persistent artwork uses CPU-backed JUCE software storage and high-quality resampling. Playback requires no external image folder or network.

| File | Actual source pixels | Runtime role |
|---|---:|---|
| `chassis-black.png` | 1774 × 887 | Clean neutral-black case |
| `chassis-personal-aged.png` | 1774 × 887 | Registered dust, dulling and damaged paint informed by the user's unit |
| `chassis-stains.png` | 1774 × 887, RGBA | Only the original envelope coffee-ring crop is sampled |
| `knob-personal-overhead-v5.png` | 1254 × 1254, RGBA | Complete near-overhead photographic knob sprite with narrower top inset and thinner black rim, preserving barrel height, grouped ribs and silver skirt; no baked tick |
| `fader-personal-overhead.png` | **1484 × 1060**, RGBA | Overhead cap, with its index crossing the centre of the top and continuing down the sides |
| `front-rail-grain.png` | 2172 × 724 | Fine matte texture for the narrow rail below the keys |
| `keys.png` | 1254 × 1254, RGBA | Shared clean key materials, individually composited over native key geometry |
| `marks.png` | 2172 × 724 | User-supplied Toyland and KEVINBERG 09 / KB-09 letterforms |
| `marks-aged.png` | 2172 × 724 | Worn coverage constrained to those supplied letterforms |

The overhead cap's actual dimensions differ from the earlier 1536 × 1024 cap assets. Its current source crop is `(125, 242, 1240, 565)`, drawn into a centred **28 × 18 body-unit** target. An opaque rounded backing covers the solid core so the slot cannot show through it. The top strip continues down the narrow sides; the vertical front lip stays black.

## Materials and projection

The case crop `(2, 7, 1770, 873)` occupies a 1500 × 750 body. The personal aged endpoint blends over the metal face and performance bay at `0.90 × a^0.85`, where `a` is normalized Age. The open keybed and molded cheeks are excluded. Three retained performance-bay screw patches are composited above the perimeter rules, with their aged source blended at `0.94 × a`. Native pale white/grey deposits cover selected areas of **12 registered case screw heads**, using separate seeds. The earlier baked upper-right POWER screw is covered with nearby registered panel material; two separately shaded native POWER heads now lie above and below the cap on its axis. These two full-head materials age differently, with a darker tarnished upper screw and broader pale oxidation below. Recesses and dull exposed metal remain visible. This depicts oxidation-like wear without identifying the fasteners' alloy or chemistry.

The coffee-ring source crop `(1420, 0, 300, 320)` appears once at normalized Age 0.60 (display 6.0), at fixed 22% opacity. Its placement is shifted **+40 right and −45 up** in body design units to clear the envelope-source switch. GATE / + / TRIG is above that switch's axis, GATE is left of its middle, and LFO is below. The other former generic case-stain events are removed.

The lower rail has a distinct fine matte grain, following photo `20261007_131213.jpg`; the main painted panel remains smoother. The renderer selects the texture's top 70 source rows and places them in the 1382 × 43 body-unit rail region, preserving its outline and upper joint. The selected generator output is `exec-e284c8b1-b01a-45b5-bcc1-5bd7ee2a219c.png`; the exact production record is [front-rail-grain-prompt.txt](front-rail-grain-prompt.txt). The earlier coarse, rock-like candidate was rejected.

The owner subsequently selected the exact generated object `exec-8bcd38b0-7cb9-4ce3-99bc-80b29603b296.png`, superseding the interim return to the pre-photo knob. The current large control draws the **complete photographic sprite**, not a native cylinder reconstructed from texture fragments. The edit request preserves that object's physical barrel height, silver skirt and grouped ribs alternating with wider smooth panels, moving the camera toward approximately 90% overhead. It must not shorten the physical barrel to obtain a smaller visible wall. These are art-direction constraints; the generated image is not measured original-part CAD.

The native projection target is **9° off the panel normal**, sine 0.156434 and cosine 0.987688, shared by the bender, exposed blades and knob overlays. The current v5 sprite uses source crop `(70, 70, 1114, 1114)` to normalize generated padding while retaining the on-panel silver-skirt width. The preceding v4 edit reduced the silver inset by approximately 5%; v5 targeted approximately 25% less radial black-rim thickness without shortening the physical barrel. These are approximate editing constraints, not measured part dimensions. [The v4 prompt](knob-personal-overhead-v4-prompt.txt) and [v5 prompt](knob-personal-overhead-v5-prompt.txt) record those edits.

For square drawing size `d`, the native top-tick centre is `(0.497d, 0.410d)`, with radii `0.079d–0.245d` and width `0.028d`. Its length is approximately one third of the silver top diameter. Only this opaque black tick rotates; the whole-image photographic lighting stays fixed. No collar tick is drawn. Independent rim stipple and grip dust remain native. The clean runtime image has no baked indicator; earlier ticked previews are provenance-only. The approved overhead fader is unchanged.

Cached native CPU material layers cover both the silver top and exposed skirt with fine irregular micro-pitting, independently seeded for Range, Waveform and Volume. Pit positions remain stable while their opacity grows as `a^1.35`; a matte layer grows at `0.84 × a` to reduce the pristine photographic reflections. Returning AGE to zero restores the clean image. Pit density, color and response are illustrative and do not establish a particular alloy or measured corrosion law.

## Native wear and controls

Each of the 32 keys combines shared clean material with an individual tint, polish highlight, seeded microdust and small soil inclusions. Seven fixed grime flecks appear at `a = 0.18 + (note − 53) × 0.023`; their geometry and intensity stay fixed after onset. The fine-dust layer grows gradually in opacity. No photographic dirt stamp is repeated across objects.

Keys become less glossy with Age: the black-key bitmap opacity is `1 − 0.84a` and the white-key bitmap opacity is `1 − 0.64a`, revealing opaque native matte key surfaces beneath. At display AGE 7, those bitmap contributions are **0.412 black / 0.552 white**. Native highlight strength uses `1 − 0.90a`, further reducing shine. This changes surface appearance without making the keys transparent or removing their hit targets.

MIDI 60 alone develops a native hairline at display AGE 5.6: width 0.32 body pixels, opacity 55%, initially 78% of its placed length and extending to 100%. [key-hairline-personal.svg](key-hairline-personal.svg) records the matching path. Neither rejected crack PNG is embedded.

The VCO mixer cap disappears at display AGE 6.4 and Release loses its cap at 8.8. Native thin longitudinal metal edges with small stepped shoulders follow the uncapped-unit photographs; the broad blade face is not presented to the near-top camera. Original slider ranges, hit targets and parameter attachments stay active. Blade dimensions and alloy remain estimates.

Small switches use raised ribbed black tongues. The bender disk and pointer form one rigid extruded outline with estimated depth 7.5 design units. Object vertices rotate together through the existing ±35° travel before the shared near-overhead camera projection; the curved rim uses continuous sampled normals and a small axial bevel to avoid polygon-like shading changes. The estimated intact tip becomes a short broken stump at display AGE 9.2. Bend range and spring return remain active. These geometry and material values are reconstructions, not measured mechanical travel or original-part CAD.

POWER uses a diffuse neutral grey cap (`#a5a6a0`, aging toward `#959487`), a broad 1.8-unit rounded bevel, stable fine molding grain and a dark recess. Estimated physical depths 18 OFF / 7 ON project through the same 9° camera to approximately 2.82 / 1.10 units of visible front wall; the identical top lowers approximately 1.72 units when ON. Its two native screws, filled side-profile ON/OFF button symbols, restrained label wear and separate lamp below the legend follow the owner's close-up. Native CPU material fields supply the plastic bevel and domed screw oxidation, with irregular tapered Phillips recesses. No extra PNG is embedded for this area. See [POWER reference](../../docs/POWER_BUTTON_REFERENCE.md) for evidence and dimensional limits.

Native keyboard feedback draws the effective held note's amber bar at the lower edge immediately during mouse presses and drags, before the next audio callback. Other held notes are blue; release removes the mouse contribution. This feedback is independent of the photographic key materials and shares MIDI's lowest-note priority.

Brand wear removes coverage only within the exact supplied shapes. Native labels, scales and outlines warm with Age; software readouts remain separate. The panel notice identifies an unofficial personal project, unaffiliated with Roland and not for sale.

## Provenance, history and limits

[Production records](PROMPTS.md) distinguish generated source materials from native reconstruction. Version 0.13's 18° native knob cylinder/skirt used `knob-personal.png` as material; that renderer is historical. The installed 0.13.1 revision used the earlier v2 overhead sprite. V3/v4 are intermediate candidates, and the older eight-PNG 0.12 inventory, pre-photo `knob.png`, older cap variants, old wear atlas and generated crack images are also historical. The current nine-PNG source list embeds v5; this does not restore the old wear atlas. Windows VST3, standalone and unsigned AAX development builds share these embedded inputs. AU remains an unbuilt native-Mac target, so artwork integration does not establish Mac wrapper validation.

See [the personal control reference](../../docs/PERSONAL_SH09_CONTROL_REFERENCE.md) for inspected photographs and [material research](../../docs/GUI_MATERIAL_AGING.md) for evidence limits. Warm source lighting is not calibrated colour evidence. Source assets remain finite-resolution: a 3840 × 2160 export does not turn the 1774-pixel case or smaller control crops into native 4K photography. No exact chemistry, measured visual equivalence or physical 4K-display test is claimed here. Build-specific results belong in [VALIDATION.md](../../docs/VALIDATION.md).
