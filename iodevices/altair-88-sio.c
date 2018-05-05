/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2016-2018 by Udo Munk
 *
 * Partial emulation of an Altair 88-SIO Rev. 0/1 for terminal I/O,
 * and 88-SIO Rev. 1 for tape I/O
 *
 * History:
 * 12-JUL-16 first version
 * 02-SEP-16 reopen tty at EOF from input redirection
 * 24-FEB-17 improved tty reopen
 * 24-MAR-17 added configuration
 * 27-MAR-17 added SIO 3 for tape connected to UNIX domain socket
 * 23-OCT-17 improved UNIX domain socket connections
 * 03-MAY-18 improved accuracy
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

int sio0_upper_case;
int sio0_strip_parity;
int sio0_drop_nulls;
int sio0_revision;

/*
 * read status register
 *
 * SIO Rev 0:
 * bit 5 = 1, character available for input from tty
 * bit 1 = 1, transmitter ready to write character to tty
 *
 * SIO Rev 1:
 * bit 0 = 0, character available for input from SIO
 * bit 7 = 0, transmitter ready to write character to SIO
 */
BYTE altair_sio0_status_in(void)
{
	BYTE status;
	struct pollfd p[1];

	if (sio0_revision == 0)
		status = 0;
	else
		status = 0x81;

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN | POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN) {
		if (sio0_revision == 0)
			status |= 32;
		else
			status &= ~1;
	}
	if (p[0].revents & POLLOUT) {
		if (sio0_revision == 0)
			status |= 2;
		else
			status &= ~128;
	}

	return(status);
}

/*
 * write status register
 */
void altair_sio0_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters
 */
BYTE altair_sio0_data_in(void)
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
		goto again;
	}

	/* process read data */
	if (sio0_upper_case)
		data = toupper(data);
	last = data;
	return(data);
}

/*
 * write data register
 *
 * can be configured to strip parity bit because some old software won't.
 * also can drop nulls usually send after CR/LF for teletypes.
 */
void altair_sio0_data_out(BYTE data)
{
	if (sio0_drop_nulls)
		if (data == 0)
			return;

	if (sio0_strip_parity)
		data &= 0x7f;

again:
	if (write(fileno(stdout), &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write altair sio0 data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

/*
 * read status register
 *
 * bit 0 = 0, character available for input from SIO
 * bit 7 = 0, transmitter ready to write character to SIO
 *
 * Rev. 1 SIO was used for the 88-ACR
 */
BYTE altair_sio3_status_in(void)
{
	BYTE status = 0x81;
	struct pollfd p[1];

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
				perror("accept server socket");
				ucons[0].ssc = 0;
			}
		}
	}

	/* if socket is connected check for I/O */
	if (ucons[0].ssc != 0) {
		p[0].fd = ucons[0].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
			status &= ~1;
		if (p[0].revents & POLLOUT)
			status &= ~128;
	}

	return(status);
}

/*
 * write status register
 */
void altair_sio3_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 */
BYTE altair_sio3_data_in(void)
{
	BYTE data;
	static BYTE last;
	struct pollfd p[1];

	/* if not connected return last */
	if (ucons[0].ssc == 0)
		return(last);

	/* if no input waiting return last */
	p[0].fd = ucons[0].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return(last);

	if (read(ucons[0].ssc, &data, 1) != 1) {
		/* EOF, close socket and return last */
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		return(last);
	}

	/* process read data */
	last = data;
	return(data);
}

/*
 * write data register
 */
void altair_sio3_data_out(BYTE data)
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
}
