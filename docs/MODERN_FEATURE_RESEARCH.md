# Modern features and modular naming research

Research date: 2026-09-26. This is a prioritization record, not an implementation checklist marked complete. The user's stated priorities are the primary requirements. Manufacturer feature sets are evidence of available workflows, not a representative survey of user preference.

## Evidence and priorities

Roland's SH-101 software adds tempo-related features, an extra envelope and additional modulation choices. TAL documents preset browsing, MIDI integration and microtuning. Softube's Model 82 adds drive/doubling outside its core voice. D16 expands into multiple layers and integrated effects. The convergent opportunity is convenient recall and expression; the larger products also show how quickly optional complexity can grow. [Roland SH-101](https://www.roland.com/global/products/rc_sh-101/), [TAL-BassLine-101](https://tal-software.com/products/tal-bassline-101), [Softube Model 82 manual](https://www.softube.com/user-manuals/model-82-sequencing-mono-synth), [D16 Lush 2](https://d16.pl/lush2).

**VERIFIED:** These manufacturers document those functions. **INFERRED:** They are reasonable workflow candidates for this project. No representative SH-09 user survey was found or performed; do not claim quantified demand.

| Candidate | Proposed priority | Reason and Authentic-mode boundary |
| --- | --- | --- |
| Preset browser, full recall, readable values | Required workflow | Lets users retain patches and compare settings; patch recall must cover every sound-affecting option. |
| Resizable panel and accessible external-input meter | High | Directly helps the requested desktop/DAW workflow without altering synthesis. |
| Quality selection | High after measurements | Explicit CPU/alias tradeoff; choose from measured modes and preserve timing/latency correctly. |
| Velocity to VCA/filter, aftertouch/mod-wheel amount | Small optional extension | Adds expression; every route defaults to zero in Authentic mode. |
| LFO tempo sync | Optional | Useful in a DAW; free-running original behavior remains default. |
| Pitch-bend range | Optional | Explicit range setting; retain the documented original panel response in Authentic mode. |
| MIDI Learn | Optional after host automation | Requires stable map serialization and safe real-time lookup; DAW mappings may already meet initial needs. |
| External-input follower sensitivity/readout | High workflow | Display and digital level adaptation must be distinguished from original hardware controls. |
| Alternate tuning | Defer unless core gates pass | Valuable specialist workflow but requires tuning/state tests. |
| Sequencer, unison, polyphony, second envelope, effects | Defer | Large behavioral/CPU/state/UI scope; not prerequisites for the requested SH-09 recreation. |
| Randomize/parameter locks | Defer | Useful only with careful bounds and complete undo/recall; must not disguise calibration uncertainty. |

This table is a research recommendation. The final README and release notes must describe only implemented, tested options.

## Authentic and Modern state contract

**INFERRED, updated to implemented behavior:** Show an obvious Authentic/Modern state. Entering Authentic disables the effect of velocity, aftertouch, tempo sync and input trim while retaining their stored settings for returning to Modern. It does not reset those settings; this supersedes the original reset-on-entry proposal. Condition remains a separately explicit option. Factory patches serialize all implemented parameter values and mode; quality is fixed at 4x and condition is deterministic rather than a separately selectable seed. Modern-dependent factory patches identify Modern in their names.

A preset labeled Authentic with Age zero denotes a healthy calibrated target. The label describes intended topology/control behavior; it is not a measured-accuracy certificate. Sound-changing condition is an explicit extension even when the main panel resembles the hardware. A cosmetic-only branding switch should not touch audio parameters, automation IDs, preset contents or VST3 class identity.

## Condition feature

Condition is expressly required by the user. Implement a deterministic shared seed for visual wear and sonic calibration offsets. Reopening the editor must not create new scratches or retune the instrument. Increasing condition should reveal modest fading, track wear, scratches and key discoloration while sonic changes remain bounded. Physical component mechanisms, correlations and outstanding measurement needs are documented in `DSP_RESEARCH.md`.

The UI must not label a numeric Age value as a literal number of elapsed years: maintenance, calibration history and component types prevent a universal age-to-sound mapping. Save the unit profile. Provide a healthy reset that restores both visuals and audio behavior.

## Naming search

This was a limited current-use search, not legal clearance. Queries included `Toyland synthesizer`, `Toyland plugin audio`, `Toyland VST`, `Toyland music equipment software`, `TH-09 synth`, `TH-09 plugin`, and `TH09 synthesizer`.

**VERIFIED current page:** Toyland Recording Studio has an active public site describing recording, editing, mixing and mastering services. Its page says the previous Cunningham Street location is no longer operating and directs clients to discuss recording options. That is adjacent audio-market use, sufficient to avoid treating TOYLAND as an obviously unused audio name. [Toyland Recording Studio](https://toyland.com.au/).

The limited searches did not identify a directly matching TOYLAND TH-09 software instrument. Absence from these results does not establish availability, registrations, jurisdictional rights or permission. TH09 also appears in other industries, including a sensor part, which makes an unrestricted uniqueness claim inappropriate. [HOPERF TH09 product](https://www.hoperf.com/sensor/temperature_sensor/TH09.html).

Proposed neutral/private alternatives are **K-09 Private**, **Monoline 09**, and **Nine Circuit**. Exact-phrase searches with `synthesizer` produced no direct competing software instrument in the inspected results. These are working concepts only; none is trademark-cleared. Prefer a neutral private-use wordmark asset plus separate model text while keeping the requested close private skin as a replaceable option.

## Asset boundary

**INFERRED implementation:** Store maker text, model text, optional original artwork, accent colors and attribution separately from layout/control code. Make the private and neutral asset sets render through the same control geometry. Do not copy a commercial emulator's logo drawing, background, controls or presets. Public photographs can inform proportions/materials without becoming a single interactive background photograph.

No commercial release, trademark application or protected-system installation is authorized by this research document. Final branding remains a private project choice rather than a legal conclusion.
