# KB-09 1.10.1 validation

Release validation completed October 31, 2026.

Issue 4 was reproduced in the official Renoise 3.5.4 Windows demo. The correction passed eight full-patch save/reopen cases and a full application restart. All 35 stored values and program identity were checked from actual component state, rather than trusting the host cache.

The SDK protocol regression fails all 24 presets against released 1.10.0 and passes all 24 against the corrected candidate. Earlier Reason correction was reported successful; final native Reason testing is not available locally.

Windows: all 15 CTest suites, native GUI and dialog checks, Steinberg VST3 validation, pluginval strictness 10, full VST3 host checks and RATE host checks passed. Native universal macOS and Linux builds and their required checks passed. macOS AU validation and native hosted RATE tests passed. All three package archives passed checksums, CRC and manual link validation. The macOS package contains arm64 and x86_64 code and is ad-hoc signed, not notarized. Every platform, physical audio/MIDI setup and DAW is not claimed tested.

## Rendering fixture

The first Mac attempt passed all 15 audio/preset suites but failed one adjacent-AGE knob image comparison. That capture used a fixed dispatch slice even though AGE intentionally commits its material later. The fixture now waits for the reported AGE commit, matching the existing cheek and bender captures. The original pixel comparison is unchanged, and adjacent knob images are retained for inspection. Production panel rendering and artwork are unchanged from 1.10.0. A complete successful native rerun is required before publication.
