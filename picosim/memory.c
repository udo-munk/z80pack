/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module implements the memory for picosim
 *
 * History:
 * 23-APR-24 derived from z80sim
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
	for (i = 0; i < 65536; i++)
		putmem(i, (BYTE) (rand() % 256));
}
