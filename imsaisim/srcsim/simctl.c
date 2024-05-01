/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * This module allows operation of the system from an IMSAI 8080 front panel
 *
 * Copyright (C) 2008-2024 by Udo Munk
 *
 * History:
 * 20-OCT-2008 first version finished
 * 26-OCT-2008 corrected LED status while RESET is hold in upper position
 * 27-JAN-2014 set IFF=0 when powered off, so that LED goes off
 * 02-MAR-2014 source cleanup and improvements
 * 15-APR-2014 added fflush() for teletype
 * 19-APR-2014 moved CPU error report into a function
 * 06-JUN-2014 forgot to disable timer interrupts when machine switched off
 * 10-JUN-2014 increased fp operation timer from 1ms to 100ms
 * 09-MAY-2015 added Cromemco DAZZLER to the machine
 * 01-MAR-2016 added sleep for threads before switching tty to raw mode
 * 08-MAY-2016 frontpanel configuration with path support
 * 06-DEC-2016 implemented status display and stepping for all machine cycles
 * 26-JAN-2017 bugfix for DATA LED's not always showing correct bus data
 * 13-MAR-2017 can't examine/deposit if CPU running HALT instruction
 * 29-JUN-2017 system reset overworked
 * 10-APR-2018 trap CPU on unsupported bus data during interrupt
 * 17-MAY-2018 improved hardware control
 * 08-JUN-2018 moved hardware initialization and reset to iosim
 * 11-JUN-2018 fixed reset so that cold and warm start works
 * 12-JUL-2018 use logging
 * 04-NOV-2019 eliminate usage of mem_base()
 * 06-NOV-2019 use correct memory access functions
 * 14-AUG-2020 allow building machine without frontpanel
 * 29-APR-2024 added CPU execution statistics
 */

#include <X11/Xlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#ifdef UNIX_TERMINAL
#include "../../iodevices/unix_terminal.h"
#endif
#include "log.h"

extern void reset_cpu(void), reset_io(void);
extern void run_cpu(void), step_cpu(void);
extern void report_cpu_error(void), report_cpu_stats(void);
extern unsigned long long get_clock_us(void);

static const char *TAG = "system";

#ifdef FRONTPANEL
static BYTE fp_led_wait;
static int cpu_switch;
static int reset;
static int power;
#endif

#ifdef FRONTPANEL
static void run_clicked(int, int), step_clicked(int, int);
static void reset_clicked(int, int);
static void examine_clicked(int, int), deposit_clicked(int, int);
static void power_clicked(int, int);
static void quit_callback(void);
#endif

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
	extern int start_net_services(void);
	start_net_services();
#endif

#ifdef FRONTPANEL
	/* initialize front panel */
	XInitThreads();

	putchar('\n');
	if (!fp_init2(&confdir[0], "panel.conf", fp_size)) {
		LOGE(TAG, "frontpanel error");
		exit(EXIT_FAILURE);
	}

	fp_addQuitCallback(quit_callback);
	fp_framerate(fp_fps);
	fp_bindSimclock(&fp_clock);
	fp_bindRunFlag(&cpu_state);

	/* bind frontpanel LED's to variables */
	fp_bindLight16("LED_ADDR_{00-15}", &fp_led_address, 1);
	fp_bindLight8("LED_DATA_{00-07}", &fp_led_data, 1);
	fp_bindLight8("LED_STATUS_{00-07}", &cpu_bus, 1);
	fp_bindLight8invert("LED_DATOUT_{00-07}", &fp_led_output, 1, 255);
	fp_bindLight8("LED_RUN", &cpu_state, 1);
	fp_bindLight8("LED_WAIT", &fp_led_wait, 1);
	fp_bindLight8("LED_INTEN", &IFF, 1);
	fp_bindLight8("LED_HOLD", &bus_request, 1);

	/* bind frontpanel switches to variables */
	fp_bindSwitch16("SW_{00-15}", &address_switch, &address_switch, 1);

	/* add callbacks for front panel switches */
	fp_addSwitchCallback("SW_RUN", run_clicked, 0);
	fp_addSwitchCallback("SW_STEP", step_clicked, 0);
	fp_addSwitchCallback("SW_RESET", reset_clicked, 0);
	fp_addSwitchCallback("SW_EXAMINE", examine_clicked, 0);
	fp_addSwitchCallback("SW_DEPOSIT", deposit_clicked, 0);
	fp_addSwitchCallback("SW_PWR", power_clicked, 0);
#endif

#ifdef UNIX_TERMINAL
	/* give threads a bit time and then empty buffer */
	SLEEP_MS(999);
	fflush(stdout);

	/* initialize terminal */
#ifndef WANT_ICE
	set_unix_terminal();
#endif
	atexit(reset_unix_terminal);
#endif

#ifdef HAS_BANKED_ROM
	if (R_flag)
		PC = 0x0000;
#endif

#ifdef FRONTPANEL
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
					fp_led_data = (int_data != -1) ?
						      (BYTE) int_data : 0xff;
			}
		}

		fp_clock++;
		fp_sampleData();

		switch (cpu_switch) {
		case 1:
			if (!reset) {
				cpu_start = get_clock_us();
				run_cpu();
				cpu_stop = get_clock_us();
			}
			break;
		case 2:
			step_cpu();
			if (cpu_switch == 2)
				cpu_switch = 0;
			break;
		default:
			break;
		}

		fp_clock++;
		fp_sampleData();

		/* wait a bit, system is idling */
		SLEEP_MS(10);
	}
#else
#ifdef WANT_ICE
	extern void ice_cmd_loop(int);

	ice_before_go = set_unix_terminal;
	ice_after_go = reset_unix_terminal;
	atexit(reset_unix_terminal);

	ice_cmd_loop(0);
#else
	/* run the CPU */
	cpu_start = get_clock_us();
	run_cpu();
	cpu_stop = get_clock_us();
#endif
#endif

#ifdef UNIX_TERMINAL
#ifndef WANT_ICE
	/* reset terminal */
	reset_unix_terminal();
#endif
	putchar('\n');
#endif

#ifdef FRONTPANEL
	/* all LED's off and update front panel */
	cpu_bus = 0;
	bus_request = 0;
	IFF = 0;
	fp_led_wait = 0;
	fp_led_output = 0xff;
	fp_led_address = 0;
	fp_led_data = 0;
	fp_sampleData();

	/* wait a bit before termination */
	SLEEP_MS(999);

	/* stop frontpanel */
	fp_quit();
#endif

	/* check for CPU emulation errors and report */
	report_cpu_error();
	report_cpu_stats();
}

#ifdef FRONTPANEL
/*
 *	Callback for RUN/STOP switch
 */
void run_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	switch (state) {
	case FP_SW_UP:
		if (cpu_state != CONTIN_RUN) {
			cpu_state = CONTIN_RUN;
			fp_led_wait = 0;
			cpu_switch = 1;
		}
		break;
	case FP_SW_DOWN:
		if (cpu_state == CONTIN_RUN) {
			cpu_state = STOPPED;
			fp_led_wait = 1;
			cpu_switch = 0;
		}
		break;
	default:
		break;
	}
}

/*
 *	Callback for STEP switch
 */
void step_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	if (cpu_state == CONTIN_RUN)
		return;

	switch (state) {
	case FP_SW_UP:
	case FP_SW_DOWN:
		cpu_switch = 2;
		break;
	default:
		break;
	}
}

/*
 * Single step through the machine cycles after M1
 */
int wait_step(void)
{
	extern BYTE (*port_in[256])(void);
	int ret = 0;

	if (cpu_state != SINGLE_STEP) {
		cpu_bus &= ~CPU_M1;
		m1_step = 0;
		return (ret);
	}

	if ((cpu_bus & CPU_M1) && !m1_step) {
		cpu_bus &= ~CPU_M1;
		return (ret);
	}

	cpu_switch = 3;

	while ((cpu_switch == 3) && !reset) {
		/* when INP update data bus LEDs */
		if (cpu_bus == (CPU_WO | CPU_INP))
			fp_led_data = (*port_in[fp_led_address & 0xff])();
		fp_clock++;
		fp_sampleData();
		SLEEP_MS(10);
		ret = 1;
	}

	cpu_bus &= ~CPU_M1;
	m1_step = 0;
	return (ret);
}

/*
 * Single step through interrupt machine cycles
 */
void wait_int_step(void)
{
	if (cpu_state != SINGLE_STEP)
		return;

	cpu_switch = 3;

	while ((cpu_switch == 3) && !reset) {
		fp_clock++;
		fp_sampleData();

		SLEEP_MS(10);
	}
}

/*
 *	Callback for RESET switch
 */
void reset_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	switch (state) {
	case FP_SW_UP:
		/* reset CPU only */
		reset = 1;
		cpu_state |= RESET;
		m1_step = 0;
		IFF = 0;
		fp_led_output = 0;
		break;
	case FP_SW_CENTER:
		if (reset) {
			/* reset CPU */
			reset = 0;
			reset_cpu();
			cpu_state &= ~RESET;

			/* update front panel */
			fp_led_address = 0;
			fp_led_data = getmem(0);
			cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		}
		break;
	case FP_SW_DOWN:
		/* reset CPU and I/O devices */
		reset = 1;
		cpu_state |= RESET;
		m1_step = 0;
		IFF = 0;
		fp_led_output = 0;
		reset_io();
		reset_memory();
		break;
	default:
		break;
	}
}

/*
 *	Callback for EXAMINE/EXAMINE NEXT switch
 */
void examine_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	if ((cpu_state == CONTIN_RUN) || (cpu_bus & CPU_HLTA))
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
void deposit_clicked(int state, int val)
{
	UNUSED(val);

	if (!power)
		return;

	if ((cpu_state == CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_data = address_switch & 0xff;
		fp_write(PC, fp_led_data);
		break;
	case FP_SW_DOWN:
		PC++;
		fp_led_address++;
		fp_led_data = address_switch & 0xff;
		fp_write(PC, fp_led_data);
		break;
	default:
		break;
	}
}

/*
 *	Callback for POWER switch
 */
void power_clicked(int state, int val)
{
	UNUSED(val);

	switch (state) {
	case FP_SW_UP:
		if (power)
			break;
		power++;
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		fp_led_address = PC;
		fp_led_data = getmem(PC);
		fp_led_wait = 1;
		fp_led_output = 0;
#ifdef UNIX_TERMINAL
		if (!isatty(fileno(stdout)) || (system("tput clear") == -1))
			puts("\r\n\r\n\r\n");
#endif
		break;
	case FP_SW_DOWN:
		if (!power)
			break;
		power--;
		cpu_switch = 0;
		cpu_state = STOPPED;
		cpu_error = POWEROFF;
		break;
	default:
		break;
	}
}

/*
 * Callback for quit (graphics window closed)
 */
void quit_callback(void)
{
	power--;
	cpu_switch = 0;
	cpu_state = STOPPED;
	cpu_error = POWEROFF;
}
#endif
