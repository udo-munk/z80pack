/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * I/O simulation for picosim
 */

/* Raspberry SDK includes */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

/* Project includes */
#include "sim.h"
#include "simglb.h"

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static void p001_out(BYTE);
static BYTE p000_in(void), p001_in(void);

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
	0,			/* port 0 */
	p001_out		/* port 1 */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	register int i;

	for (i = 2; i <= 255; i++) {
		port_in[i] = 0;
		port_out[i] = 0;
	}
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
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

	if (*port_in[addrl] != 0) /* if port used */
		return ((*port_in[addrl])());
	else
		return (0xff);	/* all other return 0xff */
}

/*
 *	This is the main handler for all OUT op-codes,
 *	called by the simulator. It calls the output
 *	function for port addrl.
 */
void io_out(BYTE addrl, BYTE addrh, BYTE data)
{
	UNUSED(addrh);

	if (*port_out[addrl] != 0) /* if port used */
		(*port_out[addrl])(data);
}

/*
 *	I/O function port 0 read:
 *	read status of the Pico UART and return:
 *	bit 0 = 0, character available for input from tty
 *	bit 7 = 0, transmitter ready to write character to tty
 */
static BYTE p000_in(void)
{
	register BYTE stat = 0b10000001; /* initially not ready */

	if (uart_is_writable(uart0))	/* check if output to UART is possible */
		stat &= 0b01111111;	/* if so flip status bit */
	if (uart_is_readable(uart0))	/* check if there is input from UART */
		stat &= 0b11111110;	/* if so flip status bit */

	return (stat);
}

/*	I/O function port 1 read:
 *	Read byte from Pico UART.
 */
static BYTE p001_in(void)
{
	while (!uart_is_readable(uart0)) /* block until data available */
		;			 /* compatible with z80sim */
	return ((BYTE)getchar());	 /* read data */
}

/*
 *	I/O function port 1 write:
 *	Write byte to Pico UART.
 */
static void p001_out(BYTE data)
{
	putchar_raw((int) data);
}
