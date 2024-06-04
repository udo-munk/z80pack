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
 * 03-JUN-2024 first version
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "config.h"
#include "mds-monitor.h"
#include "mds-isbc202.h"
#include "unix_network.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "log.h"

#define RTC_IRQ		1	/* real time clock interrupt */

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
static void interrupt(int);

static const char *TAG = "IO";

BYTE int_requests;			/* interrupt requests */
static BYTE int_mask;			/* interrupt mask */
static BYTE int_in_service;		/* interrupts in service */
static pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;

static int rtc_int_enabled;
static int rtc_status1, rtc_status0;	/* RTC status flip-flops */

static int th_suspend;			/* RTC/interrupt thread suspend flag */

int lpt_fd;				/* fd for file "printer.txt" */
struct net_connectors ncons[NUMNSOC];	/* network connection for TTY */
struct unix_connectors ucons[NUMUSOC];	/* socket connection for PTR/PTP */

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
BYTE (*port_in[256])(void) = {
	[120] = isbc202_status_in,	/* iSBC 202 subsystem status input */
	[121] = isbc202_res_type_in,	/* iSBC 202 result type input */
	[123] = isbc202_res_byte_in,	/* iSBC 202 result byte input */
	[240] = mon_prom_data_in,	/* PROM interface data input */
	[241] = mon_prom_status_in,	/* PROM interface status input */
	[244] = mon_tty_data_in,	/* TTY port data input */
	[245] = mon_tty_status_in,	/* TTY port status input */
	[246] = mon_crt_data_in,	/* CRT port data input */
	[247] = mon_crt_status_in,	/* CRT port status input */
	[248] = mon_ptr_data_in,	/* PTR port data input */
	[249] = mon_pt_status_in,	/* PTR/PTP port status input */
	[250] = mon_int_status_in,	/* interrupt status input */
	[251] = mon_lpt_status_in,	/* LPT port status input */
	[252] = int_mask_in,		/* read the interrupt mask */
	[255] = rtc_in			/* read boot switch and RTC status */
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
void (*port_out[256])(BYTE) = {
	[121] = isbc202_iopbl_out,	/* iSBC 202 IOPB address LSB output */
	[122] = isbc202_iopbh_out,	/* iSBC 202 IOPB address MSB output */
	[127] = isbc202_reset_out,	/* iSBC 202 reset disk system output */
	[240] = mon_prom_data_out,	/* PROM interface data output */
	[241] = mon_prom_high_ctl_out,	/* PROM intf MSB addr and ctl output */
	[242] = mon_prom_low_out,	/* PROM interface LSB addr output */
	[243] = mon_int_ctl_out,	/* interrupt control output */
	[244] = mon_tty_data_out,	/* TTY port data output */
	[245] = mon_tty_ctl_out,	/* TTY port control output */
	[246] = mon_crt_data_out,	/* CRT port data output */
	[247] = mon_crt_ctl_out,	/* CRT port control output */
	[248] = mon_ptp_data_out,	/* PTP port data output */
	[249] = mon_pt_ctl_out,		/* PTR/PTP port control output */
	[250] = mon_lpt_data_out,	/* LPT port data output */
	[251] = mon_lpt_ctl_out,	/* LPT port control output */
	[252] = int_mask_out,		/* define and store interrupt mask */
	[253] = int_revert_out,		/* restore interrupt priority level */
	[254] = bus_ovrrd_out,		/* override loss of the bus */
	[255] = rtc_out			/* set RTC interrupt enabled/request */
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
	static struct itimerval tim;
	static struct sigaction newact;

	for (i = 0; i <= 255; i++) {
		if (port_in[i] == NULL)
			port_in[i] = io_trap_in_00;
		if (port_out[i] == NULL)
			port_out[i] = io_trap_out;
	}

	int_mask = 0;		/* reset interrupt facility */
	int_requests = 0;
	int_in_service = 0;

	rtc_int_enabled = 0;	/* reset real time clock */
	rtc_status0 = 0;
	rtc_status1 = 0;

	mon_reset();		/* reset monitor module */

	isbc202_reset();	/* reset iSBC 202 disk controller */


	/* initialize TCP/IP networking */
#ifdef TCPASYNC
	newact.sa_handler = sigio_tcp_server_socket;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGIO, &newact, NULL);
#endif

	for (i = 0; i < NUMNSOC; i++) {
		ncons[i].port = SERVERPORT + i;
		ncons[i].telnet = 1;
		init_tcp_server_socket(&ncons[i]);
	}

	/* create local socket for PTR/PTP */
	init_unix_server_socket(&ucons[0], "intelmdssim.pt");

	/* create the thread for timer and interrupt handling */
	if (pthread_create(&thread, NULL, timing, (void *) NULL)) {
		LOGE(TAG, "can't create timing thread");
		exit(EXIT_FAILURE);
	}

	/* start 10ms interrupt timer, delayed! */
#ifndef WANT_ICE
	newact.sa_handler = interrupt;
#else
	newact.sa_handler = SIG_IGN;
#endif
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, NULL);
	tim.it_value.tv_sec = 5;
	tim.it_value.tv_usec = 0;
	tim.it_interval.tv_sec = 0;
	tim.it_interval.tv_usec = 10000;
	setitimer(ITIMER_REAL, &tim, NULL);
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
	register int i;

	/* close line printer file */
	if (lpt_fd != 0)
		close(lpt_fd);

	/* close network connections */
	for (i = 0; i < NUMNSOC; i++) {
		if (ncons[i].ssc)
			close(ncons[i].ssc);
	}
	for (i = 0; i < NUMUSOC; i++) {
		if (ucons[i].ssc)
			close(ucons[i].ssc);
	}
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

	mon_reset();		/* reset monitor module */

	isbc202_reset();	/* reset iSBC 202 disk controller */

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
			pthread_mutex_lock(&int_mutex);
			int_in_service |= 1 << irq;
			int_requests &= ~(1 << irq);
			int_int = 1;
			int_data = 0xc7 /* RST0 */ + (irq << 3);
			pthread_mutex_unlock(&int_mutex);
		}
	}
}

/*
 *	Request an interrupt
 */
void int_request(int irq)
{
	pthread_mutex_lock(&int_mutex);
	int_requests |= 1 << irq;
	pthread_mutex_unlock(&int_mutex);
	int_pending();
}

/*
 *	Cancel an interrupt
 */
void int_cancel(int irq)
{
	pthread_mutex_lock(&int_mutex);
	int_requests &= ~(1 << irq);
	pthread_mutex_unlock(&int_mutex);
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
		pthread_mutex_lock(&int_mutex);
		int_in_service &= ~(1 << irq);
		pthread_mutex_unlock(&int_mutex);
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

		/* check for pending interrupts */
		int_pending();

		/* check for monitor port interrupts */
		mon_int_check();

		/* 1ms RTC */
		rtc_status0 = 1;
		if (rtc_int_enabled)
			int_request(RTC_IRQ);

next:
		/* sleep for 1 millisecond */
		SLEEP_MS(1);
	}

	/* never reached, this thread is running endless */
	pthread_exit(NULL);
}

/*
 *	10ms interrupt handler
 */
static void interrupt(int sig)
{
	UNUSED(sig);

#ifndef TCPASYNC
	/* poll TCP sockets if SIGIO not working */
	sigio_tcp_server_socket(0);
#endif

	/* update monitor ports status bits */
	(void) mon_tty_status_in();
	(void) mon_crt_status_in();
	(void) mon_pt_status_in();
	(void) mon_lpt_status_in();

#ifdef FRONTPANEL
	if (!F_flag) {
#endif
		/*
		 * HACK ALERT! Turn off boot switch if
		 * PC is outside bootstrap ROM and TTYOUT/CRTOUT
		 */
		if (boot_switch) {
			if (0xa9 < PC && PC < 0xfd25)
				boot_switch = 0;
		}
#ifdef FRONTPANEL
	}
#endif
}

#ifdef WANT_ICE
/*
 *	Setup and start terminal I/O for the machine
 */
void ice_go(void)
{
	static struct sigaction newact;

	set_unix_terminal();

	newact.sa_handler = interrupt;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, NULL);
}

/*
 *	Give terminal I/O back to ICE
 */
void ice_break(void)
{
	static struct sigaction newact;

	newact.sa_handler = SIG_IGN;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, NULL);

	reset_unix_terminal();
}
#endif
