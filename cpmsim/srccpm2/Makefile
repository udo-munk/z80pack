CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CSTDS) $(CWARNS)
LDFLAGS= -s

all: putsys bios.bin boot.bin
	@echo "done"

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

bios.bin: bios.asm
	z80asm -vl -sn -fb -x bios.asm

boot.bin: boot.asm
	z80asm -vl -sn -fb boot.asm

clean:
	rm -f *.lis bios.bin boot.bin putsys putsys.exe

allclean: clean

.PHONY: all clean allclean
