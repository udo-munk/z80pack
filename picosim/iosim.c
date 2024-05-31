/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * I/O simulation for picosim
 *
 * History:
 * 28-APR-2024 all I/O implemented for a first release
 * 09-MAY-2024 improved so that it can run some MITS Altair software
 * 11-MAY-2024 allow us to configure the port 255 value
 * 27-MAY-2024 moved io_in & io_out to simcore
 */

/* Raspberry SDK includes */
#include <stdio.h>
#if LIB_PICO_STDIO_USB
#include <tusb.h>
#endif
#include "pico/stdlib.h"
#include "hardware/uart.h"

/* Project includes */
#include "sim.h"
#include "simglb.h"

/* Pico W also needs this */
#if PICO == 1
#include "pico/cyw43_arch.h"
#endif

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static void p000_out(BYTE), p001_out(BYTE), p255_out(BYTE);
static BYTE p000_in(void), p001_in(void), p255_in(void);
extern void fdc_out(BYTE);
extern BYTE fdc_in(void);

static BYTE sio_last;	/* last character received */
       BYTE fp_value;	/* port 255 value, can be set from ICE or config() */

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
BYTE (*port_in[256])(void) = {
	[  0] = p000_in,	/* SIO status */
	[  1] = p001_in,	/* SIO data */
	[  4] = fdc_in,		/* FDC command */
	[255] = p255_in		/* for frontpanel */
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
void (*port_out[256])(BYTE) = {
	[  0] = p000_out,	/* internal LED */
	[  1] = p001_out,	/* SIO data */
	[  4] = fdc_out,	/* FDC status */
	[255] = p255_out	/* for frontpanel */
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

	/* initialize unused ports to trap handlers */
	for (i = 0; i <= 255; i++) {
		if (port_in[i] == NULL)
			port_in[i] = io_trap_in;
		if (port_out[i] == NULL)
			port_out[i] = io_trap_out;
	}
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

#if LIB_PICO_STDIO_UART
	if (uart_is_writable(uart0))	/* check if output to UART is possible */
		stat &= 0b01111111;	/* if so flip status bit */
	if (uart_is_readable(uart0))	/* check if there is input from UART */
		stat &= 0b11111110;	/* if so flip status bit */
#endif
#if LIB_PICO_STDIO_USB
	if (tud_cdc_available())	/* check if there is input from UART */
		stat &= 0b11111110;	/* if so flip status bit */
	stat &= 0b01111111;		/* output always possible */
#endif

	return stat;
}

/*
 *	I/O function port 1 read:
 *	Read byte from Pico UART.
 */
static BYTE p001_in(void)
{
#if LIB_PICO_STDIO_UART
	if (!uart_is_readable(uart0))	/* someone reading without checking */
#endif
#if LIB_PICO_STDIO_USB
	if (!tud_cdc_available())
#endif
		return sio_last;	/* guess who does such things */
	else {
		sio_last = getchar();
		return sio_last;
	}
}

/*
 *	I/O function port 255 read:
 *	used by frontpanel machines
 */
static BYTE p255_in(void)
{
	return fp_value;
}

/*
 * 	I/O function port 0 write:
 *	Switch builtin LED on/off.
 */
static void p000_out(BYTE data)
{
	if (!data) {
		/* 0 switches LED off */
#if PICO == 1
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
#else
		gpio_put(LED, 0);
#endif
	} else {
		/* everything else on */
#if PICO == 1
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#else
		gpio_put(LED, 1);
#endif
	}
}	
 
/*
 *	I/O function port 1 write:
 *	Write byte to Pico UART.
 */
static void p001_out(BYTE data)
{
	putchar_raw((int) data & 0x7f); /* strip parity, some software won't */
}

/*
 *	This allows to set the frontpanel port with ICE p command
 */
static void p255_out(BYTE data)
{
	fp_value = data;
}
