# Validation record

Current release: [Toyland KB-09 1.5.0 validation](VALIDATION_1.5.0.md). The records below are retained as historical evidence.

## Historical Windows build â€” Toyland KB-09 0.13.2, 2026-10-07

The final v5 knob proportions, long native top ticks, gradual silver micro-pitting, mouse-held amber note feedback and photo-referenced grey POWER cap/screw area are built into the Windows VST3 and standalone. A real x64 AAX Native development bundle also compiled using JUCE's bundled AAX SDK 2.9. **AAX is unsigned and has not been loaded in Pro Tools. AU is not built: no Mac/Xcode host is available.** The package supplies a separate Mac source/build kit rather than an installable `.component`.

- VST3 inner module SHA-256: `5257325EF38A733D716D85B303443D61BEDBAC833A202A0FC348B3E3E2C9F7BE`.
- Standalone SHA-256: `ADA71047F66F13DBC72ED457D9B5FB6A20F6D1145CE2A9215ED5E3E8D4EE4B64`.
- Unsigned AAX inner module SHA-256: `77C75AE3101618096A7B8FDC04B2236A1D8447394AA5C8CA25F3047C0ED4376C`.
- Installed VST3: `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; both bundle files match the tested build. The previous 0.13.1 bundle is verified outside scanned folders at `artifacts/Backup/Toyland KB-09-0.13.1-before-0.13.2/Toyland KB-09.vst3`.
- Binary source starts at `c4f3f75244518eeadc26575ed634254f5e97f7cb`, plus the 0.13.2 format/version CMake configuration, SHA-256 `70ADB94825AC6472942A91F434E5D6AD06AD6A649B2A8008AA35B698CA84BD45`. Release documentation does not alter binary inputs.

| Check on final release | Result | Log under artifacts/Validation-KB09 |
|---|---|---|
| Configure and Release build | Exit 0; VST3, standalone, AAX development; version resources refreshed | configure-0.13.2-metadata.txt; build-0.13.2-final.txt; build-0.13.2-metadata.txt; build-0.13.2-exit.txt |
| DSP | 92 passed / 0 failed | dsp-0.13.2-final.txt |
| Processor | 20 passed / 0 failed; zero tracked C++ new allocations | processor-0.13.2-final.txt |
| Preset files | 58 passed / 0 failed; final actual-host bank | presets-0.13.2-final.txt |
| Compiled editor | 347 checks / 0 failures | gui-0.13.2-final.txt |
| Steinberg VST3 validator | 47 passed / 0 failed; exit 0 | steinberg-0.13.2-final.txt |
| Pluginval 1.0.4 level 10, GUI enabled | SUCCESS; confirmed process exit 0 | pluginval-0.13.2-final-confirmed-level10.txt; pluginval-0.13.2-confirmed-exit.txt |
| Actual VST3 host | 75 assertions / 0 failures; 600-second finite/audible render | host-0.13.2-final.txt |
| AAX inspection | x64 PE, seven ACF exports and dependencies present; unsigned | aax-0.13.2-inspect.txt; aax-0.13.2-signature.txt |
| Windows file/product versions | 0.13.2 on VST3, standalone and AAX | file-versions-0.13.2-final.txt |
| Installed VST3 and old-version backup | Both bundle files verified | installation-0.13.2-final.txt |

Pluginval uses seed 9009, rates 44.1/48/88.2/96/192 kHz and blocks 1/17/64/257/1024/8192. The first PowerShell launch produced SUCCESS but no native exit code; it is not used as process-exit evidence. The final launch explicitly waited on the process and recorded exit 0. The actual-module host covers state/automation, MIDI timing, external audio/follower, all sixteen exported factory sounds and editor/instance lifecycles. The 600-second render used 52.1202 seconds of callback time (8.6867% of realtime), with peak observed block 0.787 ms against 2.66667 ms at 128/48 kHz. This is one offline workstation measurement, not a DAW CPU guarantee.

Fresh compiled previews are in `artifacts/GUI-0.13.2-final`. The manual's unchanged full-size panel image has SHA-256 `F241C6DF612EA458B039382ED7EC57243DAC83A4CDF7BDDE1F18EE9002037EE6`. Desktop and 390Ã—844 responsive browser layouts, internal navigation, disclosures, checkboxes and the loaded panel were inspected; see `docs/manual/qa/consumer-review-0.13.2.md`. Source-guide dependency checks pass. The consumer package excludes historical images and internal evidence/QA pages. No final PDF pagination is claimed.

The package target is `artifacts/Release/Toyland KB-09-0.13.2-Windows-x64` and its sibling ZIP; the desktop copy is `C:\Users\kevin\OneDrive\Desktop\Toyland KB-09-0.13.2-Windows-x64`. Assembly verifies the exact binary identities, offline navigation, every archive entry and source-kit checksums. Final assembly and desktop-copy records are `package-0.13.2-final.txt` and `desktop-0.13.2-final.txt`; the machine-readable build/test identity is `release-0.13.2.json`. These records, rather than an inferred filename, establish completed package copies and archive hashes.

The DSP is unchanged; no new age-dependent saturation law is asserted. The existing nonlinear engine remains. No fresh Live Set recall, other native DAW test, physical 4K-display test, measured hardware audio match, Mac/AU pass or Pro Tools/AAX pass is claimed. AAX compilation/entry-point inspection does not validate its wrapper in a host. See [format status](FORMAT_ROADMAP.md).

An assembly check found that JUCE 9's Windows `rcfile` rule did not depend on its changed `Info.txt`, leaving Explorer's file version at 0.10.0 despite a current VST3 manifest. The project now adds that dependency; the regenerated resources and relinked VST3/standalone/AAX report 0.13.2. Steinberg, actual-module host and confirmed Pluginval runs were repeated on the corrected module. DSP/processor/editor fixtures use unchanged shared code. The earlier metadata candidate was retained under `artifacts/Release/Rejected-0.13.2-old-file-version` and is excluded from the final desktop package.

## Historical Windows build â€” Toyland KB-09 0.13.1, 2026-10-07

The exact knob image selected by the owner (`exec-8bcd38b0-7cb9-4ce3-99bc-80b29603b296.png`) replaces the rejected native cylinder reconstruction. Built-in edits move the viewing angle almost overhead while preserving the selected object's physical-height intent, silver skirt and alternating rib clusters / broad smooth panels. The registered clean sprite is drawn whole, with a separate moving opaque black top tick. The shared bender/post projection target is now 9Â° off normal. The approved overhead fader bitmap is unchanged. The actual compiled 3840 Ã— 2160 fresh/aged previews were inspected; this does not establish calibrated CAD, measured image-generation geometry or owner acceptance.

- VST3 module SHA-256: `2338DE51AE7B010DDEB5E69C343EA8A8BF28146483BDC61B7140140166413D8B`.
- Standalone SHA-256: `6D3FAC1F46C934234453D2B18176BDE258EB76626896349EFA8C5C11FE73FDDE`.
- Installed bundle: `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; matching module and 0.13.1 manifest verified. Previous 0.13.0 is verified at `artifacts/Backup/Toyland KB-09-0.13.0-before-0.13.1/Toyland KB-09.vst3`.
- Package: `artifacts/Release/Toyland KB-09-0.13.1-Windows-x64` and sibling ZIP, 41,861,411 bytes, all 32 files and every ZIP entry/hash verified. ZIP SHA-256: `1F2E00AD0F6F52BE334E03E84EB656661C57550243D67B122F5448EC1ACF96DB`.

| Current check | Result | Log under artifacts/Validation-KB09 |
|---|---|---|
| Release build | Exit 0 | build-0.13.1.txt; build-0.13.1-exit.txt |
| DSP | 92 passed / 0 failed | dsp-0.13.1-final.txt |
| Processor | 20 passed / 0 failed; zero tracked C++ new allocations | processor-0.13.1-final.txt |
| Preset files | 58 passed / 0 failed; actual final bank | presets-0.13.1-final.txt |
| Actual editor | 302 checks / 0 failures, including moving tick and static-indicator negative controls | gui-0.13.1-final.txt |
| Steinberg validator | 47 passed / 0 failed, exit 0 | steinberg-0.13.1-final.txt |
| Pluginval 1.0.4 level 10, GUI enabled | SUCCESS, confirmed process exit 0 | pluginval-0.13.1-final-level10.txt; pluginval-0.13.1-exit.txt |
| Actual VST3 host | 75 assertions / 0 failures; 600-second finite/audible render | host-0.13.1-final.txt |
| Installed module and backup | PASS | installation-0.13.1-final.txt |
| Package and offline navigation | PASS, including START HERE and guide | package-0.13.1-final.txt; package-manual-0.13.1-final.txt |

Pluginval uses seed 9009, rates 44.1/48/88.2/96/192 kHz and blocks 1/17/64/257/1024/8192. The confirmed run explicitly waits for the process and records its exit code; the preceding launch's log alone is not used as process-exit evidence. The host covers state/automation, external audio/follower, 16 factory exports/round-trips and editor/instance lifecycles. Endurance callback time was 59.537 seconds for 600 rendered seconds (9.92284% of realtime); peak observed block was 1.3557 ms against 2.66667 ms at 128/48 kHz. This is one offline workstation measurement, not a DAW CPU guarantee. The engine is unchanged.

The source guide's version and panel image now match 0.13.1. Complete-package offline navigation passes: 2 HTML documents, 25 IDs, 44 fragment links, 18 local references and 14 external citations. Consumer wording is unchanged from the reviewed feature-first guide. No new browser-layout, physical 4K-display, native Live recall, AU or AAX test is claimed. Current artwork and exact prompts are recorded in `Assets/Photoreal/PROMPTS.md` and `docs/KNOB_RESTORATION.md`.

## Historical release â€” Toyland KB-09 0.13.0, 2026-10-07

The revised Windows VST3 and standalone are built. The tested VST3 is installed at `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; its module hash and 0.13.0 moduleinfo match the build. The previous 0.12 bundle was verified and preserved outside DAW scan folders at `artifacts/Backup/Toyland KB-09-0.12.0-before-0.13.0/Toyland KB-09.vst3`.

- VST3 inner module SHA-256: `732AB009624C0A3B33C324C8F4AD9F1E7CB6ED6FC20E1599C22963AAF58BB065`.
- Standalone SHA-256: `1C894C7798EB4EE66A57BFEA3623B4C412D55B0C9CC75175968A3F501146E8F6`.
- Package: `artifacts/Release/Toyland KB-09-0.13.0-Windows-x64` and sibling ZIP, 41,995,027 bytes.
- ZIP SHA-256: `8FF8EAA0FE7AACAE38674580F937200371FCB06A2CD30B0EE061B674B33C39BD`. All 32 files and every ZIP entry/hash were verified against the tested module and executable. The offline guide, START HERE, factory bank and licenses are included; development evidence pages are excluded from the consumer package.

| Current check | Result | Log under artifacts/Validation-KB09 |
|---|---|---|
| Final Release build | Exit 0 | build-0.13-final-grip.txt |
| DSP | 92 passed, 0 failed; unchanged engine | dsp-0.13.txt |
| Processor | 20 passed, 0 failed; zero tracked C++ new allocations | processor-0.13-final.txt |
| Preset files | 58 passed, 0 failed; final actual-host bank | presets-0.13-final.txt |
| Actual editor | 302 checks, 0 failures | gui-0.13-final.txt |
| Steinberg validator | 47 passed, 0 failed, exit 0 | steinberg-0.13-final.txt |
| Pluginval 1.0.4 strictness 10, GUI enabled | SUCCESS, process exit 0 | pluginval-0.13-final-level10.txt; pluginval-0.13-final-exit.txt |
| Actual VST3 host | 75 assertions, 0 failures; 600-second render | host-0.13-final.txt |
| Installation and previous-version backup | Matching tested module and verified old bundle | installation-0.13-final.txt |
| Package identity and complete ZIP | 32 files verified | package-0.13-final.txt |

Pluginval uses seed 9009, sample rates 44.1/48/88.2/96/192 kHz and blocks 1/17/64/257/1024/8192. The actual-module host covers MIDI timing, state/automation, external input and follower, 16 factory exports/disk round-trips, ten native editor lifecycles and eight additional instances. The 600-second render remained finite and audible: callback time 58.9363 seconds (9.82272% of realtime), maximum observed 128-sample callback 0.9209 ms against 2.66667 ms at 48 kHz. This is one offline workstation measurement, not a DAW CPU guarantee.

The editor checks cover the common 18-degree projection, moving black top ticks with static-indicator negative controls, opaque overhead caps with displaced-cap negative controls, thin exposed blade geometry, S/M/L restoration, complete footers, held-key feedback, selector attachments, fixed/discrete wear events and bender spring return/damage. The actual compiled editor was visually inspected in its 3840 Ã— 2160 export: tall knob barrels, overhead cap tops, white/gray screw oxidation, duller aged keys, fine textured lower rail, ivory POWER cap and the relocated coffee ring. ENV legends place GATE+TRIG above the actuator, GATE to its left and LFO below. The finite-resolution source textures and image-derived dimensions are not calibrated hardware CAD or physical-display equivalence.

The feature-first consumer manual has the requested recreation subtitle, AGE GUI/circuit explanation, external filter and envelope-follower introduction. Its source and packaged dependencies/anchors pass the offline checker. The whole package check covers two HTML documents, 25 IDs, 44 fragment links, 18 local references and 14 external citations (`package-manual-0.13-final.txt`). The 0.13 browser layout review could not run because the browser runtime reported no available browser; no fresh desktop/mobile manual screenshots or PDF pagination are claimed. See `docs/manual/qa/consumer-review-0.13.md`. The earlier Live Auto-Scale confirmation is the owner's report, not a fresh 0.13 Live test. AU and AAX are not built or included.

## Historical release â€” Toyland KB-09 0.12.0, 2026-10-07

The photo-informed Windows VST3 and standalone are built and the VST3 is installed. The previous 0.11 bundle was verified and preserved outside DAW scan folders at `artifacts/Backup/Toyland KB-09-0.11.0-before-0.12.0/Toyland KB-09.vst3`.

- VST3 inner module SHA-256: `E0605E732F9BB6E6D5CC1AB01633927F2DE9EE7ECF7DAEDAE1ED5D7E6D99EE86`.
- Standalone SHA-256: `D9CF9E9E558179C7A25DEFF7B1CC686EA76CFC345EB6FF26226C441856E3CFDE`.
- Installed bundle: `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; its module hash matches the build. `moduleinfo.json` reports 0.12.0.
- Personal test package: `artifacts/Release/Toyland KB-09-0.12.0-Windows-x64` and sibling ZIP. Includes offline working guide, start page, standalone and exported factory bank.
- ZIP SHA-256: `451C38D20FAF7DEBCF83A87EDB9FFAB0CA9C3410E521BCC9C63157B291291434`. All 33 package files and every ZIP entry were verified; packaged VST3/standalone hashes match the tested build. `package-0.12-final.txt` records the result. The offline checker passes for all three packaged HTML documents, including cross-document fragment links and Start Here dependencies.

| Current check | Result | Log under artifacts/Validation-KB09 |
|---|---|---|
| Release build | Exit 0 | build-0.12-final.txt; build-0.12-final-repair.txt for final notice/test correction |
| DSP | 92 passed, 0 failed | dsp-0.12-final.txt |
| Processor | 20 passed, 0 failed; zero tracked C++ new allocations | processor-0.12-final.txt |
| Preset files | 58 passed, 0 failed; includes final actual-host bank | presets-0.12-final.txt |
| Actual editor | 278 checks, 0 failures | gui-0.12-final.txt; artifacts/GUI-0.12-final/GUIRender.log |
| Steinberg validator | 47 passed, 0 failed, exit 0 | steinberg-0.12-final.txt |
| Pluginval 1.0.4 strictness 10, GUI enabled | SUCCESS, process exit 0 | pluginval-0.12-final-level10.txt; pluginval-0.12-final-exit.txt |
| Actual VST3 host | 75 assertions, 0 failures; 600-second render | host-0.12-final.txt |
| Installed identity/backup | Matching new module and verified previous module | installation-0.12-final.txt |

Pluginval uses seed 9009, sample rates 44.1/48/88.2/96/192 kHz, and blocks 1/17/64/257/1024/8192. The actual-module host also covers 35 rate/buffer pairs, MIDI timing, external input and follower, state/automation, 16 preset exports/disk round-trips, ten native editor lifecycles and eight additional instances.

The final endurance render processed 600 seconds with finite, audible output. Callback time was 49.1264 seconds (8.18774% of realtime); maximum observed 128-sample callback was 0.7052 ms against 2.66667 ms at 48 kHz. This is one offline measurement on this workstation, not a DAW CPU guarantee. The engine did not change during this visual update.

The revised editor fixture exercises S/M/L buttons and restore, complete wide/tall footers, opaque caps and metal posts, separate cap-loss events at normalized AGE .64/.88, the single hairline at .56, fixed envelope coffee ring at .60, held white/black-key feedback, selector attachments and configured value-bubble contrast. Bender crops and callbacks check both extremes, exact spring return, and the abrupt cosmetic tip break at .92, including opposing pitch bends before/after damage. Alpha-aware pixel comparisons prevent near-transparent antialiasing edges from creating false wear jumps.

Ten principal images cover five conditions at 3840 Ã— 2160 and 2880 Ã— 1620, plus S and bender crops. `personal_age7_3840x2160.png` shows the actual compiled editor with no held keys. The photograph-based cap correction, black knob top ticks, hairline, gray screw heads, project notice and full footer were visually reviewed. The old generic wear atlas and both shaded crack bitmap attempts are excluded from runtime. Native microdust/grime has independent seeded positions; distinct final key samples are checked. This does not establish uniqueness under every transformation, exact hardware dimensions, material chemistry or sonic equivalence.

The user confirmed that disabling Live's Auto-Scale Plug-in Window fixed sharpness. That human report is recorded separately from our automated checks; it does not identify this new binary or establish a physical 4K/mixed-DPI test. New Live loading/Set recall, exhaustive mouse gestures and every DAW recipe remain owner/host checks. Previous native standalone measurements are historical below, not fresh observations of 0.12.

The HTML manual was reviewed at desktop/mobile sizes by the browser agent. Final image/status replacement passed the static offline-dependency/anchor check. PDF pagination and native print-dialog behaviour are not certified. AU and AAX are not built or included.

The initial Windows macro build failure is retained in build-0.12.txt. The final logs above supersede the intermediate GUI/preset runs described here. The first preset run used a nonexistent bank path; the final run uses the bank exported by the final VST3. The first GUI comparison amplified unpremultiplied near-transparent edge rounding; the repaired check compares visible premultiplied channels. Neither intermediate failure is represented as a pass.

## Historical release â€” Toyland KB-09 0.11.0, 2026-10-07

Final VST3 module SHA-256: `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`.
Final standalone SHA-256: `93F1660185E8C67781D0118DAED461598080580EB30976AB6D5C11D227E6216A`.

The build, staged bundle and installed module hashes match. Staged artifacts are in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`; installed bundle is `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`. Its moduleinfo reports Toyland KB-09 version 0.11.0. The prior 0.10.0 bundle is preserved in `artifacts/Backup/Toyland KB-09-0.10.0-before-0.11.0` with its original hash `FC552E9382A8B622B368CC673AC4C64B3A89CB23EEEA307131A84CC094EED48F`. The earlier SH-09 bundle remains outside scanned folders; there is one current identity.

| Final check | Result | Log under artifacts/Validation-KB09 |
|---|---|---|
| Release build | Exit 0 | build-0.11-final.txt |
| DSP | 92/0; engine unchanged by the later visual edits | dsp-0.11-materials-final.txt |
| Processor | 20 PASS records, zero failures and tracked allocations | processor-0.11-aging-events-final.txt |
| Preset files | 58/0, including imported factory-bank state | presets-0.11-aging-events-final.txt |
| Real-editor GUI | 280/0 | gui-0.11-aging-events-final.txt |
| Steinberg VST3 | 47/0, exit 0 | steinberg-0.11-final.txt |
| Pluginval strictness 10, GUI enabled | SUCCESS, confirmed process exit 0 | pluginval-0.11-final-level10.txt and pluginval-0.11-final-exit.txt |
| Actual compiled VST3 host | 75/0; 600-second render; 16 preset exports/round-trips | host-0.11-final.txt |

Pluginval uses seed 9009, rates 44.1/48/88.2/96/192kHz and blocks 1/17/64/257/1024/8192. GUI lifecycle tests run; they do not establish visual fidelity. CPU-backed artwork and software rendering on the editor's own JUCE peer preserve the shutdown fix.

The final host's 600-second offline render took 51.0971 seconds of callback time (8.51618% of realtime); maximum observed 128-sample callback was 1.006 ms against a 2.66667 ms deadline at 48 kHz. This single offline measurement includes scheduling outliers and is not a DAW CPU guarantee. It also verifies35 rate/buffer combinations, latency128, MIDI timing, state/automation, external-input/follower routing, stress, 10 editor lifecycles and eight additional instances.

GUI exports are in `artifacts/GUI-0.11-final`. The expanded fixture tests six tiny Age-threshold crossings for abrupt stains and fixed subsequent appearance, two separate crack onsets, and distinct healthy/aged samples from all 19 white and 13 black keys. It also covers physical switches, keyboard feedback, configured value-bubble contrast and transformed footer geometry at 16:9, wide and tall sizes. Main outputs are 3840x2160 and 2880x1620; underlying raster resolutions remain finite. A physical 4K monitor and every native hover-popup placement have not been validated.

The final standalone M/L windows were observed at 1051x642 and 1280x720 logical pixels on this workstation, including a complete footer in L and return to the enlarged M. Native Save produced a default .kb09 file in the earlier 0.11 checkpoint; custom-filename typing and native Load round-trip were not completed by UI automation. The 58 filesystem/preset regressions independently establish the implemented state/filename handling. Live-specific observations are separately scoped in LIVE_TEST.md; historical checks below apply to their named earlier binary.

## Historical development records

Status on 2026-09-27: **the clean Release build, 83 DSP assertions, 7 processor assertions, 75 VST3-host assertions, pluginval strictness 10 and 47 independent Steinberg tests passed**. Actual Ableton Live 11.0 loading, external routing, preset interaction and Set recall were observed as detailed below. These passes do not establish hardware equivalence or complete the explicitly unperformed host/UI checks.

## Clean build, binary identity and runtime dependencies

The development build was retained as `C:\Dev\SH-09\build-development`; a fresh `C:\Dev\SH-09\build` was configured and built from source. The fresh build log records the VST3, standalone, DSP-test, processor-test and host targets completing. No compiler warning or error lines were found in that log. Evidence: `C:\Dev\SH-09\artifacts\Validation\fresh-build.txt`.

- Framework: **JUCE 9.0.2**.
- Generator/configuration: **Visual Studio 18 2026**, **Release x64**.
- Compiler: **MSVC 19.51.36260.0**; MSBuild **18.10.1**.
- CMake: **4.3.1**; Windows SDK: **10.0.26100.0**.
- Release bundle: `C:\Dev\SH-09\artifacts\Release\SH-09.vst3`.
- Installed bundle: `C:\Program Files\Common Files\VST3\SH-09.vst3`.

The tested module and installed module have the same SHA-256 recorded in the final-results section below. Installation is not itself a successful Live scan or routing test.

The PE dependency report lists Windows system libraries and the Release Microsoft C/C++ runtime, including `MSVCP140.dll`, `VCRUNTIME140.dll` and `VCRUNTIME140_1.dll`; it does not list Debug CRT imports or project build DLLs. A destination system needs the corresponding Microsoft x64 runtime. This is an import-table inspection, not a claim that every conceivable dynamic file access has been monitored. Evidence: `C:\Dev\SH-09\artifacts\Validation\dependencies.txt`.

## Development instrument tests

Tested bundle: `C:\Dev\SH-09\build\SH09_artefacts\Release\VST3\SH-09.vst3`, version **0.9.0**, Release x64. The later clean-build process may recreate this same path; the development logs alone do not certify its new contents.

| Tool / scope | Observed result | Evidence |
|---|---|---|
| Tracktion pluginval 1.0.4, strictness 10 | SUCCESS | `artifacts/Validation/sh09-pluginval-initial.txt` |
| Steinberg VST 3.8.1 validator | 47 passed, 0 failed; exit 0 | `artifacts/Validation/sh09-steinberg.txt` |
| Project VST3 integration host, JUCE 9.0.2 | 73 assertions passed, 0 failures; exit 0 | `artifacts/Validation/sh09-host.txt` |

The integration host loads the built VST3 through JUCE's VST3 host wrapper. It does not link the synth's internal engine directly. Coverage includes:

- 35 rate/buffer combinations: 44.1, 48, 88.2, 96 and 192 kHz crossed with blocks of 1, 17, 64, 127, 256, 1024 and 8192 samples.
- Mid-block and final-sample MIDI note timing, variable and zero-length buffers within the negotiated VST3 maximum.
- All exposed parameter values saved/restored through actual VST3 state, malformed-state rejection, and three cycles through 16 factory presets with per-preset restoration.
- External stereo left/right input equality, mono/stereo fold-down equivalence, antiphase cancellation, disabled input silence, cutoff-dependent processing and a follower-driven increase in filtered output energy without MIDI.
- External NaN, positive/negative infinity and very large finite input sanitation, with output checked for finiteness and absolute magnitude no greater than 8.
- 500 blocks of parameter extremes/random changes with dense note, pitch-wheel and panic traffic.
- 30 seconds of sustained rendering, ten native editor create/process/resize/destroy cycles and eight additional complete instance lifecycles.

These are precisely bounded assertions. In particular, the finite/magnitude test is not a calibrated hardware headroom measurement; energy-based output checks do not establish timbral equivalence; editor lifecycle tests do not establish visual quality or mouse functionality.

The latest development timing measurement rendered 30 seconds in about 2.777 seconds of callback time, roughly 9.26% of realtime on this machine, with a measured maximum 128-sample callback of 1.879 ms at 48 kHz. This was an offline single-instance test including scheduling outliers, not a guaranteed Ableton CPU budget.

## Fresh DSP, processor and calibration regressions

`artifacts/Validation/DSPTests-fresh.txt` ends **83 passed, 0 failed**. It covers oscillator pitch/ranges and sub-divider modes at five rates; low-note priority and duplicate-note counts; ADSR timing/release and legato; external detector tap and release; filter/VCA processing and self-oscillation; alias rejection; bounded stress and nonlinear-solver convergence; portamento and LFO behavior; deterministic condition behavior; noise level across rates; and non-finite input/control rejection. All five stress-rate runs reported zero solver failures. The measured external impulse peak was at sample 130, including the modeled filter phase delay beyond fixed latency 128.

The coherent high-saw regression measured residual alias energy at **-57.8183 dB**, versus **-6.81064 dB** for that test's naive reference. This is one defined numerical comparison, not an all-pitches alias ceiling, a THD specification or a hardware measurement. A separate 15-second high-resonance core render completed without numerical failure. These core tests exercise the internal DSP directly; the VST3 host tests independently cover its compiled wrapper.

`artifacts/Validation/ProcessorTests-fresh.txt` records **7 passing assertions, 0 failures**, including keyboard-queue overflow recovery, GUI/MIDI duplicate-note display counts, preservation of a still-held MIDI note when the GUI releases its duplicate, and a zero-sample panic flush. The instrumented normal callback path observed **0 C++ `new`/`new[]` allocations**, including 500 blocks of note traffic and GUI-queue events. The test instruments those allocation operators on the calling thread; it does not instrument every allocator, aligned allocation, OS service, external host or VST3 wrapper, and it is not a universal real-time certification.

`artifacts/Validation/Render-fresh.txt` records **35 seconds across seven generated WAV examples**, with **0 solver failures** for each: bass, resonant lead, random modulation, self-oscillation, external follower, condition and an external-routing stimulus. Files are under `artifacts/Audio`. The renderer took 2.64311 seconds including stimulus and filesystem work; that timing is distinct from the actual-module callback benchmark below. Successful rendering does not constitute a listening comparison against hardware.

The calibration-tool self-test passed frequency, RMS, harmonic, transfer-gain and PCM round-trip checks (`artifacts/Validation/calibration-tests.txt`). Generated calibration inputs at 48 kHz include 220 Hz bursts, a 20 Hzâ€“18 kHz logarithmic sweep and seeded noise under `artifacts/Calibration/Stimuli`. The example analysis under `artifacts/Calibration/GeneratedBass` contains JSON metrics and a spectrum CSV for a generated bass excerpt. These demonstrate that the analysis pipeline operates; no imported physical SH-09 recording or measured reference-unit calibration is claimed.

## Bus metadata verified in the real module

Steinberg's Scan Buses test reported:

| Bus | Index | Name | Reported classification |
|---|---:|---|---|
| Audio input | 0 | External Input | Aux, default active |
| Audio output | 0 | Output | Main, default active |
| Event input | 0 | MIDI Input | Main, default active |

Its Bus Activation test also identified input audio bus 0 as `kAux`. This verifies the compiled wrapper's metadata; it does not by itself verify Live's routing controls or saved-project behavior.

The final integration host verified `getLatencySamples() == 128` after preparation at all 35 sample-rate/block-size combinations. Its fresh log prints `latency=128` beside every matrix result. Pluginval's pre-preparation cache value is not a measurement of signal delay. The filter's frequency-dependent group delay also contributes to a measured impulse peak beyond the fixed oversampling latency.

## Fresh Release rerun â€” final results

Final tested module: `C:\Dev\SH-09\build\SH09_artefacts\Release\VST3\SH-09.vst3\Contents\x86_64-win\SH-09.vst3`.

SHA-256: `188CD6EACAD347120DF4A339C6858BEE8D200DEDE3BF10F6D999A447BCE13EE1`, rechecked after tests and preset export.

| Final test | Actual result | Evidence |
|---|---|---|
| Clean Release x64 build | **Succeeded; no warning/error lines found** | `C:\Dev\SH-09\artifacts\Validation\fresh-build.txt` |
| Direct DSP regressions | **83 passed, 0 failed** | `artifacts/Validation/DSPTests-fresh.txt` |
| Direct processor regressions | **7 passed, 0 failed; 0 tracked C++ allocations** | `artifacts/Validation/ProcessorTests-fresh.txt` |
| Tracktion pluginval 1.0.4, strictness 10 | **SUCCESS** | `artifacts/Validation/sh09-pluginval-fresh.txt` |
| Expanded JUCE VST3 integration host | **75 assertions passed, 0 failures; exit 0** | `artifacts/Validation/sh09-host-fresh.txt` |
| Steinberg VST 3.8.1 validator | **47 passed, 0 failed; exit 0** | `artifacts/Validation/sh09-steinberg-fresh.txt` |
| Fixed reported latency | **128 samples** at all 35 rate/block combinations | Per-combination lines in final host log |
| Sustained actual-module rendering | **600 seconds**, finite/nonzero output | Final host log |
| Factory preset disk export/reload | **16 of 16** `.sh09state` files exported and restored with program/parameter comparisons | `artifacts/Presets`, final host log |

The final pluginval run used seed **9009** (`0x2331`) and all 30 combinations of 44.1, 48, 88.2, 96 and 192 kHz with blocks of 1, 17, 64, 257, 1024 and 8192 samples. Its log records audio processing, non-releasing processing, state restoration, automation, editor creation/processing/automation, background-thread state, parameter thread safety, bus changes and parameter fuzzing completing with **SUCCESS**. Its optional embedded Steinberg invocation was explicitly skipped because no path was supplied; the separate Steinberg result above supplies that independent validation. GUI lifecycle tests ran, but visual appearance and user mouse actions still require actual UI review. The reported infinite tail is intentional for Hold/drone/self-oscillation operation; the integration benchmark uses a bounded rendering duration.

The final host repeats all development integration checks listed above. During its 600-second offline render, callback processing took **53.1918 seconds**, or **8.8653% of realtime**. Maximum measured 128-sample callback was **0.5205 ms**, compared with a **2.6667 ms** audio deadline at 48 kHz. This is a single observed run, not a CPU guarantee or an Ableton measurement.

The first run against the clean plug-in used a stale host executable because copying the updated host source preserved an earlier timestamp. Its lack of latency labels/preset output exposed that harness issue. Only `Tests/Host.cpp` was touched and rebuilt; the plug-in module was not changed. The final log above is the subsequent run that explicitly executed the new latency and disk-export assertions.

Actual GUI/Live observations and remaining acceptance limits are recorded in LIVE_TEST.md. Any later plug-in-code change requires relevant validation again.

Reproduction command after rebuilding the latest `Tests/Host.cpp` (choose the desired absolute export directory):

```powershell
& '.\build\SH09Host_artefacts\Release\SH09Host.exe' '.\build\SH09_artefacts\Release\VST3\SH-09.vst3' 600 --export-presets 'C:\Dev\SH-09\artifacts\Presets'
```

Optional `.sh09state` exports are serialized state for the JUCE-host restoration API and integration regression. They are not Steinberg `.vstpreset` files, and the instrument currently has no file-import UI for them. The 16-program bank embedded in the plug-in is the normal musical workflow.

## Ableton and fidelity acceptance

**Observed in Ableton Live 11.0:** standard-folder scan, MIDI-track loading, two instances, native panel/preset interaction, the auxiliary Sidechain controls, external audio from a monitored audio track, follower-meter activity before the EXT mixer, processed output in Hold without MIDI, mute-to-silence/unmute recovery, and transport start/stop. Closing Live and reopening the saved Set restored both instances, the External Follower patch and audio routing with processed output. See LIVE_TEST.md for exact setup, the saved Set path, a save-dialog path issue and unperformed cases. These UI/meter observations are distinct from the automated tests and do not establish measured sonic fidelity.

No physical SH-09 has been measured for this project. Source-based DSP tests and validator passes cannot establish circuit accuracy, calibrated control-law accuracy or measured hardware equivalence. See the research and known-limitations documents for the evidence classification and remaining fidelity work.

Tool provenance and commands are in `tools/VALIDATORS.md`. The prerequisite disposable-plug-in checkpoint is in `VST3_SMOKE_TEST.md`.
