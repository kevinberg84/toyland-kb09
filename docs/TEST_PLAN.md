# Acceptance record

The project remains in development until the validation record records actual results. Research classifications do not substitute for executable tests.

1. Disposable VSTLab: compiled x64 Release with JUCE 9.0.2, loaded through JUCE VST3 format, generated audio at five rates and seven block sizes, state recall and repeated editor creation passed; pluginval level 5 passed. Logs are under `artifacts/Validation`.
2. Core DSP: frequency/range/bend, coherent sub division, PWM, LFO/delay, ADSR/retrigger, low-note priority, external detector and filter numerical stress.
3. Actual SH-09 module: VST3 scan/load, enabled/disabled/mono/stereo auxiliary input, offset MIDI, state/presets, automation, GUI lifecycle, many sample rates and arbitrary blocks, timed long render.
4. Independent validators: pluginval and Steinberg validator, with exact paths, versions, exit status and logs retained.
5. Live: custom scan, instrument UI, MIDI audio, auxiliary source selector and audio, follower/VCA Hold, automation, transport, project reload and instance lifecycle. Mark anything inaccessible or unperformed explicitly.
6. Freeze, fresh Release build, repeat acceptance tests after final code changes, then package.

Future physical-unit comparisons use `tools/calibrate.py`. No real-unit calibration result may be asserted without supplied captures and metadata. GUI photorealism must be visually inspected rather than inferred from paint code.
