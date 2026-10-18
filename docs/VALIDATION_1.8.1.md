# KB-09 1.8.1 validation — October 18, 2026

Production source: `201e982a1e9e5834c5acdeac05e23c99f4991b67`. Accepted test-only fixture corrections: 3adfafa68bb31bd825580999f244db4910aa5b2e.

Includes the approved screw positions, feathered and repositioned key cracks,
shorter exposed posts, and corrected POWER timing. The cap stays fully depressed
while held and springs back on release using the released 12 ms press / 20 ms
return equations. Audio turns ON at press and OFF at release; the full release
animation/repaint/idle-deferral path is retained. Regression coverage includes
47 event and spring-curve checks. The sound engine and preset format are unchanged
from 1.8.0. Parameter identities, 24 factory sounds and 128-sample latency remain.
Production excludes local audio/MIDI capture diagnostics.

Windows passed all 12 CTest suites in one complete run. Full native editor rendering
passed 881 checks after correcting the post-shape test for the approved shorter
posts. The original failing logs are retained. No production geometry was changed
to satisfy that fixture. Steinberg VST3 validation passed 47 checks; the actual VST3
host passed 77 assertions; GUI-enabled pluginval strictness 10 exited 0. All 63
dialog states and 11 independent RATE-light desktop checks passed.

[Native Mac validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/39149156647) passed all 12 suites, GUI/dialog checks and
Apple AU validation. Standalone, AU and VST3 contain arm64 and x86-64; their validated
signed binary bytes are preserved in the final package. Signing remains ad-hoc,
not Apple notarization; Intel execution and individual host/device setups need
user evaluation.

[Native Linux validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/39149154862) passed all 12 suites, GUI/dialog and
dependency checks. The package remains standalone only and UNVERIFIED pending
physical Arch audio/MIDI testing. Automated desktop checks do not establish that.

All three platform manuals identify 1.8.1. Section 02, "How we built the sound
engine," retains the earlier filter explanation and waveform figures, explains
the modeling/calibration process and its limits, and includes the requested
reader aside. The opening states free for everyone forever; redundant filter
navigation is removed. All five AGE illustrations and the MIDI-map illustration
match the approved GUI. Each package and installed help use the same self-contained
platform manual. No PDF is bundled. The unchanged sound engine/resource profile
does not justify raising the existing 8 GB standalone / 16 GB DAW recommendations;
these are not certified minimum-hardware requirements.

All three permanent Drive ZIP IDs and sharing remain unchanged. Uploaded current
versions were downloaded and SHA256/CRC/version checked before the public update
feed changed. Installed Windows files and all three Desktop Host folders match
the final packages. Previous released versions and user presets/settings are
preserved. Only the TS executable copies and test shortcuts were removed after
publication, as requested; previews, logs and local captures remain.

Evidence: `artifacts/Release181/validation`.
