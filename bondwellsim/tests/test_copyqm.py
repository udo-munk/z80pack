import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location("copyqm2raw", Path(__file__).parents[1] / "tools/copyqm2raw.py")
cqm = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cqm)


def image(raw, *, compressed=False):
    h = bytearray(133)
    h[:3] = b"CQ\x14"
    struct.pack_into("<H", h, 3, 256)
    struct.pack_into("<HH", h, 16, 18, 1)
    h[0x5A:0x5C] = bytes([40, 40])
    h[0x71] = 255
    struct.pack_into("<I", h, 0x5C, cqm.crc_copyqm(raw))
    h[-1] = -sum(h) & 255
    stream = bytearray(h)
    for start in range(0, len(raw), 16000):
        block = raw[start:start + 16000]
        stream.extend(struct.pack("<h", -len(block) if compressed else len(block)))
        stream.extend(block[:1] if compressed else block)
    return bytes(stream)


class CopyQMTests(unittest.TestCase):
    def test_literal(self):
        raw = bytes(range(256)) * 720
        decoded, meta = cqm.decode(image(raw))
        self.assertEqual(decoded, raw)
        self.assertTrue(meta["crc_verified"])

    def test_compressed(self):
        raw = b"\xe5" * 184320
        self.assertEqual(cqm.decode(image(raw, compressed=True))[0], raw)

    def test_corrupt_or_truncated(self):
        original = image(bytes(range(256)) * 720)
        for bad in [original[:132], original[:-1], original[:140], b"XX" + original[2:]]:
            with self.assertRaises(ValueError):
                cqm.decode(bad)
        corrupt = bytearray(original)
        corrupt[140] ^= 1
        with self.assertRaisesRegex(ValueError, "CRC"):
            cqm.decode(corrupt)

    def test_padding_requires_explicit_option(self):
        original = image(b"\xe5" * 184320, compressed=True)
        with self.assertRaises(ValueError):
            cqm.decode(original + b"\xf6" * 20)
        _, meta = cqm.decode(original + b"\xf6" * 20, allow_padding=True)
        self.assertEqual(meta["ignored_padding_bytes"], 20)
        with self.assertRaises(ValueError):
            cqm.decode(original + b"garbage", allow_padding=True)

    def test_zero_and_oversized_runs(self):
        original = image(b"\xe5" * 184320, compressed=True)
        for stream in [original[:133] + b"\0\0", original[:-3] + struct.pack("<h", -32768) + b"x"]:
            with self.assertRaises(ValueError):
                cqm.decode(stream)


if __name__ == "__main__":
    unittest.main()
