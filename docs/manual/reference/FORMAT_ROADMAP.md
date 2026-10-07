# KB-09 format and download-package roadmap

Working recommendation, researched 2026-10-07. This is a development plan, not a claim that future formats or installers exist.

## Recommended formats and order

Keep **VST3 + standalone** as the current Windows deliverables. Complete the photo-based panel, AGE treatment and Windows/Live checks first. Then build and test **AU for macOS** and **AAX Native for Pro Tools** from the same engine. Platform preparation can proceed in parallel with Windows fixes; distribution should wait for actual host tests.

| Deliverable | Audience | Present status | Needed before release |
|---|---|---|---|
| Windows x64 VST3 | Compatible VST3 DAWs, including the owner's Live setup | Photo-informed 0.12 built, validated and installed; personal test ZIP assembled | Owner's new-build DAW/Set review before public release |
| Windows standalone | Playing without a DAW | 0.12 executable included in the personal package | Owner's audio/MIDI setup review; platform installer later |
| macOS VST3 + AU + standalone | Mac VST3 hosts; Logic Pro/GarageBand through AU | Not built or tested | Mac/Xcode environment, supported-OS/architecture decision, signing/notarization and native host tests |
| Windows/macOS AAX Native | Pro Tools | Not built or tested | AAX SDK access/license, confirmed signing/deployment route, Pro Tools environment and format-specific tests |

AU means **Audio Unit**, Apple's plug-in format. Apple describes Audio Units as plug-ins used in audio applications, and its current support page confirms Logic Pro's AU v2/v3 support on Apple silicon. AU is not useful for this Windows-only test machine. A conventional JUCE AU desktop target is a sensible starting point; AUv3/mobile work is a separate decision, not implied by “AU.” [Apple development fundamentals](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html), [Apple compatibility guidance](https://support.apple.com/en-gb/102082).

AAX is Avid's plug-in format for Pro Tools and other Avid hosts. AAX **Native** is the appropriate initial scope; this does not promise AAX DSP/HDX support. Avid supplies an evaluation SDK through a click-through license and says an Avid account plus iLok account are needed for Pro Tools AAX testing. Its public page explicitly associates an iLok USB signing key with **commercial** AAX development. This project is a free, unofficial personal project: verify the applicable noncommercial signing/testing/distribution route with Avid/PACE before promising a loadable public AAX package. Do not infer that a free build avoids signing requirements or that AAX automatically requires end users to buy a copy-protection license. [Avid AAX SDK](https://developer.avid.com/aax/).

Apple recommends Developer ID signing for distributed apps, plug-ins and installers. A consumer Mac package should be signed and notarized, with its launch/install experience tested on a Mac. Certificate access and notarization are release work, not something a Windows VST3 validation establishes. Apple documents current Xcode/notarytool workflows and their requirements. [Apple Developer ID](https://developer.apple.com/developer-id/), [Apple notarization documentation](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution).

Older VST formats are not needed for the present owner or proposed audience. Add an older format only for a specific user's unsupported host and after checking its licensing, toolchain and maintenance burden. No bridge/wrapper should be presented as a substitute for a tested native AU/AAX release.

## A friendly package

The current personal Windows test package uses the layout below at `artifacts/Release/Toyland KB-09-0.12.0-Windows-x64`, with a sibling ZIP. This is an owner-review package; platform installers and public release work remain later steps.

```text
Toyland KB-09-<version>-Windows-x64/
  START HERE.html
  User Guide/
    index.html
    styles.css
    assets/
    reference/evidence.html
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

“START HERE” should offer two obvious paths: **Use with my DAW** and **Play without a DAW**. Lead with those actions; reserve troubleshooting and technical evidence for links. Explain that the entire VST3 bundle is copied, identify the standard Windows destination, and show rescanning/loading as an instrument. Include AU/AAX instructions only in packages containing tested builds.

The user guide should work offline, have a clickable contents list, separate host sections, and use the supplied Toyland/KEVINBERG marks without redrawing them. Its prominent AGE subtitle and examples should invite exploration while explaining the sonic and visual effects separately. HTML is the primary working format; print CSS supports a future PDF. A release PDF should be generated, visually reviewed and have contents/internal links checked before delivery.

A KB-09 icon can decorate the extracted folder through a Windows `.ico` plus `desktop.ini`, or an installer can provide branded shortcuts. A ZIP cannot promise that every file manager will show a custom folder icon. Make this optional polish after installation and documentation are sound; do not change an existing user folder or system icon settings automatically. Keep the download ZIP name and START HERE file clear even when the icon is not shown.

## Release gates

1. Finish and inspect the compiled photo-based GUI at fresh, reference AGE 7 and maximum AGE; check all sizes and note/control interaction.
2. Validate the exact Windows binary, presets, state recall, long-running audio and external input. Run Pluginval and the format validator; keep the owner's Live confirmation distinct from automated evidence.
3. Have the owner use the working manual for personal tests. Correct any instruction that does not match the actual DAW/build.
4. Build native Mac/AAX targets when their environments are available. Test parameters/state, automation, sidechain routing, editor scaling, keyboard input and preset portability in each actual host. Windows validation does not certify these formats.
5. Assemble the versioned download with the matching manual, factory presets, license/notice files and checksums. Verify extraction and installation from a clean location. Decide the signing/installer route before broad distribution.

## Manual draft status

The initial offline draft is [docs/manual/index.html](manual/index.html). It documents current Windows use, the owner's successful Live **Auto-Scale Plug-in Window** fix, vendor-guided recipes for other major DAWs, an AGE walkthrough and a personal test checklist. New appearance changes and S sizing remain visibly marked as pending until the parent build supplies final thresholds, screenshots and validation evidence.
