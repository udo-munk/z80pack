###
### START MACHINE DEPENDENT VARIABLES
###
# (simple) machine name - will be suffixed with 'sim'
# and the executable saved as '../machinesim'
MACHINE = cromemco
# emulate a machine's frontpanel
FRONTPANEL ?= YES
# show an introspection panel
INFOPANEL ?= YES
# use SDL2 instead of X11
WANT_SDL ?= NO
# machine specific system source files
MACHINE_SRCS = simcfg.c simio.c simmem.c simctl.c
# machine specific I/O source files
IO_SRCS = cromemco-wdi.c cromemco-d+7a.c cromemco-dazzler.c cromemco-fdc.c \
	cromemco-tu-art.c cromemco-hal.c unix_terminal.c unix_network.c \
	simbdos.c netsrv.c generic-at-modem.c libtelnet.c diskmanager.c
# CivetWeb library
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

SIM = ../$(MACHINE)sim

CORE_DIR = ../../z80core
IO_DIR = ../../iodevices
FP_DIR = ../../frontpanel
NET_DIR = ../../webfrontend
CIV_DIR = $(NET_DIR)/civetweb

VPATH = $(CORE_DIR) $(IO_DIR) $(FP_DIR) $(NET_DIR) $(CIV_DIR)

include $(CORE_DIR)/Makefile.in-os

###
### SDL2/X11 AND INFOPANEL PLATFORM VARIABLES
###
ifeq ($(WANT_SDL),YES)
ifeq ($(INFOPANEL),YES)
PLAT_DEFS = -DINFOPANEL -DWANT_SDL
PLAT_SRCS = simpanel.c simsdl.c
else
PLAT_DEFS = -DWANT_SDL
PLAT_SRCS = simsdl.c
endif
ifeq ($(TARGET_OS),BSD)
PLAT_INCS = -I/usr/local/include/SDL2
PLAT_LDFLAGS = -L/usr/local/lib
PLAT_LDLIBS = -lSDL2 -lSDL2main
else ifeq ($(TARGET_OS),LINUX)
PLAT_INCS = -I/usr/include/SDL2
PLAT_LDLIBS = -lSDL2 -lSDL2main
else ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/homebrew/include/SDL2 
PLAT_LDLIBS = -L/opt/homebrew/lib -lSDL2
PLAT_LDFLAGS = -Wl,-search_paths_first -Wl,-headerpad_max_install_names -Wl,-rpath,/Library/Frameworks
endif
else
ifeq ($(INFOPANEL),YES)
PLAT_DEFS = -DINFOPANEL
PLAT_SRCS = simpanel.c
endif
ifeq ($(TARGET_OS),BSD)
PLAT_INCS = -I/usr/local/include
PLAT_LDFLAGS = -L/usr/local/lib
PLAT_LDLIBS = -lX11
else ifeq ($(TARGET_OS),LINUX)
PLAT_LDLIBS = -lX11
else ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/X11/include -I/opt/local/include -I/usr/local/include
PLAT_LDFLAGS = -L/opt/X11/lib -L/usr/local/lib
PLAT_LDLIBS = -lX11
endif
endif
###
### END SDL2/X11 AND INFOPANEL PLATFORM VARIABLES
###

###
### FRONTPANEL VARIABLES
###
ifeq ($(FRONTPANEL),YES)
FP_DEFS = -DFRONTPANEL
FP_LIB = $(FP_DIR)/libfrontpanel.a
ifeq ($(WANT_SDL),YES)
ifeq ($(TARGET_OS),OSX)
FP_INCS = -I/opt/homebrew/include/SDL2 
FP_LDLIBS = -lfrontpanel -L/opt/homebrew/lib -lSDL2_mixer -lSDL2_image -framework OpenGL
else
FP_LDLIBS = -lfrontpanel -lSDL2_image -lSDL2_mixer -lGL
endif
else
FP_LDLIBS = -lfrontpanel -ljpeg -lGL
endif
endif
###
### END FRONTPANEL VARIABLES
###

DEFS = -DCONFDIR=\"$(CONF_DIR)\" -DDISKSDIR=\"$(DISKS_DIR)\" \
	-DBOOTROM=\"$(ROMS_DIR)\" -DSYSDOCROOT=\"$(DOCROOT_DIR)\" $(FP_DEFS) \
	$(PLAT_DEFS)
INCS = -I. -I$(CORE_DIR) -I$(IO_DIR) -I$(FP_DIR) -I$(NET_DIR) \
	-I$(CIV_DIR)/include $(PLAT_INCS) $(FP_INCS)
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

LDFLAGS = -L$(FP_DIR) -L$(CIV_DIR) $(PLAT_LDFLAGS)
LDLIBS = $(CIV_LDLIBS) $(FP_LDLIBS) $(PLAT_LDLIBS) -lm -lpthread

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# core system source files for the CPU simulation
CORE_SRCS = sim8080.c simcore.c simdis.c simfun.c simglb.c simice.c simint.c \
	simmain.c simz80.c simz80-cb.c simz80-dd.c simz80-ddcb.c simz80-ed.c \
	simz80-fd.c simz80-fdcb.c
SRCS = $(CORE_SRCS) $(MACHINE_SRCS) $(IO_SRCS) $(PLAT_SRCS)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(SIM)

$(SIM): $(OBJS) $(CIV_LIB) $(FP_LIB)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(DEPS): sim.h

%.d: %.c
	@$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

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
	rm -f *.o

_rm_deps:
	rm -f *.d

distclean: clean
	rm -f $(SIM)
	rm -f ../disks/drive*.dsk
	rm -f ../lpt[12].txt

.PHONY: all FORCE build install uninstall clean distclean
