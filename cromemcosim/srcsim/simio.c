/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2014-2022 Udo Munk
 *
 * This module of the simulator contains the I/O simulation
 * for a Cromemco Z-1 system
 *
 * History:
 * 15-DEC-2014 first version
 * 20-DEC-2014 added 4FDC emulation and machine boots CP/M 2.2
 * 28-DEC-2014 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-2015 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-2015 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-2015 fdc and tu-art improvements, implemented banked memory
 * 02-FEB-2015 modified MMU, implemented timers and interrupts
 * 20-FEB-2015 bug fix for release 1.25
 * 10-MAR-2015 added support for two parallel port lpt devices on TU-ART
 * 26-MAR-2015 added support for two serial port tty devices on TU-ART
 * 29-APR-2015 added Cromemco DAZZLER to the machine
 * 06-DEC-2016 implemented status display and stepping for all machine cycles
 * 22-DEC-2016 moved MMU out to the new memory interface module
 * 15-AUG-2017 modified index pulse handling
 * 22-APR-2018 implemented TCP socket polling
 * 24-APR-2018 cleanup
 * 17-MAY-2018 implemented hardware control
 * 08-JUN-2018 moved hardware initialization and reset to iosim
 * 18-JUL-2018 use logging
 * 08-SEP-2019 bug fixes provided by Alan Cox
 * 08-OCT-2019 (Mike Douglas) added OUT 161 trap to simbdos.c for host file I/O
 * 19-JUL-2020 avoid problems with some third party terminal emulations
 * 17-JUN-2021 allow building machine without frontpanel
 * 29-JUL-2021 add boot config for machine without frontpanel
 * 27-MAY-2024 moved io_in & io_out to simcore
 */

#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "simbdos.h"
#include "unix_network.h"
#include "cromemco-tu-art.h"
#ifdef HAS_MODEM
#include "generic-at-modem.h"
#endif /* HAS_MODEM */
#include "cromemco-fdc.h"
#include "cromemco-dazzler.h"
#include "cromemco-d+7a.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "simmem.h"
#include "simcfg.h"
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
#include "cromemco-hal.h"
#include "cromemco-wdi.h"
#include "unix_terminal.h"

/*
 *	Forward declarations for I/O functions
 */
static BYTE fp_in(void), mmu_in(void);
static void fp_out(BYTE data), mmu_out(BYTE data);
static BYTE hwctl_in(void);
static void hwctl_out(BYTE data);

/*
 *	Forward declarations for support functions
 */
static void *timing(void *);
static void interrupt(int);

static const char *TAG = "IO";

static int rtc;			/* flag for 512ms RTC interrupt */
int lpt1, lpt2;			/* fds for lpt printer files */

static BYTE hwctl_lock = 0xff;	/* lock status hardware control port */

/* network connections for serial ports on the TU-ART's */
struct net_connectors ncons[NUMNSOC];

static int th_suspend;		/* timing thread suspend flag */

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
BYTE (*const port_in[256])(void) = {
	[  0] = cromemco_tuart_0a_status_in,
	[  1] = cromemco_tuart_0a_data_in,
	[  3] = cromemco_tuart_0a_interrupt_in,
	[  4] = cromemco_fdc_aux_in,
	[ 14] = cromemco_dazzler_flags_in,
	[ 24] = cromemco_d7a_D_in,
	[ 25] = cromemco_d7a_A1_in,
	[ 26] = cromemco_d7a_A2_in,
	[ 27] = cromemco_d7a_A3_in,
	[ 28] = cromemco_d7a_A4_in,
	[ 29] = cromemco_d7a_A5_in,
	[ 30] = cromemco_d7a_A6_in,
	[ 31] = cromemco_d7a_A7_in,
	[ 32] = cromemco_tuart_1a_status_in,
	[ 33] = cromemco_tuart_1a_data_in,
	[ 35] = cromemco_tuart_1a_interrupt_in,
	[ 36] = cromemco_tuart_1a_parallel_in,
	[ 48] = cromemco_fdc_status_in,
	[ 49] = cromemco_fdc_track_in,
	[ 50] = cromemco_fdc_sector_in,
	[ 51] = cromemco_fdc_data_in,
	[ 52] = cromemco_fdc_diskflags_in,
	[ 64] = mmu_in,
	[ 80] = cromemco_tuart_1b_status_in,
	[ 81] = cromemco_tuart_1b_data_in,
	[ 83] = cromemco_tuart_1b_interrupt_in,
	[ 84] = cromemco_tuart_1b_parallel_in,
	[160] = hwctl_in,
	[224] = cromemco_wdi_pio0a_data_in,
	[225] = cromemco_wdi_pio0b_data_in,
	[226] = cromemco_wdi_pio0a_cmd_in,
	[227] = cromemco_wdi_pio0b_cmd_in,
	[228] = cromemco_wdi_pio1a_data_in,
	[229] = cromemco_wdi_pio1b_data_in,
	[230] = cromemco_wdi_pio1a_cmd_in,
	[231] = cromemco_wdi_pio1b_cmd_in,
	[232] = cromemco_wdi_dma0_in,
	[233] = cromemco_wdi_dma1_in,
	[234] = cromemco_wdi_dma2_in,
	[235] = cromemco_wdi_dma3_in,
	[236] = cromemco_wdi_ctc0_in,
	[237] = cromemco_wdi_ctc1_in,
	[238] = cromemco_wdi_ctc2_in,
	[239] = cromemco_wdi_ctc3_in,
	[255] = fp_in				/* front panel */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
void (*const port_out[256])(BYTE data) = {
	[  0] = cromemco_tuart_0a_baud_out,
	[  1] = cromemco_tuart_0a_data_out,
	[  2] = cromemco_tuart_0a_command_out,
	[  3] = cromemco_tuart_0a_interrupt_out,
	[  4] = cromemco_fdc_aux_out,
	[  5] = cromemco_tuart_0a_timer1_out,
	[  6] = cromemco_tuart_0a_timer2_out,
	[  7] = cromemco_tuart_0a_timer3_out,
	[  8] = cromemco_tuart_0a_timer4_out,
	[  9] = cromemco_tuart_0a_timer5_out,
	[ 14] = cromemco_dazzler_ctl_out,
	[ 15] = cromemco_dazzler_format_out,
	[ 24] = cromemco_d7a_D_out,
	[ 25] = cromemco_d7a_A1_out,
	[ 26] = cromemco_d7a_A2_out,
	[ 27] = cromemco_d7a_A3_out,
	[ 28] = cromemco_d7a_A4_out,
	[ 29] = cromemco_d7a_A5_out,
	[ 30] = cromemco_d7a_A6_out,
	[ 31] = cromemco_d7a_A7_out,
	[ 32] = cromemco_tuart_1a_baud_out,
	[ 33] = cromemco_tuart_1a_data_out,
	[ 34] = cromemco_tuart_1a_command_out,
	[ 35] = cromemco_tuart_1a_interrupt_out,
	[ 36] = cromemco_tuart_1a_parallel_out,
	[ 48] = cromemco_fdc_cmd_out,
	[ 49] = cromemco_fdc_track_out,
	[ 50] = cromemco_fdc_sector_out,
	[ 51] = cromemco_fdc_data_out,
	[ 52] = cromemco_fdc_diskctl_out,
	[ 64] = mmu_out,
	[ 80] = cromemco_tuart_1b_baud_out,
	[ 81] = cromemco_tuart_1b_data_out,
	[ 82] = cromemco_tuart_1b_command_out,
	[ 83] = cromemco_tuart_1b_interrupt_out,
	[ 84] = cromemco_tuart_1b_parallel_out,
	[160] = hwctl_out,
	[161] = host_bdos_out,			/* host file I/O hook */
	[224] = cromemco_wdi_pio0a_data_out,
	[225] = cromemco_wdi_pio0b_data_out,
	[226] = cromemco_wdi_pio0a_cmd_out,
	[227] = cromemco_wdi_pio0b_cmd_out,
	[228] = cromemco_wdi_pio1a_data_out,
	[229] = cromemco_wdi_pio1b_data_out,
	[230] = cromemco_wdi_pio1a_cmd_out,
	[231] = cromemco_wdi_pio1b_cmd_out,
	[232] = cromemco_wdi_dma0_out,
	[233] = cromemco_wdi_dma1_out,
	[234] = cromemco_wdi_dma2_out,
	[235] = cromemco_wdi_dma3_out,
	[236] = cromemco_wdi_ctc0_out,
	[237] = cromemco_wdi_ctc1_out,
	[238] = cromemco_wdi_ctc2_out,
	[239] = cromemco_wdi_ctc3_out,
	[255] = fp_out				/* front panel */
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

	/* initial TU-ART device interrupt address */
	uart0a_int = 0xff;
	uart1a_int = 0xff;
	uart1b_int = 0xff;

	/* create the thread for timer and interrupt handling */
	if (pthread_create(&thread, NULL, timing, (void *) NULL)) {
		LOGE(TAG, "can't create timing thread");
		exit(EXIT_FAILURE);
	}

#ifdef HAS_MODEM
	modem_device_init();
#endif

	hal_reset();
	LOG(TAG, "\r\n");

	wdi_init();

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

	wdi_exit();

	/* close line printer files */
	if (lpt1 != 0)
		close(lpt1);
	if (lpt2 != 0)
		close(lpt2);

	/* close network connections */
	for (i = 0; i < NUMNSOC; i++) {
		if (ncons[i].ssc)
			close(ncons[i].ssc);
	}

	/* shutdown DAZZLER */
	cromemco_dazzler_off();
}

/*
 *	This function is to reset the I/O devices. It is
 *	called from the CPU simulation when an External Clear is performed.
 */
void reset_io(void)
{
	th_suspend = 1;		/* suspend timing thread */
	SLEEP_MS(20);		/* give it enough time to suspend */
	cromemco_tuart_reset();
	cromemco_fdc_reset();
	th_suspend = 0;		/* resume timing thread */
	selbnk = 0;
	cromemco_dazzler_off();
	hwctl_lock = 0xff;
	wdi_exit();
	wdi_init();
}

/*
 *	Read input from front panel switches
 */
static BYTE fp_in(void)
{
#ifdef FRONTPANEL
	if (F_flag)
		return address_switch >> 8;
	else {
#endif
		return fp_port;
#ifdef FRONTPANEL
	}
#endif
}

/*
 *	Write output to front panel lights
 */
static void fp_out(BYTE data)
{
#ifdef FRONTPANEL
	if (F_flag)
		fp_led_output = data;
	else {
#endif
		UNUSED(data);
#ifdef FRONTPANEL
	}
#endif
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
 *	Doesn't exist in the real machine, used to shutdown
 *
 *	bit 4 = 1	switch CPU model to 8080
 *	bit 5 = 1	switch CPU model to Z80
 *	bit 7 = 1       halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
	extern void switch_cpu(int);
#endif

	/* if port is locked do nothing */
	if (hwctl_lock && (data != 0xaa))
		return;

	/* unlock port ? */
	if (hwctl_lock && (data == 0xaa)) {
		hwctl_lock = 0;
		return;
	}

	/* process output to unlocked port */

	if (data & 128) {	/* halt system */
		cpu_error = IOHALT;
		cpu_state = STOPPED;
	}

#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
	if (data & 32) {	/* switch cpu model to Z80 */
		switch_cpu(Z80);
		return;
	}

	if (data & 16) {	/* switch cpu model to 8080 */
		switch_cpu(I8080);
		return;
	}
#endif
}

/*
 *	read MMU register
 */
static BYTE mmu_in(void)
{
	return bankio;
}

/*
 *	write MMU register
 */
static void mmu_out(BYTE data)
{
	int sel;

	LOGD(TAG, "mmu select bank %02x", data);
	bankio = data;

	if (fdc_rom_active) {
		fdc_rom_active = 0;
		reset_fdc_rom_map();
	}

	/* set banks */
	switch (data) {
	case 0x00:
	case 0x01:
		sel = 0;
		common = 0;
		break;
	case 0x02:
		sel = 1;
		common = 0;
		break;
	case 0x04:
		sel = 2;
		common = 0;
		break;
	case 0x08:
		sel = 3;
		common = 0;
		break;
	case 0x10:
		sel = 4;
		common = 0;
		break;
	case 0x20:
		sel = 5;
		common = 0;
		break;
	case 0x40:
		sel = 6;
		common = 0;
		break;
	case 0x80:
	case 0x81:
		sel = 0;
		common = 1;
		break;
	default:
		LOGE(TAG, "Not supported bank select = %02x", data);
		cpu_error = IOERROR;
		cpu_state = STOPPED;
		return;
	}

	selbnk = sel;
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

		/* make sure index pulse is there long enough */
		if (index_pulse)
			index_pulse++;

		/* the tty transmit clear happens irrespective of anything */
		if (uart0a_tbe == 0)
			uart0a_tbe = 2;

		/* count down the timers */
		/* 64 usec steps, so 15*64 usec per loop iteration */
		if (uart0a_timer1 > 0) {
			uart0a_timer1 -= 15;
			if (uart0a_timer1 <= 0) {
				uart0a_timer1 = -1; /* interrupt pending */
			}
		}
		if (uart0a_timer2 > 0) {
			uart0a_timer2 -= 15;
			if (uart0a_timer2 <= 0) {
				uart0a_timer2 = -1; /* interrupt pending */
			}
		}
		if (uart0a_timer3 > 0) {
			uart0a_timer3 -= 15;
			if (uart0a_timer3 <= 0) {
				uart0a_timer3 = -1; /* interrupt pending */
			}
		}
		if (uart0a_timer4 > 0) {
			uart0a_timer4 -= 15;
			if (uart0a_timer4 <= 0) {
				uart0a_timer4 = -1; /* interrupt pending */
			}
		}
		if (uart0a_timer5 > 0) {
			uart0a_timer5 -= 15;
			if (uart0a_timer5 <= 0) {
				uart0a_timer5 = -1; /* interrupt pending */
			}
		}

		/* check for interrupts from highest priority to lowest */

		/* if last interrupt not acknowledged by CPU no new one yet */
		if (int_int)
			goto next;

		/* UART 0A timer 1 */
		if ((uart0a_timer1 == -1) && (uart0a_int_mask & 1)) {
			uart0a_int = 0xc7;
			uart0a_int_pending = 1;
			int_data = 0xc7;
			int_int = 1;
			uart0a_timer1 = 0;
			goto next;
		}

		/* UART 0A timer 2 */
		if ((uart0a_timer2 == -1) && (uart0a_int_mask & 2)) {
			uart0a_int = 0xcf;
			uart0a_int_pending = 1;
			int_data = 0xcf;
			int_int = 1;
			uart0a_timer2 = 0;
			goto next;
		}

		/* EOJ from disk */
		if ((fdc_flags & 1) && (uart0a_int_mask & 4)) {
			uart0a_int = 0xd7;
			uart0a_int_pending = 1;
			int_data = 0xd7;
			int_int = 1;
			goto next;
		}

		/* UART 0A timer 3 */
		if ((uart0a_timer3 == -1) && (uart0a_int_mask & 8)) {
			uart0a_int = 0xdf;
			uart0a_int_pending = 1;
			int_data = 0xdf;
			int_int = 1;
			uart0a_timer3 = 0;
			goto next;
		}

		/* UART 0A receive data available */
		if ((uart0a_rda) && (uart0a_int_mask & 16)) {
			uart0a_int = 0xe7;
			uart0a_int_pending = 1;
			int_data = 0xe7;
			int_int = 1;
			goto next;
		}

		/* UART 0A transmit buffer empty */
		/* We use 2 to mean has gone empty->full but an IRQ is
		   pending */
		if (uart0a_tbe == 2) {
			uart0a_tbe = 1;
			if (uart0a_int_mask & 32) {
				uart0a_int = 0xef;
				uart0a_int_pending = 1;
				int_data = 0xef;
				int_int = 1;
				goto next;
			}
		}

		/* UART 0A timer 4 */
		if ((uart0a_timer4 == -1) && (uart0a_int_mask & 64)) {
			uart0a_int = 0xf7;
			uart0a_int_pending = 1;
			int_data = 0xf7;
			int_int = 1;
			uart0a_timer4 = 0;
			goto next;
		}

		/* UART 0A timer 5 */
		if ((uart0a_timer5 == -1) && (uart0a_int_mask & 128) && !uart0a_rst7) {
			uart0a_int = 0xff;
			uart0a_int_pending = 1;
			int_data = 0xff;
			int_int = 1;
			uart0a_timer5 = 0;
			goto next;
		}

		/* 512ms RTC */
		if (rtc && uart0a_rst7) {
			rtc = 0;
			if (uart0a_int_mask & 128) {
				uart0a_int = 0xff;
				uart0a_int_pending = 1;
				int_data = 0xff;
				int_int = 1;
				goto next;
			}
		}

		/* UART 0A no pending interrupt */
		uart0a_int = 0xff;
		uart0a_int_pending = 0;

		/* UART 1A parallel port sense */
		uart1a_lpt_busy = 0;
		if (uart1a_sense != 0) {
			uart1a_int_pending = 1;
			uart1a_int = 0xd7;
			if (uart1a_int_mask & 4) {
				uart1a_sense = 0;
				int_data = 0x24;
				int_int = 1;
				goto next;
			}
		}

		/* UART 1A receive data available */
		if ((uart1a_rda) && (uart1a_int_mask & 16)) {
			uart1a_int = 0xe7;
			uart1a_int_pending = 1;
			int_data = 0x28;
			int_int = 1;
			goto next;
		}

		/* UART 1A transmit buffer empty */
		if (uart1a_tbe == 0) {
			uart1a_tbe = 1;
			if (uart1a_int_mask & 32) {
				uart1a_int = 0xef;
				uart1a_int_pending = 1;
				int_data = 0x2a;
				int_int = 1;
				goto next;
			}
		}

		/* UART 1A no pending interrupt */
		uart1a_int_pending = 0;
		uart1a_int = 0xff;

		/* UART 1B parallel port sense */
		uart1b_lpt_busy = 0;
		if (uart1b_sense != 0) {
			uart1b_int_pending = 1;
			uart1b_int = 0xd7;
			if (uart1b_int_mask & 4) {
				uart1b_sense = 0;
				int_data = 0x34;
				int_int = 1;
				goto next;
			}
		}

		/* UART 1B receive data available */
		if ((uart1b_rda) && (uart1b_int_mask & 16)) {
			uart1b_int = 0xe7;
			uart1b_int_pending = 1;
			int_data = 0x38;
			int_int = 1;
			goto next;
		}

		/* UART 1B transmit buffer empty */
		if (uart1b_tbe == 0) {
			uart1b_tbe = 1;
			if (uart1b_int_mask & 32) {
				uart1b_int = 0xef;
				uart1b_int_pending = 1;
				int_data = 0x3a;
				int_int = 1;
				goto next;
			}
		}

		/* UART 1B no pending interrupt */
		uart1b_int_pending = 0;
		uart1b_int = 0xff;

next:
		/* sleep for 1 millisecond */
		SLEEP_MS(1);

		/* reset disk index pulse */
		if (index_pulse > 2)
			index_pulse = 0;
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

	/* if motor on generate disk index pulse */
	if (motoron) {
		if (dtype == LARGE) {
			/* 170ms ~ 360rpm for 8" drives */
			if ((counter % 17) == 0)
				index_pulse = 1;
		} else {
			/* 200ms ~ 300rpm for 5.25" drives */
			if ((counter % 20) == 0)
				index_pulse = 1;
		}
	}

	/* motor timeout timer */
	if (motortimer > 0)
		motortimer--;

	/* set RTC interrupt flag every 510ms */
	if ((counter % 51) == 0)
		rtc = 1;

#ifndef TCPASYNC
	/* poll TCP sockets if SIGIO not working */
	sigio_tcp_server_socket(0);
#endif

	BYTE status = 0;
	hal_status_in(TUART0A, &status);

	if (status & 2) {
		uart0a_rda = 1;
	} else {
		uart0a_rda = 0;
	}

	status = 0;
	hal_status_in(TUART1A, &status);

	if (status & 2) {
		uart1a_rda = 1;
	} else {
		uart1a_rda = 0;
	}

	status = 0;
	hal_status_in(TUART1B, &status);

	if (status & 2) {
		uart1b_rda = 1;
	} else {
		uart1b_rda = 0;
	}
}

#ifdef WANT_ICE
/*
 *	setup and start terminal I/O for the machine
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
 *	give terminal I/O back to ICE
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
