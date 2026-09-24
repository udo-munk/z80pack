"""Synthetic MFM/HFE fixtures; no proprietary disk or ROM data."""
import binascii
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location("hfe2raw", Path(__file__).parents[1] / "tools/hfe2raw.py")
hfe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hfe)


def encode(data):
    result, previous = [], 1
    for value in data:
        for bit in f"{value:08b}":
            current = int(bit)
            result.extend((str(int(not (previous or current))), bit))
            previous = current
    return "".join(result)


def pack(bits):
    return bytes(int(bits[i:i + 8][::-1], 2) for i in range(0, len(bits), 8))


def track(cylinder=0, head=0, *, bad_id=False, bad_data=False, deleted=False, missing=False):
    result = encode(b"\x4e" * 40)
    for sector in reversed(range(17 if missing else 18)):
        for field in [bytes([0xFE, cylinder, head, sector, 1]),
                      bytes([0xF8 if deleted else 0xFB]) + bytes([sector]) * 256]:
            crc = binascii.crc_hqx(b"\xa1\xa1\xa1" + field, 0xFFFF)
            if (field[0] == 0xFE and bad_id) or (field[0] == 0xFB and bad_data):
                crc ^= 1
            result += hfe.SYNC + encode(field + struct.pack(">H", crc))
            result += encode(b"\x4e" * 20)
    return pack(result)


def image():
    output = bytearray(1024)
    output[:8] = b"HXCPICFE"
    output[9:12] = bytes([40, 2, 0])
    struct.pack_into("<H", output, 12, 250)
    struct.pack_into("<H", output, 18, 1)
    for cylinder in range(40):
        sides = [track(cylinder, head) for head in range(2)]
        block = len(output) // 512
        start = len(output)
        for pos in range(0, len(sides[0]), 256):
            output.extend(sides[0][pos:pos + 256].ljust(256, b"\0"))
            output.extend(sides[1][pos:pos + 256])
        length = len(output) - start
        output.extend(b"\0" * (-len(output) % 512))
        struct.pack_into("<HH", output, 512 + cylinder * 4, block, length)
    return bytes(output)


class HFETests(unittest.TestCase):
    def test_sector_id_order(self):
        self.assertEqual(hfe.decode_track(track(), 0, 0),
                         b"".join(bytes([s]) * 256 for s in range(18)))

    def test_cross_index_and_unaligned_bits(self):
        bits = "".join(hfe.BITS[b] for b in track())
        # Place index inside a data field, with a non-byte-aligned rotation.
        rotated = pack(bits[2201:] + bits[:2201])
        self.assertEqual(hfe.decode_track(rotated, 0, 0), hfe.decode_track(track(), 0, 0))

    def test_errors_are_not_silently_repaired(self):
        for options, message in [({"bad_id": True}, "ID CRC"),
                                 ({"bad_data": True}, "data CRC"),
                                 ({"deleted": True}, "Deleted"),
                                 ({"missing": True}, "missing sectors")]:
            with self.subTest(options=options), self.assertRaisesRegex(ValueError, message):
                hfe.decode_track(track(**options), 0, 0)

    def test_full_image_and_partial_blocks(self):
        raw, report = hfe.decode(image())
        self.assertEqual(len(raw), 368640)
        self.assertEqual(report["sectors_crc_verified"], 1440)
        self.assertEqual(raw, hfe.decode_track(track(), 0, 0) * 80)

    def test_invalid_container(self):
        original = image()
        for bad in [original[:511], original[:-512], b"HXCHFEV3" + original[8:]]:
            with self.assertRaises(ValueError):
                hfe.decode(bad)
        bad = bytearray(original)
        bad[516:520] = bad[512:516]
        with self.assertRaisesRegex(ValueError, "overlapping"):
            hfe.decode(bad)


if __name__ == "__main__":
    unittest.main()
