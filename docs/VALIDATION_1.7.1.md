# KB-09 1.7.1 validation — October 15, 2026

Approved production source: `e70321b094875fbbe8ffe727122a2f28e035da19`. Updated native regression tests: `ef08ce6a25768c9d7ae77439f2c8a594de998379`. Illustrated manuals: `a0efaf87fa52e63b77af4f51f55286425d9e7fa0`.

This patch release includes the approved panel spacing, labels, collar ticks,
hardware materials, performance-bay outlines, larger screws, centered MIDI MAP,
and consistent parameter cursors. AUTHENTIC bender return is linear over 35 ms,
then makes one visual-only recoil over 20 ms, capped at 2% of released travel.
Audio reaches neutral without overshoot. MODERN remains immediate. POWER
depresses while held and starts its 20 ms visual return on physical mouse release;
JUCE's synthetic pressed flash cannot restart it or rebuild the stationary chassis.

Windows passed all 11 CTest suites, 875 compiled-editor checks, 78 approved-control
checks, six collar/alignment checks, 63 dialog states, 47 Steinberg VST3 checks,
77 actual VST3 host assertions and GUI-enabled pluginval strictness 10 (exit 0).
All 25 production control exports match the approved desktop test pixels exactly.
Offscreen POWER release paints measured 11.26 and 10.84 ms without rebuilding
the stationary chassis; these are not physical-display latency measurements.

[Native Linux validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/38765422260) passed all 11 suites, 875 editor
checks, dialog checks and shared-library dependency checks. The first attempt
was blocked by GitHub billing; Kevin resolved it and the successful replacement
run produced the released binary. Real Arch audio/MIDI playback remains UNVERIFIED.

[Native Mac validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/38764997507) passed all 11 suites, repeated MIDI
checks, 875 editor checks, dialog checks and Apple AU validation, before Actions
artifact storage rejected the ZIP. Kevin explicitly approved private draft asset
transfer. The [transfer recovery build](https://github.com/kevinberg84/toyland-kb09/actions/runs/38768128808) verifies the prior checks
and identical binary source, assets, tests and build settings, rebuilds production
formats, and repeats factory-bank, architecture, signing and AU validation. Standalone,
AU and VST3 bundles contain arm64 and x86-64, identify 1.7.1, and retain the exact
validated ad-hoc-signed bundle bytes. They are not notarized.

DSP engine, parameter identities, preset format, 24 factory sounds and 128-sample
reported latency are retained. Regression coverage includes sample rates,
buffer sizes, interruption, actual audible return and zero tracked audio-thread
C++ new allocations. The two existing editor checks were updated for approved
slider centering and a fast return whose first observed callback can be recoil.

All packages passed CRC, 24-preset counts, self-contained OS-specific manual/link
checks, version checks and SHA256 verification after download from Google Drive.
The permanent folder and all three file IDs/sharing are retained. Desktop Host
still contains exactly three platform folders; 130 package file hashes matched.
The installed Windows VST3, current local platform folders and downloads are updated.
Previous released versions are preserved outside current folders.

The public update feed is published and verified over HTTPS (200, exact 1.7.1 JSON).
The desktop test standalone, shortcut, isolated settings, private build, temporary
preview content and helpers were removed. Source, regression tests, personal
presets, recordings and release evidence are retained.

Evidence: `artifacts/Release171/validation`.
