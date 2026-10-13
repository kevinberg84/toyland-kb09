# SH-09 / KB-09 recording session

Purpose: explain the staircase of pitches during a slow VCF sweep, then compare
hardware and model fairly. No internal hardware measurements or adjustments.
Start with the essentials; the optional takes can wait.

## Before recording

- Warm the SH-09 for about 15 minutes. Note the warm-up time and room conditions
  if convenient; there is no need to control them precisely.
- Main mono output into an interface **line input**. Prefer mono 24-bit/96 kHz
  WAV; 48 kHz is useful too. Render KB-09 directly at the same sample rate;
  32-bit float software export is also fine.
- Turn off effects, EQ, compression, limiting, noise reduction, normalization,
  automatic gain, and interface coloration such as a vintage preamp mode.
- Set gain using the loudest planned take, leaving headroom (roughly -12 dBFS
  peak is useful). Keep interface gain and instrument VOLUME fixed within the
  set. If clipping requires a gain change, label it and repeat that set.
- Save the KB-09 preset **before changing anything**. Capture the exact bad
  patch first, including its current AGE and waveform. Then use Authentic /
  Minty for the controlled model comparisons.
- Note the interface, gain setting, sample rate, KB-09 version, waveform, key,
  RANGE, TUNE position, VOLUME, RES, FREQ, VCF MOD and AGE. A clear panel photo
  and saved preset are easier than transcribing every control.

## Essential takes, in order

| Take | Settings / action | Duration | Why it matters |
|---|---|---:|---|
| Exact symptom | KB-09 patch that makes the staircase obvious; save preset | 20–30 s | Establishes the sound we must explain |
| Isolated filter sweep | VCO/SUB/EXT 0; RES max; slow sine LFO to VCF | 20–30 s | Baseline smoothness, actual range and sweep speed |
| Mixed sweeps | Same patch, VCO mixer 2, 5, then 10; three takes | 20–30 s each | Shows how input strength changes the steps |
| VCO reference | Same key, waveform and RANGE; VCF MOD/ENV 0, FREQ max, RES 0; VCO 2, 5, 10 | 5–8 s per level | Actual VCO pitch, harmonic shape and output-level reference |
| Fixed filter references | VCO/SUB/EXT 0; VCF MOD/ENV 0; RES max; three steady audible filter pitches, low/middle/high | 8–10 s per pitch | Self-oscillation level, waveform and pitch references |

Do the controlled rows on both instruments where practical. Roughly 100 Hz,
440 Hz and 1 kHz are convenient fixed filter targets, **not requirements**.
Any three stable pitches across a usable shared band help; a tuner is optional.
Record the VCO and fixed-filter references without changing VOLUME or capture
gain. These are output references, not direct probes of internal circuit nodes.

For the slow sweep set: VCO MOD 0, SUB/EXT 0, VCF ENV 0, VCA HOLD,
PORTAMENTO 0, bender centered, sine LFO, RATE minimum. The VCO stays steady;
**VCF MOD sweeps the filter's self-oscillating sine**. Choose a shared audible
band that passes the VCO pitch and, if feasible, a few higher harmonics. Keep
FREQ / VCF MOD / RES unchanged when changing only the VCO mixer level.
Record complete rising **and falling** sweeps, not just the upward portion.

Leave two seconds before/after takes and include a few seconds with the VCA
closed / sources quiet to characterize capture noise. VCO/SUB/EXT at zero with
RES max is **not** silence: the filter can still oscillate.

## Useful extras, only if convenient

1. At VCO 5, repeat with RES just below self-oscillation. This helps separate
   emphasizing VCO harmonics from an independently oscillating filter. With
   VCO down, verify that this setting does not sustain an audible tone.
2. Repeat the main mixed sweep with the fixed square waveform (not variable
   PWM), keeping the documented settings otherwise fixed. Its different
   harmonic content is useful evidence; a real square need not be perfectly
   symmetric or contain only odd harmonics.
3. Repeat one mixed take after moving TUNE a little, leaving the VCF settings
   and selected key unchanged. Record a short VCO reference at the new tuning.
   This tests whether the apparent steps follow the VCO. Changing the key is
   less clean because the SH-09 keyboard also controls the filter.
4. At one fixed filter pitch, VCO/SUB/EXT 0 and VCF MOD/ENV 0, slowly raise RES
   through self-oscillation onset, then hold maximum for five seconds.

## Matching and file handling

Close settings are valuable; exact capture-time matching is unnecessary.
Keep the hardware in a useful range. We can fit the software note/range/TUNE,
filter sweep endpoints and LFO rate to the hardware's measured behavior.
Matching measured Hz and drive relationships is more meaningful than forcing
equal-looking fader positions. Do not pitch-shift, stretch or normalize the
original audio. No sample-accurate LFO start synchronization is needed.

Keep raw files. Separate WAVs or one labeled continuous recording both work.
Suggested names: `SH09_saw_vco05_sweep.wav`, `KB09_saw_vco05_sweep.wav`,
`SH09_saw_vco_refs.wav`, `SH09_filter_fixed_refs.wav`.
Identify slider movements and scratchy/worn-control noises in notes. Do not
edit them out of the originals. Keep a copy of the starting preset/settings.

## Sources and interpretation

The [Roland owner's manual](https://thismodernday.com/wp-content/uploads/2021/07/Roland-SH-09-Owners-Manual.pdf)
describes the warm-up, sine modulation, self-oscillation, HOLD and permanent
keyboard-to-filter control. The [service notes](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf)
give calibration criteria and the actual filter circuit. Neither guarantees
that a particular surviving unit is in factory calibration. These recordings
measure Kevin's SH-09 in its present condition; suspected faults will be kept
separate from conclusions about the intended design.
