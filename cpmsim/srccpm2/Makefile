CC = gcc

CFLAGS= -O -s -Wall

all: putsys bios.bin boot.bin
	@echo "done"

putsys: putsys.c
	$(CC) $(CFLAGS) -o putsys putsys.c

bios.bin: bios.asm
	z80asm -vl -sn -x bios.asm

boot.bin: boot.asm
	z80asm -vl -sn boot.asm

clean:
	rm -f *.lis bios.bin boot.bin putsys putsys.exe
