/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module implements memory management for a Cromemco Z-1 system
 *
 * History:
 * 22-NOV-16 stuff moved to here and implemented as inline functions
 * 03-FEB-17 added ROM initialisation
 * 18-MAY-18 optimization
 * 18-JUL-18 use logging
 * 01-OCT-19 optimization
 * 30-AUG-21 new memory configuration sections
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#include "log.h"

static const char *TAG = "memory";

extern int load_file(char *, BYTE, BYTE);

struct memmap memconf[MAXMEMSECT][MAXMEMMAP] 	/* memory map */
	= { { { MEM_RW, 0, 0x100, NULL } } };	/* default config to 64K RAM only */
WORD _boot_switch[MAXMEMSECT];			/* boot address */


BYTE *memory[MAXSEG];		/* MMU with pointers to the banks */
int selbnk;			/* current selected bank */
int common;			/* flag for common writes to all banks */
int bankio;			/* data written to banking I/O port */

#define MAXPAGES 256
/* page table with memory configuration/state */
int p_tab[MAXPAGES];		/* 256 pages a 256 bytes */

void init_memory(void)
{
	register int i, j;
	char fn[MAX_LFN];
	char *pfn = fn;

	strcpy(fn, rompath);
	strcat(fn, "/");
	pfn = &fn[strlen(fn)];

	LOG(TAG, "\r\n");

	if (!memconf[M_flag][0].size) {
		LOGW(TAG, "The [MEMORY %d] section appears missing or empty, setting memory map to default", M_flag + 1);
		M_flag = 0;
	}

	/* initialise memory page table, no memory available */
	for (i = 0; i < MAXPAGES; i++)
		p_tab[i] = MEM_NONE;

	for (i = 0; i < MAXSEG; i++) {
		if ((memory[i] = malloc(65536)) == NULL) {
			LOGE(TAG, "can't allocate memory for bank %d", i);
			exit(1);
		}
	}

	/* set memory configuration from system.conf only for bank 0 */
	for (i = 0; i < MAXMEMMAP; i++) {
		if (memconf[M_flag][i].size) {

			for (j = 0; j < memconf[M_flag][i].size; j++)
				p_tab[memconf[M_flag][i].spage + j] = memconf[M_flag][i].type;

			switch (memconf[M_flag][i].type) {
				case MEM_RW:
					/* fill memory content with some initial value */
					for ( j = memconf[M_flag][i].spage << 8; j < (memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8; j++) {
						if (m_flag >= 0) {
							memory[0][j] = m_flag;
						} else {
							memory[0][j] = (BYTE) (rand() % 256);
						}
					}

					LOG(TAG, "RAM %04XH - %04XH\r\n",
					memconf[M_flag][i].spage << 8, 
					(memconf[M_flag][i].spage << 8) + (memconf[M_flag][i].size << 8) - 1);
					break;

				case MEM_RO:
					/* fill the ROM's with 0xff in case no firmware loaded */
					for (int j = memconf[M_flag][i].spage; j < (memconf[M_flag][i].spage + memconf[M_flag][i].size); j++) {
						memset(&memory[0][j << 8], 0xff, 256);
					}

					LOG(TAG, "ROM %04XH - %04XH %s\r\n\r\n",
					memconf[M_flag][i].spage << 8, 
					((memconf[M_flag][i].spage + memconf[M_flag][i].size) << 8) - 1,
					memconf[M_flag][i].rom_file?memconf[M_flag][i].rom_file:"");

					/* load firmware into ROM if specified */
					if (memconf[M_flag][i].rom_file) {
						strcpy(pfn, memconf[M_flag][i].rom_file);
						load_file(fn, memconf[M_flag][i].spage, memconf[M_flag][i].size);
					}
					break;
			}
		}
	}

	/* set preferred start of boot ROM if specified */
	if (_boot_switch[M_flag]) {
		LOG(TAG, "Power-on jump address at %04XH\r\n", _boot_switch[M_flag]);
		PC = _boot_switch[M_flag];
	} else {
		PC = 0x0000;
	}

	LOG(TAG, "\r\n");
}
