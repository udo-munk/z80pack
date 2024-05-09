###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = imsai
# emulate a machine's frontpanel
FRONTPANEL = YES
# machine specific system source files
MACHINE_SRCS = config.c iosim.c memory.c simctl.c
# machine specific I/O source files
IO_SRCS = cromemco-dazzler.c cromemco-88ccc.c cromemco-d+7a.c diskmanager.c imsai-fif.c \
	imsai-sio2.c imsai-hal.c imsai-vio.c unix_terminal.c unix_network.c netsrv.c \
	generic-at-modem.c libtelnet.c rtc.c simbdos.c am9511.c floatcnv.c ova.c
# machine specific libraries
MACHINE_LIBS = -lcivetweb

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
NET_DIR = ../../webfrontend
CIV_DIR = $(NET_DIR:=/civetweb)

VPATH = $(CORE_DIR) $(IO_DIR) $(IO_DIR:=/apu) $(FP_DIR) $(NET_DIR) $(CIV_DIR)

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
PLAT_INCS = -I/usr/local/include
PLAT_LDFLAGS = -L/usr/local/lib
PLAT_LIBS = -lthr -lm
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
INCS = -iquote . -I$(CORE_DIR) -I$(IO_DIR) -I$(FP_DIR) -I$(NET_DIR) -I$(CIV_DIR:=/include) $(PLAT_INCS)

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS = -Wall -Wextra -Wwrite-strings
CXXSTDS = -std=c++03 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CXXWARNS = -Wall -Wextra

# Production - the default
CFLAGS = -O3 $(CSTDS) $(CWARNS) $(PLAT_CFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCS)
CXXFLAGS = -O3 $(CXXSTDS) $(CXXWARNS) $(PLAT_CXXFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCS)

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
CFLAGS = -O3 $(CSTDS) $(CWARNS) $(PLAT_CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCS)
CXXFLAGS = -O3 $(CXXSTDS) $(CXXWARNS) $(PLAT_CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCS)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CFLAGS = -O -g $(CSTDS) $(CWARNS) $(PLAT_CFLAGS) $(DEFS) $(INCS)
CXXFLAGS = -O -g $(CXXSTDS) $(CXXWARNS) $(PLAT_CXXFLAGS) $(DEFS) $(INCS)
endif

LDFLAGS = -L$(FP_DIR) -L$(CIV_DIR) $(PLAT_LDFLAGS) $(MACHINE_LIBS) \
	  $(FP_LDFLAGS) -lX11 $(PLAT_LIBS)

# core system source files for the CPU simulation - only change if the core changes
CORE_SRCS = sim8080.c simcore.c simdis.c simfun.c simglb.c simice.c simint.c simmain.c \
	simz80.c simz80-cb.c simz80-dd.c simz80-ddcb.c simz80-ed.c simz80-fd.c simz80-fdcb.c
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

-lcivetweb: $(CIV_DIR:=/Makefile)
	$(MAKE) -B -C $(CIV_DIR)

-lfrontpanel: $(FP_DIR:=/Makefile)
	$(MAKE) -B -C $(FP_DIR)

build: _rm_obj all

install:
	@echo
	@echo Waiting to be written...
	@echo

clean: _rm_obj _rm_deps

_rm_obj:
	rm -f $(OBJS)

_rm_deps:
	rm -f $(SRCS:.c=.d) $(XXSRCS:.cpp=.d)

allclean: clean
	rm -f $(SIM)
	rm -f ../disks/drive*.dsk
	rm -f ../printer.txt

.PHONY: all build install clean allclean
