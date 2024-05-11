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

OBJS =	z80amain.o \
	z80atab.o \
	z80anum.o \
	z80aout.o \
	z80amfun.o \
	z80arfun.o \
	z80apfun.o \
	z80aopc.o \
	z80aglb.o

all: z80asm

z80asm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o z80asm

z80amain.o: z80amain.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80amain.c

z80atab.o: z80atab.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80atab.c

z80anum.o: z80anum.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80anum.c

z80aout.o: z80aout.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80aout.c

z80amfun.o: z80amfun.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80amfun.c

z80arfun.o: z80arfun.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80arfun.c

z80apfun.o: z80apfun.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80apfun.c

z80aopc.o: z80aopc.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) -c z80aopc.c

z80aglb.o: z80aglb.c z80a.h
	$(CC) $(CFLAGS) -c z80aglb.c

instcoh: z80asm
	strip z80asm
	cp z80asm $(DESTDIR)$(BINDIR)

install: z80asm
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL_PROGRAM) -s z80asm $(DESTDIR)$(BINDIR)

uninstall:
	rm $(DESTDIR)$(BINDIR)/z80asm

clean:
	rm -f core $(OBJS) z80asm

distclean: clean

.PHONY: all instcoh install uninstall clean distclean
