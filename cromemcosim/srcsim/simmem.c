/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2022 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module implements memory management for a Cromemco Z-1 system
 *
 * History:
 * 22-NOV-2016 stuff moved to here and implemented as inline functions
 * 03-FEB-2017 added ROM initialization
 * 18-MAY-2018 optimization
 * 18-JUL-2018 use logging
 * 01-OCT-2019 optimization
 * 30-AUG-2021 new memory configuration sections
 * 02-SEP-2021 implement banked ROM
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simfun.h"
#include "simmem.h"

#include "cromemco-fdc.h"

#include "log.h"
static const char *TAG = "memory";

memmap_t memconf[MAXMEMSECT][MAXMEMMAP] 	/* memory map */
	= { { { MEM_RW, 0, 0x100, NULL } } };	/* default config to 64K RAM only */
WORD boot_switch[MAXMEMSECT];			/* boot address */

BYTE *memory[MAXSEG];		/* MMU with pointers to the banks */
int selbnk;			/* current selected bank */
bool common;			/* flag for common writes to all banks */
int bankio;			/* data written to banking I/O port */

int num_banks = MAXSEG;

/* page table with memory configuration/state */
int p_tab[MAXPAGES];		/* 256 pages of 256 bytes */
int _p_tab[MAXPAGES];		/* copy of p_tab[] for RAM only */

void init_memory(void)
{
	register int i, j;
	char fn[MAX_LFN];
	char *pfn;

	strcpy(fn, rompath);
	strcat(fn, "/");
	pfn = &fn[strlen(fn)];

	LOG(TAG, "\r\n");

	if (!memconf[M_value][0].size) {
		LOGW(TAG, "The [MEMORY %d] section appears missing or empty, "
		     "setting memory map to default", M_value + 1);
		M_value = 0;
	}

	/* initialize memory page table, no memory available */
	for (i = 0; i < MAXPAGES; i++)
		p_tab[i] = MEM_NONE;

	for (i = 0; i < MAXSEG; i++) {
		if ((memory[i] = (BYTE *) malloc(SEGSIZ)) == NULL) {
			LOGE(TAG, "can't allocate memory for bank %d", i);
			exit(EXIT_FAILURE);
		}
	}

	/* set memory configuration from system.conf only for bank 0 */
	for (i = 0; i < MAXMEMMAP; i++) {
		if (memconf[M_value][i].size) {
			switch (memconf[M_value][i].type) {
			case MEM_RW:
				/* set the pages to RAM */
				for (j = 0; j < memconf[M_value][i].size; j++)
					MEM_RESERVE_RAM(memconf[M_value][i].spage + j);

				/* fill memory content with some initial value */
				for (j = memconf[M_value][i].spage << 8;
				     j < (memconf[M_value][i].spage + memconf[M_value][i].size) << 8;
				     j++) {
					if (m_value >= 0) {
						memory[0][j] = m_value;
					} else {
						memory[0][j] = (BYTE) (rand() % 256);
					}
				}

				LOG(TAG, "RAM %04XH - %04XH\r\n",
				    memconf[M_value][i].spage << 8,
				    ((memconf[M_value][i].spage + memconf[M_value][i].size) << 8) - 1);
				break;

			case MEM_RO:
				/* set the pages to ROM */
				LOG(TAG, "ROM %04XH - %04XH %s\r\n",
				    memconf[M_value][i].spage << 8,
				    ((memconf[M_value][i].spage + memconf[M_value][i].size) << 8) - 1,
				    memconf[M_value][i].rom_file ? memconf[M_value][i].rom_file : "");
				/* for the CROMEMCO Z-1, ROM must be
				   initialized after FDC banked ROM
				   is initialized */
				/* see below */
				break;
			}
		}
	}

	/* set preferred start of boot ROM if specified */
	if (boot_switch[M_value]) {
		LOG(TAG, "Power-on jump address at %04XH\r\n", boot_switch[M_value]);
		PC = boot_switch[M_value];
	} else {
		PC = 0x0000;
	}

	/* copy RAM page table for FDC Banked ROM handling */
	for (i = 0; i < MAXPAGES; i++) {
		_p_tab[i] = p_tab[i];
	}

	LOG(TAG, "MMU has %d additional RAM banks of %d KB\r\n", MAXSEG, 64);
	LOG(TAG, "\r\n");

	cromemco_fdc_reset(); /* activates FDC banked ROM */

	for (i = 0; i < MAXMEMMAP; i++) {
		if (memconf[M_value][i].size) {

			switch (memconf[M_value][i].type) {
			case MEM_RW:
				/* set the pages to RAM */
				/* for the CROMEMCO Z-1, RAM must be
				   initialized before FDC banked ROM
				   is initialized */
				/* see above */
				break;
			case MEM_RO:
				/* set the pages to ROM */
				for (j = 0; j < memconf[M_value][i].size; j++)
					MEM_RESERVE_ROM(memconf[M_value][i].spage + j);

				/* fill the ROM's with 0xff in case no firmware loaded */
				for (j = memconf[M_value][i].spage;
				     j < (memconf[M_value][i].spage + memconf[M_value][i].size);
				     j++) {
					memset(&memory[0][j << 8], 0xff, 256);
				}

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

	LOG(TAG, "\r\n");
}

void reset_fdc_rom_map(void)
{

	register int i;

	LOGD(TAG, "FDC BANK ROM %s", fdc_rom_active ? "ON" : "OFF");

	if (fdc_rom_active) LOG(TAG, "%s\r\n\r\n", BANKED_ROM_MSG);

	for (i = 0xC0; i < 0xE0; i++) {
		if (fdc_rom_active) {
			MEM_ROM_BANK_ON(i);
		} else {
			MEM_RELEASE(i);
		}
	}
}
