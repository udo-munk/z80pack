###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = altair
# emulate a machine's frontpanel
FRONTPANEL = YES
# machine specific system source files
MACHINE_SRCS = config.c iosim.c memory.c simctl.c
# machine specific I/O source files
IO_SRCS = cromemco-dazzler.c proctec-vdm.c tarbell_fdc.c altair-88-dcdd.c altair-88-sio.c altair-88-2sio.c unix_terminal.c unix_network.c simbdos.c
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

ROOT_DIR = $(DATAROOTDIR)/$(CPROG)
# system wide location for machines configuration files
CONF_DIR = $(ROOT_DIR)/conf
# system wide location for disk images
DISKS_DIR = $(ROOT_DIR)/disks
# default boot ROM path
ROMS_DIR = $(ROOT_DIR)/roms
###
### END MACHINE DEPENDENT VARIABLES
###

###
### FRONTPANEL VARIABLES
###
ifeq ($(FRONTPANEL),YES)
FP_SRCS = fpmain.cpp
FP_DEFS = -DFRONTPANEL
FP_LIB = -lfrontpanel
FP_LDFLAGS = $(FP_LIB) -ljpeg -lGL -lGLU
CCLD = $(CXX)
else
FP_SRCS =
FP_DEFS =
FP_LIB =
FP_LDFLAGS =
CCLD = $(CC)
endif
###
### END FRONTPANEL VARIABLES
###

SIM = ../$(MACHINE)sim

CORE_DIR = ../../z80core
IO_DIR = ../../iodevices
FP_DIR = ../../frontpanel

VPATH = $(CORE_DIR) $(IO_DIR) $(FP_DIR)

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

CC = gcc
CXX = g++
EXEC = $(SIM)
ifeq ($(TARGET_OS),BSD)
CC = cc
CXX = c++
PLAT_INCS = -I/usr/local/include
PLAT_LDFLAGS = -L/usr/local/lib
PLAT_LIBS = -lthr
endif
ifeq ($(TARGET_OS),WIN32)
EXEC := $(EXEC:=.exe)
PLAT_LDFLAGS = -lm -lpthread
endif
ifeq ($(TARGET_OS),LINUX)
PLAT_LIBS = -lm -lpthread
endif
ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/X11/include
PLAT_LDFLAGS = -L/usr/local/lib -L/opt/X11/lib
endif
###
### END O/S DEPENDENT VARIABLES
###

DEFS = -DCONFDIR=\"$(CONF_DIR)\" -DDISKSDIR=\"$(DISKS_DIR)\" -DBOOTROM=\"$(ROMS_DIR)\" $(FP_DEFS)
INCS = -iquote . -I$(CORE_DIR) -I$(IO_DIR) -I$(FP_DIR) $(PLAT_INCS)

CWARNS = -Wall -Wextra -Wwrite-strings
CXXWARNS = -Wall -Wextra

# Production - the default
CFLAGS = -O3 $(CWARNS) $(PLAT_CFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCS)
CXXFLAGS = -O3 $(CXXWARNS) $(PLAT_CXXFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCS)

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
CFLAGS = -O3 $(CWARNS) $(PLAT_CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCS)
CXXFLAGS = -O3 $(CXXWARNS) $(PLAT_CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCS)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CFLAGS = -O -g $(PLAT_CFLAGS) $(DEFS) $(INCS)
CXXFLAGS = -O -g $(PLAT_CXXFLAGS) $(DEFS) $(INCS)
endif

LDFLAGS = -L$(FP_DIR) $(PLAT_LDFLAGS) $(MACHINE_LIBS) \
	  $(FP_LDFLAGS) -lX11 $(PLAT_LIBS)

# core system source files for the CPU simulation - only change if the core changes
CORE_SRCS = sim0.c sim1.c sim1a.c sim2.c sim3.c sim4.c sim5.c sim6.c sim7.c simdis.c simfun.c simglb.c simice.c simint.c
SRCS = $(CORE_SRCS) $(MACHINE_SRCS) $(IO_SRCS)
XXSRCS = $(FP_SRCS)
OBJS = $(SRCS:.c=.o) $(XXSRCS:.cpp=.o)

all: $(SIM)
	@echo
	@echo "Done."
	@echo

$(SIM): $(OBJS) $(MACHINE_LIBS) $(FP_LIB)
	$(CCLD) $(OBJS) $(LDFLAGS) -o $@

%.d: %.c
	@$(CC) -MM $(CFLAGS) $< > $@

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@

-include $(SRCS:.c=.d) $(XXSRCS:.cpp=.d)

-lfrontpanel: $(FP_DIR:=/GNUmakefile)
	make -B -C $(FP_DIR)

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
	rm -f ../disks/mits_*.dsk
	rm -f ../printer.txt

makerules:
	@echo "# AUTOMATICALLY GENERATED DO NOT EDIT" >tmp.rules
	@echo >>tmp.rules;
	@for i in *.d; do \
		sed -e 's;$(CORE_DIR);$$(CORE_DIR);g' -e 's;$(IO_DIR);$$(IO_DIR);g' \
			-e 's;$(FP_DIR);$$(FP_DIR);g' <$$i >tmp.deps; \
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
