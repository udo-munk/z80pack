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
#define MDS_MON_TRDY	0x01	/* transmit ready */
#define MDS_MON_RBR	0x02	/* receive buffer ready */
#define MDS_MON_TBE	0x04	/* transmit empty */
#define MDS_MON_RPAR	0x08	/* receive parity error */
#define MDS_MON_ROV	0x10	/* receive overrun error */
#define MDS_MON_RFR	0x20	/* receive framing error */
#define MDS_MON_DSR	0x80	/* data set ready */

				/* TTY and CRT initialization controls */
#define MDS_MON_R48_1	0x02	/* 4800 baud @ jumper 1 */
#define MDS_MON_R96_1	0x01	/* 9600 baud @ jumper 1 */
#define MDS_MON_R24_1	0x03	/* 2400 baud @ jumper 1 */
#define MDS_MON_R6_2	0x02	/* 600 baud @ jumper 2 */
#define MDS_MON_R12_2	0x01	/* 1200 baud @ jumper 2 */
#define MDS_MON_R3_2	0x03	/* 300 baud @ jumper 2 */
#define MDS_MON_R110	0x02	/* 110 baud @ jumper 3 */
#define MDS_MON_CL7	0x08	/* character length = 7 */
#define MDS_MON_CL8	0x0c	/* character length = 8 */
#define MDS_MON_CL6	0x04	/* character length = 6 */
#define MDS_MON_CL5	0x00	/* character length = 5 */
#define MDS_MON_ST1	0x40	/* 1 stop bit */
#define MDS_MON_ST15	0x80	/* 1.5 stop bits */
#define MDS_MON_ST2	0xc0	/* 2 stop bits */
#define MDS_MON_PENB	0x10	/* parity enable */
#define MDS_MON_PEVEN	0x20	/* even parity */
#define MDS_MON_TXEN	0x01	/* transmit enable */
#define MDS_MON_DTR	0x02	/* data terminal ready */
#define MDS_MON_RXEN	0x04	/* receive enable */
#define MDS_MON_CLERR	0x10	/* clear error */
#define MDS_MON_USRST	0x40	/* usart reset */
#define MDS_MON_RTS	0x20	/* request to send */

				/* PTR, PTP, and TTY reader controls */
#define MDS_MON_PTPREV	0x10	/* punch reverse direction */
#define MDS_MON_PTPADV	0x20	/* punch advance */
#define MDS_MON_PTRREV	0x04	/* read reverse direction */
#define MDS_MON_PTRADV	0x08	/* reader advance */
#define MDS_MON_TTYADV	0x02	/* TTY advance */

				/* LPT, PTR, and PTP status bits */
#define MDS_MON_LPTRY	0x01	/* LPT ready */
#define MDS_MON_PTRDY	0x01	/* PTR ready with data */
#define MDS_MON_PTPRY	0x04	/* PTP ready for data */

				/* Programmer I/O constants */
#define MDS_MON_PCOMP	0x02	/* Programming complete */
#define MDS_MON_PGRDY	0x01	/* PROM ready */
#define MDS_MON_PSOCK	0x20	/* 16 pin socket selected */
#define MDS_MON_PNIB	0x10	/* Select upper nibble */

BYTE mds_mon_int;	/* interrupts enabled & signals */

/*
 *	PROM programmer interface data input
 */
BYTE mds_prom_data_in(void)
{
	return (0x00);
}

/*
 *	PROM programmer interface status input
 */
BYTE mds_prom_status_in(void)
{
	return (0x00);
}

/*
 *	PROM programmer interface data output
 */
void mds_prom_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PROM programmer interface MSB address and control output
 */
void mds_prom_high_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PROM programmer interface LSB address output
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
 */
BYTE mds_ptr_data_in(void)
{
	return (0x00);
}

/*
 *	PTR/PTP port status input
 */
BYTE mds_pt_status_in(void)
{
	return (0x00);
}

/*
 *	PTP port data output
 */
void mds_ptp_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PTR/PTP port control output
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
