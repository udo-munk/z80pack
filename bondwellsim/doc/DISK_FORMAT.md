# Bondwell disk images

| Model | Cylinders | Heads | Sectors/track | Bytes/sector | IDs | Size |
| --- | --- | --- | --- | --- | --- | --- |
| 12 | 40 | 1 | 18 | 256 | 0–17 | 184320 |
| 14 | 40 | 2 | 18 | 256 | 0–17 | 368640 |

Raw offset = `((cylinder * heads + head) * 18 + sector_id) * 256`.
Reference: [MAME bw12_dsk.cpp](https://github.com/mamedev/mame/blob/master/src/lib/formats/bw12_dsk.cpp).
The uPD765 N code is 1. Original ROM/BIOS commands use EOT=18 and terminal count
to end transfers; EOT must not be rejected merely because that sector ID is
absent from the disk.

## CopyQM

Reference: [LibDsk format documentation, appendix M](https://github.com/lipro-cpm4l/libdsk/blob/master/doc/libdsk.txt)
and [MAME cqm_dsk.cpp](https://github.com/mamedev/mame/blob/master/src/lib/formats/cqm_dsk.cpp).

The 133-byte header starts `43 51 14`. Geometry is at 03 (LE16 sector size),
10 (LE16 sectors), 12 (LE16 heads), 5A/5B (used/total cylinders), and 71
(first sector minus one, wrapping FF to ID 0). Comment length is LE16 at 6F.
Data starts at 133 + comment length. Signed LE16 runs encode positive-length
literals or negative-length repeats of the following byte. Data are ordered by
cylinder, head, then ascending sector number; physical interleave is not a
reason to reorder raw sectors.

The header byte sum modulo 256 must be zero. Data CRC at 5C starts from zero,
uses reflected polynomial EDB88320, and indexes its table with only the low
six bits of `CRC XOR byte`, reproducing CopyQM's historical algorithm. There
is no final complement. A zero stored CRC is reported as unverified.

The converter deliberately supports only complete standard Bondwell images;
it rejects partial cylinders, bad checksums, unsupported geometry, truncated
runs, over-expansion and unexpected trailing bytes. It does not silently
reinterpret arbitrary 360 KB PC media as Bondwell disks.

## HFE v1

`tools/hfe2raw.py` accepts revision-0 HXCPICFE containers containing 40-cylinder,
250 kbit/s IBM-MFM tracks. It deinterleaves the heads in 256-byte chunks, reads
HFE bits least-significant-first, locates missing-clock A1 sync words, verifies
CRC-CCITT for ID and normal data fields, and writes sectors in ID order.
Circular tracks can contain fields crossing index. Missing sectors, conflicting
sector duplicates, unsupported/deleted sectors, overlapping/truncated ranges,
and CRC failures are errors. Conversion does not preserve flux timing.

Layout reference: [MAME hxchfe_dsk.cpp](https://github.com/mamedev/mame/blob/master/src/lib/formats/hxchfe_dsk.cpp).

| Supplied HFE | Raw SHA-256 | Verified sectors |
| --- | --- | --- |
| DISK1_TD0.hfe | 7dd156c7ae853bf3472746c9dfe6f671aa7577577ea7070ed3bec2fc106fae55 | 1440 |
| DISK2_TD0.hfe | 57d9387ffe490e7a8b78bd56aab32e9ff71092a8fef7e7acbd2d861b2bb62217 | 1440 |
