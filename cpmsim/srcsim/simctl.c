/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2019 by Udo Munk
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
 * 24-JAN-14 Release 1.18 some improvements here and there
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 20-JUL-14 Release 1.24 bugfixes and improvements
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
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"
#include "../../iodevices/unix_terminal.h"
#include "log.h"

int boot(int);

extern void cpu_z80(void), cpu_8080(void);
extern struct dskdef disks[];

static const char *TAG = "system";

extern struct termios old_term, new_term;

/*
 *	This function initialises the terminal, loads boot code
 *	and then the Z80 CPU emulation is started.
 */
void mon(void)
{
	/* load boot code into memory */
	if (boot(0))
		exit(1);

	/* empty buffer for teletype */
	fflush(stdout);

	/* initialise terminal */
	set_unix_terminal();
	atexit(reset_unix_terminal);

	/* start CPU emulation */
	cpu_state = CONTIN_RUN;
	cpu_error = NONE;
	switch(cpu) {
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}

	/* reset terminal */
	reset_unix_terminal();

	/* check for CPU emulation errors and report */
	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		LOG(TAG, "INT disabled and HALT Op-Code reached at %04x\r\n",
		    PC - 1);
		break;
	case IOTRAPIN:
		LOGE(TAG, "I/O input Trap at %04x, port %02x",
		     PC, io_port);
		break;
	case IOTRAPOUT:
		LOGE(TAG, "I/O output Trap at %04x, port %02x",
		     PC, io_port);
		break;
	case IOHALT:
		LOG(TAG, "System halted, bye.\r\n");
		break;
	case IOERROR:
		LOGE(TAG, "Fatal I/O Error at %04x", PC);
		break;
	case OPTRAP1:
		LOGE(TAG, "Op-code trap at %04x %02x", PC - 1,
		     getmem(PC - 1));
		break;
	case OPTRAP2:
		LOGE(TAG, "Op-code trap at %04x %02x %02x",
		     PC - 2, getmem(PC - 2), getmem(PC - 1));
		break;
	case OPTRAP4:
		LOGE(TAG, "Op-code trap at %04x %02x %02x %02x %02x",
		     PC - 4, getmem(PC - 4), getmem(PC - 3),
		     getmem(PC - 2), getmem(PC - 1));
		break;
	case USERINT:
		LOG(TAG, "User Interrupt at %04x\r\n", PC);
		break;
	case INTERROR:
		LOGW(TAG, "Unsupported bus data during INT: %02x", int_data);
		break;
	case POWEROFF:
		break;
	default:
		LOGW(TAG, "Unknown error %d", cpu_error);
		break;
	}
}

/*
 *	boot from a saved core image, a boot file or from
 *	first sector of disk drive A:
 */
int boot(int level)
{
	register int i;
	int fd;
	struct stat sbuf;
	static BYTE buf[128];
	static char fn[MAX_LFN];

	LOG(TAG, "\r\nBooting...\r\n\r\n");

	/* on first boot we can run from core or file */
	if (level == 0) {
		if (l_flag)
			return(0);
		if (x_flag)
			return(0);
	}

	/* else load boot code from disk */

	/* if option -d is used disks are there */
	if (diskdir != NULL) {
		strcpy(fn, diskd);
	} else {
		/* if not first try ./disks */
		if ((stat("./disks", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
			strcpy(fn, "./disks");
		/* nope, then DISKSDIR as set in Makefile */
		} else {
			strcpy(fn, DISKSDIR);
		}
	}

	strcat(fn, "/");
	strcat(fn, disks[0].fn);

	if ((fd = open(fn, O_RDONLY)) == -1) {
		LOGE(TAG, "can't open file %s", fn);
		close(fd);
		return(1);
	}
	if (read(fd, buf, 128) != 128) {
		LOGE(TAG, "can't read file %s", fn);
		close(fd);
		return(1);
	}
	close(fd);

	for (i = 0; i < 128; i++)
		putmem(i, buf[i]);

	return(0);
}
