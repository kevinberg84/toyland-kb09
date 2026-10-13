# Toyland KB-09 1.5.2 validation

Production source: `58c7dd30cc08455ac3180c818a1a210e5c5b8882`. Windows, universal Mac and Linux standalone use this same revision. This is a GUI-only patch. DSP/processor sources, the 35 parameter identities, stored state format and 24 factory presets are unchanged.

## Windows

- Release VST3 and standalone compiled successfully. Installed file/product resources and the VST3 manifest identify 1.5.2.
- All six CTest suites passed; the compiled editor passed 811 checks, and 63 dialog/menu states rendered successfully.
- Steinberg validation passed 47 tests. GUI-enabled pluginval strictness 10 passed at 44.1/48/96 kHz and blocks 17/128/512, seed 9009, confirmed exit 0.
- The actual VST3 host passed 73 assertions before and after installation. Its 30-second build render used 2.56711 seconds of processing, peak block 0.5149 ms against a 2.66667 ms deadline. The installed run used 2.55927 seconds, peak 0.4619 ms. These are bounded offline workstation measurements, not DAW CPU guarantees.
- Moving-control/key painting measured median 10.1368 ms, p95 11.8439 ms, with 0 chassis rebuilds. These are offscreen paint timings, not measured display FPS.
- Installed VST3 SHA-256: `996737e67752152dc5bba1cc417cd026f72ae8e65393c38d34cd651907ab38db`. Portable standalone: `ea38cbf4e527381ee1619a86be3bec223330b93e863320e53512af93516d33ac`. Both match the validated packages. Preceding installed files are backed up outside scan paths.

## Native platforms

Mac run [38346158812](https://github.com/kevinberg84/toyland-kb09/actions/runs/38346158812) passed six CTest suites, 811 GUI checks, 63 dialog states and Apple AU validation. Standalone, AU and VST3 bundles passed universal arm64/x86-64 architecture and ad-hoc signature checks. Final packaging retains signed executable/resource bytes. The three bundle version plists and VST3 manifest identify 1.5.2. Bundles are not Apple-notarized; Intel execution and individual DAWs remain separate user checks.

Linux run [38346160620](https://github.com/kevinberg84/toyland-kb09/actions/runs/38346160620) passed six CTest suites, 811 GUI checks, 63 dialog states under Xvfb/Openbox, and dependency checks. Its executable is generic x86-64 ELF, embeds 1.5.2, and launch scripts retain executable permissions. The package remains **UNVERIFIED** until physical Arch launch, playback and MIDI input are confirmed. It requires glibc 2.35 or newer; no AMD-specific instruction tuning is required.

## Approved appearance and documentation

Revisions 13–17 refine bender visibility, CUTOFF FREQ/RESONANCE lettering, the left performance-bay seam, exposed fader posts and recessed dust covers. Dust accumulates as fixed additional grains at each of five appearances. POWER retains the approved rounded top, small skirt-edge radius and asymmetric wall shading. Four individual screw heads have sharper recesses, unique orientations and wear, and fresh highlights that disappear with age.

Kevin's final finish adjustment is included: only POWER's plastic uses half the global visual AGE. A pixel comparison confirms Fire Hazard's top and wall finish exactly match the preceding Vintage finish; surrounding materials retain their full aging. The illustrated manual, report-form default and all three OPEN ME FIRST! documents identify 1.5.2. HTML/manual links and all package CRCs, embedded versions and 24-preset counts passed.

## Distribution

All three existing Drive ZIP IDs and the permanent shared folder are retained. Every current ZIP was downloaded back and matched byte-for-byte by SHA-256. The Desktop Host retains exactly its three existing platform folders, updated in place; archives and previous-release backups remain outside it. The public update feed was published and verified over HTTPS: HTTP 200, exact version 1.5.2 metadata and the retained permanent folder URL.

| Verified ZIP | SHA-256 |
|---|---|
| Windows | `e30bcfcf474177bc625af3547e5fae5622f0ab0f05681d43d779179ee181fd8b` |
| Mac | `945ecffe6dc3f2c2436c9925ea5651c8966e35501840f4e8a15c0683203dc9bf` |
| Linux | `4872ce7a53cc637570542edd1ee08a0ad03f714ad7c5f41295287dbaa442e860` |

Detailed logs and machine-readable evidence are in `artifacts/Release152/validation`. No sonic change or measured SH-09 hardware equivalence is claimed. Slow VCO/self-oscillating-filter interactions and live controller frame pacing remain separate evaluation items.
