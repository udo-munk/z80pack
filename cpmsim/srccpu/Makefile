all: ex.com prelim.com zexdoc.com 8080pre.com 8080ex.com test8080.com sw8080.com swz80.com
	@echo "done"

ex.com: ex.mac
	z80asm -fb -l -sn -p0 -o$@ $<

prelim.com: prelim.mac
	z80asm -fb -l -sn -p0 -o$@ $<

zexdoc.com: zexdoc.mac
	z80asm -fb -l -sn -p0 -o$@ $<

8080pre.com: 8080pre.mac
	z80asm -8 -fb -l -sn -p0 -o$@ $<

8080ex.com: 8080ex.mac
	z80asm -8 -fb -l -sn -p0 -o$@ $<

test8080.com: test8080.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

sw8080.com: sw8080.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

swz80.com: swz80.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

clean:
	rm -f *.lis ex.com prelim.com zexdoc.com 8080pre.com 8080ex.com test8080.com sw8080.com swz80.com

allclean: clean

.PHONY: all clean allclean
