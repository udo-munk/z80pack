/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module implements memory management for mosteksim
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) Created from memory.c in the z80sim
 *	       directory. Emulate memory of the Mostek AID-80F and SYS-80FT
 *	       computers by treating 0xe000-0xefff as ROM.
 */

#include <stdlib.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"

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
