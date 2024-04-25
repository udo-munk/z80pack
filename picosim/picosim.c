/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/simmain.c
 */

/* Raspberry SDK includes */
#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

/* Project includes */
#include "sim.h"
#include "simglb.h"
#include "config.h"
#include "memory.h"

/* Pico W also needs this */
#ifdef PICO_W
#include "pico/cyw43_arch.h"
#endif

extern void init_cpu(void);
extern void run_cpu(void);
extern void report_cpu_error(void);

int main(void)
{
	stdio_init_all();	/* initialize Pico stdio */
	while (!tud_cdc_connected())
		sleep_ms(100);	/* wait until USB connected */

#ifdef PICO_W			/* initialize Pico W cyw43 hardware */
	if (cyw43_arch_init())
	{
		printf("CYW43 init failed\n");
		return -1;
	}
#else				/* initialize Pico hardware */
#define LED PICO_DEFAULT_LED_PIN	/* use builtin LED */
//#define LED 15			/* or another one */
	gpio_init(LED);		/* configure GPIO for LED output */
	gpio_set_dir(LED, GPIO_OUT);
#endif

	printf("Z80pack release %s, %s\n\n", RELEASE, COPYR);

	config();		/* read system configuration */
	init_cpu();		/* initialise CPU */
	init_memory();		/* initialise memory configuration */

	run_cpu();		/* run the CPU with whatever is in memory */

	/* switch builtin LED on */
#ifdef PICO_W
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#else
	gpio_put(LED, 1);
#endif

	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	putchar('\n');
	stdio_flush();
}
