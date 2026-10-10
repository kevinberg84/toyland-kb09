# Kevin's release preferences

After every completed build, update the installed plug-in and the platform folders in Kevin's OneDrive Desktop Host Folder
and the two existing shared Google Drive ZIPs, then notify him. Verify matching hashes.
If a DAW locks the installed binary, ask Kevin to save and close it; never kill
an application with an unsaved set. Preserve the preceding installed/package
copies outside plug-in scan folders.

Friend-facing packages have the installer, OPEN ME FIRST!.html, User Manual.html and
Support at the root. Hide technical material inside Support. Friends should not
have to select or copy plug-in bundles manually. Keep the illustrated HTML manual synchronized with
changes to the controls, installation and available formats.

Mac is a verified released platform. Do not restore the old verification label.
Distribute and document only usable plug-in formats. The shared download folder
contains one current Mac ZIP and one current Windows ZIP; upload new versions
of those existing files to preserve links. Publish the update feed only after
both platform uploads are verified.

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
validation records and update feed all identify the same release. Rebuild both
platforms when the binary version changes. Keep the illustrated manual current.

Do not generate or bundle a PDF manual. Users can print or save PDF from the HTML manual.
