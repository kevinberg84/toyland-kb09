# User interface

The instrument has a native JUCE vector panel with the original SH-09 module order: Modulator, VCO, Sub Oscillator, Audio Mixer, VCF, VCA and Envelope. A left controller section provides volume, VCO/VCF bend depth, portamento and a spring-return bender. The 32-key keyboard spans MIDI 53-84 (F3-C6 using middle C = C4).

## Controls

* Drag faders vertically; drag rotary knobs horizontally or vertically. Double-click a sound control to return to its initial value. Tuning displays cents; continuous panel faders use a 0-10 display. Choice controls show their selected mode.
* The Factory Patch menu loads the complete sound settings. Branding remains independent of presets. Custom edits are retained in host state rather than given a new factory program name.
* The lower strip contains Authentic/Modern selection and optional condition, velocity, aftertouch, LFO tempo sync and input trim. Modern-only controls are dimmed and disabled in Authentic mode; their values remain available when Modern is re-enabled. Age remains separately available.
* Clicking and dragging over the keyboard sends note events through a fixed-capacity single-producer/single-consumer queue. The drag keeps gate legato by pressing the destination before releasing the old note. Note release also occurs when the keyboard loses focus, becomes hidden or is destroyed.
* Held notes are tinted; the currently selected lowest note receives a thin gold marker. The keyboard display reads atomic state published by the audio processor. It does not share the DSP engine or a locking MIDI-keyboard state object.
* The mouse bender returns to center when a drag ends. Its latest value reaches the processor through an atomic handoff. MIDI pitch bend also controls the engine, independently of mouse position; the GUI bender does not visually mirror incoming MIDI bend.
* ENV/EXT indicators show envelope and external follower activity. EXT AUDIO opens instructions over the keyboard explaining external routing and VCA Hold. The overlay intercepts mouse clicks so reading it cannot accidentally play hidden keys.

## Scaling and appearance

The default editor size is 1280 x 660, constrained to the same aspect ratio between 960 x 495 and 1920 x 990. A transformed native component tree scales controls, labels and hit targets together. JUCE provides the device-scale rendering layer. Host-specific high-DPI behavior must be tested in the target host; scalable drawing alone is not a claim of passing every display scale.

The physical reference was the original owner's manual panel diagram and first-hand photographs of real instruments, including two units shown together on the [Still Not Working repair page](https://snw.lonningdal.no/sh09.php). Dark casing, white module outlines, silver rotary caps, black slider caps, key arrangement and section order follow those references. Gradients provide dimensional shading. Condition adds deterministic panel scratches, screw discoloration and warm key discoloration.

This is a **functional vector reconstruction**, not a photographic asset reproduction. Exact typography, knob geometries, toggle mechanisms, panel proportions and wear patterns remain interpretations. Native choice menus replace the original physical switches for clear interaction. The power indicator is decorative; host bypass remains the host's responsibility. The original logo symbol is not traced. No photorealistic or pixel-identical claim is supported.

## Branding and runtime resources

`Assets/Authentic/Branding.h` and `Assets/AlternativeBranding/Branding.h` hold identity strings independently of DSP and layout. The Branding parameter selects Roland SH-09 or Private Mono 09 text. The alternate identity is prepared for replacement, without a trademark-clearance claim.

No downloaded photograph, remote image, generated raster image or external font file is required at runtime. Arial is requested through the system font API; JUCE/system fallback applies where unavailable. All panel geometry is compiled into the binary. The references under Research are evidence, not runtime dependencies or redistributed UI textures.

## Review notes

A fresh source review checked parameter attachment ownership, timer shutdown, destruction order, MIDI/GUI separation, keyboard release and scaling. The explicit attachment objects are destroyed before their sliders and choice controls. A discovered parent-paint help overlay would have been hidden by the child keyboard; it was replaced by a child overlay. Processor review separately identified queue-overflow recovery and duplicate-note display accounting for correction by the processor owner. Compile, validator and visible-host results belong in `VALIDATION.md`; this document does not substitute for those results.
