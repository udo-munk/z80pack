/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/sim0.c
 */

#include "pico/stdlib.h"

extern void config(void);
extern void init_memory(void);
extern void init_cpu(void);

int main(int argc, char *argv[])
{
	stdio_init_all();	/* initialize Pico stdio */

	config();		/* read system configuration */
	init_cpu();		/* initialise CPU */
	init_memory();	/* initialise memory configuration */

	// now we might be able to start the Z80 CPU here
}
