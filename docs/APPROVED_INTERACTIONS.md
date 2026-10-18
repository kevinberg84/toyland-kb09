# Approved interaction contracts

## Power button

Approved by the user; preserve independently of appearance changes:

- OFF to ON: enable instrument power immediately on mouse-down, while the cap is still depressed and before spring-back.
- ON to OFF: keep instrument power enabled throughout the press; disable it on release.
- Releasing after an OFF-to-ON press must not toggle power off again.
- Stay fully depressed while held; spring back on release, using the released 12 ms press and 20 ms eased return curve.
- Preserve the complete release animation/repaint/idle-deferral callback, even when mouse-down already enabled audio.
- Spring animation must never control the audio power state.
- Do not change this behavior without an explicit user request. Clarify ambiguous instructions before implementation.

`SH09PowerButton` exercises the real mouse event handlers and verifies state at press, during the hold, immediately at release, and after spring-back. It is part of CTest.

## Approved appearance

The CrackPosts267 crack position and shorter exposed posts are approved and included in Approved268. The screw positions remain those approved in ScrewPlacement261, and the crack-end feathering remains approved in CrackFeather263.
