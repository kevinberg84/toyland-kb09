# Toyland KB-09 user guide

Open `index.html` in a browser. Keep `styles.css` and the complete `assets/` folder alongside it. The guide uses local images and system fonts, so its contents, instructions and pictures work offline. Links to DAW vendors require an internet connection.

`index.html` is the shared editorial source. Distribute the separate Windows, Mac or Linux edition from `platforms/<os>/User Manual.html`; each is self-contained and includes only that platform's setup and troubleshooting. Linux remains standalone-only and UNVERIFIED. OS-specific `docs/open-me-first*.html` accompanies the corresponding manual.

Use **Print / Save PDF** for a printable copy. The browser expands the troubleshooting answers for printing and restores their earlier state afterward. HTML is the supplied navigable manual; there is no separately reviewed PDF edition.

Run `tools/make_manual_html.py` after source edits to regenerate all three editions. `tools/manual_platforms.py` owns platform-specific adjustments. The top-level `User Manual.html` is a Windows compatibility copy, not a combined manual. Copy the selected platform's self-contained manual to the package root and to `Support/User Guide/index.html` for installed help. Do not bundle the master source or other platforms' editions. The five panel images and NEXT AGE button are embedded, so cycling works offline. The `qa/` and `reference/` folders are development material.
