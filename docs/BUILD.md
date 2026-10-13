# Toyland KB-09 1.5.2 — source/build instructions

JUCE **9.0.2** is an external dependency; this project does not modify it. The development toolchain is MSVC 19.51.36260.0, Visual Studio 18 2026, the Windows SDK and C++20. CMake's generator selects the compiler without changing PowerShell execution policy.

## Configure and build

From the project directory:

```powershell
cmake -S . -B build-kb09 -G "Visual Studio 18 2026" -A x64 -DJUCE_DIR=C:/Dev/JUCE
cmake --build build-kb09 --config Release --parallel 8
```

The workstation's development checkout is `C:\Dev\SH-09`, with output under `build-kb09`. Build-directory names are arbitrary; use the same name for configure, build and checks. Alternatively, the supplied script uses the project-local `build` directory:

```powershell
$env:JUCE_DIR = 'C:\Dev\JUCE'
& '.\scripts\build.cmd'
```

The script searches PATH and the known Visual Studio Community CMake installation. Other installations can invoke CMake explicitly. `JUCE_DIR` defaults to `../JUCE`. Runtime needs neither the source checkout nor JUCE source files.

The project adds the generated `Info.txt` dependency to JUCE 9's Windows resource rule, so incremental version changes regenerate the Explorer file/product version. Check these versions as well as the VST3 module manifest when packaging; a current manifest alone can hide a stale resource object.

## Targets and source contents

The internal target remains `SH09`; the public product is **Toyland KB-09**. CMake sets version 1.5.2. Windows builds **VST3** and **Standalone** by default. Native macOS builds also enable **AU** by default; **AAX Native** is an explicit option on Windows/macOS. Target configuration does not establish that a particular format was built, signed or host-tested. Consult [VALIDATION.md](VALIDATION.md) for the actual release artifacts. `Assets/Branding.h` defines the visible identity; compatibility keys remain unchanged as described in [REBRANDING.md](REBRANDING.md).

| Target | Release output relative to build directory |
|---|---|
| `SH09_VST3` | `SH09_artefacts/Release/VST3/Toyland KB-09.vst3` |
| `SH09_Standalone` | `SH09_artefacts/Release/Standalone/Toyland KB-09.exe` |
| `SH09_AAX` (opt-in, Windows) | `SH09_artefacts/Release/AAX/Toyland KB-09.aaxplugin/Contents/x64/Toyland KB-09.aaxplugin` |
| `SH09_AU` (macOS) | `SH09_artefacts/Release/AU/Toyland KB-09.component` |
| `SH09Host` | `SH09Host_artefacts/Release/SH09Host.exe` |
| `SH09ProcessorTests` | `SH09ProcessorTests_artefacts/Release/SH09ProcessorTests.exe` |
| `SH09PresetFilesTests` | `SH09PresetFilesTests_artefacts/Release/SH09PresetFilesTests.exe` |
| `SH09GUIRender` | `SH09GUIRender_artefacts/Release/SH09GUIRender.exe` |
| `SH09DSPTests` | `Release/SH09DSPTests.exe` |
| `SH09Render` | `Release/SH09Render.exe` |

The source package must retain `CMakeLists.txt`, `Source`, `Tests`, `VSTLab`, `Assets`, scripts and documentation: the top-level build includes the test/lab directories. `KB09Artwork` compiles **ten PNGs** from `Assets/Photoreal`:

- `chassis-black.png`
- `chassis-personal-aged.png`
- `chassis-stains.png` — only the retained envelope coffee-ring region is rendered.
- `knob-personal-overhead-v5.png`
- `fader-smooth-overhead-v1.png`
- `cheeks-personal-independent-v1.png`
- `front-rail-grain.png`
- `keys.png`
- `marks.png`
- `marks-aged.png`

Retain every listed input. The new knob is a complete 1254 × 1254 photographic sprite derived from the owner's selected object, with its grouped ribs and silver skirt. The camera-edit constraint preserves physical barrel height; it does not prove measured CAD. Native code supplies the moving black top tick and a shared 11° target for tick, bender and blade projection. The 0.13 native-cylinder/18° material mapping is historical. The smooth photographic fader body uses a native silver insert; the independent cheek atlas supplies distinct left/right wear. The old wear atlas is not compiled or used. Native code also supplies independently seeded key dust/grime, the single thin crack, knob rim/tarnish details, labels/symbols, switches, exposed stepped fader posts and the bender. Kevin's multi-angle photographs guide the geometry and wear; his replacement volume knob, sticker and taped waveform marks are excluded. `CMakeLists.txt` and `Assets/Photoreal/README.md` distinguish compiled layers from provenance-only files, including the non-runtime ticked knob preview. No runtime Assets directory is required. Asset provenance/prompts are retained in `Assets/Photoreal`.

The offline manual is `docs/manual/index.html`. The 0.13.2 package uses `styles.css`, `assets/marks.png` and `assets/panel-0.13.2.png` with it. The consumer guide excludes the separate technical `reference/evidence.html`, development-only `qa` folder and historical images. Check the version-matched guide's local links and illustrations before packaging. External vendor links still require internet access.

## AAX Native development build

The installed JUCE 9.0.2 tree contains AAX SDK 2.9 (revision 20209000) at `C:/Dev/JUCE/modules/juce_audio_plugin_client/AAX/SDK`. No additional SDK download is needed on this workstation. `KB09_BUILD_AAX` defaults to `OFF`; enabling it validates the SDK's `Interfaces/ACF`, `Libs/AAXLibrary/source` and `Utilities/PlugIn.ico`, then calls JUCE's `juce_set_aax_sdk_path` before adding plug-in targets.

```powershell
cmake -S . -B build-kb09 -G "Visual Studio 18 2026" -A x64 -DJUCE_DIR=C:/Dev/JUCE -DKB09_BUILD_AAX=ON
cmake --build build-kb09 --config Release --target SH09_AAX --parallel 8
```

For an external compatible SDK, add `-DKB09_AAX_SDK_ROOT=C:/path/to/AAX_SDK`. The AAX identifier defaults to the existing bundle ID `local.privateinstruments.sh09`; manufacturer `PrIv`, product code `Sh09`, MIDI input and software-instrument category remain defined by the shared target. This creates an **AAX Native binary**, not AAX DSP/HDX support.

Compilation does not provide AAX digital signing. Avid's [Pro Tools SDK guide](https://learn-cdn.avid.com/AAX_SDK_2p1p1/Documentation/Doxygen/output/html/a00274.html) requires a valid digital signature for ordinary Pro Tools hosts and describes the developer-build exception. Its [current SDK page](https://developer.avid.com/aax/) describes Avid/iLok accounts for testing and the commercial signing route. No Pro Tools developer host or AAX signing tool is configured on this workstation. An unsigned bundle must be labeled **development / unsigned / not tested in Pro Tools** and kept separate from the ready-to-use VST3. Do not install it into a normal user's AAX scan folder or imply it will load there. The bundled SDK contains no standalone AAX validation executable; a real Pro Tools developer-host test, signing and normal-host test remain outstanding.

## Native Linux standalone build

The release uses JUCE 9.0.2 and Ubuntu 22.04 to produce a generic x86-64 ELF executable, with no `-march=native` requirement. A current Arch x86-64 system is a target for user playback evaluation. Install the ALSA/JACK, curl, FreeType/fontconfig and X11 development packages appropriate to the build system, then run:

```sh
cmake -S . -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR=/path/to/JUCE -DKB09_STANDALONE_ONLY=ON -DKB09_BUILD_AU=OFF -DKB09_BUILD_AAX=OFF
cmake --build build-linux --target SH09_Standalone --parallel 2
```

The output is `build-linux/SH09_artefacts/Release/Standalone/Toyland KB-09`. The standalone package adds launch and optional desktop-shortcut scripts; it contains no Linux plug-in format. The plugin target requests JUCE's `NEEDS_CURL` dependency so HTTPS update checking links correctly. Native CI also checks tests under Xvfb and `ldd` dependencies; this cannot establish physical audio-device playback. See `.github/workflows/build-linux.yml` for the exact package/test recipe and the manual for runtime requirements.

## Native macOS / AU build

AU means Audio Unit. The conventional desktop AU target is macOS-only; this is not an AUv3/mobile build. `KB09_BUILD_AU` defaults to `ON` on Apple platforms and `OFF` elsewhere. Explicitly enabling it on Windows fails with a clear native-macOS requirement. Native CI and Apple AU validation results for 1.5.2 must be recorded after the release build completes; see its validation record. These commands remain a build recipe; see the release validation record for the exact tested artifacts. Intel execution and individual DAW behavior need their own checks.

On a Mac with Xcode and JUCE 9.0.2, run from the source directory:

```sh
cmake -S . -B build-mac -G Xcode -DJUCE_DIR=/path/to/JUCE -DKB09_BUILD_AU=ON '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build-mac --config Release --target SH09_AU SH09_VST3 SH09_Standalone --parallel 8
```

The requested universal architectures must both be supported by the selected Xcode/SDK and dependencies. Set the minimum macOS version deliberately before a Mac release. The AU identity is `aumu` / `Sh09` / `PrIv` (`kAudioUnitType_MusicDevice`). After copying the entire built `.component` bundle to `~/Library/Audio/Plug-Ins/Components/`, a Mac developer can run `auval -v aumu Sh09 PrIv` and then test it in Logic Pro/GarageBand. Verify MIDI, state, automation, external audio/envelope-follower routing, preset files and editor scaling in the actual AU host. Passing Windows tests does not establish AU wrapper behavior.

Developer ID signing and notarization are separate distribution work. Apple documents [plug-in signing](https://developer.apple.com/developer-id/) and [notarization requirements](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution). No Apple certificate, account operation, notarization submission or Mac installation is performed by this project build. Optional macOS AAX uses the same `-DKB09_BUILD_AAX=ON` option and the separate AAX signing/host requirements above.

## Checks and compiled previews

Run registered DSP, processor and preset-file tests:

```powershell
ctest --test-dir build-kb09 -C Release --output-on-failure
```

Run the actual-module host and compiled-editor fixture separately:

```powershell
& '.\build-kb09\SH09Host_artefacts\Release\SH09Host.exe' '.\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3' 600
& '.\build-kb09\SH09GUIRender_artefacts\Release\SH09GUIRender.exe' '.\artifacts\GUI-0.13.2-final'
```

The GUI fixture exports patch/material conditions, a button-selected S viewport, bender positions and component metadata. It exercises S/M/L sizing and restoration, footer bounds at wide/tall sizes, held white/black-key feedback, configured value-bubble contrast, original parameter identities, physical switches, the single crack, the two cap-loss events, the broken bender and distinct healthy/aged key samples. Its 3840 × 2160 exports are compiled renders, not physical 4K-display observations; photographic sources retain their finite resolution. Native hover-popup placement needs separate observation. See [UI.md](UI.md).

Run GUI-enabled pluginval strictness 10 and Steinberg's validator against the **same rebuilt bundle**, recording process exit codes. [Validator instructions](../tools/VALIDATORS.md) use the current product path. The audio renderer and Python calibration tools are separate from these GUI tests.

[VALIDATION.md](VALIDATION.md) records completed results, exact filenames, binary hashes and bounded test scope for each release. Historical 0.11–0.13.1 passes identify their respective binaries rather than the changed 0.13.2 materials, POWER area and mouse-key feedback. Revalidate the rebuilt bundle after source or asset changes; an older output path does not identify a new build's contents. No native DAW or physical-display test is implied by these build instructions.

Historical 0.10 pluginval strictness 10 completed with GUI tests and exit 0 after CPU-backed artwork and the editor's own software renderer avoided an observed NVIDIA shutdown wait. That remains an implementation precaution, not a substitute for validating the current build or a guarantee about every driver.

## Installation and clean builds

The system installation location is `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; the 0.13.2 VST3/standalone staging convention is `artifacts/Release/Toyland KB-09-0.13.2-Windows-x64`. Use `artifacts/Presets-0.13.2-final` for the matching exported factory bank and `artifacts/GUI-0.13.2-final` for editor renders. These path names do not establish completed exports. Use [VALIDATION.md](VALIDATION.md) for the installed and staged binary identities rather than a version label alone. For installation, copy the entire bundle, including `Contents`, into the system location or a custom VST3 folder scanned by the DAW. `COPY_PLUGIN_AFTER_BUILD` is false; build output and installation remain separate operations. The standalone `.exe` is usable without a DAW after selecting audio/MIDI devices.

Legacy SH-09 and KB-09 share VST3 identifiers; expose one current bundle in scanned folders and keep historical backups outside them. Preserve user Sets and evaluate upgrade recall on a copy. [LIVE_TEST.md](LIVE_TEST.md) scopes older observations separately from Kevin's confirmed Auto-Scale fix. Matching identifiers supports recall but does not establish an actual host result. Since 0.11, older Modern patches may sound deeper with nonzero MOD; Authentic mappings are unchanged. The [manual's DAW sections](manual/index.html#daws) collect the confirmed Live setting and version-scoped vendor guidance for other hosts.

For a clean-build comparison, close hosts using the binary, configure a new empty directory such as `build-kb09-clean`, and rebuild/recheck there. Retain previous logs until the new result is recorded. Release packaging is separate from these commands; no publishing or installation is implied.

`python tools/package_windows.py` assembles the personal Windows package from this workstation's `C:\Dev\SH-09\build-kb09` outputs. Before running it for 0.13.2, align its version, validated source hashes, factory-bank directory and `panel-0.13.2.png` path with that release's recorded evidence. The script verifies source hashes, copies the selected runtime/manual dependencies, exports checksums, validates offline links and checks every ZIP entry. It requires a fresh destination and preserves any existing package. It neither installs nor publishes a plugin. Any separately included unsigned AAX development bundle and macOS source kit must have their actual status visible in the package rather than masquerading as tested native releases.

## Runtime and licensing

Release x64 builds use Windows system libraries and the Microsoft C++ x64 runtime. Dependency reports include `MSVCP140.dll`, `VCRUNTIME140.dll` and `VCRUNTIME140_1.dll`; inspect the final module's imports when packaging. The destination PC needs the corresponding release runtime. Python/NumPy serve the optional offline calibration tools, not plugin playback.

JUCE licensing applies independently to distribution. The installed JUCE 9 `LICENSE.md` offers its commercial EULA or AGPLv3 route; this project does not choose a commercial licence for the user. The bundled AAX `SDK/LICENSE.txt` offers the Avid SDK agreement/privacy terms or GPLv3; the bundled AU SDK license is Apache 2.0. Preserve the dependency notices and actual source license files in any source kit. Including source does not establish that all third-party distribution obligations or signing requirements are satisfied, and this build does not assign a new license to Kevin's project. Validator binaries are development tools, not runtime dependencies. No third-party emulator source or copied commercial plugin artwork is incorporated; runtime artwork combines project-generated assets informed by user-supplied reference photographs with user-supplied marks and native drawing.
