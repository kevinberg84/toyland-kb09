# Manual draft review — 2026-10-07

Reviewed through the supported in-app browser on a local loopback server. No site was published, no account used, and no future plug-in format built.

- Desktop cover and panel view inspected at the normal 1280 × 720 viewport. No document-wide horizontal overflow. The supplied logo crops render correctly and the compiled-editor image loads when its section is reached.
- Responsive cover inspected at 390 × 844. Contents switch to two columns; text and buttons stay within the page. Wide reference tables have their own horizontal scroll container rather than forcing page overflow. The viewport override was reset afterward.
- Clicked contents → panel, contents → DAWs → Live, and contents → test drive. Hashes target the intended sections; the fixed header leaves headings readable. Smooth scrolling needs a settled observation before measuring the final target.
- Checked a personal-test checkbox and confirmed its checked state. Checkboxes deliberately have no persistence or external transmission.
- Static HTML check: **25 unique IDs, 36 internal anchors, 11 local references, 13 external citations; PASS**. All local targets are inside the manual folder. `check_manual.py` is the repeatable check; `link-check.txt` records its output.
- Reviewed `print-preview.html`, a development-only rendering of the print stylesheet at 794-pixel A4 page width. Navigation/print buttons are hidden, type is compact, the cover is legible, the main content has no horizontal overflow, and disclosure answers are expanded in this preview. Print CSS sets A4 margins, table header repetition, appropriate break avoidance and readable monochrome links.
- **No paginated PDF was generated or certified.** Browser print/PDF dialogue behaviour and page-break quality remain to be checked if a PDF is later delivered. The production beforeprint/afterprint helper expands disclosures and restores their prior states; its code was inspected, but a native print-dialog round trip was not claimed.

Screenshots: `desktop.png`, `mobile.png`, `panel.png`, `live.png`, `print-style.png`. They show the working draft with the historical 0.11 panel reference. Final photo-update screenshots and current validation must replace that reference/status before release. Supporting Markdown copies in `reference/` must be refreshed after the parent task finishes its release documentation.

Exclude this `qa/` folder from the consumer download. It is development evidence, not part of the user manual's runtime dependencies.

## Final 0.12 text and image update

The parent task replaced the panel reference with the actual compiled 3840 × 2160 AGE-7 image, updated the build status and implemented event descriptions, and added an offline HTML evidence page. The HTML/CSS layout and panel-image aspect ratio are retained. The earlier browser screenshots remain historical draft evidence; they do not show the final image replacement. The final static checker verifies both HTML documents, local dependencies and cross-document fragment targets. Its current output is in `link-check.txt`. No new paginated PDF or native print-dialog review is claimed.
