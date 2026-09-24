# Validation and remaining work

Validated locally on Windows with GCC 13.2 / CMake, against unmodified z80pack
dev CPU sources. The Windows executable uses only Windows system DLLs.

## Observed with original software

* BOOTROM SHA-1 `eefe5ad6b1ef77a1caf0af743b74de5fa1c4c19d`.
* CHAROM SHA-1 `50132b759a6d84c22c387c39c0f57535cd380411`.
* Recovered SYSTEM1 CopyQM CRC `70ac5d9d`; checksum verified before use.
* Model 12: 64K installed, **PASS ALL TESTS**, boot-key prompt.
* Model 14: 128K installed, **PASS ALL TESTS**, boot-key prompt.
* Model 14 SYSTEM1: authentic SETUP program appears; keyboard E exits to CP/M
  `A>`; `DIR` lists the system disk; `DIR B:` reads a second mounted copy.
* Guest writes: PIP created TEST.TXT on B: and saved to a separate .updated
  image; remounting it and TYPE B:TEST.TXT returned HELLO. The original
  SYSTEM1 file was unchanged.
* HFE archive pair: DISK1_TD0.hfe boots CP/M 3; DIR B: reads the actual
  DISK2_TD0.hfe, including MAC, RMAC, LINK, GENCPM and BIOS sources.
  All 2880 sector ID/data CRC pairs verified during conversion.
* The display is rendered from actual video RAM and CHAROM, not synthesized
  terminal text. Exported frame was visually inspected.

`tests/boot_smoke.py` reproduces ROM diagnostics and the second-drive directory
test with explicitly supplied assets. It is opt-in and not part of public CI.

## Automated tests

CTest runs machine-level C tests (bank isolation, read-triggered LS259 changes,
ROM protection, Model 12 absent banks, real Z80 relocation and port execution,
FDC read/write/protection/invalid units, keyboard serial order/queue, SIO A/B file transfers and TX timing, PIT latching,
  and glyph pixels)
and Python CopyQM corruption/truncation/CRC/padding tests, plus HFE sector-order,
circular-track, CRC, deleted-mark and container-boundary tests. These are asset-free.

## Limits

This is a bootable initial implementation, not a claim of complete cycle-accurate
hardware emulation. Remaining validation includes the original schematic pages,
Model 12 CP/M 2.2 media, SYSTEM2 applications, physical serial PCGET/PCPUT,
SIO interrupt priorities/modem signals, all PIT modes, complete printer
handshaking, live DAC sound, and copy-protected/nonstandard floppy tracks.

FDC byte-ready events and seeks are immediate; host time does not govern disk
rotation. Serial file output is suitable for regression tests, not yet a
substitute for a real timed RS-232 connection. Native Windows display and the
portable headless build are supported; the SDL frontend needs testing on Unix.
