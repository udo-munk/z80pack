/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2018 by Udo Munk
 *
 * Emulation of an IMSAI SIO-2 S100 board
 *
 * History:
 * 20-OCT-08 first version finished
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 18-JUL-14 don't block on read from terminal
 * 09-OCT-14 modified to support SIO 2
 * 23-MAR-15 drop only null's
 * 22-AUG-17 reopen tty at EOF from input redirection
 * 03-MAY-18 improved accuracy
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "unix_terminal.h"

int sio1_upper_case;
int sio1_strip_parity;
int sio1_drop_nulls;
int sio1_baud_rate = 115200;

int sio2_upper_case;
int sio2_strip_parity;
int sio2_drop_nulls;

struct timeval t1, t2, tdiff;

#define BAUDTIME 10000000

static BYTE status = 0;

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to tty
 * bit 1 = 1, character available for input from tty
 */
BYTE imsai_sio1_status_in(void)
{
	struct pollfd p[1];

	gettimeofday(&t2, NULL);
	timersub(&t2, &t1, &tdiff);

	if((tdiff.tv_sec == 0) && (tdiff.tv_usec < BAUDTIME/sio1_baud_rate)) return(status);

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN | POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		status |= 2;
	if (p[0].revents & POLLOUT)
		status |= 1;

	gettimeofday(&t1, NULL);

	return(status);
}

/*
 * write status register
 */
void imsai_sio1_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters
 */
BYTE imsai_sio1_data_in(void)
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

	gettimeofday(&t1, NULL);
	status &= 0b11111101;

	/* process read data */
	if (sio1_upper_case)
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
void imsai_sio1_data_out(BYTE data)
{
	if (sio1_strip_parity)
		data &= 0x7f;

	if (sio1_drop_nulls)
		if (data == 0)
			return;

again:
	if (write(fileno(stdout), (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write imsai sio2 data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
	gettimeofday(&t1, NULL);
	status &= 0b11111110;
}
