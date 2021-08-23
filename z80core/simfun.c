/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 * 06-JAN-21 Release 1.37 bugfixes and improvements
 */

/*
 *	This module contains some commonly used functions
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"
#include "log.h"

static const char *TAG = "func";

#define BUFSIZE	256		/* buffer size for file I/O */

int load_file(char *, BYTE pstart, WORD psize);
static int load_mos(char *, BYTE pstart, WORD psize), load_hex(char *, BYTE pstart, WORD psize), checksum(char *);

/*
 *	atoi for hexadecimal numbers
 */
int exatoi(char *str)
{
	register int num = 0;

	while (isxdigit((int)*str)) {
		num *= 16;
		if (*str <= '9')
			num += *str - '0';
		else
			num += toupper((int)*str) - '7';
		str++;
	}
	return(num);
}

/*
 *	Wait for a single keystroke without echo
 */
int getkey(void)
{
	register int c;
	struct termios old_term, new_term;

	tcgetattr(0, &old_term);
	new_term = old_term;
	new_term.c_lflag &= ~(ICANON | ECHO);
	new_term.c_cc[VMIN] = 1;
	tcsetattr(0, TCSADRAIN, &new_term);
	c = getchar();
	tcsetattr(0, TCSADRAIN, &old_term);
	return(c);
}

/*
 *	Sleep for time milliseconds, 999 max
 */
void sleep_ms(int time)
{
	struct timespec timer, rem;
	int err;

	timer.tv_sec = 0;
	timer.tv_nsec = 1000000L * time;

again:
	if (nanosleep(&timer, &rem) == -1) {
		if ((err = errno) == EINTR) {
			/* interrupted, resume with the remaining time */
			if (rem.tv_nsec > 0L) {
				memcpy(&timer, &rem, sizeof(struct timespec));
				goto again;
			}
		} else {
			/* some error */
			LOGE(TAG, "sleep_ms(%d) %s", time, strerror(err));
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

/*
 *	Compute difference between two timeval in microseconds
 *
 *	Note: yes there are timersub() and friends, but not
 *	defined in POSIX.1 and implemented wrong on some
 *	systems. Some systems define tv_usec as unsigned int,
 *	here we assume that a long is longer than unsigned.
 *	If that is not the case cast to (long long).
 */
int time_diff(struct timeval *t1, struct timeval *t2)
{
	long sec, usec;

	sec = (long) t2->tv_sec - (long) t1->tv_sec;
	usec = (long) t2->tv_usec - (long) t1->tv_usec;
	/* normalize result */
	if (usec < 0L) {
		sec--;
		usec += 1000000L;
	}
	if (sec != 0L)
		return(-1); /* result is to large */
	else
		return((int) usec);
}

/*
 *	Read a file into the memory of the emulated CPU.
 *	The following file formats are supported:
 *
 *		binary images with Mostek header
 *		Intel hex
 */
int load_file(char *s, BYTE pstart, WORD psize)
{
	char fn[MAX_LFN];
	char *pfn = fn;
	BYTE d;
	int fd;

	while (isspace((int)*s))
		s++;
	while (*s != ',' && *s != '\n' && *s != '\0')
		*pfn++ = *s++;
	*pfn = '\0';

	if (strlen(fn) == 0) {
		LOGE(TAG, "no input file given");
		return(1);
	}

	if ((fd	= open(fn, O_RDONLY)) == -1) {
		LOGE(TAG, "can't open file %s\n", fn);
		return(1);
	}

	//TODO: What is this? We need to remove references to wrk_ram and mem_base()
	// if (*s == ',')
	// 	wrk_ram	= mem_base() + exatoi(++s);
	// else
	// 	wrk_ram	= NULL;

	read(fd, (char *) &d, 1);	/* read first byte of file */
	close(fd);

	if (d == 0xff) {		/* Mostek header ? */
		return(load_mos(fn, pstart, psize));
	} else {
		return(load_hex(fn, pstart, psize));
	}
}

/*
 *	Loader for binary images with Mostek header.
 *	Format of the first 3 bytes:
 *
 *	0xff ll	lh
 *
 *	ll = load address low
 *	lh = load address high
 */
static int load_mos(char *fn, BYTE pstart, WORD psize)
{
	register int i;
	int fd;
	int laddr, count;
	BYTE fileb[3];

	if ((fd	= open(fn, O_RDONLY)) == -1) {
		LOGE(TAG, "can't open file %s\n", fn);
		return(1);
	}

	read(fd, (char *) fileb, 3);	/* read load address */
	laddr = (fileb[2] << 8) + fileb[1];

	if (psize && laddr < (pstart << 8)) {
		LOGW(TAG, "tried to load mos file outside expected address range. Address: %04X", laddr);
		return(1);
	}

	count = 0;
	for (i = laddr; i < 65536; i++) {
		if (read(fd, fileb, 1) == 1) {
			if (psize && i > ((pstart + psize) << 8)) {
				LOGW(TAG, "tried to load mos file outside expected address range. Address: %04X", i);
				return(1);
			}
			count++;
			putmem(i, fileb[0]);
		} else {
			break;
		}
	}

	close(fd);

	LOG(TAG, "Loader statistics for file %s:\r\n", fn);
	LOG(TAG, "START : %04XH\r\n", laddr);
	LOG(TAG, "END   : %04XH\\rn", laddr + count);
	LOG(TAG, "LOADED: %04XH (%d)\r\n\r\n", count, count);

	PC = laddr;

	return(0);
}

/*
 *	Loader for Intel hex
 */
static int load_hex(char *fn, BYTE pstart, WORD psize)
{
	register int i;
	FILE *fd;
	char buf[BUFSIZE];
	char *s;
	int count = 0;
	int addr = 0;
	int saddr = 0xffff;
	int eaddr = 0;
	int data;

	if ((fd = fopen(fn, "r")) == NULL) {
		LOGE(TAG, "can't open file %s\n", fn);
		return(1);
	}

	while (fgets(&buf[0], BUFSIZE, fd) != NULL) {
		s = &buf[0];
		while (isspace((int)*s))
			s++;
		if (*s != ':')
			continue;
		if (checksum(s + 1) != 0) {
			LOGE(TAG, "invalid checksum in hex record: %s", s);
			return(1);
		}
		s++;
		count = (*s <= '9') ? (*s - '0') << 4 :
				      (*s - 'A' + 10) << 4;
		s++;
		count += (*s <= '9') ? (*s - '0') :
				       (*s - 'A' + 10);
		s++;
		if (count == 0)
			break;
		addr = (*s <= '9') ? (*s - '0') << 4 :
				     (*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
				      (*s - 'A' + 10);
		s++;
		addr *= 256;
		addr += (*s <= '9') ? (*s - '0') << 4 :
				      (*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
				      (*s - 'A' + 10);
		s++;

		if (psize) {
			if (addr < (pstart << 8) || (addr + count) >= ((pstart + psize) << 8)) {
				LOGW(TAG, "tried to load hex record outside expected address range. Address: %04X", addr);
				return(1);
			}
		}

		if (addr < saddr)
			saddr = addr;
		if (addr >= eaddr)
			eaddr = addr + count - 1;
		s += 2;
		for (i = 0; i < count; i++) {
			data = (*s <= '9') ? (*s - '0') << 4 :
					     (*s - 'A' + 10) << 4;
			s++;
			data += (*s <= '9') ? (*s - '0') :
					      (*s - 'A' + 10);
			s++;
			putmem(addr + i, data);
		}
	}

	fclose(fd);

	count = eaddr - saddr + 1;
	LOG(TAG, "Loader statistics for file %s:\r\n", fn);
	LOG(TAG, "START : %04XH\r\n", saddr);
	LOG(TAG, "END   : %04XH\r\n", eaddr);
	LOG(TAG, "LOADED: %04XH (%d)\r\n\r\n", count & 0xffff, count & 0xffff);

	PC = saddr;

	return(0);
}

/*
 *	Verify checksum of Intel hex records
 */
static int checksum(char *s)
{
	int chk = 0;

	while ((*s != '\r') && (*s != '\n')) {
		chk += (*s <= '9') ?
			(*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		chk += (*s <= '9') ?
			(*s - '0') :
			(*s - 'A' + 10);
		s++;
	}

	if ((chk & 255) == 0)
		return(0);
	else
		return(1);
}
