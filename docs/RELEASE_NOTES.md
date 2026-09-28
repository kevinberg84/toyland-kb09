# 0.9.0 release candidate

First compiled Windows x64 SH-09 instrument candidate, with one shared monophonic core, auxiliary external input, follower-driven filtering, sixteen factory patches, state/automation support and a resizable playable panel. Ableton Live 11.0 scanned and loaded the instrument, processed auxiliary audio in Hold with the follower preset, and restored patch and routing after reopening the saved Set. Exact scope and remaining checks are in LIVE_TEST.md.

The code has passed through implementation, native builds, two independent VST3 validators, actual-module tests, DSP tests and a fresh independent review. Corrections include nonlinear self-oscillation startup, pulse DC coupling, Hold-mode All Sound Off, rate-independent noise level, discrete selector switching, keyboard event overflow and duplicate-note display accounting. Exact post-freeze build/test results and binary hash appear in VALIDATION.md.

This is a usable software instrument candidate with documented fidelity gaps. It is not a fully measured high-fidelity emulation or a photorealistic finished skin. See KNOWN_LIMITATIONS.md before treating it as a complete match to the original brief. No commercial preset or graphical asset was copied. Primary documents are research references and are not bundled as licensed product artwork.
