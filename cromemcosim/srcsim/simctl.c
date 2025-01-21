/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * This module allows operation of the system from a Cromemco Z-1 front panel
 *
 * Copyright (C) 2014-2024 by Udo Munk
 * Copyright (C) 2025 by Thomas Eberhardt
 *
 * History:
 * 15-DEC-2014 first version
 * 20-DEC-2014 added 4FDC emulation and machine boots CP/M 2.2
 * 28-DEC-2014 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-2015 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-2015 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-2015 fdc and tu-art improvements, implemented banked memory
 * 24-APR-2015 added Cromemco DAZZLER to the machine
 * 01-MAR-2016 added sleep for threads before switching tty to raw mode
 * 09-MAY-2016 frontpanel configuration with path support
 * 06-DEC-2016 implemented status display and stepping for all machine cycles
 * 13-MAR-2017 can't examine/deposit if CPU running HALT instruction
 * 29-JUN-2017 system reset overworked
 * 10-APR-2018 trap CPU on unsupported bus data during interrupt
 * 17-MAY-2018 implemented hardware control
 * 08-JUN-2018 moved hardware initialization and reset to iosim
 * 11-JUN-2018 fixed reset so that cold and warm start works
 * 18-JUL-2018 use logging
 * 04-NOV-2019 eliminate usage of mem_base()
 * 17-JUN-2021 allow building machine without frontpanel
 * 29-APR-2024 added CPU execution statistics
 * 04-JAN-2025 add SDL2 support
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simcfg.h"
#include "simmem.h"
#include "simio.h"
#include "simport.h"
#ifdef WANT_ICE
#include "simice.h"
#endif
#include "simctl.h"

#include "unix_terminal.h"
#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif

#ifdef FRONTPANEL
#ifdef WANT_SDL
#include "simsdl.h"
#else
#include <X11/Xlib.h>
#endif
#include "frontpanel.h"
#include "log.h"
static const char *TAG = "system";

				/* cpu_switch states */
#define CPUSW_STOP	0	/* stopped */
#define CPUSW_RUN	1	/* running */
#define CPUSW_STEP	2	/* single step */
#define CPUSW_STEPCYCLE	3	/* machine cycle step */

static BYTE fp_led_wait;
static BYTE fp_led_speed;
static int cpu_switch;
static int reset;
static BYTE power;

static void run_clicked(int state, int val), step_clicked(int state, int val);
static void reset_clicked(int state, int val);
static void examine_clicked(int state, int val);
static void deposit_clicked(int state, int val);
static void power_clicked(int state, int val);
static void quit_callback(void);

#ifdef WANT_SDL
static int fp_win_id;	/* frontpanel window id */
static win_funcs_t fp_win_funcs = {
	fp_openWindow,
	fp_quit,
	fp_procEvent,
	fp_draw
};
#endif
#endif /* FRONTPANEL */

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
#ifdef HAS_NETSERVER
	if (n_flag)
		start_net_services(ns_port);
#endif

#ifdef FRONTPANEL
	if (F_flag) {
#ifndef WANT_SDL
		XInitThreads();
#endif
		/* initialize front panel */
		if (!fp_init2(confdir, "panel.conf", fp_size)) {
			LOGE(TAG, "frontpanel error");
			exit(EXIT_FAILURE);
		}
#ifdef WANT_SDL
		fp_win_id = simsdl_create(&fp_win_funcs);
#endif

		fp_addQuitCallback(quit_callback);
		fp_framerate(fp_fps);
		fp_bindSimclock(&fp_clock);
		fp_bindRunFlag(&cpu_state);
		fp_bindPowerFlag(&power);

		/* bind frontpanel LED's to variables */
		fp_bindLight16("LED_ADDR_{00-15}", &fp_led_address, 1);
		fp_bindLight8("LED_DATA_{00-07}", &fp_led_data, 1);
		fp_bindLight8("LED_STATUS_00", &cpu_bus, 1);
		fp_bindLight8("LED_STATUS_01", &cpu_bus, 2);
		fp_bindLight8("LED_STATUS_02", &fp_led_speed, 1);
		fp_bindLight8("LED_STATUS_03", &cpu_bus, 4);
		fp_bindLight8("LED_STATUS_04", &cpu_bus, 5);
		fp_bindLight8("LED_STATUS_05", &cpu_bus, 6);
		fp_bindLight8("LED_STATUS_06", &cpu_bus, 7);
		fp_bindLight8("LED_STATUS_07", &cpu_bus, 8);
		fp_bindLight8invert("LED_DATOUT_{00-07}",
				    &fp_led_output, 1, 255);
		fp_bindLight8("LED_RUN", &cpu_state, 1);
		fp_bindLight8("LED_WAIT", &fp_led_wait, 1);
		fp_bindLight8("LED_INTEN", &IFF, 1);
		fp_bindLight8("LED_HOLD", &bus_request, 1);

		/* bind frontpanel switches to variables */
		fp_bindSwitch16("SW_{00-15}", &address_switch,
				&address_switch, 1);

		/* add callbacks for front panel switches */
		fp_addSwitchCallback("SW_RUN", run_clicked, 0);
		fp_addSwitchCallback("SW_STEP", step_clicked, 0);
		fp_addSwitchCallback("SW_RESET", reset_clicked, 0);
		fp_addSwitchCallback("SW_EXAMINE", examine_clicked, 0);
		fp_addSwitchCallback("SW_DEPOSIT", deposit_clicked, 0);
		fp_addSwitchCallback("SW_PWR", power_clicked, 0);
	}
#endif /* FRONTPANEL */

	/* give threads a bit time and then empty buffer */
	sleep_for_ms(999);
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
			if (reset) {
				cpu_bus = 0xff;
				fp_led_address = 0xffff;
				fp_led_data = 0xff;
			} else {
				if (power) {
					fp_led_address = PC;
					if (!(cpu_bus & CPU_INTA))
						fp_led_data = getmem(PC);
					else
						fp_led_data = (int_data != -1)
							      ? (BYTE) int_data
							      : 0xff;
				}
			}

			/* set FDC autoboot flag from fp switch */
			if (address_switch & 256)
				fdc_flags |= 64;

			fp_clock++;
			fp_sampleData();

			/* run CPU if not idling */
			switch (cpu_switch) {
			case CPUSW_RUN:
				if (!reset)
					run_cpu();
				break;
			case CPUSW_STEP:
				step_cpu();
				if (cpu_switch == CPUSW_STEP)
					cpu_switch = CPUSW_STOP;
				break;
			default:
				break;
			}

			fp_clock++;
			fp_sampleData();

			/* wait a bit, system is idling */
			sleep_for_ms(10);
		}
	} else {
#endif /* FRONTPANEL */
		/* set FDC autoboot flag from fp switch */
		if (fp_port & 1)
			fdc_flags |= 64;
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
		power = 0;
		cpu_bus = 0;
		bus_request = 0;
		IFF = 0;
		fp_led_wait = 0;
		fp_led_speed = 0;
		fp_led_output = 0xff;
		fp_led_address = 0;
		fp_led_data = 0;
		fp_sampleData();

		/* wait a bit before termination */
		sleep_for_ms(999);

		/* shutdown frontpanel */
#ifdef WANT_SDL
		simsdl_destroy(fp_win_id);
#else
		fp_quit();
#endif
	}
#endif

	/* check for CPU emulation errors and report */
	report_cpu_error();
	report_cpu_stats();
}

#ifdef FRONTPANEL
/*
 *	Callback for RUN/STOP switch
 */
static void run_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	switch (state) {
	case FP_SW_UP:
		if (cpu_state != ST_CONTIN_RUN) {
			cpu_state = ST_STOPPED;
			fp_led_wait = 0;
			cpu_switch = CPUSW_RUN;
		}
		break;
	case FP_SW_DOWN:
		if (cpu_state == ST_CONTIN_RUN) {
			cpu_state = ST_STOPPED;
			fp_led_wait = 1;
			cpu_switch = CPUSW_STOP;
		}
		break;
	default:
		break;
	}
}

/*
 *	Callback for STEP switch
 */
static void step_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	if (cpu_state == ST_CONTIN_RUN)
		return;

	switch (state) {
	case FP_SW_UP:
	case FP_SW_DOWN:
		cpu_switch = CPUSW_STEP;
		break;
	default:
		break;
	}
}

/*
 * Single step through the machine cycles after M1
 */
bool wait_step(void)
{
	bool ret = false;
	uint64_t t1, t2;

	if (cpu_state != ST_SINGLE_STEP) {
		cpu_bus &= ~CPU_M1;
		m1_step = false;
		return ret;
	}

	if ((cpu_bus & CPU_M1) && !m1_step) {
		cpu_bus &= ~CPU_M1;
		return ret;
	}

	cpu_switch = CPUSW_STEPCYCLE;

	t1 = get_clock_us();
	while ((cpu_switch == CPUSW_STEPCYCLE) && !reset) {
		/* when INP update data bus LEDs */
		if (cpu_bus == (CPU_WO | CPU_INP)) {
			if (port_in[fp_led_address & 0xff])
				fp_led_data =
					(*port_in[fp_led_address & 0xff])();
		}
		fp_clock++;
		fp_sampleData();
		sleep_for_ms(10);
		ret = true;
		t2 = get_clock_us();
		cpu_time -= t2 - t1;
		t1 = t2;
	}

	cpu_bus &= ~CPU_M1;
	m1_step = false;
	return ret;
}

/*
 * Single step through interrupt machine cycles
 */
void wait_int_step(void)
{
	uint64_t t1, t2;

	if (cpu_state != ST_SINGLE_STEP)
		return;

	cpu_switch = CPUSW_STEPCYCLE;

	t1 = get_clock_us();
	while ((cpu_switch == CPUSW_STEPCYCLE) && !reset) {
		fp_clock++;
		fp_sampleData();
		sleep_for_ms(10);
		t2 = get_clock_us();
		cpu_time -= t2 - t1;
		t1 = t2;
	}
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
	case FP_SW_UP:
		/* reset CPU only */
		reset = 1;
		cpu_state |= ST_RESET;
		m1_step = false;
		IFF = 0;
		fp_led_output = 0;
		break;
	case FP_SW_CENTER:
		if (reset) {
			/* reset CPU */
			reset_cpu();
#ifdef HAS_BANKED_ROM
			PC = boot_switch[M_value];
#endif
			reset = 0;
			cpu_state &= ~ST_RESET;

			/* update front panel */
			fp_led_address = PC;
			fp_led_data = getmem(PC);
			cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		}
		break;
	case FP_SW_DOWN:
		/* reset CPU and I/O devices */
		reset = 2;
		cpu_state |= ST_RESET;
		m1_step = false;
		IFF = 0;
		fp_led_output = 0;
		reset_io();
		break;
	default:
		break;
	}
}

/*
 *	Callback for EXAMINE/EXAMINE NEXT switch
 */
static void examine_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	if ((cpu_state == ST_CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_address = address_switch;
		fp_led_data = getmem(address_switch);
		PC = address_switch;
		break;
	case FP_SW_DOWN:
		fp_led_address++;
		fp_led_data = getmem(fp_led_address);
		PC = fp_led_address;
		break;
	default:
		break;
	}
}

/*
 *	Callback for DEPOSIT/DEPOSIT NEXT switch
 */
static void deposit_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	if ((cpu_state == ST_CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_data = address_switch & 0xff;
		putmem(PC, fp_led_data);
		break;
	case FP_SW_DOWN:
		PC++;
		fp_led_address++;
		fp_led_data = address_switch & 0xff;
		putmem(PC, fp_led_data);
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
	case FP_SW_UP:
		if (power)
			break;
		power = 1;
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		fp_led_address = PC;
		fp_led_data = getmem(PC);
		fp_led_speed = (f_value == 0 || f_value >= 4) ? 1 : 0;
		fp_led_wait = 1;
		fp_led_output = 0;
		if (!isatty(fileno(stdout)) || (system("tput clear") == -1))
			puts("\r\n\r\n\r\n");
		break;
	case FP_SW_DOWN:
		if (!power)
			break;
		power = 0;
		cpu_switch = CPUSW_STOP;
		cpu_state = ST_STOPPED;
		cpu_error = POWEROFF;
		break;
	default:
		break;
	}
}

/*
 * Callback for quit (graphics window closed)
 */
static void quit_callback(void)
{
	power = 0;
	cpu_switch = CPUSW_STOP;
	cpu_state = ST_STOPPED;
	cpu_error = POWEROFF;
}
#endif /* FRONTPANEL */
