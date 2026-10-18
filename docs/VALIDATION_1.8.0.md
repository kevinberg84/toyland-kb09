# KB-09 1.8.0 validation — October 18, 2026

Production source: `1ba0b32fbec7b8781370c5150b565576f27f7c0a`. Manuals and footer fixture: `f63fe25dae2bb185226933f875b13f9a73012104`.
Later accepted CI commits change test waiting limits only; the production source is identical.

This release promotes the approved hardware appearance, panel proportions, radial
printing, caps, shadows, key materials and indicators into all production formats.
Windows native ASIO is enabled. DirectSound and WASAPI remain available. The sound
engine matches 1.7.2; saved parameter identities, presets, 24 factory sounds and
128-sample processing latency are retained. ASIO does not establish a universal
fix for intermittent clicking. Local audio/MIDI capture diagnostics are excluded.

Windows passed all 11 CTest suites: ten initially and the standalone mapping suite
after correcting its test crop for the approved 26-unit panel reduction. That
targeted rerun passed 174 checks with zero tracked audio-thread allocations.
Production editor rendering passed 881 checks, Steinberg VST3 passed 47 checks,
the actual VST3 host passed 77 assertions, and GUI-enabled pluginval strictness 10
exited 0. All 63 dialog states passed. Eleven actual-desktop RATE lamp checks
passed, including blocked UI work, AGE changes, notes, transparent corners,
resize, minimize/restore and editor closure. No square artifact was detected.

[Native Mac validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/39112037477) passed its native suites, editor/dialog
checks and Apple AU validation. Standalone, AU and VST3 contain arm64 and x86-64
and identify 1.8.0; the final package preserves the validated signed binary bytes.
Signing is ad-hoc, not Apple notarization.

[Native Linux validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/39112689646) passed native suites, editor/dialog
checks and dependency checks. The asynchronous material-bank test allows a longer
warmup on a software-rendered CI runner; the responsive fallback is checked
separately. Physical Arch playback and MIDI remain UNVERIFIED, and the package
remains standalone only.

The manuals contain five matching production-rendered AGE illustrations, the
Live 12 routing link and the free-forever sharing message. Platform editions are
self-contained, with no bundled PDF. Existing 8 GB standalone / 16 GB DAW RAM
recommendations are retained. Windows observations do not certify minimum
hardware requirements or Mac/Linux performance.

All three permanent Drive ZIP file IDs, names and sharing are retained. Downloads
were verified against staged SHA256 hashes, embedded versions, CRC and 24 presets.
The installed Windows module, Desktop Host folders and desktop manual are updated;
prior releases, user settings and the running diagnostic TS are preserved.

Evidence: `artifacts/Release180/validation`.
