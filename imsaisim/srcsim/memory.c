/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module implements the memory for an IMSAI 8080 system
 *
 * History:
 * 19-DEC-2016 stuff moved to here for better memory abstraction
 * 30-DEC-2016 implemented 1 KB page table and setup for that
 * 26-JAN-2017 initialise ROM with 0xff
 */

#include "stdio.h"
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"

/* 64KB non banked memory */
BYTE memory[64*1024];
/* 2KB banked ROM & RAM for MPU-B */
BYTE mpubrom[2*1024];
BYTE mpubram[2*1024];

/* Memory access read and write vector tables */
BYTE *rdrvec[64];
BYTE *wrtvec[64];
int cyclecount;
bool mempage = false;
int groupsel;

/* page table with memory configuration/state */
int p_tab[64];		/* 64 pages a 1 KB */

void groupswap() {
	mempage = false;
	printf("\r\nMPU-B Banked ROM/RAM group select %02x\r\n", groupsel);

	if(groupsel & 0x40) {
		rdrvec[0] = &memory[0x0000];
		rdrvec[1] = &memory[0x0400];
	} else {
		rdrvec[0] = &mpubrom[0x0000];
		rdrvec[1] = &mpubrom[0x0400];
	}

	if(groupsel & 0x80) {
		rdrvec[52] = &memory[52*1024];
		rdrvec[53] = &memory[53*1024];
		wrtvec[52] = &memory[52*1024];
		wrtvec[53] = &memory[53*1024];

		rdrvec[54] = &memory[54*1024];
		rdrvec[55] = &memory[55*1024];
	} else {
		rdrvec[52] = &mpubram[0x0000];
		rdrvec[53] = &mpubram[0x0400];
		wrtvec[52] = &mpubram[0x0000];
		wrtvec[53] = &mpubram[0x0400];

		rdrvec[54] = &mpubrom[0x0000];
		rdrvec[55] = &mpubrom[0x0400];
	}
}

void init_memory(void)
{
	register int i;

	/* initialise memory page table, no memory available */
	for (i = 0; i < 64; i++) {
		p_tab[i] = MEM_NONE;
		wrtvec[i] = &memory[i<<10];
		rdrvec[i] = &memory[i<<10];		
	}

	/* then set the first ram_size pages to RAM */
	for (i = 0; i < ram_size; i++)
		p_tab[i] = MEM_RW;

	groupsel=0x00;
	groupswap();
	cyclecount=0;

	/* set D800 - DFFF to ROM, this is one of the boot ROM's */
	// p_tab[54] = MEM_RO;
	// p_tab[55] = MEM_RO;
	p_tab[52] = MEM_RW;
	p_tab[53] = MEM_RW;
	p_tab[54] = MEM_RW;
	p_tab[55] = MEM_RW;

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
		*wrtaddr(i) = 0xff;

	for (i = 0xf800; i <= 0xffff; i++)
		*wrtaddr(i) = 0xff;
}

void ctrl_port_out(BYTE data) {

	printf("\r\nMPU-B Banked ROM/RAM control OUT @ %04x\r\n", PC-1);

	groupsel = data;
	mempage= true;
	cyclecount=3;

}

BYTE ctrl_port_in(void) {
	// If a port read resets the MPU-B banks then the survey command will stall!
	// groupsel = 0x00;
	// mempage= true;
	// cyclecount=3;

	printf("\r\nMPU-B Banked ROM/RAM control IN @ %04x\r\n", PC-1);

	return groupsel; // Not to spec. should return 0xff;
}
