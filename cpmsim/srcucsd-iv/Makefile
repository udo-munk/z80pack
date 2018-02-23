CC = gcc

CFLAGS= -O -s -Wall

all: boot.bin bios.bin putsys
	@echo "done"

boot.bin: boot.asm
	z80asm -fb -l -sn boot.asm

bios.bin: bios.asm
	z80asm -fb -l -sn bios.asm

putsys: putsys.c
	$(CC) $(CFLAGS) -o putsys putsys.c

clean:
	rm -f *.lis *.bin putsys putsys.exe
