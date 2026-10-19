"""Assemble the README's five-age animation from the approved manual screenshots.

Requires Pillow. This only creates documentation media; it does not render or
change the instrument, screenshots, presets or released binaries.
"""
from pathlib import Path
import hashlib
import json
from PIL import Image, ImageDraw, ImageFont, ImageSequence

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "docs/manual/assets"
NAMES = ["Minty", "Seen Some Shit", "Vintage, Baby!", "Sold As-Is", "Fire Hazard"]
DURATIONS = [3200, 2600, 2600, 2600, 3400]
WIDTH, HEIGHT, FOOTER = 1440, 810, 82


def main():
    font = ImageFont.truetype(str(ROOT / "Assets/Fonts/Jost-Semibold.ttf"), 34)
    frames, sources = [], []
    for index, name in enumerate(NAMES, 1):
        source = ASSETS / f"panel-age-{index}.png"
        image = Image.open(source).convert("RGB")
        assert image.size == (2880, 1620)
        canvas = Image.new("RGB", (WIDTH, HEIGHT + FOOTER), "#101719")
        canvas.paste(image.resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS))
        draw = ImageDraw.Draw(canvas)
        draw.line((0, HEIGHT, WIDTH, HEIGHT), fill="#535c5a", width=2)
        draw.text((28, HEIGHT + 17), f"AGE {index} / 5  —  {name}", font=font, fill="#eee9d9")
        for dot in range(5):
            x, y = WIDTH - 208 + dot * 35, HEIGHT + 41
            draw.ellipse((x - 8, y - 8, x + 8, y + 8), fill="#e2bc70" if dot == index - 1 else "#465052")
        frames.append(canvas.quantize(colors=256, method=Image.Quantize.MEDIANCUT))
        sources.append({"age": index, "name": name, "source": source.relative_to(ROOT).as_posix(), "sha256": hashlib.sha256(source.read_bytes()).hexdigest()})
    target = ASSETS / "kb09-five-ages.gif"
    frames[0].save(target, save_all=True, append_images=frames[1:], duration=DURATIONS, loop=0, disposal=2, optimize=False)
    with Image.open(target) as animation:
        assert animation.n_frames == 5 and animation.info["loop"] == 0
        assert [frame.info["duration"] for frame in ImageSequence.Iterator(animation)] == DURATIONS
    assert target.stat().st_size < 10_000_000
    proof = ROOT / "artifacts/ReadmeAge190"
    proof.mkdir(parents=True, exist_ok=True)
    contact = Image.new("RGB", (720, (HEIGHT + FOOTER) // 2 * 5))
    for index, frame in enumerate(frames):
        contact.paste(frame.convert("RGB").resize((720, (HEIGHT + FOOTER) // 2)), (0, index * ((HEIGHT + FOOTER) // 2)))
    contact.save(proof / "five-ages-contact.png")
    (proof / "preview.json").write_text(json.dumps({"frames": 5, "loop": "forever", "durationsMs": DURATIONS, "dimensions": [WIDTH, HEIGHT + FOOTER], "bytes": target.stat().st_size, "sha256": hashlib.sha256(target.read_bytes()).hexdigest(), "sources": sources}, indent=2))
    print(f"Created {target.relative_to(ROOT)}: five labeled frames, {target.stat().st_size:,} bytes")


if __name__ == "__main__":
    main()
