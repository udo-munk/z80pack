/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2022 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module implements the memory for an Altair 8800 system
 *
 * History:
 * 19-DEC-2016 stuff moved to here for better memory abstraction
 * 02-FEB-2017 initialize ROM with 0xff
 * 13-JUN-2017 added Tarbell bootstrap ROM
 * 16-AUG-2017 overworked memrdr()
 * 07-MAY-2018 added memory configuration needed by apple monitor
 * 11-JUN-2018 fixed bug in Tarbell ROM mapping
 * 21-AUG-2018 improved memory configuration
 * 29-AUG-2021 new memory configuration sections
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "frontpanel.h"
#include "memory.h"
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"

static const char *TAG = "memory";

extern int load_file(char *, WORD, int);

struct memmap memconf[MAXMEMSECT][MAXMEMMAP] 	/* memory map */
	= { { { MEM_RW, 0, 0x100, NULL } } };	/* default config to 64K RAM only */
WORD _boot_switch[MAXMEMSECT];			/* boot address for switch */

extern int  boot_switch;			/* boot address for switch */

/* 64KB non banked memory */
BYTE memory[65536];

#define MAXPAGES 256
/* page table with memory configuration/state */
int p_tab[MAXPAGES];		/* 256 pages a 256 bytes */

/* memory write protected flag */
BYTE mem_wp;

void init_memory(void)
{
	register int i, j;
	char fn[MAX_LFN];
	char *pfn;

	strcpy(fn, rompath);
	strcat(fn, "/");
	pfn = &fn[strlen(fn)];

	if (!memconf[M_flag][0].size) {
		LOGW(TAG, "The [MEMORY %d] section appears missing or empty, "
		     "setting memory map to default", M_flag + 1);
		M_flag = 0;
	}

	/* initialize memory page table, no memory available */
	for (i = 0; i < MAXPAGES; i++)
		p_tab[i] = MEM_NONE;

	/* set memory configuration from system.conf */
	for (i = 0; i < MAXMEMMAP; i++) {
		if (memconf[M_flag][i].size) {

			for (j = 0; j < memconf[M_flag][i].size; j++)
				p_tab[memconf[M_flag][i].spage + j] = memconf[M_flag][i].type;

			switch (memconf[M_flag][i].type) {
			case MEM_RW:
				/* fill memory content with some initial value */
				for (j = memconf[M_flag][i].spage << 8;
				     j < (memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8;
				     j++) {
					if (m_flag >= 0) {
						memory[j] = m_flag;
					} else {
						memory[j] = (BYTE) (rand() % 256);
					}
				}

				LOG(TAG, "RAM %04XH - %04XH\r\n",
				    memconf[M_flag][i].spage << 8,
				    ((memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8) - 1);
				break;

			case MEM_RO:
				/* fill the ROM's with 0xff in case no firmware loaded */
				for (j = memconf[M_flag][i].spage;
				     j < (memconf[M_flag][i].spage + memconf[M_flag][i].size);
				     j++) {
					memset(&memory[j << 8], 0xff, 256);
				}

				LOG(TAG, "ROM %04XH - %04XH %s\r\n\r\n",
				    memconf[M_flag][i].spage << 8,
				    ((memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8) - 1,
				    memconf[M_flag][i].rom_file ? memconf[M_flag][i].rom_file : "");

				/* load firmware into ROM if specified */
				if (memconf[M_flag][i].rom_file) {
					strcpy(pfn, memconf[M_flag][i].rom_file);
					load_file(fn, memconf[M_flag][i].spage << 8,
						  memconf[M_flag][i].size << 8);
				}
				break;
			}
		}
	}

	/* set preferred start of boot ROM if specified */
	if (_boot_switch[M_flag]) {
		LOG(TAG, "Boot switch address at %04XH\r\n", _boot_switch[M_flag]);
		boot_switch = _boot_switch[M_flag];
		PC = _boot_switch[M_flag];
	} else {
		PC = 0x0000;
	}

	tarbell_rom_enabled = R_flag;
	LOG(TAG, "Tarbell bootstrap ROM %s\r\n",
	    (tarbell_rom_enabled) ? "enabled" : "disabled");

	LOG(TAG, "\r\n");
}
