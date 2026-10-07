# Toyland KB-09 0.12.0 — Windows x64 build

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

## Targets and source contents

The internal target remains `SH09`; the public product is **Toyland KB-09**. CMake sets version 0.12.0 and builds **VST3** and **Standalone**. AU and AAX are future platform work, not targets or release outputs in this build. `Assets/Branding.h` defines the visible identity; compatibility keys remain unchanged as described in [REBRANDING.md](REBRANDING.md).

| Target | Release output relative to build directory |
|---|---|
| `SH09_VST3` | `SH09_artefacts/Release/VST3/Toyland KB-09.vst3` |
| `SH09_Standalone` | `SH09_artefacts/Release/Standalone/Toyland KB-09.exe` |
| `SH09Host` | `SH09Host_artefacts/Release/SH09Host.exe` |
| `SH09ProcessorTests` | `SH09ProcessorTests_artefacts/Release/SH09ProcessorTests.exe` |
| `SH09PresetFilesTests` | `SH09PresetFilesTests_artefacts/Release/SH09PresetFilesTests.exe` |
| `SH09GUIRender` | `SH09GUIRender_artefacts/Release/SH09GUIRender.exe` |
| `SH09DSPTests` | `Release/SH09DSPTests.exe` |
| `SH09Render` | `Release/SH09Render.exe` |

The source package must retain `CMakeLists.txt`, `Source`, `Tests`, `VSTLab`, `Assets`, scripts and documentation: the top-level build includes the test/lab directories. `KB09Artwork` compiles **eight PNGs** from `Assets/Photoreal`:

- `chassis-black.png`
- `chassis-personal-aged.png`
- `chassis-stains.png` — only the retained envelope coffee-ring region is rendered.
- `knob-personal.png`
- `fader-personal-v2.png`
- `keys.png`
- `marks.png`
- `marks-aged.png`

Retain every listed input. The old wear atlas is not compiled or used by the current renderer. Native code supplies independently seeded key dust/grime, the single thin crack, knob rim/tarnish details, labels/symbols, switches, exposed stepped fader posts and the bender. Kevin's multi-angle photographs guide the current control geometry and wear; his replacement volume knob, sticker and taped waveform marks are excluded. `CMakeLists.txt` and `Assets/Photoreal/README.md` distinguish compiled layers from provenance-only source images. No runtime Assets directory is required. Asset provenance/prompts are retained in `Assets/Photoreal`.

The offline working manual is `docs/manual/index.html`. Package it with `styles.css`, `assets/marks.png`, `assets/panel-0.12.png` and `reference/evidence.html` so its internal DAW/evidence links and illustrations work without a network connection. Exclude its development-only `qa` folder and historical images. External vendor links still require internet access.

## Checks and compiled previews

Run registered DSP, processor and preset-file tests:

```powershell
ctest --test-dir build-kb09 -C Release --output-on-failure
```

Run the actual-module host and compiled-editor fixture separately:

```powershell
& '.\build-kb09\SH09Host_artefacts\Release\SH09Host.exe' '.\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3' 600
& '.\build-kb09\SH09GUIRender_artefacts\Release\SH09GUIRender.exe' '.\artifacts\GUI-0.12-final'
```

The GUI fixture exports patch/material conditions, a button-selected S viewport, bender positions and component metadata. It exercises S/M/L sizing and restoration, footer bounds at wide/tall sizes, held white/black-key feedback, configured value-bubble contrast, original parameter identities, physical switches, the single crack, the two cap-loss events, the broken bender and distinct healthy/aged key samples. Its 3840 × 2160 exports are compiled renders, not physical 4K-display observations; photographic sources retain their finite resolution. Native hover-popup placement needs separate observation. See [UI.md](UI.md).

Run GUI-enabled pluginval strictness 10 and Steinberg's validator against the **same rebuilt bundle**, recording process exit codes. [Validator instructions](../tools/VALIDATORS.md) use the current product path. The audio renderer and Python calibration tools are separate from these GUI tests.

[VALIDATION.md](VALIDATION.md) records completed results, exact filenames, binary hashes and bounded test scope for each release. Historical 0.11 passes identify the preceding binary rather than the changed 0.12 GUI. Revalidate the rebuilt bundle after source or asset changes; an older output path does not identify a new build's contents.

Historical 0.10 pluginval strictness 10 completed with GUI tests and exit 0 after CPU-backed artwork and the editor's own software renderer avoided an observed NVIDIA shutdown wait. That remains an implementation precaution, not a substitute for validating the current build or a guarantee about every driver.

## Installation and clean builds

The system installation location is `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; the 0.12 VST3/standalone staging location is `artifacts/Release/Toyland KB-09-0.12.0-Windows-x64`. Use [VALIDATION.md](VALIDATION.md) for the installed and staged binary identities rather than a version label alone. For another installation, copy the entire bundle, including `Contents`, into the system location or a custom VST3 folder scanned by the DAW. `COPY_PLUGIN_AFTER_BUILD` is false; build output and installation remain separate operations. The standalone `.exe` is usable without a DAW after selecting audio/MIDI devices.

Legacy SH-09 and KB-09 share VST3 identifiers; expose one current bundle in scanned folders and keep historical backups outside them. Preserve user Sets and evaluate upgrade recall on a copy. [LIVE_TEST.md](LIVE_TEST.md) scopes older observations separately from Kevin's confirmed Auto-Scale fix. Matching identifiers supports recall but does not establish an actual host result. Since 0.11, older Modern patches may sound deeper with nonzero MOD; Authentic mappings are unchanged. The [manual's DAW sections](manual/index.html#daws) collect the confirmed Live setting and version-scoped vendor guidance for other hosts.

For a clean-build comparison, close hosts using the binary, configure a new empty directory such as `build-kb09-clean`, and rebuild/recheck there. Retain previous logs until the new result is recorded. Release packaging is separate from these commands; no publishing or installation is implied.

`python tools/package_windows.py` assembles the exact validated 0.12 personal Windows package from this workstation's `C:\Dev\SH-09\build-kb09` outputs. It verifies source hashes, copies only runtime/manual dependencies, exports checksums, validates offline links and checks every ZIP entry. It requires a fresh destination and preserves any existing package. It neither installs nor publishes a plugin.

## Runtime and licensing

Release x64 builds use Windows system libraries and the Microsoft C++ x64 runtime. Dependency reports include `MSVCP140.dll`, `VCRUNTIME140.dll` and `VCRUNTIME140_1.dll`; inspect the final module's imports when packaging. The destination PC needs the corresponding release runtime. Python/NumPy serve the optional offline calibration tools, not plugin playback.

JUCE licensing applies independently to distribution. The installed JUCE 9 source offers its commercial EULA or AGPLv3 route; this project does not choose a commercial licence for the user. Validator binaries are development tools, not runtime dependencies. No third-party emulator source or copied commercial plugin artwork is incorporated; runtime artwork combines project-generated assets informed by user-supplied reference photographs with user-supplied marks and native drawing.
