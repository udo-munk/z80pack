CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CSTDS) $(CWARNS)
LDFLAGS= -s

Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMOPTS = -l -sn -p0

all: putsys boot.bin bios.bin
	@echo
	@echo "Done."
	@echo

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

boot.bin: boot.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fb $<

bios.bin: bios.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fb $<

$(Z80ASM):
	$(MAKE) -C $(Z80ASMDIR) z80asm

clean:
	rm -f putsys boot.bin boot.lis bios.bin bios.lis

allclean: clean

.PHONY: all clean allclean
