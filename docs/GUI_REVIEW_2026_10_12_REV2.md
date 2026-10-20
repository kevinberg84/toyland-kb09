> **Historical record.** Version numbers, proposals, pending statuses and test results below refer to the revision/date described, not the current release. See [1.9.0 validation](VALIDATION_1.9.0.md) and [current sound-model evidence](FIDELITY.md).

# Five-stage GUI proposal — revision 2, 12 October 2026

**Uncommitted; awaiting Kevin's preview approval.** No install, packaging, push, publication or version bump is authorized until the GUI is approved. Released 1.4.0 files remain unchanged. The earlier preview and its images remain available for comparison.

Gallery: `artifacts/gui-review-next/revision-2/index.html`, served locally at `http://127.0.0.1:18709/revision-2/index.html`. It contains the same factory patch rendered at five ages, with separate close-ups of the performance bay, cap hardware, black/white keys, cheeks and active Modern footer scales.

## Exactly five appearance stages

| GUI position | Name | Visible missing caps |
|---|---|---|
| 0 | Minty | None |
| 0.25 | Seen Some Shit | None; subtle twist/lean begins |
| 0.50 | Vintage, Baby! | VCO mixer |
| 0.75 | Smells Like 1983 | VCO mixer, LFO delay, EXT mixer |
| 1.00 | Fire Hazard | The above plus RELEASE and VCF MOD |

The GUI AGE slider snaps to these five positions, and its five ticks correspond to these five names. Appearance lookup and drag previews use the same five profiles without crossfading intermediate looks. The underlying parameter identity, range, stored values and audio-engine behavior remain compatible: existing presets and host automation can retain finer AGE values, while their GUI maps to the nearest appearance stage. JUCE's range-update implementation uses `dontSendNotification`, and the preview's legacy-value checks confirm that incoming finer values remain stored. The earlier released appearance was continuous with five cached drag images and ten names; this revision changes that behavior deliberately.

VELOCITY, AFTERTOUCH and INPUT TRIM receive 0–10 ticks and 0/5/10 numerals. Full 30-unit grab areas are retained. AFTERTOUCH remains a variable amount: in Modern, channel pressure opens the filter by an amount scaled by this control, up to three octaves at full pressure and amount. Switching it to binary would remove useful depth control. Authentic behavior is unchanged.

## Materials and spacing

- POWER and the PORTAMENTO column move left 16 design units. Adjacent volume/bender centers move left 8 to retain balance. The right outline is X302 and the first key starts X326, leaving a 24-unit outline-to-key gap (18 from the physical frame). The keyboard right edge and textured strip endpoint remain fixed. The accepted shorter depth tracks, connector shapes, rounded outlines and smaller screws remain.
- Cheek wear adds fixed local scar regions at successive stages. Plastic shading is separated from high-contrast damage so reveal masks do not create soft patch-shaped colour/grain changes. Maximum wear uses the previous half-strength Vintage endpoint, with the right cheek darkened 12% under common left-side lighting. Separate left/right wear sources remain; lower folds stay flush with the strip.
- Smooth plastic replaces the pebbled cap sprite. Fresh photographic shine compresses toward a dull cached body endpoint, with fine individual native handling scuffs developing on the plastic. Silver inserts and side wraps render as native geometry at the actual display resolution, with a fading left highlight dot and neutral pitting. The source sprite no longer supplies a low-resolution painted stripe.
- Healthy bender-tip arcs use 24 segments per quarter, a fuller rounded crown and stronger fresh surface reflections. The approved jagged broken geometry remains, appearing at Fire Hazard.
- POWER gets a visible edge reflection that fades with age. Its raised OFF/depressed ON geometry remains.
- Dust paths tied to the printed PORTAMENTO rectangle are removed. Physical perimeter/bender-well deposits remain.
- White keys gain the approved fine-path scuff treatment with subtle ivory-appropriate contrast and independently seeded placement. Black-key bevels and scratch construction remain as approved.

## Verification and release boundary

The actual compiled GUI fixture checks five-stage names/selection, same-stage appearance stability, stored intermediate AGE compatibility, cap/blade shapes and counts at every stage, missing-cap adjustment/state restore, layout gaps, cached-key visibility, footer bounds, MIDI/bender/mod-wheel feedback, descriptions and rendering performance. Exact final counts and source hashes are in the gallery's `preview-manifest.json` and renderer log. Every linked state PNG and footer crop is checked for readability as an image file.

No DSP or processor source changed. This preview does not establish new Windows plug-in validator, native Mac/Linux release or hardware-audio results. Do not publish a new version from preview-only checks.

## Manual changes staged for acceptance

Replace the ten old appearance names with the five above. Explain the five discrete GUI positions, the variable Modern AFTERTOUCH depth, the additional footer scales, and gradual accumulation of local wear. Replace old cap-loss counts/onsets and panel images. Keep installation instructions, deployed bug-report settings, preset files and unrelated manual material intact. Document finer legacy/automation AGE values in the compatibility section rather than complicating the main playing instructions. Apply these wording/image changes with the approved GUI before packaging.
