/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/sim0.c
 */

#include <stdio.h>
#include <time.h>
#include "pico/stdlib.h"
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "memory.h"

extern void init_cpu(void);
extern void run_cpu(void);
extern void report_cpu_error(void);

/*
 *	Compute difference between two timeval in microseconds
 *
 *	Note: yes there are timersub() and friends, but not
 *	defined in POSIX.1 and implemented wrong on some
 *	systems. Some systems define tv_usec as unsigned int,
 *	here we assume that a long is longer than unsigned.
 *	If that is not the case cast to (long long).
 */
int time_diff(struct timeval *t1, struct timeval *t2)
{
	long sec, usec;

	sec = (long) t2->tv_sec - (long) t1->tv_sec;
	usec = (long) t2->tv_usec - (long) t1->tv_usec;
	/* normalize result */
	if (usec < 0L) {
		sec--;
		usec += 1000000L;
	}
	if (sec != 0L)
		return (-1); /* result is to large */
	else
		return ((int) usec);
}

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
