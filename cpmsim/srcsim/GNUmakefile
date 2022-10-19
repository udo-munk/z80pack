###
### START MACHINE DEPENDANT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = cpm
# machine specific system source files
MACHINE_SRCS = config.c iosim.c memory.c simctl.c
# machine specific I/O source files
IO_SRCS = unix_terminal.c rtc.c simbdos.c
#machine specifc libraries
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

ROOT_DIR = $(DATAROOTDIR)/$(CPROG)
# system wide location for machines configuration files
CONF_DIR = $(ROOT_DIR)/conf
# system wide location for disk images
DISKS_DIR = $(ROOT_DIR)/disks
###
### END MACHINE DEPENDANT VARIABLES
###

SIM = ../$(MACHINE)sim
EXEC = $(SIM)

CORE_DIR = ../../z80core
IO_DIR = ../../iodevices

VPATH = $(CORE_DIR) $(IO_DIR)
CC = gcc

###
### START O/S PLATFORM DEPENDANT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
CC = cc
PLAT_INCS =
PLAT_LDFLAGS =
PLAT_LIBS =
endif
ifeq ($(TARGET_OS),WIN32)
EXEC := $(EXEC:=.exe)
PLAT_LDFLAGS =
.LIBPATTERNS += lib%.dll.a
endif
ifeq ($(TARGET_OS),LINUX)
PLAT_LIBS =
endif
ifeq ($(TARGET_OS),OSX)
PLAT_INCS =
PLAT_LDFLAGS =
.LIBPATTERNS += lib%.dylib
endif
###
### END O/S DEPENDANT VARIABLES
###

INCS = -iquote . -I$(CORE_DIR) -I$(IO_DIR) $(PLAT_INCS)
DEFS = -DCONFDIR=\"$(CONF_DIR)\" -DDISKSDIR=\"$(DISKS_DIR)\"

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
CORE_SRCS = sim0.c sim1.c sim1a.c sim2.c sim3.c sim4.c sim5.c sim6.c sim7.c simdis.c simfun.c simglb.c simice.c simint.c
SRCS = $(CORE_SRCS) $(MACHINE_SRCS) $(IO_SRCS)
OBJS = $(SRCS:.c=.o)

all: /tmp/.z80pack/cpmsim.auxin /tmp/.z80pack/cpmsim.auxout $(SIM)
	@echo
	@echo "Done."
	@echo

/tmp/.z80pack/cpmsim.auxin:
	test -d /tmp/.z80pack || mkdir /tmp/.z80pack
	test -f /tmp/.z80pack/cpmsim.auxin || \
		mknod /tmp/.z80pack/cpmsim.auxin p

/tmp/.z80pack/cpmsim.auxout:
	test -d /tmp/.z80pack || mkdir /tmp/.z80pack
	test -f /tmp/.z80pack/cpmsim.auxout || \
		mknod /tmp/.z80pack/cpmsim.auxout p

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
	rm -f *.o

_rm_deps:
	rm -f *.d

allclean: clean
	rm -f $(EXEC)
	rm -f ../disks/drive*.dsk
	rm -f ../auxiliaryin.txt ../auxiliaryout.txt ../printer.txt

#test: ; @echo $(ROOT_DIR)

.PHONY: all build install clean allclean
