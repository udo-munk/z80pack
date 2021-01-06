/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2018 by Udo Munk
 *
 * This module implements the memory for an Altair 8800 system
 *
 * History:
 * 19-DEC-2016 stuff moved to here for better memory abstraction
 * 02-FEB-2017 initialise ROM with 0xff
 * 13-JUN-2017 added Tarbell bootstrap ROM
 * 16-AUG-2017 overworked memrdr()
 * 07-MAY-2018 added memory configuratione needed by apple monitor
 * 11-JUN-2018 fixed bug in Tarbell ROM mapping
 * 21-AUG-2018 improved memory configuration
 */

#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"

/* 64KB non banked memory */
BYTE memory[65536];

/* page table with memory configuration/state */
int p_tab[256];		/* 256 pages a 256 bytes */

/* memory write protected flag */
BYTE mem_wp;

void init_memory(void)
{
	register int i, j;

	/* initialise memory page table, no memory available */
	for (i = 0; i < 256; i++)
		p_tab[i] = MEM_NONE;

	/* set memory configuration from system.conf */
	for (i = 0; i < MAXSEG; i++) {
		if (memconf[i].type != -1) {
			for (j = memconf[i].spage;
			     j < memconf[i].spage + memconf[i].size; j++)
				p_tab[j] = memconf[i].type;
		}
	}
}

/*
 * fill the ROM's with 0xff in case no firmware loaded
 */
void init_rom(void)
{
	register int i;

	for (i = 0; i < 256; i++) {
		if (p_tab[i] == MEM_RO) {
			memset(&memory[i << 8], 0xff, 256);
		}
	}
}
