/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module implements the memory for an Intel MDS-800 system
 *
 * History:
 */

#include <stdint.h>
#include <stdlib.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"

/* 64KB non banked memory */
BYTE memory[65536];		/* 64KB RAM */

void init_memory(void)
{
	register int i;

	/* fill memory content with some initial value */
	if (m_flag >= 0) {
		for (i = 0; i < 65536; i++)
			putmem(i, m_flag);
	} else {
		for (i = 0; i < 65536; i++)
			putmem(i, (BYTE) (rand() % 256));
	}
}
