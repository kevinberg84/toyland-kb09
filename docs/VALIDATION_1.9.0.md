# KB-09 1.9.0 release validation — in progress

Public release remains 1.8.2. This file is a checklist, not a declaration that tests passed.

## Remaining gates

- Build exact 1.9.0 Windows, universal Mac and Linux sources; record commit and hashes.
- Run all core/preset/processor/MIDI/POWER/RATE/startup regressions, including new channel filtering and UI menu checks.
- Render the production GUI and settings dialogs. Mac AGE snapshots must wait for the committed material rather than assuming one timer slice.
- Validate Windows VST3 in pluginval/Steinberg and the actual host; validate native Mac AU/VST3 and universal architecture/signatures.
- Exercise startup prompts/fullscreen under KDE/XWayland and retain real hardware limitations.
- Check all three platform manuals, installation/update paths, license notices, ZIP integrity, executable permissions and hashes.
- Publish matching source and packages on GitHub; preserve legacy Drive update links for older versions that cannot read GitHub URLs. Advance the update feed only after downloads are verified.

## Baseline evidence retained

The 1.8.3 RATE correction was confirmed by Kevin in Live and passed Windows host regressions.
The latest Mac baseline passed 15 core suites but failed two bender AGE visual assertions.
The headless Arch diagnostic crashed in KWin::RenderDevice::eglShareContext before even an independent X11 clock could stay open. That is evidence about the diagnostic environment, not proof of the friend's freeze cause or a successful fix.

## Support migration

Retain a short manual section linking to GitHub Issues and the issue form. Reports are public and require a GitHub account; no automatic diagnostic uploads. Remove the old Google submission form from new manual editions without deleting historical reports or the service.
