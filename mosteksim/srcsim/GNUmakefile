###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = mostek
# machine specific system source files
MACHINE_SRCS = config.c iosim.c memory.c simctl.c
# machine specific I/O source files
IO_SRCS = simbdos.c unix_terminal.c mostek-cpu.c mostek-fdc.c
# machine specifc libraries
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

CC = gcc
EXEC = $(SIM)
ifeq ($(TARGET_OS),BSD)
CC = cc
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
INCS = -iquote . -I$(CORE_DIR) -I$(IO_DIR) $(PLAT_INCS)

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
	rm -f *.o

_rm_deps:
	rm -f *.d

allclean: clean
	rm -f $(EXEC)

makerules:
	@echo "# AUTOMATICALLY GENERATED DO NOT EDIT" >tmp.rules
	@echo >>tmp.rules;
	@for i in *.d; do \
		sed -e 's;$(CORE_DIR);$$(CORE_DIR);g' \
			-e 's;$(IO_DIR);$$(IO_DIR);g' <$$i >tmp.deps; \
		cat tmp.deps >>tmp.rules; \
		f=`sed 's/.*: \([^ ]*\).*/\1/;2,$$d' tmp.deps`; \
		if grep -q '\.cpp' tmp.deps; then \
			echo '	$$(CXX) $$(CXXFLAGS) -c -o $$@' $$f >>tmp.rules; \
		else \
			echo '	$$(CC) $$(CFLAGS) -c -o $$@' $$f >>tmp.rules; \
		fi ; \
		echo >>tmp.rules; \
	done
	@echo '# DO NOT PUT ANYTHING AFTER THIS LINE' >>tmp.rules;
	@for i in Makefile.*; do \
		sed '/^# AUTOMATICALLY GENERATED DO NOT EDIT/,$$d' <$$i >tmp.make; \
		cat tmp.rules >>tmp.make; \
		mv -f tmp.make $$i; \
	done
	@rm -f tmp.rules tmp.deps tmp.make

.PHONY: all build install clean allclean makerules
