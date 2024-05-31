Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

all: mdsmon20.hex boot20.hex mon20.hex

mdsmon20.hex: mdsmon20.asm
	$(Z80ASM) $(Z80ASMFLAGS) -8 -fh $<

boot20.hex: mdsmon20.hex
	head -8 mdsmon20.hex >$@
	echo ":00000001FF" >>$@

mon20.hex: mdsmon20.hex
	tail +9 mdsmon20.hex >$@
