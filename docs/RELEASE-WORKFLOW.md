# Kevin's release preferences

GitHub release policy (October 18, 2026, final migration decision): GitHub Releases
is the sole maintained public download destination from 1.9.0 onward. Kevin will
personally direct the few people using Drive-only update checkers to GitHub for a
manual upgrade. Do not maintain or upload new Drive versions. Leave the old Drive
files and sharing intact unless Kevin explicitly asks otherwise. The update feed's
releasePage and platform fields point to the verified GitHub release. The old
checkers cannot open these URLs; this one-time manual migration is intentional.
Do not announce an update before all platform downloads are verified. Keep the
newest stable GitHub release marked Latest and retain future stable releases as
history. Do not publish historical private binary backups or test builds.

License packaging (October 18, 2026): include the current root LICENSE.md as
Support/Licenses/KB-09-LICENSE.md in every new platform package, alongside all
applicable third-party notices. KB-09 is source available under its custom Free Use
and No Sale license. Commercial music-making is allowed; software sales are not.
See docs/LICENSING.md for scope and the recorded JUCE interpretation uncertainty.

Retention preference (October 13, 2026): keep all released 1.x.x versions for
now. Only obsolete pre-release/test builds are cleanup candidates. Preserve the
production source and regression tests. Kevin approved release 1.6.0 and asked
to remove the standalone MIDI audition, its shortcut, isolated preferences,
temporary build and rollback packages. That test no longer needs preservation.
Two older locked DSP test modules have one-time cleanup queued for the next
Windows sign-in. Local-only test requests do not authorize a GitHub push,
Drive upload, update-feed publication, or replacement of the released product.

After every completed validated release build, update the installed plug-in and the platform folders in Kevin's OneDrive Desktop Host Folder, then notify him. Publish the Windows, Mac and Linux ZIPs on GitHub Releases and verify matching hashes. Drive maintenance is retired.
The Desktop Host folder must contain exactly three platform folders: Windows,
Mac, and Linux standalone (UNVERIFIED until hardware verification). Keep ZIP
archives and previous-release backups outside that Host folder. Preserve the
three existing platform folder paths and update their contents in place.
If a DAW locks the installed binary, ask Kevin to save and close it; never kill
an application with an unsaved set. Preserve the preceding installed/package
copies outside plug-in scan folders.

Friend-facing packages have OPEN ME FIRST!.html, User Manual.html and Support at
the root, plus the platform installer or Linux launch/shortcut scripts. Hide technical material inside Support. Friends should not
have to select or copy plug-in bundles manually. Keep the illustrated HTML manual synchronized with
changes to the controls, installation and available formats.
Generate the three manual editions with `tools/make_manual_html.py`. Each package
must use its own `docs/manual/platforms/<os>/User Manual.html` and OS-specific
OPEN ME FIRST file. Installed help uses that same self-contained edition as
`Support/User Guide/index.html`. Do not include other operating systems' guides.

Mac is a verified released platform. Do not restore the old verification label.
Distribute and document only usable plug-in formats. The Linux folder and ZIP
retain UNVERIFIED until real launch, audio playback and MIDI input are confirmed
on a physical Arch desktop. Automated checks alone do not remove that tag.
Publish the update feed only after all three GitHub platform downloads are verified.

### Download destinations

- Project: https://github.com/kevinberg84/toyland-kb09
- Current stable downloads: https://github.com/kevinberg84/toyland-kb09/releases/latest
- Public update feed: https://raw.githubusercontent.com/kevinberg84/toyland-kb09-updates/main/version.json

Historical Drive IDs are retained in tools/release-destinations.json as an archive
record, not active upload targets. Do not delete the existing shared folder or files
as part of GitHub migration. Keep the local Desktop Host folder and three platform
folder paths intact. Public downloads are release Assets, not source-code archives.

## Release numbering (required)

Use major.minor.patch for every released build. Classify the combined changes
since the preceding published release before choosing a version:

- Patch: bug fixes, appearance corrections, wording and documentation changes
  that do not add user capabilities (1.1.0 -> 1.1.1).
- Minor: any new backward-compatible user feature, including an indicator,
  reporting form or new control; increment minor and reset patch (1.1.1 -> 1.2.0).
- Major: an incompatible change to supported plug-in/project, preset or parameter
  behavior; increment major and reset minor and patch (1.2.3 -> 2.0.0).

Use the highest applicable category. Never reuse a published version or silently
change the version after packaging. The MIDI activity indicator and screenshot
bug-report form are features covered by the 1.1.0 release numbering correction;
subsequent fixes to them use patch releases, and subsequent new features use a
new minor release.

Before packaging, verify CMake/plugin and standalone metadata, the update menu,
manual edition and report-form default, OPEN ME FIRST!, release notes, package
validation records and update feed all identify the same release. Rebuild all
platforms when the binary version changes. Keep the illustrated manual current.

Do not generate or bundle a PDF manual. Users can print or save PDF from the HTML manual.

Before the next release, review the manual's suggested system requirements
against the final build's resource measurements. Current recommendations are
8 GB RAM for standalone and 16 GB or more for DAW sessions; the October 2026
Windows test does not justify raising them. Repeat the check if the final build
changes materially, and update and regenerate every OS edition together if
needed. Windows measurements are not certified Mac/Linux or minimum-hardware
results. Keep this release checklist out of the user manual itself.
