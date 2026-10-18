# Project instructions

## Preserve approved behavior; clarify ambiguity

Kevin explicitly requested on October 18, 2026:
- Do not change approved behavior incidentally while implementing another instruction.
- If an instruction is ambiguous about behavior or conflicts with an approval, ask for clarification before making the dependent change. Kevin prefers a clarifying question to an incorrect implementation.
- Do not change the approved power-button behavior again unless Kevin explicitly requests that change.

Power-button contract:
- Stay fully depressed while the mouse is held down; spring back only upon release.
- Preserve the released standalone's spring motion (12 ms press, 20 ms eased return; OFF resting height 2, ON resting height 1, fully depressed 0).
- Enable instrument audio immediately on an OFF-to-ON mouse-down.
- Keep instrument audio ON during an ON-to-OFF hold; disable it at release.
- Do not bypass the release animation/repaint/idle-deferral path to avoid a duplicate audio toggle.
- Verify both the mechanical motion and audio timing with `SH09PowerButton` before updating the TS.

See `docs/APPROVED_INTERACTIONS.md` for approved interaction and appearance details.
