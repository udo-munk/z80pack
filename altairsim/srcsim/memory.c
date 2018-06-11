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
 */

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
	register int i;

	/* use memory configuration from system.conf */
#ifndef MONITORMEM
	/* initialise memory page table, no memory available */
	for (i = 0; i < 256; i++)
		p_tab[i] = MEM_NONE;

	/* then set the first ram_size pages to RAM */
	for (i = 0; i < ram_size; i++)
		p_tab[i] = MEM_RW;

	/* then set rom_size pages in upper memory to ROM */
	for (i = 256 - rom_size; i < 256; i++)
		p_tab[i] = MEM_RO;

	/* memory configuration needed by TDL Apple */
#else
	/* 0000 - EFFF RAM */
	for (i = 0; i < 240; i++)
		p_tab[i] = MEM_RW;

	/* F000 - F7FF ROM */
	for (i = 240; i < 248; i++)
		p_tab[i] = MEM_RO;

	/* F800 - FFFF RAM */
	for (i = 248; i < 256; i++)
		p_tab[i] = MEM_RW;
#endif
}

/*
 * fill the ROM's with 0xff in case no firmware loaded
 */
void init_rom(void)
{
	register int i;

	/* use memory configuration from system.conf */
#ifndef MONITORMEM
	for (i = (256 - rom_size) << 8; i <= 0xffff; i++)
		memory[i] = 0xff;

	/* TDL Apple ROM */
#else
	for (i = 0xf000; i < 0xf800; i++)
		memory[i] = 0xff;
#endif
}
