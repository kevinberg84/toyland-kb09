# KB-09 1.9.0 release validation

Binary source: `4a155170158ea945ed6de9b1738055b695500a6e` on `codex/sh09-emulation`. The runtime implementation is commit `28314180e0fbbd275e7f1d4e738aa3a379a7c9bd`; the later binary-source commit corrects historical release-note encoding. Publication adds documentation and diagnostic-workflow records without changing the tested production source, tests, assets or CMake configuration.

## Completed checks

| Area | Result |
| --- | --- |
| Windows x64 | 15/15 core suites, including processor, DSP, presets, POWER, RATE, startup-dialog reservations, input memory and update-link selection |
| Windows rendering | 881 GUI checks; 63 actual dialog/menu states plus all 17 channel choices and three small settings-dialog layouts |
| Windows standalone MIDI | 219 checks, zero failures and zero audio-thread C++ new allocations |
| Windows VST3 | 47 Steinberg checks; pluginval strictness 10 with GUI enabled; 77 actual host assertions plus 8 hosted RATE assertions |
| Windows installation | Fresh and upgrade installs in an isolated TestRoot; expected binaries/license installed, SHA256 matched and sentinel user file preserved |
| Universal Mac | Native arm64 runner: 15/15 core suites, repeated MIDI interleaving tests, 881 GUI checks and 63 dialog states; actual hosted RATE checks in AU and VST3 |
| Mac formats | Standalone, AU and VST3 contain arm64 and x86_64; ad-hoc signatures verified; Apple AU validation succeeded |
| Linux x86-64 | Native build: 15/15 core suites, 881 GUI checks and 63 dialog states; ELF/shared libraries and executable permissions checked |
| Documentation and packages | All three OS-specific manuals and OPEN ME FIRST! guides; local references/anchors, ZIP CRC, 24 presets, applicable license notices and package SHA256 |

Native evidence: [Mac run 39185447069](https://github.com/kevinberg84/toyland-kb09/actions/runs/39185447069) and [Linux run 39185447028](https://github.com/kevinberg84/toyland-kb09/actions/runs/39185447028). Windows logs, package manifests and preservation hashes are retained in the local release-validation archive. Installation smoke tests did not replace user preferences or exercise every real system configuration.

## Confirmed user observation

The corrected Windows VST was reported working in Ableton Live. Hosted regressions cover window restacking and idle audio processing. This is bounded evidence, not a guarantee for every DAW or system.

## Reported Linux desktop issue

A report from Arch Linux with KDE, XWayland and PipeWire described the update and shortcut questions opening together, followed by a temporary desktop freeze. The GPU/driver and Plasma version were not established.

A real startup-dialog race was corrected: a shared reservation is acquired before asynchronous dialog creation and held until dismissal. The second question and update results wait for it. Both launch orders, queued-before-visible requests and unrelated modal windows are covered by 16 regression checks. Whether this resolves the reported desktop freeze has not yet been confirmed on a physical system.

The JUCE Linux native-titlebar maximize path uses standard window-manager maximize states, not Windows fullscreen APIs or a monitor resolution change. Headless KDE diagnostics did not reproduce the complete reported scenario: the virtual compositor failed before an independent X11 clock, and a nested X11 test explicitly reported missing DRI3 1.2 support in Xvfb. Those failures describe the diagnostic environment, not proof of a defective GPU driver on a user's machine.

**Linux physical desktop, audio and MIDI remain UNVERIFIED.** Keep this label on the Linux download until real hardware confirmation.

## Preservation and other limits

The DSP engine equations, parameter identities, preset compatibility and processing latency are retained. Raw hashes confirm no changes to Engine.cpp, PowerButtonTravel.h or InsetRedLamp.h. Approved POWER mechanics/timing and LED artwork remain intact; production audio/MIDI capture diagnostics are disabled.

Mac Intel slices were built and architecture-checked, but were not executed on a physical Intel Mac. Mac binaries are ad-hoc signed, not Apple-notarized. Hardware SH-09 equivalence is not established. No automatic diagnostic uploads are performed.

## Distribution and support

GitHub Releases hosts the 1.9.0 platform ZIPs. The manual links to GitHub Issues and explains that reports are public and require a free account. Downloading releases does not require an account.

The public 1.9.0 release has three platform ZIPs, each downloaded without authentication and verified against its full SHA256. The update feed now uses the trusted GitHub release URL in `releasePage` and every platform field. Users of Drive-only old builds need a manual upgrade from GitHub; further Drive maintenance is retired. Old checkers cannot consume the new GitHub URLs. Existing Drive files were left untouched.

First-party material is source available under the KB-09 Free Use and No Sale License; dependency licenses remain separate. See [licensing scope](LICENSING.md).
