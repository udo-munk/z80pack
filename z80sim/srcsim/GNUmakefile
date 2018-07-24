###
### START MACHINE DEPENDANT VARIABLES 
###
# (simple) machine name - will be suffixed with 'sim' and the executable saved as '../machinesim'
MACHINE = z80
# machine specific system source files
SRC_MACHINE = config.c disas.c iosim.c memory.c simctl.c 
# machine specific I/O source files
SRC_IO = 
#machine specifc libraries 
SRC_LIBS =

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

# DIR_ROOT = $(DATAROOTDIR)/$(CPROG)
###
### END MACHINE DEPENDANT VARIABLES 
###

SIM = ../$(MACHINE)sim
EXEC = $(SIM)

DIR_CORE = ../../z80core

VPATH = $(DIR_CORE)
CC = gcc

###
### START O/S PLATFORM DEPENDANT VARIABLES
###
include ../../z80core/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
CC = cc
PLAT_INCL =
PLAT_LFLAGS =
endif
ifeq ($(TARGET_OS),WIN32)
EXEC := $(EXEC:=.exe)
PLAT_LFLAGS =
.LIBPATTERNS += lib%.dll.a
endif
ifeq ($(TARGET_OS),LINUX)
PLAT_LFLAGS =
endif 
ifeq ($(TARGET_OS),OSX)
PLAT_INCL =
PLAT_CFLAGS = -Wno-self-assign
PLAT_LFLAGS =
.LIBPATTERNS += lib%.dylib
endif
###
### END O/S DEPENDANT VARIABLES
###

INCL = -iquote . -I$(DIR_CORE)

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

LFLAGS = $(PLAT_LFLAGS)

# core system source files for the CPU simulation - only change if the core changes
SRC_CORE = sim0.c sim1.c sim1a.c sim2.c sim3.c sim4.c sim5.c sim6.c sim7.c simfun.c simglb.c simint.c
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

#test : ; @echo $(DIR_ROOT)

.PHONY : all build install clean allclean
