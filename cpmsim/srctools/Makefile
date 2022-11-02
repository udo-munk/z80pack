#
# some places where the tools usually are installed
#
INSTALLDIR=${HOME}/bin
#INSTALLDIR=/usr/local/bin

#CC = cc
CC = gcc

CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O3 $(CWARNS)

all: test mkdskimg bin2hex cpmsend cpmrecv ptp2bin
	@echo
	@echo "Done."
	@echo "Now run make install"
	@echo

test:
	@test -d ${INSTALLDIR} || (echo "${INSTALLDIR} doesn't exist, fix INSTALLDIR"; exit 1)

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

install:
	cp mkdskimg ${INSTALLDIR}
	cp bin2hex ${INSTALLDIR}
	cp cpmsend ${INSTALLDIR}
	cp cpmrecv ${INSTALLDIR}
	cp ptp2bin ${INSTALLDIR}
	@echo
	@echo "Tools installed in ${INSTALLDIR}, make sure it is"
	@echo "included in the systems search PATH"
	@echo

clean:
	rm -f mkdskimg mkdskimg.exe bin2hex bin2hex.exe cpmrecv cpmrecv.exe \
	cpmsend cpmsend.exe ptp2bin ptp2bin.exe
