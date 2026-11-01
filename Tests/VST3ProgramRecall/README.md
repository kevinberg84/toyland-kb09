# VST3 program recall protocol regression

Uses the official Steinberg SDK to reproduce the ordering captured in Renoise
3.5.4: choose a factory program, restore the saved custom component/controller
state, then deliver parameter edits previously published by the program change.
It inspects the processor's state chunk, rather than trusting the host's cached
parameter values. It checks all 24 factory program references and exits nonzero
if any custom cutoff is overwritten. No audio or MIDI is recorded.

The released Windows 1.10.0 fails all 24 cases. The Issue 4 corrected candidate
passes all 24. `Tests/Host.cpp --program-recall` additionally checks complete
patch retention through JUCE's host, which does not itself reproduce Renoise's
queued factory-parameter echo.

Build with an existing official SDK checkout and its built Release libraries:

```powershell
cmake -S Tests/VST3ProgramRecall -B build/vst3-recall -DSDK=C:/path/to/vst3sdk -DSDK_LIBS=C:/path/to/sdk-build/lib/Release
cmake --build build/vst3-recall --config Release
build/vst3-recall/Release/KB09VST3ProgramRecall.exe "C:/path/to/Toyland KB-09.vst3"
```

The protocol probe is Windows-only. The project-local wrapper guard is built for
VST3 on all supported platforms, using `.mm` on macOS. Standalone, AU, DSP, RATE
presentation and power-button behavior are outside this correction.
