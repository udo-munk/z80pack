Here are some patches to get the Zilog Z80 CPU test suite from
https://github.com/raxoft/z80test, which was written for the
ZX Spectrum, to run on z80sim.

Unfortunately z80asm can't assemble the sources, because they
use a completely different macro facility and have other syntactic
differences from classic Z80 assembler.

One needs to download sjasm from https://xl2s.tk/ .

bin2hex from z80pack-x.x/cpmsim/srctools must also be installed.

The patches are:

z80sim.patch	adds an I/O port 254 that always reads 0xbf
		like on a ZX Spectrum
z80test.patch	patches to make z80test run with z80sim and
		also disable the tests that use undocumented
		instructions that are not implemented in z80pack

cd $HOME/z80pack-x.x/z80sim/srcsim
patch -p3 < ../z80test/z80sim.patch
make

cd $HOME
git clone https://github.com/raxoft/z80test
cd z80test
patch -p1 < $HOME/z80pack-x.x/z80sim/z80test/z80test.patch
cd src
sjasm z80doc.asm 
sjasm z80docflags.asm 

cd $HOME/z80pack-x.x/z80sim
bin2hex -o 32768 $HOME/z80test/src/z80doc.out z80doc.hex
bin2hex -o 32768 $HOME/z80test/src/z80docflags.out z80docflags.hex

Finally run "./z80sim -x z80doc.hex" etc.
