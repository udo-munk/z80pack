/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/simmain.c
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "memory.h"

extern void init_cpu(void);
extern void run_cpu(void);
extern void report_cpu_error(void);

int main(void)
{
	stdio_init_all();	/* initialize Pico stdio */

	printf("Z80pack release %s, %s\n\n", RELEASE, COPYR);

	config();		/* read system configuration */
	init_cpu();		/* initialise CPU */
	init_memory();		/* initialise memory configuration */

	run_cpu();		/* run the CPU with whatever is in memory */

	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	puts("Bye.\n");
	stdio_flush();
}
