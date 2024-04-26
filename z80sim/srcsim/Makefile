###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = z80
# machine specific system source files
MACHINE_SRCS = config.c iosim.c memory.c simctl.c
# machine specific I/O source files
IO_SRCS =
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

VPATH = $(CORE_DIR)

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

EXEC = $(SIM)
ifeq ($(TARGET_OS),BSD)
endif
ifeq ($(TARGET_OS),WIN32)
EXEC := $(EXEC:=.exe)
endif
ifeq ($(TARGET_OS),LINUX)
endif
ifeq ($(TARGET_OS),OSX)
endif
###
### END O/S DEPENDENT VARIABLES
###

DEFS =
INCS = -iquote . -I$(CORE_DIR) $(PLAT_INCS)

CWARNS = -Wall -Wextra -Wwrite-strings

# Production - the default
CFLAGS = -O3 $(CWARNS) $(PLAT_CFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCS)

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
CFLAGS = -O3 $(CWARNS) $(PLAT_CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCS)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CFLAGS = -O -g $(PLAT_CFLAGS) $(DEFS) $(INCS)
endif

LDFLAGS = $(PLAT_LDFLAGS) $(MACHINE_LIBS) \
	  $(PLAT_LIBS)

# core system source files for the CPU simulation - only change if the core changes
CORE_SRCS = sim8080.c simcore.c simdis.c simfun.c simglb.c simice.c simint.c simmain.c \
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

install: $(SIM)
	cp $(SIM) ${HOME}/bin

clean: _rm_obj _rm_deps

_rm_obj:
	rm -f *.o

_rm_deps:
	rm -f *.d

allclean: clean
	rm -f $(EXEC)

.PHONY: all build install clean allclean
