/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 by Udo Munk
 *
 * This module of the simulator contains the I/O simulation
 * for an Altair 8800 system
 *
 * History:
 * 20-OCT-2008 first version finished
 * 19-JAN-2014 unused I/O ports need to return 00 and not FF
 * 02-MAR-2014 source cleanup and improvements
 * 14-MAR-2014 added Tarbell SD FDC and printer port
 * 15-MAR-2014 modified printer port for Tarbell CP/M 1.4 BIOS
 * 23-MAR-2014 added 10ms timer interrupt for Kildall's timekeeper PL/M program
 * 16-JUL-2014 unused I/O ports need to return FF, see survey.mac
 * 18-JUL-2014 completed all ports used, so that survey would find them
 * 31-JAN-2015 took over some improvements made for the Z-1 emulation
 * 29-APR-2015 added Cromemco DAZZLER to the machine
 * 12-JUL-2016 added 88-SIO to ports 0/1, also connected to tty
 * 11-AUG-2016 moved printer ports to 2/3, most software want it there
 * 06-DEC-2016 implemented status display and stepping for all machine cycles
 * 26-FEB-2017 implemented X11 keyboard for VDM
 * 22-MAR-2017 connected SIO 2 to UNIX domain socket
 * 27-MAR-2017 connected SIO 3 to UNIX domain socket
 * 24-APR-2018 cleanup
 * 17-MAY-2018 improved hardware control
 * 08-JUN-2018 moved hardware initialization and reset to iosim
 * 15-JUL-2018 use logging
 * 10-AUG-2018 added MITS 88-DCDD floppy disk controller
 * 08-OCT-2019 (Mike Douglas) added OUT 161 trap to simbdos.c for host file I/O
 * 31-JUL-2021 allow building machine without frontpanel
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
#include "log.h"
#include "unix_network.h"
#include "altair-88-sio.h"
#include "altair-88-2sio.h"
#include "tarbell_fdc.h"
#include "altair-88-dcdd.h"
#include "cromemco-dazzler.h"
#include "proctec-vdm.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "simmem.h"
#include "simcfg.h"
#include "simcore.h"

/*
 *	Forward declarations for I/O functions
 */
static BYTE hwctl_in(void), fp_in(void);
static void hwctl_out(BYTE), fp_out(BYTE data);
static BYTE lpt_status_in(void), lpt_data_in(void);
static void lpt_status_out(BYTE), lpt_data_out(BYTE data);
static BYTE kbd_status_in(void), kbd_data_in(void);

static void io_no_card_out(BYTE data);
#if 0	/* currently not used */
static BYTE io_no_card_in(void);
#endif

static const char *TAG = "IO";

static int printer;		/* fd for file "printer.txt" */
static BYTE hwctl_lock = 0xff;	/* lock status hardware control port */

struct unix_connectors ucons[NUMUSOC]; /* socket connections for SIO's */

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
BYTE (*const port_in[256])(void) = {
	[  0] = altair_sio0_status_in,	/* SIO 0 connected to console */
	[  1] = altair_sio0_data_in,	/*  "  */
	[  2] = lpt_status_in,		/* printer status */
	[  3] = lpt_data_in,		/* printer data */
	[  4] = kbd_status_in,		/* status VDM keyboard */
	[  5] = kbd_data_in,		/* data VDM keyboard */
	[  6] = altair_sio3_status_in,	/* SIO 3 connected to socket */
	[  7] = altair_sio3_data_in,	/*  "  */
	[  8] = altair_dsk_status_in,	/* MITS 88-DCDD status */
	[  9] = altair_dsk_sec_in,	/* MITS 88-DCDD sector position */
	[ 10] = altair_dsk_data_in,	/* MITS 88-DCDD read data */
	[ 14] = cromemco_dazzler_flags_in, /* Cromemco Dazzler */
	[ 16] = altair_sio1_status_in,	/* SIO 1 connected to console */
	[ 17] = altair_sio1_data_in,	/*  "  */
	[ 18] = altair_sio2_status_in,	/* SIO 2 connected to socket */
	[ 19] = altair_sio2_data_in,	/*  "  */
	[160] = hwctl_in,		/* virtual hardware control */
	[248] = tarbell_stat_in,	/* Tarbell 1011D status */
	[249] = tarbell_track_in,	/* Tarbell 1011D track */
	[250] = tarbell_sec_in,		/* Tarbell 1011D sector */
	[251] = tarbell_data_in,	/* Tarbell 1011D data */
	[252] = tarbell_wait_in,	/* Tarbell 1011D wait */
	[255] = fp_in			/* frontpanel */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
void (*const port_out[256])(BYTE data) = {
	[  0] = altair_sio0_status_out,	/* SIO 0 connected to console */
	[  1] = altair_sio0_data_out,	/*  "  */
	[  2] = lpt_status_out,		/* printer status */
	[  3] = lpt_data_out,		/* printer data */
	[  4] = io_no_card_out,		/* status VDM keyboard */
	[  5] = io_no_card_out,		/* data VDM keyboard */
	[  6] = altair_sio3_status_out,	/* SIO 3 connected to socket */
	[  7] = altair_sio3_data_out,	/*  "  */
	[  8] = altair_dsk_select_out,	/* MITS 88-DCDD disk select */
	[  9] = altair_dsk_control_out,	/* MITS 88-DCDD control disk */
	[ 10] = altair_dsk_data_out,	/* MITS 88-DCDD write data */
	[ 14] = cromemco_dazzler_ctl_out, /* Cromemco Dazzler */
	[ 15] = cromemco_dazzler_format_out, /*  "  */
	[ 16] = altair_sio1_status_out,	/* SIO 1 connected to console */
	[ 17] = altair_sio1_data_out,	/*  "  */
	[ 18] = altair_sio2_status_out,	/* SIO 2 connected to socket */
	[ 19] = altair_sio2_data_out,	/*  "  */
	[160] = hwctl_out,		/* virtual hardware control */
	[161] = host_bdos_out,		/* host file I/O hook */
	[200] = proctec_vdm_out,	/* Processor Technology VDM */
	[248] = tarbell_cmd_out,	/* Tarbell 1011D command */
	[249] = tarbell_track_out,	/* Tarbell 1011D track */
	[250] = tarbell_sec_out,	/* Tarbell 1011D sector */
	[251] = tarbell_data_out,	/* Tarbell 1011D data */
	[252] = tarbell_ext_out,	/* Tarbell 1011D extended cmd */
	[255] = fp_out			/* front panel */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	/* create local sockets */
	init_unix_server_socket(&ucons[0], "altairsim.tape");
	init_unix_server_socket(&ucons[1], "altairsim.sio2");
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

	/* shutdown DAZZLER */
	cromemco_dazzler_off();

	/* shutdown VDM */
	proctec_vdm_off();
}

/*
 *	This function is to reset the I/O devices. It is
 *	called from the CPU simulation when an External Clear is performed.
 */
void reset_io(void)
{
	cromemco_dazzler_off();
	tarbell_reset();
	altair_dsk_reset();
	hwctl_lock = 0xff;
}

#if 0		/* currently not used */
/*
 *	No card I/O input trap function
 *	Used for input ports where I/O cards might be
 *	installed, but haven't.
 */
static BYTE io_no_card_in(void)
{
	return (BYTE) IO_DATA_UNUSED;
}
#endif

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
		return address_switch >> 8;
	else {
#endif
		return fp_port;
#ifdef FRONTPANEL
	}
#endif
}

/*
 *	Output to front panel switch port won't do anything
 */
static void fp_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	timer interrupt causes RST 38 in IM 0 and IM 1
 */
static void int_timer(int sig)
{
	UNUSED(sig);

	int_int = 1;
	int_data = 0xff;	/* RST 38H for IM 0 */
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
 *	and for RST 38H interrupts every 10ms.
 *
 *	bit 0 = 1	start interrupt timer
 *	bit 0 = 0	stop interrupt timer
 *	bit 4 = 1	switch CPU model to 8080
 *	bit 5 = 1	switch CPU model to Z80
 *	bit 7 = 1       halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
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

/*
 *	Input from data printer port
 */
static BYTE lpt_data_in(void)
{
	return (BYTE) 0;
}

/*
 *	Print data into the printer file
 */
static void lpt_data_out(BYTE data)
{
	if (printer == 0) {
		if ((printer = creat("printer.txt", 0664)) == -1) {
			LOGE(TAG, "can't create printer.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			printer = 0;
			return;
		}
	}

	if ((data != '\r') && (data != 0x00)) {
again:
		if (write(printer, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				LOGE(TAG, "can't write to printer.txt");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}
	}
}

/*
 *	I/O handler for line printer status in:
 *	Our printer file never is busy, so always return ready.
 */
static BYTE lpt_status_in(void)
{
	return (BYTE) 3;
}

/*
 *	Output to line printer status port won't do anything
 */
static void lpt_status_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	Return status of the VDM keyboard
 */
static BYTE kbd_status_in(void)
{
	extern int proctec_kbd_status;

	return (BYTE) proctec_kbd_status;
}

/*
 * Return next data byte from the VDM keyboard
 */
static BYTE kbd_data_in(void)
{
	int data;

	if (proctec_kbd_data == -1)
		return (BYTE) 0;

	/* take over data and reset status */
	data = proctec_kbd_data;
	proctec_kbd_data = -1;
	proctec_kbd_status = 1;

	return (BYTE) data;
}
