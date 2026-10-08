# Consumer manual review for 0.13 integration

Reviewed 7 October 2026. Scope: `docs/manual/index.html` and the consumer
`START HERE.html` string in `tools/package_windows.py`. This review covers
the revised text and offline navigation. The final 0.13 version label and
panel screenshot update are still pending integration; the reviewed guide
currently names Windows 0.12.0 and uses `assets/panel-0.12.png`.

## Wording review

- Hero subtitle is “A recreation of Kevin's Roland SH-09.” The former AGE
  slogan is absent from the guide and START HERE copy.
- Hero and first numbered section explain AGE's appearance and circuit-model
  behavior, external filtering and the envelope follower. The feature-led
  introduction precedes installation and quick start.
- Wear is described as modeled after Kevin's own aged SH-09. There is no AGE 7
  reference, personal reference scale or wear-event threshold table.
- The “What this draft is based on” section and its navigation entry are gone.
  Development validation/evidence reports are not linked from the main guide.
- The consumer test-drive checklist remains useful for playing, comparing
  sounds, saving/recalling presets, external-audio routing and display settings.
- DAW recipes retain inline vendor citations, conditional menu wording and
  the confirmed Live Auto-Scale remedy. Trying a new Live instance is
  conditional advice, not a required step.
- Current Windows VST3 and standalone availability is distinguished from
  planned AU/Mac and AAX/Pro Tools downloads. Practical audio and host limits
  remain; no measured hardware equivalence is asserted.
- No wording fixes were necessary during this independent review.

## Offline navigation

`check_manual.py` passed: two HTML documents, 30 IDs, 38 fragment links,
10 local references and 16 external citations. Output is retained in
`link-check-0.13.txt`. This checks local dependencies and target anchors;
it does not establish remote website availability.

## Browser layout review

Desktop and mobile screenshots were **not captured**. The Browser skill was
read, but its runtime reported “No browser is available.” The prescribed
troubleshooting discovery returned an empty connected-browser list. No other
browser-control surface or source-code workaround was used. Existing
unversioned QA screenshots predate this revision and are not evidence for
the current text layout.

The stylesheet's desktop, narrow-width and print rules were read, but static
inspection does not confirm rendered layout. Visual QA remains to be completed
when a browser is available, preferably after the final panel image is updated.

## Integration reminder

The packaging script's other generated files were outside this review's edit
scope. Its 0.12 Release Notes still contain the former AGE 7 reference and event
threshold prose. Update those separately with the 0.13 package content.

## Final integration

The parent integration updated the guide's version to Windows 0.13.0 and its
compiled-editor screenshot to `assets/panel-0.13.png`. START HERE, Release Notes
and the packaged guide now use the revised consumer wording, without the AGE 7
reference, event threshold table or development evidence page. The 32-file ZIP
and all contained hashes match the tested build. The packaged guide passes its
offline checker (one HTML document, 25 IDs, 36 fragment links, six local
references and 14 external citations). Browser layout QA remains unverified
for the reason recorded above.

Checking the entire extracted package also passes: two HTML documents, 25 IDs,
44 fragment links, 18 local references and 14 external citations. This adds
START HERE and its links to the guide; the result is retained in
`artifacts/Validation-KB09/package-manual-0.13-final.txt`.
