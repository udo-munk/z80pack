# am9511
AM9511 floating point chip emulator. Includes a library to allow conversion of 32 bit floating point formats. AM9511,
Microsoft, IEEE, Hi-Tech C formats are supported.

This is designed to run both on a Z80, or on a generic sytem with gcc (or tcc). floatcnv works on both Z80 and hosting
platforms. am9511 is primarily to be integrated into emulation platforms. howto.txt has some instructions for integrating
into Zxc and RunCPM.

Emulation of AM9511 is now feature complete -- can be integrated. NOTE - this is beta only. If integrated, or using
live chip, AM9511.BAS is MBASIC test program that can drive both the emulator or the actual chip.

Some sample reference floating values are given for tests for floatcnv.

We are mapping AM9511 functionality into the host. This part (the AM9511) could be used with 8080, Z80, 8085, 6800,
z8000, and even Apple 2 (6502) systems, providing 16 and 32 bit integer and 32 bit floating point. After validation
with the native host floating point, I intend on providing alternate implementations that mirror the actual AM9511
chip implementation (future project).

test.com/test14.com/test58.com/test912.com  is a basic test of the emulator. These are also linked as testhw.com/testhw14.com,
testhw58.com and testhw912.com to go straight to hardware.

testhw.com (14 and 58) is the same as test.com, but runs on the actual chip:

  testhw -d port -s port
  -d port sets data port for the am9511
  -s port sets status/command port for the am9511

both port values must be in decimal. Typical values would be 80 and 81.

ova.c implements integer 16 and 32 bit arithmetic, with overflow.

am9511 is now in testing phase. All features are in, but not extensively tested.

getopt.c is the BSD getopt() function, because HI-TECH C doesn't have it.

HI-TECH C is not "fully" ANSI C 89 compatible. "ansi.h" defines signed,
const, volatile. Can't blame HI-TECH C 3.09 -- the standard was two years
AFTER this compiler.
