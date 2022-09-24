all: float.hex z80main.hex z80opsall.hex 8080opsall.hex

float.hex: float.asm
	z80asm -v -l -sn float.asm

z80main.hex: z80main.asm z80dis.asm z80ops.asm
	z80asm -v -l -sn z80main.asm

z80opsall.hex: z80opsall.asm
	z80asm -v -l -sn -u z80opsall.asm

8080opsall.hex: 8080opsall.asm
	z80asm -v -l -sn 8080opsall.asm

clean:
	rm -f *.hex *.lis
