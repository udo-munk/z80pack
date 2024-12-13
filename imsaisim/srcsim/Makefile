###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim'
# and the executable saved as '../machinesim'
MACHINE = imsai
# emulate a machine's frontpanel
FRONTPANEL = YES
# machine specific system source files
MACHINE_SRCS = simcfg.c simio.c simmem.c simctl.c
# machine specific I/O source files
IO_SRCS = cromemco-dazzler.c cromemco-88ccc.c cromemco-d+7a.c diskmanager.c \
	imsai-fif.c imsai-sio2.c imsai-hal.c imsai-vio.c unix_terminal.c \
	unix_network.c netsrv.c generic-at-modem.c libtelnet.c rtc80.c \
	simbdos.c am9511.c floatcnv.c ova.c
# machine specific libraries
CIV_LIB = $(CIV_DIR)/libcivetweb.a
CIV_LDLIBS = -lcivetweb

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
# web-based frontend document root
DOCROOT_DIR = $(DATADIR)/www
###
### END MACHINE DEPENDENT VARIABLES
###

###
### FRONTPANEL VARIABLES
###
ifeq ($(FRONTPANEL),YES)
FP_SRCS = fpmain.cpp
FP_DEFS = -DFRONTPANEL
FP_LIB = $(FP_DIR)/libfrontpanel.a
FP_LDLIBS = -lfrontpanel -ljpeg -lGL -lGLU
LINK = $(CXX)
LINKFLAGS = $(CXXFLAGS)
else
FP_SRCS =
FP_DEFS =
FP_LIB =
FP_LDLIBS =
LINK = $(CC)
LINKFLAGS = $(CFLAGS)
endif
###
### END FRONTPANEL VARIABLES
###

SIM = ../$(MACHINE)sim

CORE_DIR = ../../z80core
IO_DIR = ../../iodevices
FP_DIR = ../../frontpanel
NET_DIR = ../../webfrontend
CIV_DIR = $(NET_DIR)/civetweb

VPATH = $(CORE_DIR) $(IO_DIR) $(IO_DIR)/apu $(FP_DIR) $(NET_DIR) $(CIV_DIR)

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
PLAT_INCS = -I/usr/local/include
PLAT_LDFLAGS = -L/usr/local/lib
PLAT_LDLIBS = -lthr -lm
endif
ifeq ($(TARGET_OS),LINUX)
PLAT_LDLIBS = -lm -lpthread
endif
ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/X11/include
PLAT_LDFLAGS = -L/usr/local/lib -L/opt/X11/lib
endif
###
### END O/S DEPENDENT VARIABLES
###

DEFS = -DCONFDIR=\"$(CONF_DIR)\" -DDISKSDIR=\"$(DISKS_DIR)\" \
	-DBOOTROM=\"$(ROMS_DIR)\" -DSYSDOCROOT=\"$(DOCROOT_DIR)\" $(FP_DEFS)
INCS = -I. -I$(CORE_DIR) -I$(IO_DIR) -I$(FP_DIR) -I$(NET_DIR) \
	-I$(CIV_DIR)/include $(PLAT_INCS)
CPPFLAGS = $(DEFS) $(INCS)

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS = -Wall -Wextra -Wwrite-strings
CXXSTDS = -std=c++03 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CXXWARNS = -Wall -Wextra

# Production - the default
COPTS = -O3 -U_FORTIFY_SOURCE
CXXOPTS = -O3 -U_FORTIFY_SOURCE

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
COPTS = -O3 -fstack-protector-all -D_FORTIFY_SOURCE=2
CXXOPTS = -O3 -fstack-protector-all -D_FORTIFY_SOURCE=2
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
COPTS = -O -g
CXXOPTS = -O -g
endif

CFLAGS = $(CSTDS) $(COPTS) $(CWARNS) $(PLAT_CFLAGS)
CXXFLAGS = $(CXXSTDS) $(CXXOPTS) $(CXXWARNS) $(PLAT_CXXFLAGS)

LDFLAGS = -L$(FP_DIR) -L$(CIV_DIR) $(PLAT_LDFLAGS)
LDLIBS = $(CIV_LDLIBS) $(FP_LDLIBS) -lX11 $(PLAT_LDLIBS)

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# core system source files for the CPU simulation
CORE_SRCS = sim8080.c simcore.c simdis.c simfun.c simglb.c simice.c simint.c \
	simmain.c simz80.c simz80-cb.c simz80-dd.c simz80-ddcb.c simz80-ed.c \
	simz80-fd.c simz80-fdcb.c
SRCS = $(CORE_SRCS) $(MACHINE_SRCS) $(IO_SRCS)
XXSRCS = $(FP_SRCS)
OBJS = $(SRCS:.c=.o) $(XXSRCS:.cpp=.o)
DEPS = $(SRCS:.c=.d) $(XXSRCS:.cpp=.d)

all: $(SIM)

$(SIM): $(OBJS) $(CIV_LIB) $(FP_LIB)
	$(LINK) $(LINKFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(DEPS): sim.h

%.d: %.c
	@$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $(CPPFLAGS) $< > $@

-include $(DEPS)

$(CIV_LIB): FORCE
	$(MAKE) -C $(CIV_DIR)

$(FP_LIB): FORCE
	$(MAKE) -C $(FP_DIR)

FORCE:

build: _rm_obj all

install: $(SIM)
#	$(INSTALL) -d $(DESTDIR)$(BINDIR)
#	$(INSTALL_PROGRAM) -s $(SIM) $(DESTDIR)$(BINDIR)
#	$(INSTALL) -d $(DESTDIR)$(DOCROOT_DIR)
#	(cd $(NET_DIR)/www/$(MACHINE); \
#	find . -type d -exec $(INSTALL) -d $(DESTDIR)$(DOCROOT_DIR)/\{\} \;; \
#	find . -type f -exec $(INSTALL_DATA) \{\} $(DESTDIR)$(DOCROOT_DIR) \;)

uninstall:
#	rm -f $(DESTDIR)$(BINDIR)/$(CPROG)
#	rm -rf $(DESTDIR)$(DATADIR)

clean: _rm_obj _rm_deps

_rm_obj:
	rm -f $(OBJS)

_rm_deps:
	rm -f $(DEPS)

distclean: clean
	rm -f $(SIM)
	rm -f ../disks/drive*.dsk
	rm -f ../printer.txt

.PHONY: all FORCE build install uninstall clean distclean
