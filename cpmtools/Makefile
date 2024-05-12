# CP/M tools
TOOLS = r.com w.com bye.com reset.com sw8080.com swz80.com

# CPU tests by various authors, modified for using the CPU switch feature
CPUTESTS = ex8080.com exz80doc.com exz80all.com prelim.com 8080pre.com test8080.com

Z80ASMDIR = ../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMOPTS = -l -T -sn -p0

all: $(TOOLS) $(CPUTESTS)

r.com: r.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

w.com: w.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

bye.com: bye.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

reset.com: reset.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

sw8080.com: sw8080.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

swz80.com: swz80.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

ex8080.com: ex.mac $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -dex8080 -fb -l$(@:.com=.lis) -o$@ $<

exz80doc.com: ex.mac $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -dexz80doc -fb -l$(@:.com=.lis) -o$@ $<

exz80all.com: ex.mac $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -dexz80all -fb -l$(@:.com=.lis) -o$@ $<

prelim.com: prelim.mac $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -fb -o$@ $<

8080pre.com: 8080pre.mac $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

test8080.com: test8080.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMOPTS) -8 -fb -o$@ $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

install:

uninstall:

clean:
	rm -f $(TOOLS) $(TOOLS:.com=.lis) $(CPUTESTS) $(CPUTESTS:.com=.lis)

distclean: clean

.PHONY: all FORCE install uninstall clean distclean
