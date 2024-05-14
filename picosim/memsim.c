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

/* do nothing for now */
void init_memory(void)
{
}
