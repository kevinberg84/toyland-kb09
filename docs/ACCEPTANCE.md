# Final acceptance checklist — 0.9.0 candidate

This records the requested checklist against observed evidence. PASS means the bounded test below passed; it does not imply a measured hardware match. PARTIAL items remain limitations.

| Requirement | Status and evidence |
|---|---|
| Clean x64 Release build; no critical warnings; VST3 artifact exists | PASS: fresh configure/build, no warning/error lines, preserved build log |
| Runtime independent of project sources | PASS for inspected dependencies and installed Live loading: compiled panel, no external DSP/UI asset files, release CRT imports |
| MIDI sound; monophony; low-note priority; oscillator range; sub oscillator | PASS numerical/actual-module tests; hardware timbre equivalence remains unverified |
| LFO; ADSR; portamento; VCA | PASS scoped DSP and host tests |
| Filter stability; resonance; self-oscillation | PASS finite/bounded stress, zero solver failures in tested rates and sustained render; all possible inputs are not proven |
| External audio and envelope follower | PASS core/actual-module assertions plus Live auxiliary audio route, detector meter, Hold output and mute recovery |
| VST3 validator; test-host instantiation | PASS Steinberg 47/0, pluginval strictness 10, actual-module host 75/0 |
| State/presets save and restore | PASS all parameters and 16 exported program states; Live Set reopened with patch and routing restored |
| Automation; multiple sample rates/block sizes | PASS automated host/pluginval matrices, including parameter stress and GUI lifecycle; Live automation-lane testing remains unperformed |
| Every GUI control; keyboard; MIDI keyboard display; Age appearance | PARTIAL: attachments/source reviewed and event/count regressions passed; visual all-control, held-key display and Age-sweep acceptance not completed |
| Labels and mouse interaction | PASS scoped Live inspection, factory menu and cutoff drag; not an exhaustive layout/input matrix |
| Resizing and high DPI | PARTIAL: automated editor resize/lifecycle tests and local Live display worked; no full DPI/monitor matrix |
| No known crash or hang | PASS in recorded tests; no unresolved crash/hang identified |
| No NaNs, infinities or uncontrolled filter runaway | PASS recorded finite-output, stress and endurance assertions; bounded test conditions only |
| No normal render-path heap allocation | PASS tracked C++ new/new[] path regression; broader allocator instrumentation not performed |
| No blocking audio-thread locks | PASS source review of owned callback path; no universal host/library real-time claim |
| Presets load; state restoration; clean build; final validation rerun | PASS for frozen plug-in binary identified by SHA-256 in VALIDATION.md |
| Documentation matches behavior | PASS final independent documentation audit; retained/bypassed Modern settings and observed Live scope corrected |
| Photorealistic skin; measured high-fidelity hardware emulation | NOT COMPLETE: vector interpretation and circuit-informed model with the explicit gaps in KNOWN_LIMITATIONS.md |

The project is delivered as a tested release candidate. The unchecked fidelity and broad GUI cases are not silently marked complete.
