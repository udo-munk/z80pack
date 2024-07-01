/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2016-2020 by Udo Munk
 *
 * Partial emulation of an Altair 88-SIO Rev. 0/1 for terminal I/O,
 * and 88-SIO Rev. 1 for tape I/O
 *
 * History:
 * 12-JUL-2016 first version
 * 02-SEP-2016 reopen tty at EOF from input redirection
 * 24-FEB-2017 improved tty reopen
 * 24-MAR-2017 added configuration
 * 27-MAR-2017 added SIO 3 for tape connected to UNIX domain socket
 * 23-OCT-2017 improved UNIX domain socket connections
 * 03-MAY-2018 improved accuracy
 * 04-JUL-2018 added baud rate to terminal SIO
 * 15-JUL-2018 use logging
 * 24-NOV-2019 configurable baud rate for tape SIO
 * 19-JUL-2020 avoid problems with some third party terminal emulations
 */

#include <stdint.h>
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
#include "simfun.h"
#include "log.h"
#include "unix_terminal.h"
#include "unix_network.h"

#define BAUDTIME 10000000

static const char *TAG = "SIO";

int sio0_upper_case;
int sio0_strip_parity;
int sio0_drop_nulls;
int sio0_revision;
int sio0_baud_rate = 115200;

static uint64_t sio0_t1, sio0_t2;
static BYTE sio0_stat;

int sio3_baud_rate = 1200;

static uint64_t sio3_t1, sio3_t2;
static BYTE sio3_stat = 0x81;

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
	struct pollfd p[1];
	int tdiff;

	if (sio0_revision == 0)
		sio0_stat = 0;
	else
		sio0_stat = 0x81;

	sio0_t2 = get_clock_us();
	tdiff = sio0_t2 - sio0_t1;
	if (sio0_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME / sio0_baud_rate))
			return sio0_stat;

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN) {
		if (sio0_revision == 0)
			sio0_stat |= 32;
		else
			sio0_stat &= ~1;
	}
	if (p[0].revents & POLLNVAL) {
		LOGE(TAG, "can't use terminal, try 'screen simulation ...'");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
	}
	if (sio0_revision == 0)
		sio0_stat |= 2;
	else
		sio0_stat &= ~128;

	sio0_t1 = get_clock_us();

	return sio0_stat;
}

/*
 * write status register
 */
void altair_sio0_status_out(BYTE data)
{
	UNUSED(data);
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
		return last;

	if (read(fileno(stdin), &data, 1) == 0) {
		/* try to reopen tty, input redirection exhausted */
		if (freopen("/dev/tty", "r", stdin) == NULL)
			LOGE(TAG, "can't reopen /dev/tty");
		set_unix_terminal();
		goto again;
	}

	sio0_t1 = get_clock_us();
	if (sio0_revision == 0)
		sio0_stat &= 0b11011111;
	else
		sio0_stat |= 0b00000001;

	/* process read data */
	if (sio0_upper_case)
		data = toupper(data);
	last = data;
	return data;
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
			LOGE(TAG, "can't write sio0 data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}

	sio0_t1 = get_clock_us();
	if (sio0_revision == 0)
		sio0_stat &= 0b11111101;
	else
		sio0_stat |= 0b10000000;
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

	sio3_t2 = get_clock_us();
	tdiff = sio3_t2 - sio3_t1;
	if (sio3_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME / sio3_baud_rate))
			return sio3_stat;

	/* if socket is connected check for I/O */
	if (ucons[0].ssc != 0) {
		p[0].fd = ucons[0].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
			sio3_stat &= ~1;
		if (p[0].revents & POLLOUT)
			sio3_stat &= ~128;
	}

	sio3_t1 = get_clock_us();

	return sio3_stat;
}

/*
 * write status register
 */
void altair_sio3_status_out(BYTE data)
{
	UNUSED(data);
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
		return last;

	/* if no input waiting return last */
	p[0].fd = ucons[0].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return last;

	if (read(ucons[0].ssc, &data, 1) != 1) {
		/* EOF, close socket and return last */
		close(ucons[0].ssc);
		ucons[0].ssc = 0;
		return last;
	}

	sio3_t1 = get_clock_us();
	sio3_stat |= 0b00000001;

	/* process read data */
	last = data;
	return data;
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

	sio3_t1 = get_clock_us();
	sio3_stat |= 0b10000000;
}
