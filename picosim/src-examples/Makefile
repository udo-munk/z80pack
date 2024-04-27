all :	serial.c tb.c blink.c
	@echo
	@echo "Done."
	@echo

clean :
	rm -rf serial.c tb.c blink.c

serial.c :	serial.asm
	z80asm -fc -l -sn -p0 $?

tb.c :		tb.asm
	z80asm -8 -fc -l -sn -p0 -x $?

blink.c :	blink.asm
	z80asm -fc -l -sn -p0 $?
