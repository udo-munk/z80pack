/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module of the simulator contains the I/O simulation
 * for an IMSAI 8080 system
 *
 * History:
 * 20-OCT-2008 first version finished
 * 19-JAN-2014 unused I/O ports need to return 00 and not FF
 * 02-MAR-2014 source cleanup and improvements
 * 23-MAR-2014 added 10ms timer interrupt for Kildall's timekeeper PL/M program
 * 16-JUL-2014 unused I/O ports need to return FF, see survey.mac
 * 14-OCT-2014 support for SIO 2 added, parallel ports problem with ROM avoided
 * 31-JAN-2015 took over some improvements made for the Z-1 emulation
 * 09-MAY-2015 added Cromemco DAZZLER to the machine
 * 06-DEC-2016 implemented status display and stepping for all machine cycles
 * 11-JAN-2017 implemented X11 keyboard input for VIO
 * 24-APR-2018 cleanup
 * 17-MAY-2018 improved hardware control
 * 08-JUN-2018 moved hardware initialization and reset to iosim
 * 12-JUL-2018 use logging
 * 14-JUL-2018 integrate webfrontend
 * 12-JUL-2019 implemented second SIO
 * 27-JUL-2019 more correct emulation of IMSAI SIO-2
 * 17-SEP-2019 more consistent SIO naming
 * 23-SEP-2019 added AT-modem
 * 08-OCT-2019 (Mike Douglas) added OUT 161 trap to simbdos.c for host file I/O
 * 18-OCT-2019 add MMU and memory banks
 * 24-OCT-2019 add RTC
 * 04-NOV-2019 eliminate usage of mem_base()
 * 12-NOV-2019 implemented SIO control ports
 * 14-AUG-2020 allow building machine without frontpanel
 * 15-JUL-2021 refactor serial keyboard
 * 01-AUG-2021 integrated HAL
 * 05-AUG-2021 add boot config for machine without frontpanel
 * 07-AUG-2021 add APU emulation
 * 27-MAY-2024 moved io_in & io_out to simcore
 */

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "simbdos.h"
#include "unix_network.h"
#include "imsai-sio2.h"
#include "imsai-fif.h"
#ifdef HAS_MODEM
#include "generic-at-modem.h"
#endif /* HAS_MODEM */
#ifdef HAS_DAZZLER
#include "cromemco-dazzler.h"
#include "cromemco-d+7a.h"
#endif /* HAS_DAZZLER */
#ifdef HAS_CYCLOPS
#include "cromemco-88ccc.h"
#endif /* HAS_CYCLOPS */
#include "imsai-vio.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "memsim.h"
#include "config.h"
#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif
#include "log.h"
#include "rtc.h"
#include "imsai-hal.h"
#ifdef HAS_APU
#include "apu/am9511.h"
#endif

#define AM_DATA   0xA2	/* instantiate am9511 for these ports */
#define AM_STATUS 0xA3

/*
 *	Forward declarations for I/O functions
 */
static BYTE io_no_card_in(void);
static void io_no_card_out(BYTE);
static BYTE fp_in(void);
static void fp_out(BYTE);
static BYTE hwctl_in(void);
static void hwctl_out(BYTE);
void lpt_reset(void);
static BYTE lpt_in(void);
static void lpt_out(BYTE);
static BYTE io_pport_in(void);
static BYTE mmu_in(void);
static void mmu_out(BYTE);
extern void ctrl_port_out(BYTE);
extern BYTE ctrl_port_in(void);
#ifdef HAS_APU
static BYTE apu_data_in(void);
static BYTE apu_status_in(void);
static void apu_data_out(BYTE);
static void apu_status_out(BYTE);
#endif

static const char *TAG = "IO";

static int printer;		/* fd for file "printer.txt" */
struct unix_connectors ucons[NUMUSOC]; /* socket connections for SIO's */
BYTE hwctl_lock = 0xff;		/* lock status hardware control port */
#ifdef HAS_APU
void *am9511 = NULL;		/* am9511 instantiation */
#endif

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
BYTE (*port_in[256])(void) = {
	[  0] = imsai_sio_nofun_in,	/* IMSAI SIO-2 */
	[  1] = imsai_sio_nofun_in,
	[  2] = imsai_sio1a_data_in,	/* Channel A, console */
	[  3] = imsai_sio1a_status_in,
	[  4] = imsai_sio1b_data_in,	/* Channel B, keyboard for VIO */
	[  5] = imsai_sio1b_status_in,
	[  6] = imsai_sio_nofun_in,
	[  7] = imsai_sio_nofun_in,
	[  8] = imsai_sio1_ctl_in,	/* SIO Control for A and B */
	[  9] = imsai_sio_nofun_in,
	[ 10] = imsai_sio_nofun_in,
	[ 11] = imsai_sio_nofun_in,
	[ 12] = imsai_sio_nofun_in,
	[ 13] = imsai_sio_nofun_in,
#ifdef HAS_DAZZLER
	[ 14] = cromemco_dazzler_flags_in,
#else
	[ 14] = imsai_sio_nofun_in,
#endif /* !HAS_DAZZLER */
	[ 15] = imsai_sio_nofun_in,
#ifdef HAS_CYCLOPS
	[ 16] = cromemco_88ccc_ctrl_a_in,
#endif
	[ 20] = io_pport_in,		/* parallel port */
	[ 21] = io_pport_in,		/*       "       */
	[ 24] = cromemco_d7a_D_in,
	[ 25] = cromemco_d7a_A1_in,
	[ 26] = cromemco_d7a_A2_in,
	[ 27] = cromemco_d7a_A3_in,
	[ 28] = cromemco_d7a_A4_in,
	[ 29] = cromemco_d7a_A5_in,
	[ 30] = cromemco_d7a_A6_in,
	[ 31] = cromemco_d7a_A7_in,
	[ 32] = imsai_sio_nofun_in,	/* IMSAI SIO-2 */
	[ 33] = imsai_sio_nofun_in,
	[ 34] = imsai_sio2a_data_in,	/* Channel A, UNIX socket */
	[ 35] = imsai_sio2a_status_in,
	[ 36] = imsai_sio2b_data_in,	/* Channel B, AT-modem over TCP/IP (telnet) */
	[ 37] = imsai_sio2b_status_in,
	[ 38] = imsai_sio_nofun_in,
	[ 39] = imsai_sio_nofun_in,
	[ 40] = imsai_sio2_ctl_in,	/* SIO Control for A and B */
	[ 41] = imsai_sio_nofun_in,
	[ 42] = imsai_sio_nofun_in,
	[ 43] = imsai_sio_nofun_in,
	[ 44] = imsai_sio_nofun_in,
	[ 45] = imsai_sio_nofun_in,
	[ 46] = imsai_sio_nofun_in,
	[ 47] = imsai_sio_nofun_in,
	[ 64] = mmu_in,			/* MMU */
	[ 65] = clkc_in,		/* RTC command */
	[ 66] = clkd_in,		/* RTC data */
	[160] = hwctl_in,		/* virtual hardware control */
#ifdef HAS_APU
	[162] = apu_data_in,
	[163] = apu_status_in,
#endif
	[239] = io_no_card_in,		/* unknown card */
	[243] = ctrl_port_in,		/* software memory control */
	[246] = lpt_in,			/* IMSAI PTR-300 line printer */
	[247] = io_no_card_in,		/* prio interrupt controller */
	[253] = imsai_fif_in,		/* FIF disk controller */
	[254] = io_no_card_in,		/* memory write protect */
	[255] = fp_in			/* front panel */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
void (*port_out[256])(BYTE) = {
	[  0] = imsai_sio_nofun_out,	/* IMSAI SIO-2 */
	[  1] = imsai_sio_nofun_out,
	[  2] = imsai_sio1a_data_out,	/* Channel A, console */
	[  3] = imsai_sio1a_status_out,
	[  4] = imsai_sio1b_data_out,	/* Channel B, keyboard */
	[  5] = imsai_sio1b_status_out,
	[  6] = imsai_sio_nofun_out,
	[  7] = imsai_sio_nofun_out,
	[  8] = imsai_sio1_ctl_out,	/* SIO Control for A and B */
	[  9] = imsai_sio_nofun_out,
	[ 10] = imsai_sio_nofun_out,
	[ 11] = imsai_sio_nofun_out,
	[ 12] = imsai_sio_nofun_out,
	[ 13] = imsai_sio_nofun_out,
#ifdef HAS_DAZZLER
	[ 14] = cromemco_dazzler_ctl_out,
	[ 15] = cromemco_dazzler_format_out,
#else
	[ 14] = imsai_sio_nofun_out,
	[ 15] = imsai_sio_nofun_out,
#endif /* !HAS_DAZZLER */
#ifdef HAS_CYCLOPS
	[ 16] = cromemco_88ccc_ctrl_a_out,
	[ 17] = cromemco_88ccc_ctrl_b_out,
	[ 18] = cromemco_88ccc_ctrl_c_out,
#endif
	[ 20] = io_no_card_out,		/* parallel port */
	[ 21] = io_no_card_out,		/*       "       */
	[ 24] = cromemco_d7a_D_out,
	[ 25] = cromemco_d7a_A1_out,
	[ 26] = cromemco_d7a_A2_out,
	[ 27] = cromemco_d7a_A3_out,
	[ 28] = cromemco_d7a_A4_out,
	[ 29] = cromemco_d7a_A5_out,
	[ 30] = cromemco_d7a_A6_out,
	[ 31] = cromemco_d7a_A7_out,
	[ 32] = imsai_sio_nofun_out,	/* IMSAI SIO-2 */
	[ 33] = imsai_sio_nofun_out,
	[ 34] = imsai_sio2a_data_out,	/* Channel A, UNIX socket */
	[ 35] = imsai_sio2a_status_out,
	[ 36] = imsai_sio2b_data_out,	/* Channel B, AT-modem over TCP/IP (telnet) */
	[ 37] = imsai_sio2b_status_out,
	[ 38] = imsai_sio_nofun_out,
	[ 39] = imsai_sio_nofun_out,
	[ 40] = imsai_sio2_ctl_out,	/* SIO Control for A and B */
	[ 41] = imsai_sio_nofun_out,
	[ 42] = imsai_sio_nofun_out,
	[ 43] = imsai_sio_nofun_out,
	[ 44] = imsai_sio_nofun_out,
	[ 45] = imsai_sio_nofun_out,
	[ 46] = imsai_sio_nofun_out,
	[ 47] = imsai_sio_nofun_out,
	[ 64] = mmu_out,		/* MMU */
	[ 65] = clkc_out,		/* RTC command */
	[ 66] = clkd_out,		/* RTC data */
	[160] = hwctl_out,		/* virtual hardware control */
	[161] = host_bdos_out,		/* host file I/O hook */
#ifdef HAS_APU
	[162] = apu_data_out,
	[163] = apu_status_out,
#endif
	[239] = io_no_card_out,		/* unknown card */
	[243] = ctrl_port_out,		/* software memory control */
	[246] = lpt_out,		/* IMSAI PTR-300 line printer */
	[247] = io_no_card_out,		/* prio interrupt controller */
	[253] = imsai_fif_out,		/* FIF disk controller */
	[254] = io_no_card_out,		/* memory write protect */
	[255] = fp_out			/* front panel */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	extern BYTE io_trap_in(void);
	extern void io_trap_out(BYTE);

	register int i;

	/* initialize unused ports to trap handlers */
	for (i = 0; i <= 255; i++) {
		if (port_in[i] == NULL)
			port_in[i] = io_trap_in;
		if (port_out[i] == NULL)
			port_out[i] = io_trap_out;
	}

	/* initialize IMSAI VIO if firmware is loaded */
	if ((getmem(0xfffd) == 'V') && (getmem(0xfffe) == 'I') &&
	    (getmem(0xffff) == '0')) {
		imsai_vio_init();
	}

	imsai_fif_reset();

#ifdef HAS_DAZZLER
	cromemco_d7a_init();
#endif
#ifdef HAS_MODEM
	modem_device_init();
#endif
#ifdef HAS_APU
	am9511 = am_create(AM_STATUS, AM_DATA);
#endif

	hal_reset();
	lpt_reset();

	/* create local socket for SIO's */
	init_unix_server_socket(&ucons[0], "imsaisim.sio2");
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
	register int i;

	/* close line printer file */
	if (printer != 0)
		close(printer);

	/* close network connections */
	for (i = 0; i < NUMUSOC; i++) {
		if (ucons[i].ssc)
			close(ucons[i].ssc);
	}

#ifdef HAS_DAZZLER
	/* shutdown DAZZLER */
	cromemco_dazzler_off();
#endif

	/* shutdown VIO */
	imsai_vio_off();
}

/*
 *	This function is to reset the I/O devices. It is
 *	called from the CPU simulation when an External Clear is performed.
 */
void reset_io(void)
{
#ifdef HAS_DAZZLER
	cromemco_dazzler_off();
#endif
	imsai_fif_reset();
	hwctl_lock = 0xff;
#ifdef HAS_APU
	am_reset(am9511);
#endif
}

/*
 *	No card I/O input trap function
 *	Used for input ports where I/O cards might be
 *	installed, but haven't.
 */
static BYTE io_no_card_in(void)
{
	return ((BYTE) 0xff);
}

/*
 *	No card I/O output trap function
 *	Used for output ports where I/O cards might be
 *	installed, but haven't.
 */
static void io_no_card_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Read input from front panel switches
 */
static BYTE fp_in(void)
{
#ifdef FRONTPANEL
	if (F_flag)
		return (address_switch >> 8);
	else {
#endif
		return (fp_port);
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
 *	timer interrupt causes RST 38 in IM 0 and IM 1
 */
static void int_timer(int sig)
{
	UNUSED(sig);

	int_int = 1;
	int_data = 0xff;	/* RST 38H */
}

/*
 *	Input from virtual hardware control port
 *	returns lock status of the port
 */
static BYTE hwctl_in(void)
{
	return (hwctl_lock);
}

/*
 *	Port is locked until magic number 0xaa is received!
 *
 *	Virtual hardware control output.
 *	Doesn't exist in the real machine, used to shutdown
 *	and for RST 38H interrupts every 10ms.
 *
 *	bit 0 = 1	start interrupt timer
 *	bit 0 = 0	stop interrupt timer
 *	bit 4 = 1	switch CPU model to 8080
 *	bit 5 = 1	switch CPU model to Z80
 *	bit 7 = 1	halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
	extern void switch_cpu(int);
#endif

	static struct itimerval tim;
	static struct sigaction newact;

	/* if port is locked do nothing */
	if (hwctl_lock && (data != 0xaa))
		return;

	/* unlock port ? */
	if (hwctl_lock && (data == 0xaa)) {
		hwctl_lock = 0;
		return;
	}

	/* process output to unlocked port */

	if (data & 128) {
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

	if (data & 1) {
		newact.sa_handler = int_timer;
		sigemptyset(&newact.sa_mask);
		newact.sa_flags = 0;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 10000;
		tim.it_interval.tv_sec = 0;
		tim.it_interval.tv_usec = 10000;
		setitimer(ITIMER_REAL, &tim, NULL);
	} else {
		newact.sa_handler = SIG_IGN;
		sigemptyset(&newact.sa_mask);
		newact.sa_flags = 0;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &tim, NULL);
	}
}

void lpt_reset(void)
{
	if (printer) {
		close(printer);
	}
	printer = creat("printer.txt", 0664); /* clear file on init */
}

/*
 *	Print into the printer file any data with bit 7 = 0.
 *	Data with bit 7 = 1 are commands which we ignore here.
 */
static void lpt_out(BYTE data)
{
	if (data == 0x80) {
		lpt_reset();
#ifdef HAS_NETSERVER
		if (n_flag)
			net_device_send(DEV_LPT, (char *) &data, 1);
#endif
		return;
	}

	if ((data != '\r') && !(data & 0x80)) {
again:
		if (write(printer, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				LOGE(TAG, "can't write to printer");
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
 *	I/O handler for line printer in:
 *	The IMSAI line printer returns F4 for ready and F0 for busy.
 *	Our printer file never is busy, so always return ready.
 */
static BYTE lpt_in(void)
{
	return ((BYTE) 0xf4);
}

/*
 *	Parallel ports not implemented yet.
 *	Needs to return 0, otherwise the ROM monitor
 *	detects a parallel keyboard and won't use SIO 1
 */
static BYTE io_pport_in(void)
{
	return ((BYTE) 0);
}

/*
 *	read MMU register
 */
static BYTE mmu_in(void)
{
	return (selbnk);
}

/*
 *	write MMU register
 */
static void mmu_out(BYTE data)
{
	if (data >= MAXSEG) {
		LOGE(TAG, "selected bank %d not available", data);
		cpu_error = IOERROR;
		cpu_state = STOPPED;
	}

	selbnk = data;
}

#ifdef HAS_APU
/*
 *	read am9511 data port
 */
static BYTE apu_data_in(void)
{
	return am_pop(am9511);
}

/*
 *	read am9511 status port
 */
static BYTE apu_status_in(void)
{
	return am_status(am9511);
}

/*
 *	write am9511 data port
 */
static void apu_data_out(BYTE data)
{
	am_push(am9511, data);
}

/*
 *	write am9511 status port
 */
static void apu_status_out(BYTE status)
{
	am_command(am9511, status);
}
#endif
