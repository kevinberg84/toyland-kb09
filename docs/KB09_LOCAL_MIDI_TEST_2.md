# Local Windows DSP Test 2 — MIDI feedback

The MPK249 modulation wheel worked in the standalone but behaved poorly in Live. The physical controller was also checked against a Live instrument. Direct Live/MPK testing remains Kevin's acceptance check; the automated reproduction exercises the processor and the actual VST3 module.

## Reproduction and correction

The previous feedback code classified parameter writes by comparing their value to the newest wheel target. If another MIDI event arrived while the message thread published an older value, that legitimate publication could be misclassified as a manual edit and re-arm pickup. A delayed host parameter write or a first CC1 message above zero could also leave a VST3 wheel waiting for a crossing that never occurred.

The new regression failed four of five cases against the previous code and passes all five after correction. The processor now distinguishes its own synchronous feedback callbacks from external edits using a thread-local origin and an atomic edit revision. MIDI feedback uses a coherent, bounded atomic mailbox. Genuine edits and preset changes invalidate queued feedback. Parameter listeners and host notifications remain off the audio sample loop; the existing processor allocation checks pass with zero audio-thread allocations.

VST3 treats the next CC1 event as an absolute wheel movement, including after a host edit or preset change. Standalone soft pickup remains intact and passes the existing Authentic/Modern tests. Other CCs retain their DAW-assigned roles rather than receiving additional built-in mappings.

## Validation

- Eight CTest suites passed.
- Actual Test 2 VST3: 77 assertions passed, including 34 sound controls plus Bypass and Program at five legal positions; CC1 up/down sweeps and takeover after host automation; ordinary CCs on all 16 channels; state, MIDI timing, editor lifecycle, finite output and latency checks.
- The host harness dispatches platform messages to exercise the loaded DLL's timers, rather than just calling the host executable's separate JUCE Timer singleton.
- Seven packaged listening presets load/save/reload successfully: 82 preset assertions passed.
- Engine.cpp, Engine.h and ResonanceModel.h hashes match delivered Test 1. Reported latency remains 128 samples.

## Delivery and limits

Local folder: `output/KB-09 Local DSP Test 2/Windows`. Product name is **Toyland KB-09 DSP Test 2**, version 1.5.3, distinct VST3 code `Sh0U`. Test 1 and the public installed plug-in remain intact. No GitHub commit/push, Google Drive upload, update-feed publication, Mac build or Linux build was performed.

Automatic approval review rejected installing Test 2 into the shared system VST3 folder as an unrequested system-level change. **No Test 2 installation occurred.** Use the supplied VST3 bundle with an authorized custom scan location, or obtain explicit authorization before attempting system installation. Do not work around the rejection by silently installing elsewhere.

The test manual uses the requested dark reading theme. The Windows, Mac and Linux opening-page sources are prepared with the same screen colors for the next release; print styles stay light. These document changes remain local.
