# KB-09 format and download-package roadmap

Current Windows source/package target: **0.13.1**, updated 2026-10-07. Windows VST3 and standalone remain the personal test-package formats. Exact new-build validation, packaging and installation status belongs in [VALIDATION.md](VALIDATION.md); the earlier 0.13 package is historical evidence. The Mac/AAX sections remain a development plan, not a claim that future formats or installers exist.

## Recommended formats and order

Keep **VST3 + standalone** as the current Windows deliverables. Version 0.13.1 source replaces the native-cylinder knob with the selected complete photographic sprite, grouped ribs and a native black tick; the shared native camera target is 9°. AGE materials and the approved overhead fader are retained. Owner DAW/Set review remains separate from software checks. Build and test **AU for macOS** and **AAX Native for Pro Tools** from the same engine when their environments are available; distribution of those formats should follow actual host tests.

| Deliverable | Audience | Present status | Needed before release |
|---|---|---|---|
| Windows x64 VST3 | Compatible VST3 DAWs, including the owner's Live setup | 0.13.1 source revision; exact module/package status in VALIDATION.md | Matching rebuilt-binary evidence and owner's new-build DAW/Set review before broader distribution |
| Windows standalone | Playing without a DAW | 0.13.1 package target; exact executable status in VALIDATION.md | Matching build evidence and owner's audio/MIDI setup review; platform installer later |
| macOS VST3 + AU + standalone | Mac VST3 hosts; Logic Pro/GarageBand through AU | Not built or tested | Mac/Xcode environment, supported-OS/architecture decision, signing/notarization and native host tests |
| Windows/macOS AAX Native | Pro Tools | Not built or tested | AAX SDK access/license, confirmed signing/deployment route, Pro Tools environment and format-specific tests |

AU means **Audio Unit**, Apple's plug-in format. Apple describes Audio Units as plug-ins used in audio applications, and its current support page confirms Logic Pro's AU v2/v3 support on Apple silicon. AU is not useful for this Windows-only test machine. A conventional JUCE AU desktop target is a sensible starting point; AUv3/mobile work is a separate decision, not implied by “AU.” [Apple development fundamentals](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html), [Apple compatibility guidance](https://support.apple.com/en-gb/102082).

AAX is Avid's plug-in format for Pro Tools and other Avid hosts. AAX **Native** is the appropriate initial scope; this does not promise AAX DSP/HDX support. Avid supplies an evaluation SDK through a click-through license and says an Avid account plus iLok account are needed for Pro Tools AAX testing. Its public page explicitly associates an iLok USB signing key with **commercial** AAX development. This project is a free, unofficial personal project: verify the applicable noncommercial signing/testing/distribution route with Avid/PACE before promising a loadable public AAX package. Do not infer that a free build avoids signing requirements or that AAX automatically requires end users to buy a copy-protection license. [Avid AAX SDK](https://developer.avid.com/aax/).

Apple recommends Developer ID signing for distributed apps, plug-ins and installers. A consumer Mac package should be signed and notarized, with its launch/install experience tested on a Mac. Certificate access and notarization are release work, not something a Windows VST3 validation establishes. Apple documents current Xcode/notarytool workflows and their requirements. [Apple Developer ID](https://developer.apple.com/developer-id/), [Apple notarization documentation](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution).

Older VST formats are not needed for the present owner or proposed audience. Add an older format only for a specific user's unsupported host and after checking its licensing, toolchain and maintenance burden. No bridge/wrapper should be presented as a substitute for a tested native AU/AAX release.

## A friendly package

The 0.13.1 personal Windows test package uses the layout below at `artifacts/Release/Toyland KB-09-0.13.1-Windows-x64`, with a sibling ZIP. Versioned source conventions are `artifacts/Presets-0.13.1-final` for the factory-bank export, `artifacts/GUI-0.13.1-final` for compiled previews and `docs/manual/assets/panel-0.13.1.png` for the guide image. These paths do not themselves establish completed exports or package validation. This is an owner-review package; platform installers and broader distribution remain later steps. Earlier packages and their test records are historical.

```text
Toyland KB-09-<version>-Windows-x64/
  START HERE.html
  User Guide/
    index.html
    styles.css
    assets/
  VST3/
    Toyland KB-09.vst3/
      Contents/...
  Standalone/
    Toyland KB-09.exe
  Factory Presets/
  Release Notes.txt
  SHA256SUMS.txt
  Licenses/
```

“START HERE” offers **Use with my DAW** and **Play without a DAW** paths. The package explains copying the complete VST3 bundle, its standard Windows destination, and loading it as an instrument. AU/AAX are identified as future downloads rather than included formats. The consumer package deliberately excludes `User Guide/reference/evidence.html`; detailed test and research records remain in the development documentation.

The feature-first user guide works offline, uses the supplied Toyland/KEVINBERG marks, and covers playing, AGE, presets, installation and DAW settings. Keep its clickable contents, host sections and matching `panel-0.13.1.png` with the new package. HTML remains the primary format; print CSS supports printing or saving a PDF, but a separately certified PDF with reviewed pagination and internal links is not included.

A KB-09 icon can decorate the extracted folder through a Windows `.ico` plus `desktop.ini`, or an installer can provide branded shortcuts. A ZIP cannot promise that every file manager will show a custom folder icon. Make this optional polish after installation and documentation are sound; do not change an existing user folder or system icon settings automatically. Keep the download ZIP name and START HERE file clear even when the icon is not shown.

## Release gates

1. Record matching 0.13.1 compiled GUI evidence for fresh, reference AGE 7 and maximum AGE. Its nine-PNG/native renderer includes the selected complete photographic knob sprite, grouped ribs, moving top ticks and shared 9° native projection. The knob camera edit must preserve physical barrel height. Pale screw deposits, matte key/rail materials, native ivory POWER cap, moved coffee ring and corrected envelope labels remain. Software checks do not replace owner visual acceptance; historical 0.13 18° native-cylinder renders do not validate this revision.
2. Retain the matching binary, preset/state, long-running audio, external-input, Pluginval and format-validator records in [VALIDATION.md](VALIDATION.md). Keep the owner's Live Auto-Scale confirmation distinct from new-build host/Set recall.
3. Have the owner use the updated manual for personal DAW tests. Correct any instruction that does not match the actual host/build.
4. Build native Mac/AAX targets when their environments are available. Test parameters/state, automation, sidechain routing, editor scaling, keyboard input and preset portability in each actual host. Windows validation does not certify these formats.
5. Preserve the assembled versioned download with matching manual, factory presets, notices and checksums. Check package-file/binary identity, ZIP entries and offline links for the matching revision and retain those records. Earlier package checks are historical. Clean-machine installation and the signing/installer route remain separate work before broader distribution.

## Current manual and package status

The [offline user guide](manual/index.html) documents the instrument's features and controls, the owner's successful Live **Auto-Scale Plug-in Window** fix, vendor-guided recipes for other major DAWs, and the AGE progression. The 0.13.1 source/package convention uses `panel-0.13.1.png` alongside S/M/L sizing guidance. The existing feature-first manual is not a pending first draft; its matching image and offline links still require release-specific checks. No new native DAW test is claimed here.

The historical 0.13 package's offline check covered **two HTML documents** (START HERE and the guide), 25 IDs, 44 fragment links, 18 local references and 14 external citations. Those counts do not establish a new 0.13.1 pass. External vendor links need internet; the guide's local content works offline. Exact package checks, binary identities and release scope are centralized in [VALIDATION.md](VALIDATION.md).
