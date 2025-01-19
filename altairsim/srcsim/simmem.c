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

#include <stdlib.h>
#include <string.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simctl.h"
#include "simfun.h"
#include "simmem.h"

#include "tarbell_fdc.h"

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "memory";

memmap_t memconf[MAXMEMSECT][MAXMEMMAP] 	/* memory map */
	= { { { MEM_RW, 0, 0x100, NULL } } };	/* default config to 64K RAM only */
WORD _boot_switch[MAXMEMSECT];			/* boot address for switch */

/* 64KB non banked memory */
BYTE memory[65536];

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

	if (!memconf[M_value][0].size) {
		LOGW(TAG, "The [MEMORY %d] section appears missing or empty, "
		     "setting memory map to default", M_value + 1);
		M_value = 0;
	}

	/* initialize memory page table, no memory available */
	for (i = 0; i < MAXPAGES; i++)
		p_tab[i] = MEM_NONE;

	/* set memory configuration from system.conf */
	for (i = 0; i < MAXMEMMAP; i++) {
		if (memconf[M_value][i].size) {

			for (j = 0; j < memconf[M_value][i].size; j++)
				p_tab[memconf[M_value][i].spage + j] = memconf[M_value][i].type;

			switch (memconf[M_value][i].type) {
			case MEM_RW:
				/* fill memory content with some initial value */
				for (j = memconf[M_value][i].spage << 8;
				     j < (memconf[M_value][i].spage + memconf[M_value][i].size) << 8;
				     j++) {
					if (m_value >= 0) {
						memory[j] = m_value;
					} else {
						memory[j] = (BYTE) (rand() % 256);
					}
				}

				LOG(TAG, "RAM %04XH - %04XH\r\n",
				    memconf[M_value][i].spage << 8,
				    ((memconf[M_value][i].spage + memconf[M_value][i].size) << 8) - 1);
				break;

			case MEM_RO:
				/* fill the ROM's with 0xff in case no firmware loaded */
				for (j = memconf[M_value][i].spage;
				     j < (memconf[M_value][i].spage + memconf[M_value][i].size);
				     j++) {
					memset(&memory[j << 8], 0xff, 256);
				}

				LOG(TAG, "ROM %04XH - %04XH %s\r\n\r\n",
				    memconf[M_value][i].spage << 8,
				    ((memconf[M_value][i].spage + memconf[M_value][i].size) << 8) - 1,
				    memconf[M_value][i].rom_file ? memconf[M_value][i].rom_file : "");

				/* load firmware into ROM if specified */
				if (memconf[M_value][i].rom_file) {
					strcpy(pfn, memconf[M_value][i].rom_file);
					load_file(fn, memconf[M_value][i].spage << 8,
						  memconf[M_value][i].size << 8);
				}
				break;
			}
		}
	}

	/* set preferred start of boot ROM if specified */
	if (_boot_switch[M_value]) {
		LOG(TAG, "Boot switch address at %04XH\r\n", _boot_switch[M_value]);
		boot_switch = _boot_switch[M_value];
		PC = _boot_switch[M_value];
	} else {
		PC = 0x0000;
	}

	tarbell_rom_enabled = R_flag;
	LOG(TAG, "Tarbell bootstrap ROM %s\r\n",
	    (tarbell_rom_enabled) ? "enabled" : "disabled");

	LOG(TAG, "\r\n");
}
