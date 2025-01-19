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
 * 07-JUN-2024 rewrite of the monitor ports and the timing thread
 * 09-JUN-2024 add hwctl and simbdos ports
 */

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcfg.h"
#include "simfun.h"
#include "simmem.h"
#include "simctl.h"
#include "simport.h"
#include "simio.h"

#include "mds-monitor.h"
#include "mds-isbc201.h"
#include "mds-isbc202.h"
#include "mds-isbc206.h"
#include "simbdos.h"
#include "unix_network.h"
#ifdef WANT_ICE
#include "unix_terminal.h"
#endif

#ifdef FRONTPANEL
#include "frontpanel.h"
#endif

#include "log.h"
static const char *TAG = "IO";

#define RTC_IRQ		1	/* real time clock interrupt */

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static BYTE int_mask_in(void), rtc_in(void), hwctl_in(void);
static void int_mask_out(BYTE data), int_revert_out(BYTE data);
static void bus_ovrrd_out(BYTE data), rtc_out(BYTE data), hwctl_out(BYTE data);

/*
 *	Forward declarations for support functions
 */
static void *timing(void *arg);
static void interrupt(int sig);

BYTE int_requests;			/* interrupt requests */
static BYTE int_mask;			/* interrupt mask */
static BYTE int_in_service;		/* interrupts in service */
static pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t rtc_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool rtc_int_enabled;
static bool rtc_status1, rtc_status0;	/* RTC status flip-flops */

static bool th_suspend;			/* RTC/interrupt thread suspend flag */

int lpt_fd;				/* fd for file "printer.txt" */
net_connector_t ncons[NUMNSOC];		/* network connection for TTY */
unix_connector_t ucons[NUMUSOC];	/* socket connection for PTR/PTP */
static BYTE hwctl_lock = 0xff;		/* lock status hardware control port */

/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
in_func_t *const port_in[256] = {
#ifdef HAS_ISBC206
	[104] = isbc206_status_in,	/* iSBC 206 subsystem status input */
	[105] = isbc206_res_type_in,	/* iSBC 206 result type input */
	[107] = isbc206_res_byte_in,	/* iSBC 206 result byte input */
#endif
#ifdef HAS_ISBC202
	[120] = isbc202_status_in,	/* iSBC 202 subsystem status input */
	[121] = isbc202_res_type_in,	/* iSBC 202 result type input */
	[123] = isbc202_res_byte_in,	/* iSBC 202 result byte input */
#endif
#ifdef HAS_ISBC201
	[136] = isbc201_status_in,	/* iSBC 201 subsystem status input */
	[137] = isbc201_res_type_in,	/* iSBC 201 result type input */
	[139] = isbc201_res_byte_in,	/* iSBC 201 result byte input */
#endif
	[160] = hwctl_in,		/* virtual hardware control */
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
out_func_t *const port_out[256] = {
#ifdef HAS_ISBC206
	[105] = isbc206_iopbl_out,	/* iSBC 206 IOPB address LSB output */
	[106] = isbc206_iopbh_out,	/* iSBC 206 IOPB address MSB output */
	[111] = isbc206_reset_out,	/* iSBC 206 reset disk system output */
#endif
#ifdef HAS_ISBC202
	[121] = isbc202_iopbl_out,	/* iSBC 202 IOPB address LSB output */
	[122] = isbc202_iopbh_out,	/* iSBC 202 IOPB address MSB output */
	[127] = isbc202_reset_out,	/* iSBC 202 reset disk system output */
#endif
#ifdef HAS_ISBC201
	[137] = isbc201_iopbl_out,	/* iSBC 201 IOPB address LSB output */
	[138] = isbc201_iopbh_out,	/* iSBC 201 IOPB address MSB output */
	[143] = isbc201_reset_out,	/* iSBC 201 reset disk system output */
#endif
	[160] = hwctl_out,		/* virtual hardware control */
	[161] = host_bdos_out,		/* host file I/O hook */
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
	register int i;
	pthread_t thread;
	static struct itimerval tim;
	static struct sigaction newact;

	int_mask = 0;		/* reset interrupt facility */
	int_requests = 0;
	int_in_service = 0;

	rtc_int_enabled = false; /* reset real time clock */
	rtc_status0 = false;
	rtc_status1 = false;

	mon_reset();		/* reset monitor module */
#ifdef HAS_ISBC201
	isbc201_reset();	/* reset iSBC 201 disk controller */
#endif
#ifdef HAS_ISBC202
	isbc202_reset();	/* reset iSBC 202 disk controller */
#endif
#ifdef HAS_ISBC206
	isbc206_reset();	/* reset iSBC 206 disk controller */
#endif

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
	th_suspend = true;	/* suspend timing thread */
	sleep_for_ms(20);	/* give it enough time to suspend */

	pthread_mutex_lock(&rtc_mutex);
	rtc_int_enabled = false; /* reset real time clock */
	rtc_status0 = false;
	rtc_status1 = false;
	pthread_mutex_unlock(&rtc_mutex);

	pthread_mutex_lock(&int_mutex);
	int_mask = 0;		/* reset interrupt facility */
	int_requests = 0;
	int_in_service = 0;
	pthread_mutex_unlock(&int_mutex);

	mon_reset();		/* reset monitor module */
#ifdef HAS_ISBC201
	isbc201_reset();	/* reset iSBC 201 disk controller */
#endif
#ifdef HAS_ISBC202
	isbc202_reset();	/* reset iSBC 202 disk controller */
#endif
#ifdef HAS_ISBC206
	isbc206_reset();	/* reset iSBC 206 disk controller */
#endif

	th_suspend = false;	/* resume timing thread */
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
	return irq;
}

/*
 *	Check for pending interrupts
 */
static void int_pending(void)
{
	int irq, mask;

	if (!int_int && (mask = (int_requests & ~int_mask)) != 0) {
		irq = int_highest(mask);
		if (irq < int_highest(int_in_service)) {
			pthread_mutex_lock(&int_mutex);
			int_in_service |= 1 << irq;
			int_requests &= ~(1 << irq);
			int_int = true;
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
	return int_mask;
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
	BYTE data = 0;

	if (boot_switch)
		data |= 0x02;
	if (rtc_status1)
		data |= 0x01;
	pthread_mutex_lock(&rtc_mutex);
	rtc_status1 = rtc_status0;
	rtc_status0 = false;
	pthread_mutex_unlock(&rtc_mutex);
	return data;
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
 *	Input from virtual hardware control port
 *	returns lock status of the port
 */
static BYTE hwctl_in(void)
{
	return hwctl_lock;
}

/*
 *	Port is locked until magic number 0xaa is received!
 *
 *	Virtual hardware control output.
 *	Doesn't exist in the real machine, used to shutdown.
 *
 *	bit 7 = 1       halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
	/* if port is locked do nothing */
	if (hwctl_lock && (data != 0xaa))
		return;

	/* unlock port ? */
	if (hwctl_lock && (data == 0xaa)) {
		hwctl_lock = 0;
		return;
	}

	/* process output to unlocked port */
	/* but first lock port again */
	hwctl_lock = 0xff;

	if (data & 128) {	/* halt system */
		cpu_error = IOHALT;
		cpu_state = ST_STOPPED;
	}
}

/*
 *	Thread for timing and interrupts
 */
static void *timing(void *arg)
{
	uint64_t t, tick;
	int64_t tdiff;

	UNUSED(arg);

	tick = 0;

	while (true) {	/* 260 usec per loop iteration */

		t = get_clock_us();

		/* do nothing if thread is suspended */
		if (th_suspend)
			goto next;

		/* check monitor ports */
		if (tick % tty_clock_div == 0)
			mon_tty_periodic();
		if (tick % crt_clock_div == 0)
			mon_crt_periodic();
		if (tick % pt_clock_div == 0)
			mon_pt_periodic();
		if (tick % lpt_clock_div == 0)
			mon_lpt_periodic();

		/* 0.9765ms RTC (here 1.04 ms) */
		if (tick % 4 == 0) {
			pthread_mutex_lock(&rtc_mutex);
			rtc_status0 = true;
			pthread_mutex_unlock(&rtc_mutex);
			if (rtc_int_enabled)
				int_request(RTC_IRQ);
		}

		/* check for pending interrupts */
		int_pending();

next:
		tdiff = 260L - (get_clock_us() - t);
		if (tdiff > 0)
			sleep_for_us(tdiff);

		tick++;
	}

	/* never reached, this thread is running endless */
	pthread_exit(NULL);
}

/*
 *	10ms interrupt handler
 */
static void interrupt(int sig)
{
	static unsigned long counter = 0L;

	UNUSED(sig);

	counter++;

#ifndef TCPASYNC
	/* poll TCP sockets if SIGIO not working */
	sigio_tcp_server_socket(0);
#endif

	/* check disk image files each second */
#ifdef HAS_ISBC201
	if ((counter % 100) == 0)
		isbc201_disk_check();
#endif
#ifdef HAS_ISBC202
	if ((counter % 100) == 33)
		isbc202_disk_check();
#endif
#ifdef HAS_ISBC206
	if ((counter % 100) == 66)
		isbc206_disk_check();
#endif

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
#endif /* WANT_ICE */
