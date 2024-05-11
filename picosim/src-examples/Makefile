Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMOPTS = -l -sn -p0

all: serial.c tb.c blink.c
	@echo
	@echo "Done."
	@echo

serial.c: serial.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fc $<

tb.c: tb.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fc -x $<

blink.c: blink.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fc $<

$(Z80ASM):
	$(MAKE) -C $(Z80ASMDIR) z80asm

clean:
	rm -f serial.c tb.c blink.c

allclean: clean

.PHONY: all clean allclean
