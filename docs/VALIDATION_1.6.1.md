# KB-09 1.6.1 validation — October 14, 2026

Release source: `d33fe0dd142f401f64bffe561819f0d32479bc1f`. This is a GUI and documentation patch. Audio and
plug-in source files compare identically with v1.6.0 after normalizing line
endings. Parameter identities, MIDI behavior, 24 factory presets and the
128-sample reported processing latency are retained.

Windows passed all nine CTest suites, 811 actual editor checks, 160 standalone
MIDI checks (including rendered-pixel footer spacing in standalone and VST3),
63 dialog/menu states, three audio-settings layouts, 47 Steinberg VST3 checks,
77 actual VST3 host assertions and pluginval strictness 10.

Native Linux run [38597320280](https://github.com/kevinberg84/toyland-kb09/actions/runs/38597320280)
passed nine suites, the 811 editor checks, 63 dialog/menu states and dependency
checks. Linux remains UNVERIFIED pending real Arch desktop audio/MIDI playback.

Native Mac run [38597315249](https://github.com/kevinberg84/toyland-kb09/actions/runs/38597315249)
passed nine suites, three repeated MIDI race checks, 811 editor checks,
63 dialog/menu states and Apple AU validation. All three universal bundles
contain arm64 and x86-64 and identify 1.6.1. Final packaging preserves the
validated signed binary bytes. The installer banner identifies 1.6.1.

All three archives passed CRC, 24-preset counts and embedded manual/link checks.
OS-specific content audits found no foreign-OS setup instructions. The manual's
Next AGE control was exercised through all five conditions and back to Minty.
Its images work offline; waveform figures are ordinary images rather than links.

Uploads replaced the permanent Drive file versions, retaining their file and
folder identities and sharing. Each current archive was downloaded again and
matched the staged SHA256. The Desktop Host contains exactly three platform
folders, with 130 package files hash-verified. Windows installation was
updated, and older releases were preserved outside the current Host folders.

Release proof files and checksums are in `artifacts/Release161/validation`.
