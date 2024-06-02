/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module of the simulator contains the I/O simulation
 * for an Intel Intellec MDS-800 system.
 * It maintains the real time clock and interrupt facility.
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

#define RTC_IRQ		1	/* Real time clock interrupt */

#ifdef FRONTPANEL
extern BYTE int_sw_requests;
#endif

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
static void *rtc_int_thread(void *);

static const char *TAG = "IO";

/*
 *	Interrupt assignments:
 *	INT 1 = Real time clock
 *	INT 2 = iSBC-202 diskette controller
 *	INT 3 = Monitor module
 */

static BYTE int_mask;			/* interrupt mask */
static BYTE int_requests;		/* interrupt requests */
static BYTE int_in_service;		/* interrupts in service */

static int rtc_int_enabled;
static int rtc_status1, rtc_status0;	/* RTC status flip-flops */

static int th_suspend;			/* RTC/interrupt thread suspend flag */

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
	if (pthread_create(&thread, NULL, rtc_int_thread, (void *) NULL)) {
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

	int_mask = 0;		/* reset interrupt facility */
	int_requests = 0;
	int_in_service = 0;

	rtc_int_enabled = 0;	/* reset real time clock */
	rtc_status0 = 0;
	rtc_status1 = 0;

	th_suspend = 0;		/* resume timing thread */
}

/*
 *	I/O input trap function
 *	This function should be added into all unused
 *	entries of the input port array. It can stop the
 *	emulation with an I/O error.
 *	It returns 0x00 instead of io_trap_in()'s 0xff.
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
 *	Get highest priority interrupt from mask
 */
static int int_highest(int mask)
{
	int irq, irq_mask;

	if (mask == 0)
		return 8;
	irq = 0;
	irq_mask = 0x01;
	while ((mask & irq_mask) == 0) {
		irq++;
		irq_mask <<= 1;
	}
	return (irq);
}

/*
 *	Check for pending interrupts
 */
static void int_pending(void)
{
	int irq, mask;

	if (int_int == 0 && (mask = (int_requests & ~int_mask)) != 0) {
		irq = int_highest(mask);
		if (irq < int_highest(int_in_service)) {
#ifdef FRONTPANEL
			if (F_flag)
				int_sw_requests &= ~(1 << irq);
#endif
			int_in_service |= 1 << irq;
			int_requests &= ~(1 << irq);
			int_int = 1;
			int_data = 0xc7 /* RST0 */ + (irq << 3);
		}
	}
}

/*
 *	Request an interrupt
 */
void int_request(int irq)
{
	int_requests |= 1 << irq;
	int_pending();
}

/*
 *	Cancel an interrupt
 */
void int_cancel(int irq)
{
	int_requests &= ~(1 << irq);
	int_pending();
}

/*
 *	Read the interrupt mask
 */
static BYTE int_mask_in(void)
{
	return (int_mask);
}

/*
 *	Define and store interrupt mask
 */
static void int_mask_out(BYTE data)
{
	int_mask = data;
	int_pending();
}

/*
 *	Restore interrupt priority level
 */
static void int_revert_out(BYTE data)
{
	int irq;
	UNUSED(data);

	if (int_in_service != 0) {
		irq = int_highest(int_in_service);
		int_in_service &= ~(1 << irq);
		int_pending();
	}
}

/*
 *	Override loss of the bus
 *	(Not implemented)
 */
static void bus_ovrrd_out(BYTE data)
{
	UNUSED(data);
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
 *	Set RTC interrupt enabled and request bits
 */
static void rtc_out(BYTE data)
{
	rtc_int_enabled = ((data & 0x01) == 0);
	if (data & 0x02)
		int_cancel(RTC_IRQ);
}

/*
 *	Thread for real time clock and interrupts
 */
static void *rtc_int_thread(void *arg)
{
	UNUSED(arg);

	while (1) {	/* 1 msec per loop iteration */

		/* Do nothing if thread is suspended */
		if (th_suspend)
			goto next;

		/* If last interrupt not acknowledged by CPU no new one yet */
		if (int_int)
			goto next;

		int_pending();

		/* 1ms RTC */
		rtc_status0 = 1;
		if (rtc_int_enabled)
			int_request(RTC_IRQ);

#ifdef FRONTPANEL
		if (!F_flag) {
#endif
			/*
			 * HACK ALERT! Turn off boot switch if in
			 * "bootstrap mode disabled check" loop.
			 */
			if (boot_switch) {
				if (0xf830 <= PC && PC <= 0xf836)
					boot_switch = 0;
			}
#ifdef FRONTPANEL
		}
#endif

next:
		/* Sleep for 1 millisecond */
		SLEEP_MS(1);
	}

	/* Never reached, this thread is running endless */
	pthread_exit(NULL);
}
