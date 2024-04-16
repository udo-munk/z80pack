# Developer Makefile to easily build/clean everything.
# Don't use indiscriminately, because "make tools" will install files.
#
# Targets:
#	all - build all tools and simulators
#	reassemble - reassemble all ROMs and assembler programs
#	clean
#	allclean

DESTDIR=${HOME}/bin
#DESTDIR=/usr/local/bin

LIBS = frontpanel webfrontend/civetweb
BIOSES = cpmsim/srccpm2 cpmsim/srccpm3 cpmsim/srcmpm cpmsim/srcucsd-iv \
	imsaisim/srcucsd-iv
MISC = z80sim
MACHINES = altairsim cpmsim cromemcosim imsaisim mosteksim z80sim

Z80ASM_FLAGS = -p0 -e16 -l -sn

ALTAIR_8080 = \
	altairsim/basic8k78.asm \
	altairsim/dzmation.asm \
	altairsim/fdct1.asm \
	altairsim/killbits.asm \
	altairsim/killbits2.asm \
	altairsim/kscope.asm \
	altairsim/life.asm \
	altairsim/microchess.asm \
	altairsim/roms/als8-rom.asm \
	altairsim/roms/apple.asm \
	altairsim/roms/bootromt.asm \
	altairsim/roms/bootromt-old.asm \
	altairsim/roms/cuter-mits.asm \
	altairsim/roms/dbl.asm \
	altairsim/roms/mbl.asm \
	altairsim/roms/miniboot.asm \
	altairsim/roms/tinybasic-1.0.asm \
	altairsim/roms/tinybasic-2.0.asm \
	altairsim/roms/turnmon.asm

ALTAIR_Z80 = \
	altairsim/roms/umzapex.asm \
	altairsim/roms/zapple.asm

CROMEMCO_8080 = \
	cromemcosim/dzmation.asm \
	cromemcosim/kscope.asm \
	cromemcosim/life.asm \
	cromemcosim/microchess.asm \
	cromemcosim/roms/z1mon-1.0.asm

CROMEMCO_Z80 = \
	cromemcosim/roms/rdos1.asm \
	cromemcosim/roms/rdos252.asm \
	cromemcosim/roms/z1mon-1.4.asm

IMSAI_8080 = \
	imsaisim/dzmation.asm \
	imsaisim/kscope.asm \
	imsaisim/life.asm \
	imsaisim/microchess.asm \
	imsaisim/roms/basic8k.asm \
	imsaisim/roms/memon80.asm \
	imsaisim/roms/viofm1.asm \
	imsaisim/scs1.asm

IMSAI_Z80 = \
	imsaisim/roms/basic4k.asm

all: z80asm cpmtools libs bioses misc machines

z80asm:
	$(MAKE) -C z80asm "DESTDIR=$(DESTDIR)" install

cpmtools:
	$(MAKE) -C cpmsim/srctools "DESTDIR=$(DESTDIR)" install

libs:
	@set -e; for subdir in $(LIBS); do \
		$(MAKE) -C $$subdir; \
	done

bioses:
	@set -e; for subdir in $(BIOSES); do \
		$(MAKE) -C $$subdir; \
	done

misc:
	@set -e; for subdir in $(MISC); do \
		$(MAKE) -C $$subdir; \
	done

machines:
	@set -e; for subdir in $(MACHINES); do \
		$(MAKE) -C $$subdir/srcsim; \
	done

reassemble: z80asm
	@set -e; for file in $(ALTAIR_8080) $(CROMEMCO_8080) $(IMSAI_8080); do \
		z80asm -8 $(Z80ASM_FLAGS) "$$file"; \
	done
	@set -e; for file in $(ALTAIR_Z80) $(CROMEMCO_Z80) $(IMSAI_Z80); do \
		z80asm $(Z80ASM_FLAGS) "$$file"; \
	done

clean:
	@set -e; for subdir in $(TOOLS) $(LIBS) $(BIOSES) $(MISC); do \
		$(MAKE) -C $$subdir clean; \
	done
	@set -e; for subdir in $(MACHINES); do \
		$(MAKE) -C $$subdir/srcsim clean; \
	done

allclean:
	@set -e; for subdir in $(TOOLS) $(LIBS) $(BIOSES) $(MISC); do \
		$(MAKE) -C $$subdir allclean; \
	done
	@set -e; for subdir in $(MACHINES); do \
		$(MAKE) -C $$subdir/srcsim allclean; \
	done

.PHONY: all z80asm cpmtools libs bioses misc machines reassemble clean allclean
