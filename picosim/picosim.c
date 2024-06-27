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
 * 31-MAY-2024 use USB UART
 * 09-JUN-2024 implemented boot ROM
 */

/* Raspberry SDK and FatFS includes */
#include <stdio.h>
#include <string.h>
#if LIB_PICO_STDIO_USB
#include <tusb.h>
#endif
#include "pico/stdlib.h"
#include "pico/time.h"
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

/* global variables for access to MicroSD card */

/* Configuration of RP2040 hardware SPI object */
static spi_t spi = {  
	.hw_inst = spi0,  /* RP2040 SPI component */
	.sck_gpio = 18,   /* GPIO number (not Pico pin number) */
	.mosi_gpio = 19,
	.miso_gpio = 16,
	.baud_rate = 12 * 1000 * 1000 /* Actual frequency: 10416666 */
};

/* SPI Interface */
static sd_spi_if_t spi_if = {
	.spi = &spi,  /* Pointer to the SPI driving this card */
	.ss_gpio = 17 /* The SPI slave select GPIO for this SD card */
};

/* Configuration of the SD Card socket object */
static sd_card_t sd_card = {   
	.type = SD_IF_SPI,
	.spi_if_p = &spi_if /* Pointer to the SPI interface driving this card */
};

FATFS fs;       /* FatFs on MicroSD */
FIL sd_file;	/* at any time we have only one file open */
FRESULT sd_res;	/* result code from FatFS */
char disks[2][22]; /* path name for 2 disk images /DISKS80/filename.DSK */

/* CPU speed */
int speed = CPU_SPEED;

extern void init_cpu(void), init_io(void), run_cpu(void);
extern void report_cpu_error(void), report_cpu_stats(void);

uint64_t get_clock_us(void);
void gpio_callback(uint, uint32_t);

/* Callbacks used by the SD library */
size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
	if (num == 0) {
		return &sd_card;
	} else {
		return NULL;
	}
}

int main(void)
{
	stdio_init_all();	/* initialize Pico stdio */

#if PICO == 1			/* initialize Pico W hardware */
	if (cyw43_arch_init())
	{
		printf("CYW43 init failed\n");
		return -1;
	}
#else				/* initialize Pico hardware */
	gpio_init(LED);		/* configure GPIO for LED output */
	gpio_set_dir(LED, GPIO_OUT);
#endif

	gpio_init(SWITCH_BREAK); /* setupt interrupt for break switch */
	gpio_set_dir(SWITCH_BREAK, GPIO_IN);
	gpio_set_irq_enabled_with_callback(SWITCH_BREAK, GPIO_IRQ_EDGE_RISE,
					   true, &gpio_callback);

	/* when using USB UART wait until it is connected */
#if LIB_PICO_STDIO_USB
	while (!tud_cdc_connected())
		sleep_ms(100);
#endif

	/* print banner */
	printf("\fZ80pack release %s, %s\n", RELEASE, COPYR);
	printf("%s release %s\n", USR_COM, USR_REL);
	printf("%s\n\n", USR_CPR);

	/* try to mount SD card */
	sd_res = f_mount(&fs, "", 1);
	if (sd_res != FR_OK)
		panic("f_mount error: %s (%d)\n", FRESULT_str(sd_res), sd_res);

	init_cpu();		/* initialize CPU */
	init_memory();		/* initialize memory configuration */
	init_io();		/* initialize I/O devices */
NOPE:	config();		/* configure the machine */

	/* check if there are disks in the drives */
	if (strlen(disks[0]) != 0) {
		/* they will try this for sure, so ... */
		if (!strcmp(disks[0], disks[1])) {
			printf("Not with this config dude\n");
			goto NOPE;
		}
	}

	/* setup speed of the CPU */
	f_flag = speed;
	tmax = speed * 10000; /* theoretically */

	/* power on jump into the boot ROM */
	PC = 0xff00;

	/* run the CPU with whatever is in memory */
#ifdef WANT_ICE
	extern void ice_cmd_loop(int);

	ice_cmd_loop(0);
#else
	run_cpu();
#endif

	/* unmount SD card */
	f_unmount("");

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
	sleep_ms(500);
	return 0;
}

uint64_t get_clock_us(void)
{
	return to_us_since_boot(get_absolute_time());
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
			buf[i++] = c;
			putchar(c);
		} else {
			break;
		}
	}
	buf[i++] = '\0';
	putchar('\n');
	return 0;
}
