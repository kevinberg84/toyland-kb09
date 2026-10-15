# KB-09 1.7.2 validation — October 15, 2026

Approved production source: `46386459e4e080be375209dce397f1a3a5654c81`. Illustrated manuals: `0c3150b18d2ff8e6639d7331b185ab3e5480c583`.

This release contains the approved photographic screws and gentler dark aging,
revised cheek scratches/scuffs and outer-edge gouges, raised cheek joints with
upper-panel dust, knob markings and uneven grime, unaged utility controls,
POWER/bender printing, ENV/GATE symbols and the close-prompt drag guard.
RANGE and WAVEFORM use instantaneous fixed-position switching. The attempted
40/25 ms animation is not shipped. AUTHENTIC bender return remains 35 ms plus
a 20 ms visual recoil; POWER starts its fast spring return on mouse release.

Windows passed all 11 CTest suites, 875 compiled-editor checks, eight additional
slow-drag detent checks, 45 utility-aging/close-prompt checks, 63 actual dialog states, 47 Steinberg VST3 checks,
77 actual VST3 host assertions and GUI-enabled pluginval strictness 10 (exit 0).
Prepared cheek materials and lazily generated screw materials reduce startup
work. Earlier native measurements improved first editor plus first paint from
approximately 2.87 s to 1.23 s on this machine. These exclude OS loading and
audio-device initialization and do not predict startup on every computer.

[Native Linux validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/38805104997) passed all 11 suites, editor and
dialog checks and shared-library dependency checks. Real Arch audio/MIDI
playback remains UNVERIFIED; the Linux download remains standalone only.

[Native Mac validation](https://github.com/kevinberg84/toyland-kb09/actions/runs/38805108397) passed all 11 suites, repeated MIDI
checks, editor/dialog checks and Apple AU validation. Standalone, AU and VST3
contain arm64 and x86-64, identify 1.7.2, and retain their exact validated
ad-hoc-signed binary bytes. They are not Apple-notarized. A private unpublished
draft asset transfers the native package without publishing a GitHub release.

The DSP engine, parameter identities and MIDI-map storage match v1.7.1.
Saved preset compatibility, 24 factory sounds and 128-sample latency are retained.
All packages passed CRC, preset counts, self-contained platform manual/link
checks, version checks and SHA256 verification after download from Google Drive.
All three manuals include fresh compiled-editor illustrations at all five ages.
The permanent Drive folder, file IDs and sharing are retained. The installed
Windows VST3, local release and Desktop Host packages are updated; prior releases,
personal presets and recordings are preserved.

The public update feed is verified over HTTPS (200, exact 1.7.2 JSON).
The desktop test standalone, shortcut, isolated settings, private build and
temporary hardware previews were removed. Source regression tests and release
validation evidence are retained.

Evidence: `artifacts/Release172/validation`.
