# Ableton Live observations

## Current scope — 2026-10-07

Toyland KB-09 0.11.0 is now installed in the standard VST3 folder, with verified module SHA-256 `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`. Its actual-VST3 test host passed 75 assertions and a 600-second render; that is separate from Live UI evidence.

In the renamed 0.10 checkpoint, Live opened the copied `artifacts/HostTest/KB-09 Upgrade Recall.als` with both prior SH-09 instances identified as Toyland KB-09. External Follower and Sub Drone patches, the 3-Audio/Post FX auxiliary route, 0dB/100% sidechain settings and sustained processed master output returned. This established that particular upgrade recall for 0.10 while preserving the original Set. It is not a new 0.11 observation.

After installing 0.11, Live was launched into its fresh Untitled window. Computer Use could list that window but repeatedly failed to inspect it with `foreground window did not report a process id`, including one bounded activation recovery. Fresh 0.11 scan/loading/recall and native plugin L were therefore not asserted. No existing user Set was edited or overwritten. The earlier copied Set remains available for a manual/current-host retest.

The final 0.11 standalone was inspected at M (1051x642 logical pixels) and L (1280x720 logical pixels) on the 1920x1080 display at 150% Windows scaling. Both footers were completely visible, and L returned to the enlarged M. The 280 GUI/asset assertions cover 4K exports, wide/tall footer geometry, independent wear shapes, opaque fader caps and held white/black notes. A physical 4K display and exhaustive Live GUI interaction are outside this evidence. [HIDPI.md](HIDPI.md) documents the Live Auto-Scale setting and fresh-instance check; the current Live menu could not be inspected, so no Auto-Scale change or native Live sharpness result is claimed.

## Historical SH-09 0.9.0 observations

Test date: 2026-09-27. The application and saved Set identify Ableton Live 11.0 (Set format 11.0_433). The installed SH-09 0.9.0 module has SHA-256 `188CD6EACAD347120DF4A339C6858BEE8D200DEDE3BF10F6D999A447BCE13EE1`, matching the clean validated Release build.

## Observed workflow

1. Live scanned the standard Windows VST3 folder. SH-09 appeared under Plug-Ins > VST3 > Private Instruments > SH-09 and loaded on a MIDI track.
2. Live exposed the instrument's Sidechain source, tap, gain, mix and mute controls directly. No separate effect target was needed for this test.
3. A second SH-09 on track 2 used factory patch 11 / Sub Drone. Audio track 3 monitored track 2, Post Mixer, with Monitor In. Tracks 2 and 3 were routed to Sends Only with sends down, removing their direct master contribution.
4. Track 1's SH-09 Sidechain was set to 3-Audio, Post FX, 0.00 dB gain and 100% mix. Before opening its EXT mixer, the plug-in EXT detector meter responded while its output was silent, verifying arrival at the pre-mixer follower tap.
5. Selecting factory patch 13 / External Follower changed the receiving instance to VCO/Sub down, EXT up, VCF Follower and VCA Hold. Track 1 and master meters then showed sustained processed output without MIDI. With source peak approximately -6.34 dBFS, the settled receiving peak meter showed approximately -20.9 dBFS. These are Live meter observations, not a calibrated transfer measurement or listening comparison.
6. Sidechain Mute silenced the receiving track and master after the filter/follower tail. Unmuting restored output. Live transport start and stop preserved processing, as expected for Hold.
7. Both editors opened, displayed the panel and factory menus, accepted preset selection and closed. After recall, dragging the filter cutoff moved its fader and readout to 9.77/10. The panel was legible at the local display configuration. This is not a complete GUI interaction or multi-DPI matrix.

## Saved Set

The test Set is `artifacts/HostTest/SH-09 Live External.als`; it contains two SH-09 instances and the audio relay above, with no external media dependencies. It starts a sustained test signal when audio processing is enabled.

The Windows Save dialog displayed the entered project path but Live saved `Untitled.als` in its previously used `C:\Ableton Projects\KB-101 Test Project` folder. The new file was copied to the intended artifact path. Automatic approval review rejected moving it out of that existing project, so the extra `Untitled.als` was retained. No existing Set was overwritten.

Live was closed and relaunched with the copied Set. Its title became `SH-09 Live External`, both instances and the audio relay returned, Sidechain still showed 3-Audio/Post FX/0.00 dB/100%, and the master again showed processed audio. Reopening the first editor confirmed 13 / External Follower, VCA Hold, VCO/Sub down, EXT up, and follower activity. This establishes patch and routing recall for this particular Set. Live meter readings varied slightly after restart because patch state does not serialize running oscillator/filter state.

## Scope remaining

This test does not establish every Live acceptance case. Recorded audio export, physical MIDI-device performance, a Live automation-lane sweep, distinct stereo source channels, input-level sweeps, and a full all-control/multi-DPI interaction matrix remain unperformed. The automated VST3 host covers the documented numerical and state/bus cases separately. No actual hardware recording was compared.
