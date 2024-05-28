/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk & Thomas Eberhardt
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/simmain.c
 *
 * History:
 * 28-APR-2024 implemented first release of Z80 emulation
 * 09-MAY-2024 test 8080 emulation
 * 27-MAY-2024 add access to files on MicroSD
 * 28-MAY-2024 implemented boot from disk images with some OS
 */

/* Raspberry SDK and FatFS includes */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sd_card.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

/* Project includes */
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "memsim.h"
#include "sd-fdc.h"

/* Pico W also needs this */
#if PICO == 1
#include "pico/cyw43_arch.h"
#endif

#define SWITCH_BREAK 15 /* switch we use to interrupt the system */

#define BS  0x08 /* backspace */
#define DEL 0x7f /* delete */

/* global variables for access to SPI MicroSD drive */
sd_card_t *SD;	/* one MicroSD drive */
FIL sd_file;	/* at any time we have only one file open */
FRESULT sd_res;	/* result code from FatFS */
char disks[2][22]; /* path name for 2 disk images /DISKS80/filename.BIN */


extern void init_cpu(void), run_cpu(void);
extern void report_cpu_error(void), report_cpu_stats(void);
extern BYTE read_sec(int, int, int, WORD);

uint64_t get_clock_us(void);
void gpio_callback(uint, uint32_t);

int main(void)
{
	BYTE stat;

	stdio_init_all();	/* initialize Pico stdio */

#if PICO == 1			/* initialize Pico W hardware */
	if (cyw43_arch_init())
	{
		printf("CYW43 init failed\n");
		return -1;
	}
#else				/* initialize Pico hardware */
#define LED PICO_DEFAULT_LED_PIN	/* use builtin LED */
//#define LED 14			/* or another one */
	gpio_init(LED);		/* configure GPIO for LED output */
	gpio_set_dir(LED, GPIO_OUT);
#endif
	gpio_init(SWITCH_BREAK); /* setupt interrupt for break switch */
	gpio_set_dir(SWITCH_BREAK, GPIO_IN);
	gpio_set_irq_enabled_with_callback(SWITCH_BREAK, GPIO_IRQ_EDGE_RISE,
					   true, &gpio_callback);

	/* print banner */
	printf("\fZ80pack release %s, %s\n", RELEASE, COPYR);
	printf("%s release %s\n", USR_COM, USR_REL);
	printf("%s\n\n", USR_CPR);

	/* try to mount SD card */
	SD = sd_get_by_num(0);
	sd_res = f_mount(&SD->fatfs, SD->pcName, 1);
	if (sd_res != FR_OK)
		panic("f_mount error: %s (%d)\n", FRESULT_str(sd_res), sd_res);

	/* setup speed of the CPU */
	f_flag = CPU_SPEED;
	tmax = CPU_SPEED * 10000; /* theoretically */

	/* tell us what we are using */
	printf("CPU is %s\n", cpu == Z80 ? "Z80" : "8080");
	if (f_flag > 0)
		printf("CPU speed is %d MHz", f_flag);
	else
		printf("CPU speed is unlimited");
#ifndef UNDOC_INST
	printf(", CPU doesn't execute undocumented instructions\n");
#else
	printf(", CPU executes undocumented instructions\n");
#endif
	printf("\n");

	init_cpu();		/* initialize CPU */
	init_memory();		/* initialize memory configuration */
NOPE:	config();		/* configure the machine */

	/* if there is a disk in drive 0 try to boot from it */
	if (strlen(disks[0]) != 0) {
		/* they will try this for sure, so ... */
		if (!strcmp(disks[0], disks[1])) {
			printf("Not with this config dude\n");
			goto NOPE;
		}
		stat = read_sec(0, 0, 1, 0); /* read track 0 sector 1 */
		if (stat != FDC_STAT_OK) {
			printf("Disk 0 read error: %d\n", stat);
			f_unmount(SD->pcName);
			stdio_flush();
			return (0);
		}
	}

	/* run the CPU with whatever is in memory */
#ifdef WANT_ICE
	extern void ice_cmd_loop(int);

	ice_cmd_loop(0);
#else
	run_cpu();
#endif

	/* unmount SD card */
	f_unmount(SD->pcName);

	/* switch builtin LED on */
#if PICO == 1
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#else
	gpio_put(LED, 1);
#endif

#ifndef WANT_ICE
	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	report_cpu_stats();	/* print some execution statistics */
#endif
	putchar('\n');
	stdio_flush();
	return (0);
}

uint64_t get_clock_us(void)
{
	return (to_us_since_boot(get_absolute_time()));
}

/*
 * interrupt handler for break switch
 * stops CPU
 */
void gpio_callback(uint gpio, uint32_t events)
{
	cpu_error = USERINT;
	cpu_state = STOPPED;
}

/*
 * read an ICE or config command line from the terminal
 */
int get_cmdline(char *buf, int len)
{
	int i = 0;
	char c;

	while (i < len - 1) {
		c = getchar();
		if ((c == BS) || (c == DEL)) {
			if (i >= 1) {
				putchar(BS);
				putchar(' ');
				putchar(BS);
				i--;
			}
		} else if (c != '\r') {
			buf[i] = c;
			putchar(c);
			i++;
		} else {
			putchar('\r');
			putchar('\n');
			break;
		}
	}
	buf[i] = '\0';
	return (0);
}
