/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Simulation of an Intel Intellec MDS-800 monitor module.
 * It includes the monitor ROM, TTY, CRT, PTR, PTP, line printer,
 * and PROM programmer interface.
 *
 * History:
 */

#include <stdint.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "mds-monitor.h"

			/* TTY and CRT status bits */
#define TRDY	0x01	/* transmit ready */
#define RBR	0x02	/* receive buffer ready */
#define TBE	0x04	/* transmit empty */
#define RPAR	0x08	/* receive parity error */
#define ROV	0x10	/* receive overrun error */
#define RFR	0x20	/* receive framing error */
#define DSR	0x80	/* data set ready */

			/* TTY and CRT initialization controls */
#define R48_1	0x02	/* 4800 baud @ jumper 1 */
#define R96_1	0x01	/* 9600 baud @ jumper 1 */
#define R24_1	0x03	/* 2400 baud @ jumper 1 */
#define R6_2	0x02	/* 600 baud @ jumper 2 */
#define R12_2	0x01	/* 1200 baud @ jumper 2 */
#define R3_2	0x03	/* 300 baud @ jumper 2 */
#define R110	0x02	/* 110 baud @ jumper 3 */
#define CL7	0x08	/* character length = 7 */
#define CL8	0x0c	/* character length = 8 */
#define CL6	0x04	/* character length = 6 */
#define CL5	0x00	/* character length = 5 */
#define ST1	0x40	/* 1 stop bit */
#define ST15	0x80	/* 1.5 stop bits */
#define ST2	0xc0	/* 2 stop bits */
#define PENB	0x10	/* parity enable */
#define PEVEN	0x20	/* even parity */
#define TXEN	0x01	/* transmit enable */
#define DTR	0x02	/* data terminal ready */
#define RXEN	0x04	/* receive enable */
#define CLERR	0x10	/* clear error */
#define USRST	0x40	/* usart reset */
#define RTS	0x20	/* request to send */

			/* PTR, PTP, and TTY reader controls */
#define PTPREV	0x10	/* punch reverse direction */
#define PTPADV	0x20	/* punch advance */
#define PTRREV	0x04	/* read reverse direction */
#define PTRADV	0x08	/* reader advance */
#define TTYADV	0x02	/* TTY advance */

			/* LPT, PTR, and PTP status bits */
#define LPTRY	0x01	/* LPT ready */
#define PTRDY	0x01	/* PTR ready with data */
#define PTPRY	0x04	/* PTP ready for data */

			/* Programmer I/O constants */
#define PCOMP	0x02	/* Programming complete */
#define PGRDY	0x01	/* PROM ready */
#define PSOCK	0x20	/* 16 pin socket selected */
#define PNIB	0x10	/* Select upper nibble */

BYTE mds_mon_int;	/* Interrupts enabled & signals */

/*
 *	PROM programmer interface data input
 *	(Not implemented)
 */
BYTE mds_prom_data_in(void)
{
	return (0x00);
}

/*
 *	PROM programmer interface status input
 *	(Not implemented)
 */
BYTE mds_prom_status_in(void)
{
	return (0x00);
}

/*
 *	PROM programmer interface data output
 *	(Not implemented)
 */
void mds_prom_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PROM programmer interface MSB address and control output
 *	(Not implemented)
 */
void mds_prom_high_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PROM programmer interface LSB address output
 *	(Not implemented)
 */
void mds_prom_low_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	TTY port data input
 */
BYTE mds_tty_data_in(void)
{
	return (0x00);
}

/*
 *	TTY port status input
 */
BYTE mds_tty_status_in(void)
{
	return (0x00);
}

/*
 *	TTY port data output
 */
void mds_tty_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	TTY port control output
 */
void mds_tty_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	CRT port data input
 */
BYTE mds_crt_data_in(void)
{
	return (0x00);
}

/*
 *	CRT port status input
 */
BYTE mds_crt_status_in(void)
{
	return (0x00);
}

/*
 *	CRT port data output
 */
void mds_crt_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	CRT port control output
 */
void mds_crt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PTR port data input
 *	(Currently not implemented)
 */
BYTE mds_ptr_data_in(void)
{
	return (0x00);
}

/*
 *	PTR/PTP port status input
 *	(Currently not implemented)
 */
BYTE mds_pt_status_in(void)
{
	return (0x00);
}

/*
 *	PTP port data output
 *	(Currently not implemented)
 */
void mds_ptp_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PTR/PTP port control output
 *	(Currently not implemented)
 */
void mds_pt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	LPT port status input
 */
BYTE mds_lpt_status_in(void)
{
	return (0x00);
}

/*
 *	LPT port data output
 */
void mds_lpt_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	LPT port control output
 */
void mds_lpt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Interrupt control output
 */
void mds_int_ctl_out(BYTE data)
{
	mds_mon_int = (data & 0x80) | (mds_mon_int & (~data & 0x7f));
}

/*
 *	Interrupt status input
 */
BYTE mds_int_status_in(void)
{
	return (mds_mon_int & 0x7f);
}
