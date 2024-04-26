/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module implements the memory for z80sim
 *
 * History:
 * 22-DEC-2016 stuff moved to here for better memory abstraction
 * 03-FEB-17 added ROM initialization
 * 15-AUG-17 don't use macros, use inline functions that coerce appropriate
 */

#include <stdlib.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"

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
