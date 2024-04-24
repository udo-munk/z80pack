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
#include "memory.h"

extern void config(void);
extern void init_memory(void);
extern void init_cpu(void);
extern void cpu_z80(void);

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
	init_memory();	/* initialise memory configuration */

	cpu_state = CONTIN_RUN;
	cpu_z80();		/* run the Z80 with whatever is in memory */

	printf("\n");
	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		printf("INT disabled and HALT Op-Code reached at %04x",
		       PC - 1);
		break;
	case IOTRAPIN:
		printf("I/O input Trap at %04x, port %02x", PC, io_port);
		break;
	case IOTRAPOUT:
		printf("I/O output Trap at %04x, port %02x", PC, io_port);
		break;
	case IOHALT:
		printf("System halted");
		break;
	case IOERROR:
		printf("Fatal I/O Error at %04x", PC);
		break;
	case OPTRAP1:
		printf("Op-code trap at %04x %02x", PC - 1,
		       getmem(PC - 1));
		break;
	case OPTRAP2:
		printf("Op-code trap at %04x %02x %02x",
		       PC - 2, getmem(PC - 2),
		       getmem(PC - 1));
		break;
	case OPTRAP4:
		printf("Op-code trap at %04x %02x %02x %02x %02x",
		       PC - 4, getmem(PC - 4), getmem(PC - 3),
		       getmem(PC - 2), getmem(PC - 1));
		break;
	case INTERROR:
		printf("Unsupported bus data during INT: %02x",
		       int_data);
		break;
	default:
		printf("Unknown error %d", cpu_error);
		break;
	}
	printf("\nBye.\n\n");
	stdio_flush();
}
