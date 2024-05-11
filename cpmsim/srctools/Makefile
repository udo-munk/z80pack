#
# some places where the tools usually are installed
#
DESTDIR=$(HOME)/bin
#DESTDIR=/usr/local/bin

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O3 $(CSTDS) $(CWARNS)

TOOLS = mkdskimg bin2hex cpmsend cpmrecv ptp2bin

all: $(TOOLS)
	@echo
	@echo "Done."
	@echo "Now run make install"
	@echo

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
	install -d $(DESTDIR)
	install -s $(TOOLS) $(DESTDIR)
	@echo
	@echo "Tools installed in ${DESTDIR}, make sure it is"
	@echo "included in the systems search PATH"
	@echo

clean:
	rm -f $(TOOLS)

allclean: clean

.PHONY: all test install clean allclean
