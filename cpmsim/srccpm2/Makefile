CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CWARNS)
LDFLAGS= -s

all: putsys bios.bin boot.bin
	@echo "done"

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

bios.bin: bios.asm
	z80asm -vl -sn -fm -x bios.asm

boot.bin: boot.asm
	z80asm -vl -sn -fm boot.asm

clean:
	rm -f *.lis bios.bin boot.bin putsys putsys.exe

allclean: clean

.PHONY: all clean allclean
