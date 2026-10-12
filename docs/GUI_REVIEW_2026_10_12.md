# GUI proposal — 12 October 2026

Status: **uncommitted, awaiting Kevin's visual approval**. Base release is 1.4.0, HEAD `756b902`. Do not install, package, push, publish, or change version numbers before approval. The released Host-folder files, installed plug-in and Drive downloads remain unchanged.

Actual compiled preview gallery: `artifacts/gui-review-next/index.html`. Includes fresh, Seen Some Shit, Vintage, Baby!, AGE 7, and maximum AGE. Native PNGs and control metadata accompany it. Check counts and exact preview source hashes are in `preview-manifest.json`.

## Proposed changes

- Performance-bay right rule moves from design X245 to X318, below the MOD–RANGE gap midpoint X320. Painted bay and its frame are rebuilt to remove the old seam. Keyboard starts at X326 instead of X286, retaining right edge X1428: 3.5% narrower. The textured strip retains its right edge.
- VCO/VCF bender-depth tracks are shorter than the main panel tracks. Their shared ticks remain close, with outside numerals. New photo-referenced connector lengths, stepped section outlines, larger clear areas around the bender, and small independent screw heads replace the oversized baked heads. Volume/POWER compartments expand with the bay.
- All cap wear starts gradually at normalized AGE .40 (Seen Some Shit). Independently seeded twists reach roughly 2–8.6 degrees; sideways lean reaches .35–1.7 design pixels in independently chosen directions. Exposed blades keep their roots centered on their slots. No controller identities, hit targets or sound settings change.
- Cap losses: VCO mixer .50 (Vintage, Baby!), LFO delay .64, EXT mixer .76, RELEASE .88, VCF MOD .96. Five missing at maximum. All remain adjustable, including after state restore.
- Silver strips have a narrow new-metal reflection and separately seeded gray oxidation/pitting with AGE. No yellow ink or red rust is applied to the strips.
- Cheeks use two distinct photo-informed generated wear sprites, with different gouges, scratches and corner damage. No mirrored or duplicated wear image. Their native clipping ends flush with the textured strip. Asset provenance is `Assets/Photoreal/cheeks-personal-independent-v1.md`.
- Black keys have a more visible 10-design-pixel front chamfer, fine individual scuffs/scratches and broader duller reflections. Fine scratches were reduced after first inspection so they read as surface wear. Pressed-key dimming and disappearing ivory front slivers remain intact.
- POWER retains its approved raised OFF/depressed ON behavior, with a flatter moulded face, smaller bevel, neutral fine texture and a small edge reflection.
- Printing and ivory yellowing are reduced about 15% at the endpoint, with continuous blending.

## Verification

Final GUI renderer: **717 checks passed, zero failures**. New checks locate the actual rendered right divider below the MOD–RANGE gap, confirm shorter bender-depth controls, and check disjoint slider/keyboard hit regions and the fixed keyboard right edge. Existing coverage includes cached keyboard visibility, cap-loss targets/adjustability, all 35 legacy state slots, bender/MIDI/mod-wheel feedback, pressed keys, descriptions and window sizes. Five cap-loss events and exposed-blade presence are exercised.

Final offscreen moving-control median/p95: 7.2964/9.3377 ms, zero chassis rebuilds during the measured sequence. This measures rendering work, not displayed FPS. Gallery reference/PNG checks pass. Audio-engine and processor source files are unchanged; no new audio fix, platform release validation or hardware match is claimed.

## Manual wording to apply after visual approval

“With AGE, cap alignment gradually becomes uneven from Seen Some Shit onward. The first cap disappears at Vintage, Baby!, with more lost at older settings. Missing caps remain fully usable. Silver strips and collars dull and develop fine surface wear without iron rust. Independent cheek scuffs and key scratches accompany gentler ivory and printing yellowing.”

Replace the current manual's three-cap count/onsets and update panel images after acceptance. Keep unrelated instructions, bug-report deployment and platform installation content intact. Choose the release designation according to the existing semantic-version policy when scope is final.

## Audio discussion

The digital garbling remains unresolved. 1.4.0 retained the restored engine. The 64-render slow VCO/self-oscillation study was stable; healthy-AGE 4x/32x differences were small, and envelope distributions were effectively unchanged. That does not prove hardware equivalence or exclude other-patch aliasing. Maximum AGE had greater phase-trajectory sensitivity, which must not be labeled an alias-only metric.

Kevin's hardware captures are the next useful calibration evidence: self-oscillation alone, then saw and square at fixed VCO pitch with a slow FREQ sweep across it. Instructions are `artifacts/gui-review-next/RECORDING_GUIDE.md` and the fuller `docs/SH09_RECORDING_GUIDE.md`. Avoid arbitrary warming EQ/noise or an unvalidated replacement engine. The mixer antialiasing candidate remains a laboratory experiment, not a shipping fix.
