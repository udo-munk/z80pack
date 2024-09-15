/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * Emulation of the console I/O ports on the Mostek SDB-80 CPU board
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) created from altair-88-2sio.c
 */

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"

#include "unix_terminal.h"
#include "mostek-cpu.h"

#include "log.h"
static const char *TAG = "console";

/*
 * read status register
 *
 * bit 6 = 1, character available for input
 * bit 7 = 1, transmitter ready to write character
 */
BYTE sio_status_in(void)
{
	BYTE status = 0;
	struct pollfd p[1];

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN | POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		status |= 0x40;
	if (p[0].revents & POLLOUT)
		status |= 0x80;

	return status;
}

/*
 * write control register
 */
void sio_control_out(BYTE data)
{
	UNUSED(data);
}

/*
 * read data register
 */
BYTE sio_data_in(void)
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

	/* process read data */
	last = data;
	return data;
}

/*
 * write data register
 */
void sio_data_out(BYTE data)
{

again:
	if (write(fileno(stdout), &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write data sio1");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
}

/*
 * read handshake register
 *
 * bit 0 = 1, RTS asserted from terminal
 * bit 1 = 1, DTR asserted from terminal
 * bit 7 = serial data line in, used to detect baud rate
 */
BYTE sio_handshake_in(void)
{
	static BYTE handshake_data = 3;		/* DSR and RTS asserted */

	handshake_data ^= 0x80;			/* toggle serial line each call */
	return handshake_data;
}

/*
 * write control register
 */
void sio_handshake_out(BYTE data)
{
	UNUSED(data);
}
