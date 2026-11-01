# KB-09 1.10.1 validation

Release candidate: validation in progress.

Issue 4 was reproduced in the official Renoise 3.5.4 Windows demo. The correction passed eight full-patch save/reopen cases and a full application restart. All 35 stored values and program identity were checked from actual component state, rather than trusting the host cache.

The SDK protocol regression fails all 24 presets against released 1.10.0 and passes all 24 against the corrected candidate. Earlier Reason correction was reported successful; final native Reason testing is not available locally.

Native build and release-package checks will be recorded before publication. Every platform, physical audio/MIDI setup and DAW is not claimed tested.
