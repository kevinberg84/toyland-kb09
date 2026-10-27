# KB-09 formats and personal package

Current 1.10.0 distribution: Windows x64 VST3/standalone, Mac universal AU/VST3/standalone, and Linux x86-64 standalone only. Mac native CI, AU validation and hosted AU/VST3 checks passed; signing is ad-hoc, without notarization. Linux operation was reported on Arch/KDE with XWayland and PipeWire. AAX is not part of the friend-facing release. See [current validation](VALIDATION_1.10.0.md), [build instructions](BUILD.md) and [release destinations](RELEASE-WORKFLOW.md).

## Historical 0.13.2 plan

Updated 7 October 2026. The owner requested a finished Windows update, AU/AAX where possible, an updated manual and a desktop package. The shared CMake project now defines the native format paths below. Exact build, validator, package and installation results are recorded in [VALIDATION.md](VALIDATION.md); a configured target is not a host-tested release.

## Historical 0.13.2 formats and boundaries

| Deliverable | CMake target | This workstation | Remaining platform work |
|---|---|---|---|
| Windows x64 VST3 | `SH09_VST3` | Supported native build; primary owner format | Record matching release checks, then owner DAW/Set review |
| Windows standalone | `SH09_Standalone` | Supported native build; plays without a DAW | Record matching build checks; owner audio/MIDI device review |
| Windows x64 AAX Native | `SH09_AAX` with `KB09_BUILD_AAX=ON` | Bundled AAX SDK 2.9 is present, so a real unsigned development binary can be compiled | AAX digital signing, Pro Tools developer-host validation and normal Pro Tools testing |
| macOS VST3 + AU + standalone | `SH09_VST3`, `SH09_AU`, `SH09_Standalone` | Source/build recipe only; no Mac/Xcode build host available | Build on Mac, validate AU with `auval`, test actual hosts, then sign/notarize distribution |
| macOS AAX Native | `SH09_AAX` on Mac with `KB09_BUILD_AAX=ON` | Source configuration only | Mac build, AAX signing and Pro Tools tests |

No AU binary can be honestly included from a Windows-only build. An AU source kit must be labeled as source, with the native Mac build requirement visible. An unsigned AAX is an actual compiled AAX development artifact, but it is not ready for ordinary Pro Tools installation. Keep it under a clearly marked Development directory and do not include it in a ready-to-use installation action.

The current GUI revision includes the narrower photographic knob tops/rims, long native top ticks, gradual metal micro-pitting/dulling, immediate mouse-key amber feedback and the photo-informed grey POWER area with its tall OFF position. The approved fader, keyboard aging, textured front rail, bender, moved coffee ring and envelope labels remain. These changes share the same synth engine; no new age-dependent audio distortion law is asserted by the format work.

## AU on a Mac

**AU means Audio Unit**, Apple's plug-in format. It serves Mac hosts such as Logic Pro and GarageBand. The intended initial target is a conventional desktop AU (`.component`), not AUv3/mobile. Apple documents Audio Units as audio application plug-ins and identifies Logic Pro's AU support. [Apple Audio Unit fundamentals](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html), [Apple Logic Pro compatibility guidance](https://support.apple.com/en-gb/102082).

The CMake option `KB09_BUILD_AU` defaults to ON on Apple platforms and rejects explicit enablement outside Darwin. [BUILD.md](BUILD.md#native-macos--au-build) gives the Xcode/universal-build recipe and AU identity `aumu` / `Sh09` / `PrIv`. This prepares a build path; it does not demonstrate successful Mac compilation, sidechain/external audio routing, editor scaling or Logic/GarageBand compatibility. Those require a real Mac test session.

For consumer distribution, Apple recommends Developer ID signing of apps, plug-ins and installers. Its notarization service has separate signature, timestamp and hardened-runtime requirements. No Apple signing identity, notarization submission or account setup is provided by a Windows build. [Apple Developer ID](https://developer.apple.com/developer-id/), [Apple notarization documentation](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution).

## AAX Native for Pro Tools

**AAX is Avid's plug-in format** for Pro Tools and other Avid hosts. This project targets **AAX Native**, not AAX DSP/HDX. The installed JUCE 9.0.2 source already contains AAX SDK 2.9, including its Interfaces, library source and utility icon; an SDK download or account registration is not necessary to compile with that existing copy. `KB09_BUILD_AAX` is opt-in, with `KB09_AAX_SDK_ROOT` accepting a different compatible SDK. See [BUILD.md](BUILD.md#aax-native-development-build).

Avid's current SDK page describes Avid/iLok accounts for Pro Tools AAX testing and the commercial signing route. The Avid-hosted Pro Tools SDK guide states that ordinary hosts require valid plug-in digital signatures and describes a developer-build exception. No signing tool or Pro Tools developer host is configured here. A free personal project does not establish that an unsigned binary will load in a normal Pro Tools release. [Avid AAX SDK](https://developer.avid.com/aax/), [Avid Pro Tools SDK guide](https://learn-cdn.avid.com/AAX_SDK_2p1p1/Documentation/Doxygen/output/html/a00274.html).

The existing iLok License Manager is not evidence of developer signing credentials. The bundled SDK has no standalone AAX validator executable. AAX validation must include real host creation, MIDI, state/automation, presets, external audio/follower routing, UI resizing, save/reopen and subsequent signed normal-host loading. Windows VST3 tests do not certify this wrapper.

## Personal desktop package

Use one clearly named folder containing the usable Windows files, the offline manual and the truthful development/source status. A possible layout is:

```text
Toyland KB-09-0.13.2-Windows-x64/
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
  Development/
    AAX-Windows-UNSIGNED/
      READ ME.txt
      Toyland KB-09.aaxplugin/
    macOS-AU-Build-Source.zip
    Format Status.txt
  Release Notes.txt
  SHA256SUMS.txt
  Licenses/
```

Only include a development bundle/source archive if it was actually produced and checked; the tree above is a layout convention, not an artifact manifest. The release validation record and package checksums establish what is present. A source archive may preserve the project plus the exact JUCE dependency source for a future Mac build, while excluding generated build outputs and credentials. Preserve JUCE `LICENSE.md`, the bundled AAX `SDK/LICENSE.txt`, the AU SDK license and vendored notices. This package work does not assign a new license to Kevin's own project or establish broader distribution compliance.

START HERE should make **Use with my DAW** and **Play without a DAW** straightforward. The ordinary installation path copies the entire VST3 bundle into its standard Windows directory. Development AAX and Mac source are separate choices with their present limitations visible. No empty `.component` directory, renamed VST3 or wrapper should be presented as a finished AU/AAX release.

The feature-first HTML guide works offline and covers the SH-09 recreation, external filter, envelope follower, AGE, presets, installation and DAW settings. Keep its clickable contents, host sections and release-matched `panel-0.13.2.png`. Print CSS supports browser printing/saving to PDF; a separately reviewed PDF is not implied. External vendor links still need internet. Development evidence pages need not appear in the consumer manual.

A KB-09 `.ico` plus `desktop.ini` can decorate the extracted desktop folder. A ZIP alone cannot guarantee a custom folder icon in every file manager; the folder name and START HERE remain usable without it. Do not change global Explorer icon settings.

## Completion checks

1. Build matching VST3/standalone and the requested unsigned AAX development target. Record exact source/binary identities and the AAX signature/host-test limitation.
2. Run the existing DSP, processor, preset, compiled-GUI, long-running host, Steinberg and GUI-enabled Pluginval checks against the actual rebuilt VST3. Keep native DAW/physical-display observations separate.
3. Match the manual image and instructions to the release; verify offline local links and fragments. The amber marker should appear immediately while a mouse-played effective note is held, and disappear on release.
4. Preserve the existing installation outside scanned folders before updating the installed VST3. Confirm the installed file hash matches the validated bundle.
5. Assemble a fresh package with presets, manual, actual license notices, development/source items and checksums. Verify ZIP entries and the desktop copy. Keep earlier packages and logs intact.
6. Complete native Mac AU and signed Pro Tools release work only when their environments and credentials are available. No Windows validator result replaces those checks.