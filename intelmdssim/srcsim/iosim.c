/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module of the simulator contains the I/O simulation
 * for an Intel Intellec MDS-800 system
 *
 * History:
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "config.h"
#include "mds-monitor.h"
#include "mds-isbc202.h"
#include "log.h"

extern BYTE boot_switch;

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static BYTE io_trap_in_00(void);
static BYTE int_mask_in(void), rtc_in(void);
static void int_mask_out(BYTE), int_revert_out(BYTE);
static void bus_ovrrd_out(BYTE), rtc_out(BYTE);

/*
 *	Forward declarations for support functions
 */
static void *timing(void *);

static const char *TAG = "IO";

/*
 *	Interrupt assignments:
 *	INT 1 = Real time clock
 *	INT 2 = iSBC-202 diskette controller
 *	INT 3 = Monitor module
 */

static BYTE int_mask;

static int bus_override;
static int rtc_int_enabled;
static int rtc_int_req;			/* RTC interrupt request */
static int rtc_status1, rtc_status0;	/* RTC status flip-flops */

static int th_suspend;			/* timing thread suspend flag */

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
BYTE (*port_in[256])(void) = {
	[120] = isbc202_status_in,	/* iSBC 202 subsystem status input */
	[121] = isbc202_res_type_in,	/* iSBC 202 result type input */
	[123] = isbc202_res_byte_in,	/* iSBC 202 result byte input */
	[240] = mds_prom_data_in,	/* PROM interface data input */
	[241] = mds_prom_status_in,	/* PROM interface status input */
	[244] = mds_tty_data_in,	/* TTY port data input */
	[245] = mds_tty_status_in,	/* TTY port status input */
	[246] = mds_crt_data_in,	/* CRT port data input */
	[247] = mds_crt_status_in,	/* CRT port status input */
	[248] = mds_ptr_data_in,	/* PTR port data input */
	[249] = mds_pt_status_in,	/* PTR/PTP port status input */
	[250] = mds_int_status_in,	/* Interrupt status input */
	[251] = mds_lpt_status_in,	/* LPT port status input */
	[252] = int_mask_in,		/* Read the interrupt mask */
	[255] = rtc_in			/* Read boot switch and RTC status */
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
void (*port_out[256])(BYTE) = {
	[121] = isbc202_iopbl_out,	/* iSBC 202 IOPB address LSB output */
	[122] = isbc202_iopbh_out,	/* iSBC 202 IOPB address MSB output */
	[127] = isbc202_reset_out,	/* iSBC 202 reset disk system output */
	[240] = mds_prom_data_out,	/* PROM interface data output */
	[241] = mds_prom_high_ctl_out,	/* PROM intf MSB addr and ctl output */
	[242] = mds_prom_low_out,	/* PROM interface LSB addr output */
	[243] = mds_int_ctl_out,	/* Interrupt control output */
	[244] = mds_tty_data_out,	/* TTY port data output */
	[245] = mds_tty_ctl_out,	/* TTY port control output */
	[246] = mds_crt_data_out,	/* CRT port data output */
	[247] = mds_crt_ctl_out,	/* CRT port control output */
	[248] = mds_ptp_data_out,	/* PTP port data output */
	[249] = mds_pt_ctl_out,		/* PTR/PTP port control output */
	[250] = mds_lpt_data_out,	/* LPT port data output */
	[251] = mds_lpt_ctl_out,	/* LPT port control output */
	[252] = int_mask_out,		/* Define and store interrupt mask */
	[253] = int_revert_out,		/* Restore interrupt priority level */
	[254] = bus_ovrrd_out,		/* Override loss of the bus */
	[255] = rtc_out			/* Set RTC interrupt enabled/request */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	extern void io_trap_out(BYTE);

	register int i;
	pthread_t thread;

	for (i = 0; i <= 255; i++) {
		if (port_in[i] == NULL)
			port_in[i] = io_trap_in_00;
		if (port_out[i] == NULL)
			port_out[i] = io_trap_out;
	}

	/* create the thread for timer and interrupt handling */
	if (pthread_create(&thread, NULL, timing, (void *) NULL)) {
		LOGE(TAG, "can't create timing thread");
		exit(EXIT_FAILURE);
	}
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
}

/*
 *	This function is to reset the I/O devices. It is
 *	called from the CPU simulation when an External Clear is performed.
 */
void reset_io(void)
{
	th_suspend = 1;		/* suspend timing thread */
	SLEEP_MS(20);		/* give it enough time to suspend */
	int_mask = 0;
	bus_override = 0;
	rtc_int_enabled = 0;
	rtc_int_req = 0;
	rtc_status1 = rtc_status0 = 0;
	th_suspend = 0;		/* resume timing thread */
}

/*
 *	I/O input trap function
 *	This function should be added into all unused
 *	entries of the input port array. It can stop the
 *	emulation with an I/O error.
 */
static BYTE io_trap_in_00(void)
{
	if (i_flag) {
		cpu_error = IOTRAPIN;
		cpu_state = STOPPED;
	}
	return ((BYTE) 0x00);
}

/*
 *	Read the interrupt mask
 */
static BYTE int_mask_in(void)
{
	return (int_mask);
}

/*
 *	Read boot switch and RTC status bit
 */
static BYTE rtc_in(void)
{
	BYTE val = 0;

	if (boot_switch)
		val |= 0x02;
	if (rtc_status1)
		val |= 0x01;
	rtc_status1 = rtc_status0;
	rtc_status0 = 0;
	return (val);
}

/*
 *	Define and store interrupt mask
 */
static void int_mask_out(BYTE data)
{
	int_mask = data;
}

/*
 *	Restore interrupt priority level
 */
static void int_revert_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Override loss of the bus
 */
static void bus_ovrrd_out(BYTE data)
{
	bus_override = data & 1;
}

/*
 *	Set RTC interrupt enabled and request bits
 */
static void rtc_out(BYTE data)
{
	rtc_int_enabled = ((data & 0x01) == 0);
	if (data & 0x02)
		rtc_int_req = 0;
}

/*
 *	Request an interrupt at level int_level
 */
void int_request(int int_level)
{
	if (int_mask & (1 << int_level))
		return;
}

/*
 *	Acknowledge last interrupt
 */
void int_acknowledge(void)
{
}

/*
 *	Thread for timing and interrupts
 */
static void *timing(void *arg)
{
	UNUSED(arg);

	while (1) {	/* 1 msec per loop iteration */

		/* do nothing if thread is suspended */
		if (th_suspend)
			goto next;

		/* if last interrupt not acknowledged by CPU no new one yet */
		if (int_int)
			goto next;

		/* 1ms RTC */
		rtc_status0 = 1;
		if (rtc_int_enabled)
			rtc_int_req = 1;

next:
		/* sleep for 1 millisecond */
		SLEEP_MS(1);
	}

	/* never reached, this thread is running endless */
	pthread_exit(NULL);
}
