# External audio architecture decision

Research date: 2026-09-26. Test update: 2026-09-27. Status: **single-instrument auxiliary routing was observed working in Ableton Live 11.0**, including saved-Set recall. Exact evidence and unperformed cases are in LIVE_TEST.md; the research below distinguishes protocol support from observed host behavior.

## Evidence

**VERIFIED — VST3 protocol:** input audio buses can be classified `kAux`; hosts activate buses independently. An auxiliary label does not promise a particular host UI. [Steinberg bus specification](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/group__vstBus.html)

**VERIFIED — JUCE 9.0.2 implementation:** the first audio input is `kMain` by default. Overriding `juce::VST3ClientExtensions::getPluginHasMainInput()` to return false makes it `kAux`. Later input buses are already auxiliary. Source checked locally: `C:\Dev\JUCE\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp`, lines 3109–3127, and `modules/juce_audio_processors_headless/utilities/juce_VST3ClientExtensions.h`, lines 99–105. [JUCE source](https://github.com/juce-framework/JUCE/blob/master/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp)

**VERIFIED — Live documentation:** Live 11 section 15.5.2.7 explicitly describes routing an audio track into a vocoder plug-in and says the instrument variant on a MIDI track uses the same sidechain routing. Live 12 retains that guidance in section 17.5.2.7. This is stronger evidence than assuming instrument audio-input support solely from VST3. [Live 11 routing](https://www.ableton.com/en/live-manual/11/routing-and-i-o/#feeding-sidechain-inputs), [Live 12 routing](https://www.ableton.com/en/manual/routing-and-i-o/#feeding-sidechain-inputs)

**VERIFIED — modern Live UI:** Ableton documents the integrated third-party sidechain panel in Live 10.1 and later. The user enables Sidechain and selects the source in Audio From. This is generic documentation, not certification of this plug-in. [Ableton sidechain guide](https://help.ableton.com/hc/en-us/articles/209775325-Sidechaining-a-third-party-plug-in)

**OBSERVED LOCAL ENVIRONMENT:** `C:\ProgramData\Ableton\Live 11 Suite\Program\Ableton Live 11 Suite.exe` exists. Windows file/product metadata reports `11.0`; the application About dialog is needed to establish any finer version. This proves installation only.

## Selected design

Use one SH-09 VST3 instrument initially:

| Direction/type | JUCE index | Name | Supported channels | Default |
|---|---:|---|---|---|
| Audio auxiliary input | 0 | External Input | Disabled, mono, stereo | Stereo enabled request |
| Audio main output | 0 | Output | Mono or stereo | Stereo |
| Event input | 0 | MIDI Input | JUCE MIDI channels | Enabled |

The processor implements `VST3ClientExtensions` and explicitly returns its extension pointer through `getVST3ClientExtensions()`. `getPluginHasMainInput()` returns false. There is no dummy main audio input. `isBusesLayoutSupported` accepts only the arrangements above, including disabled external input, and never assumes input channel count equals output channel count.

This is an **engineering choice**, based on the official routing evidence. It preserves the preferred single-device workflow and avoids a redundant effect product before a real host limitation has been demonstrated.

## DSP contract

1. Read the enabled external input bus with `getBusBuffer(buffer, true, 0)`.
2. For mono use that sample; for stereo compute `(L + R) * 0.5` before writing output. The hardware model is mono, so the default design is a mono fold-down, not two independent SH-09 filters. Opposite-phase stereo material can cancel and this must be documented.
3. Apply the researched input sensitivity/preamp and overload behavior. The precise calibration is governed by SH-09 evidence and later hardware measurements, not by VST3's nominal float scale.
4. Feed the external signal to the shared mixer/filter/VCA path and feed its envelope detector at the documented circuit tap.
5. Use the detector according to the researched hardware routing. The follower is distinct from a MIDI-triggered ADSR. The UI should make input level and follower activity observable.
6. Disabled or absent bus produces zero input while synth notes continue. A silent input must not leave stale audio behind. Changes of bus configuration must not allocate in the process callback.

The input and output may share memory. Clearing the entire process buffer before reading the external bus would destroy the external audio. Read input samples before writing output, or copy through preallocated fixed-capacity chunks. Do not clear input-only channels.

## Live workflow

1. Add SH-09 to a MIDI track and an audio source to another track.
2. Expand the SH-09 device's Sidechain panel, enable Sidechain, choose the source track under Audio From and choose the desired tap (usually Post FX).
3. Set the sidechain mix to 100% and establish a known gain. Open the plug-in's external-input preset or set its input level/filter/VCA controls explicitly.
4. Prevent the source track's direct output from confusing the test, using appropriate source monitoring/output routing. Verify SH-09's output is the only monitored processed path.
5. Check external input/follower activity, then move cutoff, resonance and follower controls. The external signal can become audible through this plug-in's DSP because it deliberately routes auxiliary samples into its audio path.
6. Save the Live Set, close/reopen it, and confirm patch and source routing survive. This was observed in the shipped test Set; see LIVE_TEST.md.

If the integrated device panel is absent, the documented alternative is the source track's Audio To chooser: select the SH-09 MIDI track, then select the plug-in sidechain/input destination in the lower chooser. This is a documented route to test, not a guarantee that the present binary is discoverable under that name.

Ableton's generic sidechain prose describes compressor-like detectors where the trigger is not heard. It does not impose a VST3 ban on using those input samples as audio; this instrument's intentional processing path determines audibility. [Live plug-in sidechain controls](https://www.ableton.com/en/live-manual/11/working-with-instruments-and-effects/#sidechain-parameters)

## Required evidence and fallback trigger

Record a screenshot or exact UI observation of the discovered input, the Live version, source tap, gain/mix, plug-in settings and output capture. Verify mono source, stereo source, silence, low/high level, follower response, filter movement, source change, disabled sidechain, MIDI plus external audio, transport stop/start and Set reopen. Distinguish Live routing evidence from direct DSP or validator evidence.

If both documented routes fail after verifying correct `kAux` metadata and successful input processing in an independent test host, implement a second **SH-09 External** effect target. It must use the same engine classes, filter, VCA, follower, gain staging and saturation. Only bus adapter/default presentation should differ. It receives the ordinary audio-track main input, appears as `Fx Filter`, and needs a separate persistent product identity. That fallback is conditional on an observed host problem; it is not justified solely by uncertainty or by a blocked Live license dialog.

If Live cannot be operated in the environment, preserve the preferred architecture, maximize direct-host tests, and clearly mark Live workflow acceptance as unverified. Do not assert either that a single instrument is reliable in the installed Live version or that a fallback is required without that evidence.

## Compiled-module evidence, 2026-09-27

Steinberg VST 3.8.1 validator actually loaded the compiled SH-09 0.9.0 module and reported audio input 0 `External Input` as **Aux, default active**, audio output 0 `Output` as **Main, default active**, and event input 0 `MIDI Input` as **Main, default active**. The fresh-build log `artifacts/Validation/sh09-steinberg-fresh.txt` records 47 tests passed and zero failed.

The project VST3 host also exercised mono, stereo and disabled external input, equal left/right contribution, antiphase cancellation, filter cutoff response and a follower-driven increase in filtered output energy without MIDI. Non-finite and extreme finite input remained bounded at the output. These establish the compiled module's bus and processing behavior in independent test hosts. They do not substitute for Live UI/routing/project-reload observations.
