#
# by default compile on POSIX compatible systems
# the alternative options are provided for old ANSI C89 systems
#

# Use these to compile on Coherent
#CC = acc
#CFLAGS = -O

CSTDS = -std=c89 -D_POSIX_C_SOURCE=1
CWARNS = -Wall -Wextra -Wwrite-strings
CFLAGS = -O3 $(CSTDS) $(CWARNS)

#
# installation directory
#
PREFIX ?= $(HOME)
#PREFIX ?= /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

OBJS =	z80asm.o z80alst.o z80amfun.o z80anum.o z80aobj.o z80aopc.o \
	z80apfun.o z80arfun.o z80atab.o

all: z80asm

z80asm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o z80asm

z80asm.o: z80asm.c z80asm.h z80amfun.h z80anum.h z80alst.h z80aobj.h \
		z80aopc.h z80apfun.h z80atab.h
	$(CC) $(CFLAGS) -c z80asm.c

z80alst.o: z80alst.c z80asm.h z80amfun.h z80atab.h z80alst.h
	$(CC) $(CFLAGS) -c z80alst.c

z80amfun.o: z80amfun.c z80asm.h z80alst.h z80anum.h z80apfun.h z80amfun.h
	$(CC) $(CFLAGS) -c z80amfun.c

z80anum.o: z80anum.c z80asm.h z80atab.h z80anum.h
	$(CC) $(CFLAGS) -c z80anum.c

z80aobj.o: z80aobj.c z80asm.h z80aobj.h
	$(CC) $(CFLAGS) -c z80aobj.c

z80aopc.o: z80aopc.c z80asm.h z80alst.h z80amfun.h z80apfun.h z80arfun.h \
		z80aopc.h
	$(CC) $(CFLAGS) -c z80aopc.c

z80apfun.o: z80apfun.c z80asm.h z80alst.h z80amfun.h z80anum.h z80aobj.h \
		z80aopc.h z80atab.h z80apfun.h
	$(CC) $(CFLAGS) -c z80apfun.c

z80arfun.o: z80arfun.c z80asm.h z80anum.h z80aopc.h z80arfun.h
	$(CC) $(CFLAGS) -c z80arfun.c

z80atab.o: z80atab.c z80asm.h z80alst.h z80atab.h
	$(CC) $(CFLAGS) -c z80atab.c

instcoh: z80asm
	strip z80asm
	cp z80asm $(DESTDIR)$(BINDIR)

install: z80asm
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL_PROGRAM) -s z80asm $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/z80asm

clean:
	rm -f core $(OBJS) z80asm

distclean: clean

.PHONY: all instcoh install uninstall clean distclean
