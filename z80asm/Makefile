#
# by default compile on POSIX compatible systems
# the alternative options are provided for old ANSI C89 systems
#
CC = gcc
#CC = cc
#CC = acc

CFLAGS = -c -O3 -Wall -Wextra -D_POSIX_C_SOURCE=200809L
#CFLAGS = -c -O

LFLAGS =

#
# installation directory
#
DESTDIR = "${HOME}/bin"
#DESTDIR = /usr/local/bin

OBJ =   z80amain.o \
	z80atab.o \
	z80anum.o \
	z80aout.o \
	z80arfun.o \
	z80apfun.o \
	z80aopc.o \
	z80aglb.o

z80asm : $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o z80asm

z80amain.o : z80amain.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) z80amain.c

z80atab.o : z80atab.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) z80atab.c

z80anum.o : z80anum.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) z80anum.c

z80aout.o : z80aout.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) z80aout.c

z80arfun.o : z80arfun.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) z80arfun.c

z80apfun.o : z80apfun.c z80a.h z80aglb.h
	$(CC) $(CFLAGS) z80apfun.c

z80aopc.o : z80aopc.c z80a.h
	$(CC) $(CFLAGS) z80aopc.c

z80aglb.o : z80aglb.c z80a.h
	$(CC) $(CFLAGS) z80aglb.c

clean:
	rm -f core *.o z80asm

install: z80asm
	strip z80asm
	cp z80asm ${DESTDIR}
