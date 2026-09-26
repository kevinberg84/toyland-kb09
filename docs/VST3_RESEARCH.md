# JUCE 9 / VST3 engineering research

Research date: 2026-09-26. The installed framework at `C:\Dev\JUCE` is the authoritative implementation reference for this build; public documentation links supplement it. This is an engineering research record, not a record of completed plug-in tests. Actual results belong in `VST3_SMOKE_TEST.md` and `VALIDATION.md`.

## Build and identity

**Verified requirements.** JUCE's CMake API provides `juce_add_plugin`, `IS_SYNTH`, `NEEDS_MIDI_INPUT`, `VST3_CATEGORIES`, persistent four-character manufacturer/product codes, and `COPY_PLUGIN_AFTER_BUILD`. Leaving a product code unspecified can generate a different code on reconfiguration. Automatically copying plug-ins may target protected folders. [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md); local `docs/CMake API.md`, lines 517–526, 531–598, 642–660.

**Project choices.** Use C++17 or later; build only x64 VST3 and useful test executables; explicitly set a stable company code, product code, bundle identifier and version. Instrument metadata is `IS_SYNTH TRUE`, `NEEDS_MIDI_INPUT TRUE`, `NEEDS_MIDI_OUTPUT FALSE`, and `VST3_CATEGORIES Instrument Synth`. Do not create an automatic system installation step. Keep framework location configurable and source independent of absolute runtime asset paths. Compile resources into the binary.

Use the installed Visual Studio generator with `-A x64`, then `cmake --build <build> --config Release`. With multi-configuration generators the build selects Release, not `CMAKE_BUILD_TYPE`. Preserve the build command, tool versions and logs. A separate empty build directory is needed to establish reproducibility. Microsoft documents the Visual Studio developer environment and CMake command-line workflow. [Microsoft CMake documentation](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170)

## AudioProcessor and buses

**Verified requirements.** `prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock)` receives an estimate. JUCE explicitly permits changing block lengths, zero-length blocks, and blocks exceeding that estimate. The process buffer contains bundled bus channels; `getBusBuffer` selects the correct bus. Channels that are input-only may be read-only. Output-only channels may initially contain garbage. [AudioProcessor API](https://docs.juce.com/master/classjuce_1_1AudioProcessor.html); local `modules/juce_audio_processors_headless/processors/juce_AudioProcessor.h`, lines 118–215.

**Project choices.** Query actual sample count and bus channel counts on every block. Render directly sample by sample, or chunk through fixed-capacity prepared storage. Do not resize storage if the host exceeds its expected block size. Write every enabled output sample, without touching input-only channels. In-place input/output aliasing requires consuming external input before output overwrites it. Disabled external input supplies exact zero. Recompute time constants and frequency limits at every preparation; do not assume 44.1 or 48 kHz.

VST3 distinguishes main and auxiliary audio buses and lets hosts activate/deactivate buses. Bus default-active flags are requests rather than guarantees. [Steinberg bus specification](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/group__vstBus.html)

For the single-instrument input design, use `VST3ClientExtensions::getPluginHasMainInput() == false` so JUCE marks its first audio input `kAux`. Otherwise JUCE marks that first bus `kMain` even when the product is a synthesizer. This is directly verified in local `modules/juce_audio_processors_headless/utilities/juce_VST3ClientExtensions.h`, lines 99–105, and `modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp`, lines 3109–3127. [JUCE VST3 client source](https://github.com/juce-framework/JUCE/blob/master/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp)

## MIDI and automation timing

**Verified.** Incoming `MidiBuffer` events carry a sample position within the current block. Rendering the whole block then applying notes destroys this timing. `MidiMessageMetadata` gives non-owning event bytes and a sample position; constructing an owning `MidiMessage` for an arbitrary SysEx message can require storage. Local `juce_audio_basics/midi/juce_MidiBuffer.h`, lines 48–77; [MidiBuffer API](https://docs.juce.com/master/classjuce_1_1MidiBuffer.html).

**Project choices.** Render until the next event offset, apply all events at that offset in their given order, and continue. Test note-on and note-off at zero, mid-block and the last valid sample. Ignore unsupported long messages without copying their payload. Treat velocity-zero note-on as note-off, handle all-notes-off, all-sound-off and pitch wheel, and deliberately define sustain behavior. GUI keyboard input should enter a bounded event queue, not call mutable DSP objects from the UI.

**Important limitation verified in JUCE 9.0.2 source.** The VST3 wrapper's `processParameterChanges` reads the last point from each ordinary parameter queue and calls the JUCE parameter setter before `processBlock`. MIDI-controller emulation instead transfers individual queue points to MIDI with offsets. Therefore ordinary APVTS automation through this wrapper must not be advertised as sample-accurate VST3 automation. See local `juce_audio_plugin_client_VST3.cpp`, lines 3500–3540. Control smoothing reduces discontinuities but cannot recover discarded host automation points.

**Project choices.** Expose stable parameter IDs and explicit ranges/units. Read cached `std::atomic<float>*` parameter pointers on the audio thread. Smooth audible continuous controls at the internal processing rate using sample-rate-derived ramps/time constants; use appropriate exponential/log-frequency mapping for cutoff. Discrete modes require a deliberate safe transition. Pitch glide and the hardware envelope are musical behavior, not interchangeable with generic automation smoothing. [SmoothedValue API](https://docs.juce.com/master/classjuce_1_1SmoothedValue.html); local `juce_audio_basics/utilities/juce_SmoothedValue.h`.

## State and thread ownership

**Verified.** APVTS offers parameter attachments and atomic raw parameter access. `copyState` and `replaceState` are thread-safe but explicitly not real-time-safe because they lock. [APVTS API](https://docs.juce.com/master/classjuce_1_1AudioProcessorValueTreeState.html); local `juce_audio_processors/utilities/juce_AudioProcessorValueTreeState.h`, lines 337–404.

**Project choices.** Save a versioned state tree in `getStateInformation`, restore validated trees in `setStateInformation`, and keep both out of render-loop code. Reject malformed data, invalid roots and non-finite parameters. State includes every sound control, mode, preset choice and deterministic condition seed; editor size and branding can be separate documented UI properties. Preset changes on an audio callback must not parse XML or replace a ValueTree. State reload must reach DSP via atomics or a bounded state handoff.

Audio processing owns oscillator, filter, envelope and note-stack mutation. The GUI reads only published atomic meters/note state and updates on a message-thread timer. Attachments handle host gestures and UI parameter synchronization. The callback does not allocate, free, lock, perform I/O, log, send synchronous messages, or create GUI objects. Preallocate event/storage buffers; do not mistake JUCE facilities containing locks for guaranteed real-time-safe APIs. Use `juce::ScopedNoDenormals` at the callback boundary and bound numerical recurrences. Verify denormal/finite behavior separately from subjective sound quality. Local `juce_audio_basics/buffers/juce_FloatVectorOperations.h` documents the denormal guard.

## Editor and high DPI

**Verified.** `AudioProcessorEditor` supports `setResizable`, `setResizeLimits`, `getConstrainer`, and host `setScaleFactor`. Local `juce_audio_processors/processors/juce_AudioProcessorEditor.h`, lines 128–200; [editor API](https://docs.juce.com/master/classjuce_1_1AudioProcessorEditor.html).

**Project choices.** Use logical coordinates, vector-painted panel/control geometry, a fixed aspect ratio, minimum readable dimensions and embedded assets. Repaint only the necessary areas; generate deterministic wear once or from a stable seed. Exercise native editor creation, destruction, resize and Windows scaling changes. A generic host UI or a successful editor-construction test does not establish realistic appearance or interaction quality.

## Validation layers

1. Unit/DSP regression tests exercise numerical properties and control behavior.
2. A real VST3 loader must instantiate the built module, negotiate buses, deliver MIDI and external audio, inspect nonzero finite output, automate controls and restore state.
3. Tracktion pluginval exercises common host behavior, lifecycle, variable block/sample rates, parameter changes, state and GUI. The downloaded version recommends strictness 5 as a compatibility minimum; higher levels add stress/fuzzing. [pluginval project](https://github.com/Tracktion/pluginval)
4. Steinberg's validator is an independent VST3 conformance host, available from its official SDK. Passing it establishes API checks, not hardware emulation accuracy or Ableton workflow. [Steinberg validator](https://steinbergmedia.github.io/vst3_dev_portal/pages/What%2Bis%2Bthe%2BVST%2B3%2BSDK/Validator.html)
5. Actual Live scan, editor, MIDI sound, automation, routing, transport and project reload tests remain essential. Live is installed locally; access or licensing blockers must be recorded instead of calling these tests passed.

Suggested matrix: 44.1/48/88.2/96/192 kHz; 0, 1, 17, 64, 257, 1024 and 8192 samples; bus disabled, mono and stereo external audio; rapid notes; overlapping/repeated notes; maximum resonance/modulation/drive; repeated preset and editor changes; sustained rendering and complete instance destruction. A host may reject a particular arrangement; record both requested and accepted arrangements. Repeat relevant tests after fixes and against the final clean Release binary.

See `tools/VALIDATORS.md` for exact acquired tool provenance and invocation. Test results are deliberately not inferred from this research.
