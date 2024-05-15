Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

all: serial.c tb.c blink.c

serial.c: serial.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -fc $<

tb.c: tb.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -8 -fc -x $<

blink.c: blink.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -fc $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

install:

uninstall:

clean:
	rm -f serial.c tb.c blink.c

distclean: clean

.PHONY: all FORCE install uninstall clean distclean
