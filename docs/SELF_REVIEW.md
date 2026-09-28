# Release-candidate self-review

Reviewed the compiled architecture and source after the first main-instrument passes, including an independent second review of DSP and GUI by another engineering agent. No external proprietary code was inspected.

Corrected findings:

* GUI keyboard queue overflow previously cleared the engine and then replayed stale note-ons. Overflow now discards the queued backlog before allowing fresh events. A direct-processor regression confirms silence and recovery.
* GUI key display previously used booleans while the engine counted duplicate note-ons. MIDI and mouse now update matching bounded reference counts. A regression verifies a mouse note-off does not hide a remaining MIDI note.
* A parent-painted external-audio help panel was hidden by the child keyboard. It is now a dedicated overlay component that intercepts input.
* Slider attachments overrode initial value text formatting. Formatting and inverse conversion are installed after attachment creation.
* Mouse keyboard slides now enqueue the new key before releasing the old one, preserving a continuous legato gate. Bender wheel/key focus is disabled; spring return and destruction only reset an active UI bend.

DSP corrections and measurements are in DSP_VALIDATION.md. Independent review confirmed the source/control alignment and 128-sample resampling latency, fixed-size render storage, no explicit render locks, the external detector's pre-mixer tap, and the documented keyboard/envelope behavior.

Processor MIDI parsing uses raw short-message bytes so ignored long SysEx does not create copied MidiMessage storage on the render path. Parameter atomic pointers are cached at construction. XML state copying/restoration stays outside processBlock. External inputs are consumed before aliased output channels are written. Nonfinite external input is rejected. State input has size/type/schema and finite-value checks. The audio-thread C++ allocation guard recorded zero allocations in its tested normal processing path; this is not interception of every possible OS/library allocator.

Residual fidelity approximations are intentional and documented in KNOWN_LIMITATIONS.md. They are not marked as measured equivalence. Final binary checks follow a fresh configure/build with the old build directory preserved separately, followed by rerunning core, processor, actual-module and independent-validator checks.
