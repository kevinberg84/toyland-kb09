# Toyland KB-09 0.11.0 — Windows x64 build

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

The internal target remains `SH09`; the public product is **Toyland KB-09**. CMake sets version 0.11.0. `Assets/Branding.h` defines the visible identity; compatibility keys remain unchanged as described in [REBRANDING.md](REBRANDING.md).

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

The source package must retain `CMakeLists.txt`, `Source`, `Tests`, `VSTLab`, `Assets`, scripts and documentation: the top-level build includes the test/lab directories. `KB09Artwork` compiles nine PNGs from `Assets/Photoreal`: `chassis-black.png`, `chassis-patina.png`, `chassis-stains.png`, `knob.png`, `fader.png`, `keys.png`, `wear-atlas.png`, `marks.png` and `marks-aged.png`. Retain every listed input. The wear atlas supplies 32 independent key stains, two independent cracks and three independent rotary-corrosion patches; the old repeated aged-key/knob/crack images are historical inputs only. `CMakeLists.txt` and `Assets/Photoreal/README.md` distinguish compiled layers from provenance-only source images. Native code also supplies text, calibrations, switches and the projected bender. No runtime Assets directory is required. Asset provenance/prompts are retained in `Assets/Photoreal`.

## Checks and compiled previews

Run registered DSP, processor and preset-file tests:

```powershell
ctest --test-dir build-kb09 -C Release --output-on-failure
```

Run the actual-module host and compiled-editor fixture separately:

```powershell
& '.\build-kb09\SH09Host_artefacts\Release\SH09Host.exe' '.\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3' 600
& '.\build-kb09\SH09GUIRender_artefacts\Release\SH09GUIRender.exe' '.\artifacts\GUI-0.11-final'
```

The GUI fixture exports patch/material conditions, bender positions and component metadata. It checks footer bounds at wide/tall sizes, held white/black-key feedback, configured value-bubble contrast, original parameter identities, physical switches, stain/crack events and distinct healthy/aged key samples. Its 3840 × 2160 exports are compiled renders, not physical 4K-display observations; the chassis raster remains 1774 × 887. Native hover-popup placement was not observed. See [UI.md](UI.md).

Run GUI-enabled pluginval strictness 10 and Steinberg's validator against the **same rebuilt bundle**, recording process exit codes. [Validator instructions](../tools/VALIDATORS.md) use the current product path. The audio renderer and Python calibration tools are separate from these GUI tests.

Final October 7 results are **92 DSP, 20 processor, 58 preset-file, 280 GUI, 47 Steinberg and 75 actual-VST3 host checks passed**, all without failures; GUI-enabled pluginval strictness 10 reported SUCCESS and exited 0. Processor guards recorded zero tracked C++ allocations in covered callbacks. Processor/preset/GUI logs use `-0.11-aging-events-final.txt`; Steinberg/host use `-0.11-final.txt`. DSP retains `dsp-0.11-materials-final.txt` because the engine did not change during the later visual revisions. [VALIDATION.md](VALIDATION.md) records the full filenames, binary hashes and bounded test scope. Revalidate after source or asset changes; an older output path does not identify a new build's contents.

Historical 0.10 pluginval strictness 10 completed with GUI tests and exit 0 after CPU-backed artwork and the editor's own software renderer avoided an observed NVIDIA shutdown wait. That remains an implementation precaution, not a substitute for validating 0.11 or a guarantee about every driver.

## Installation and clean builds

The final 0.11 bundle is installed at `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`; staged VST3/standalone artifacts are in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`. The build, staged and installed VST3 module SHA-256 is `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`; the staged standalone hash is `93F1660185E8C67781D0118DAED461598080580EB30976AB6D5C11D227E6216A`. For another installation, copy the entire bundle, including `Contents`, into the system location or a custom VST3 folder scanned by the DAW. `COPY_PLUGIN_AFTER_BUILD` is false; build output and installation remain separate operations.

Legacy SH-09 and KB-09 share VST3 identifiers. The preceding 0.10 bundle is preserved outside scanned folders; expose one current bundle. Preserve user Sets and evaluate upgrade recall on a copy. Live-specific 0.11 tests were not repeated because desktop automation could not resolve Live's foreground process after three recovery attempts; [LIVE_TEST.md](LIVE_TEST.md) scopes older observations. Matching identifiers supports recall but does not establish an actual host result. Older Modern patches may sound deeper with nonzero MOD in 0.11; Authentic mappings are unchanged.

For a clean-build comparison, close hosts using the binary, configure a new empty directory such as `build-kb09-clean`, and rebuild/recheck there. Retain previous logs until the new result is recorded. Release packaging is separate from these commands; no publishing or installation is implied.

## Runtime and licensing

Release x64 builds use Windows system libraries and the Microsoft C++ x64 runtime. Dependency reports include `MSVCP140.dll`, `VCRUNTIME140.dll` and `VCRUNTIME140_1.dll`; inspect the final module's imports when packaging. The destination PC needs the corresponding release runtime. Python/NumPy serve the optional offline calibration tools, not plugin playback.

JUCE licensing applies independently to distribution. The installed JUCE 9 source offers its commercial EULA or AGPLv3 route; this project does not choose a commercial licence for the user. Validator binaries are development tools, not runtime dependencies. No third-party emulator source or copied commercial plugin artwork is incorporated; runtime artwork combines project-generated assets with user-supplied marks.
