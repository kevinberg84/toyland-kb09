# KB-09 1.6.2 validation — October 14, 2026

Release source: `040a72facdf8442feaa60d1f705e2c04f7d1416d`. GUI and documentation patch. Audio engine and
plug-in source compare identically with v1.6.1 after normalizing line endings.
Parameter identities, MIDI behavior, 24 factory presets and the 128-sample
reported processing latency are retained.

Windows passed all ten CTest suites, 863 actual editor checks, 160 standalone
MIDI checks, 63 dialog/menu states, three audio-settings layouts, 47 Steinberg
VST3 checks, 77 actual VST3 host assertions and pluginval strictness 10.

Native Linux run [38614123330](https://github.com/kevinberg84/toyland-kb09/actions/runs/38614123330)
passed ten suites, 863 editor checks, 63 dialog/menu states and dependency checks.
Real Arch audio/MIDI playback remains UNVERIFIED.

Native Mac run [38614168834](https://github.com/kevinberg84/toyland-kb09/actions/runs/38614168834)
passed ten suites, three repeated MIDI race checks, 863 editor checks,
63 dialog/menu states and Apple AU validation. The three universal bundles
contain arm64 and x86-64, identify 1.6.2 and retain their validated signed bytes.

The rendered-meter regression has 51 checks, covering small release tails,
segment-boundary transitions, MIDI note-off and zero-velocity note-on, mouse
notes, 48/96 kHz and 64/256-sample buffers. The former code failed 13 checks;
the corrected code passed all 51. The compiled RATE tooltip was rendered and
visually inspected without character artifacts.

All three packages passed CRC, 24-preset counts and embedded manual/link checks.
Both requested troubleshooting deletions are absent from each platform edition.
Drive uploads retained the existing folder/file identities and sharing; the
current downloads matched the staged SHA256. The Desktop Host still has exactly
three platform folders, with 130 package files hash-verified. Installed Windows
files and current local downloads were updated; earlier releases were archived.

The current standalone test, desktop shortcut and associated folder are deleted.
Two older DSP test DLLs remain mapped by exited Live processes and retain their
previously authorized next-sign-in cleanup. User recordings, personal presets,
source and regression tests are retained.

Release proofs: `artifacts/Release162/validation`.
