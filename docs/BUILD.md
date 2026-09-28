# Windows x64 build

JUCE 9.0.2 is an external dependency; this project does not modify JUCE. The tested compiler is MSVC 19.51.36260.0 with Visual Studio 18 2026 and the Windows SDK. CMake's Visual Studio generator sets up the compiler without running PowerShell scripts or changing execution policy.

From the project directory, run `scripts\build.cmd`, or configure/build explicitly:

```
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DJUCE_DIR=C:/Dev/JUCE
cmake --build build --config Release --parallel 8
```

The default JUCE location is `../JUCE`; override `JUCE_DIR` to build elsewhere. `scripts/build.cmd` first finds CMake on PATH and then tries the known Visual Studio installation. Other installations should use their CMake path explicitly. No source path is needed at runtime.

Targets include the SH-09 VST3/Standalone instrument, the preserved VSTLab, SH09Host, and DSP tests when configured. Standalone is for local GUI/audio testing; VST3 is the DAW deliverable. Build products go under `build`; accepted artifacts are copied to `artifacts/Release` after validation.

The conventional system VST3 destination is `C:\Program Files\Common Files\VST3\SH-09.vst3` (the whole bundle directory). Installation there requires explicit permission and is not performed by the build. Live can instead scan a custom VST3 folder pointing at the local release folder. Copy the entire `.vst3` directory, including `Contents`, not only its inner binary.

For a fresh-build test, first close hosts using the project binary, rename the existing project-local `build` directory to a dated backup, configure into a new `build`, rebuild Release x64, and rerun validation on the new bundle. Do not delete unrelated files or change system security settings.

JUCE licensing applies independently to the user's distribution model. The installed JUCE 9 source offers its commercial EULA or AGPLv3 route. No third-party emulator source or assets are incorporated. This project does not choose a commercial JUCE license on the user's behalf.

The clean binary imports the release Microsoft C++ x64 runtime (`MSVCP140.dll`, `VCRUNTIME140.dll`, `VCRUNTIME140_1.dll`) and Windows system DLLs. It requires that runtime on the destination PC. `artifacts/Validation/dependencies.txt` records the actual imports; there are no Debug-runtime imports or JUCE/source-folder runtime references.
