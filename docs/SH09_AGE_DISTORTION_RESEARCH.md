# SH-09 nonlinearities and AGE coloration

Research date: 2026-10-07. Documentary investigation and read-only source review; no hardware measurements, listening comparison or DSP change. **Recommendation: retain the existing nonlinear DSP. The evidence does not justify automatically adding distortion as AGE increases.**

## Verified circuit locations

The original [Roland SH-09 Service Notes, March 27, 1980](https://synthfool.com/docs/Roland/SH_Series/Roland%20SH09%20Service%20Manual.pdf) were opened through the web tool. Local renders `Research/rendered/filter-5.png`, `service-large-5.png`, `service-large-4.png` and `service-6.png` were directly displayed and visually inspected in this pass. The local source PDF is `Research/ROLAND_SH-09_SERVICE_NOTES.pdf` (851,698 bytes); the existing renders were used rather than freshly rendering it. References below use printed pages.

| Location | Circuit fact and interpretation |
|---|---|
| VCF, p. 5, OPH47 | IC11–14 are four selected BA662 OTAs with buffers Q15–18. The replacement note requires matching transconductance and checks self-oscillation. OTA large-signal compression is a plausible inherent nonlinearity; its exact transfer is not established by this schematic. |
| Resonance, p. 5 | Opposing D12/D13 connect the resonance-return node to ground. VR111 feeds that node; R118 is a 1.5 kΩ shunt and R109 returns through 12 kΩ to the first stage. The diodes limit opposite polarities of **audio-frequency feedback**. They are present in a healthy instrument, not an AGE-only serial output clipper. |
| VCA/output, pp. 5/4 | IC15 is another BA662. The following volume path includes coupling and Q38; the headphone amplifier is separate. No post-VCA antiparallel distortion pair is shown. |
| External input, p. 5 | Q32/Q33 feed both audio mixing and a separate follower branch. D19/D20 rectify/clamp the follower before smoothing. They are not a serial distortion stage in the external mixer feed. |

The normal semiconductor stages can run out of headroom when overdriven. That is circuit analysis, not evidence that the normal SH-09 line output is deliberately clipped at every setting. No measured volts-to-dBFS calibration, VCA THD curve or output clipping threshold is available here.

## Manufacturer evidence and its limits

- **Actual diode type:** Roland's schematic legend allows 1S2473 or 1S1555 for ordinary diodes. The original [ROHM 1S2471/1S2472/1S2473 data sheet](https://datasheet4u.com/pdf-down/1/S/2/1S2473_Rohm.pdf), mirrored manufacturer document, p. 2 / printed p. 73, gives current-dependent forward voltage and temperature-dependent forward/reverse curves (Figs. 1–2). Its 0.92 V typical forward value is specified at **100 mA**; it is not a fixed SH-09 clamp threshold. These curves support a nonlinear, operating-point-dependent limiter, not an age-dependent distortion law. Manufacturer text was available through indexed extraction; no numerical curve fit was performed.
- **BA662 limitation:** a complete original ROHM BA662A manufacturer data sheet was not obtained. The [Coolaudio V662A replacement sheet](https://www.coolaudio.com/docs/Datasheet_CA_V662A_2020-10-12_Rev.0_Signed.pdf) has indexed control-current/headroom curves, but direct retrieval failed; it is a replacement device and cannot establish exact original BA662 distortion. No LM13700, BA6110 or modern clone curve is treated as an original-part measurement.

## What aging could plausibly change

[Nichicon CAT.8101H](https://www.nichicon.co.jp/english/wp-content/uploads/CAT.8101.pdf), section 2-2, printed pp. 13–14 / PDF pp. 12–13, distinguishes degradation from catastrophic failure and describes capacitance loss, higher loss/ESR, leakage and electrolyte/seal deterioration. Conditions and failure mechanisms vary. This is general component evidence, not identification of the fitted SH-09 capacitor brands or a model for every film/polystyrene capacitor.

**Conditional inference:** if an individual unit's coupling, bias or supply components deteriorate enough, frequency response, noise, bias and available signal swing may change. Increased distortion is possible, but neither its amount nor its sign/asymmetry follows from calendar age alone. Analog Devices' [AN-581, “Zener Diode Biasing,” Figs. 4–5](https://www.analog.com/en/resources/app-notes/an-581.html) illustrates asymmetric clipping when supply falls relative to bias in a single-supply example. That mechanism is general context, not proof that this SH-09's regulated supply sags or that its OTA stages age in the same way.

The reviewed primary sources provide no universal relationship between instrument age, additional pleasant harmonics and a numerical AGE control. Cosmetic rust, dusty knobs or yellowed keys do not establish an audio-path fault or calibrated distortion amount.

## Current software and conservative treatment

Read-only inspection of [Engine.cpp](../Source/DSP/Engine.cpp) found mixer saturation `1.6 * sat(mix / 1.6)`, four nonlinear filter cells with scale `1.1`, bounded resonance feedback with scale `2.8`, and post-filter/VCA saturation with scale `1.8`. These are already circuit-informed approximations, not measured voltage scales. AGE changes tuning, cutoff, resonance and envelope timing; it does not currently change those saturation scales. Its parameter shifts can still alter the signal reaching existing nonlinearities.

Do not add another diode clipper merely because the schematic contains diodes, or claim progressively reduced headroom as a documented aging fact. Leave the DSP unchanged unless hardware measurements or a deliberate artistic decision justify a change. If the user's permitted subtle maximum-AGE coloration is selected later, identify it as an **optional creative condition model**. Prefer a small, smooth, level-matched change to an existing nonlinear stage, preserve AGE-zero behavior, and verify level-dependent harmonics/intermodulation, self-oscillation, DC, aliasing and preset stability. Its amount must be an explicit engineering choice, not a fabricated historical specification.

For later calibration, record external sine and two-tone level sweeps with VCO/Sub off, VCA Hold, documented cutoff/resonance and fixed output loading; include low-level and self-oscillating cases. Log actual signal levels, supply/bias condition and service history. Compare the same unit before/after identified repair or adjustment where possible. A single old-versus-new recording with different gain, calibration or components cannot isolate aging. Match listening levels and retain raw captures. None of these measurements has been performed in this investigation.
