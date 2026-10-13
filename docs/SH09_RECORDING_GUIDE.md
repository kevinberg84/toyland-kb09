# SH-09 comparison recordings

For the next session, use [the shorter session checklist](SH09_SESSION_CHECKLIST.md).
It prioritizes the reported VCF staircase, includes isolated level/pitch
references, and separates essential takes from optional calibration work.
The broader reference protocol below remains useful for later investigations.

Start with the first set below. No special analyzer or internal circuit probing
is needed. These recordings will help separate waveform shape, mixer drive and
filter behavior instead of guessing what produces the instrument's body/grit.

## Capture setup

- Let the SH-09 warm up normally and note roughly how long it has been on.
- Connect its main mono output directly to an audio interface **line input**.
  Record mono WAV, ideally 24-bit / 96 kHz; 24-bit / 48 kHz is also useful.
- Disable DAW effects, EQ, compression, limiting, noise reduction, normalization,
  and automatic gain. No microphone or room recording.
- Set a conservative input gain: the loudest test should stay comfortably below
  clipping (about -12 dBFS peak is a useful target). Keep interface gain and the
  SH-09 VOLUME unchanged across a set. If any clip distorts at the interface,
  lower gain and restart that set.
- Include a few seconds of silence with VCO/SUB/EXT at zero, and a panel photo
  for each setup. Tell us the interface, sample rate, VOLUME setting, range,
  and which keys you played. A single long recording with spoken/file notes
  between tests is fine; don't talk over the synth signal.
- No exact tuning is necessary. Keep TUNE fixed. We can measure the pitch.

## First priority: plain saw and square

Set RANGE to 8', VCO mixer to 10, SUB and EXT to zero, VCF FREQ fully up,
RESONANCE zero, both VCO and VCF MOD zero, VCF ENV depth zero, VCA HOLD,
PORTAMENTO zero, bender centered. Use the fixed square waveform, not PWM, for
the square set. Turn listening volume down before engaging HOLD.

For **saw**, then **square**, record each of these keys for about five seconds:

1. Lowest F on the keyboard.
2. First C above that F.
3. Next C one octave higher.
4. Highest C.

Leave a short gap between keys/recordings. HOLD keeps the last pitch sounding
after releasing a key, so separate clips or short mixer-down gaps are fine.
Please do not trim every clip precisely to a waveform boundary.

Then on the first C, repeat the saw at VCO mixer **2, 5, and 10**, five seconds
each, keeping VOLUME and interface gain fixed. This distinguishes the mixer's
drive response from simple output loudness. Photograph those positions.

## Second priority: the filter's own voice

Keep the same range, key, VOLUME, input gain, VCA HOLD and modulation settings.
Put VCO, SUB and EXT mixer levels at zero. Raise RESONANCE to maximum.

- Record five seconds each at several FREQ positions that produce low, middle,
  and high audible sine-like tones; include panel photos. Exact slider numbers
  are less important than stable, documented positions.
- Sweep FREQ slowly from low to high over about ten seconds, then back down.
- At one middle FREQ position, slowly increase RESONANCE from zero to maximum
  over ten seconds. Hold maximum for another five seconds. This captures onset
  and limiting, not just the final tone.

These can be loud; set the listening level appropriately. If the recording
clips, reduce interface gain and document the different gain for this set.

## Optional later tests

At the first C, saw VCO 10, SUB/EXT zero: slow FREQ sweeps at resonance zero,
about halfway, and just below self-oscillation. Then compare VCO alone, SUB
alone, and both at matched documented mixer positions. A short musical example
of the exact sound KB-09 fails to reproduce would also help.

Raw WAVs and panel photos are more valuable than processed demonstrations.
One SH-09 measures one instrument, not every unit ever manufactured. We will
keep its identifiable characteristics separate from optional AGE effects.

## Current priority: the slow VCF MOD stepping — 13 October 2026

For this investigation, start here rather than recording every test above.

Use the waveform and settings where the problem is obvious. On the SH-09,
set sine LFO, RATE minimum, VCO MOD zero, SUB/EXT zero, VCF ENV zero,
maximum RES, VCA HOLD, and keep one key/note selected with the bender centered.
Choose low FREQ and enough VCF MOD for a slow audible sweep through the steady
VCO's pitch. Keep FREQ, MOD, RES, TUNE, VOLUME and interface gain fixed.

Record about 20 seconds each with the VCO mixer at **0, 2, 5, then 10**.
Keep the original mono WAVs, preferably 24-bit/96 kHz (48 kHz is useful),
with no effects or normalization. Include a panel photo and identify the key,
range, waveform and interface. Repeat the same four takes on KB-09 in
Authentic/Minty; save its preset as well. A square-wave repeat can follow
after the first set, rather than making this first session unnecessarily large.

Exact pitch matching is unnecessary at capture time. We will measure the
hardware's VCO frequency and filter sweep, then fit the software note/range/
TUNE and modulation to that common band. The overall pitch-range mismatch does
not prevent this test. Do not digitally pitch-shift the recordings, and do not
adjust internal hardware trimmers. Keep output gain consistent so the
relationship between injected VCO and filter self-oscillation remains measurable.

## Slow VCO / filter beating

With VCA HOLD, SUB/EXT zero and maximum RESONANCE, choose a middle filter
self-oscillation pitch. Record the filter alone first, then add saw at VCO
mixer 5 and slowly move FREQ across the VCO pitch and back over 20 seconds.
Repeat with square/pulse. Keep recording level and VOLUME fixed; photograph
the waveform and pulse-width settings. If the modulation sliders are unreliable,
a careful manual FREQ sweep is useful. Describe any discontinuity when touching
a worn slider so it is not mistaken for the instrument's normal behaviour.

If practical, repeat with FREQ stationary and slowly changing VCO pitch.
Include five seconds at fixed settings before and after each sweep. This
comparison targets the bubbling Kevin hears when the two pitches move relative
to one another; an isolated oscillator recording cannot establish its cause.
