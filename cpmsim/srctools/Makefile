#
# some places where the tools usually are installed
#
PREFIX ?= $(HOME)
#PREFIX ?= /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O3 $(CSTDS) $(CWARNS)

TOOLS = mkdskimg bin2hex cpmsend cpmrecv ptp2bin

all: $(TOOLS)

mkdskimg: mkdskimg.c
	$(CC) $(CFLAGS) -o mkdskimg mkdskimg.c

bin2hex: bin2hex.c
	$(CC) $(CFLAGS) -o bin2hex bin2hex.c

cpmsend: cpmsend.c
	$(CC) $(CFLAGS) -o cpmsend cpmsend.c

cpmrecv: cpmrecv.c
	$(CC) $(CFLAGS) -o cpmrecv cpmrecv.c

ptp2bin: ptp2bin.c
	$(CC) $(CFLAGS) -o ptp2bin ptp2bin.c

install: $(TOOLS)
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL_PROGRAM) -s $(TOOLS) $(DESTDIR)$(BINDIR)

uninstall:
	cd $(DESTDIR)$(BINDIR) && rm -f $(TOOLS)

clean:
	rm -f $(TOOLS)

distclean: clean

.PHONY: all test install uninstall clean distclean
