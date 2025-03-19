/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 * Copyright (C) 2025 by Thomas Eberhardt
 *
 * This module implements the memory for the Z80/8080 CPU.
 *
 * History:
 * 23-APR-2024 derived from z80sim
 * 09-JUN-2024 implemented boot ROM
 * 28-JUN-2024 added second memory bank
 * 29-JUN-2024 implemented banked memory
 * 12-MAR-2025 added more memory banks for RP2350
 */

#include <stdlib.h>

#include "sim.h"
#include "simdefs.h"
#include "simmem.h"

/* 64KB bank 0 + common segment */
BYTE __aligned(4) bnk0[65536];
/* NUMSEG memory banks of size SEGSIZ */
BYTE __aligned(4) bnks[NUMSEG][SEGSIZ];
/* selected bank */
BYTE selbnk, *curbnk;

/* boot ROM code */
#define MEMSIZE 256
#include "bootrom.c"

void init_memory(void)
{
	register int i, j;

	/* copy boot ROM into write protected top memory page */
	for (i = 0; i < MEMSIZE; i++)
		bnk0[0xff00 + i] = code[i];

	/* trash memory like in a real machine after power on */
	for (i = 0; i < 0xff00; i++)
		bnk0[i] = rand() % 256;
	for (j = 0; j < NUMSEG; j++) {
		curbnk = bnks[j];
		for (i = 0; i < SEGSIZ; i++)
			curbnk[i] = rand() % 256;
	}

	selbnk = 0;
}

void reset_memory(void)
{
	selbnk = 0;
}
