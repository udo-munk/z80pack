Z80ASMDIR = ../../z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

all: ../srcsim/bootrom.c

../srcsim/bootrom.c: bootrom.asm $(Z80ASM)
	$(Z80ASM) $(Z80ASMFLAGS) -fc -o../srcsim/bootrom.c $<

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

.PHONY: all FORCE
