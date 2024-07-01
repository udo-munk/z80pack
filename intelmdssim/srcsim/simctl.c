/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module contains the user interface an Intel Intellec MDS-800 system
 *
 * History:
 * 03-JUN-2024 first version
 * 07-JUN-2024 rewrite of the monitor ports and the timing thread
 */

#include <stdint.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "sim.h"
#include "simglb.h"
#include "simcfg.h"
#include "simmem.h"
#include "simctl.h"
#include "simio.h"
#include "simcore.h"
#ifdef WANT_ICE
#include "simice.h"
#endif
#include "unix_terminal.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#include "log.h"
#endif

#ifdef FRONTPANEL
static const char *TAG = "system";

static BYTE power;

static void int_clicked(int state, int val);
static void reset_clicked(int state, int val);
static void power_clicked(int state, int val);
static void quit_callback(void);
#endif

static int cpu_wait;	/* CPU wait flag */

BYTE boot_switch;	/* status of boot switch */

/*
 *	This function initializes the front panel and terminal.
 *	Then the machine waits to be operated from the front panel,
 *	until power switched OFF again.
 *
 *	If the machine is build without front panel then just run
 *	the CPU with the configured ROM or software loaded with -x option.
 */
void mon(void)
{
#ifdef FRONTPANEL
	if (F_flag) {
		/* initialize frontpanel */
		XInitThreads();

		if (!fp_init2(&confdir[0], "panel.conf", fp_size)) {
			LOGE(TAG, "frontpanel error");
			exit(EXIT_FAILURE);
		}

		fp_addQuitCallback(quit_callback);
		fp_framerate(fp_fps);
		fp_bindSimclock(&fp_clock);
		fp_bindRunFlag(&cpu_state);

		/* bind frontpanel LED's to variables */
		fp_bindLight8("LED_INT_{0-7}", &int_requests, 1);
		fp_bindLight8("LED_PWR", &power, 1);
		fp_bindLight8("LED_RUN", &cpu_state, 1 /* CONTIN_RUN */);
		fp_bindLight8("LED_HALT", &cpu_bus, 4 /* CPU_HLTA */) ;

		/* bind frontpanel switches to variables */
		fp_bindSwitch8("SW_BOOT", &boot_switch, &boot_switch, 1);
		fp_sampleSwitches();

		/* add callbacks for frontpanel switches */
		fp_addSwitchCallback("SW_INT_7", int_clicked, 7);
		fp_addSwitchCallback("SW_INT_6", int_clicked, 6);
		fp_addSwitchCallback("SW_INT_5", int_clicked, 5);
		fp_addSwitchCallback("SW_INT_4", int_clicked, 4);
		fp_addSwitchCallback("SW_INT_3", int_clicked, 3);
		fp_addSwitchCallback("SW_INT_2", int_clicked, 2);
		fp_addSwitchCallback("SW_INT_1", int_clicked, 1);
		fp_addSwitchCallback("SW_INT_0", int_clicked, 0);
		fp_addSwitchCallback("SW_RESET", reset_clicked, 0);
		fp_addSwitchCallback("SW_PWR", power_clicked, 0);
	} else {
#endif
		boot_switch = 1;
#ifdef FRONTPANEL
	}
#endif

	/* give threads a bit time and then empty buffer */
	SLEEP_MS(999);
	fflush(stdout);

#ifndef WANT_ICE
	/* initialize terminal */
	set_unix_terminal();
#endif
	atexit(reset_unix_terminal);

#ifdef FRONTPANEL
	if (F_flag) {
		/* operate machine from front panel */
		while (cpu_error == NONE) {
			/* update frontpanel LED's */
			fp_clock++;
			fp_sampleData();

			/* run CPU if not idling */
			if (power && !cpu_wait)
				run_cpu();

			fp_clock++;
			fp_sampleData();

			/* wait a bit, system is idling */
			SLEEP_MS(10);
		}
	} else {
#endif
#ifdef WANT_ICE
		ice_before_go = ice_go;
		ice_after_go = ice_break;
		ice_cmd_loop(0);
#else
		/* run the CPU */
		run_cpu();
#endif
#ifdef FRONTPANEL
	}
#endif

#ifndef WANT_ICE
	/* reset terminal */
	reset_unix_terminal();
#endif
	putchar('\n');

#ifdef FRONTPANEL
	if (F_flag) {
		/* all LED's off and update front panel */
		cpu_bus = 0;
		bus_request = 0;
		IFF = 0;
		power = 0;
		int_requests = 0;
		fp_sampleData();

		/* wait a bit before termination */
		SLEEP_MS(999);

		/* shutdown frontpanel */
		fp_quit();
	}
#endif

	/* check for CPU emulation errors and report */
	report_cpu_error();
	report_cpu_stats();
}

#ifdef FRONTPANEL
/*
 *	Callback for INT_7-0 switches
 */
static void int_clicked(int state, int val)
{
	if (!power)
		return;

	switch (state) {
	case FP_SW_CENTER:
		break;
	case FP_SW_UP:
		if (!cpu_wait)
			int_request(val);
		break;
	default:
		break;
	}
}

/*
 *	Single step through the machine cycles after M1
 */
int wait_step(void)
{
	cpu_bus &= ~CPU_M1;
	m1_step = 0;
	return 0;
}

/*
 *	Single step through interrupt machine cycles
 */
void wait_int_step(void)
{
}

/*
 *	Callback for RESET switch
 */
static void reset_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	switch (state) {
	case FP_SW_CENTER:
		break;
	case FP_SW_UP:
		cpu_state |= RESET;
		m1_step = 0;
		IFF = 0;
		reset_io();
		reset_cpu();
		cpu_state &= ~RESET;
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		cpu_wait = 0;
		break;
	default:
		break;
	}
}

/*
 *	Callback for POWER switch
 */
static void power_clicked(int state, int val)
{
	UNUSED(val);

	switch (state) {
	case FP_SW_DOWN:
		if (!power)
			break;
		power--;
		cpu_state = STOPPED;
		cpu_error = POWEROFF;
		break;
	case FP_SW_UP:
		if (power)
			break;
		power++;
		cpu_wait = 1;
		if (!isatty(fileno(stdout)) || (system("tput clear") == -1))
			puts("\r\n\r\n\r\n");
		break;
	default:
		break;
	}
}

/*
 *	Callback for quit (graphics window closed)
 */
static void quit_callback(void)
{
	power = 0;
	cpu_state = STOPPED;
	cpu_error = POWEROFF;
}

#endif /* FRONTPANEL */
