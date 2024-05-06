# CP/M tools
TOOLS = r.com w.com bye.com reset.com sw8080.com swz80.com

# CPU tests by various authors, modified for using the CPU switch feature
CPUTESTS = ex8080.com exz80doc.com prelim.com zexdoc.com 8080pre.com test8080.com

all: $(TOOLS) $(CPUTESTS)
	@echo
	@echo "Done."
	@echo

r.com: r.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

w.com: w.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

bye.com: bye.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

reset.com: reset.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

sw8080.com: sw8080.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

swz80.com: swz80.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

ex8080.com: ex.mac
	z80asm -dex8080 -fb -l$(@:.com=.lis) -sn -p0 -o$@ $<

exz80doc.com: ex.mac
	z80asm -dexz80doc -fb -l$(@:.com=.lis) -sn -p0 -o$@ $<

prelim.com: prelim.mac
	z80asm -fb -l -sn -p0 -o$@ $<

zexdoc.com: zexdoc.mac
	z80asm -fb -l -sn -p0 -o$@ $<

8080pre.com: 8080pre.mac
	z80asm -8 -fb -l -sn -p0 -o$@ $<

test8080.com: test8080.asm
	z80asm -8 -fb -l -sn -p0 -o$@ $<

clean:
	rm -f *.lis $(TOOLS) $(CPUTESTS)

allclean: clean

.PHONY: all clean allclean
