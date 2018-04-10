/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * This module allows operation of the system from a Cromemco Z-1 front panel
 *
 * Copyright (C) 2014-2018 by Udo Munk
 *
 * History:
 * 15-DEC-14 first version
 * 20-DEC-14 added 4FDC emulation and machine boots CP/M 2.2
 * 28-DEC-14 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-15 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-15 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-15 fdc and tu-art improvements, implemented banked memory
 * 24-APR-15 added Cromemco DAZZLER to the machine
 * 01-MAR-16 added sleep for threads before switching tty to raw mode
 * 09-MAY-16 frontpanel configuration with path support
 * 06-DEZ-16 implemented status display and stepping for all machine cycles
 * 13-MAR-17 can't examine/deposit if CPU running HALT instruction
 * 29-JUN-17 system reset overworked
 */

#include <X11/Xlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#include "../../iodevices/cromemco-fdc.h"
#include "../../iodevices/cromemco-dazzler.h"
#include "../../iodevices/unix_terminal.h"

extern int load_file(char *);
extern int load_core(void);
extern void cpu_z80(void), cpu_8080(void), reset_cpu(void);

static BYTE fp_led_wait;
static BYTE fp_led_speed;
static int cpu_switch;
static int reset;
static int power;

static int load(void);
static void run_cpu(void), step_cpu(void);
static void run_clicked(int, int), step_clicked(int, int);
static void reset_clicked(int, int);
static void examine_clicked(int, int), deposit_clicked(int, int);
static void power_clicked(int, int);
static void quit_callback(void);

/*
 *	This function initialises the front panel and terminal.
 *	Boot code gets loaded if provided and then the machine
 *	waits to be operated from the front panel, until power
 *	switched OFF again.
 */
void mon(void)
{
	static struct timespec timer;
	static struct sigaction newact;

	/* load memory from file */
	if (load())
		exit(1);

	/* initialise front panel */
	XInitThreads();

	if (!fp_init2(&confdir[0], "panel.conf", fp_size)) {
		puts("frontpanel error");
		exit(1);
	}

	fp_addQuitCallback(quit_callback);
	fp_framerate(fp_fps);
	fp_bindSimclock(&fp_clock);
	fp_bindRunFlag(&cpu_state);

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

	/* give threads a bit time and then empty buffer */
	sleep(1);
	fflush(stdout);

	/* initialise terminal */
	set_unix_terminal();

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
					fp_led_data = *(mem_base() + PC);
				else
					fp_led_data = (int_data != -1) ?
							(BYTE) int_data : 0xff;
			}
		}

		/* set FDC autoboot flag from fp switch */
		if (address_switch & 256)
			fdc_flags |= 64;

		fp_clock++;
		fp_sampleData();

		/* run CPU if not ideling */
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

		/* wait a bit, system is idling */
		timer.tv_sec = 0;
		timer.tv_nsec = 10000000L;
		nanosleep(&timer, NULL);
	}

	/* timer interrupts off */
	newact.sa_handler = SIG_IGN;
	memset((void *) &newact.sa_mask, 0, sizeof(newact.sa_mask));
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, NULL);

	/* reset terminal */
	reset_unix_terminal();
	putchar('\n');

	/* all LED's off and update front panel */
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
	sleep(1);

	/* shutdown frontpanel */
	fp_quit();
}

/*
 *	Load code into memory from file, if provided
 */
int load(void)
{
	if (l_flag) {
		return(load_core());
	}

	if (x_flag) {
		return(load_file(xfn));
	}

	return(0);
}

/*
 *	Report CPU error
 */
void report_error(void)
{
	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		printf("\r\nINT disabled and HALT Op-Code reached at %04x\r\n",
		       PC - 1);
		break;
	case IOTRAPIN:
		printf("\r\nI/O input Trap at %04x, port %02x\r\n", PC, io_port);
		break;
	case IOTRAPOUT:
		printf("\r\nI/O output Trap at %04x, port %02x\r\n", PC, io_port);
		break;
	case IOHALT:
		printf("\nSystem halted, bye.\n");
		break;
	case IOERROR:
		printf("\r\nFatal I/O Error at %04x\r\n", PC);
		break;
	case OPTRAP1:
		printf("\r\nOp-code trap at %04x %02x\r\n", PC - 1,
		       *(mem_base() + PC - 1));
		break;
	case OPTRAP2:
		printf("\r\nOp-code trap at %04x %02x %02x\r\n",
		       PC - 2, *(mem_base() + PC - 2), *(mem_base() + PC - 1));
		break;
	case OPTRAP4:
		printf("\r\nOp-code trap at %04x %02x %02x %02x %02x\r\n",
		       PC - 4, *(mem_base() + PC - 4), *(mem_base() + PC - 3),
		       *(mem_base() + PC - 2), *(mem_base() + PC - 1));
		break;
	case USERINT:
		printf("\r\nUser Interrupt at %04x\r\n", PC);
		break;
	case INTERROR:
		printf("Unsupported bus data during INT: %02x\r\n", int_data);
		break;
	case POWEROFF:
		printf("\r\nSystem powered off, bye.\r\n");
		break;
	default:
		printf("\r\nUnknown error %d\r\n", cpu_error);
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
	val = val; /* to avoid compiler warning */

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
	val = val; /* to avoid compiler warning */

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
 * Singe step through the machine cycles after M1
 */
void wait_step(void)
{
	static struct timespec timer;

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
		fp_clock++;
		fp_sampleData();
		timer.tv_sec = 0;
		timer.tv_nsec = 10000000L;
		nanosleep(&timer, NULL);
	}

	cpu_bus &= ~CPU_M1;
	m1_step = 0;
}

/*
 * Singe step through interrupt machine cycles
 */
void wait_int_step(void)
{
	static struct timespec timer;

	if (cpu_state != SINGLE_STEP)
		return;

	cpu_switch = 3;

	while ((cpu_switch == 3) && !reset) {
		fp_clock++;
		fp_sampleData();
		timer.tv_sec = 0;
		timer.tv_nsec = 10000000L;
		nanosleep(&timer, NULL);
	}
}

/*
 *	Callback for RESET switch
 */
void reset_clicked(int state, int val)
{
	val = val; /* to avoid compiler warning */

	if (!power)
		return;

	switch (state) {
	case FP_SW_UP:
		reset = 1;
		cpu_state |= RESET;
		m1_step = 0;
		IFF = 0;
		fp_led_output = 0;
		break;
	case FP_SW_CENTER:
		if (reset) {
			/* reset I/O devices */
			cromemco_dazzler_off();
			cromemco_fdc_reset();

			/* reset CPU */
			reset = 0;
			reset_cpu();
			cpu_state &= ~RESET;

			/* update front panel */
			fp_led_address = 0;
			fp_led_data = *(mem_base());
			cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		}
		break;
	case FP_SW_DOWN:
		cromemco_dazzler_off();
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
	val = val; /* to avoid compiler warning */

	if (!power)
		return;

	if ((cpu_state == CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_address = address_switch;
		fp_led_data = *(mem_base() + address_switch);
		PC = address_switch;
		break;
	case FP_SW_DOWN:
		fp_led_address++;
		fp_led_data = *(mem_base() + fp_led_address);
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
	val = val; /* to avoid compiler warning */

	if (!power)
		return;

	if ((cpu_state == CONTIN_RUN) || (cpu_bus & CPU_HLTA))
		return;

	switch (state) {
	case FP_SW_UP:
		fp_led_data = *(mem_base() + PC) = address_switch & 0xff;
		break;
	case FP_SW_DOWN:
		PC++;
		fp_led_address++;
		fp_led_data = *(mem_base() + PC) = address_switch & 0xff;
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
	val = val; /* to avoid compiler warning */

	switch (state) {
	case FP_SW_UP:
		if (power)
			break;
		power++;
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
		fp_led_address = PC;
		fp_led_data = *(mem_base() + PC);
		fp_led_speed = (f_flag == 0 || f_flag >= 4) ? 1 : 0;
		fp_led_wait = 1;
		fp_led_output = 0;
		if (isatty(1))
			system("tput clear");
		else {
			puts("\r\n\r\n\r\n");
			fflush(stdout);
		}
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
