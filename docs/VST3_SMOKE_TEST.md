# Disposable VST3 checkpoint

The disposable **SH09 VST Lab** was built and instantiated before the main instrument implementation. This file records actual test evidence; it is not a claim of finished SH-09 fidelity or Ableton compatibility.

## Build

- Sources: `VSTLab/CMakeLists.txt` and `VSTLab/Lab.cpp`.
- Framework: locally installed JUCE **9.0.2** at `C:\Dev\JUCE`.
- Toolchain: Visual Studio 18 / MSVC **19.51.36260.0**, Windows SDK, Release **x64**.
- Build directory: `C:\Dev\SH-09\build`.
- Actual module bundle: `C:\Dev\SH-09\build\VSTLab\VSTLab_artefacts\Release\VST3\SH09 VST Lab.vst3`.
- Product/version: Private Instruments / SH09 VST Lab **0.1.0**.

The lab contains a MIDI-controlled sine oscillator, one-pole low-pass filter, automatable cutoff/gain, APVTS state and a native generic editor. It has stereo output and no external audio input. It is intentionally disposable and is not the emulation DSP.

## Initial checkpoint, 2026-09-26

`SH09Host.exe` scanned and loaded the actual VST3, delivered note-on messages and inspected output samples. It exercised all **35 combinations** of 44.1, 48, 88.2, 96 and 192 kHz with blocks of 1, 17, 64, 127, 256, 1024 and 8192 samples. Every combination produced nonzero finite, bounded output. A parameter state round trip and repeated editor creation/destruction were checked. Result: **PASS, 0 failures**. Original log: `C:\Dev\SH-09\artifacts\Validation\lab-host.txt`.

Tracktion **pluginval 1.0.4**, strictness **5**, loaded and exercised the bundle; its log ends **SUCCESS**. Log: `artifacts/Validation/LabPluginval/SH09 VST Lab.vst3_20260926T162952.740-0500.txt`. That run explicitly skipped invoking Steinberg's external validator because no path was set; a separate independent run followed.

Steinberg **VST 3.8.1 validator**, locally built from official source, independently loaded the lab bundle. Result: **47 passed, 0 failed; exit 0**. Log: `artifacts/Validation/lab-steinberg.txt`.

## Expanded integration regression, 2026-09-27

The expanded `Tests/Host.cpp` again loaded the lab module and passed **46 assertions, 0 failures**:

- The same 35 sample-rate/block-size combinations.
- Note-on at sample 173 and at the last sample, verifying no audio before the event and audible output afterward where measured.
- Variable and zero-length callbacks within the negotiated VST3 maximum block size.
- All exposed parameter values restored from VST3 state, and malformed state rejected without patch corruption.
- Dense note/pitch-wheel/panic traffic over 500 blocks.
- A 12-second sustained render, finite and audible throughout.
- Ten native editor create/process/resize/destroy cycles, plus eight additional instance create/process/destroy cycles.

Expanded log: `artifacts/Validation/lab-host-expanded.txt`. The lab's single program is omitted by the VST3 wrapper, so the log explicitly skips program enumeration and SH-09 external-input tests. Those require the main instrument.

Two test-harness problems were corrected during expansion. The initial harness exceeded VST3's negotiated `maxSamplesPerBlock`; this violated the host contract and overflowed the JUCE host's prepared mapping storage. Varying blocks now stay within the negotiated maximum. The lab's legacy gain parameter uses 0.01 native increments; a requested normalized 0.79 represents 0.395 and is quantized to 0.40, or normalized 0.80. The corrected state test uses representable values and retains a strict 0.0002 normalized comparison, rather than widening tolerance to hide this distinction.

## Limits of this checkpoint

The logs establish actual compilation, module loading, audio processing, basic state and editor lifecycle. Editor lifecycle tests do not establish mouse interaction, typography, resizing appearance or high-DPI visual quality. These are not Ableton tests. No lab test assesses the final oscillator, filter, external-input circuit or hardware matching. Main-instrument and release results belong in `VALIDATION.md` and its logs.
