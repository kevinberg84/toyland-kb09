# Acceptance — Toyland KB-09 0.11.0

Reviewed 2026-10-07. The usable Windows x64 VST3 and standalone are built, validated and staged. The VST3 is installed at `C:\Program Files\Common Files\VST3\Toyland KB-09.vst3`. Its module SHA-256 is `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`. The preceding 0.10.0 bundle was hash-verified and backed up outside scanned VST3 folders. [VALIDATION.md](VALIDATION.md) identifies the exact final artifacts and logs.

| Requirement | Final evidence and scope |
|---|---|
| Working monophonic synth, external input/follower, Modern extensions | 92 DSP, 20 processor and 75 actual-module host assertions pass; no measured physical-unit equivalence is claimed. Modern MOD depth increases while Authentic retains its existing limits. |
| Real-time processor work | Zero tracked C++ new/new[] allocations in the covered processor callbacks, including power transitions; bounded coverage, not every host/framework allocator. |
| VST3 compatibility and shutdown | Steinberg 47/0; Pluginval strictness 10 with GUI tests reports SUCCESS and exits 0. |
| Stability/endurance | Actual final VST3 renders 600 seconds of finite, audible output; repeated editor and additional-instance lifecycle tests pass. |
| Patches and state | 58 preset-file checks pass; actual module exports/restores all 16 factory presets. Save/Load supports .kb09 and compatible .sh09state files. Same-index factory replacement refreshes a previously custom filename. All 35 parameter IDs/order and schema1 remain compatible. |
| GUI and high-resolution export | 280 real-editor checks pass, with eight main 3840x2160/2880x1620 PNGs and three bender-position crops. Includes transformed footer bounds at wide/tall sizes, physical selectors, white/black held-note feedback and configured value-popup contrast. |
| Materials and condition | Neutral black smooth-metal base; gradual yellow printing/key tint, knob/screw corrosion and dulling; exact supplied-brand coverage constrains chipped ink. Case/key stains appear as individual fixed-size/opacity events. Two different cracks begin separately and extend slightly. Fresh and aged samples of all 19 white and 13 black keys differ; all 37 new source-alpha shapes are distinct under the bounded mirror/quarter-turn comparisons. Solid cap pixels are independent of contrasting backgrounds. |
| Hardware geometry | Original-unit research informs proportions, switches and hidden-axle PB-4 bender. Exact travel/dimensions remain inferred. Every fader cap is retained because the exposed assembled main-panel actuator shape has not been verified. Screw heads overlay the bay outline; LED aligns with power-button center. |
| Native sizing | Final standalone M and L observed on this workstation (1051x642 and 1280x720 logical pixels); L fills available work area and shows the complete footer. Earlier restore/snap observations are scoped in GUI_WINDOW_BEHAVIOR.md. Physical 4K, mixed-DPI/multi-monitor and every DAW frame are not validated. |
| Ableton | Earlier renamed-build upgrade recall, two instances and follower routing are recorded separately in LIVE_TEST.md. Actual-module validation of the final build does not substitute for every Live-specific workflow. |

The 3840-pixel preview is native editor output. The chassis rasters remain 1774x887 and other bitmaps also have finite source resolution; they are not native 4K photographs. Generated photographic-style composites do not prove material chemistry, exact hardware geometry, or equivalence to a commercial reference plugin.

AAX, older-format decisions, the lay user manual and the final download/install package remain subsequent project phases. The current deliverable is the Windows VST3/standalone build, with technical build and validation documentation.
