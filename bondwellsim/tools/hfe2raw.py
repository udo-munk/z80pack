#!/usr/bin/env python3
"""Convert HFE v1 IBM-MFM Bondwell disks to raw images, verifying sector CRCs.

SPDX-License-Identifier: BSD-3-Clause
Layout reference: MAME src/lib/formats/hxchfe_dsk.cpp.
"""
import argparse
import binascii
import hashlib
import json
from pathlib import Path
import struct

SYNC = "0100010010001001" * 3
BITS = tuple(f"{value:08b}"[::-1] for value in range(256))


def mfm_bytes(bits, start, count):
    if start + count * 16 > len(bits):
        raise ValueError("Truncated MFM field")
    return bytes(int(bits[p + 1:p + 16:2], 2)
                 for p in range(start, start + count * 16, 16))


def decode_track(track, cylinder, head):
    original = "".join(BITS[b] for b in track)
    bits = original + original  # circular track; fields can cross index
    cursor, pending = 0, None
    sectors = {}
    while True:
        start = bits.find(SYNC, cursor)
        if start < 0 or start >= len(original) + 8192:
            break
        cursor = start + len(SYNC)
        mark = mfm_bytes(bits, cursor, 1)[0]
        if mark == 0xFE:
            field = mfm_bytes(bits, cursor, 7)
            if binascii.crc_hqx(b"\xa1\xa1\xa1" + field, 0xFFFF):
                raise ValueError(f"C{cylinder} H{head}: ID CRC mismatch")
            c, h, r, n = field[1:5]
            if (c, h, n) != (cylinder, head, 1) or r >= 18:
                raise ValueError(f"C{cylinder} H{head}: unsupported CHRN={c,h,r,n}")
            pending = r
            cursor += 7 * 16
        elif mark in (0xFB, 0xF8):
            if pending is None:
                continue  # index may lie between a sector's ID and data
            if mark == 0xF8:
                raise ValueError("Deleted-data marks cannot be preserved in raw images")
            field = mfm_bytes(bits, cursor, 259)
            if binascii.crc_hqx(b"\xa1\xa1\xa1" + field, 0xFFFF):
                raise ValueError(f"C{cylinder} H{head} R{pending}: data CRC mismatch")
            payload = field[1:257]
            if pending in sectors and sectors[pending] != payload:
                raise ValueError(f"C{cylinder} H{head}: conflicting sector {pending}")
            sectors[pending] = payload
            pending = None
            cursor += 259 * 16
    missing = sorted(set(range(18)) - sectors.keys())
    if missing:
        raise ValueError(f"C{cylinder} H{head}: missing sectors {missing}")
    return b"".join(sectors[r] for r in range(18))


def decode(data):
    if len(data) < 512 or data[:8] != b"HXCPICFE" or data[8] != 0:
        raise ValueError("Expected HFE v1/revision 0 (HXCPICFE); v3 is unsupported")
    cylinders, heads, encoding = data[9:12]
    bitrate = struct.unpack_from("<H", data, 12)[0]
    if cylinders != 40 or heads not in (1, 2) or encoding != 0 or bitrate != 250:
        raise ValueError("Expected 40-cylinder, 1/2-head, 250 kbit/s IBM-MFM Bondwell image")
    for side in range(heads):
        if data[22 + side * 2] == 0 and data[23 + side * 2] != 0:
            raise ValueError("Non-MFM track-0 alternate encoding is unsupported")
    lut = struct.unpack_from("<H", data, 18)[0] * 512
    if lut < 512 or lut + cylinders * 4 > len(data):
        raise ValueError("Invalid/truncated HFE track table")
    output, ranges = bytearray(), []
    for cylinder in range(cylinders):
        block, length = struct.unpack_from("<HH", data, lut + cylinder * 4)
        offset = block * 512
        if offset < lut + cylinders * 4 or length < 512 or offset + length > len(data):
            raise ValueError(f"C{cylinder}: invalid/truncated track range")
        if any(offset < end and offset + length > begin for begin, end in ranges):
            raise ValueError(f"C{cylinder}: overlapping track ranges")
        ranges.append((offset, offset + length))
        whole, tail = divmod(length, 512)
        if tail and tail <= 256:
            raise ValueError(f"C{cylinder}: invalid final interleaved block")
        side_length = whole * 256 + (tail - 256 if tail else 0)
        for head in range(heads):
            track = bytearray()
            for start in range(0, side_length, 256):
                n = min(256, side_length - start)
                source = offset + (start // 256) * 512 + head * 256
                track.extend(data[source:source + n])
            output.extend(decode_track(track, cylinder, head))
    raw = bytes(output)
    return raw, {"cylinders": cylinders, "heads": heads, "sectors_per_track": 18,
                 "sector_size": 256, "first_sector": 0, "bytes": len(raw),
                 "sectors_crc_verified": cylinders * heads * 18,
                 "source_sha256": hashlib.sha256(data).hexdigest(),
                 "raw_sha256": hashlib.sha256(raw).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    try:
        raw, report = decode(args.source.read_bytes())
        with args.destination.open("xb") as output:
            output.write(raw)
    except (OSError, ValueError) as error:
        parser.exit(1, f"hfe2raw: {error}\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
