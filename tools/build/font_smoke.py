#!/usr/bin/env python3
"""Run supplied C++ layout with real font advances and render the first page locally."""
import argparse
import hashlib
import math
import subprocess
from pathlib import Path

def main():
    from PIL import Image, ImageDraw, ImageFont
    from fontTools.ttLib import TTFont
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--font", type=Path, required=True)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    text = "中文阅读：春天到了，窗外的树叶轻轻摇动。\n\n我们阅读 English 2026，数字 123 和标点。\n极简阅读，保留段落与原文内容。"
    size = 16
    font = ImageFont.truetype(str(args.font), size, layout_engine=ImageFont.Layout.BASIC)
    cmap = TTFont(args.font).getBestCmap()
    glyphs = set(text) - {"\n", "\r"}
    missing = [c for c in glyphs if ord(c) not in cmap]
    if missing:
        raise RuntimeError(f"Actual font missing sample glyphs: {missing}")
    (args.output / "sample.txt").write_text(text, encoding="utf-8")
    (args.output / "metrics.txt").write_text(str(size) + "\n" + "".join(
        f"{ord(c)} {math.ceil(font.getlength(c))}\n" for c in sorted(glyphs)), encoding="ascii")
    ascent, descent = font.getmetrics()
    line_height = ascent + descent
    result = subprocess.run([str(args.binary.resolve()), str(args.output / "sample.txt"),
                             str(args.output / "metrics.txt"), str(line_height)],
                            check=True, text=True, capture_output=True)
    (args.output / "glyphs.tsv").write_text(result.stdout)
    image = Image.new("1", (256, 256), 1)
    draw = ImageDraw.Draw(image)
    count = 0
    for line in result.stdout.splitlines():
        cp, x, top = map(int, line.split("\t"))
        char = chr(cp)
        left, upper, right, lower = font.getbbox(char, anchor="ls")
        if not (x + left >= 0 and x + right <= 196 and top + ascent + upper >= 0
                and top + ascent + lower <= 216):
            raise RuntimeError("Glyph ink exceeds the actual viewport")
        draw.text((30 + x, 20 + top + ascent), char, font=font, fill=0, anchor="ls")
        count += 1
    if not count or image.getextrema() != (0, 1):
        raise RuntimeError("No actual glyph pixels rendered")
    image.save(args.output / "chinese-page.png")
    print(result.stderr.strip())
    print(f"PASS: {count} measured/rasterized glyphs within viewport; host preview only")
    print("Font SHA256:", hashlib.sha256(args.font.read_bytes()).hexdigest())

if __name__ == "__main__":
    main()
