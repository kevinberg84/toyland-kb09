# Toyland KB-09 identity and compatibility

Version **0.11.0** presents one instrument: **Toyland KB-09**, by **Toyland**. `Assets/Branding.h` supplies the maker/model/display constants and processor name; CMake supplies matching product/company metadata. The editor displays the supplied **KEVINBERG 09 / KB-09** mark on the left and **Toyland** on the right. No alternate branded instrument is selected by a parameter.

## Preserved identity and sound state

The internal target/class remain `SH09` / `SH09Processor`. Manufacturer code `PrIv`, plugin code `Sh09`, bundle ID `local.privateinstruments.sh09`, state root `SH09State`, schema **1**, and all **35** parameter IDs/order remain unchanged. They are compatibility keys rather than visible branding.

JUCE 9.0.2's VST3 module-info implementation derives component/controller identities from manufacturer code, plugin code and interface type when VST2 replacement is disabled, as here. The changed product/company strings therefore preserve those IDs on this build path. A host may need a rescan for its displayed name/filename. Expose one identity in scanned plugin folders; legacy SH-09 and renamed KB-09 are not independent side-by-side instruments.

Schema compatibility is distinct from identical sound. **0.11 Modern LFO MOD maxima are ±36 semitones VCO / ±8 octaves VCF; Authentic remains ±12/±5.** Old Modern patches with nonzero MOD can sound deeper. Parameters and their normalised state still load; these are deliberate engineering mappings, not measured hardware ranges. Bender sensitivity controls are unchanged.

## User preset files

SAVE writes the complete current schema-1 binary XML sound state as `.kb09`; LOAD also accepts compatible legacy `.sh09state` exports, including the supported JUCE-hosted wrapper. The files preserve parameter values and the factory-program reference while retaining all sixteen factory programs. This is not a new factory-bank format or a generic `.vstpreset` importer.

Files are bounded and validated before state changes; unsupported/malformed files leave the sound unchanged. Saving checks a temporary file before replacement. Asynchronous dialogs use weak callback/owner checks so closing the editor does not leave callbacks accessing it. A loaded filename remains visible until a factory recall; a processor program-revision counter makes same-index factory recall restore the correct name too.

State does not include held keys, oscillator/filter/envelope histories, power or window placement. Power is a transient processor-owned mute/reset control; it keeps callbacks running, consumes MIDI while off and prevents old held notes from returning. External Hold processing may resume without MIDI. Power and program-revision regressions are included in the recorded 20 processor checks.

## Legacy branding slot and assets

The original `branding` parameter retains its index, ID, range, default and two stored values. It is named **Legacy Compatibility**, is nonautomatable, and both labels read **Toyland KB-09**. It does not reach DSP or select assets; factory recalls preserve the inert value. A host's generic parameter view can still expose this compatibility slot because it is omitted only from the custom editor.

`isMetaParameter()` is not a hidden-parameter flag. No JUCE framework modification was made to hide the slot. Legacy research/test identifiers remain intentionally historical.

Age blends isolated patina and yellow/chipped ink gradually, while separate stain events retain their fixed size and opacity. Aged mark coverage is multiplied by the original supplied letter masks, so wear removes ink without redrawing letterforms or adding marks outside them. The nine embedded runtime PNGs include separate clean/aged supplied marks; the stain and key-crack layers do not replace the branding. The single identity remains constant at every Age.

## Evidence scope

The final October 7 compiled GUI fixture reports **280 assertions without failures**, including original parameter IDs/indices, schema-1 round trips for both legacy branding values and the single processor identity. Processor tests report **20 passes, zero failures and zero tracked C++ allocations** in covered callbacks, including same-index program-revision handling; preset-file tests report **58 passes and zero failures**. These logs use `-0.11-aging-events-final.txt` under `artifacts/Validation-KB09`. The actual final VST3 host reports **75 passes and zero failures**, including all sixteen factory preset exports/round-trips. GUI-enabled pluginval strictness 10 and Steinberg validation also pass with confirmed exit 0.

The build, staged and installed final module share SHA-256 `46BE3DC797F48DCC1468AEF235CE2B1E40938E9ED885275F42397B443155DACC`. [VALIDATION.md](VALIDATION.md) and [ACCEPTANCE.md](ACCEPTANCE.md) identify the tested artifacts. Live-specific 0.11 recall was not repeated because desktop automation could not resolve Live's foreground process after three recovery attempts; [LIVE_TEST.md](LIVE_TEST.md) preserves earlier observations separately. Compatibility tests do not establish every DAW's cache or project-recall behaviour.

The chosen name follows the user's instruction and is not a trademark-clearance claim. Earlier naming research remains in `MODERN_FEATURE_RESEARCH.md`.
