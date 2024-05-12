Z80ASMDIR = ../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMOPTS = -l -T -sn -p0

all: float.hex z80main.hex z80opsall.hex 8080opsall.hex

float.hex: float.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fh $<

z80main.hex: z80main.asm z80dis.asm z80ops.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fh $<

z80opsall.hex: z80opsall.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -u -fh $<

8080opsall.hex: 8080opsall.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fh $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

install:

uninstall:

clean:
	rm -f float.hex float.lis z80main.hex z80main.lis \
		z80opsall.hex z80opsall.lis 8080opsall.hex 8080opsall.lis

distclean: clean

.PHONY: all FORCE install uninstall clean distclean
