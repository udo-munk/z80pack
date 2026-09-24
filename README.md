# z80pack with Bondwell 12/14 emulation

This fork adds **Bondwell 12/14 emulation** to z80pack using its existing,
unmodified Z80 CPU core.

The Bondwell 14 boots the original CP/M 3 system disk through the real
Bondwell boot ROM. The display renders the original character ROM, and
keyboard input passes through the emulated Bondwell keyboard interface.

## Current status

- Model 12: 64 KB configuration passes the original ROM diagnostics.
- Model 14: 128 KB configuration passes diagnostics and boots CP/M 3.
- Both floppy drives work; `DIR` and `DIR B:` have been verified.
- CP/M file creation and persistence across emulator restarts are verified.
- Native Windows display and optional SDL2 frontend are included.
- CopyQM and HFE converters validate disk checksums before conversion.

Model 12 operating-system boot still requires validation with suitable
single-sided media. Live serial connections, sound playback, and some
peripheral behaviours remain under development.

## Included ROMs and disks

The repository includes:

- `BOOTROM.BIN` — original boot ROM.
- `CHAROM.BIN` — original character ROM.
- `DISK1_HFE.img` — bootable CP/M 3 system disk.
- `DISK2_HFE.img` — companion utilities and system-source disk.
- Original HFE images and the earlier `SYSTEM1.img`.

See [media checksums](bondwellsim/media-sha256.json) and
[disk documentation](bondwellsim/disks/README.md).

## Get the Bondwell version

```sh
git clone --branch bondwell https://github.com/royedmund/z80pack.git
cd z80pack
```

## Build on Windows

Install CMake and MinGW GCC, with `gcc` and `mingw32-make` on PATH:

```powershell
cmake -S bondwellsim -B bondwellsim/build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build bondwellsim/build
ctest --test-dir bondwellsim/build --output-on-failure
```

After building, double-click **Launch Bondwell.cmd** in `bondwellsim`,
or launch both disks directly:

```powershell
cd bondwellsim
.\build\bondwellsim.exe --disk-a disks/DISK1_HFE.img --disk-b disks/DISK2_HFE.img
```

Allow the ROM diagnostics to finish, press **Space** at the boot prompt,
then choose **E** in Bondwell SETUP to reach the CP/M `A>` prompt.

Try:

```text
DIR
DIR B:
```

Disk images are read-only by default. With `--writable`, changes are saved
to separate `.updated` files, preserving the mounted originals.

For Linux/macOS builds, headless operation, tracing, disk conversion,
and peripheral limitations, see the
[Bondwell emulator guide](bondwellsim/README.md).

---

## Upstream z80pack

The original z80pack documentation follows.

# z80pack

z80pack is a mature emulator of multiple platforms with 8080 and Z80 CPU.


Full documentation is at https://www.icl1900.co.uk/unix4fun/z80pack

## Ubuntu 

### Building
First install the needed dependencies for X11:

    sudo apt install build-essential libglu1-mesa-dev libjpeg9-dev

or for SDL2:

    sudo apt install build-essential libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev

Then for X11 run  

    make

or for SDL2 run  

    WANT_SDL=YES make

to build all the MACHINES mentioned in the Makefile.  

## Release vs Development

Sometimes I get asked questions why something doesn't work, and this might
be caused by using an older version and not the latest current. Sometimes
people miss it, that the repository most of the time has two branches:

```
master - finished and final releases
dev - latest sources still under development, but usually stable
```

To use the latest dev version you need to do this:

```
git clone https://github.com/udo-munk/z80pack.git
cd z80pack
git checkout dev
```

You now will build everything from the latest dev branch and not some older
finished release.

### Running CP/M 2.2

CP/M 2.2 is the ancestor of MS-DOS. Use this command to invoke CP/M 2.2 with
two disks containing some sample programs and sources.


    (cd cpmsim; ./cpm22)

Use `DIR` to see files on disk. Exit again with `BYE`

Sample execution in WSL under Windows 10: 

```
#######  #####    ###            #####    ###   #     #
     #  #     #  #   #          #     #    #    ##   ##
    #   #     # #     #         #          #    # # # #
   #     #####  #     #  #####   #####     #    #  #  #
  #     #     # #     #               #    #    #     #
 #      #     #  #   #          #     #    #    #     #
#######  #####    ###            #####    ###   #     #

Release 1.38, Copyright (C) 1987-2024 by Udo Munk and others

CPU speed is unlimited, CPU executes undocumented instructions

Booting...

64K CP/M Vers. 2.2 (Z80 CBIOS V1.2 for Z80SIM, Copyright 1988-2007 by Udo Munk)

A>dir
A: DUMP     COM : SDIR     COM : SUBMIT   COM : ED       COM
A: STAT     COM : BYE      COM : RMAC     COM : CREF80   COM
A: LINK     COM : L80      COM : M80      COM : SID      COM
A: RESET    COM : WM       HLP : ZSID     COM : MAC      COM
A: TRACE    UTL : HIST     UTL : LIB80    COM : WM       COM
A: HIST     COM : DDT      COM : Z80ASM   COM : CLS      COM
A: SLRNK    COM : MOVCPM   COM : ASM      COM : LOAD     COM
A: XSUB     COM : LIB      COM : PIP      COM : SYSGEN   COM
A>dir B:
B: BOOT     HEX : BYE      ASM : CLS      MAC : SURVEY   MAC
B: R        ASM : CLS      COM : BOOT     Z80 : W        ASM
B: RESET    ASM : BYE      COM : SYSGEN   SUB : BIOS     HEX
B: CPM64    SYS : SPEED    C   : BIOS     Z80 : SPEED    COM
B: SURVEY   COM : R        COM : RESET    COM : W        COM
A>bye

System halted
CPU ran 3 ms and executed 1958078 t-states
Clock frequency 630.22 MHz
```

### Running CP/M 3

CP/M 3 was the next generation of CP/M with features from MP/M to notably
be able to use more RAM along with a lot of other nice features.  

Run with:

    (cd cpmsim; ./cpm3)

Sample run:

``` 
#######  #####    ###            #####    ###   #     #
     #  #     #  #   #          #     #    #    ##   ##
    #   #     # #     #         #          #    # # # #
   #     #####  #     #  #####   #####     #    #  #  #
  #     #     # #     #               #    #    #     #
 #      #     #  #   #          #     #    #    #     #
#######  #####    ###            #####    ###   #     #

Release 1.38, Copyright (C) 1987-2024 by Udo Munk and others

CPU speed is unlimited, CPU executes undocumented instructions

Booting...


LDRBIOS3 V1.2 for Z80SIM, Copyright 1989-2007 by Udo Munk

CP/M V3.0 Loader
Copyright (C) 1998, Caldera Inc.

 BNKBIOS3 SPR  FC00  0400
 BNKBIOS3 SPR  8600  3A00
 RESBDOS3 SPR  F600  0600
 BNKBDOS3 SPR  5800  2E00

 61K TPA

BANKED BIOS3 V1.6-HD, Copyright 1989-2015 by Udo Munk

A>setdef [no display]

Program Name Display - Off

A>setdef [uk]

Date format used     - UK

A>setdef *,a:,b:,i:

Drive Search Path:
1st Drive            - Default
2nd Drive            - A:
3rd Drive            - B:
4th Drive            - I:


A>setdef [order=(com,sub)]

Search Order         - COM, SUB

A>setdef [temporary=a]

Temporary Drive      - A:

A>hist

History RSX active
A>vt100dyn
(C) Alexandre MONTARON - 2015 - VT100DYN

RSX loaded and initialized.

Try

 A>DEVICE CONSOLE [PAGE]

to see if it works...

A>dir a:
A: CPM3     SYS : VT100DYN COM : TRACE    UTL : HIST     UTL : PROFILE  SUB
SYSTEM FILE(S) EXIST
A>dir b:
B: BNKBDOS3 SPR : CPM3     SYS : LDRBIOS3 MAC : SCB      MAC : RESBDOS3 SPR
B: BIOS3    MAC : PATCH    COM : GENCPM   COM : BDOS3    SPR : GENCPM   DAT
B: BOOT     Z80 : M80      COM : LINK     COM : L80      COM : WM       COM
B: MAC      COM : WM       HLP : BNKBIOS3 SPR : LDR      SUB : INITDIR  COM
B: CPMLDR   COM : COPYSYS  COM : CPMLDR   REL : RMAC     COM : SYSGEN   SUB
A>bye

System halted
CPU ran 14 ms and executed 10493728 t-states
Clock frequency 713.42 MHz
```

