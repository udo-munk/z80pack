CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CWARNS)
LDFLAGS= -s

all: boot.bin bios.bin putsys
	@echo "done"

boot.bin: boot.asm
	z80asm -fb -l -sn boot.asm

bios.bin: bios.asm
	z80asm -fb -l -sn bios.asm

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

clean:
	rm -f *.lis *.bin putsys putsys.exe

allclean: clean

.PHONY: all clean allclean
