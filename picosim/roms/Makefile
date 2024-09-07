Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

all: ../rp2040/bootrom.c ../rp2350/bootrom.c

../rp2040/bootrom.c: bootrom.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -fc -o../rp2040/bootrom.c $<

../rp2350/bootrom.c: bootrom.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -fc -o../rp2350/bootrom.c $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:
