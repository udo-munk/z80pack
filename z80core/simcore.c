/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	This module contains functions for CPU/Bus-handling
 */

#include <stdio.h>
#ifndef PICO
#include <stdlib.h>
#include <sys/time.h>
#else
#include "pico/stdlib.h"
#include "pico/time.h"
#endif
#include "sim.h"
#include "simglb.h"
#include "memory.h"

#ifdef CORE_LOG
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"

static const char *TAG = "core";
#endif

extern void cpu_z80(void), cpu_8080(void);

/*
 *	Initialize the CPU
 */
void init_cpu(void)
{
	/* same for i8080 and Z80 */
	PC = 0;
	SP = rand() % 65536;
	A = rand() % 256;
	B = rand() % 256;
	C = rand() % 256;
	D = rand() % 256;
	E = rand() % 256;
	H = rand() % 256;
	L = rand() % 256;
	F = rand() % 256;

	switch (cpu) {
#ifndef EXCLUDE_Z80
	case Z80:
		I = 0;
		A_ = rand() % 256;
		B_ = rand() % 256;
		C_ = rand() % 256;
		D_ = rand() % 256;
		E_ = rand() % 256;
		H_ = rand() % 256;
		L_ = rand() % 256;
		F_ = rand() % 256;
		IX = rand() % 65536;
		IY = rand() % 65536;
		break;
#endif
#ifndef EXCLUDE_I8080
	case I8080:
		F &= ~(N2_FLAG | N1_FLAG);
		F |= N_FLAG;
		break;
#endif
	default:
		break;
	}
}

/*
 *	Reset the CPU
 */
void reset_cpu(void)
{
	IFF = int_int = int_protection = 0;
	int_data = -1;

	PC = 0;

	switch (cpu) {
#ifndef EXCLUDE_Z80
	case Z80:
		I = 0;
		R_ = R = 0;
		int_nmi = int_mode = 0;
		break;
#endif
#ifndef EXCLUDE_I8080
	case I8080:
		break;
#endif
	default:
		break;
	}
}

/*
 *	Run CPU
 */
void run_cpu(void)
{
	cpu_state = CONTIN_RUN;
	cpu_error = NONE;
	switch (cpu) {
#ifndef EXCLUDE_Z80
	case Z80:
		cpu_z80();
		break;
#endif
#ifndef EXCLUDE_I8080
	case I8080:
		cpu_8080();
		break;
#endif
	default:
		break;
	}
}

/*
 *	Step CPU
 */
void step_cpu(void)
{
	cpu_state = SINGLE_STEP;
	cpu_error = NONE;
	switch (cpu) {
#ifndef EXCLUDE_Z80
	case Z80:
		cpu_z80();
		break;
#endif
#ifndef EXCLUDE_I8080
	case I8080:
		cpu_8080();
		break;
#endif
	default:
		break;
	}
	cpu_state = STOPPED;
}

/*
 *	Report CPU error
 */
void report_cpu_error(void)
{
	/* always start on a new line */
#ifdef CORE_LOG
	LOG(TAG, "\r\n");
#else
	printf("\n");
#endif

	switch (cpu_error) {
	case NONE:
		break;
#ifdef CORE_LOG
	case OPHALT:
		LOG(TAG, "INT disabled and HALT Op-Code reached at 0x%04x\r\n",
		    PC - 1);
		break;
	case IOTRAPIN:
		LOGE(TAG, "I/O input Trap at 0x%04x, port 0x%02x", PC, io_port);
		break;
	case IOTRAPOUT:
		LOGE(TAG, "I/O output Trap at 0x%04x, port 0x%02x", PC, io_port);
		break;
	case IOHALT:
		LOG(TAG, "System halted\r\n");
		break;
	case IOERROR:
		LOGE(TAG, "Fatal I/O Error at 0x%04x", PC);
		break;
	case OPTRAP1:
		LOGE(TAG, "Op-code trap at 0x%04x 0x%02x", PC - 1,
		     getmem(PC - 1));
		break;
	case OPTRAP2:
		LOGE(TAG, "Op-code trap at 0x%04x 0x%02x 0x%02x",
		     PC - 2, getmem(PC - 2),
		     getmem(PC - 1));
		break;
	case OPTRAP4:
		LOGE(TAG, "Op-code trap at 0x%04x 0x%02x 0x%02x 0x%02x 0x%02x",
		     PC - 4, getmem(PC - 4), getmem(PC - 3),
		     getmem(PC - 2), getmem(PC - 1));
		break;
	case USERINT:
		LOG(TAG, "User Interrupt at 0x%04x\r\n", PC);
		break;
	case INTERROR:
		LOGW(TAG, "Unsupported bus data during INT: 0x%02x",
		     int_data);
		break;
	case POWEROFF:
		LOG(TAG, "System powered off\r\n");
		break;
	default:
		LOGW(TAG, "Unknown error %d", cpu_error);
		break;
#else
	case OPHALT:
		printf("INT disabled and HALT Op-Code reached at 0x%04x\n",
		       PC - 1);
		break;
	case IOTRAPIN:
		printf("I/O input Trap at 0x%04x, port 0x%02x\n", PC, io_port);
		break;
	case IOTRAPOUT:
		printf("I/O output Trap at 0x%04x, port 0x%02x\n", PC, io_port);
		break;
	case IOHALT:
		printf("System halted\n");
		break;
	case IOERROR:
		printf("Fatal I/O Error at 0x%04x\n", PC);
		break;
	case OPTRAP1:
		printf("Op-code trap at 0x%04x 0x%02x\n", PC - 1,
		       getmem(PC - 1));
		break;
	case OPTRAP2:
		printf("Op-code trap at 0x%04x 0x%02x 0x%02x\n",
		       PC - 2, getmem(PC - 2),
		       getmem(PC - 1));
		break;
	case OPTRAP4:
		printf("Op-code trap at 0x%04x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		       PC - 4, getmem(PC - 4), getmem(PC - 3),
		       getmem(PC - 2), getmem(PC - 1));
		break;
	case INTERROR:
		printf("Unsupported bus data during INT: 0x%02x\n",
		       int_data);
		break;
	case POWEROFF:
		printf("System powered off\n");
		break;
	default:
		printf("Unknown error %d\n", cpu_error);
		break;
#endif
	}
}

/*
 *	Start a bus request cycle
 */
void start_bus_request(BusDMA_t mode, Tstates_t (*bus_master)(BYTE bus_ack))
{

	bus_mode = mode;
	dma_bus_master = bus_master;
	bus_request = 1;
}

/*
 *	End a bus request cycle
 */
void end_bus_request(void)
{

	bus_mode = BUS_DMA_NONE;
	dma_bus_master = NULL;
	bus_request = 0;
}

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
