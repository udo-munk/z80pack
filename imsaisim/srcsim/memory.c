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
BYTE memory[64<<10];
/* 2KB banked ROM & RAM for MPU-B */
BYTE mpubrom[2<<10];
BYTE mpubram[2<<10];

/* Memory access read and write vector tables */
BYTE *rdrvec[64];
BYTE *wrtvec[64];
int cyclecount;
static BYTE groupsel;

/* page table with memory configuration/state */
int p_tab[64];		/* 64 pages a 1 KB */

void groupswap() {
#ifdef DEBUG
	printf("\r\nMPU-B Banked ROM/RAM group select %02X\r\n", groupsel);
#endif

	if(groupsel & _GROUP0) {
		rdrvec[0] = &memory[0x0000];
		rdrvec[1] = &memory[0x0400];
	} else {
		rdrvec[0] = &mpubrom[0x0000];
		rdrvec[1] = &mpubrom[0x0400];
	}

	if(groupsel & _GROUP1) {
		rdrvec[52] = &memory[52<<10];
		// rdrvec[53] = &memory[53<<10];
		wrtvec[52] = &memory[52<<10];
		// wrtvec[53] = &memory[53<<10];

		rdrvec[54] = &memory[54<<10];
		rdrvec[55] = &memory[55<<10];

		p_tab[52] = (ram_size > 52)?MEM_RW:MEM_NONE; // Only RW if ram_size allows
		// p_tab[53] = (ram_size > 53)?MEM_RW:MEM_NONE; // Only RW if ram_size allows
		p_tab[54] = (ram_size > 54)?MEM_RW:MEM_NONE; // Only RW if ram_size allows
		p_tab[55] = (ram_size > 55)?MEM_RW:MEM_NONE; // Only RW if ram_size allows
	} else {
		rdrvec[52] = &mpubram[0x0000];
		// rdrvec[53] = &mpubram[0x0400];
		wrtvec[52] = &mpubram[0x0000];
		// wrtvec[53] = &mpubram[0x0400];

		rdrvec[54] = &mpubrom[0x0000];
		rdrvec[55] = &mpubrom[0x0400];

		p_tab[52] = MEM_RW; // Must be RW while RAM is switched in
		// p_tab[53] = MEM_RW; // Must be RW while RAM is switched in
		p_tab[54] = (ram_size > 54)?MEM_RW:MEM_RO; // Only RW if ram_size allows
		p_tab[55] = (ram_size > 55)?MEM_RW:MEM_RO; // Only RW if ram_size allows
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

#ifdef HAS_BANKED_ROM
	if(r_flag) {
		groupsel = _GROUPINIT;
		printf("MPU-B Banked ROM/RAM enabled: group select %02X\n", groupsel);
	} else {
		groupsel = _GROUP0 | _GROUP1;
	}
	groupswap();
	cyclecount = 0;
#else
	p_tab[54] = MEM_RO; 
	p_tab[55] = MEM_RO;
#endif

	/* set F000 - F800 to RAM, this is display memory for the VIO */
	p_tab[60] = MEM_RW;
	p_tab[61] = MEM_RW;

	/* set F800 - FFFF to ROM, this is the VIO firmware ROM */
	p_tab[62] = MEM_RO;
	p_tab[63] = MEM_RO;
}

void reset_memory(void) {
	
#ifdef HAS_BANKED_ROM
	if(r_flag) {
		groupsel = _GROUPINIT;
	} else {
		groupsel = _GROUP0 | _GROUP1;
	}
	groupswap();
	cyclecount = 0;
#endif
}

/*
 * fill the ROM's with 0xff in case no firmware loaded
 */
void init_rom(void)
{
	register unsigned int i;

	if(r_flag) {
		for (i = 0xd800; i <= 0xdfff; i++)
			_MEMMAPPED(i) = 0xff;
		for (i = 0xf800; i <= 0xffff; i++)
			_MEMMAPPED(i) = 0xff;
	}
}

void ctrl_port_out(BYTE data) {

#ifdef HAS_BANKED_ROM
	if(r_flag) {
	groupsel = data;
	cyclecount = 3;
	}
#else
	data = data;
#endif
}

BYTE ctrl_port_in(void) {

#ifdef HAS_BANKED_ROM
	if(r_flag) {
		groupsel = _GROUP0 | _GROUP1;
		cyclecount = 3;
	}
#endif
	return (BYTE) 0xff;
}
