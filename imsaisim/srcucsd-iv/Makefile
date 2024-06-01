CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CSTDS) $(CWARNS)
LDFLAGS= -s

Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

all: putsys boot.bin bios.bin

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

boot.bin: boot.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -fb $<

bios.bin: bios.asm
	$(Z80ASM) $(Z80ASMFLAGS) -fb $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

install:

uninstall:

clean:
	rm -f putsys boot.bin boot.lis bios.bin bios.lis

distclean: clean

.PHONY: all FORCE install uninstall clean distclean
