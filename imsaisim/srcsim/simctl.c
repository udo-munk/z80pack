/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * This module allows operation of the system from an IMSAI 8080 front panel
 *
 * Copyright (C) 2008-2018 by Udo Munk
 *
 * History:
 * 20-OCT-08 first version finished
 * 26-OCT-08 corrected LED status while RESET is hold in upper position
 * 27-JAN-14 set IFF=0 when powered off, so that LED goes off
 * 02-MAR-14 source cleanup and improvements
 * 15-APR-14 added fflush() for teletype
 * 19-APR-14 moved CPU error report into a function
 * 06-JUN-14 forgot to disable timer interrupts when machine switched off
 * 10-JUN-14 increased fp operation timer from 1ms to 100ms
 * 09-MAY-15 added Cromemco DAZZLER to the machine
 * 01-MAR-16 added sleep for threads before switching tty to raw mode
 * 08-MAY-16 frontpanel configuration with path support
 * 06-DEC-16 implemented status display and stepping for all machine cycles
 * 26-JAN-17 bugfix for DATA LED's not always showing correct bus data
 * 13-MAR-17 can't examine/deposit if CPU running HALT instruction
 * 29-JUN-17 system reset overworked
 * 10-APR-18 trap CPU on unsupported bus data during interrupt
 * 17-MAY-18 improved hardware control
 * 08-JUN-18 moved hardware initialisation and reset to iosim
 * 11-JUN-18 fixed reset so that cold and warm start works
 * 12-JUL-18 use logging
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

extern void cpu_z80(void), cpu_8080(void);
extern void reset_cpu(void), reset_io(void);

static const char *TAG = "system";

static BYTE fp_led_wait;
static int cpu_switch;
static int reset;
static int power;

static void run_cpu(void), step_cpu(void);
static void run_clicked(int, int), step_clicked(int, int);
static void reset_clicked(int, int);
static void examine_clicked(int, int), deposit_clicked(int, int);
static void power_clicked(int, int);
static void quit_callback(void);

/*
 *	This function initialises the front panel and terminal.
 *	Then the machine waits to be operated from the front panel,
 *	until power switched OFF again.
 */
void mon(void)
{
#ifdef HAS_NETSERVER
	extern int start_net_services (void);
	start_net_services();
#endif

	/* initialise front panel */
	XInitThreads();

	if (!fp_init2(&confdir[0], "panel.conf", fp_size)) {
		LOGE(TAG, "frontpanel error");
		exit(1);
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

#ifdef UNIX_TERMINAL
	/* give threads a bit time and then empty buffer */
	SLEEP_MS(999);
	fflush(stdout);

	/* initialise terminal */
	set_unix_terminal();
#endif

#ifdef HAS_BANKED_ROM
	if(r_flag)
		PC = 0x0000;
#endif

	/* operate machine from front panel */
	while (cpu_error == NONE) {
		if (reset) {
			cpu_bus = 0xff;
			fp_led_address = 0xffff;
			fp_led_data = 0xff;
		} else {
			if (power) {
				fp_led_address = PC;
				if (!(cpu_bus & CPU_INTA))
					fp_led_data = dma_read(PC);
				else
					fp_led_data = (int_data != -1) ?
							(BYTE) int_data : 0xff;
			}
		}

		fp_clock++;
		fp_sampleData();

		switch (cpu_switch) {
		case 1:
			if (!reset) run_cpu();
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

		SLEEP_MS(10);
	}

#ifdef UNIX_TERMINAL
	/* reset terminal */
	reset_unix_terminal();
	putchar('\n');
#endif

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
}

/*
 *	Report CPU error
 */
void report_error(void)
{
	LOG(TAG, "\r\n");

	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		LOG(TAG, "INT disabled and HALT Op-Code reached at %04x\r\n",
		    PC - 1);
		break;
	case IOTRAPIN:
		LOGE(TAG, "I/O input Trap at %04x, port %02x\r\n",
		     PC, io_port);
		break;
	case IOTRAPOUT:
		LOGE(TAG, "I/O output Trap at %04x, port %02x\r\n",
		     PC, io_port);
		break;
	case IOHALT:
		LOG(TAG, "System halted, bye.\r\n");
		break;
	case IOERROR:
		LOGE(TAG, "Fatal I/O Error at %04x\r\n", PC);
		break;
	case OPTRAP1:
		LOGE(TAG, "Op-code trap at %04x %02x\r\n",
		     PC - 1 , *(mem_base() + PC - 1));
		break;
	case OPTRAP2:
		LOGE(TAG, "Op-code trap at %04x %02x %02x\r\n",
		     PC - 2, *(mem_base() + PC - 2), *(mem_base() + PC - 1));
		break;
	case OPTRAP4:
		LOGE(TAG, "Op-code trap at %04x %02x %02x %02x %02x\r\n",
		       PC - 4, *(mem_base() + PC - 4), *(mem_base() + PC - 3),
		       *(mem_base() + PC - 2), *(mem_base() + PC - 1));
		break;
	case USERINT:
		LOG(TAG, "User Interrupt at %04x\r\n", PC);
		break;
	case INTERROR:
		LOGW(TAG, "Unsupported bus data during INT: %02x\r\n",
		     int_data);
		break;
	case POWEROFF:
		LOG(TAG, "System powered off, bye.\r\n");
		break;
	default:
		LOGW(TAG, "Unknown error %d\r\n", cpu_error);
		break;
	}
}

/*
 *	Run CPU
 */
void run_cpu(void)
{
	cpu_state = CONTIN_RUN;
	cpu_error = NONE;
	switch(cpu) {
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}
	report_error();
}

/*
 *	Step CPU
 */
void step_cpu(void)
{
	cpu_state = SINGLE_STEP;
	cpu_error = NONE;
	switch(cpu) {
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}
	cpu_state = STOPPED;
	report_error();
}

/*
 *	Callback for RUN/STOP switch
 */
void run_clicked(int state, int val)
{
	val = val;	/* to avoid compiler warning */

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
	val = val;	/* to avoid compiler warning */

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
void wait_step(void)
{
	if (cpu_state != SINGLE_STEP) {
		cpu_bus &= ~CPU_M1;
		m1_step = 0;
		return;
	}

	if ((cpu_bus & CPU_M1) && !m1_step) {
		cpu_bus &= ~CPU_M1;
		return;
	}

	cpu_switch = 3;

	while ((cpu_switch == 3) && !reset) {
		/* when INP on port 0FFh - feed Programmed Input
		   toggles to Data Bus LEDs */
		if ((cpu_bus == (CPU_WO | CPU_INP)) &&
		    (fp_led_address == 0xffff)) {
			fp_led_data = address_switch >> 8;
		}
		fp_clock++;
		fp_sampleData();
		SLEEP_MS(10);
	}

	cpu_bus &= ~CPU_M1;
	m1_step = 0;
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
	val = val;	/* to avoid compiler warning */

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
			fp_led_data = dma_read(0);
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
	val = val;	/* to avoid compiler warning */

	if (!power)
		return;

	if ((cpu_state == CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_address = address_switch;
		fp_led_data = dma_read(address_switch);
		PC = address_switch;
		break;
	case FP_SW_DOWN:
		fp_led_address++;
		fp_led_data = dma_read(fp_led_address);
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
	val = val;	/* to avoid compiler warning */

	if (!power)
		return;

	if ((cpu_state == CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_data = address_switch & 0xff;
		dma_write(PC, fp_led_data);
		break;
	case FP_SW_DOWN:
		PC++;
		fp_led_address++;
		fp_led_data = address_switch & 0xff;
		dma_write(PC, fp_led_data);
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
	val = val;	/* to avoid compiler warning */

	switch (state) {
	case FP_SW_UP:
		if (power)
			break;
		power++;
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		fp_led_address = PC;
		fp_led_data = dma_read(PC);
		fp_led_wait = 1;
		fp_led_output = 0;
#ifdef UNIX_TERMINAL
		if (isatty(1))
			system("tput clear");
		else
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
