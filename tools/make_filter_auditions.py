"""Make listening clips only from actual engine captures; never add grit or noise."""
from pathlib import Path
import csv
import json
import struct
import sys
import wave

import numpy as np


def read_float_wav(path):
    raw = path.read_bytes()
    assert raw[:4] == b"RIFF" and raw[8:12] == b"WAVE"
    pos = 12
    sr = None
    audio = None
    while pos + 8 <= len(raw):
        size = struct.unpack_from("<I", raw, pos + 4)[0]
        body = raw[pos + 8:pos + 8 + size]
        if raw[pos:pos + 4] == b"fmt ":
            fmt, channels, sr = struct.unpack_from("<HHI", body)
            assert fmt == 3 and channels == 1
        if raw[pos:pos + 4] == b"data":
            audio = np.frombuffer(body, dtype="<f4").astype(float)
        pos += 8 + size + (size & 1)
    assert sr == 48000 and audio is not None and np.all(np.isfinite(audio))
    return audio


def write_pcm(path, audio):
    assert np.max(np.abs(audio)) < 1
    pcm = np.rint(audio * 32767).astype("<i2")
    with wave.open(str(path), "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(48000)
        f.writeframes(pcm.tobytes())


def main(folder):
    out = folder / "Listening"
    out.mkdir(exist_ok=True)
    rows = {model: list(csv.DictReader((folder / (model + ".csv")).open()))
            for model in ("baseline", "current")}
    captures = {}
    manifest = []
    gap = np.zeros(14400)
    for age in (0, 1):
        for model in rows:
            pieces = [gap]
            for cutoff in (.4, .6, .8):
                row = next(r for r in rows[model] if float(r["sample_rate"]) == 48000
                           and float(r["resonance"]) == 1 and float(r["age"]) == age
                           and float(r["cutoff"]) == cutoff and int(r["block_size"]) == 256)
                x = read_float_wav(folder / row["file"])
                captures[model, age, cutoff] = x
                manifest.append(dict(model=model, age_display=age * 10, cutoff=cutoff,
                                     frequency_hz=float(row["frequency_hz"]), source=row["file"]))
                x = x.copy()
                x[:480] *= np.linspace(0, 1, 480)
                x[-480:] *= np.linspace(1, 0, 480)
                pieces.extend([x, gap])
            write_pcm(out / f"{model}-AGE-{age*10}-original-level.wav", np.concatenate(pieces))
        # Match RMS so extra level does not masquerade as more harmonic character.
        # Preserve each model's pitch; this is not a pitch-matched hardware comparison.
        pieces = [gap]
        for cutoff in (.4, .6, .8):
            for model in ("baseline", "current"):
                x = captures[model, age, cutoff].copy()
                x *= 10**(-18 / 20) / np.sqrt(np.mean(x*x))
                x[:480] *= np.linspace(0, 1, 480)
                x[-480:] *= np.linspace(1, 0, 480)
                pieces.extend([x, gap])
        write_pcm(out / f"AB-AGE-{age*10}-matched-level.wav", np.concatenate(pieces))
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    (out / "README.md").write_text("""# Filter listening comparisons

All clips are actual filter-engine output, VCO/SUB/EXT zero, VCA HOLD,
RESONANCE maximum, ENV/MOD zero, volume 0.7, reference note C4, 48 kHz.
They play cutoff 0.4, 0.6, then 0.8. AGE filenames use the GUI's 0–10 scale.

`original-level` files preserve the original recorded output gain.
`AB-matched-level` files alternate **1.1.3 first, new model second** at each
cutoff, both normalized to -18 dBFS RMS before the short edge fades.
Pitch is not matched: the large-signal resonance response changes it.
Only 10 ms edge fades and silence between examples were added; no effect,
EQ, noise or harmonic processing was applied. PCM export is 16-bit mono.

These compare two software models, not software against physical hardware.
""", encoding="utf-8")
    print("Saved six A/B listening clips:", out)


if __name__ == "__main__":
    main(Path(sys.argv[1]))
