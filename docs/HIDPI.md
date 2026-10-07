# Sharp rendering in Ableton Live on Windows

KB-09 draws its controls, text, scales and panel lines at the resolution of the editor surface supplied by the host. It does not render the complete GUI into a fixed 1500-pixel screenshot. M and L choose window size; they are not image-quality settings. M now fits within 90% of work-area width and height before aspect fitting and chrome, about 20% larger in each dimension than the earlier M. L fills the available work area, with the complete footer.

Windows scaling and rendering resolution are different. At 150% scaling, a 2560-logical-pixel window can cover 3840 physical pixels and still be sharp. A host can instead enlarge a lower-resolution plug-in bitmap, which makes the entire interface soft. Enlarging that bitmap further cannot recover detail.

## Live 11 check

1. Save the current sound with KB-09's SAVE button before changing or replacing instances.
2. In Live's Plug-ins browser, right-click Toyland KB-09. If **Auto-Scale Plug-in Window** is available and checked, disable it. The same option may be available on the plug-in window title bar. Keep Live's HiDPI mode enabled.
3. Create a **fresh instance on a new MIDI track**, then LOAD your saved sound. An existing instance may retain its original scaling context; closing and reopening its editor alone may not refresh it.
4. Select M or L within KB-09. Inspect the labels at normal display magnification, alongside the standalone application if helpful.

This is supported by a [JUCE maintainer's direct Live 11/VST3 explanation](https://forum.juce.com/t/blurry-vst3-in-ableton-live-11-hidpi/61587). [Ableton's own HiDPI page](https://help.ableton.com/hc/en-us/articles/209775985-High-DPI-monitor-support) describes the setting but contains inconsistent VST3 wording, so its availability must be checked in the installed version. It was not possible to verify this workstation's menu: the desktop inspection tool repeatedly returned `foreground window did not report a process id`. No fresh Live sharpness result is claimed.

## What the plugin corrects

The Windows sizing helper measures the editor's actual child-client pixels separately from the conservative permission to move a host-owned floating frame. Its first attached fit waits for the peer's scale to settle; it does not rely only on a pre-attachment monitor estimate. This fixes an independent sizing error, and does not override Live's DPI awareness or force a host to stop bitmap scaling. JUCE handles the VST3 host's content-scale notifications, which may arrive before or after attachment. [Steinberg scale contract](https://steinbergmedia.github.io/vst3_doc/base/classSteinberg_1_1IPlugViewContentScaleSupport.html)

The GUI keeps labels and other native detail separate from photographic materials and uses high-quality image sampling when scaling them. This improves smaller views and avoids baking tiny text into a low-resolution background. The same native geometry renders in the actual 3840x2160 editor exports.

## Artwork detail limit

The clean chassis photograph is 1774x887. Native 4K GUI output does not make that source a native 4K photograph. Key and knob close-ups have substantially more source detail per displayed control; each of the 37 wear decals has its own source crop. The smooth case's broad shading tolerates enlargement, while finite raster detail remains a limit. A physical 4K display and the current Live auto-scale setting have not been verified. See [artwork inventory](../Assets/Photoreal/README.md) and [validation scope](VALIDATION.md).
