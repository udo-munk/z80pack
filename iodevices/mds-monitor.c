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
 * 07-JUN-2024 rewrite of the monitor ports and the timing thread
 */

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
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

static const char *TAG = "MONITOR";

static BYTE mon_int;	/* Interrupts enabled & signals */
static pthread_mutex_t mon_int_mutex = PTHREAD_MUTEX_INITIALIZER;

int tty_upper_case;
int tty_strip_parity;
int tty_drop_nulls;
int tty_clock_div = 32;	/* 1200 baud */

static int tty_init;	/* TTY initialized flag */
static BYTE tty_cmd;	/* TTY command byte */
static int tty_trdy;	/* TTY transmit ready */
static int tty_rbr;	/* TTY receiver buffer ready */

int crt_upper_case;
int crt_strip_parity;
int crt_drop_nulls;
int crt_clock_div = 1;	/* 38400 baud */

static int crt_init;	/* CRT initialized flag */
static BYTE crt_cmd;	/* CRT command byte */
static int crt_trdy;	/* CRT transmit ready */
static int crt_rbr;	/* CRT receiver buffer ready */

int pt_clock_div = 16;	/* 2400 baud */

static int ptp_rdy;	/* PTP ready */
static int ptr_rdy;	/* PTR ready */

int lpt_clock_div = 1;	/* 38400 baud */

static int lpt_rdy;	/* LPT ready */

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
 *	Update interrupt request line
 */
static void mon_int_update(BYTE ints_cleared, BYTE ints_set)
{
	extern void int_request(int), int_cancel(int);

	pthread_mutex_lock(&mon_int_mutex);
	mon_int &= ~ints_cleared;
	mon_int |= ints_set;
	pthread_mutex_unlock(&mon_int_mutex);

	/* check if interrupts are enabled and there are any requests */
	if (mon_int > 0x80)
		int_request(MON_IRQ);
	else
		int_cancel(MON_IRQ);
}

/*
 *	Interrupt control output
 */
void mon_int_ctl_out(BYTE data)
{
	pthread_mutex_lock(&mon_int_mutex);
	mon_int = (data & 0x80) | (mon_int & 0x7f);
	pthread_mutex_unlock(&mon_int_mutex);

	mon_int_update(data & 0x7f, 0);
}

/*
 *	Interrupt status input
 */
BYTE mon_int_status_in(void)
{
	return (mon_int & 0x7f);
}

/*
 *	TTY port periodic status update
 */
void mon_tty_periodic(void)
{
	struct pollfd p[1];

	if (!(tty_cmd & RXEN))
		return;

	/* if socket is connected check for I/O */
	if (ncons[0].ssc != 0) {
		if (!tty_rbr) {
			p[0].fd = ncons[0].ssc;
			p[0].events = POLLIN;
			p[0].revents = 0;
			poll(p, 1, 0);
			if (p[0].revents & POLLHUP) {
				close(ncons[0].ssc);
				ncons[0].ssc = 0;
			} else if (p[0].revents & POLLIN) {
				tty_rbr = 1;
				mon_int_update(0, ITTYI);
			}
		}
		if (!tty_trdy) {
			tty_trdy = 1;
			mon_int_update(0, ITTYO);
		}
	} else {
		tty_trdy = tty_rbr = 0;
		mon_int_update(ITTYI | ITTYO, 0);
	}
}

/*
 *	TTY port data input
 */
BYTE mon_tty_data_in(void)
{
	BYTE data, dummy;
	static BYTE last;

	if (!(tty_cmd & RXEN) || !tty_rbr)
		return (last);

	if (ncons[0].ssc == 0) {
		data = last;
		goto done;
	}

	if (read(ncons[0].ssc, &data, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			/* EOF, close socket and return last */
			close(ncons[0].ssc);
			ncons[0].ssc = 0;
			data = last;
			goto done;
		} else {
			LOGE(TAG, "can't read tcpsocket data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			data = last;
			goto done;
		}
	}

	/* process read data */
	/* telnet client sends \r\n or \r\0, drop second character */
	if (ncons[0].telnet && (data == '\r'))
		if (read(ncons[0].ssc, &dummy, 1) != 1)
			LOGE(TAG, "can't read tcpsocket data");
	if (tty_upper_case)
		data = toupper(data);
	last = data;

done:
	tty_rbr = 0;
	mon_int_update(ITTYI, 0);

	return (data);
}

/*
 *	TTY port status input
 */
BYTE mon_tty_status_in(void)
{
	BYTE data;

	if (!(tty_cmd & RXEN))
		return (0);

	data = DSR;
	if (tty_trdy)
		data |= TBE | TRDY;
	if (tty_rbr)
		data |= RBR;

	return (data);
}

/*
 *	TTY port data output
 */
void mon_tty_data_out(BYTE data)
{
	if (!(tty_cmd & TXEN) || !tty_trdy)
		return;

	/* return if socket not connected */
	if (ncons[0].ssc == 0)
		goto done;

	if (tty_drop_nulls && data == 0)
		goto done;

	if (tty_strip_parity)
		data &= 0x7f;

again:
	if (write(ncons[0].ssc, &data, 1) != 1) {
		if (errno == EINTR)
			goto again;
		else {
			LOGE(TAG, "can't write socket data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}

done:
	tty_trdy = 0;
	mon_int_update(ITTYO, 0);
}

/*
 *	TTY port control output
 */
void mon_tty_ctl_out(BYTE data)
{
	if (!tty_init) {
		/* ignore baud rate, character length, parity and stop bits */
		tty_init = 1;
		return;
	}
	if (data & USRST) {
		tty_init = tty_cmd = 0;
		tty_rbr = tty_trdy = 0;
		mon_int_update(ITTYI | ITTYO, 0);
	} else
		tty_cmd = data & (RTS | CLERR | RXEN | DTR | TXEN);
}

/*
 *	CRT port periodic status update
 */
void mon_crt_periodic(void)
{
	struct pollfd p[1];

	if (!(crt_cmd & RXEN))
		return;

	if (!crt_rbr) {
		p[0].fd = fileno(stdin);
		p[0].events = POLLIN;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN) {
			crt_rbr = 1;
			mon_int_update(0, ICRTI);
		}
		if (p[0].revents & POLLNVAL) {
			LOGE(TAG, "can't use terminal, "
			     "try 'screen simulation ...'");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
	if (!crt_trdy) {
		crt_trdy = 1;
		mon_int_update(0, ICRTO);
	}
}

/*
 *	CRT port data input
 */
BYTE mon_crt_data_in(void)
{
	BYTE data;
	static BYTE last;

	if (!(crt_cmd & RXEN) || !crt_rbr)
		return (last);

again:
	if (read(fileno(stdin), &data, 1) == 0) {
		/* try to reopen tty, input redirection exhausted */
		if (freopen("/dev/tty", "r", stdin) == NULL)
			LOGE(TAG, "can't reopen /dev/tty");
		set_unix_terminal();
		goto again;
	}

	/* process read data */
	if (crt_upper_case)
		data = toupper(data);
	last = data;

	crt_rbr = 0;
	mon_int_update(ICRTI, 0);

	return (data);
}

/*
 *	CRT port status input
 */
BYTE mon_crt_status_in(void)
{
	BYTE data;

	if (!(crt_cmd & RXEN))
		return (0);

	data = DSR;
	if (crt_trdy)
		data |= TBE | TRDY;
	if (crt_rbr)
		data |= RBR;

	return (data);
}

/*
 *	CRT port data output
 */
void mon_crt_data_out(BYTE data)
{
	if (!(crt_cmd & TXEN) || !crt_trdy)
		return;

	if (crt_drop_nulls && data == 0)
		goto done;

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

done:
	crt_trdy = 0;
	mon_int_update(ICRTO, 0);
}

/*
 *	CRT port control output
 */
void mon_crt_ctl_out(BYTE data)
{
	if (!crt_init) {
		/* ignore baud rate, character length, parity and stop bits */
		crt_init = 1;
		return;
	}
	if (data & USRST) {
		crt_init = crt_cmd = 0;
		crt_rbr = crt_trdy = 0;
		mon_int_update(ICRTI | ICRTO, 0);
	} else
		crt_cmd = data & (RTS | CLERR | RXEN | DTR | TXEN);
}

/*
 *	PTR/PTP periodic status update
 */
void mon_pt_periodic(void)
{
	struct pollfd p[1];

	/* if socket is connected check for I/O */
	if (ucons[0].ssc != 0) {
		p[0].fd = ucons[0].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (!ptr_rdy && (p[0].revents & POLLIN)) {
			ptr_rdy = 1;
			mon_int_update(0, IPTR);
		}
		if (!ptp_rdy && (p[0].revents & POLLOUT)) {
			ptp_rdy = 1;
			mon_int_update(0, IPTP);
		}
	}
}

/*
 *	PTR port data input
 */
BYTE mon_ptr_data_in(void)
{
	BYTE data;
	static BYTE last;

	if (!ptr_rdy)
		return (last);

	/* if not connected return last */
	if (ucons[0].ssc == 0) {
		data = last;
		goto done;
	}

	if (read(ucons[0].ssc, &data, 1) != 1) {
		/* EOF, close socket and return last */
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		data = last;
		goto done;
	}

	/* process read data */
	last = data;

done:
	ptr_rdy = 0;
	mon_int_update(IPTR, 0);

	return (data);
}

/*
 *	PTR/PTP port status input
 */
BYTE mon_pt_status_in(void)
{
	BYTE data;
	struct pollfd p[1];

	/* if socket is not connected check for a new connection */
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

	data = 0;
	if (ptr_rdy)
		data |= PTRDY;
	if (ptp_rdy)
		data |= PTPRY;

	return (data);
}

/*
 *	PTP port data output
 */
void mon_ptp_data_out(BYTE data)
{
	if (!ptp_rdy)
		return;

	/* return if socket not connected */
	if (ucons[0].ssc == 0)
		goto done;

again:
	if (write(ucons[0].ssc, &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			close(ucons[0].ssc);
			ucons[0].ssc = 0;
		}
	}

done:
	ptp_rdy = 0;
	mon_int_update(IPTP, 0);
}

/*
 *	PTR/PTP port control output
 */
void mon_pt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	LPT port periodic status update
 */
void mon_lpt_periodic(void)
{
	lpt_rdy = 1;
	mon_int_update(0, ILPT);
}

/*
 *	LPT port status input
 */
BYTE mon_lpt_status_in(void)
{
	return (lpt_rdy ? LPTRY : 0);
}

/*
 *	LPT port data output
 */
void mon_lpt_data_out(BYTE data)
{
	extern int lpt_fd;

	if (!lpt_rdy)
		return;

	if (lpt_fd == 0) {
		if ((lpt_fd = creat("printer.txt", 0664)) == -1) {
			LOGE(TAG, "can't create printer.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			lpt_fd = 0;
			goto done;
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

done:
	lpt_rdy = 0;
	mon_int_update(ILPT, 0);
}

/*
 *	LPT port control output
 */
void mon_lpt_ctl_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Monitor module reset
 */
void mon_reset(void)
{
	tty_init = tty_cmd = 0;
	tty_trdy = tty_rbr = 0;
	crt_init = crt_cmd = 0;
	crt_trdy = crt_rbr = 0;
	ptp_rdy = ptr_rdy = 0;
	lpt_rdy = 0;
	pthread_mutex_lock(&mon_int_mutex);
	mon_int = 0;
	pthread_mutex_unlock(&mon_int_mutex);
	mon_int_update(0, 0);
}
