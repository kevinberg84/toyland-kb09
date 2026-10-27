# KB-09 1.10.0 release validation

Production source: `3ff3e060ef68e064825e39d848bb773895d5610f`. Mac native CI also includes test-only revision `37356a7dbc7cd4fb5a2087ac5a6573ab7092a8ba`: it waits for the deferred AGE commit before comparing cheek screenshots, retaining the same artwork and pixel threshold. The initial Mac run exposed a fixed-delay test race; the corrected fixture was then rerun. The publication commit adds final documentation and website cache refreshes without changing the tested synth source, assets or CMake configuration.

## Completed checks

| Area | Result |
| --- | --- |
| Windows x64 | 15/15 core suites, including DSP, processor, presets, POWER, RATE, startup-dialog reservations, input memory and update handling |
| Windows rendering | 881 GUI checks; 66 actual dialog/menu states and three device-settings layouts |
| Windows standalone MIDI | 219 checks; zero failures and zero audio-thread C++ new allocations |
| Windows VST3 | 47 Steinberg checks; pluginval strictness 10 with GUI enabled at 44.1, 48 and 96 kHz and blocks of 17, 128 and 512; 77 actual host assertions plus 8 hosted RATE assertions |
| Windows installation | Fresh and upgrade installations in an isolated TestRoot; binary hashes matched and a sentinel user file was retained |
| Universal Mac | Native arm64 runner: 15/15 core suites, repeated MIDI interleaving checks, 881 GUI checks and 66 dialog/menu states; actual hosted RATE checks in AU and VST3 |
| Mac formats | Standalone, AU and VST3 contain arm64 and x86_64; ad-hoc signatures verified; Apple AU validation succeeded |
| Linux x86-64 | Native build: 15/15 core suites, 881 GUI checks and 66 dialog/menu states; ELF/shared libraries and executable permissions checked |
| Documentation and packages | Three OS-specific manuals and OPEN ME FIRST! guides; local references and anchors, ZIP CRC, 24 presets, applicable license notices and package SHA-256 checked |
| Website | 70 local file/anchor references and 14 manual resources verified; desktop and mobile demo controls, seeking, mute and the played/remaining-only timeline checked |
| Website analytics | GoatCounter received labelled prelaunch page-view and download-click tests; six production/local/privacy guard cases passed; ordinary local previews do not send statistics |

Native evidence: [Mac run 40452801951](https://github.com/kevinberg84/toyland-kb09/actions/runs/40452801951) and [Linux run 40449096932](https://github.com/kevinberg84/toyland-kb09/actions/runs/40449096932). Windows logs, manifests and preservation hashes are retained in the local release-validation archive.

## Preset-load correction

A successful LOAD previously reached the error-notification branch because that branch also handled successful loads that were not saves. The completion path now displays the warning only when the result failed. Regression coverage exercises selected-file loading through the same completion and notification path, checks successful state restoration without a warning, and checks that a missing file warns while preserving the current patch. The shared implementation is used by standalone and plug-in formats.

## Preservation and remaining limits

Normalized source hashes confirm that Engine.cpp, PowerButtonTravel.h and InsetRedLamp.h are unchanged from the previous release. Approved POWER press/hold/release motion and audio timing, independent RATE rendering, parameter identities, preset compatibility and processing latency are retained. Production audio/MIDI capture diagnostics remain disabled. Tests passing do not establish every DAW, device or desktop configuration.

Linux standalone operation was reported on Arch/KDE with XWayland and PipeWire. Startup update and shortcut prompts are serialized to prevent overlapping dialogs; this correction is treated as addressed. Automated regression coverage checks both prompt orders and queued-before-visible requests.

Mac Intel slices were built and architecture-checked, but were not executed on a physical Intel Mac. Mac binaries remain ad-hoc signed and are not Apple-notarized. Repackaging retained signed binary bytes and executable permissions. Hardware SH-09 equivalence is not established; see [sound-model evidence and limits](FIDELITY.md).

## Updates and distribution

GitHub Releases hosts the three platform ZIPs. The update feed is advanced only after their public downloads match the validated full SHA-256 values. Existing 1.9.0 installations retain their original generic update prompt for this upgrade; 1.10.0 can display the optional release highlights on subsequent updates. Installation remains manual and retains normal presets and audio/MIDI preferences.

Website analytics count visits and download-button clicks, not completed downloads. The dashboard is private. The visible footer explains the collection; there are no analytics prompts or counters inside the synth. Labelled verification traffic is separate from organic interest.

First-party material is source available under the KB-09 Free Use and No Sale License; dependency licenses remain separate. See [licensing scope](LICENSING.md).
