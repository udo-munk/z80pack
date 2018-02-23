/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2017 by Udo Munk
 *
 * Partial emulation of an Altair 88-2SIO S100 board
 *
 * History:
 * 20-OCT-08 first version finished
 * 31-JAN-14 use correct name from the manual
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 17-JUL-14 don't block on read from terminal
 * 09-OCT-14 modified to support 2 SIO's
 * 23-MAR-15 drop only null's
 * 02-SEP-16 reopen tty at EOF from input redirection
 * 24-FEB-17 improved tty reopen
 * 22-MAR-17 connected SIO 2 to UNIX domain socket
 * 23-OCT-17 improved UNIX domain socket connections
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sim.h"
#include "simglb.h"
#include "unix_terminal.h"
#include "unix_network.h"

int sio1_upper_case;
int sio1_strip_parity;
int sio1_drop_nulls;

int sio2_upper_case;
int sio2_strip_parity;
int sio2_drop_nulls;

/*
 * read status register
 *
 * bit 0 = 1, character available for input from tty
 * bit 1 = 1, transmitter ready to write character to tty
 */
BYTE altair_sio1_status_in(void)
{
	BYTE status = 0;
	struct pollfd p[1];

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN | POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		status |= 1;
	if (p[0].revents & POLLOUT)
		status |= 2;

	return(status);
}

/*
 * write status register
 */
void altair_sio1_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * Can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters.
 */
BYTE altair_sio1_data_in(void)
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
		return(last);

	if (read(fileno(stdin), &data, 1) == 0) {
		/* try to reopen tty, input redirection exhausted */
		freopen("/dev/tty", "r", stdin);
		set_unix_terminal();
		goto again;
	}

	/* process read data */
	last = data;
	if (sio1_upper_case)
		data = toupper(data);
	return(data);
}

/*
 * write data register
 *
 * Can be configured to strip parity bit because some old software won't.
 * Also can drop nulls usually send after CR/LF for teletypes.
 */
void altair_sio1_data_out(BYTE data)
{
	if (sio1_drop_nulls)
		if (data == 0)
			return;

	if (sio1_strip_parity)
		data &= 0x7f;

again:
	if (write(fileno(stdout), &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write data sio1");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

/*
 * read status register
 *
 * bit 0 = 1, character available for input from tty
 * bit 1 = 1, transmitter ready to write character to tty
 */
BYTE altair_sio2_status_in(void)
{
	BYTE status = 0;
	struct pollfd p[1];

	/* if socket not connected check for a new connection */
	if (ucons[1].ssc == 0) {
		p[0].fd = ucons[1].ss;
		p[0].events = POLLIN;
		p[0].revents = 0;
		poll(p, 1, 0);
		/* accept a new connection */
		if (p[0].revents) {
			if ((ucons[1].ssc = accept(ucons[1].ss, NULL,
			     NULL)) == -1) {
				perror("accept server socket");
				ucons[1].ssc = 0;
			}
		}
	}

	/* if socket is connected check for I/O */
	if (ucons[1].ssc != 0) {
		p[0].fd = ucons[1].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
			status |= 1;
		if (p[0].revents & POLLOUT)
			status |= 2;
	}

	return(status);
}

/*
 * write status register
 */
void altair_sio2_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * Can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters.
 */
BYTE altair_sio2_data_in(void)
{
	BYTE data;
	static BYTE last;
	struct pollfd p[1];

	/* if not connected return last */
	if (ucons[1].ssc == 0)
		return(last);

	/* if no input waiting return last */
	p[0].fd = ucons[1].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return(last);

	if (read(ucons[1].ssc, &data, 1) != 1) {
		close(ucons[1].ssc);
		ucons[1].ssc = 0;
		return(last);
	}

	last = data;
	if (sio2_upper_case)
		data = toupper(data);
	return(data);
}

/*
 * write data register
 *
 * Can be configured to strip parity bit because some old software won't.
 * Also can drop nulls usually send after CR/LF for teletypes.
 */
void altair_sio2_data_out(BYTE data)
{
	struct pollfd p[1];

	/* return if socket not connected */
	if (ucons[1].ssc == 0)
		return;

	/* if output not possible close socket and return */
	p[0].fd = ucons[1].ssc;
	p[0].events = POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLOUT)) {
		close(ucons[1].ssc);
		ucons[1].ssc = 0;
		return;
	}

	if (sio2_drop_nulls)
		if (data == 0)
			return;

	if (sio2_strip_parity)
		data &= 0x7f;

again:
	if (write(ucons[1].ssc, &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			close(ucons[1].ssc);
			ucons[1].ssc = 0;
		}
	}
}
