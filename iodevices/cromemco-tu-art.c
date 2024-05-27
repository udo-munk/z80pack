/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 Udo Munk
 *
 * Emulation of a Cromemco TU-ART S100 board
 *
 * History:
 *    DEC-2014 first version
 *    JAN-2015 better subdue of non printable characters in output
 * 02-FEB-2015 implemented the timers and interrupt flag for TBE
 * 05-FEB-2015 implemented interrupt flag for RDA
 * 14-FEB-2015 improvements, so that the Cromix tty driver works
 * 10-MAR-2015 lpt's implemented for CP/M, CDOS and Cromix
 * 23-MAR-2015 drop only null's
 * 26-MAR-2015 tty's implemented for CDOS and Cromix
 * 25-APR-2018 cleanup
 * 03-MAY-2018 improved accuracy
 * 15-JUL-2018 use logging
 * 06-SEP-2021 implement reset
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"
#include "unix_terminal.h"
#include "unix_network.h"
#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif

#include "cromemco-hal.h"

static const char *TAG = "TU-ART";

void lpt_reset(void)
{
	extern int lpt1;

	if (lpt1) {
		close(lpt1);
	}
	if ((lpt1 = creat("lpt1.txt", 0664)) == -1) {
		LOGE(TAG, "can't create lpt1.txt");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
		lpt1 = 0;
	}
}

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

	return (status);
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
	UNUSED(data);
}

BYTE cromemco_tuart_0a_data_in(void)
{
	int data;
	static BYTE last;

	uart0a_rda = 0;

	data = hal_data_in(TUART0A);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	/* process read data */
	last = data;
	return ((BYTE) data);
}

void cromemco_tuart_0a_data_out(BYTE data)
{
	data &= 0x7f;
	uart0a_tbe = 0;
	if (data == 0x00)
		return;

	hal_data_out(TUART0A, data);
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
	return ((BYTE) uart0a_int);
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
	return ((BYTE) 0);
}

void cromemco_tuart_0a_parallel_out(BYTE data)
{
	UNUSED(data);
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

int uart1a_int_mask, uart1a_int, uart1a_int_pending;
int uart1a_sense, uart1a_lpt_busy;
int uart1a_tbe, uart1a_rda;

BYTE cromemco_tuart_1a_status_in(void)
{
	BYTE status = 0;

	status = (hal_alive(TUART1A)) ? 4 : 0;

	if (uart1a_tbe)
		status |= 128;

	if (uart1a_rda)
		status |= 64;

	if (uart1a_int_pending)
		status |= 32;

	return (status);
}

void cromemco_tuart_1a_baud_out(BYTE data)
{
	UNUSED(data);
}

BYTE cromemco_tuart_1a_data_in(void)
{
	int data;
	static BYTE last;

	uart1a_rda = 0;

	data = hal_data_in(TUART1A);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	last = data;
	return ((BYTE) data);
}

void cromemco_tuart_1a_data_out(BYTE data)
{
	uart1a_tbe = 0;
	data &= 0x7f;
	if (data == 0x00)
		return;

	hal_data_out(TUART1A, data);
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
	return ((BYTE) uart1a_int);
}

void cromemco_tuart_1a_interrupt_out(BYTE data)
{
	uart1a_int_mask = data;
}

BYTE cromemco_tuart_1a_parallel_in(void)
{
	if (uart1a_lpt_busy == 0)
		return ((BYTE) ~0x20);
	else
		return ((BYTE) 0xff);
}

void cromemco_tuart_1a_parallel_out(BYTE data)
{
	extern int lpt2;

	if (lpt2 == 0) {
		if ((lpt2 = creat("lpt2.txt", 0664)) == -1) {
			LOGE(TAG, "can't create lpt2.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			lpt2 = 0;
			return;
		}
	}

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
				LOGE(TAG, "can't write to lpt2.txt");
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
	BYTE status = 0;

	status = (hal_alive(TUART1B)) ? 4 : 0;

	if (uart1b_tbe)
		status |= 128;

	if (uart1b_rda)
		status |= 64;

	if (uart1b_int_pending)
		status |= 32;

	return (status);
}

void cromemco_tuart_1b_baud_out(BYTE data)
{
	UNUSED(data);
}

BYTE cromemco_tuart_1b_data_in(void)
{
	int data;
	static BYTE last;

	uart1b_rda = 0;

	data = hal_data_in(TUART1B);
	/* if no new data available return last */
	if (data < 0) {
		return last;
	}

	last = data;
	return ((BYTE) data);
}

void cromemco_tuart_1b_data_out(BYTE data)
{
	uart1b_tbe = 0;
	data &= 0x7f;
	if (data == 0x00)
		return;

	hal_data_out(TUART1B, data);
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
	return ((BYTE) uart1b_int);
}

void cromemco_tuart_1b_interrupt_out(BYTE data)
{
	uart1b_int_mask = data;
}

BYTE cromemco_tuart_1b_parallel_in(void)
{
	if (uart1b_lpt_busy == 0)
		return ((BYTE) ~0x20);
	else
		return ((BYTE) 0xff);
}

void cromemco_tuart_1b_parallel_out(BYTE data)
{
	extern int lpt1;

	if (lpt1 == 0) {
		if ((lpt1 = creat("lpt1.txt", 0664)) == -1) {
			LOGE(TAG, "can't create lpt1.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			lpt1 = 0;
			return;
		}
	}

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
				LOGE(TAG, "can't write to lpt1.txt");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}

#ifdef HAS_NETSERVER
		if (n_flag)
			net_device_send(DEV_LPT, (char *) &data, 1);
#endif
	}
}

/*
 * reset all TU-ART
 */
void cromemco_tuart_reset(void)
{
	uart0a_int = 0xff;
	uart0a_int_mask = 0;
	uart0a_int_pending = 0;
	uart0a_rda = 0;
	uart0a_tbe = 1;
	uart0a_timer1 = uart0a_timer2 = uart0a_timer3 = 0;
	uart0a_timer4 = uart0a_timer5 = 0;
	uart0a_rst7 = 0;

	uart1a_int = 0xff;
	uart1a_int_mask = 0;
	uart1a_int_pending = 0;
	uart1a_rda = 0;
	uart1a_tbe = 1;

	uart1b_int = 0xff;
	uart1b_int_mask = 0;
	uart1b_int_pending = 0;
	uart1b_rda = 0;
	uart1b_tbe = 1;
}
