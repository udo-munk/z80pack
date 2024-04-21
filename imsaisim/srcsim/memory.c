/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2022 Udo Munk
 * Copyright (C) 2018-2021 David McNaughton
 *
 * This module implements the memory for an IMSAI 8080 system
 *
 * History:
 * 19-DEC-2016 stuff moved to here for better memory abstraction
 * 30-DEC-2016 implemented 1 KB page table and setup for that
 * 26-JAN-2017 initialise ROM with 0xff
 * 04-JUL-2018 optimization
 * 07-JUL-2018 implemended banked ROM/RAM
 * 12-JUL-2018 use logging
 * 18-JUL-2019 bug fix so that fp shows mapped memory contents
 * 18-OCT-2019 add MMU and memory banks
 * 20-JUL-2021 log banked memory
 * 29-AUG-2021 new memory configuration sections
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "memory.h"
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"

static const char *TAG = "memory";

extern int load_file(char *, WORD, int);

struct memmap memconf[MAXMEMSECT][MAXMEMMAP] 	/* memory map */
	= { { { MEM_RW, 0, 0x100, NULL } } };	/* default config to 64K RAM only */
WORD boot_switch[MAXMEMSECT];			/* boot address */

/* 64KB memory system bank 0 */
BYTE memory[64 << 10];
/* 2KB banked ROM & RAM for MPU-B */
BYTE mpubrom[2 << 10];
BYTE mpubram[2 << 10];

#define MAXPAGES 256
/* Memory access read and write vector tables system bank 0 */
BYTE *rdrvec[MAXPAGES];
BYTE *wrtvec[MAXPAGES];
int cyclecount;
static BYTE groupsel;

/* page table with memory configuration/state system bank 0 */
int p_tab[MAXPAGES];		/* 256 pages of 256 bytes */
int _p_tab[MAXPAGES];		/* copy of p_tab[] for RAM only */

/* additional memory banks */
static BYTE bnk1[SEGSIZ];
static BYTE bnk2[SEGSIZ];
static BYTE bnk3[SEGSIZ];
static BYTE bnk4[SEGSIZ];
static BYTE bnk5[SEGSIZ];
static BYTE bnk6[SEGSIZ];
static BYTE bnk7[SEGSIZ];

BYTE *banks[MAXSEG] = {
	NULL,		/* system bank 0, not addressable here */
	&bnk1[0],
	&bnk2[0],
	&bnk3[0],
	&bnk4[0],
	&bnk5[0],
	&bnk6[0],
	&bnk7[0]
};

int num_banks = sizeof(banks) / sizeof(BYTE *) - 1;
int selbnk;		/* current selected bank */

void groupswap(void)
{
	LOGD(TAG, "MPU-B Banked ROM/RAM group select %02X", groupsel);

	if (groupsel & _GROUP0) {
		rdrvec[0] = &memory[0x0000];
		rdrvec[1] = &memory[0x0100];
		rdrvec[2] = &memory[0x0200];
		rdrvec[3] = &memory[0x0300];
		rdrvec[4] = &memory[0x0400];
		rdrvec[5] = &memory[0x0500];
		rdrvec[6] = &memory[0x0600];
		rdrvec[7] = &memory[0x0700];
	} else {
		rdrvec[0] = &mpubrom[0x0000];
		rdrvec[1] = &mpubrom[0x0100];
		rdrvec[2] = &mpubrom[0x0200];
		rdrvec[3] = &mpubrom[0x0300];
		rdrvec[4] = &mpubrom[0x0400];
		rdrvec[5] = &mpubrom[0x0500];
		rdrvec[6] = &mpubrom[0x0600];
		rdrvec[7] = &mpubrom[0x0700];
	}

	if (groupsel & _GROUP1) {
		rdrvec[0xD0] = &memory[0xD000];
		wrtvec[0xD0] = &memory[0xD000];

		rdrvec[0xD8] = &memory[0xD800];
		rdrvec[0xD9] = &memory[0xD900];
		rdrvec[0xDA] = &memory[0xDA00];
		rdrvec[0xDB] = &memory[0xDB00];
		rdrvec[0xDC] = &memory[0xDC00];
		rdrvec[0xDD] = &memory[0xDD00];
		rdrvec[0xDE] = &memory[0xDE00];
		rdrvec[0xDF] = &memory[0xDF00];

		MEM_RELEASE(0xD0);

		MEM_RELEASE(0xD8);
		MEM_RELEASE(0xD9);
		MEM_RELEASE(0xDA);
		MEM_RELEASE(0xDB);
		MEM_RELEASE(0xDC);
		MEM_RELEASE(0xDD);
		MEM_RELEASE(0xDE);
		MEM_RELEASE(0xDF);
	} else {
		rdrvec[0xD0] = &mpubram[0x0000];
		wrtvec[0xD0] = &mpubram[0x0000];

		rdrvec[0xD8] = &mpubrom[0x0000];
		rdrvec[0xD9] = &mpubrom[0x0100];
		rdrvec[0xDA] = &mpubrom[0x0200];
		rdrvec[0xDB] = &mpubrom[0x0300];
		rdrvec[0xDC] = &mpubrom[0x0400];
		rdrvec[0xDD] = &mpubrom[0x0500];
		rdrvec[0xDE] = &mpubrom[0x0600];
		rdrvec[0xDF] = &mpubrom[0x0700];

		MEM_RESERVE_RAM(0xD0);

		MEM_ROM_BANK_ON(0xD8);
		MEM_ROM_BANK_ON(0xD9);
		MEM_ROM_BANK_ON(0xDA);
		MEM_ROM_BANK_ON(0xDB);
		MEM_ROM_BANK_ON(0xDC);
		MEM_ROM_BANK_ON(0xDD);
		MEM_ROM_BANK_ON(0xDE);
		MEM_ROM_BANK_ON(0xDF);
	}
}

void init_memory(void)
{
	register int i, j;
	char fn[MAX_LFN];
	char *pfn;

	strcpy(fn, rompath);
	strcat(fn, "/");
	pfn = &fn[strlen(fn)];

	LOG(TAG,"\r\n");

	if (!memconf[M_flag][0].size) {
		LOGW(TAG, "The [MEMORY %d] section appears missing or empty, setting memory map to default", M_flag + 1);
		M_flag = 0;
	}

	/* initialise memory page table, no memory available */
	for (i = 0; i < MAXPAGES; i++) {
		p_tab[i] = MEM_NONE;
		wrtvec[i] = &memory[i << 8];
		rdrvec[i] = &memory[i << 8];		
	}

	for (i=0; i < MAXMEMMAP; i++) {
		if (memconf[M_flag][i].size) {
			switch (memconf[M_flag][i].type) {
				case MEM_RW:
					/* set the pages to RAM */
					for (j = 0; j < memconf[M_flag][i].size; j++)
						MEM_RESERVE_RAM(memconf[M_flag][i].spage + j);

					/* fill memory content of bank 0 with some initial value */
					for (j = memconf[M_flag][i].spage << 8; j < (memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8; j++) {
						if (m_flag >= 0) {
							_MEMMAPPED(j) = m_flag;
						} else {
							_MEMMAPPED(j) = (BYTE) (rand() % 256);
						}
					}

					LOG(TAG, "RAM %04XH - %04XH\r\n",
					memconf[M_flag][i].spage << 8, 
					(memconf[M_flag][i].spage << 8) + (memconf[M_flag][i].size << 8) - 1);
					break;

				case MEM_RO:
					/* set the pages to ROM */
					LOG(TAG, "ROM %04XH - %04XH %s\r\n",
					memconf[M_flag][i].spage << 8, 
					((memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8) - 1,
					memconf[M_flag][i].rom_file?memconf[M_flag][i].rom_file:"");
					/* for the IMSAI, ROM must be
					   initialised after MPU-B banked ROM
					   is intialised */
					/* see below */
					break;
			}
		}
	}

	if (boot_switch[M_flag]) {
		LOG(TAG, "Power-on jump address at %04XH\r\n", boot_switch[M_flag]);
	}

	/* copy RAM page table for MPU-B banked ROM/RAM handling */
	for (i = 0; i < MAXPAGES; i++) {
		_p_tab[i] = p_tab[i];
	}

#ifdef HAS_BANKED_ROM
	if (R_flag) {
		groupsel = _GROUPINIT;
		LOG(TAG, "%s\r\n", BANKED_ROM_MSG);
	} else {
		groupsel = _GROUP0 | _GROUP1;
	}
	groupswap();
	cyclecount = 0;
#endif

	LOG(TAG, "MMU has %d additional RAM banks of %d KB\r\n", num_banks, SEGSIZ >> 10);
	LOG(TAG, "\r\n");

	for (i = 0; i < MAXMEMMAP; i++) {
		if (memconf[M_flag][i].size) {
			switch (memconf[M_flag][i].type) {
				case MEM_RW:
					/* set the pages to RAM */
					/* for the IMSAI, RAM must be
					   initialised before MPU-B banked ROM
					   is intialised */
					/* see above */
					break;

				case MEM_RO:
					/* set the pages to ROM */
					for (j = 0; j < memconf[M_flag][i].size; j++)
						MEM_RESERVE_ROM(memconf[M_flag][i].spage + j);

					/* fill the ROM's with 0xff in case no firmware loaded */
					for (j = memconf[M_flag][i].spage << 8; j < (memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8; j++) {
						_MEMMAPPED(j) = 0xff;
					}

					/* load firmware into ROM if specified */
					if (memconf[M_flag][i].rom_file) {
						strcpy(pfn, memconf[M_flag][i].rom_file);
						load_file(fn, memconf[M_flag][i].spage << 8, memconf[M_flag][i].size << 8);
					}
					break;
			}
		}
	}

	/* set preferred start of boot ROM if specified */
	if (boot_switch[M_flag]) {
		PC = boot_switch[M_flag];
	} else {
		PC = 0x0000;
	}
}

void reset_memory(void)
{
#ifdef HAS_BANKED_ROM
	if (R_flag) {
		groupsel = _GROUPINIT;
	} else {
		groupsel = _GROUP0 | _GROUP1;
	}
	groupswap();
	cyclecount = 0;
#endif
	selbnk = 0;
}

void ctrl_port_out(BYTE data)
{
#ifdef HAS_BANKED_ROM
	if (R_flag) {
		groupsel = data;
		cyclecount = 3;
	}
#else
	data = data;
#endif
}

BYTE ctrl_port_in(void)
{
#ifdef HAS_BANKED_ROM
	if (R_flag) {
		groupsel = _GROUP0 | _GROUP1;
		cyclecount = 3;
	}
#endif
	return ((BYTE) 0xff);
}
