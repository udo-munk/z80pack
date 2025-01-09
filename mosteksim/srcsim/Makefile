###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim'
# and the executable saved as '../machinesim'
MACHINE = mostek
# machine specific system source files
MACHINE_SRCS = simcfg.c simio.c simmem.c simctl.c
# machine specific I/O source files
IO_SRCS = simbdos.c unix_terminal.c mostek-cpu.c mostek-fdc.c

# Installation directories by convention
# http://www.gnu.org/prep/standards/html_node/Directory-Variables.html
PACKAGE = z80pack
CPROG = $(MACHINE)sim
PREFIX ?= /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
DATAROOTDIR = $(PREFIX)/share
DATADIR = $(DATAROOTDIR)/$(PACKAGE)/$(CPROG)
DOCDIR = $(DATAROOTDIR)/doc/$(PACKAGE)/$(CPROG)

# system wide location for machines configuration files
CONF_DIR = $(DATADIR)/conf
# system wide location for disk images
DISKS_DIR = $(DATADIR)/disks
# default boot ROM path
ROMS_DIR = $(DATADIR)/roms
###
### END MACHINE DEPENDENT VARIABLES
###

SIM = ../$(MACHINE)sim

CORE_DIR = ../../z80core
IO_DIR = ../../iodevices

VPATH = $(CORE_DIR) $(IO_DIR)

include $(CORE_DIR)/Makefile.in-os

DEFS = -DCONFDIR=\"$(CONF_DIR)\" -DDISKSDIR=\"$(DISKS_DIR)\" \
	-DBOOTROM=\"$(ROMS_DIR)\"
INCS = -I. -I$(CORE_DIR) -I$(IO_DIR)
CPPFLAGS = $(DEFS) $(INCS)

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS = -Wall -Wextra -Wwrite-strings

# Production - the default
COPTS = -O3 -U_FORTIFY_SOURCE

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
COPTS = -O3 -fstack-protector-all -D_FORTIFY_SOURCE=2
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
COPTS = -O -g
endif

CFLAGS = $(CSTDS) $(COPTS) $(CWARNS)

LDFLAGS =
LDLIBS =

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# core system source files for the CPU simulation
CORE_SRCS = sim8080.c simcore.c simdis.c simfun.c simglb.c simice.c simint.c \
	simmain.c simsdl.c simz80.c simz80-cb.c simz80-dd.c simz80-ddcb.c \
	simz80-ed.c simz80-fd.c simz80-fdcb.c
SRCS = $(CORE_SRCS) $(MACHINE_SRCS) $(IO_SRCS)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(SIM)

$(SIM): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(DEPS): sim.h

%.d: %.c
	@$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

-include $(DEPS)

build: _rm_obj all

install: $(SIM)
#	$(INSTALL) -d $(DESTDIR)$(BINDIR)
#	$(INSTALL_PROGRAM) -s $(SIM) $(DESTDIR)$(BINDIR)

uninstall:
#	rm -f $(DESTDIR)$(BINDIR)/$(CPROG)

clean: _rm_obj _rm_deps

_rm_obj:
	rm -f $(OBJS)

_rm_deps:
	rm -f $(DEPS)

distclean: clean
	rm -f $(SIM)

.PHONY: all build install uninstall clean distclean
