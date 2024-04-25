/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/simmain.c
 */

/* Raspberry SDK includes */
#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

/* Project includes */
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
	while (!tud_cdc_connected())
		sleep_ms(100);	/* wait until USB connected */

	printf("Z80pack release %s, %s\n\n", RELEASE, COPYR);

	config();		/* read system configuration */
	init_cpu();		/* initialise CPU */
	init_memory();		/* initialise memory configuration */

	run_cpu();		/* run the CPU with whatever is in memory */

	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	putchar('\n');
	stdio_flush();
}
