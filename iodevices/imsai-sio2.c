/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2019 by Udo Munk
 * Copyright (C) 2018-2019 David McNaughton
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
 * 03-JUL-18 implemented baud rate for terminal SIO
 * 13-JUL-18 use logging
 * 14-JUL-18 integrate webfrontend
 * 12-JUL-19 implemented second SIO
 * 27-JUL-19 more correct emulation
 * 17-SEP-19 more consistent SIO naming
 * 23-SEP-19 added AT-modem
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "sim.h"
#include "simglb.h"
#include "unix_terminal.h"
#include "unix_network.h"
#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif
#include "log.h"

#define BAUDTIME 10000000

static const char *TAG = "SIO";

int sio1a_upper_case;
int sio1a_strip_parity;
int sio1a_drop_nulls;
int sio1a_baud_rate = 115200;

static struct timeval sio1a_t1, sio1a_t2;
static BYTE sio1a_stat;

int sio2a_upper_case;
int sio2a_strip_parity;
int sio2a_drop_nulls;
int sio2a_baud_rate = 115200;

static struct timeval sio2a_t1, sio2a_t2;
static BYTE sio2a_stat;

/*
 * the IMSAI SIO-2 occupies 16 I/O ports, from which only
 * 5 have a function. the following two functions are used
 * for the ports without function.
 */
BYTE imsai_sio_nofun_in(void)
{
	return((BYTE) 0);
}

void imsai_sio_nofun_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to tty
 * bit 1 = 1, character available for input from tty
 */
BYTE imsai_sio1a_status_in(void)
{
	extern int time_diff(struct timeval *, struct timeval *);

	struct pollfd p[1];
	int tdiff;

	gettimeofday(&sio1a_t2, NULL);
	tdiff = time_diff(&sio1a_t1, &sio1a_t2);
	if (sio1a_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME/sio1a_baud_rate))
			return(sio1a_stat);

#ifdef HAS_NETSERVER
	if (net_device_alive(DEV_SIO1)) {
		if (net_device_poll(DEV_SIO1)) {
			sio1a_stat |= 2;
		}
		sio1a_stat |= 1;
	} else 
#endif
	{
		p[0].fd = fileno(stdin);
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
			sio1a_stat |= 2;
		if (p[0].revents & POLLOUT)
			sio1a_stat |= 1;
	}

	gettimeofday(&sio1a_t1, NULL);

	return(sio1a_stat);
}

/*
 * write status register
 */
void imsai_sio1a_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters
 */
BYTE imsai_sio1a_data_in(void)
{
	BYTE data;
	static BYTE last;
	struct pollfd p[1];

#ifdef HAS_NETSERVER
	if (net_device_alive(DEV_SIO1)) {
		int res = net_device_get(DEV_SIO1);
		if (res < 0) {
			return(last);
		}
		data = res;
	} else 
#endif
	{
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
	}

	gettimeofday(&sio1a_t1, NULL);
	sio1a_stat &= 0b11111101;

	/* process read data */
	if (sio1a_upper_case)
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
void imsai_sio1a_data_out(BYTE data)
{
	if (sio1a_strip_parity)
		data &= 0x7f;

	if (sio1a_drop_nulls)
		if (data == 0)
			return;

#ifdef HAS_NETSERVER
	if (net_device_alive(DEV_SIO1)) {
		net_device_send(DEV_SIO1, (char *) &data, 1);
	} else 
#endif
	{
again:
		if (write(fileno(stdout), (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				LOGE(TAG, "can't write data");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}
	}

	gettimeofday(&sio1a_t1, NULL);
	sio1a_stat &= 0b11111110;
}

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to tty
 * bit 1 = 1, character available for input from tty
 */
BYTE imsai_sio2a_status_in(void)
{
	extern int time_diff(struct timeval *, struct timeval *);

	struct pollfd p[1];
	int tdiff;

	gettimeofday(&sio2a_t2, NULL);
	tdiff = time_diff(&sio2a_t1, &sio2a_t2);
	if (sio2a_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME/sio2a_baud_rate))
			return(sio2a_stat);

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

	/* if socket is connected check for I/O */
	if (ucons[0].ssc != 0) {
		p[0].fd = ucons[0].ssc;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLIN)
			sio2a_stat |= 2;
		if (p[0].revents & POLLOUT)
			sio2a_stat |= 1;
	} else {
		sio2a_stat = 0;
	}

	gettimeofday(&sio2a_t1, NULL);

	return(sio2a_stat);
}

/*
 * write status register
 */
void imsai_sio2a_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters
 */
BYTE imsai_sio2a_data_in(void)
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

	gettimeofday(&sio2a_t1, NULL);
	sio2a_stat &= 0b11111101;

	/* process read data */
	if (sio2a_upper_case)
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
void imsai_sio2a_data_out(BYTE data)
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

	if (sio2a_strip_parity)
		data &= 0x7f;

	if (sio2a_drop_nulls)
		if (data == 0)
			return;

again:
	if (write(ucons[0].ssc, &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			close(ucons[0].ssc);
			ucons[0].ssc = 0;
		}
	}

	gettimeofday(&sio2a_t1, NULL);
	sio2a_stat &= 0b11111110;
}

#ifdef HAS_MODEM
#include "generic-at-modem.h"
static BYTE sio2b_stat;

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to device
 * bit 1 = 1, character available for input from device
 */
BYTE imsai_sio2b_status_in(void)
{
	// ESP_LOGI(TAG, "SIO2B STAT IN");
	if (modem_device_alive(DEV_SIO2B)) {
		if (modem_device_poll(DEV_SIO2B)) {
			sio2b_stat |= 2;
		}
		sio2b_stat |= 1;
	}

	return(sio2b_stat);
}

/*
 * write status register
 */
void imsai_sio2b_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 */
BYTE imsai_sio2b_data_in(void)
{
	BYTE data = 0;
	static BYTE last;

	if (modem_device_alive(DEV_SIO2B)) {
		int res = modem_device_get(DEV_SIO2B);
		if (res < 0) {
			return(last);
		}
		data = res;
	} 

	sio2b_stat &= 0b11111101;

	last = data;
	return(data);
}

/*
 * write data register
 */
void imsai_sio2b_data_out(BYTE data)
{
	if (modem_device_alive(DEV_SIO2B)) {
		modem_device_send(DEV_SIO2B, (char) data);
	} else 

	sio2b_stat &= 0b11111110;
}


#endif
