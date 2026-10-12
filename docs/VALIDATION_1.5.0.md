# Toyland KB-09 1.5.0 validation

Production source: `55b449ab433ca226d776c2eeb3f929004179811d`. Windows, Mac and Linux use this same revision. This release implements the approved five-stage GUI and updated manuals; the audio engine, parameter IDs and 24 factory presets are retained.

## Windows

- Six CTest suites passed. The compiled editor passed 795 checks; 63 actual dialog/menu states rendered, including device-settings layouts at 1800×1400, 1024×768 and 800×600.
- Steinberg VST3 validation passed 47 tests. GUI-enabled pluginval strictness 10 passed at 44.1/48/96 kHz and blocks 17/128/512, with confirmed exit 0.
- The actual VST3 host passed 73 assertions both before and after installation. Its 30-second audible, finite render used 2.60019 seconds of processing; peak observed callback was 0.3986 ms against a 2.66667 ms deadline. These are offline workstation measurements, not a displayed frame-rate or DAW CPU guarantee.
- Pixel comparisons against the approved revision-3 previews found unchanged caps, black keys and footer at every stage. Only Sold As-Is white-key scuffs changed, as requested: 11,716 pixels, mean absolute RGB difference 0.05861. Other white-key stages, including the maximum, match the approved previews.
- Installed VST3 module SHA-256: `80e88f89095ed3ef4551dfd854981e9ee31bdee6cda233fc7e8e0cbdbf1422f6`. Portable standalone SHA-256: `92661540d0cfe34d896094f564d7545deb34a3e06e14b169639ac639269bdecc`. Preceding files are backed up outside scan paths.

## Native builds

Mac run [38225417078](https://github.com/kevinberg84/toyland-kb09/actions/runs/38225417078) passed six CTest suites, 795 GUI checks, 63 dialog states and Apple AU validation. Standalone, AU and VST3 are universal arm64/x86-64; signature and architecture checks passed. Packaging preserves signed executable and bundle resource bytes. The bundles are ad-hoc signed, not Apple-notarized; Intel execution still needs user evaluation.

Linux run [38225417258](https://github.com/kevinberg84/toyland-kb09/actions/runs/38225417258) passed six CTest suites, 795 GUI checks, 63 dialog states under Xvfb/Openbox, and shared-library dependency checks. Packaging checks x86-64 ELF identity and executable launcher permissions. Linux remains UNVERIFIED until launch, playback and MIDI input are confirmed on a physical Arch desktop. The generic executable requires glibc 2.35 or newer.

## Distribution and documentation

All three final archives pass CRC, 24-preset counts, manual edition and offline-link/anchor checks. Source manual QA passed: 3 HTML documents, 99 IDs, 88 fragment links and 12 local references. Each package contains OPEN ME FIRST! and User Manual.html. The Desktop Host retains only the three platform folders; ZIPs and backups are outside it. Existing shared Drive folder and download file identities are retained by uploading new versions.

| Final ZIP | SHA-256 |
|---|---|
| Windows | `161afc9e77d27f54ceaeb279f34b1a8b228d4629ee88a0223bb520299ec75143` |
| Mac | `c100223f9e32f7d920bce23916fdb1d568da684a6908676950bbe61b56f07852` |
| Linux | `cae9db77911ab5350f79f1ffe9fc074eb6b9a49fc848544ef3d239954774f503` |

Evidence is in `artifacts/Release150/validation`, including native logs, archive manifests, installed hashes, approved-preview comparisons, and downloaded Drive package hashes. Update publication is gated on matching all three downloaded archive hashes.

No new sonic correction is claimed. Slow self-oscillation/VCO interactions remain under investigation as documented in `ANALOG_INVESTIGATION_2026_10.md`; recordings of the physical SH-09 are needed for calibration. Hardware equivalence, real Arch playback and live controller frame pacing are not established by these automated tests.
