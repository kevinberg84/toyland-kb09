# Window sizing and maximise behaviour — 0.11.0

`Source/GUI/WindowSizing.h/.cpp` handles editor geometry independently of DSP and patch state. The design canvas is **1500 × 834 logical units**, including the complete footer. The panel is uniformly scaled and centred; extra width or height is letterboxed. The outer editor can use a free aspect ratio.

## User actions

| Action | Behaviour |
|---|---|
| M | Fit the panel within approximately 90% of current monitor work-area width and height, accounting for measurable native chrome. Centre an eligible floating window. |
| L | Request the available work area, excluding the taskbar. Standalone maximises its own window; VST3 requests editor dimensions through its host. |
| Maximise button / empty-header double-click | Toggle L and the preceding size. |
| Custom-header drag | Move an eligible floating window. A maximised window restores when the pointer actually moves, retaining the grabbed position; a single click leaves it large. |
| Release custom-header drag near the screen top | Request L on the pointer's monitor. The standalone threshold is approximately 12 logical screen units; native hosted movement converts its threshold to physical pixels. |

L means normal maximised work-area behaviour, not taskbar-covering kiosk mode. M now uses a 90% width-and-height bound, approximately 20% larger in each dimension than the previous 75% setting before chrome/aspect fitting. This is a window-size choice, not an image-quality setting. [HIDPI.md](HIDPI.md) explains Live's separate bitmap-scaling behavior and the finite resolution of the photographic sources.

## 0.11 client-pixel correction

The Windows helper measures both corners of the **actual native child-client rectangle in physical pixels**, maps the editor's area within its JUCE peer, and derives horizontal/vertical pixels per editor unit. These read-only child/monitor metrics are independent of permission to move a host frame. A host window rejected by the movement whitelist can therefore still supply its actual editor scale.

Chrome is measured only for the owned standalone frame or a floating owned/tool frame tightly surrounding the editor; the DAW's main-window contents cannot be counted as decoration. When that frame cannot be identified, sizing retains the measured child scale and monitor work area with a small logical chrome allowance. Only when child metrics are also unavailable does it fall back to JUCE's logical display area. Hosts may still constrain requests. The complete panel/footer fit uniformly without cropping.

`peerAttached()` is forwarded from editor parent/visibility changes and standalone titlebar setup. It samples attached scale/work-area geometry every 120 ms, waits at least **720 ms**, and requires three consecutive stable measurements before one initial refit of the intended M/L mode. It stops after **1.44 seconds** if geometry does not stabilise. Explicit M/L/restore actions or detected drag/resize input cancel the pending fit; it does not repeatedly override later user sizes. The initial constructor fit remains a fallback until this bounded attachment pass. No host content scale is forced or multiplied manually; JUCE retains the VST3 scale negotiation.

Native geometry operations temporarily scope the current thread to per-monitor-aware coordinates and immediately restore it; that scope does not span a JUCE resize. No process-wide DPI setting, host window style or DAW rendering engine is changed. The Windows editor chooses software rendering only for its own JUCE peer. [Microsoft DPI documentation](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdpiforwindow)

## Ownership and movement

Standalone actions require the processor's Standalone wrapper and its exact desktop `ResizableWindow` ancestor. A deferred callback enables that owned window's native caption and buttons. L maximises its existing normal window, preserving Windows' Restore placement. Custom-header movement uses JUCE `ComponentDragger`, an initial mouse-down anchor, live pointer positions and a re-entry guard.

Hosted movement requires a visible top-level ancestor in the current process/UI thread, an owned/tool window, an identifying Toyland/KB-09/legacy title, and an editor occupying at least 65% of the client area with limited surrounding chrome. Title alone is insufficient. The ancestor walk uses `GA_ROOT`, never `GA_ROOTOWNER`; native moves use `SetWindowPos` without resizing or changing activation/z-order. The candidate is rechecked before movement. Native maximise is never applied to a DAW window. [GetAncestor](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getancestor), [SetWindowPos](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowpos)

Docked/unidentifiable frames, other UI threads or substantial surrounding host UI can fail that conservative movement test. M/L still request sizes through the host. A host-owned title bar remains the host's responsibility. Placement settling is bounded to eight 35-ms corrections, stops before dragging/destruction, and installs no native hook or subclass.

All helper methods run on the message thread. Forward only empty-header gestures, not control/keyboard gestures. `resized()` uses `contentBounds()` and a uniform scale/translation for the complete design; do not reinstate a fixed outer-window aspect constraint. `onChange` must not destroy the helper.

## Evidence and limits

The final October 7 build reports **280 GUI/asset assertions, zero failures** in `artifacts/Validation-KB09/gui-0.11-aging-events-final.txt`, including transformed footer bounds at standard, wide and tall sizes, independent wear-shape checks and solid-cap opacity. Its previews are in `artifacts/GUI-0.11-final`. Binary identity and exact release logs are recorded in [VALIDATION.md](VALIDATION.md). Compiled layout checks do not establish every host's native interaction or actual hover-popup placement.

The final standalone was observed on a **1920 × 1080 physical display at 150% scaling**, at **M 1051 × 642 / L 1280 × 720 logical pixels**, with the complete footer in both views and return from L to enlarged M. The preceding revision's M was 857 × 534 logical pixels. These observations establish size/footer behavior on this workstation; they do not establish Live sharpness or physical 4K rendering. They are logical window observations rather than physical-pixel measurements.

Historical **0.10** standalone observations on October 5 covered native Restore/Maximise, single-click remaining large, double-click restoring M, restore-under-pointer dragging and custom-header top-edge snap. The snap observation preceded the final 0.10 native-Restore correction, which retained the same mover/threshold. Those gesture observations belong to 0.10; the preceding 0.11 measurements above are separate evidence.

Final binary identity and installation are recorded in [VALIDATION.md](VALIDATION.md). Live-specific 0.11 window checks were not repeated because desktop automation could not resolve Live's foreground process after three recovery attempts; [LIVE_TEST.md](LIVE_TEST.md) distinguishes older observations. Native-caption top-snap, multiple-monitor/mixed-DPI movement and unsupported/docked hosts need their own evidence. A physical 4K display was unavailable; 3840 × 2160 editor exports verify rendering dimensions only. The finite-resolution chassis also limits raster detail independently of window sizing.
