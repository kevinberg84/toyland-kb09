# Toyland KB-09 1.5.1 validation

Production source: `f3fee777d3f173317250b88325b7b9dc7d9089d8`. Windows, universal Mac and Linux standalone use this same revision. This is the approved GUI-only patch: the DSP/processor sources, 35 parameter identities, saved-preset format and 24 factory presets are unchanged.

## Windows

- Release standalone and VST3 compiled successfully; both Windows file/product resources and the VST3 manifest identify 1.5.1.
- All six CTest suites passed (DSP, filter nonlinear regression, processor, preset files, factory bank and updates). The compiled editor passed 811 checks; 63 dialog/menu states rendered successfully.
- Steinberg VST3 validation passed 47 tests. GUI-enabled pluginval strictness 10 passed at 44.1/48/96 kHz and blocks 17/128/512, with confirmed process exit 0.
- The actual VST3 host passed 73 assertions both before and after installation, covering state/parameter behavior, dense MIDI/automation and finite audio. The build's 30-second render used 2.54701 seconds of processing; peak observed block was 0.3851 ms against a 2.66667 ms deadline. The installed run used 3.15804 seconds, peak 0.620 ms. These are bounded offline workstation measurements, not DAW CPU guarantees.
- Dirty-region moving-control/key painting measured median 9.6896 ms and p95 10.9105 ms, with zero chassis rebuilds. This is offscreen paint timing, not measured display FPS or a live-controller frame-pacing result.
- Installed VST3 module SHA-256: `ed5300e91f5b20b3abb491e17d2189ea346c2b109b95412e594f5e1d9c51c41b`. Portable standalone SHA-256: `76867244190079f3a4da85d15c7175dadb676a0a1c93ee8f82f5b7f97863aa6c`. Installed files match the tested build; preceding copies are backed up outside scan paths.

## Native builds

Mac run [38314411355](https://github.com/kevinberg84/toyland-kb09/actions/runs/38314411355) passed six CTest suites, 811 GUI checks, 63 dialog states and Apple AU validation. The standalone, AU and VST3 architecture/signature checks passed for arm64/x86-64 universal bundles. Packaging preserves their executable/resource bytes. Bundles are ad-hoc signed, not Apple-notarized. Intel execution and particular DAWs remain separate user checks.

Linux run [38314389425](https://github.com/kevinberg84/toyland-kb09/actions/runs/38314389425) passed six CTest suites, 811 GUI checks, 63 dialog states under Xvfb/Openbox, and shared-library dependency checks. The package checks x86-64 ELF identity and executable launcher permissions. It remains UNVERIFIED until physical Arch launch, playback and MIDI input are confirmed. The generic executable requires glibc 2.35 or newer; no AMD-specific instruction tuning is required.

## Appearance and documentation

The approved revisions refine panel printing, ENV margins, the performance bay, keyboard seams, bender geometry, photographic cheeks and spatial rust growth. All five appearances are rendered by the actual editor. Tests retain missing-cap identities/hit areas and sound values, verify the changed PULSE WIDTH cap-loss event, and exercise bender position/return, staged damage and cache stability.

The final authorized polish uses the disc's sharpened existing highlight on the rounded pointer and tapered object-space abrasions on the rotating bender. Surface appearance remains a photographic/reference-informed reconstruction, not measured part CAD. The manual contains an actual final-editor illustration and describes the 0/25/50/75/100% wear-count/twist progression. Source manual QA passed: 3 HTML documents, 99 IDs, 88 fragment links and 12 local references.

## Distribution

Final archive hashes, permanent Drive IDs, downloaded-byte matches, package CRC/manual-link/preset checks and local Host synchronization are recorded in `artifacts/Release151/validation`. The Desktop Host retains exactly its three existing platform folders; archives and backups stay outside it. All three current shared downloads have matching hashes. Update-feed publication is pending Kevin's specific approval after automatic approval review rejected that external distribution action.

No sonic correction or hardware-level SH-09 accuracy is claimed. Slow VCO/self-oscillating-filter interactions remain under investigation; physical SH-09 recordings are needed for calibration. Live controller frame pacing, physical Arch playback/MIDI and every native DAW remain user evaluation items.

| Verified current ZIP | SHA-256 |
|---|---|
| Windows | `16b0f07abbbf932835f2ade9b99e500f1807d777001b74a1fd273ed14a9c0c5f` |
| Mac | `f43dd00fd180c6f3c1eb367c9a5659fb600c3dfe99b6c85e82d3501be705387b` |
| Linux | `2ae76e72bac7dc5e0502fced1953bf721869e7cfef4c15d50d3d700c877dc92b` |
