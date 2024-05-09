CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CSTDS) $(CWARNS)
LDFLAGS= -s

all: putsys boot.bin bios.bin
	@echo
	@echo "Done."
	@echo

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

boot.bin: boot.asm
	z80asm -fb -l -sn -p0 $<

bios.bin: bios.asm
	z80asm -fb -l -sn -p0 $<

clean:
	rm -f putsys boot.bin boot.lis bios.bin bios.lis

allclean: clean

.PHONY: all clean allclean
