Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -fb -l -T -sn -p0

all: serial.bin tb.bin blink.bin test8080.bin

serial.bin: serial.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) $<

tb.bin: tb.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -8 -x $<

blink.bin: blink.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) $<

test8080.bin: test8080.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -8 -x $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

install:

uninstall:

clean:
	rm -f serial.bin tb.bin blink.bin

distclean: clean

.PHONY: all FORCE install uninstall clean distclean
