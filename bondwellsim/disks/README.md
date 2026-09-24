# Included Bondwell disks

| File | Contents / provenance |
| --- | --- |
| DISK1_HFE.img | Bootable system disk converted from DISK1_TD0.hfe |
| DISK2_HFE.img | Companion CP/M tools/source disk converted from DISK2_TD0.hfe |
| SYSTEM1.img | Earlier system disk recovered from the CopyQM SYSTEM1_RAW.img |
| source/DISK1_TD0.hfe | Original HFE from the owner's Bondwell archive |
| source/DISK2_TD0.hfe | Original HFE from the owner's Bondwell archive |

The Windows launcher uses DISK1_HFE.img in A: and DISK2_HFE.img in B:.
All raw images are 368640 bytes. Both HFE conversions passed every ID/data CRC.
The source HFE files are preserved byte-for-byte. See ../media-sha256.json for
checksums and [disk format](../doc/DISK_FORMAT.md) for layout and conversion.

Images are included at the repository owner's request. The emulator mounts
them read-only by default. With --writable, changed content is saved separately
as .updated files; those test/working copies are not included in Git.
