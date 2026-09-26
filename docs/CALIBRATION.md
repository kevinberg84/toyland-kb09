# Hardware recording comparison

`tools/calibrate.py` imports PCM 8/16/24/32-bit or IEEE float WAV and produces JSON plus spectrum and optional transfer CSV files. It requires Python and NumPy. No hardware recording has been supplied, so the tool is a future measurement framework, not a validation result.

Record raw waveforms, filter sweeps, resonance/self-oscillation, ADSR and follower bursts, saturation levels, and noise separately. Keep the unit/revision, warmup time, interface gain, sample rate, exact control positions, note, input level and recording time in an adjacent text file. Disable interface processing and DAW normalization. Preserve raw takes.

Examples (run from the project folder):

```
python tools/calibrate.py --stimuli --out artifacts/Calibration/Stimuli
python tools/calibrate.py hardware-saw.wav --start 1 --duration 2 --fundamental 440 --out artifacts/Calibration/Saw
python tools/calibrate.py hardware-output.wav --compare model-output.wav --out artifacts/Calibration/Compare
python Tests/test_calibration.py
```

Frequency uses a Hann-window FFT with log-peak interpolation. Supply the known fundamental for waveforms whose strongest partial is not their fundamental. Harmonics integrate a small frequency band; reported THD includes up to harmonic 32 within Nyquist. RMS, peak, DC and a 5 ms RMS envelope provide level and transient evidence. The first 10/50/90% crossings are relative to the observed peak and do not automatically identify an ADSR stage.

Transfer CSV uses averaged cross/auto spectra and includes coherence and an excited-bin flag. Only interpret it for aligned captures driven by the same excitation. Free-running VCO captures are unsuitable for phase comparison. Different analog takes may need deliberate time alignment; the tool never silently normalizes gain or aligns away an error. Filter sweeps should be small-signal; saturation tests need stepped levels and identical gain. For noise, compare spectra of equal-length excerpts and RMS; one random waveform cannot be directly subtracted from another.

Synthetic regression tests establish the analysis tool's frequency/level/transfer bookkeeping. They do not establish hardware fidelity. Record hardware-versus-model deltas and uncertainty before changing calibration constants.
