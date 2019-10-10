###
### START MACHINE DEPENDANT VARIABLES 
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = imsai
# machine specific system source files
SRC_MACHINE = config.c iosim.c memory.c simctl.c 
# machine specific I/O source files
SRC_IO = cromemco-dazzler.c cromemco-88ccc.c diskmanager.c imsai-fif.c imsai-sio2.c imsai-vio.c unix_terminal.c unix_network.c netsrv.c generic-at-modem.c
#machine specifc libraries 
SRC_LIBS = -lcivetweb -lfrontpanel

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

DIR_ROOT = $(DATAROOTDIR)/$(CPROG)
# system wide location for machines configuration files
CONF = $(DIR_ROOT)/conf
# system wide location for disk images
DISKS = $(DIR_ROOT)/disks
# default boot ROM
ROM = $(DIR_ROOT)/bootrom.hex
###
### END MACHINE DEPENDANT VARIABLES 
###

SIM = ../$(MACHINE)sim
EXEC = $(SIM)

DIR_CORE = ../../z80core
DIR_IO = ../../iodevices
DIR_FP = ../../frontpanel
DIR_NET = ../../webfrontend 
DIR_CIV = $(DIR_NET:=/civetweb)

VPATH = $(DIR_CORE) $(DIR_IO) $(DIR_FP) $(DIR_NET) $(DIR_CIV)
CC = gcc

###
### START O/S PLATFORM DEPENDANT VARIABLES
###
include ../../z80core/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
CC = cc
PLAT_INCL = -I/usr/local/include 
PLAT_LFLAGS = -L/usr/local/lib -lthr
endif
ifeq ($(TARGET_OS),WIN32)
EXEC := $(EXEC:=.exe)
PLAT_LFLAGS = -lXext -lm -lpthread
.LIBPATTERNS += lib%.dll.a
endif
ifeq ($(TARGET_OS),LINUX)
PLAT_LFLAGS = -lXext -lXmu -lXt -lm -lpthread
endif 
ifeq ($(TARGET_OS),OSX)
PLAT_INCL = -I/opt/X11/include 
PLAT_CFLAGS = -Wno-self-assign
PLAT_LFLAGS = -L/usr/local/lib -L/opt/X11/lib 
.LIBPATTERNS += lib%.dylib
endif
###
### END O/S DEPENDANT VARIABLES
###

INCL = -iquote . -I$(DIR_CORE) -I$(DIR_IO) -I$(DIR_FP) -I$(DIR_NET) -I$(DIR_CIV:=/include) $(PLAT_INCL)
DEFS = -DCONFDIR=\"$(CONF)\" -DDISKSDIR=\"$(DISKS)\" -DBOOTROM=\"$(ROM)\"

# Production - the default
CFLAGS = -O3 -c -Wall -Wextra $(PLAT_CFLAGS) -U_FORTIFY_SOURCE $(DEFS) $(INCL)

# Development - use `MODE=DEV make build`
ifeq ($(MODE), DEV)
CFLAGS = -O3 -c -Wall -Wextra $(PLAT_CFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(DEFS) $(INCL)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CFLAGS = -O -g -c $(PLAT_CFLAGS) $(DEFS) $(INCL)
endif

LFLAGS = -L$(DIR_FP) -L$(DIR_CIV) $(SRC_LIBS) \
        -ljpeg -lGL -lGLU -lX11 \
        $(PLAT_LFLAGS)

# core system source files for the CPU simulation - only change if the core changes
SRC_CORE = sim0.c sim1.c sim1a.c sim2.c sim3.c sim4.c sim5.c sim6.c sim7.c simfun.c simglb.c simint.c simbdos.c
SRC = $(SRC_CORE) $(SRC_MACHINE) $(SRC_IO)
OBJ = $(SRC:.c=.o)

all : $(SIM) 
	@echo
	@echo "Done."
	@echo

$(SIM) : $(OBJ) $(SRC_LIBS)
	$(CC) $(OBJ) $(LFLAGS) -o $@

%.d : %.c
	@$(CC) -MM $(CFLAGS) $< > $@

-include $(SRC:.c=.d)

-lcivetweb: $(DIR_CIV:=/Makefile)
	make -B -C $(DIR_CIV)

build: _rm_obj all

install: 
	@echo
	@echo Waiting to be written...
	@echo

clean : _rm_obj _rm_deps

_rm_obj:
	rm -f *.o

_rm_deps:
	rm -f *.d

allclean: clean
	rm -f $(EXEC)
	rm -f ../disks/drive*.dsk
	rm -f ../printer.txt

#test : ; @echo $(DIR_ROOT)

.PHONY : all build install clean allclean
