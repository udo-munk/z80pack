CWARNS= -Wall -Wextra -Wwrite-strings
CFLAGS= -O $(CWARNS)
LDFLAGS= -s

all: putsys boot.bin
	echo done

putsys: putsys.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o putsys putsys.c

boot.bin: boot.asm
	z80asm -vl -sn -fb boot.asm

clean:
	rm -f *.lis putsys boot.bin

allclean: clean

.PHONY: all clean allclean
