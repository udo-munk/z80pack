all: serial.c tb.c blink.c
	@echo
	@echo "Done."
	@echo

serial.c: serial.asm
	z80asm -fc -l -sn -p0 $<

tb.c: tb.asm
	z80asm -8 -fc -l -sn -p0 -x $<

blink.c: blink.asm
	z80asm -fc -l -sn -p0 $<

clean:
	rm -f serial.c tb.c blink.c

allclean: clean

.PHONY: all clean allclean
