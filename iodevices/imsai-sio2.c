/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2021 by Udo Munk
 * Copyright (C) 2018-2021 David McNaughton
 *
 * Emulation of IMSAI SIO-2 S100 boards
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
 * 06-OCT-19 started to implement telnet protocol for modem device
 * 07-OCT-19 implemented baud rate for modem device
 * 09-OCT-19 implement telnet binary transfer
 * 12-NOV-19 implemented SIO control ports
 * 19-JUL-20 avoid problems with some third party terminal emulations
 * 14-JUL-21 added all options for SIO 2B
 * 15-JUL-21 refactor serial keyboard
 * 16-JUL-21 added all options for SIO 1B
 * 01-AUG-21 integrated HAL
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "imsai-hal.h"
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#define LOG_LOCAL_LEVEL LOG_WARN
#include "log.h"

#define BAUDTIME 10000000

static const char *TAG = "SIO";

static BYTE sio1_ctl, sio2_ctl;

int sio1a_upper_case;
int sio1a_strip_parity;
int sio1a_drop_nulls;
int sio1a_baud_rate = 115200;

static struct timeval sio1a_t1, sio1a_t2;
static BYTE sio1a_stat = 0;

int sio1b_upper_case;
int sio1b_strip_parity;
int sio1b_drop_nulls;
int sio1b_baud_rate = 110;

static struct timeval sio1b_t1, sio1b_t2;
static BYTE sio1b_stat = 0;

int sio2a_upper_case;
int sio2a_strip_parity;
int sio2a_drop_nulls;
int sio2a_baud_rate = 115200;

static struct timeval sio2a_t1, sio2a_t2;
static BYTE sio2a_stat = 0;

int sio2b_upper_case;
int sio2b_strip_parity;
int sio2b_drop_nulls;
int sio2b_baud_rate = 2400;

static struct timeval sio2b_t1, sio2b_t2;
static BYTE sio2b_stat = 0;

/*
 * the IMSAI SIO-2 occupies 16 I/O ports, from which only
 * 5 have a function. the following two functions are used
 * for the ports without function.
 */
BYTE imsai_sio_nofun_in(void)
{
	LOGD(TAG,"INVALID SIO PORT"); /* suppress TAG and _log_write warnings */
				      /* won't be seen unless */
				      /* LOG_LOCAL_LEVEL = DEBUG */
	return((BYTE) 0);
}

void imsai_sio_nofun_out(BYTE data)
{
	LOGD(TAG,"INVALID SIO PORT");
	data = data; /* to avoid compiler warning */
}

/* -------------------- SIO 1 Channel A -------------------- */

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to tty
 * bit 1 = 1, character available for input from tty
 */
BYTE imsai_sio1a_status_in(void)
{
	extern int time_diff(struct timeval *, struct timeval *);
	int tdiff;

	gettimeofday(&sio1a_t2, NULL);
	tdiff = time_diff(&sio1a_t1, &sio1a_t2);
	if (sio1a_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME/sio1a_baud_rate))
			return(sio1a_stat);

	hal_status_in(SIO1A, &sio1a_stat);

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
	int data;
	static BYTE last;

	data = hal_data_in(SIO1A);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	gettimeofday(&sio1a_t1, NULL);
	sio1a_stat &= 0b11111101;

	/* process read data */
	if (sio1a_upper_case)
		data = toupper(data);
	last = data;
	return((BYTE) data);
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

	hal_data_out(SIO1A, data);

	gettimeofday(&sio1a_t1, NULL);
	sio1a_stat &= 0b11111110;
}

/* -------------------- SIO 1 Channel B -------------------- */

/*
 * read status register
 */
BYTE imsai_sio1b_status_in(void)
{
	extern int time_diff(struct timeval *, struct timeval *);
	int tdiff;

	gettimeofday(&sio1b_t2, NULL);
	tdiff = time_diff(&sio1b_t1, &sio1b_t2);
	if (sio1b_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME/sio1b_baud_rate))
			return(sio1b_stat);

	hal_status_in(SIO1B, &sio1b_stat);

	gettimeofday(&sio1b_t1, NULL);

	return(sio1b_stat);
}

/*
 * write status register
 */
void imsai_sio1b_status_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 * read data register
 */
BYTE imsai_sio1b_data_in(void)
{
	int data;
	static BYTE last;

	data = hal_data_in(SIO1B);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	gettimeofday(&sio1b_t1, NULL);
	sio1b_stat &= 0b11111101;

	/* process read data */
	if (sio1b_upper_case)
		data = toupper(data);
	last = data;
	return((BYTE) data);
}

/*
 * write data register
 */
void imsai_sio1b_data_out(BYTE data)
{
	if (sio1b_strip_parity)
		data &= 0x7f;

	if (sio1b_drop_nulls)
		if (data == 0)
			return;
			
	hal_data_out(SIO1B, data);

	gettimeofday(&sio1b_t1, NULL);
	sio1b_stat &= 0b11111110;
}

/* -------------------- SIO 2 Channel A -------------------- */

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to tty
 * bit 1 = 1, character available for input from tty
 */
BYTE imsai_sio2a_status_in(void)
{
	extern int time_diff(struct timeval *, struct timeval *);
	int tdiff;

	gettimeofday(&sio2a_t2, NULL);
	tdiff = time_diff(&sio2a_t1, &sio2a_t2);
	if (sio2a_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME/sio2a_baud_rate))
			return(sio2a_stat);

	hal_status_in(SIO2A, &sio2a_stat);

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
	int data;
	static BYTE last;

	data = hal_data_in(SIO2A);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	gettimeofday(&sio2a_t1, NULL);
	sio2a_stat &= 0b11111101;

	/* process read data */
	if (sio2a_upper_case)
		data = toupper(data);
	last = data;
	return((BYTE)data);
}

/*
 * write data register
 *
 * can be configured to strip parity bit because some old software won't.
 * also can drop nulls usually send after CR/LF for teletypes.
 */
void imsai_sio2a_data_out(BYTE data)
{
	if (sio2a_strip_parity)
		data &= 0x7f;

	if (sio2a_drop_nulls)
		if (data == 0)
			return;

	hal_data_out(SIO2A, data);

	gettimeofday(&sio2a_t1, NULL);
	sio2a_stat &= 0b11111110;
}

/* -------------------- SIO 2 Channel B -------------------- */

/*
 * read status register
 *
 * bit 0 = 1, transmitter ready to write character to device
 * bit 1 = 1, character available for input from device
 */
BYTE imsai_sio2b_status_in(void)
{
	extern int time_diff(struct timeval *, struct timeval *);
	int tdiff;

	gettimeofday(&sio2b_t2, NULL);
	tdiff = time_diff(&sio2b_t1, &sio2b_t2);
	if (sio2b_baud_rate > 0)
		if ((tdiff >= 0) && (tdiff < BAUDTIME/sio2b_baud_rate))
			return(sio2b_stat);

	hal_status_in(SIO2B, &sio2b_stat);

	gettimeofday(&sio2b_t1, NULL);

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
 *
 * can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters
 */
BYTE imsai_sio2b_data_in(void)
{
	int data;
	static BYTE last;

	data = hal_data_in(SIO2B);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	gettimeofday(&sio2b_t1, NULL);
	sio2b_stat &= 0b11111101;

	/* process read data */
	if (sio2b_upper_case)
		data = toupper(data);
	last = data;
	return((BYTE)data);
}

/*
 * write data register
 *
 * can be configured to strip parity bit because some old software won't.
 * also can drop nulls usually send after CR/LF for teletypes.
 */
void imsai_sio2b_data_out(BYTE data)
{
	if (sio2b_strip_parity)
		data &= 0x7f;

	if (sio2b_drop_nulls)
		if (data == 0)
			return;

	hal_data_out(SIO2B, data);

	gettimeofday(&sio2b_t1, NULL);
	sio2b_stat &= 0b11111110;
}

/* -------------------- SIO control -------------------- */

/*
 * SIO control input bits:
 * 0 - always 1
 * 1 - always 1
 * 2 - Carrier Detect channel A
 * 3 - Clear To Send channel A
 * 4 - always 1
 * 5 - always 1
 * 6 - Carrier Detect channel B
 * 7 - Clear To Send channel B
 */
BYTE imsai_sio1_ctl_in(void)
{
	int cd_a = hal_carrier_detect(SIO1A);
	int cd_b = hal_carrier_detect(SIO1B);

	return(0b10111011 | (cd_a << 2) | cd_b << 6);
}

BYTE imsai_sio2_ctl_in(void)
{
	int cd_a = hal_carrier_detect(SIO2A);
	int cd_b = hal_carrier_detect(SIO2B);

	return(0b10111011 | (cd_a << 2) | cd_b << 6);
}

/*
 * SIO control output bits:
 * 0 - Interrupt Enable channel A (not implemented)
 * 1 - Carrier Detect channel A
 * 2 - no function
 * 3 - no function
 * 4 - Interrupt Enable channel B (not implemented)
 * 5 - Carrier Detect channel B (jumpered for receive, so no function)
 * 6 - no function
 * 7 - no function
 */
void imsai_sio1_ctl_out(BYTE data)
{
	sio1_ctl = data;
}

void imsai_sio2_ctl_out(BYTE data)
{
	sio2_ctl = data;
}
