# Toyland KB-09 1.4.0 validation

## Windows

The final Windows standalone and VST3 use production source commit `a3cca60be705bad7735a63b49aad1a0eda1b77ec`. The six-suite Windows run used `7a49877c9e5ef557e2c049287aafcfe65e6f0cac`; the later revision changes only the printed pulse-width bracket and caption position. The final renderer, VST3 validator, pluginval and actual host checks use the final binary.

- Six CTest suites passed, including the preserved-engine comparison, processor, preset and update checks.
- The compiled GUI suite passed 613 checks. The dialog tool rendered 63 actual states, including three device-settings layouts.
- Steinberg VST3 validation passed 47 checks. GUI-enabled pluginval passed strictness 10 at 44.1/48/96 kHz and blocks 17/128/512, with confirmed process exit 0.
- The actual VST3 host passed 73 assertions and exited cleanly, both from the build and after installation. The 30-second build-module render used 2.648 seconds of callback time; the highest observed callback was 0.447 ms against a 2.667 ms block deadline. This is one offline workstation measurement, not a DAW performance guarantee.
- Moving-control offscreen rendering had a median of 6.8799 ms and p95 of 8.0873 ms, compared with 38.7613/43.9232 ms for the released 1.3.0 fixture. The chassis was not regenerated during movement. These numbers measure rendering work, not displayed frame rate.

The missing keyboard was reproduced by painting its actual component cache: six visibility checks failed before the correction. Clearing the cache had left its graphics opacity at zero, making key image blits transparent. The corrected cache restores opaque drawing and clips dirty areas in physical pixels; visibility and partial-invalidation checks pass at 1x, 1.5x and 2x display scales, at fresh and maximum AGE. Direct keyboard snapshots alone had bypassed this faulty path.

Rectangular cheek artifacts came from a fractional power of a tiny negative floating-point sine at a gouge endpoint. Clamping that profile input to zero removes invalid path coordinates. Fresh and aged compiled previews were inspected. Silver fader-cap strips retain photographed highlights and dull progressively without yellowing or rust; smaller waveform symbols, smaller NOISE and a pulse-width bracket traced from the supplied close-up improve the VCO layout. Collar marks now share the top index width and stop short of the collar rim; slider groups have shorter shared rules, closer tracks and rounded shaded openings.

The installed VST3 and portable Host-folder standalone match the tested binaries by SHA-256. Prior copies are preserved outside plug-in scan paths. Windows evidence is under `artifacts/Release140/validation`; compiled previews are under `artifacts/panel-final-bracket-140`.

## Audio boundaries

The restored 1.1.3 filter remains the shipping path, as in 1.3.0. No production filter or saturation revision was made for these GUI corrections. The slow self-oscillation/VCO interaction study and its limitations are recorded in `ANALOG_INVESTIGATION_2026_10.md`. Hardware SH-09 equivalence has not been established; recordings from the physical instrument are needed for calibration.

Modern TUNE intentionally expands stored nonzero settings to ±12 semitones. Authentic remains ±65 cents. Parameter identities and stored ranges are retained. Frequency checks cover both modes at 44.1/48/96 kHz.

## Native platform boundaries

Mac run [38204835139](https://github.com/kevinberg84/toyland-kb09/actions/runs/38204835139), source `a3cca60be705bad7735a63b49aad1a0eda1b77ec`, passed six CTest suites, 613 GUI checks, 63 dialog states and Apple AU validation. All three bundles passed universal-architecture and signature verification. Final packaging retains every signed executable byte and bundle resource; only external documents and presets are curated. The final Mac ZIP has SHA-256 `e11bcfba319660bd4f8d465e1f7b31ce9ab55a3d1a20836da0af0057841fa5ea`.

Linux run [38205426751](https://github.com/kevinberg84/toyland-kb09/actions/runs/38205426751), at the same final source commit, passed six CTest suites, 613 GUI checks, 63 dialog states and its shared-library dependency check. GUI verification runs under Xvfb with Openbox, providing normal X11 window-manager protocols. The generic x86-64 ELF executable and launcher permissions are checked while packaging. The final Linux ZIP has SHA-256 `947ed3efd8eea976902cb74e4b21aafaee537f2756634dfcd6c89830caf8cccd`. Its folder and ZIP remain tagged UNVERIFIED pending launch, audio playback and MIDI confirmation on a real Arch desktop.

Mac packages are universal arm64/x86-64 and ad-hoc signed, without Apple notarization. Intel execution requires user evaluation. Linux is a generic x86-64 standalone with glibc 2.35 or newer; the CI virtual display and dependency checks cannot establish physical Arch audio playback. Live controller frame pacing, real hardware recordings and other users' audio devices remain separate user checks.
