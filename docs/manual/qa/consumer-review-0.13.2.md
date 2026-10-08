# Consumer guide review — Windows 0.13.2

Reviewed 7 October 2026. This review covers `index.html`, the consumer README and the final compiled-panel asset. `qa/` and historical `reference/` material are excluded from the consumer package.

## Content

The guide retains the owner's approved features-first introduction to his Roland SH-09 recreation, AGE's appearance and circuit-model behavior, external filtering and the envelope follower. Installation and quick start follow the feature introduction. The former AGE slogan, reference-at-7 claim, appearance-event table and internal source/validation section remain absent.

The visible edition is now Windows 0.13.2. The guide distinguishes the supplied, usable Windows VST3 and standalone app from the separate unsigned Windows AAX development build and unbuilt AU source kit. It makes clear that there is no signed, Pro Tools-tested AAX release and no installable AU. Practical setup, saved presets, sidechain routing and host display guidance remain self-contained.

The playing section describes immediate mouse-held and MIDI-held blue highlighting plus the amber effective-note bar. The AGE section includes gradual key dulling, fine knob-metal pitting and pale screw oxidation. The panel instructions explain the grey POWER button's tall OFF / lower ON positions, the red lamp below its label, and fresh-note behavior after power cycling.

## Browser review

The in-app Browser was available for this revision. At its normal desktop viewport, the logos, hero, sidebar navigation and format notice render clearly. Internal navigation to installation works. At a 390×844 viewport, the guide switches to its narrow layout, without page overflow; the measured document and viewport widths are equal. The external-audio jump link reaches its section, with the heading and instructions visible. Troubleshooting answers expand and the test-drive checkboxes toggle. The responsive preview override is reset after review.

Review captures are `desktop-0.13.2.png`, `install-0.13.2.png`, `mobile-0.13.2.png` and `panel-0.13.2.png` in this folder. These are development QA material, not manual dependencies. No stylesheet change was necessary.

The fresh 0.13.2 compiled aged panel is copied unchanged from `artifacts/GUI-0.13.2-final/personal_age7_3840x2160.png`. The source and manual image SHA-256 are both `F241C6DF612EA458B039382ED7EC57243DAC83A4CDF7BDDE1F18EE9002037EE6`. The Browser loaded its actual 3840×2160 pixels and the panel page was visually inspected.

`check_manual.py` passes, with output in `link-check-0.13.2.txt`: two HTML documents, 30 IDs, 39 fragment links, ten local references and 16 external citations. These counts include the historical reference HTML retained in the development folder; it is excluded from the consumer package. The package generator separately checks the extracted consumer guide. No final PDF pagination or native print-dialog certification is claimed; the supplied manual is navigable offline HTML.
