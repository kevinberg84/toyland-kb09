# Validator tools

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
& '.\tools\pluginval\pluginval.exe' --strictness-level 5 --sample-rates 44100,48000,88200,96000,192000 --block-sizes 1,17,64,257,1024,8192 --random-seed 9009 --output-dir '.\artifacts\Validation' --output-filename 'pluginval-level5.txt' --validate '.\artifacts\Release\SH-09.vst3'
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
& '.\tools\vst3sdk-build\bin\Release\validator.exe' '.\artifacts\Release\SH-09.vst3' *> '.\artifacts\Validation\steinberg.txt'
$LASTEXITCODE
```

The actual executable's help lists `-e` for extensive tests, `-l` for a fresh local instance per test, and `-selftest` for validator self-tests. Basic validation was executed on the lab and initial main instrument: each reported **47 passed, 0 failed**, exit 0. Logs are `artifacts/Validation/lab-steinberg.txt` and `artifacts/Validation/sh09-steinberg.txt`. These results apply to the binaries and revisions named in those logs; final release validation must be rerun after code changes. An informational line says 64-bit *sample precision* is unsupported; this does not mean the x64 module is unsupported. These plugins process 32-bit float samples in an x64 process.

pluginval 1.0.4 accepts `--vst3validator <path>` to invoke this tool as part of a validation run. The independent runs above preserve distinct output. Neither tool establishes sonic equivalence to hardware or a completed Live routing test.

Tool binaries and archives are development dependencies, not required runtime content of the distributed SH-09 bundle. Preserve applicable upstream licenses if redistributing them.
