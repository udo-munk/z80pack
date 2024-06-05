/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec MDS-800 monitor module.
 * It includes the TTY, CRT, PTR, PTP, line printer, and
 * PROM programmer interface.
 *
 * History:
 * 03-JUN-2024 first version
 */

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "mds-monitor.h"
#include "unix_network.h"
#include "unix_terminal.h"
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

			/* interrupt status and control bits */
#define ITTYO	0x01	/* output TTY */
#define ITTYI	0x02	/* input TTY */
#define IPTP	0x04	/* PTP */
#define IPTR	0x08	/* PTR */
#define ICRTO	0x10	/* output CRT */
#define ICRTI	0x20	/* input CRT */
#define ILPT	0x40	/* LPT */
#define MENB	0x80	/* enable monitor interrupts */

#define MON_IRQ	3	/* monitor module interrupt */

#define BAUDTIME 10000000

extern uint64_t get_clock_us(void);

static const char *TAG = "MONITOR";

static BYTE mon_int;	/* Interrupts enabled & signals */

int tty_upper_case;
int tty_strip_parity;
int tty_drop_nulls;
int tty_baud_rate = 1200;

static uint64_t tty_t1, tty_t2;
static int tty_init;	/* TTY initialized flag */
static BYTE tty_cmd;	/* TTY command byte */
static BYTE tty_stat;	/* TTY status byte */

int crt_upper_case;
int crt_strip_parity;
int crt_drop_nulls;
int crt_baud_rate = 115200;

static uint64_t crt_t1, crt_t2;
static int crt_init;	/* CRT initialized flag */
static BYTE crt_cmd;	/* TTY command byte */
static BYTE crt_stat;	/* TTY status byte */

int pt_baud_rate = 2400;

static uint64_t pt_t1, pt_t2;
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
	BYTE data, dummy;
	static BYTE last;
	struct pollfd p[1];

	if ((tty_cmd & RXEN) == 0)
		return (last);

	/* if not connected return last */
	if (ncons[0].ssc == 0)
		return (last);

	/* if no input waiting return last */
	p[0].fd = ncons[0].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return (last);

	if (read(ncons[0].ssc, &data, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			/* EOF, close socket and return last */
			close(ncons[0].ssc);
			ncons[0].ssc = 0;
			return (last);
		} else {
			LOGE(TAG, "can't read tcpsocket data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			return (last);
		}
	}

	tty_t1 = get_clock_us();
	tty_stat &= ~RBR;

	/* process read data */
	/* telnet client sends \r\n or \r\0, drop second character */
	if (ncons[0].telnet && (data == '\r'))
		if (read(ncons[0].ssc, &dummy, 1) != 1)
			LOGE(TAG, "can't read tcpsocket data");
	if (tty_upper_case)
		data = toupper(data);
	last = data;
	return (data);
}

/*
 *	TTY port status input
 */
BYTE mon_tty_status_in(void)
{
	extern void int_request(int);

	struct pollfd p[1];
	int tdiff;

	if ((tty_cmd & RXEN) == 0)
		return (tty_stat);

	tty_t2 = get_clock_us();
	tdiff = tty_t2 - tty_t1;
	if (tty_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME / tty_baud_rate))
			return (tty_stat);

	/* if socket is connected check for I/O */
	if (ncons[0].ssc != 0) {
		p[0].fd = ncons[0].ssc;
		p[0].events = POLLIN;
		p[0].revents = 0;
		poll(p, 1, 0);
		tty_stat &= ~(TRDY | TBE | RBR);
		if (p[0].revents & POLLHUP) {
			close(ncons[0].ssc);
			ncons[0].ssc = 0;
		} else if (p[0].revents & POLLIN) {
			tty_stat |= RBR;
			if (mon_int & (MENB | ITTYI))
				int_request(MON_IRQ);
		} else {
			tty_stat |= TRDY | TBE;
			if (mon_int & (MENB | ITTYO))
				int_request(MON_IRQ);
		}
	} else
		tty_stat &= ~(TRDY | TBE | RBR);

	tty_t1 = get_clock_us();

	return (tty_stat);
}

/*
 *	TTY port data output
 */
void mon_tty_data_out(BYTE data)
{
	if ((tty_cmd & TXEN) == 0)
		return;

	/* return if socket not connected */
	if (ncons[0].ssc == 0)
		return;

	if (tty_drop_nulls)
		if (data == 0)
			return;

	if (tty_strip_parity)
		data &= 0x7f;

again:
	if (write(ncons[0].ssc, &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write socket data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}

	tty_t1 = get_clock_us();
	tty_stat &= ~(TBE | TRDY);
}

/*
 *	TTY port control output
 */
void mon_tty_ctl_out(BYTE data)
{
	if (!tty_init) {
		/* ignore baud rate, character length, parity and stop bits */
		tty_init = 1;
		tty_stat = DSR | TBE | TRDY;
		return;
	}
	if (data & USRST)
		mon_tty_reset();
	else
		tty_cmd = data & (RTS | CLERR | RXEN | DTR | TXEN);
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

	if ((crt_cmd & RXEN) == 0)
		return (last);

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

	crt_t1 = get_clock_us();
	crt_stat &= ~RBR;

	/* process read data */
	if (crt_upper_case)
		data = toupper(data);
	last = data;
	return (data);
}

/*
 *	CRT port status input
 */
BYTE mon_crt_status_in(void)
{
	extern void int_request(int);

	struct pollfd p[1];
	int tdiff;

	if ((crt_cmd & RXEN) == 0)
		return (crt_stat);

	crt_t2 = get_clock_us();
	tdiff = crt_t2 - crt_t1;
	if (crt_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME / crt_baud_rate))
			return (crt_stat);

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN) {
		crt_stat |= RBR;
		if (mon_int & (MENB | ICRTI))
			int_request(MON_IRQ);
	}
	if (p[0].revents & POLLNVAL) {
		LOGE(TAG, "can't use terminal, try 'screen simulation ...'");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
	}
	crt_stat |= TBE | TRDY;
	if (mon_int & (MENB | ICRTO))
		int_request(MON_IRQ);

	crt_t1 = get_clock_us();

	return (crt_stat);
}

/*
 *	CRT port data output
 */
void mon_crt_data_out(BYTE data)
{
	if ((crt_cmd & TXEN) == 0)
		return;

	if (crt_drop_nulls)
		if (data == 0)
			return;

	if (crt_strip_parity)
		data &= 0x7f;

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

	crt_t1 = get_clock_us();
	crt_stat &= ~(TBE | TRDY);
}

/*
 *	CRT port control output
 */
void mon_crt_ctl_out(BYTE data)
{
	if (!crt_init) {
		/* ignore baud rate, character length, parity and stop bits */
		crt_init = 1;
		crt_stat = DSR | TBE | TRDY;
		return;
	}
	if (data & USRST)
		mon_crt_reset();
	else
		crt_cmd = data & (RTS | CLERR | RXEN | DTR | TXEN);
}

/*
 *	PTR/PTP port reset
 */
void mon_pt_reset(void)
{
	pt_stat = 0;
}

/*
 *	PTR port data input
 */
BYTE mon_ptr_data_in(void)
{
	BYTE data;
	static BYTE last;
	struct pollfd p[1];

	/* if not connected return last */
	if (ucons[0].ssc == 0)
		return (last);

	/* if no input waiting return last */
	p[0].fd = ucons[0].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return (last);

	if (read(ucons[0].ssc, &data, 1) != 1) {
		/* EOF, close socket and return last */
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		return (last);
	}

	pt_t1 = get_clock_us();
	pt_stat &= ~PTRDY;

	/* process read data */
	last = data;
	return (data);
}

/*
 *	PTR/PTP port status input
 */
BYTE mon_pt_status_in(void)
{
	struct pollfd p[1];
	int tdiff;

	/* if socket not connected check for a new connection */
	if (ucons[0].ssc == 0) {
		p[0].fd = ucons[0].ss;
		p[0].events = POLLIN;
		p[0].revents = 0;
		poll(p, 1, 0);
		/* accept a new connection */
		if (p[0].revents) {
			if ((ucons[0].ssc = accept(ucons[0].ss, NULL,
						   NULL)) == -1) {
				LOGW(TAG, "can't accept server socket");
				ucons[0].ssc = 0;
			}
		}
	}

	pt_t2 = get_clock_us();
	tdiff = pt_t2 - pt_t1;
	if (pt_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME / pt_baud_rate))
			return (pt_stat);

	/* if socket is connected check for I/O */
	if (ucons[0].ssc != 0) {
		p[0].fd = ucons[0].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
			pt_stat |= PTRDY;
		if (p[0].revents & POLLOUT)
			pt_stat |= PTPRY;
	}

	pt_t1 = get_clock_us();

	return (pt_stat);
}

/*
 *	PTP port data output
 */
void mon_ptp_data_out(BYTE data)
{
	struct pollfd p[1];

	/* return if socket not connected */
	if (ucons[0].ssc == 0)
		return;

	/* if output not possible close socket and return */
	p[0].fd = ucons[0].ssc;
	p[0].events = POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLOUT)) {
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		return;
	}

again:
	if (write(ucons[0].ssc, &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			close(ucons[0].ssc);
			ucons[0].ssc = 0;
		}
	}

	pt_t1 = get_clock_us();
	pt_stat &= ~PTPRY;
}

/*
 *	PTR/PTP port control output
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
	lpt_stat |= LPTRY;

	return (lpt_stat);
}

/*
 *	LPT port reset
 */
void mon_lpt_reset(void)
{
	lpt_stat = 0;
}

/*
 *	LPT port data output
 */
void mon_lpt_data_out(BYTE data)
{
	extern int lpt_fd;

	if (lpt_fd == 0) {
		if ((lpt_fd = creat("printer.txt", 0664)) == -1) {
			LOGE(TAG, "can't create printer.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			lpt_fd = 0;
			return;
		}
	}

	/* uncomplement data */
	data = ~data;

	if ((data != '\r') && (data != 0x00)) {
again:
		if (write(lpt_fd, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				LOGE(TAG, "can't write to printer.txt");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}
	}
	lpt_stat &= ~LPTRY;
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

	if (((mon_int & ITTYO) && (tty_stat & TXEN) && (tty_cmd & TBE)) ||
	    ((mon_int & ITTYI) && (tty_stat & RXEN) && (tty_cmd & RBR)) ||
	    ((mon_int & IPTP) && (pt_stat & PTPRY)) ||
	    ((mon_int & IPTR) && (pt_stat & PTRDY)) ||
	    ((mon_int & ICRTO) && (crt_stat & TXEN) && (tty_cmd & TBE)) ||
	    ((mon_int & ICRTI) && (crt_stat & RXEN) && (tty_cmd & RBR)) ||
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
