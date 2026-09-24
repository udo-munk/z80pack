#!/usr/bin/env python3
"""Strict CopyQM to raw Bondwell disk conversion (standard library only).

SPDX-License-Identifier: BSD-3-Clause
Format references: MAME cqm_dsk.cpp and LibDsk doc/libdsk.txt appendix M.
CopyQM's CRC intentionally uses a six-bit table index, unlike standard CRC32.
"""
import argparse
import json
from pathlib import Path
import struct


def crc_copyqm(data):
    table = []
    for value in range(64):
        for _ in range(8):
            value = (value >> 1) ^ (0xEDB88320 if value & 1 else 0)
        table.append(value)
    crc = 0
    for value in data:
        crc = (crc >> 8) ^ table[(crc ^ value) & 0x3F]
    return crc


def decode(data, *, allow_padding=False):
    if len(data) < 133 or data[:3] != b"CQ\x14":
        raise ValueError("Not a CopyQM 1.4 image (expected CQ 14 header)")
    if sum(data[:133]) & 255:
        raise ValueError("CopyQM header checksum mismatch")
    sector_size = struct.unpack_from("<H", data, 3)[0]
    sectors, heads = struct.unpack_from("<HH", data, 0x10)
    used, cylinders = data[0x5A:0x5C]
    sector_base = (data[0x71] + 1) & 255
    if (sector_size, sectors, cylinders, sector_base) != (256, 18, 40, 0) or heads not in (1, 2):
        raise ValueError(f"Not Bondwell geometry: {cylinders} cylinders, {heads} heads, "
                         f"{sectors} sectors, {sector_size} bytes, first sector {sector_base}")
    if used != cylinders:
        raise ValueError("Partially imaged disks are unsupported; obtain a complete image")
    comment_len = struct.unpack_from("<H", data, 0x6F)[0]
    pos = 133 + comment_len
    if pos > len(data):
        raise ValueError("Truncated CopyQM comment")
    target = cylinders * heads * sectors * sector_size
    output = bytearray()
    while len(output) < target:
        if pos + 2 > len(data):
            raise ValueError("Truncated CopyQM run length")
        length = struct.unpack_from("<h", data, pos)[0]
        pos += 2
        if not length:
            raise ValueError("Zero-length CopyQM run")
        if len(output) + abs(length) > target:
            raise ValueError("CopyQM run exceeds declared disk geometry")
        if length < 0:
            if pos == len(data):
                raise ValueError("Truncated CopyQM repeated byte")
            output.extend(data[pos:pos + 1] * -length)
            pos += 1
        else:
            if pos + length > len(data):
                raise ValueError("Truncated CopyQM literal run")
            output.extend(data[pos:pos + length])
            pos += length
    trailing = data[pos:]
    if trailing and not (allow_padding and len(set(trailing)) == 1 and trailing[0] in (0, 0xE5, 0xF6, 0xFF)):
        raise ValueError("Trailing data after CopyQM stream; use --allow-padding only for uniform padding")
    expected_crc = struct.unpack_from("<I", data, 0x5C)[0]
    actual_crc = crc_copyqm(output)
    if expected_crc and expected_crc != actual_crc:
        raise ValueError(f"CopyQM data CRC mismatch: {actual_crc:08x} != {expected_crc:08x}")
    return bytes(output), {
        "cylinders": cylinders, "heads": heads, "sectors_per_track": sectors,
        "sector_size": sector_size, "first_sector": sector_base,
        "crc": f"{actual_crc:08x}", "crc_verified": bool(expected_crc),
        "source_stream_bytes": pos, "ignored_padding_bytes": len(trailing),
        "comment": data[133:133 + comment_len].decode("cp437", errors="replace"),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--allow-padding", action="store_true")
    args = parser.parse_args()
    try:
        raw, metadata = decode(args.source.read_bytes(), allow_padding=args.allow_padding)
        # Never overwrite the source, an existing conversion, or other user data.
        with args.destination.open("xb") as output:
            output.write(raw)
    except (OSError, ValueError) as error:
        parser.exit(1, f"copyqm2raw: {error}\n")
    print(json.dumps(metadata, indent=2))


if __name__ == "__main__":
    main()
