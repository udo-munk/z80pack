/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2022 Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module implements the memory for an Intel Intellec MDS-800 system.
 * We are fully loaded with 64K RAM.
 *
 * History:
 * 03-JUN-2024 first version
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "simmem.h"
#include "simcfg.h"
#include "log.h"

static const char *TAG = "memory";

/* 64KB non banked memory */
BYTE memory[65536];		/* 64KB RAM */

BYTE boot_rom[BOOT_SIZE];	/* bootstrap ROM */

char *boot_rom_file;		/* bootstrap ROM file path */
char *mon_rom_file;		/* monitor ROM file path */
int mon_enabled;		/* monitor ROM enabled flag */

void init_memory(void)
{
	register int i;
	char fn[MAX_LFN];
	char *pfn;

	strcpy(fn, rompath);
	strcat(fn, "/");
	pfn = &fn[strlen(fn)];

	if (boot_rom_file == NULL) {
		LOGE(TAG, "no bootstrap ROM file specified in config file");
		exit(EXIT_FAILURE);
	}
	if (mon_enabled && mon_rom_file == NULL) {
		LOGE(TAG, "no monitor ROM file specified in config file");
		exit(EXIT_FAILURE);
	}

	strcpy(pfn, boot_rom_file);
	if (load_file(fn, 0, BOOT_SIZE)) {
		LOGE(TAG, "couldn't load bootstrap ROM");
		exit(EXIT_FAILURE);
	}
	memcpy(boot_rom, memory, BOOT_SIZE);

	if (mon_enabled) {
		mon_enabled = 0;
		strcpy(pfn, mon_rom_file);
		if (load_file(fn, 65536 - MON_SIZE, MON_SIZE)) {
			LOGE(TAG, "couldn't load monitor ROM");
			exit(EXIT_FAILURE);
		}
		mon_enabled = 1;
	}

	/* fill memory content with some initial value */
	if (m_flag >= 0) {
		for (i = 0; i < 65536; i++)
			putmem(i, m_flag);
	} else {
		for (i = 0; i < 65536; i++)
			putmem(i, (BYTE) (rand() % 256));
	}

	PC = 0x0000;
}
