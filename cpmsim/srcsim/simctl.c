/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2022 by Udo Munk
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"
#include "../../iodevices/unix_terminal.h"
#include "log.h"

int boot(int);

extern struct dskdef disks[];

static const char *TAG = "system";

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

#ifdef WANT_ICE
	extern void ice_cmd_loop(int);

	ice_before_go = set_unix_terminal;
	ice_after_go = reset_unix_terminal;
	atexit(reset_unix_terminal);

	ice_cmd_loop(0);
#else
	extern void cpu_z80(void), cpu_8080(void);

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
#endif
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
