/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module implements the memory for the Z80/8080 CPU
 *
 * History:
 * 23-APR-2024 derived from z80sim
 */

#include <stdlib.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"

/* 64KB non banked memory */
#define MEMSIZE 65536
#include "z80code.h"	/* pull in z80asm generated code */

void init_memory(void)
{
	register int i;

	// fill top page of memory with 0xff, write protected ROM
	for (i = 0xff00; i <= 0xffff; i++)
		code[i] = 0xff;
}
