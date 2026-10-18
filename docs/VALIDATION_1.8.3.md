# KB-09 1.8.3 preparation — October 18, 2026

Status: unreleased candidate. The public release remains 1.8.2.

## RATE correction

Kevin reports the installed correction appears to solve the intermittent RATE LED
freeze in Live. The previous installed VST reproduced frozen visible pixels when
its floating editor covered the independent LED window. The corrected VST passed
the same window-restacking regression, including idle audio, RATE edits and a
covering host/tool window. It repairs ordering relative to its owner, without
global topmost status or focus changes.

Windows validation passed 14 CTest suites, 881 GUI assertions, 63 dialog states,
47 Steinberg checks, 77 VST3-host assertions, 8 hosted RATE assertions and 11
independent-presenter assertions. Shared wrapper lifecycle tests passed 48 checks.
The independent desktop-pixel trace showed 290 transitions with a longest observed
gap of 116.771 ms at 5 Hz during blocked UI, AGE changes and note activity; surface
corners preserved the underlying panel pixels.

The initial GUI-enabled pluginval run timed out in the editor test. That failure
is retained. An isolated rerun succeeded, followed by three complete strictness-10
runs with GUI tests enabled, exit 0. These results do not establish compatibility
with every DAW or hardware configuration.

The correction retains the sound-engine equations, saved patch values, parameter
identities, approved POWER behavior and LED artwork. Previous installed Windows
files were backed up before installing the candidate. No public release was made.

## Outstanding release checks

Mac passed all 14 suites and dialog checks, but the GUI run failed the RANGE octave
label refresh assertion. Its new native AU/VST3 RATE-view step has not yet run.
Resolve and rerun those checks before packaging a public Mac update.

Linux passed 14 suites, 881 GUI assertions and 63 dialog states. The first isolated
Arch/KDE diagnostic could not start its compositor inside the test container;
this is a diagnostic-environment failure, not a reproduced app failure.

The reported Linux startup/fullscreen freeze is under investigation separately:
Arch Linux, KDE, XWayland and PipeWire; update and shortcut prompts appeared
together, followed by a several-second desktop stall and recovery. A startup
dialog-ordering race exists in the source. Do not describe the desktop freeze as
fixed until the correction has been exercised in the relevant environment.
