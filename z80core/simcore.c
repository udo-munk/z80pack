/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	This module contains functions for CPU/Bus-handling
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simport.h"
#include "simmem.h"
#include "simio.h"
#ifndef EXCLUDE_I8080
#include "sim8080.h"
#endif
#ifndef EXCLUDE_Z80
#include "simz80.h"
#endif
#include "simcore.h"

#ifdef FRONTPANEL
#include <stdbool.h>
#include "frontpanel.h"
#include "simctl.h"
#endif

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "core";

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

#ifndef EXCLUDE_Z80
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
#endif

#ifndef EXCLUDE_I8080
	if (cpu == I8080) {
		F &= ~(Y_FLAG | X_FLAG);
		F |= N_FLAG;
	}
#endif
}

/*
 *	Reset the CPU
 */
void reset_cpu(void)
{
	IFF = int_int = int_protection = 0;
	int_data = -1;

	PC = 0;

#ifndef EXCLUDE_Z80
	I = 0;
	R_ = R = 0;
	int_nmi = int_mode = 0;
#endif
}

#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
/*
 *	Switch the CPU model
 */
void switch_cpu(int new_cpu)
{
	if (cpu != new_cpu) {
		if (new_cpu == I8080) {
			F &= ~(Y_FLAG | X_FLAG);
			F |= N_FLAG;
		}
		cpu = new_cpu;
		cpu_state = ST_MODEL_SWITCH;
	}
}
#endif

/*
 *	Run CPU
 */
void run_cpu(void)
{
	uint64_t t;

	cpu_state = ST_CONTIN_RUN;
	cpu_error = NONE;
	t = get_clock_us();
	for (;;) {
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
		if (cpu_state == ST_MODEL_SWITCH) {
			cpu_state = ST_CONTIN_RUN;
			continue;
		} else
			break;
	}
	cpu_time += get_clock_us() - t;
}

/*
 *	Step CPU
 */
void step_cpu(void)
{
	uint64_t t;

	cpu_state = ST_SINGLE_STEP;
	cpu_error = NONE;
	t = get_clock_us();
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
	cpu_time += get_clock_us() - t;
	cpu_state = ST_STOPPED;
}

/*
 *	Report CPU error
 */
void report_cpu_error(void)
{
	if (cpu_error == NONE)
		return;

	/* always start on a new line */
	LOG(TAG, "\r\n");

	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		LOG(TAG, "INT disabled and HALT Op-Code reached at 0x%04x\r\n",
		    PC - 1);
		break;
	case IOTRAPIN:
		LOGE(TAG, "I/O input Trap at 0x%04x, port 0x%02x",
		     PC, io_port);
		break;
	case IOTRAPOUT:
		LOGE(TAG, "I/O output Trap at 0x%04x, port 0x%02x",
		     PC, io_port);
		break;
	case IOHALT:
		LOG(TAG, "System halted\r\n");
		break;
	case IOERROR:
		LOGE(TAG, "Fatal I/O Error at 0x%04x", PC);
		break;
	case OPTRAP1:
		LOGE(TAG, "Op-code trap at 0x%04x 0x%02x",
		     PC - 1, getmem(PC - 1));
		break;
	case OPTRAP2:
		LOGE(TAG, "Op-code trap at 0x%04x 0x%02x 0x%02x",
		     PC - 2, getmem(PC - 2), getmem(PC - 1));
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
		LOGW(TAG, "Unsupported bus data during INT: 0x%02x", int_data);
		break;
	case POWEROFF:
		LOG(TAG, "System powered off\r\n");
		break;
	default:
		LOGW(TAG, "Unknown error %d", cpu_error);
		break;
	}
}

/*
 * print some execution statistics
 */
void report_cpu_stats(void)
{
	if (cpu_time)
	{
		printf("CPU ran %" PRIu64 " ms ", cpu_time / 1000);
		printf("and executed %" PRIu64 " t-states\n", T);
		printf("Clock frequency %4.2f MHz\n",
		       (float) (T) / (float) cpu_time);
	}
}

/*
 *	This function is called for every IN opcode from the
 *	CPU emulation. It calls the handler for the port,
 *	from which input is wanted.
 */
BYTE io_in(BYTE addrl, BYTE addrh)
{
	uint64_t clk;
#ifdef FRONTPANEL
	bool val;
#else
#ifndef SIMPLEPANEL
	UNUSED(addrh);
#endif
#endif

	clk = get_clock_us();

	io_port = addrl;
	if (port_in[addrl])
		io_data = (*port_in[addrl])();
	else {
		if (i_flag) {
			cpu_error = IOTRAPIN;
			cpu_state = ST_STOPPED;
		}
		io_data = IO_DATA_UNUSED;
	}

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_INP;
#endif

#ifdef FRONTPANEL
	if (F_flag) {
		fp_clock += 3;
		fp_led_address = (addrh << 8) + addrl;
		fp_led_data = io_data;
		fp_sampleData();
		val = wait_step();

		/* when single stepped INP get last set value of port */
		if (val && port_in[addrl])
			io_data = (*port_in[addrl])();
	}
#endif
#ifdef SIMPLEPANEL
	fp_led_address = (addrh << 8) + addrl;
	fp_led_data = io_data;
#endif

#ifdef IOPANEL
	port_flags[addrl].in = true;
#endif

	LOGD(TAG, "input %02x from port %02x", io_data, io_port);

	cpu_time -= get_clock_us() - clk;

	return io_data;
}

/*
 *	This function is called for every OUT opcode from the
 *	CPU emulation. It calls the handler for the port,
 *	to which output is wanted.
 */
void io_out(BYTE addrl, BYTE addrh, BYTE data)
{
	uint64_t clk;
#if !defined(FRONTPANEL) && !defined(SIMPLEPANEL)
	UNUSED(addrh);
#endif

	clk = get_clock_us();

	io_port = addrl;
	io_data = data;

	LOGD(TAG, "output %02x to port %02x", io_data, io_port);

	busy_loop_cnt = 0;

	if (port_out[addrl])
		(*port_out[addrl])(data);
	else {
		if (i_flag) {
			cpu_error = IOTRAPOUT;
			cpu_state = ST_STOPPED;
		}
	}

#ifdef BUS_8080
	cpu_bus = CPU_OUT;
#endif

#ifdef FRONTPANEL
	if (F_flag) {
		fp_clock += 6;
		fp_led_address = (addrh << 8) + addrl;
		fp_led_data = IO_DATA_UNUSED;
		fp_sampleData();
		wait_step();
	}
#endif
#ifdef SIMPLEPANEL
	fp_led_address = (addrh << 8) + addrl;
	fp_led_data = IO_DATA_UNUSED;
#endif

#ifdef IOPANEL
	port_flags[addrl].out = true;
#endif

	cpu_time -= get_clock_us() - clk;
}

/*
 *	Start a bus request cycle
 */
void start_bus_request(BusDMA_t mode, BusDMAFunc_t *bus_master)
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
