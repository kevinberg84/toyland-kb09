# KB-09 1.7.0 validation — October 15, 2026

Binary source: `4827a939c674fd35643113ba572af7a5d5955454`. Documentation illustrations: `20e19c0fdfbd052d430d9b589dd1eac4749212d7`.
This backward-compatible feature release adds standalone MIDI-input memory and
arrow-key dialog selection, with the approved hardware rendering shared by
standalone and plug-ins. The AUTHENTIC onscreen bender return is linear over
50 ms, stopping at center without easing. No recoil is implemented.

The DSP engine, parameter identities, preset format, 24 factory sounds and
128-sample reported processing latency are retained. Regression tests cover
linear bender position, audible pitch, interruptions and allocation-free audio
at 44.1/48/96 kHz with block sizes 1/127/512.

Windows passed all 11 CTest suites, 875 compiled-editor checks, 174 standalone
MIDI checks, 16 input-selection persistence/reconnection checks, 63 dialog/menu
states, three audio-settings layouts, 47 Steinberg VST3 checks, 77 actual VST3
host assertions, and pluginval strictness 10 with confirmed exit 0.

[Native Linux validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/38688355117) passed all 11 suites, 875 editor
checks, dialog checks and shared-library dependency checks. Real Arch audio
and MIDI playback remain UNVERIFIED.

[Native Mac validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/38688563474) passed all 11 suites, three repeated
MIDI interleaving checks, 875 editor checks, dialog checks and Apple AU validation.
Standalone, AU and VST3 bundles contain arm64 and x86-64, identify 1.7.0, and
retain the exact validated ad-hoc-signed bundle bytes. They are not notarized.

All three packages passed CRC, 24-preset counts, self-contained OS-specific
manual/link checks and version checks. Drive updates preserved the permanent
folder and three file IDs and their existing sharing. All three downloaded
Drive ZIPs matched the staged SHA256 exactly. The existing Desktop Host still
contains exactly three platform folders, with 130 package-file hashes verified.
Installed Windows VST3 and current local downloads were updated. Prior released
versions are preserved outside the current folders.

The 1.7.0 public update notice is published and verified over HTTPS with status
200. The test standalone, desktop shortcut, isolated preferences, all packaged
hardware test iterations, private test build and temporary preview content were
removed. Source, regression tests, personal presets, recordings and release
validation records are retained.

Evidence: `artifacts/Release170/validation`.
