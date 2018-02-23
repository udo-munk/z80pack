/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module implements the memory for z80sim
 *
 * History:
 * 22-DEC-2016 stuff moved to here for better memory abstraction
 * 03-FEB-17 added ROM initialisation
 * 15-AUG-17 don't use macros, use inline functions that coerce appropriate
 */

#include "sim.h"

/* 64KB non banked memory */
BYTE memory[65536];		/* 64KB RAM */

void init_memory(void)
{
}

void init_rom(void)
{
}
