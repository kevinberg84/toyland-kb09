# Kevin's release preferences

Retention preference (October 13, 2026): keep all released 1.x.x versions for
now. Only obsolete pre-release/test builds are cleanup candidates. Preserve the
current standalone MIDI test and its specifically requested prior-iteration
rollback snapshot. Local-only test requests do not authorize a GitHub push,
Drive upload, update-feed publication, or replacement of the released product.

After every completed build, update the installed plug-in and the platform folders in Kevin's OneDrive Desktop Host Folder
and the current Windows, Mac and Linux shared Google Drive ZIPs, then notify him. Verify matching hashes.
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

Mac is a verified released platform. Do not restore the old verification label.
Distribute and document only usable plug-in formats. The shared download folder
contains one current Mac ZIP, one Windows ZIP and one Linux standalone ZIP;
upload new versions of those existing files to preserve links. The first Linux folder and ZIP carry an UNVERIFIED tag until Kevin confirms launch, audio playback and MIDI input on a real Arch desktop. Automated checks alone do not remove that tag. Publish the
update feed only after all three platform uploads are verified.

### Permanent shared destinations

Keep the existing **KB-09 Host Folder**, its name, and its sharing settings.
Its permanent folder ID is `18-S-iz4-5i8mkjPSaRtNXy4BTG2D1VBo`.
Never create a replacement folder for an iteration. Update only its contents.
Also preserve these existing download file IDs by uploading a **new version**
of each ZIP (Drive: Manage versions > Upload new version), rather than deleting
and re-uploading them as new files:

- `Toyland KB-09 for Windows.zip`: `1E3zEFVccSlNBXAaLd_MuO8YCAzyJxW7U`
- `Toyland KB-09 for Mac.zip`: `1rGIq_l31dPX4o3fMvlskKsKcTXBwBd3P`
- `Toyland KB-09 for Linux (Standalone Only) - UNVERIFIED.zip`: `11oRmSe2V4g1pbXrnavfndi8oz5Wdk3Jr`

These destinations are recorded in `tools/release-destinations.json`. Verify
the folder and all platform file IDs before and after each upload, then download the
current ZIPs and verify their hashes and embedded versions. Do not assume a
matching filename proves the linked file is the intended item.

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
