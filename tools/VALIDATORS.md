# Validator tools

## Current 0.13.2 release

Validate the exact rebuilt bundle and confirm process exit. The final VST3 module SHA-256 is `5257325EF38A733D716D85B303443D61BEDBAC833A202A0FC348B3E3E2C9F7BE`, matching the installed build. DSP 92, processor 20 (zero tracked allocations), presets 58, compiled-editor 347, Steinberg 47 and actual-VST3 host 75 checks pass with no failures. GUI-enabled Pluginval strictness 10 reports SUCCESS and confirmed process exit 0. Exact release logs and scope are in [VALIDATION.md](../docs/VALIDATION.md).

On this workstation, invoking the GUI-subsystem pluginval with PowerShell's `&` did not populate `$LASTEXITCODE`. The confirmed run used `Start-Process -Wait -PassThru -WindowStyle Hidden` and recorded the returned `ExitCode`. A SUCCESS line alone is not process-completion evidence.

AAX is a compiled unsigned x64 development bundle with inspected ACF exports/imports, not an AAX host validation pass. No Pro Tools host/signing tools or Mac/AU tools are available here. Keep format status distinct from the successful VST3 tests.

## Historical 0.11 validation scope

Validate the exact rebuilt **Toyland KB-09.vst3** bundle, preserve each tool's exit code, and record the inner module SHA-256. Final October 7 module hash: `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`, identical in build, staging and installation. Results: **92 DSP, 20 processor, 58 preset-file, 280 GUI, 47 Steinberg and 75 actual-VST3 host checks passed**, all without failures. GUI-enabled pluginval strictness 10 reported SUCCESS and exited 0. [VALIDATION.md](../docs/VALIDATION.md) and [ACCEPTANCE.md](../docs/ACCEPTANCE.md) record the exact final scope. Revalidate after any source or asset change.

Final logs under `artifacts/Validation-KB09` are `processor-0.11-aging-events-final.txt`, `presets-0.11-aging-events-final.txt`, `gui-0.11-aging-events-final.txt`, `steinberg-0.11-final.txt`, `host-0.11-final.txt`, `pluginval-0.11-final-level10.txt` and `pluginval-0.11-final-exit.txt`. DSP evidence remains `dsp-0.11-materials-final.txt`; the engine was unchanged by later visual revisions. The host's 600-second offline render took 51.0971 seconds of callback time, with a maximum observed 128-sample callback of 1.006 ms at 48 kHz. This is a bounded measurement, not a DAW CPU guarantee.

The GUI fixture checks transformed footer bounds, held white/black notes, bender positions and configured value-popup contrast. Expanded damage checks cover six abrupt stain thresholds with fixed subsequent appearance, two separate crack events and distinct healthy/aged samples of all 32 keys. All fader caps remain fitted. Its `artifacts/GUI-0.11-final` previews are compiled-editor exports, not physical 4K display or DAW-window tests. Final standalone M/L were separately observed with the complete footer. No physical 4K display or actual native hover-popup placement was validated. Preset-file validation has a separate `SH09PresetFilesTests` target; neither pluginval nor Steinberg alone establishes every native file-dialog interaction. Live-specific 0.11 checks were not repeated after desktop automation failed to resolve its foreground process; see [LIVE_TEST.md](../docs/LIVE_TEST.md).

Windows editors use CPU-backed persistent artwork and select software rendering for their own JUCE peer. This addressed an observed historical NVIDIA shutdown wait without altering the DAW renderer. Keep GUI-enabled pluginval testing and verify actual process exit; a log ending in SUCCESS without process completion is insufficient.

## Acquired: Tracktion pluginval 1.0.4 (Windows)

Retrieved 2026-09-26 from the official Tracktion GitHub release. GitHub's release API returned `v1.0.4` as latest during acquisition.

- Project: https://github.com/Tracktion/pluginval
- Release: https://github.com/Tracktion/pluginval/releases/tag/v1.0.4
- Download: https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip
- Archive: `tools/pluginval_Windows_v1.0.4.zip`, 2,408,590 bytes.
- SHA-256: `C08E61CE3B96DB41636F8EC7E76F4C7E2C13EBDAC7FA1B5A1F52B4F32EC715AB` (locally computed, not a signature verification).
- Extracted executable: `tools/pluginval/pluginval.exe`, 5,909,504 bytes.
- Executed `--help` successfully; executed `--version`, which printed `pluginval - 1.0.4`. Its help banner reports JUCE 8.0.3. A validator can use a different JUCE version from the plug-in under test.

No plug-in pass is implied by these tool availability checks.

Run from the project root in PowerShell, substituting the actual compiled bundle path:

```powershell
New-Item -ItemType Directory -Path '.\artifacts\Validation' -Force | Out-Null
& '.\tools\pluginval\pluginval.exe' --strictness-level 5 --sample-rates 44100,48000,88200,96000,192000 --block-sizes 1,17,64,257,1024,8192 --random-seed 9009 --output-dir '.\artifacts\Validation' --output-filename 'pluginval-level5.txt' --validate '.\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3'
$LASTEXITCODE
```

Then run strictness 10 with its own filename and preserve the exit status. Default timeout is 30 seconds without output; `--timeout-ms 120000` can accommodate stress tests. `--skip-gui-tests` is available for a true headless run, but use a separate GUI-enabled run in the accessible desktop session and do not report skipped GUI coverage as passed. Available CLI options were read from the acquired executable rather than assumed from a newer development branch. Avoid passing block size zero to a validator unless the particular test supports it; the project's direct host test should explicitly exercise a zero-length callback.

`pluginval` exits 0 when all selected tests pass, 1 on failure. Do not infer success only from a log file being created.

## Built: independent Steinberg validator 3.8.1

Official source: https://github.com/steinbergmedia/vst3sdk (recursive submodules required). Documentation: https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Validator.html . The project contains the validator under `public.sdk/samples/vst-hosting/validator`. The locally installed JUCE SDK subset does not contain that validator target.

Cloned the official repository with recursive submodules on 2026-09-26, then built the `validator` target in Release x64 using MSVC 19.51.36260.0. No JUCE source was modified.

- SDK commit: `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`.
- `public.sdk` commit: `586dc5e6c8012c3e4b01c79389375cbe96bdb1da`.
- Source: `tools/vst3sdk`.
- Executable: `tools/vst3sdk-build/bin/Release/validator.exe`.
- Executable SHA-256: `4A9E45298815893C750EB39E1C8ABC8BB6AA9B91B679AA108E9F2229C2D0C773`.
- `--version`: `VST 3.8.1 Plug-in Validator`, built Sep 26 2026.
- Build-time validator self-test: **51 tests executed, 0 failed**.

Configuration/build commands (use the installed CMake executable or a developer environment with it on PATH):

```powershell
cmake -S '.\tools\vst3sdk' -B '.\tools\vst3sdk-build' -G 'Visual Studio 18 2026' -A x64 -DSMTG_ENABLE_VST3_PLUGIN_EXAMPLES=OFF -DSMTG_ENABLE_VST3_HOSTING_EXAMPLES=ON -DSMTG_ENABLE_VSTGUI_SUPPORT=OFF
cmake --build '.\tools\vst3sdk-build' --config Release --target validator -j 4
& '.\tools\vst3sdk-build\bin\Release\validator.exe' '.\build-kb09\SH09_artefacts\Release\VST3\Toyland KB-09.vst3' *> '.\artifacts\Validation\steinberg.txt'
$LASTEXITCODE
```

The actual executable's help lists `-e` for extensive tests, `-l` for a fresh local instance per test, and `-selftest` for validator self-tests. Historical basic validation was executed on the lab and initial SH-09 instrument: each reported **47 passed, 0 failed**, exit 0. Logs are `artifacts/Validation/lab-steinberg.txt` and `artifacts/Validation/sh09-steinberg.txt`. These results apply to the binaries and revisions named in those logs; final release validation must be rerun after code changes. An informational line says 64-bit *sample precision* is unsupported; this does not mean the x64 module is unsupported. These plugins process 32-bit float samples in an x64 process.

pluginval 1.0.4 accepts `--vst3validator <path>` to invoke this tool as part of a validation run. The independent runs above preserve distinct output. Neither tool establishes sonic equivalence to hardware or a completed Live routing test.

Tool binaries and archives are development dependencies, not required runtime content of the distributed Toyland KB-09 bundle. Preserve applicable upstream licenses if redistributing them.
