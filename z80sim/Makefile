all: float.hex z80main.hex z80opsall.hex 8080opsall.hex

float.hex: float.asm
	z80asm -fh -l -sn -p0 $<

z80main.hex: z80main.asm z80dis.asm z80ops.asm
	z80asm -fh -l -sn -p0 $<

z80opsall.hex: z80opsall.asm
	z80asm -u -fh -l -sn -p0 $<

8080opsall.hex: 8080opsall.asm
	z80asm -fh -l -sn -p0 $<

clean:
	rm -f float.hex float.lis z80main.hex z80main.lis \
		z80opsall.hex z80opsall.lis 8080opsall.hex 8080opsall.lis

allclean: clean

.PHONY: all clean allclean
