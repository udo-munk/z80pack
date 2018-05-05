/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2018 by Udo Munk
 *
 * Emulation of a Cromemco TU-ART S100 board
 *
 * History:
 *    DEC-14 first version
 *    JAN-15 better subdue of non printable characters in output
 * 02-FEB-15 implemented the timers and interrupt flag for TBE
 * 05-FEB-15 implemented interrupt flag for RDA
 * 14-FEB-15 improvements, so that the Cromix tty driver works
 * 10-MAR-15 lpt's implemented for CP/M, CDOS and Cromix
 * 23-MAR-15 drop only null's
 * 26-MAR-15 tty's implemented for CDOS and Cromix
 * 25-APR-18 cleanup
 * 03-MAY-18 improved accuracy
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "sim.h"
#include "simglb.h"
#include "unix_terminal.h"
#include "unix_network.h"

/************************/
/*	Device 0A	*/
/************************/

int uart0a_int_mask, uart0a_int, uart0a_int_pending, uart0a_rst7;
int uart0a_timer1, uart0a_timer2, uart0a_timer3, uart0a_timer4, uart0a_timer5;
int uart0a_tbe, uart0a_rda;

/*
 * D7	Transmit Buffer Empty
 * D6	Read Data Available
 * D5	Interrupt Pending
 * D4	Start Bit Detected
 * D3	Full Bit Detected
 * D2	Serial Rcv
 * D1	Overrun Error
 * D0	Frame Error
 */
BYTE cromemco_tuart_0a_status_in(void)
{
	BYTE status = 4;

	if (uart0a_tbe)
		status |= 128;

	if (uart0a_rda)
		status |= 64;

	if (uart0a_int_pending)
		status |= 32;

	return(status);
}

/*
 * D7	Stop Bits	high=1 low=2
 * D6	9600		A high one of the lower seven bits selects the
 * D5	4800		corresponding baud rate. If more than one bit is high,
 * D4	2400		the highest rate selected will result. If none of the
 * D3	1200		bits are high, the serial transmitter and receiver will
 * D2	300		be disabled.
 * D1	150
 * D0	110
 */
void cromemco_tuart_0a_baud_out(BYTE data)
{
	data = data;	/* to avoid compiler warning */
}

BYTE cromemco_tuart_0a_data_in(void)
{
	BYTE data;
	static BYTE last;
	struct pollfd p[1];

	uart0a_rda = 0;

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
	last = data;
	return(data);
}

void cromemco_tuart_0a_data_out(BYTE data)
{
	data &= 0x7f;
	uart0a_tbe = 0;
	if (data == 0x00)
		return;

again:
	if (write(fileno(stdout), (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write tu-art 0a data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

/*
 * D7	Not Used
 * D6	Not Used
 * D5	Test
 * D4	High Baud
 * D3	INTA Enable
 * D2	RST7 Select
 * D1	Break
 * D0	Reset
 */
void cromemco_tuart_0a_command_out(BYTE data)
{
	uart0a_rst7 = (data & 4) ? 1 : 0;

	if (data & 1) {
		uart0a_rda = 0;
		uart0a_tbe = 1;
		uart0a_timer1 = 0;
		uart0a_timer2 = 0;
		uart0a_timer3 = 0;
		uart0a_timer4 = 0;
		uart0a_timer5 = 0;
		uart0a_int_pending = 0;
	}
}

/*
 * C7	Timer 1
 * CF	Timer 2
 * D7	!Sens
 * DF	Timer 3
 * E7	Receiver Data Available
 * EF	Transmitter Buffer Supply
 * F7	Timer 4
 * FF	Timer 5 or PI7
 */
BYTE cromemco_tuart_0a_interrupt_in(void)
{
	return((BYTE) uart0a_int);
}

/*
 * D7	Timer 5 or PI7
 * D6	Timer 4
 * D5	TBE
 * D4	RDA
 * D3	Timer 3
 * D2	!Sens
 * D1	Timer 2
 * D0	Timer 1
 */
void cromemco_tuart_0a_interrupt_out(BYTE data)
{
	uart0a_int_mask = data;
}

/*
 *	The parallel port is used on the FDC's
 *	for auxiliary disk control/status,
 *	so don't implement something here.
 */

BYTE cromemco_tuart_0a_parallel_in(void)
{
	return((BYTE) 0);
}

void cromemco_tuart_0a_parallel_out(BYTE data)
{
	data = data;	/* to avoid compiler warning */
}

void cromemco_tuart_0a_timer1_out(BYTE data)
{
	uart0a_timer1 = data;
}

void cromemco_tuart_0a_timer2_out(BYTE data)
{
	uart0a_timer2 = data;
}

void cromemco_tuart_0a_timer3_out(BYTE data)
{
	uart0a_timer3 = data;
}

void cromemco_tuart_0a_timer4_out(BYTE data)
{
	uart0a_timer4 = data;
}

void cromemco_tuart_0a_timer5_out(BYTE data)
{
	uart0a_timer5 = data;
}

/************************/
/*	Device 1A	*/
/************************/

int uart1a_int_mask;
int uart1a_int_mask, uart1a_int, uart1a_int_pending;
int uart1a_sense, uart1a_lpt_busy;
int uart1a_tbe, uart1a_rda;

BYTE cromemco_tuart_1a_status_in(void)
{
	BYTE status = (ncons[0].ssc) ? 4 : 0;

	if (uart1a_tbe)
		status |= 128;

	if (uart1a_rda)
		status |= 64;

	if (uart1a_int_pending)
		status |= 32;

	return(status);
}

void cromemco_tuart_1a_baud_out(BYTE data)
{
	data = data;	/* to avoid compiler warning */
}

BYTE cromemco_tuart_1a_data_in(void)
{
	BYTE data, dummy;
	static BYTE last;
	struct pollfd p[1];

	uart1a_rda = 0;

	/* if not connected return last */
	if (ncons[0].ssc == 0)
		return(last);

	/* if no input waiting return last */
	p[0].fd = ncons[0].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return(last);

	if (read(ncons[0].ssc, &data, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			/* EOF, close socket and return last */
			close(ncons[0].ssc);
			ncons[0].ssc = 0;
			return(last);
		} else {
			perror("read tu-art 1a data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			return(0);
		}
	}

	/* process read data */
	/* telnet client sends \r\n or \r\0, drop second character */
	if (ncons[0].telnet && (data == '\r'))
		read(ncons[0].ssc, &dummy, 1);
	last = data;
	return(data);
}

void cromemco_tuart_1a_data_out(BYTE data)
{
	uart1a_tbe = 0;
	if (ncons[0].ssc == 0)
		return;
	data &= 0x7f;
	if (data == 0x00)
		return;

again:
	if (write(ncons[0].ssc, (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write tu-art 1a data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

void cromemco_tuart_1a_command_out(BYTE data)
{
	if (data & 1) {
		uart1a_rda = 0;
		uart1a_tbe = 1;
		uart1a_int_pending = 0;
	}
}

BYTE cromemco_tuart_1a_interrupt_in(void)
{
	return((BYTE) uart1a_int);
}

void cromemco_tuart_1a_interrupt_out(BYTE data)
{
	uart1a_int_mask = data;
}

BYTE cromemco_tuart_1a_parallel_in(void)
{
	if (uart1a_lpt_busy == 0)
		return((BYTE) ~0x20);
	else
		return((BYTE) 0xff);
}

void cromemco_tuart_1a_parallel_out(BYTE data)
{
	extern int lpt2;

	if (lpt2 == 0)
		lpt2 = creat("lpt2.txt", 0664);

	uart1a_sense = 1;
	uart1a_lpt_busy = 1;

	/* bit 7 is strobe, every byte is send 3 times. First with
	   strobe on, then with strobe off, then with on again.
	   Take over data when strobe not set */

	if (!(data & 0x80) && (data != '\r')) {
again:
		if (write(lpt2, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				perror("write lpt2.txt");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}
	}
}

/************************/
/*	Device 1B	*/
/************************/

int uart1b_int_mask, uart1b_int, uart1b_int_pending;
int uart1b_sense, uart1b_lpt_busy;
int uart1b_tbe, uart1b_rda;

BYTE cromemco_tuart_1b_status_in(void)
{
	BYTE status = (ncons[1].ssc) ? 4 : 0;

	if (uart1b_tbe)
		status |= 128;

	if (uart1b_rda)
		status |= 64;

	if (uart1b_int_pending)
		status |= 32;

	return(status);
}

void cromemco_tuart_1b_baud_out(BYTE data)
{
	data = data;	/* to avoid compiler warning */
}

BYTE cromemco_tuart_1b_data_in(void)
{
	BYTE data, dummy;
	static BYTE last;
	struct pollfd p[1];

	uart1b_rda = 0;

	/* if not connected return last */
	if (ncons[1].ssc == 0)
		return(last);

	/* if no input waiting return last */
	p[0].fd = ncons[1].ssc;
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return(last);

	if (read(ncons[1].ssc, &data, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			/* EOF, close socket and return last */
			close(ncons[1].ssc);
			ncons[1].ssc = 0;
			return(last);
		} else {
			perror("read tu-art 1b data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			return(0);
		}
	}

	/* process read data */
	/* telnet client sends \r\n or \r\0, drop second character */
	if (ncons[1].telnet && (data == '\r'))
		read(ncons[1].ssc, &dummy, 1);
	last = data;
	return(data);
}

void cromemco_tuart_1b_data_out(BYTE data)
{
	uart1b_tbe = 0;
	if (ncons[1].ssc == 0)
		return;
	data &= 0x7f;
	if (data == 0x00)
		return;

again:
	if (write(ncons[1].ssc, (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write tu-art 1b data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
}

void cromemco_tuart_1b_command_out(BYTE data)
{
	if (data & 1) {
		uart1b_rda = 0;
		uart1b_tbe = 1;
		uart1b_int_pending = 0;
	}
}

BYTE cromemco_tuart_1b_interrupt_in(void)
{
	return((BYTE) uart1b_int);
}

void cromemco_tuart_1b_interrupt_out(BYTE data)
{
	uart1b_int_mask = data;
}

BYTE cromemco_tuart_1b_parallel_in(void)
{
	if (uart1b_lpt_busy == 0)
		return((BYTE) ~0x20);
	else
		return((BYTE) 0xff);
}

void cromemco_tuart_1b_parallel_out(BYTE data)
{
	extern int lpt1;

	if (lpt1 == 0)
		lpt1 = creat("lpt1.txt", 0664);

	uart1b_sense = 1;
	uart1b_lpt_busy = 1;

	/* bit 7 is strobe, every byte is send 3 times. First with
	   strobe on, then with strobe off, then with on again.
	   Take over data when strobe not set */

	if (!(data & 0x80) && (data != '\r')) {
again:
		if (write(lpt1, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				perror("write lpt1.txt");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}
	}
}
