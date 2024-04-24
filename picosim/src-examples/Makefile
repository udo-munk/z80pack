all :	serial.c
	@echo
	@echo "Done."
	@echo

clean :
	rm -rf *.c

serial.c :	serial.asm
	z80asm -fc -l -p0 $?
