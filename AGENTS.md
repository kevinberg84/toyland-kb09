# Project instructions

## Update the installed VST3 after releases

- Explicit instruction on October 31, 2026: whenever a new public version is released, replace Kevin's installed Windows VST3 with that latest release as part of completing the release.
- Verify the installed binary version and hash against the released package. Publishing alone does not complete the update.
- Preserve a rollback copy. Do not replace a loaded plug-in while a DAW is using it; arrange a safe close first.
- Keep experimental issue-test builds separate unless Kevin explicitly requests installing one.
- Explicit instruction on October 31, 2026: normally test only KB-09's latest versions. When investigating a bug report, test both the version used by the reporter when the bug occurs and the latest version. Verify the actual loaded plug-in path, version and checksum before testing; do not silently test an older installed build. Keep reporter-version baselines and current candidate results clearly identified.

## Statistics checks must not control the user's computer

- Explicit instruction on October 31, 2026: check downloads, demo plays, referral sources and countries through APIs or background access only.
- Do not move the cursor, send keyboard input, change browser focus, or use native UI automation to check statistics. Keyboard navigation is not an acceptable workaround.
- If background access cannot retrieve a statistic, report that limitation and any available verified figures; do not take over the user's browser.

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

## Personal information in public material

- Do not publish anyone's personal details, including their name, without the user's explicit consent. A name mentioned in conversation is not permission to publish it.
- Describe bug reports and validation observations anonymously. Preserve names only where their public use was explicitly approved, such as the project dedication.
