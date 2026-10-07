# Validation record

## Current release — Toyland KB-09 0.11.0, 2026-10-07

Final VST3 module SHA-256: `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`.
Final standalone SHA-256: `93F1660185E8C67781D0118DAED461598080580EB30976AB6D5C11D227E6216A`.

The build, staged bundle and installed module hashes match. Staged artifacts are in `artifacts/Release/Toyland KB-09-0.11.0-Windows-x64`; installed bundle is `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`. Its moduleinfo reports Toyland KB-09 version 0.11.0. The prior 0.10.0 bundle is preserved in `artifacts/Backup/Toyland KB-09-0.10.0-before-0.11.0` with its original hash `FC552E9382A8B622B368CC673AC4C64B3A89CB23EEEA307131A84CC094EED48F`. The earlier SH-09 bundle remains outside scanned folders; there is one current identity.

| Final check | Result | Log under artifacts/Validation-KB09 |
|---|---|---|
| Release build | Exit 0 | build-0.11-final.txt |
| DSP | 92/0; engine unchanged by the later visual edits | dsp-0.11-materials-final.txt |
| Processor | 20 PASS records, zero failures and tracked allocations | processor-0.11-aging-events-final.txt |
| Preset files | 58/0, including imported factory-bank state | presets-0.11-aging-events-final.txt |
| Real-editor GUI | 280/0 | gui-0.11-aging-events-final.txt |
| Steinberg VST3 | 47/0, exit 0 | steinberg-0.11-final.txt |
| Pluginval strictness 10, GUI enabled | SUCCESS, confirmed process exit 0 | pluginval-0.11-final-level10.txt and pluginval-0.11-final-exit.txt |
| Actual compiled VST3 host | 75/0; 600-second render; 16 preset exports/round-trips | host-0.11-final.txt |

Pluginval uses seed 9009, rates 44.1/48/88.2/96/192kHz and blocks 1/17/64/257/1024/8192. GUI lifecycle tests run; they do not establish visual fidelity. CPU-backed artwork and software rendering on the editor's own JUCE peer preserve the shutdown fix.

The final host's 600-second offline render took 51.0971 seconds of callback time (8.51618% of realtime); maximum observed 128-sample callback was 1.006 ms against a 2.66667 ms deadline at 48 kHz. This single offline measurement includes scheduling outliers and is not a DAW CPU guarantee. It also verifies35 rate/buffer combinations, latency128, MIDI timing, state/automation, external-input/follower routing, stress, 10 editor lifecycles and eight additional instances.

GUI exports are in `artifacts/GUI-0.11-final`. The expanded fixture tests six tiny Age-threshold crossings for abrupt stains and fixed subsequent appearance, two separate crack onsets, and distinct healthy/aged samples from all 19 white and 13 black keys. It also covers physical switches, keyboard feedback, configured value-bubble contrast and transformed footer geometry at 16:9, wide and tall sizes. Main outputs are 3840x2160 and 2880x1620; underlying raster resolutions remain finite. A physical 4K monitor and every native hover-popup placement have not been validated.

The final standalone M/L windows were observed at 1051x642 and 1280x720 logical pixels on this workstation, including a complete footer in L and return to the enlarged M. Native Save produced a default .kb09 file in the earlier 0.11 checkpoint; custom-filename typing and native Load round-trip were not completed by UI automation. The 58 filesystem/preset regressions independently establish the implemented state/filename handling. Live-specific observations are separately scoped in LIVE_TEST.md; historical checks below apply to their named earlier binary.

## Historical development records

Status on 2026-09-27: **the clean Release build, 83 DSP assertions, 7 processor assertions, 75 VST3-host assertions, pluginval strictness 10 and 47 independent Steinberg tests passed**. Actual Ableton Live 11.0 loading, external routing, preset interaction and Set recall were observed as detailed below. These passes do not establish hardware equivalence or complete the explicitly unperformed host/UI checks.

## Clean build, binary identity and runtime dependencies

The development build was retained as `C:\Dev\SH-09\build-development`; a fresh `C:\Dev\SH-09\build` was configured and built from source. The fresh build log records the VST3, standalone, DSP-test, processor-test and host targets completing. No compiler warning or error lines were found in that log. Evidence: `C:\Dev\SH-09\artifacts\Validation\fresh-build.txt`.

- Framework: **JUCE 9.0.2**.
- Generator/configuration: **Visual Studio 18 2026**, **Release x64**.
- Compiler: **MSVC 19.51.36260.0**; MSBuild **18.10.1**.
- CMake: **4.3.1**; Windows SDK: **10.0.26100.0**.
- Release bundle: `C:\Dev\SH-09\artifacts\Release\SH-09.vst3`.
- Installed bundle: `C:\Program Files\Common Files\VST3\SH-09.vst3`.

The tested module and installed module have the same SHA-256 recorded in the final-results section below. Installation is not itself a successful Live scan or routing test.

The PE dependency report lists Windows system libraries and the Release Microsoft C/C++ runtime, including `MSVCP140.dll`, `VCRUNTIME140.dll` and `VCRUNTIME140_1.dll`; it does not list Debug CRT imports or project build DLLs. A destination system needs the corresponding Microsoft x64 runtime. This is an import-table inspection, not a claim that every conceivable dynamic file access has been monitored. Evidence: `C:\Dev\SH-09\artifacts\Validation\dependencies.txt`.

## Development instrument tests

Tested bundle: `C:\Dev\SH-09\build\SH09_artefacts\Release\VST3\SH-09.vst3`, version **0.9.0**, Release x64. The later clean-build process may recreate this same path; the development logs alone do not certify its new contents.

| Tool / scope | Observed result | Evidence |
|---|---|---|
| Tracktion pluginval 1.0.4, strictness 10 | SUCCESS | `artifacts/Validation/sh09-pluginval-initial.txt` |
| Steinberg VST 3.8.1 validator | 47 passed, 0 failed; exit 0 | `artifacts/Validation/sh09-steinberg.txt` |
| Project VST3 integration host, JUCE 9.0.2 | 73 assertions passed, 0 failures; exit 0 | `artifacts/Validation/sh09-host.txt` |

The integration host loads the built VST3 through JUCE's VST3 host wrapper. It does not link the synth's internal engine directly. Coverage includes:

- 35 rate/buffer combinations: 44.1, 48, 88.2, 96 and 192 kHz crossed with blocks of 1, 17, 64, 127, 256, 1024 and 8192 samples.
- Mid-block and final-sample MIDI note timing, variable and zero-length buffers within the negotiated VST3 maximum.
- All exposed parameter values saved/restored through actual VST3 state, malformed-state rejection, and three cycles through 16 factory presets with per-preset restoration.
- External stereo left/right input equality, mono/stereo fold-down equivalence, antiphase cancellation, disabled input silence, cutoff-dependent processing and a follower-driven increase in filtered output energy without MIDI.
- External NaN, positive/negative infinity and very large finite input sanitation, with output checked for finiteness and absolute magnitude no greater than 8.
- 500 blocks of parameter extremes/random changes with dense note, pitch-wheel and panic traffic.
- 30 seconds of sustained rendering, ten native editor create/process/resize/destroy cycles and eight additional complete instance lifecycles.

These are precisely bounded assertions. In particular, the finite/magnitude test is not a calibrated hardware headroom measurement; energy-based output checks do not establish timbral equivalence; editor lifecycle tests do not establish visual quality or mouse functionality.

The latest development timing measurement rendered 30 seconds in about 2.777 seconds of callback time, roughly 9.26% of realtime on this machine, with a measured maximum 128-sample callback of 1.879 ms at 48 kHz. This was an offline single-instance test including scheduling outliers, not a guaranteed Ableton CPU budget.

## Fresh DSP, processor and calibration regressions

`artifacts/Validation/DSPTests-fresh.txt` ends **83 passed, 0 failed**. It covers oscillator pitch/ranges and sub-divider modes at five rates; low-note priority and duplicate-note counts; ADSR timing/release and legato; external detector tap and release; filter/VCA processing and self-oscillation; alias rejection; bounded stress and nonlinear-solver convergence; portamento and LFO behavior; deterministic condition behavior; noise level across rates; and non-finite input/control rejection. All five stress-rate runs reported zero solver failures. The measured external impulse peak was at sample 130, including the modeled filter phase delay beyond fixed latency 128.

The coherent high-saw regression measured residual alias energy at **-57.8183 dB**, versus **-6.81064 dB** for that test's naive reference. This is one defined numerical comparison, not an all-pitches alias ceiling, a THD specification or a hardware measurement. A separate 15-second high-resonance core render completed without numerical failure. These core tests exercise the internal DSP directly; the VST3 host tests independently cover its compiled wrapper.

`artifacts/Validation/ProcessorTests-fresh.txt` records **7 passing assertions, 0 failures**, including keyboard-queue overflow recovery, GUI/MIDI duplicate-note display counts, preservation of a still-held MIDI note when the GUI releases its duplicate, and a zero-sample panic flush. The instrumented normal callback path observed **0 C++ `new`/`new[]` allocations**, including 500 blocks of note traffic and GUI-queue events. The test instruments those allocation operators on the calling thread; it does not instrument every allocator, aligned allocation, OS service, external host or VST3 wrapper, and it is not a universal real-time certification.

`artifacts/Validation/Render-fresh.txt` records **35 seconds across seven generated WAV examples**, with **0 solver failures** for each: bass, resonant lead, random modulation, self-oscillation, external follower, condition and an external-routing stimulus. Files are under `artifacts/Audio`. The renderer took 2.64311 seconds including stimulus and filesystem work; that timing is distinct from the actual-module callback benchmark below. Successful rendering does not constitute a listening comparison against hardware.

The calibration-tool self-test passed frequency, RMS, harmonic, transfer-gain and PCM round-trip checks (`artifacts/Validation/calibration-tests.txt`). Generated calibration inputs at 48 kHz include 220 Hz bursts, a 20 Hz–18 kHz logarithmic sweep and seeded noise under `artifacts/Calibration/Stimuli`. The example analysis under `artifacts/Calibration/GeneratedBass` contains JSON metrics and a spectrum CSV for a generated bass excerpt. These demonstrate that the analysis pipeline operates; no imported physical SH-09 recording or measured reference-unit calibration is claimed.

## Bus metadata verified in the real module

Steinberg's Scan Buses test reported:

| Bus | Index | Name | Reported classification |
|---|---:|---|---|
| Audio input | 0 | External Input | Aux, default active |
| Audio output | 0 | Output | Main, default active |
| Event input | 0 | MIDI Input | Main, default active |

Its Bus Activation test also identified input audio bus 0 as `kAux`. This verifies the compiled wrapper's metadata; it does not by itself verify Live's routing controls or saved-project behavior.

The final integration host verified `getLatencySamples() == 128` after preparation at all 35 sample-rate/block-size combinations. Its fresh log prints `latency=128` beside every matrix result. Pluginval's pre-preparation cache value is not a measurement of signal delay. The filter's frequency-dependent group delay also contributes to a measured impulse peak beyond the fixed oversampling latency.

## Fresh Release rerun — final results

Final tested module: `C:\Dev\SH-09\build\SH09_artefacts\Release\VST3\SH-09.vst3\Contents\x86_64-win\SH-09.vst3`.

SHA-256: `188CD6EACAD347120DF4A339C6858BEE8D200DEDE3BF10F6D999A447BCE13EE1`, rechecked after tests and preset export.

| Final test | Actual result | Evidence |
|---|---|---|
| Clean Release x64 build | **Succeeded; no warning/error lines found** | `C:\Dev\SH-09\artifacts\Validation\fresh-build.txt` |
| Direct DSP regressions | **83 passed, 0 failed** | `artifacts/Validation/DSPTests-fresh.txt` |
| Direct processor regressions | **7 passed, 0 failed; 0 tracked C++ allocations** | `artifacts/Validation/ProcessorTests-fresh.txt` |
| Tracktion pluginval 1.0.4, strictness 10 | **SUCCESS** | `artifacts/Validation/sh09-pluginval-fresh.txt` |
| Expanded JUCE VST3 integration host | **75 assertions passed, 0 failures; exit 0** | `artifacts/Validation/sh09-host-fresh.txt` |
| Steinberg VST 3.8.1 validator | **47 passed, 0 failed; exit 0** | `artifacts/Validation/sh09-steinberg-fresh.txt` |
| Fixed reported latency | **128 samples** at all 35 rate/block combinations | Per-combination lines in final host log |
| Sustained actual-module rendering | **600 seconds**, finite/nonzero output | Final host log |
| Factory preset disk export/reload | **16 of 16** `.sh09state` files exported and restored with program/parameter comparisons | `artifacts/Presets`, final host log |

The final pluginval run used seed **9009** (`0x2331`) and all 30 combinations of 44.1, 48, 88.2, 96 and 192 kHz with blocks of 1, 17, 64, 257, 1024 and 8192 samples. Its log records audio processing, non-releasing processing, state restoration, automation, editor creation/processing/automation, background-thread state, parameter thread safety, bus changes and parameter fuzzing completing with **SUCCESS**. Its optional embedded Steinberg invocation was explicitly skipped because no path was supplied; the separate Steinberg result above supplies that independent validation. GUI lifecycle tests ran, but visual appearance and user mouse actions still require actual UI review. The reported infinite tail is intentional for Hold/drone/self-oscillation operation; the integration benchmark uses a bounded rendering duration.

The final host repeats all development integration checks listed above. During its 600-second offline render, callback processing took **53.1918 seconds**, or **8.8653% of realtime**. Maximum measured 128-sample callback was **0.5205 ms**, compared with a **2.6667 ms** audio deadline at 48 kHz. This is a single observed run, not a CPU guarantee or an Ableton measurement.

The first run against the clean plug-in used a stale host executable because copying the updated host source preserved an earlier timestamp. Its lack of latency labels/preset output exposed that harness issue. Only `Tests/Host.cpp` was touched and rebuilt; the plug-in module was not changed. The final log above is the subsequent run that explicitly executed the new latency and disk-export assertions.

Actual GUI/Live observations and remaining acceptance limits are recorded in LIVE_TEST.md. Any later plug-in-code change requires relevant validation again.

Reproduction command after rebuilding the latest `Tests/Host.cpp` (choose the desired absolute export directory):

```powershell
& '.\build\SH09Host_artefacts\Release\SH09Host.exe' '.\build\SH09_artefacts\Release\VST3\SH-09.vst3' 600 --export-presets 'C:\Dev\SH-09\artifacts\Presets'
```

Optional `.sh09state` exports are serialized state for the JUCE-host restoration API and integration regression. They are not Steinberg `.vstpreset` files, and the instrument currently has no file-import UI for them. The 16-program bank embedded in the plug-in is the normal musical workflow.

## Ableton and fidelity acceptance

**Observed in Ableton Live 11.0:** standard-folder scan, MIDI-track loading, two instances, native panel/preset interaction, the auxiliary Sidechain controls, external audio from a monitored audio track, follower-meter activity before the EXT mixer, processed output in Hold without MIDI, mute-to-silence/unmute recovery, and transport start/stop. Closing Live and reopening the saved Set restored both instances, the External Follower patch and audio routing with processed output. See LIVE_TEST.md for exact setup, the saved Set path, a save-dialog path issue and unperformed cases. These UI/meter observations are distinct from the automated tests and do not establish measured sonic fidelity.

No physical SH-09 has been measured for this project. Source-based DSP tests and validator passes cannot establish circuit accuracy, calibrated control-law accuracy or measured hardware equivalence. See the research and known-limitations documents for the evidence classification and remaining fidelity work.

Tool provenance and commands are in `tools/VALIDATORS.md`. The prerequisite disposable-plug-in checkpoint is in `VST3_SMOKE_TEST.md`.
