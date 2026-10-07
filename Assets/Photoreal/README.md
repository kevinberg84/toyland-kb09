# KB-09 artwork inventory — 0.12.0

The October 7 update uses Kevin's personal SH-09 photographs as the visual reference for AGE 7. The photos inform geometry and wear; they are not copied into runtime graphics. The replacement Volume knob, bunny sticker and waveform tape are excluded. The stock Range/Wave knob geometry is also used for Volume.

## Runtime inputs

CMake embeds exactly eight PNGs. Persistent images use CPU-backed JUCE software storage; downscaling uses high quality resampling. Runtime requires no external image folder or network.

| File | Actual pixels | Use |
|---|---:|---|
| chassis-black.png | 1774 × 887 | Clean neutral black case |
| chassis-personal-aged.png | 1774 × 887 | Registered dust, dulling and damaged paint based on the user's unit; subdued metallic screw heads |
| chassis-stains.png | 1774 × 887 RGBA | Only the existing envelope coffee ring is retained |
| knob-personal.png | 1254 × 1254 RGBA | Flared silver skirt, ribbed black barrel, smaller flat metal top |
| fader-personal-v2.png | 1536 × 1024 RGBA | Compact cap based on the old footprint; stripe across top and down sides, broad finger face black |
| keys.png | 1254 × 1254 RGBA | Common clean key materials, individually composited |
| marks.png | 2172 × 724 | User's corrected Toyland and KEVINBERG 09 / KB-09 marks |
| marks-aged.png | 2172 × 724 | Wear mask constrained to the supplied lettering |

The case crop is (2, 7, 1770, 873), placed on a 1500 × 750 body. The aged case blends over the metal face and performance bay at 0.90 × AGE^0.85. The keybed opening is excluded. Four performance-bay screws are repainted above the perimeter rule, with their aged source blended at 0.94 × AGE. The front rail retains its geometry and receives native dulling only. This is image compositing, not a physically relit 3-D material.

Only stain crop (1420, 0, 300, 320) is drawn, once, at AGE 6.0 with fixed 22% opacity. It preserves the requested envelope coffee ring. Other old generic case stains and the entire old wear-atlas.png are no longer sampled.

## Native details and independent wear

Each of the 32 keys has its own tint, polish highlight, seeded microdust and small soil inclusions. Dust gathers gradually along back edges/gaps and bevels. Seven fixed grime flecks appear at normalized AGE 0.18 + (note - 53) × 0.023. Their positions and sizes do not grow. No photographic stain stamp is repeated on another object. These are individual composites of common clean materials, not 32 independently photographed keys.

MIDI 60 alone develops a hairline at AGE 5.6. The owner clarified that it is almost a hairline, so both generated crack images with broad shading were rejected. The native curved path is 0.32 body pixels wide at 55% opacity, without a surrounding dirt halo. It initially reveals 78% of its length, extending to 100% at maximum AGE. key-hairline-personal.svg records the matching path; runtime draws it natively for sharp scaling.

The new knob photograph intentionally has no fixed index. A separate solid black tick rotates on its projected top disc at normalized centre (0.495, 0.300); lighting stays fixed. Independent seeded, fine golden-brown stippling follows each metal rim. Fine gray dust gathers on the grip sides. No large generic corrosion patch is stamped on the top. The photos do not identify an alloy or prove that aluminum rusts brown.

The corrected fader stripe runs across its TOP and down its SIDES, never across the broad front finger face. An opaque registered backing prevents source-alpha leakage through the plastic. At AGE 6.4 the VCO mixer cap disappears; at 8.8 RELEASE loses its cap. Flat stepped blades, bright cut ends and distinct scuffs follow photos 131114/131131/131137. All original controls and attachments remain usable.

Small switches have raised ribbed black tongues without a painted white index. The bender is a raised arched wheel with molded channels and an estimated intact tip. At AGE 9.2 the tip becomes a short broken stump. Bend range, pointer operation and spring return remain functional. The personal reference document distinguishes directly seen features from estimated intact-tip geometry.

Brand wear only removes coverage within the original supplied shapes. Native labels, scale marks and outlines yellow; parameter readouts remain readable. The panel notice identifies this as an unofficial personal project, unaffiliated with Roland and not for sale.

## Production and limits

Selected new bitmaps were produced with the built-in image-generation tool. Exact prompts are chassis-personal-aged-prompt.txt, knob-personal-prompt.txt and fader-personal-v2-prompt.txt. The earlier fader-personal.png was rejected after the user's geometry correction; key-crack-personal.png and the generated wear-grid variants were also rejected. Historical inputs/prompts remain provenance only and are not silently substituted into the build.

See ../../docs/PERSONAL_SH09_CONTROL_REFERENCE.md for the inspected photo evidence and ../../docs/GUI_MATERIAL_AGING.md for material research. The user's warm lighting is not calibrated color evidence. No exact dimensions, chemistry, commercial-plugin equivalence or physical 4K-monitor test is claimed. Native text and details render at output resolution; the 1774-pixel case source remains finite even in a 3840 × 2160 export. See ../../docs/VALIDATION.md for the current compiled results.
