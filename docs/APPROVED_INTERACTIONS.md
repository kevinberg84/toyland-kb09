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

## Approved top panel and cap finish (October 18, 2026)

Approved from CapGlimmer281, including TickExpansion280 and its predecessors:
- Center the section outlines and complete control/printing group on the top panel; center the disclaimer too.
- Align the left branding with the MODULATOR left outline; retain Toyland's placement.
- Match the VCF middle ENV label/symbol distance to the VCA switch legend spacing.
- Use one short outer tick length on every vertical fader, including ADSR and the performance bay; extend 0/5/10 uniformly toward their numerals. Every tick reaches the slot edge.
- Keep the long shared tick spans and cap widths. Reclaim spare switch/control space rather than shrinking them.
- Use CUTOFF's 35-unit border-to-axis margin for the border-facing number columns throughout the top panel. VCF ENV/MOD, its switch, labels and bracket move together.
- Center RATE LED over the waveform switch using shared switch bounds; preserve the independent LED clock/presenter.
- Add a narrow left-shoulder glimmer to the black cap bodies: strongest at Minty, retaining 78 percent at full age. Preserve the silver-band material, cap shape and position-stable rendering.

Slider-response review is requested before the next public release. Appearance approval does not authorize changing control tapers or the sound engine.

## Squared physical response and Linux JACK (October 18, 2026)

User authorized a squared physical-position curve for PORTAMENTO, VCO MOD, VCF MOD and VCF ENV/FOL'R depth, plus Linux JACK support. Retain stored effect amounts, parameter IDs/ranges and DAW parameter automation. GUI caps/readouts use inverse mapping; typed values and learned MIDI use forward mapping. CC1 uses the same curve with inverse-position pickup. Keep the current endpoints, other control responses, and the approved power-button/independent LED behavior.

Linux builds enable JUCE_ALSA=1 and JUCE_JACK=1. Windows retains ASIO/WASAPI/DirectSound, and macOS retains Core Audio. JACK is not a Windows TS backend. Physical Linux audio validation remains separate from native build/backend-factory checks.

## RATE host robustness correction (October 18, 2026)
The independent RATE window must remain above its own editor after host restacking,
without rising above unrelated applications or taking focus. Keep the fallback
renderer current. The visual clock follows live RATE/mode/sync controls even while
a host suspends audio callbacks; AGE, notes, reset and editor reopening must not
leave a frozen LED. Preserve the approved circular material and independent
Windows presenter. These corrections address the reported Live failure.
