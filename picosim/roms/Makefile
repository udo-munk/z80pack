Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

all: bootrom.c

bootrom.c: bootrom.asm
	$(Z80ASM) $(Z80ASMFLAGS) -fc $<
