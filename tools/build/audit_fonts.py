#!/usr/bin/env python3
"""Read upstream IBMF and SFNT Unicode maps with standard-library parsers."""
import argparse
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SAMPLE = "中文阅读，。ABC012"


def ibmf_codes(data):
    if data[:4] != b"IBMF" or data[5] >> 5 != 1:
        raise ValueError("Expected IBMF UTF32 directory")
    count = data[4]
    directory = ((6 + count + 3) & ~3) + 4 * count
    bundles = directory + 24
    codes = set()
    for plane in range(4):
        start, entries, _ = struct.unpack_from("<HHH", data, directory + plane * 6)
        for index in range(start, start + entries):
            first, last = struct.unpack_from("<HH", data, bundles + index * 4)
            if first > last:
                raise ValueError("Invalid Unicode bundle")
            codes.update((plane << 16) + code for code in range(first, last + 1))
    return codes


def ttf_codes(data):
    count = struct.unpack_from(">H", data, 4)[0]
    cmap = None
    for index in range(count):
        tag, _, offset, length = struct.unpack_from(">4sIII", data, 12 + index * 16)
        if tag == b"cmap":
            cmap = data[offset:offset + length]
    if cmap is None:
        raise ValueError("No cmap")
    codes = set()
    count = struct.unpack_from(">H", cmap, 2)[0]
    for index in range(count):
        platform, encoding, offset = struct.unpack_from(">HHI", cmap, 4 + index * 8)
        if platform != 0 and not (platform == 3 and encoding in (1, 10)):
            continue
        fmt = struct.unpack_from(">H", cmap, offset)[0]
        if fmt == 12:
            groups = struct.unpack_from(">I", cmap, offset + 12)[0]
            for group in range(groups):
                start, end, glyph = struct.unpack_from(">III", cmap, offset + 16 + group * 12)
                if end > 0x10FFFF or start > end:
                    raise ValueError("Invalid Unicode range")
                codes.update(range(start + (glyph == 0), end + 1))
        elif fmt == 4:
            segments = struct.unpack_from(">H", cmap, offset + 6)[0] // 2
            ends = offset + 14
            starts = ends + 2 * segments + 2
            deltas = starts + 2 * segments
            ranges = deltas + 2 * segments
            for segment in range(segments):
                first = struct.unpack_from(">H", cmap, starts + 2 * segment)[0]
                last = struct.unpack_from(">H", cmap, ends + 2 * segment)[0]
                delta = struct.unpack_from(">h", cmap, deltas + 2 * segment)[0]
                range_offset = struct.unpack_from(">H", cmap, ranges + 2 * segment)[0]
                for code in range(first, min(last, 0xFFFE) + 1):
                    if range_offset:
                        pos = ranges + 2 * segment + range_offset + 2 * (code - first)
                        glyph = struct.unpack_from(">H", cmap, pos)[0]
                        glyph = ((glyph + delta) & 0xFFFF) if glyph else 0
                    else:
                        glyph = (code + delta) & 0xFFFF
                    if glyph:
                        codes.add(code)
    return codes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, default=ROOT / "sources/official/checkouts")
    args = parser.parse_args()
    base = args.source_root / "embedded-app"
    paths = ["src/UI/Fonts/IBMFFonts/SolSans_75.ibmf",
             "src/UI/Fonts/IBMFFonts/NotoSans-Light.ttf",
             "src/UI/Fonts/TTFFonts/SolPrivate-Light.ttf"]
    report = []
    for name in paths:
        data = (base / name).read_bytes()
        codes = ibmf_codes(data) if name.endswith(".ibmf") else ttf_codes(data)
        report.append({"path": name, "mapped_codepoints": len(codes),
                       "cjk_unified_4e00_9fff": sum(0x4E00 <= code <= 0x9FFF for code in codes),
                       "sample": {char: ord(char) in codes for char in SAMPLE}})
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
