/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module of the simulator contains the I/O simulation
 * for an Intel MDS-800 system
 *
 * History:
 */

#include <stdint.h>
#include <stddef.h>
#include "sim.h"
#include "simglb.h"

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
BYTE (*port_in[256])(void) = {
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
void (*port_out[256])(BYTE) = {
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	extern BYTE io_trap_in(void);
	extern void io_trap_out(BYTE);

	register int i;

	for (i = 0; i <= 255; i++) {
		if (port_in[i] == NULL)
			port_in[i] = io_trap_in;
		if (port_out[i] == NULL)
			port_out[i] = io_trap_out;
	}
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
}
