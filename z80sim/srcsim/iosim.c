/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 *
 * This module of the simulator contains a simple terminal I/O
 * simulation as an example.
 */

/*
 *	Sample I/O-handler
 *
 *	Port 0 input:	reads status of stdin
 *	Port 1 input:	reads the next byte from stdin
 *	Port 1 output:	writes the byte to stdout
 *
 *	All the other ports are connected to an I/O-trap handler,
 *	I/O to this ports stops the simulation with an I/O error.
 */

#include <stdio.h>
#include <sys/poll.h>
#include "sim.h"
#include "simglb.h"

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static BYTE io_trap_in(void);
static void io_trap_out(BYTE);
static BYTE p000_in(void), p001_in(void);
static void p001_out(BYTE);

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
static BYTE (*port_in[256])(void) = {
	p000_in,		/* port 0 */
	p001_in			/* port 1 */
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
static void (*port_out[256])(BYTE) = {
	io_trap_out,		/* port 0 */
	p001_out		/* port 1 */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 *
 *	In this sample I/O simulation we initialize all
 *	unused port with an error trap handler, so that
 *	simulation stops at I/O on the unused ports.
 *
 *	See the I/O simulation of of the other systems
 *	for more complex examples.
 */
void init_io(void)
{
	register int i;

	for (i = 2; i <= 255; i++) {
		port_in[i] = io_trap_in;
		port_out[i] = io_trap_out;
	}
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 *
 *	Nothing to do here, see the I/O simulation
 *	of CP/M for a more complex example.
 */
void exit_io(void)
{
}

/*
 *	This is the main handler for all IN op-codes,
 *	called by the simulator. It calls the input
 *	function for port addrl.
 */
BYTE io_in(BYTE addrl, BYTE addrh)
{
	UNUSED(addrh);

	io_port = addrl;
	io_data = (*port_in[addrl])();
	return (io_data);
}

/*
 *	This is the main handler for all OUT op-codes,
 *	called by the simulator. It calls the output
 *	function for port addrl.
 */
void io_out(BYTE addrl, BYTE addrh, BYTE data)
{
	UNUSED(addrh);

	io_port = addrl;
	io_data = data;
	(*port_out[addrl])(data);
}

/*
 *	I/O input trap function
 *	This function should be added into all unused
 *	entries of the input port array. It can stop the
 *	emulation with an I/O error.
 */
static BYTE io_trap_in(void)
{
	if (i_flag) {
		cpu_error = IOTRAPIN;
		cpu_state = STOPPED;
	}
	return ((BYTE) 0xff);
}

/*
 *	I/O trap function
 *	This function should be added into all unused
 *	entries of the output port array. It can stop the
 *	emulation with an I/O error.
 */
static void io_trap_out(BYTE data)
{
	UNUSED(data);

	if (i_flag) {
		cpu_error = IOTRAPOUT;
		cpu_state = STOPPED;
	}
}

/*
 *	I/O function port 0 read:
 *	Read status from stdin:
 *	bit 0 = 0, character available for input from stdin
 *	bit 7 = 0, transmitter ready to write character to stdout
 *		   always true
 */
static BYTE p000_in(void)
{
	struct pollfd p[1];
	register BYTE tty_stat = 0x01;

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		tty_stat &= ~1;
	if (p[0].revents & POLLNVAL) {
		cpu_error = IOERROR;
		cpu_state = STOPPED;
	}
	return (tty_stat);
}

/*
 *	I/O function port 1 read:
 *	Read next byte from stdin.
 */
static BYTE p001_in(void)
{
	extern BYTE getkey(void);

	return ((BYTE) getkey());
}

/*
 *	I/O function port 1 write:
 *	Write byte to stdout and flush the output.
 */
static void p001_out(BYTE data)
{
	putchar((int) data);
	fflush(stdout);
}
