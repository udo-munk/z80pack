/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module implements memory management for a Cromemco Z-1 system
 *
 * History:
 * 22-NOV-16 stuff moved to here and implemented as inline functions
 * 03-FEB-17 added ROM initialisation
 */

#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "simglb.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"

BYTE *memory[MAXSEG];		/* MMU with pointers to the banks */
int selbnk;			/* current selected bank */
int common;			/* flag for common writes to all banks */
int bankio;			/* data written to banking I/O port */

void init_memory(void)
{
	register int i;

	for (i = 0; i < MAXSEG; i++) {
		if ((memory[i] = malloc(65536)) == NULL) {
			printf("can't allocate memory for bank %d\n", i);
			exit(1);
		}
	}
}

void init_rom(void)
{
}
