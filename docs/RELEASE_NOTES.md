# 1.9.0 — Toyland KB-09

- Standalone AUDIO / MIDI adds MIDI receive channel: All channels (default) or 1–16. The selection is remembered across launches and applies to all enabled input devices. Changing it releases excluded MIDI holds while preserving GUI notes. Presets and DAW MIDI routing are unchanged.
- Includes the approved RATE LED host-restacking and idle-audio correction, plus serialized startup update and shortcut prompts.
- Downloads, update instructions and the public update feed use GitHub Releases. The few users of older Drive-only builds should upgrade manually from GitHub; further Drive maintenance is retired.
- The manual and installation guides explain release Assets and Source code downloads, retain platform-specific security instructions, and route bug reports to GitHub Issues.
- First-party source, documentation and assets use the KB-09 Free Use and No Sale License. Commercial music-making is allowed; selling the software or modified versions is prohibited. Third-party licenses remain separate.

Windows, universal Mac and Linux automated release checks passed; see VALIDATION_1.9.0.md for scope. Linux physical desktop, audio and MIDI verification remains outstanding. Mac binaries are ad-hoc signed, not Apple-notarized. New builds use the GitHub release page for update downloads. Older Drive-only builds require a one-time manual upgrade from GitHub.

# 1.8.3 — Toyland KB-09

- Fixed the Windows RATE LED being covered after a DAW restacks its floating plug-in editor. The independent surface now repairs its order relative to the editor without becoming globally always-on-top or taking focus.
- Keep the underlying LED renderer current so a covered or temporarily detached native surface cannot expose an old frozen frame.
- The visual RATE now reads current control values even when a host pauses audio processing. RATE and sync division edits no longer depend on another audio callback before the light responds.
- Added hosted VST3 window-restacking regression coverage, shared wrapper lifecycle and idle-audio checks, and native Mac hosted AU/VST3 view checks.
- Serialize automatic update and desktop-shortcut prompts, including their asynchronous creation, so startup questions cannot open on top of one another.

The sound-engine equations, presets, parameter ranges, POWER mechanics and approved LED artwork are unchanged from 1.8.2. Linux physical audio/MIDI playback remains UNVERIFIED. Mac signing remains ad-hoc rather than Apple notarization.

# 1.8.2 — Toyland KB-09

- Integrated the approved centered top panel, evenly spaced border-facing values, consistent extended short ticks, aligned RATE LED and brighter black cap shoulders. All approved POWER motion and audio timing remain unchanged.
- PORTAMENTO, VCO MOD, VCF MOD and VCF ENV/FOL’R depth now use squared physical response for finer low settings, preserving their maximum ranges. MIDI Learn, pickup and CC1 follow the same response. Saved presets and DAW parameter automation retain their effect amounts; recorded MIDI CC performances intentionally use the new curve.
- Linux standalone adds JACK alongside ALSA, including use with PipeWire’s JACK compatibility layer. Windows retains ASIO, WASAPI and DirectSound; macOS retains CoreAudio.
- Refreshed the manual and current GUI illustrations, including response-curve and Linux audio guidance.

The DSP engine, parameter identities, 24 factory sounds and processing latency are unchanged from 1.8.1. Linux hardware playback remains UNVERIFIED. Mac builds are ad-hoc signed, not Apple-notarized. Release builds do not record audio or MIDI diagnostics.

# 1.8.1 — Toyland KB-09

- Corrected POWER timing: switching ON enables audio on press; switching OFF keeps audio on until release. The cap stays fully depressed while held and retains the released spring-back motion at release. The full release animation/repaint path is preserved and covered by regression checks.
- Updated the approved upper and lower panel screw positions, with level pairs and clean surrounding paint.
- Key crack ends have longer, softer feathering. The rightmost crack has the approved off-center position.
- Exposed fader posts are shorter, retaining the approved width, material and value positions.
- Expanded manual section 02 into "How we built the sound engine," preserving the existing filter explanation and waveform illustrations and explaining the model's development and calibration limits.
- The opening states "Free for everyone, forever." Removed the redundant introductory filter link, consolidated technical filter details and refreshed all five AGE illustrations plus the MIDI-mapping illustration to the approved GUI.

The sound engine, parameter identities, preset compatibility, 24 factory sounds and 128-sample processing latency are unchanged from 1.8.0. Windows retains native ASIO, WASAPI and DirectSound. Linux standalone remains UNVERIFIED pending physical Arch audio/MIDI testing. Release builds do not record audio or MIDI diagnostics.

# 1.8.0 — Toyland KB-09

- Windows standalone now offers native ASIO drivers in AUDIO / MIDI, including the installed Audient driver. Existing device choices are retained until changed by the user.
- Includes the approved panel proportions, knob tops/collars and reflections, compact radial scales and printing alignment, inset slider ticks with correct cap positions, and centered VOLUME hardware.
- Revised slider caps have silver bands with a left-side glimmer that narrows with AGE, retained aged shine and progressive tarnish. Highlights stay stable while sliding. Exposed posts are thicker and shorter.
- Refined POWER shape, screws and aging; restored cheek bottom chamfers; added hardware shadows, including white keys and the performance bay onto the textured strip.
- Black keys have longer rounded chamfers, narrower raised tops, material wear and edge glimmer through Fire Hazard. Hairline crack ends fade naturally. White-key active amber markers have a dark backing for clearer contrast.
- Matching LED lenses and single recessed wells reflect red illumination within the ring, with a bottom-right edge highlight. On Windows the RATE light renders independently of the main editor during AGE changes and keyboard activity.
- EXT and IN indicators ignore levels at or below −80 dBFS, so inaudible input noise no longer holds the first display segment on. Audio and envelope-follower processing are unchanged.
- The manual links to Live 12 sidechain routing and explains that KB-09 is free for everyone forever: share it with friends and enjoy.

The sound engine, parameter identities, saved presets, 24 factory sounds and 128-sample processing latency are retained. Native ASIO is a driver option; a universal fix for intermittent clicking has not been established. The release does not record audio or MIDI diagnostics. Linux standalone remains UNVERIFIED pending real Arch audio and MIDI confirmation.

# 1.7.2 — Toyland KB-09

- Refined photographic screw heads, smaller profiles, restrained Minty shine and gradual dark tarnish. Worn heads stay matte without excessive pitting; selected heads become stripped.
- More natural cheek scratches and scuffs, edge and corner gouges, and clearer wear at normal viewing sizes. Recessed cheek joints gather dust along the upper panel; the lower textured rail stays out of the gap.
- Corrected knob top markings and shorter, tapered inner collar ticks, with less top rust and uneven grime deposits.
- The bottom utility strip, its switches and all dialogs remain clean at every AGE setting. Utility caps retain their silver markings.
- Refined POWER and bender markings, ENV / GATE symbols and legend spacing.
- SAVE / DON'T SAVE no longer inherits a drag when it opens; moving it requires a fresh deliberate click.
- Prepared cheek materials and lazy screw rendering reduce editor startup work. RANGE and WAVEFORM retain instantaneous switching between fixed positions.

The sound engine, parameter identities, saved presets, 24 factory sounds and 128-sample processing latency are retained. AUTHENTIC bender return remains 35 ms with a 20 ms visual recoil, and POWER springs back immediately on release. Linux standalone remains UNVERIFIED pending real Arch audio and MIDI confirmation.

# 1.7.1 — Toyland KB-09

- AUTHENTIC bender snaps to center over 35 ms without easing, followed by a small 20 ms visual recoil. Pitch reaches neutral without overshoot. MODERN remains immediate.
- POWER depresses while held and springs back immediately on release, with no extra hold delay. Its darker base, tighter shoulders and upper-left reflection retain a little shine through AGE.
- Corrected inner collar ticks, knob alignment, label sizes and baselines, switch symbols, fader spacing and performance-bay outlines. Larger POWER and bender screws match the panel proportions.
- Refined bender geometry, seam, underside rib, scuffs and progressive finish; knob grips and cheeks age more naturally. The VOLUME knob also gathers dust.
- MIDI MAP is centered while keeping assignable controls exposed. Parameter hovers use the normal arrow cursor consistently.

The sound engine, parameter identities, saved presets, 24 factory sounds and 128-sample processing latency are retained. Linux standalone remains UNVERIFIED pending real Arch audio and MIDI confirmation.

# 1.7.0 — Toyland KB-09

- The standalone remembers enabled MIDI inputs and reconnects them when available. Change your selection in AUDIO / MIDI at any time.
- Use arrow keys to select actions in MIDI MAP and other button dialogs; press Enter to activate the selection. MIDI MAP is centered while keeping assignable controls exposed.
- Refined SH-09-inspired hardware: moving grouped knob grips and top wear, six-rib switches, darker switch-track edges, individual dust patterns, and clearer lighting and aging on POWER, cheeks, screws and the bender.
- The onscreen bender returns at constant speed over 50 ms in AUTHENTIC mode and stops at center without easing. MODERN still returns immediately.
- Improved panel alignment and smaller left-side lettering. Each platform includes its own installation guide, illustrated manual and these release notes.

The sound engine, parameter identities, saved presets and 128-sample processing latency are retained. Linux standalone remains UNVERIFIED pending real Arch audio and MIDI confirmation.

# 1.6.2 — Toyland KB-09

Fixes the ENV meter's first segment remaining lit after a note has released. Meter redraws now follow visible segment transitions, including the final transition to zero; the same correction applies to the external follower and input meters. The audio envelope, MIDI note handling and sound engine are unchanged.

Shortens the AGE and large-window descriptions, adds a Check for updates description to TOYLAND / KB-09, fixes Windows character artifacts in the RATE description, and explains the VCF envelope follower's external-input auto-wah use. Removes two unnecessary troubleshooting entries from all three platform-specific manuals.

Includes a rendered-meter regression covering both MIDI note-off encodings and mouse notes at 48 and 96 kHz. Presets, parameter identities and 128-sample processing latency are retained. Linux remains UNVERIFIED pending real Arch playback and MIDI confirmation.

# 1.6.1 — Toyland KB-09

Centers the performance-bay controls and printing, fills the left end of the textured keybed strip, and balances footer spacing from rendered assets in standalone and plug-in editors.

Provides separate Windows, Mac and Linux manuals and OPEN ME FIRST pages, with platform-specific installation, display troubleshooting and desktop shortcut instructions. Linux includes unverified launch/audio workarounds. The panel guide cycles through all five AGE appearances; the filter-modeling story explains the hardware/software recordings and oscillator/filter interaction.

The sound engine, MIDI behavior, parameter identities, presets and 128-sample processing latency are unchanged. Linux remains UNVERIFIED pending real Arch playback and MIDI confirmation.

# 1.6.0 — Toyland KB-09

Adds standalone MIDI mapping for knobs, faders and buttons. Assignments remember their channel and behavior between launches, separately from sound presets. Continuous controls use pickup; buttons cycle multi-position controls, and knobs/faders select positions directly. CLEAR SELECTED, confirmed CLEAR ALL and optional behavior correction are included. Plug-ins retain host mapping and normal mod-wheel behavior.

Unifies message windows around the Save / Don't Save typography and black/white Toyland styling across standalone and plug-ins. The compact mapping window centers over the keyboard and packs visible buttons with equal gaps. Footer control spacing is balanced in both formats.

Removes the extra outer bottom performance-bay line and lets the rightmost key crack begin at Sold As-Is before reaching its full Fire Hazard length. Updates the manual with standalone mapping instructions. The sound engine, parameter identities, saved patch compatibility, 24 factory presets and 128-sample reported processing latency are retained.

Windows standalone/VST3, universal Mac standalone/AU/VST3 and Linux x86-64 standalone are built from the same source. Linux remains UNVERIFIED pending real Arch launch, playback and MIDI confirmation.

# 1.5.3 — Toyland KB-09

Refines the self-oscillating filter's interaction with VCO input. Stage-dependent headroom and a separate nonlinear feedback limiter improve resonant amplitude consistency during slow sweeps. The coupled stages and limiter are solved together; no automatic leveling or separate sine oscillator is added. Resonance-dependent tuning correction preserves free-running pitch. Natural beating and harmonic interaction remain; individual SH-09 instruments and patches can differ.

Fixes MIDI CC1 losing control when delayed slider feedback was mistaken for a manual edit. VST3 mod-wheel input takes control on the next movement, including after parameter edits or preset changes; the standalone retains soft pickup. Actual VST3 tests exercise every exposed sound parameter, host Bypass/Program, ordinary CCs and recorded-style wheel sweeps. Parameter IDs, preset/state formats, 24 factory sounds, 4x processing and 128-sample reported latency are retained. The revised filter uses somewhat more CPU; the smallest stable buffer depends on the computer and audio setup.

The first white-key crack begins at Vintage, Baby!, at 40% of its Sold As-Is length. The illustrated manual explains the filter-modeling work with hardware/model waveform comparisons, clearer system recommendations, consistent platform troubleshooting boxes, envelope-filter terminology and display gestures. Manual and OPEN ME FIRST use a comfortable dark screen theme, retaining light printing. The manual's footer now reads October 2026.

Windows standalone/VST3, universal Mac standalone/AU/VST3 and Linux x86-64 standalone are prepared from the same source. Linux retains UNVERIFIED pending physical Arch launch, audio and MIDI confirmation. Platform validation and distribution status are recorded separately.

# 1.5.2 — Toyland KB-09

Includes all approved GUI revisions through Revision 17. The bender and pointer have slightly brighter dark surfaces while retaining their polished reflection and rounded geometry. CUTOFF FREQ and RESONANCE printing more closely follows the supplied SH-09 reference. The performance bay now meets the left cheek without the former narrow gap.

Exposed fader posts have a narrow silver edge ahead of a darker blade face. Recessed felt dust covers acquire additional fixed grains and fibres at each of the five AGE stages. Existing deposits retain their contrast; AGE does not reduce control functionality.

The POWER button retains its rounded top and upper-left reflection, with small rounded skirt edges, a softer left shadow and a darker right shadow. Its plastic finish ages more slowly: Fire Hazard retains the sheen formerly shown at Vintage, Baby! The four POWER/bender screw heads have sharper Phillips recesses, distinct orientations and wear, and metal highlights that dull with age.

The audio engine, 35 parameter identities, stored states, automation and 24-sound factory bank are unchanged. Windows, universal Mac and Linux standalone are rebuilt. Linux remains UNVERIFIED pending physical Arch playback/MIDI confirmation. Platform-specific validation is recorded separately.

# 1.5.1 — Toyland KB-09

Approved panel corrections: narrower waveform symbols and refined MODULATOR/PULSE WIDTH printing, balanced ENV track margins, centered performance bay, matching divider heights, and keyboard/cheek seams. The bender has a smoothly rounded pointer, a thicker rounded disc and visible underside rib. Its existing reflection has sharper boundaries; the pointer uses the same polished highlight. Fine tapered handling scuffs follow the rotating plastic and accumulate with age.

Independent photographic cheek wear appears mark by mark as the body lightens progressively. Rust grows spatially in separate patches, retaining the panel's original lighting. Key/cap scuff counts and cap twist/lean progress at 0/25/50/75/100% across the five conditions. Each cap has an individual fixed angle rather than an alternating pattern; FREQ/RES movement is restrained. The PULSE WIDTH cap replaces the former VCF MOD cap-loss event at Fire Hazard; cutoff retains its cap. Missing-cap controls remain usable.

Audio engine, 35 parameter identities, presets, automation and the 24-sound factory bank are unchanged. Windows, universal Mac and Linux standalone are rebuilt. Linux remains UNVERIFIED pending physical Arch playback/MIDI confirmation. Release-specific tests and limitations are recorded separately.

# 1.5.0 — Toyland KB-09

CONDITION / AGE now has five marked GUI positions and five matching appearances: Minty, Seen Some Shit, Vintage, Baby!, Sold As-Is and Fire Hazard. Existing saved finer AGE values and host automation remain continuous in the circuit model and display the nearest visual stage. All 35 parameter identities and the 24 factory presets are retained.

Adds footer scales for VELOCITY, AFTERTOUCH and INPUT TRIM; AFTERTOUCH remains a variable Modern filter-depth control. The performance bay follows the supplied SH-09 proportions with shorter bender-depth tracks, smaller screws, a painted margin up to the keyboard and an open top outline.

Smooth plastic caps develop varied twist, lean and micro-scuffs. Sharp native silver inserts reflect when fresh, then dull and pit without yellowing or rusting. Cap loss progresses from one at Vintage, Baby! to three at Sold As-Is and five at Fire Hazard. The bender pointer breaks at Fire Hazard.

Independent cheek scrapes accumulate at fixed contrast, with the right cheek darker under left-side lighting. Black-key bevels and micro-scuffs, white-key scuffs, a smoother bender tip and refined POWER reflections follow the approved preview. White-key scuffs are lighter at Sold As-Is and strongest at Fire Hazard. Overall yellowing is reduced.

The illustrated manual describes these controls and conditions. The audio engine is unchanged. Windows, Mac and Linux are rebuilt; Linux retains its UNVERIFIED tag pending physical Arch playback and MIDI confirmation. Release-specific validation is recorded separately.

# 1.4.0 — Toyland KB-09

Adds a native Linux x86-64 standalone package, with launch and optional desktop-shortcut scripts and the local manual. Windows and Mac packages keep their existing formats.

Modern TUNE now spans ±12 semitones; Authentic retains ±65 cents. The parameter ID, stored range and centre are unchanged. Modern presets/automation with nonzero TUNE values intentionally cover a wider pitch range.

The stationary panel is cached at display resolution. Keyboard note changes repaint affected areas, the bender rasterises at display resolution, and AGE preview warming waits until performance is idle. This prioritises moving controls without changing audio scheduling. Panel printing, power-button height, plastic chamfers and wear follow Kevin's SH-09 reference photographs. AGE never disables controls.

Persistent artwork and control caches use CPU memory, including the aged rail texture and dialog logo. This fixes a graphics-driver wait observed when Windows unloaded an aged plug-in instance.

Keyboard component-cache clearing restores opaque drawing before blitting keys, with fractional display-scale coverage verified. Slider groups use shorter shared rules and rounded, shaded openings. Plastic edge damage has irregular textured outlines; collar index marks match the top marks' thickness and stop short of the rim.

Fader-cap index strips are silver, retaining photographic reflections when fresh and dulling with AGE without yellowing or rust. Waveform symbols and NOISE are smaller, and the pulse-width slider and bracket have more clearance. The first Linux download is tagged UNVERIFIED pending playback confirmation on a real Arch desktop.

The restored filter remains the shipping signal path. Slow VCO/filter pitch interactions are being measured separately; no hardware-accuracy or alias-elimination claim is made.

# 1.3.0 — Toyland KB-09

Restored the pre-upgrade 1.1.3 oscillator, filter, saturation and output engine after listening feedback on 1.2.0. Four-times processing and 128-sample reported latency return with it. The preserved 1.1.3 engine is compiled separately for sample-for-sample regression comparison. The manual now describes the restored circuit-informed approximation without claiming the superseded component model.

Modern FREE mode extends the lowest fifth of RATE travel to 0.05 Hz (20 seconds per cycle), preserving the higher-rate mapping. Authentic retains Roland’s published 0.2–25 Hz range; tempo-sync divisions are unchanged.

Incoming MIDI pitch wheel now moves the GUI bender, including when reopening its window. GUI feedback does not retransmit MIDI, override a mouse drag or reset an external wheel when the editor closes. Record controller movement through MIDI Pitch Bend envelopes; the existing VCO/VCF depth parameters remain separately automatable.

Releasing the onscreen bender now returns its sound and animation over approximately 100 ms in AUTHENTIC mode. MODERN returns immediately with existing audio smoothing. New wheel or mouse input interrupts the return. The short spring curve is a playing approximation, not hardware-calibrated mechanical modelling. External wheels follow their own MIDI messages without an added spring delay.

MIDI CC1 now controls VCO MOD in both modes, with pickup after preset loads or manual/host edits. The visible fader follows, its setting is saved, and sound changes at the incoming event without waiting for GUI delivery. Parameter notifications stay off the audio thread.

The MIDI lamp refreshes independently at 120 Hz with a shorter 90 ms activity flash. Host buffering and message-thread rendering can still affect visible latency. Aftertouch help now explicitly describes its Modern channel-pressure-to-filter route. Parameter IDs, preset formats, and the 24 factory definitions are retained.

# 1.2.1 — Toyland KB-09

Fixed the bare chassis flashing while dragging AGE during parameter automation. The editor keeps the last complete instrument preview while building a replacement bank; incomplete banks never replace it. A complete startup preview also keeps the first AGE drag responsive. Releasing AGE restores live control positions and keyboard feedback. The audio engine, parameter IDs and preset definitions are unchanged from 1.2.0.

# 1.2.0 — Toyland KB-09

Historical release: its filter/output revision was reverted in 1.3.0; the expanded preset bank remains.

The filter now models four coupled OTA stages and the opposing-diode resonance limiter described in the SH-09 service schematic. Self-oscillation comes from the filter feedback loop, with stronger output and restrained harmonic colour. Eight-times oversampling and continuous AGE changes support nonlinear headroom and asymmetry. This is a reduced circuit model; hardware-level accuracy has not been established.

The bank now contains 24 factory sounds, preserving the original sixteen definitions and order and appending eight bass, PWM, expression and filter-feedback patches across AGE 0–10. The manual covers the filter model, new sounds and system recommendations. The GUI and textured strip remain as released in 1.1.3.

Presets, parameter IDs and automation formats are preserved. Self-oscillating patches may need cutoff/level adjustment: the representative fresh case is 3.23 dB stronger and 145 cents higher at the same cutoff. The measured core costs about 2.4 times as much CPU; start at 44.1/48 kHz. Reported latency remains 128 samples.

Windows: six test suites, 548 GUI checks, 47 Steinberg VST3 tests, strictness-10 headless pluginval and isolated installer verification passed. The actual VST3 host passed 75 assertions but waited at shutdown; this is not a clean host exit. Similar GUI-validator shutdown waits were reproduced on released 1.1.3. Mac: universal standalone/AU/VST3 built; six suites, 548 GUI checks and Apple AU validation passed on the arm64 CI runner. Intel execution and comparison against Kevin's hardware remain to be evaluated. Full measurements and limitations are recorded in the filter validation artifacts and model notes.

Existing shared Drive folder and download file IDs are retained. Both current ZIP downloads must be hash-verified before publishing the update announcement.

# 0.13.2 — Toyland KB-09

This update brings the owner's final control/material refinements into the Windows VST3 and standalone build. A real Windows AAX Native development bundle also compiles from the shared source; it is **unsigned and untested in Pro Tools**, so it belongs in the package's clearly labeled Development area. Native AU configuration and Mac build instructions are supplied, but no AU binary has been built because no Mac/Xcode host is available. A Mac source kit is not an installable Audio Unit.

- The selected v5 photographic knobs retain their full physical barrel height, near-overhead view, silver skirt and grouped ribs alternating with smooth panels. Silver top insets and black rims are narrower. The native black top tick is approximately one third of the silver top's diameter; photographed lighting stays fixed as the tick rotates, and no collar tick is drawn.
- Fine irregular micro-pitting develops across each silver top and skirt with independent deterministic patterns. Reflections dull progressively with AGE so the knobs age with the panel; returning AGE to zero restores the clean metal. This is photo-informed illustrative wear, not a measured corrosion law.
- The POWER area follows the owner's close-up: diffuse grey plastic, a broad rounded bevel, substantial raised OFF position and lower ON position, filled button-profile ON/OFF symbols, restrained print wear and the lamp below the legend. Its two surrounding screws are aligned with the cap and age differently: a dark tarnished upper head and broader pale oxidation below. Height, travel, alloy and chemistry are not measured hardware claims.
- The approved top-down fader caps retain their stripe across the top and down the sides, with black front lips. Thin exposed fader blades and the rigid extruded bender retain the shared 9° native camera target; the bender's disk and pointer rotate together. Matte key aging, the almost hairline single crack, fine textured lower rail, moved coffee ring and corrected GATE / + / TRIG selector legends remain.
- Click-and-hold mouse playing now shows the effective note's amber bar immediately at the key's lower edge. Presses, upward/downward drags and release update before the next audio callback; MIDI and mouse retain the same lowest-held-note priority, with other held notes blue.
- The offline HTML guide and release image are updated for 0.13.2. The desktop package separates usable Windows VST3/standalone files from unsigned AAX development material and Mac build source, with explicit format status and preserved dependency notices. Exact assembled files and installation status are recorded separately.
- Windows file/product version resources now regenerate when the project version changes. Explorer, the plugin manifest and all three Windows binaries report 0.13.2 consistently after an incremental build.

The DSP and parameter/state identities remain unchanged from the preceding release. No additional age-dependent saturation was introduced: the [research](SH09_AGE_DISTORTION_RESEARCH.md) does not establish a universal distortion curve for aged SH-09 hardware. Older Modern patches retain the deeper MOD mapping introduced in 0.11; Authentic mappings are unchanged.

Release-specific test counts, binary hashes, package checks and installed identity belong in [VALIDATION.md](VALIDATION.md). Successful compilation alone does not validate the AAX wrapper or establish a new native DAW, Mac/AU, Pro Tools or physical-display test. [Format status](FORMAT_ROADMAP.md) and [known limitations](KNOWN_LIMITATIONS.md) preserve these boundaries.

# Historical 0.12.0 — Toyland KB-09

Photo-informed controls and AGE wear now follow Kevin's personal SH-09. This Windows VST3/standalone build includes corrected compact opaque caps, stripes across their top and down the sides, native black knob-top indicator ticks, stepped exposed fader blades, raised ribbed switches and an arched bender. The owner's replacement volume knob, sticker and waveform tape are excluded.

- The personal instrument represents AGE 7. Gradual dust collects at seams, slider openings, knob sides and key gaps; ink and individual keys yellow, metal dulls and knob rims tarnish finely. Gray screw heads replace red-rust stamps; brown corrosion follows exposed panel paint. The existing envelope coffee ring is retained. The old generic wear atlas is excluded from runtime.
- MIDI 60 alone receives a nearly hairline crack, with no broad shadow or halo. It appears at displayed AGE 5.6 and can lengthen slightly. The VCO mixer cap disappears at 6.4, RELEASE at 8.8, and the bender tip breaks at 9.2. These discrete damage events leave the controls functional and reverse when AGE is reduced.
- S joins M/L: approximately 65%, 90% and the full available work area. Complete header/panel/footer fitting remains uniform. A small unofficial/unaffiliated/not-for-sale notice is printed above the panel.
- The personal test ZIP includes VST3, standalone, 16 exported factory sounds and a branded offline HTML guide. It documents the owner's confirmed Live Auto-Scale remedy and individually sourced settings for other DAWs. AU/macOS and AAX remain future builds.

All current checks pass: DSP 92/0, processor 20/0 with zero tracked allocations, preset files 58/0, compiled GUI 278/0, Steinberg 47/0, actual-VST3 host 75/0 including a 600-second render, and GUI-enabled Pluginval strictness 10 with exit 0. The installed module matches the tested build. Exact hashes and evidence boundaries are in [VALIDATION.md](VALIDATION.md). Physical 4K/mixed-DPI and new-build owner Live recall remain separate checks.

DSP and parameter/state identities are unchanged from 0.11. Older Modern patches still have the deeper MOD mapping introduced in 0.11; Authentic mappings are unchanged.

# Historical 0.11.0 — Toyland KB-09

This release adds user-preset files, deeper Modern modulation, revised black-metal materials and Age wear, clearer keyboard/value feedback, and a correction to Windows editor sizing. The single Toyland KB-09 identity, VST3 identifiers, 35 parameter slots and schema-1 sound state remain compatible. **Older Modern patches with nonzero MOD can sound deeper** because the Modern depth mapping has changed.

## Changes

- The smooth black-painted chassis has restrained reflections. Age gradually develops rust/patina and yellowed ink/keys on the main face, fold and performance bay, tarnishes knobs and chips branding strictly inside the supplied original letter masks. Six case stains appear separately at 0.25/0.38/0.49/0.60/0.72/0.86 with fixed size and 22% drawing opacity. Registered aged screw patches preserve their panel positions; the clean front rail is dulled natively.
- Each key has a distinct tint and its own independent photographic stain, used once. Key stains appear individually from Age 0.18 to 0.893 and retain their size/opacity. Transparent cracks start on MIDI 60/77 at 0.82/0.94, initially showing 78% of their eventual length, then extending the remaining 22%. All fader caps remain fitted; the unverified exposed-post depiction has been removed. Nine runtime PNGs provide separate patina, case stains and a 37-mark atlas: 32 unique key stains, two independent fractures and three independent rotary corrosion patches. No mirrored fracture or repeated aged-key/knob stamp is drawn. Age remains an illustrative condition/tolerance profile, not a measured aging curve.
- The bender depicts a curved PB-4-style sector beneath a horizontal opening, with a projecting tab and spring return. Its inferred front-to-back axle and ±35° projected travel are engineering estimates supported by cross-model PB-4 documentation, not measured SH-09 mechanics. Compact physical switches, calibrated faders and detented upper rotary arcs remain.
- M now uses 90% of monitor work-area dimensions, about 20% larger linearly. M/L sizing uses actual native child-client pixels independently of host movement eligibility, optional measurable surrounding chrome, and a bounded initial fit after peer scale stabilizes. The complete footer scales within standard, wide and tall editor rectangles. The helper does not change process-wide DPI awareness or the DAW's renderer; host-owned frame restrictions still apply.
- SAVE/LOAD use native asynchronous dialogs for `.kb09` presets and compatible earlier `.sh09state` exports. Validated schema-1 current sound state is saved separately from the retained sixteen-program factory bank. Invalid loads leave the current patch unchanged. Checked temporary writes protect existing files; weak callback/owner lifetime checks cover editor closure.
- Loaded user filenames appear in the preset field. Recalling a factory program resets the displayed name even when its index matches the stored reference, using a program revision counter.
- Modern LFO MOD maxima increase to **±36 semitones VCO / ±8 octaves VCF**. Authentic retains **±12 semitones / ±5 octaves**. These engineering ranges are unmeasured; the separate bender depth controls are unchanged.
- Held white and black MIDI notes now have verified visible feedback. Value-bubble colours have a checked **12.68:1 configured contrast ratio**; this is not a full accessibility certification.

## 0.11 validation record

Final October 7 results apply to VST3 module SHA-256 `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`, identical in the build, staged bundle and installed location. The standalone SHA-256 is `93F1660185E8C67781D0118DAED461598080580EB30976AB6D5C11D227E6216A`.

| Check | Result | Evidence under `artifacts/Validation-KB09` |
|---|---|---|
| DSP | 92 passed, 0 failed | `dsp-0.11-materials-final.txt` |
| Processor | 20 passed, 0 failed; 0 tracked C++ allocations | `processor-0.11-aging-events-final.txt` |
| Preset files | 58 passed, 0 failed | `presets-0.11-aging-events-final.txt` |
| Compiled GUI | 280 assertions, 0 failures | `gui-0.11-aging-events-final.txt` |
| Steinberg validator | 47 passed, 0 failed; exit 0 | `steinberg-0.11-final.txt` |
| Pluginval strictness 10, GUI enabled | SUCCESS; confirmed exit 0 | `pluginval-0.11-final-level10.txt`, `pluginval-0.11-final-exit.txt` |
| Actual VST3 host | 75 passed, 0 failed; 600-second render | `host-0.11-final.txt` |

GUI exports and component metadata are in `artifacts/GUI-0.11-final`, including 2880 × 1620 and 3840 × 2160 images and bender positions. Tests cover footer bounds at wide/tall sizes, held-key feedback, configured contrast, fixed stain events, separate crack onsets and distinct healthy/aged samples of all 32 keys. Subtle individual polish reflections prevent duplicated healthy samples at 8-bit output precision. A physical 4K display was unavailable; the chassis remains a **1774 × 887 source raster**. Native hover-popup placement was not observed.

The final VST3 is installed at `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; both release formats are staged in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`. The actual VST3's 600-second offline render consumed **51.0971 seconds of callback time**, with a maximum measured 128-sample callback of **1.006 ms** at 48 kHz. This is a single offline measurement, not a DAW CPU guarantee. The preceding 0.11 standalone M/L were observed at **857 × 534 / 1280 × 720 logical units**, with the complete footer, on a 1920 × 1080 physical display at 150% scaling.

Live-specific 0.11 checks were not repeated: desktop automation could not resolve Live's foreground process after three recovery attempts. Earlier Live observations remain explicitly scoped in [LIVE_TEST.md](LIVE_TEST.md). [VALIDATION.md](VALIDATION.md) and [ACCEPTANCE.md](ACCEPTANCE.md) record current software-test scope and artifact identity; actual-module tests do not substitute for every Live workflow.

## Historical 0.10 checkpoint

Version 0.10 established the Toyland identity, photographic-style panel, power control and native window actions. Its October 5 records contain 83 DSP, 17 processor, 94 GUI, 75 actual-VST3 host and 47 Steinberg passes, plus GUI-enabled pluginval strictness 10 with confirmed exit 0. CPU-backed artwork and software rendering on the editor's own Windows peer avoided the observed NVIDIA-driver shutdown wait. Those results describe that binary and do not certify 0.11. Installation/recall status is recorded centrally rather than inferred from these older logs.

## Fidelity scope

The instrument remains a circuit-informed approximation. Physical SH-09 captures, a validated full-circuit SPICE reference and a completed hardware listening A/B are unavailable. Waveform shape, source gains, saturation, filter/follower behaviour, control tapers and condition offsets retain the explicit [known limitations](KNOWN_LIMITATIONS.md). Artwork is a finite-resolution reconstruction, not measured panel CAD. Generated assets and factory patches are original project content; branding marks were supplied by the user. No Universal Audio graphics or commercial presets were copied.
