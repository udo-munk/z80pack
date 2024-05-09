###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = mostek
# machine specific system source files
MACHINE_SRCS = config.c iosim.c memory.c simctl.c
# machine specific I/O source files
IO_SRCS = simbdos.c unix_terminal.c mostek-cpu.c mostek-fdc.c
# machine specific libraries
MACHINE_LIBS =

# Installation directories by convention
# http://www.gnu.org/prep/standards/html_node/Directory-Variables.html
CPROG = $(MACHINE)sim
PREFIX ?= /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
DATAROOTDIR = $(PREFIX)/share
DOCDIR = $(DATAROOTDIR)/doc/$(CPROG)
SYSCONFDIR = $(PREFIX)/etc
HTMLDIR = $(DOCDIR)
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include
LIBDIR = $(DESTDIR)$(EXEC_PREFIX)/lib
###
### END MACHINE DEPENDENT VARIABLES
###

SIM = ../$(MACHINE)sim

CORE_DIR = ../../z80core
IO_DIR = ../../iodevices

VPATH = $(CORE_DIR) $(IO_DIR)

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
endif
ifeq ($(TARGET_OS),LINUX)
endif
ifeq ($(TARGET_OS),OSX)
endif
###
### END O/S DEPENDENT VARIABLES
###

DEFS =
INCS = -iquote . -I$(CORE_DIR) -I$(IO_DIR) $(PLAT_INCS)

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS = -Wall -Wextra -Wwrite-strings

# Production - the default
CFLAGS = -O3 $(CSTDS) $(CWARNS) $(PLAT_CFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCS)

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
CFLAGS = -O3 $(CSTDS) $(CWARNS) $(PLAT_CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCS)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CFLAGS = -O -g $(CSTDS) $(CWARNS) $(PLAT_CFLAGS) $(DEFS) $(INCS)
endif

LDFLAGS = $(PLAT_LDFLAGS) $(MACHINE_LIBS) \
	  $(PLAT_LIBS)

# core system source files for the CPU simulation - only change if the core changes
CORE_SRCS = simcore.c simdis.c simfun.c simglb.c simice.c simint.c simmain.c \
	simz80.c simz80-cb.c simz80-dd.c simz80-ddcb.c simz80-ed.c simz80-fd.c simz80-fdcb.c
SRCS = $(CORE_SRCS) $(MACHINE_SRCS) $(IO_SRCS)
OBJS = $(SRCS:.c=.o)

all: $(SIM)
	@echo
	@echo "Done."
	@echo

$(SIM): $(OBJS) $(MACHINE_LIBS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.d: %.c
	@$(CC) -MM $(CFLAGS) $< > $@

-include $(SRCS:.c=.d)

build: _rm_obj all

install:
	@echo
	@echo Waiting to be written...
	@echo

clean: _rm_obj _rm_deps

_rm_obj:
	rm -f $(OBJS)

_rm_deps:
	rm -f $(SRCS:.c=.d)

allclean: clean
	rm -f $(SIM)

.PHONY: all build install clean allclean
