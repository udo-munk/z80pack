# Bondwell 12/14 for z80pack

An experimental Bondwell machine emulator using z80pack's **unmodified Z80 CPU
core**. The original Bondwell ROM passes its diagnostics in both 64 KB Model 12
and 128 KB Model 14 configurations. Model 14 has been tested booting the original
CP/M 3 SYSTEM1 disk, exiting SETUP, and running `DIR` on drives A and B.

## Build

From the repository root, with CMake 3.16+ and a C99 compiler:

```sh
cmake -S bondwellsim -B bondwellsim/build -DCMAKE_BUILD_TYPE=Release
cmake --build bondwellsim/build
ctest --test-dir bondwellsim/build --output-on-failure
```

On Windows, use MinGW GCC (the native display uses Win32/GDI). For example,
if GCC and mingw32-make are on PATH:

```powershell
cmake -S bondwellsim -B bondwellsim/build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build bondwellsim/build
```

Linux/macOS use SDL2 when installed, otherwise the headless emulator is built.
The top-level `make bondwell` target builds just this machine. Existing machines
and the common CPU sources are unchanged.

## ROMs and disks

The boot and character ROMs, validated raw CP/M disks, and original HFE disk
images are included. See [ROM provenance](roms/README.md),
[disk contents](disks/README.md), and [SHA-256 checksums](media-sha256.json).
A fresh checkout has the media needed by the Windows launcher.

Native disk geometry is **40 cylinders, 18 sectors of 256 bytes, IDs 0–17**,
with one head (Model 12, 184320 bytes) or two (Model 14, 368640 bytes).
Raw files are in cylinder/head/sector order. A PC 360 KB image with 512-byte
sectors is not interchangeable. See [disk format](doc/DISK_FORMAT.md).

Convert CopyQM files regardless of their extension:

```sh
python bondwellsim/tools/copyqm2raw.py SYSTEM1.IMG bondwellsim/disks/SYSTEM1.img
```

The converter verifies header and data checksums and refuses existing output
files. `--allow-padding` explicitly permits uniform trailing 00/E5/F6/FF padding.
This was necessary for the local `SYSTEM1_RAW.img`, which actually held a valid
368974-byte CopyQM stream padded to 737280 bytes with F6. Its data CRC is
`70ac5d9d`. The original file is never modified.

## HFE archive images

The original HFE v1 archive images are included under `disks/source/`; their
converted images are already provided. To convert another copy:

```powershell
python bondwellsim/tools/hfe2raw.py "G:\Bondwell 14\bw14dsks\IMAGES\DISK1_TD0.hfe" bondwellsim/disks/DISK1_HFE.img
python bondwellsim/tools/hfe2raw.py "G:\Bondwell 14\bw14dsks\IMAGES\DISK2_TD0.hfe" bondwellsim/disks/DISK2_HFE.img
```

Every sector ID and data CRC is checked before writing the new image. The
original HFE is unchanged. HFE v3, deleted-data sectors, CRC errors and
nonstandard geometries are rejected rather than silently losing information.
The Windows launcher prefers this pair when both converted images exist.
Both supplied disks passed all 1440 sector checks; DISK1 boots CP/M 3 and DISK2
lists its assembler/linker and system-building utilities through `DIR B:`.

## Run

On Windows you can double-click **Launch Bondwell.cmd** after building. Or run from `bondwellsim` so the default ROM paths resolve:

```powershell
cd bondwellsim
.\build\bondwellsim.exe --disk-a disks/SYSTEM1.img
```

On Linux/macOS use `./build/bondwellsim`. The real ROM performs several seconds
of diagnostics. At **PRESS ANY KEY**, press Space. The supplied SYSTEM1 starts
Bondwell SETUP automatically; choose **E** to reach `A>`, then type `DIR`.
Close the display or press Ctrl+C in the console to exit.

Add `--disk-b disks/SYSTEM2.img` for a second disk, or `--model 12` for the 64 KB
model (requires a single-sided disk). A Model 12 operating-system boot has not
yet been validated with a genuine Model 12 disk.

Disk images are read-only by default. `--writable` allows guest writes in memory
and saves changed disks as `original-path.updated` on exit, preserving the
mounted original. Mount that sidecar on the next run to retain changes. An
existing `.updated` sidecar is replaced by the next successful save.

Letters, digits, punctuation, Return, Escape, Backspace, arrows and F1–F12 are
mapped through the Bondwell keyboard encoder and PIA. Host keypad characters
currently follow the main keyboard mapping. Display pixels come directly from
CHAROM with 6845 start-address, scanline and cursor handling.

## Diagnostics and testing

```sh
./build/bondwellsim --headless --turbo --cycles 50000000 --text
./build/bondwellsim --disk-a disks/SYSTEM1.img --trace io.log --screenshot screen.ppm
python tests/boot_smoke.py --exe build/bondwellsim.exe --rom roms/BOOTROM.BIN --chargen roms/CHAROM.BIN --disk disks/SYSTEM1.img
```

`--cpu-trace` records instructions/registers, `--trace` records ports/FDC commands,
and `--screenshot` exports a character-ROM-rendered PPM. Trace files can grow
quickly during polling. `--type`, `--type-after` (seconds), and `--type-delay`
(milliseconds) provide scripted keyboard input. Leave enough time for boot and
SETUP to finish before typing subsequent commands. `--cycles` bounds execution
in emulated T-states; the default is unlimited. `--turbo` disables 4 MHz pacing.

The optional boot test requires the January 1984 SYSTEM1 with SETUP/AUTORUN.
The normal CTest tests use synthetic ROM/disk data and require no proprietary
assets. See [validation and limitations](doc/VALIDATION.md).

## Peripheral scope

* uPD765 programmed-I/O: specify, seek/recalibrate, status, read ID, normal data
  read/write, format, terminal count, and two drives. This is sector-level
  emulation, not flux/rotation/DMA timing or copy-protected media support.
* SIO: polled data/status/register programming and transmitter timing. Binary
  file endpoints: `--serial-a-in`, `--serial-a-out`, `--serial-b-in`,
  `--serial-b-out`. These are file streams, not COM ports or a virtual serial
  cable. SIO interrupt/daisy-chain and modem-line fidelity are incomplete;
  PCGET/PCPUT over a live host serial connection remains future work.
* 8253: binary counter/latch and timing behaviour used by boot/CP/M; modes 1/4/5,
  BCD and external gates are not fully modelled.
* Printer: PIA data and manual CA2 strobe can write a `--printer` file;
  full Centronics handshaking is not yet implemented.
* Sound: `--audio` records DAC writes as T-state/sample CSV. There is no live
  audio playback yet.

The hardware map is corroborated against MAME and real-ROM behaviour. The
original six schematic pages were not available during this implementation;
[hardware notes](doc/BONDWELL_HARDWARE.md) distinguish that limitation explicitly.
