PACKAGE = z80pack
PREFIX = $(HOME)
#PREFIX = /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
DATAROOTDIR = $(PREFIX)/share
DATADIR = $(DATAROOTDIR)/$(PACKAGE)
DOCDIR = $(DATAROOTDIR)/doc/$(PACKAGE)

TOOLS = z80asm cpmsim/srctools
LIBS = frontpanel webfrontend/civetweb
BIOSES = cpmsim/srccpm2 cpmsim/srccpm3 cpmsim/srcmpm cpmsim/srcucsd-iv \
	intelmdssim/srccpm2 imsaisim/srcucsd-iv picosim/srccpm2 \
	picosim/srccpm3 picosim/srcucsd-iv
MISC = z80sim cpmtools
MACHINES = altairsim cpmsim cromemcosim imsaisim mosteksim z80sim intelmdssim

Z80ASMDIR = z80asm
Z80ASM = $(Z80ASMDIR)/z80asm
Z80ASMFLAGS = -l -T -sn -p0

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

all: tools libs bioses misc machines

tools:
	@set -e; for subdir in $(TOOLS); do \
		$(MAKE) -C $$subdir; \
	done

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

reassemble: $(Z80ASM)
	@set -e; for file in $(ALTAIR_8080) $(CROMEMCO_8080) $(IMSAI_8080); do \
		$(Z80ASM) $(Z80ASMFLAGS) -8 -fh -e16 "$$file"; \
	done
	@set -e; for file in $(ALTAIR_Z80) $(CROMEMCO_Z80) $(IMSAI_Z80); do \
		$(Z80ASM) $(Z80ASMFLAGS) -fh -e16 "$$file"; \
	done

$(Z80ASM): FORCE
	$(MAKE) -C $(Z80ASMDIR)

FORCE:

install:
#	@set -e; for subdir in $(TOOLS) $(LIBS) $(BIOSES) $(MISC); do \
#		$(MAKE) -C $$subdir "PREFIX=$(PREFIX)" install; \
#	done
#	@set -e; for subdir in $(MACHINES); do \
#		$(MAKE) -C $$subdir/srcsim "PREFIX=$(PREFIX)" install; \
#	done

uninstall:
#	@set -e; for subdir in $(TOOLS) $(LIBS) $(BIOSES) $(MISC); do \
#		$(MAKE) -C $$subdir "PREFIX=$(PREFIX)" uninstall; \
#	done
#	@set -e; for subdir in $(MACHINES); do \
#		$(MAKE) -C $$subdir/srcsim "PREFIX=$(PREFIX)" uninstall; \
#	done
#	rmdir $(DESTDIR)$(DATADIR)

clean:
	@set -e; for subdir in $(TOOLS) $(LIBS) $(BIOSES) $(MISC); do \
		$(MAKE) -C $$subdir clean; \
	done
	@set -e; for subdir in $(MACHINES); do \
		$(MAKE) -C $$subdir/srcsim clean; \
	done

distclean:
	@set -e; for subdir in $(TOOLS) $(LIBS) $(BIOSES) $(MISC); do \
		$(MAKE) -C $$subdir distclean; \
	done
	@set -e; for subdir in $(MACHINES); do \
		$(MAKE) -C $$subdir/srcsim distclean; \
	done

.PHONY: all tools libs bioses misc machines reassemble FORCE \
		install uninstall clean distclean
