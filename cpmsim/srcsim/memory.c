/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2017 by Udo Munk
 *
 * This module implements banked memory management for cpmsim
 *
 *      MMU:
 *      ===
 *
 *      +--------+
 * 16KB | common |
 *      +--------+
 *      +--------+  +--------+  ..........  +--------+
 *      |        |  |        |              |        |
 * 48KB |        |  |        |  ..........  |        |
 *      | bank 0 |  | bank 1 |              | bank n |
 *      +--------+  +--------+  ..........  +--------+
 *
 * This is an example for 48KB segments as it was implemented originally.
 * The segment size now can be configured via port 22.
 * If the segment size isn't configured the default is 48 KB as it was
 * before, to maintain compatibility.
 *
 * History:
 * 21-DEC-16 moved banked memory implementation to here
 * 03-FEB-17 added ROM initialisation
 */

#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"

BYTE *memory[MAXSEG];		/* MMU with pointers to the banks */
int selbnk;			/* current selected bank */
int maxbnk;			/* number of allocated banks */
int segsize = SEGSIZ;		/* segment size of banks, default 48KB */
int wp_common;			/* write protect/unprotect common segment */

void init_memory(void)
{
	/* allocate the first 64KB bank, so that we have some memory */
	if ((memory[0] = malloc(65536)) == NULL) {
		printf("can't allocate memory for bank 0\r\n");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
		return;
	}
	maxbnk = 1;
}

void init_rom(void)
{
}
