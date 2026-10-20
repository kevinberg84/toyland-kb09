> **Historical record.** Version numbers, proposals, pending statuses and test results below refer to the revision/date described, not the current release. See [1.9.0 validation](VALIDATION_1.9.0.md) and [current sound-model evidence](FIDELITY.md).

# TS 283 validation — squared controls and Linux JACK

Source commit: `7f648a537951995e2e92067c0a3c474a49260a6f` on `codex/sh09-emulation`.

The desktop Test Standalone shortcut points to `artifacts/DesktopTestStandalone/Windows/CurvesJack283/Toyland KB-09 test standalone.exe`. Its SHA256 is `0814BCA727451AF2B4C3F70101F2CE25E94F6473215ABA829F9BE8B3B66FE4B6`. The build was installed and opened; normal settings were preserved during the isolated startup probe. ASIO, WASAPI and DirectSound remain enumerated.

PORTAMENTO, VCO MOD, VCF MOD and VCF ENV/FOL'R use squared physical-position mappings in GUI and learned MIDI/CC1. Stored effect amounts, parameter IDs/ranges, presets and DAW parameter automation retain their meaning. Existing recorded MIDI CC performances intentionally use the new response. Numeric readouts, typed entry and inverse-position pickup are covered by the new tests. Engine.cpp and the protected power-button travel and independent LED implementations are unchanged.

Windows: all 13 CTest suites passed, including 55 control-response checks. Four pre-existing regression executables were initially absent from the development directory; they were built and the four Not Run suites then passed. The other nine suites had already passed. Full actual GUI rendering passed 881 checks, power-button checks passed 47 and native independent RATE LED stress checks passed 11. Evidence is in `artifacts/CurvesJack283/`.

Linux: [native verification run 39162963362](https://github.com/kevinberg84/toyland-kb09/actions/runs/39162963362) completed successfully at the same source commit. All 13 suites passed, including checks that the JACK and ALSA backend factories are present; all 881 editor checks and 63 dialog states passed. Linux compile definitions enable JUCE_JACK=1 and retain JUCE_ALSA=1. JACK also provides the integration route for PipeWire's JACK-compatible server/client libraries. This does not constitute physical Linux audio-device testing; that remains unverified. JACK is a Linux backend in this build and is not added to Windows' audio-device list.

No public release, host-folder deployment, installed production VST3 update or update-feed publication was performed. Before the next public release, audition the new response, refresh the manual GUI captures/control-response text, and run the Mac/platform release checks on the final approved revision.
