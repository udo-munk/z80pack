/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2018 by Udo Munk
 *
 * This module implements the memory for an IMSAI 8080 system
 *
 * History:
 * 19-DEC-2016 stuff moved to here for better memory abstraction
 * 30-DEC-2016 implemented 1 KB page table and setup for that
 * 26-JAN-2017 initialise ROM with 0xff
 * 04-JUL-2018 optimization
 */

#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"

/* 64KB non banked memory */
BYTE memory[65536];

/* page table with memory configuration/state */
int p_tab[64];		/* 64 pages a 1 KB */

void init_memory(void)
{
	register int i;

	/* initialise memory page table, no memory available */
	for (i = 0; i < 64; i++)
		p_tab[i] = MEM_NONE;

	/* then set the first ram_size pages to RAM */
	for (i = 0; i < ram_size; i++)
		p_tab[i] = MEM_RW;

	/* set D800 - DFFF to ROM, this is one of the boot ROM's */
	p_tab[54] = MEM_RO;
	p_tab[55] = MEM_RO;

	/* set F000 - F800 to RAM, this is display memory for the VIO */
	p_tab[60] = MEM_RW;
	p_tab[61] = MEM_RW;

	/* set F800 - FFFF to ROM, this is the VIO firmware ROM */
	p_tab[62] = MEM_RO;
	p_tab[63] = MEM_RO;
}

/*
 * fill the ROM's with 0xff in case no firmware loaded
 */
void init_rom(void)
{
	register int i;

	for (i = 0xd800; i <= 0xdfff; i++)
		memory[i] = 0xff;

	for (i = 0xf800; i <= 0xffff; i++)
		memory[i] = 0xff;
}
