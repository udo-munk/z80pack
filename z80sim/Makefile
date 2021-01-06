all: float.hex z80main.hex

float.hex: float.asm
	z80asm -v -l -sn float.asm

z80main.hex: z80main.asm z80dis.asm z80ops.asm
	z80asm -v -l -sn z80main.asm

clean:
	rm -f *.hex *.lis
