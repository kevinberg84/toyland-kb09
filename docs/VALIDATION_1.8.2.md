# KB-09 1.8.2 validation — October 18, 2026

Production source: `acdaedc00c1413fae2f5f315521289b13e842cc8`.

Includes the approved centered panel, uniform short ticks and border-facing values,
aligned RATE LED and cap shoulder glimmers. Four physical controls use squared
response: PORTAMENTO, VCO MOD, VCF MOD and VCF ENV/FOL’R depth. Existing presets
and DAW parameter automation retain stored effect amounts and parameter IDs.
Recorded MIDI CC performances intentionally follow the new response. Full ranges,
the DSP engine, approved POWER mechanics/timing and independent RATE presenter
are unchanged. Production excludes local audio/MIDI capture diagnostics.

Two obsolete linear CC1 expectations in the VST3 host test were updated to the
approved squared response. The original failure log is retained; production
code and binaries did not change. Native builds use the identical production
source at f66eb313d7d6e1458802fdc543d45255e57a6e34.

Windows passed all 13 CTest suites, 881 GUI checks, 47 POWER checks and 11 independent
RATE checks. Dialog preview passed 63 states. Steinberg VST3 validator passed 47
checks; actual VST3 host passed 77 assertions; GUI-enabled pluginval strictness 10
exited 0. Protected file hashes are recorded in preservation.json.

[Mac native validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/39164910276) and
[Linux native validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/39164910408) passed all 13 suites plus GUI and
dialog checks. Mac AU validation passed; standalone, AU and VST3 contain arm64 and
x86-64. Validated signed binary bytes were retained; signing is ad-hoc, not Apple
notarization. Intel hardware execution remains untested here. Linux retains ALSA
and adds JACK; actual Arch audio/MIDI hardware testing remains UNVERIFIED.

All three manuals identify 1.8.2 and embed current approved GUI illustrations.
The sound-engine story, original filter figures, free-forever wording and requested
reader aside remain. Squared response, MIDI compatibility and Linux JACK guidance
are included. Each package contains 24 factory sounds and its self-contained manual.

All permanent Drive IDs and sharing are preserved. Uploaded current ZIPs were
redownloaded and SHA256/CRC checked before publishing the 1.8.2 update feed.
Windows installation, Desktop Host folders and desktop manual match the release.
Only the requested TS executables, shortcut and old test builds were removed;
source, automated tests, presets, preferences, previews and logs were preserved.

Evidence: `artifacts/Release182/validation`.
