/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module implements the memory for an Altair 8800 system
 *
 * History:
 * 19-DEC-2016 stuff moved to here for better memory abstraction
 * 02-FEB-2017 initialise ROM with 0xff
 * 13-JUN-2017 added Tarbell bootstrap ROM
 * 16-AUG-2017 overworked memrdr()
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

	/* initialise memory page table, no memory available */
	for (i = 0; i < 256; i++)
		p_tab[i] = MEM_NONE;

	/* then set the first ram_size pages to RAM */
	for (i = 0; i < ram_size; i++)
		p_tab[i] = MEM_RW;

	/* then set rom_size pages in upper memory to ROM */
	for (i = 256 - rom_size; i < 256; i++)
		p_tab[i] = MEM_RO;
}

/*
 * fill the ROM's with 0xff in case no firmware loaded
 */
void init_rom(void)
{
	register int i;

	for (i = (256 - rom_size) << 8; i <= 0xffff; i++)
		memory[i] = 0xff;
}
