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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "mds-monitor.h"
#include "unix_terminal.h"
#define LOG_LOCAL_LEVEL LOG_DEBUG
#include "log.h"

			/* TTY and CRT status bits */
#define TRDY	0x01	/* transmit ready */
#define RBR	0x02	/* receive buffer ready */
#define TBE	0x04	/* transmit empty */
#define RPAR	0x08	/* receive parity error */
#define ROV	0x10	/* receive overrun error */
#define RFR	0x20	/* receive framing error */
#define DSR	0x80	/* data set ready */

			/* TTY and CRT command controls */
#define TXEN	0x01	/* transmit enable */
#define DTR	0x02	/* data terminal ready */
#define RXEN	0x04	/* receive enable */
#define CLERR	0x10	/* clear error */
#define RTS	0x20	/* request to send */
#define USRST	0x40	/* usart reset */

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

			/* Interrupt status and control bits */
#define ITTYO	0x01	/* Output TTY */
#define ITTYI	0x02	/* Input TTY */
#define IPTP	0x04	/* PTP */
#define IPTR	0x08	/* PTR */
#define ICRTO	0x10	/* Output CRT */
#define ICRTI	0x20	/* Input CRT */
#define ILPT	0x40	/* LPT */
#define MENB	0x80	/* Enable monitor interrupts */

#define MON_IRQ	3	/* Monitor module interrupt */

static const char *TAG = "MONITOR";

static BYTE mon_int;	/* Interrupts enabled & signals */

static int tty_init;	/* TTY initialized flag */
static BYTE tty_cmd;	/* TTY command byte */
static BYTE tty_stat;	/* TTY status byte */

static int crt_init;	/* CRT initialized flag */
static BYTE crt_cmd;	/* TTY command byte */
static BYTE crt_stat;	/* TTY status byte */

static BYTE pt_stat;	/* PTR/PTP status byte */

static int lpt_stat;	/* LPT status byte */

/*
 *	PROM programmer interface data input
 *	(Not implemented)
 */
BYTE mon_prom_data_in(void)
{
	return (0x00);
}

/*
 *	PROM programmer interface status input
 *	(Not implemented)
 */
BYTE mon_prom_status_in(void)
{
	return (0x00);
}

/*
 *	PROM programmer interface data output
 *	(Not implemented)
 */
void mon_prom_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PROM programmer interface MSB address and control output
 *	(Not implemented)
 */
void mon_prom_high_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PROM programmer interface LSB address output
 *	(Not implemented)
 */
void mon_prom_low_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	TTY port reset
 */
void mon_tty_reset(void)
{
	tty_init = 0;
	tty_cmd = 0;
	tty_stat = 0;
}

/*
 *	TTY port data input
 */
BYTE mon_tty_data_in(void)
{
	return (0x00);
}

/*
 *	TTY port status input
 */
BYTE mon_tty_status_in(void)
{
	return (0x00);
}

/*
 *	TTY port data output
 */
void mon_tty_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	TTY port control output
 */
void mon_tty_ctl_out(BYTE data)
{
	if (!tty_init) {
		/* Ignore baud rate, character length, parity and stop bits */
		tty_init = 1;
		tty_stat = DSR | TBE | TRDY;
		return;
	}
	if (data & USRST)
		tty_init = 0;
}

/*
 *	CRT port reset
 */
void mon_crt_reset(void)
{
	crt_init = 0;
	crt_cmd = 0;
	crt_stat = 0;
}

/*
 *	CRT port data input
 */
BYTE mon_crt_data_in(void)
{
	BYTE data;
	static BYTE last;
	struct pollfd p[1];

again:
	/* if no input waiting return last */
	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return (last);

	if (read(fileno(stdin), &data, 1) == 0) {
		/* try to reopen tty, input redirection exhausted */
		if (freopen("/dev/tty", "r", stdin) == NULL)
			LOGE(TAG, "can't reopen /dev/tty");
		set_unix_terminal();
		goto again;
	}
	crt_stat &= ~RBR;

	last = data;
	return (data);
}

/*
 *	CRT port status input
 */
BYTE mon_crt_status_in(void)
{
	struct pollfd p[1];

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		crt_stat |= RBR;
	if (p[0].revents & POLLNVAL) {
		LOGE(TAG, "can't use terminal, try 'screen simulation ...'");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
	}
	return (crt_stat);
}

/*
 *	CRT port data output
 */
void mon_crt_data_out(BYTE data)
{
again:
	if (write(fileno(stdout), &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write CRT data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

/*
 *	CRT port control output
 */
void mon_crt_ctl_out(BYTE data)
{
	if (!crt_init) {
		/* Ignore baud rate, character length, parity and stop bits */
		crt_init = 1;
		crt_stat = DSR | TBE | TRDY;
		return;
	}
	if (data & USRST)
		crt_init = 0;
}

/*
 *	PTR/PTP port reset
 */
void mon_pt_reset(void)
{
}

/*
 *	PTR port data input
 *	(Currently not implemented)
 */
BYTE mon_ptr_data_in(void)
{
	return (0x00);
}

/*
 *	PTR/PTP port status input
 *	(Currently not implemented)
 */
BYTE mon_pt_status_in(void)
{
	return (0x00);
}

/*
 *	PTP port data output
 *	(Currently not implemented)
 */
void mon_ptp_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	PTR/PTP port control output
 *	(Currently not implemented)
 */
void mon_pt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	LPT port status input
 */
BYTE mon_lpt_status_in(void)
{
	return (0x00);
}

/*
 *	LPT port reset
 */
void mon_lpt_reset(void)
{
}

/*
 *	LPT port data output
 */
void mon_lpt_data_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	LPT port control output
 */
void mon_lpt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Interrupt control output
 */
void mon_int_ctl_out(BYTE data)
{
	mon_int = (data & 0x80) | (mon_int & (~data & 0x7f));
}

/*
 *	Interrupt status input
 */
BYTE mon_int_status_in(void)
{
	return (mon_int & 0x7f);
}

/*
 *	Check for interrupts
 */
void mon_int_check(void)
{
	extern void int_request(int);

	if ((mon_int & MENB) == 0)
		return;

	if (((mon_int & ITTYO) && (tty_stat & TBE) && (tty_cmd & TXEN)) ||
	    ((mon_int & ITTYI) && (tty_stat & RBR) && (tty_cmd & RXEN)) ||
	    ((mon_int & IPTP) && (pt_stat & PTPRY)) ||
	    ((mon_int & IPTR) && (pt_stat & PTRDY)) ||
	    ((mon_int & ICRTO) && (crt_stat & TBE) && (tty_cmd & TXEN)) ||
	    ((mon_int & ICRTI) && (crt_stat & RBR) && (tty_cmd & RXEN)) ||
	    ((mon_int & ILPT) && (lpt_stat & LPTRY)))
		int_request(MON_IRQ);
}

/*
 *	Monitor module reset
 */
void mon_reset(void)
{
	mon_int = 0;

	mon_tty_reset();
	mon_crt_reset();
	mon_pt_reset();
	mon_lpt_reset();
}