/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024-2025 by Udo Munk & Thomas Eberhardt
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
 * 16-MAY-2025 connect WiFi and use NTP on Pico W
 */

/* Raspberry SDK and FatFS includes */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if LIB_PICO_STDIO_USB || LIB_STDIO_MSC_USB
#include <tusb.h>
#endif
#if defined(RASPBERRYPI_PICO_W) || defined(RASPBERRYPI_PICO2_W)
#include "net_ntp.h"
#include "pico/cyw43_arch.h"
#endif
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"

#include "gpio.h"
#include "hw_config.h"
#include "my_rtc.h"

/* Project includes */
#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcfg.h"
#include "simmem.h"
#include "simcore.h"
#include "simport.h"
#include "simio.h"
#ifdef WANT_ICE
#include "simice.h"
#endif

#include "disks.h"
#include "rgbled.h"
#include "picosim.h"

#ifdef WANT_ICE
static void picosim_ice_cmd(char *cmd, WORD *wrk_addr);
static void picosim_ice_help(void);
#endif

#define BS  0x08 /* ASCII backspace */
#define DEL 0x7f /* ASCII delete */

/* CPU speed */
int speed = CPU_SPEED;

/* PIO and sm used for RGB LED */
PIO pio = pio1;
uint sm;

/*
 *	callback for TinyUSB when terminal sends a break
 *	stops CPU
 */
#if LIB_PICO_STDIO_USB || (LIB_STDIO_MSC_USB && !STDIO_MSC_USB_DISABLE_STDIO)
void tud_cdc_send_break_cb(uint8_t itf, uint16_t duration_ms)
{
	UNUSED(itf);
	UNUSED(duration_ms);

	cpu_error = USERINT;
	cpu_state = ST_STOPPED;
}
#endif

/*
 *	interrupt handler for break switch
 *	stops CPU
 */
static void gpio_callback(uint gpio, uint32_t events)
{
	UNUSED(gpio);
	UNUSED(events);

	cpu_error = USERINT;
	cpu_state = ST_STOPPED;
}

/*
 *	read the onboard temperature sensor
 */
float read_onboard_temp(void)
{
	/* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
	const float conversionFactor = 3.3f / (1 << 12);

	float adc = (float) adc_read() * conversionFactor;
	float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

	return tempC;
}

int main(void)
{
	char s[2];
	uint32_t rgb = 0x005500;/* initial value for the RGB LED */

	/* strings for picotool, so that it shows used pins */
	bi_decl(bi_1pin_with_name(SWITCH_BREAK, "Interrupt switch"));
	bi_decl(bi_1pin_with_name(WS2812_PIN, "WS2812 RGB LED"));
	bi_decl(bi_4pins_with_names(SD_SPI_CLK, "SD card CLK",
				    SD_SPI_SI, "SD card SI",
				    SD_SPI_SO, "SD card SO",
				    SD_SPI_CS, "SD card CS"));
	bi_decl(bi_2pins_with_names(DS3231_I2C_SDA_PIN, "DS3231 I2C SDA",
				    DS3231_I2C_SCL_PIN, "DS3231 I2C SCL"));

	stdio_init_all();	/* initialize stdio */
#if LIB_STDIO_MSC_USB
	sd_init_driver();	/* initialize SD card driver */
	tusb_init();		/* initialize TinyUSB */
	stdio_msc_usb_init();	/* initialize MSC USB stdio */
#endif
	time_init();		/* initialize FatFS RTC */

	/*
	 * initialize hardware AD converter, enable onboard
	 * temperature sensor and select its channel
	 */
	adc_init();
	adc_set_temp_sensor_enabled(true);
	adc_select_input(4);

	/* setup interrupt for break switch */
	gpio_init(SWITCH_BREAK);
	gpio_set_dir(SWITCH_BREAK, GPIO_IN);
	gpio_set_irq_enabled_with_callback(SWITCH_BREAK, GPIO_IRQ_EDGE_RISE,
					   true, &gpio_callback);

	/* initialize RGB LED */
	sm = pio_claim_unused_sm(pio, true);
	uint offset = pio_add_program(pio, &ws2812_program);
	ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, true);
	put_pixel(rgb); /* LED red */

#if LIB_PICO_STDIO_UART
	uart_inst_t *my_uart = uart_default;
	/* destroy random input from UART after activation */
	if (uart_is_readable(my_uart))
		getchar();
#endif

	/* when using USB UART wait until it is connected */
	/* but also get out if there is input at default UART */
#if LIB_PICO_STDIO_USB || (LIB_STDIO_MSC_USB && !STDIO_MSC_USB_DISABLE_STDIO)
	while (!tud_cdc_connected()) {
#if LIB_PICO_STDIO_UART
		if (uart_is_readable(my_uart)) {
			getchar();
			break;
		}
#endif
		rgb = rgb - 0x000100;	/* while waiting make */
		if (rgb == 0)		/* RGB LED fading red */
			rgb = 0x005500;
		put_pixel(rgb);
		sleep_ms(50);
	}
#endif
	put_pixel(0x000044); /* LED blue */

	/* print banner */
	printf("\fZ80pack release %s, %s\n", RELEASE, COPYR);
	printf("%s release %s\n", USR_COM, USR_REL);
#if PICO_RP2350
#if PICO_RISCV
	printf("running on Hazard3 RISC-V cores at %i MHz\n", SYS_CLK_MHZ);
#else
	printf("running on ARM Cortex-M33 cores at %i MHz\n", SYS_CLK_MHZ);
#endif
#else
	printf("running on ARM Cortex-M0+ cores at %i MHz\n", SYS_CLK_MHZ);
#endif
	printf("%s\n\n", USR_CPR);

#if defined(RASPBERRYPI_PICO_W) || defined(RASPBERRYPI_PICO2_W)
	/* initialize Pico W WiFi hardware */
	if (cyw43_arch_init())
		panic("CYW43 init failed\n");

	/* try to connect WiFi */
	int wifi_retry = 5;	/* max Wifi connect retries */
	int result;

	cyw43_arch_enable_sta_mode();
	printf("Connecting to Wi-Fi... ");
	if (!strlen(WIFI_SSID)) {
		puts("no WiFi SSID defined.");
		goto wifi_done;
	}
	while (wifi_retry) {
		if ((result = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
		    CYW43_AUTH_WPA2_AES_PSK, 30000))) {
			printf("retry... ");
			wifi_retry--;
		} else
			break;
	}

	if (result) {
		puts("failed.");
	} else {
		puts("connected.");
		do_ntp();
	}
wifi_done:
#endif

	init_cpu();		/* initialize CPU */
	PC = 0xff00;		/* power on jump into the boot ROM */
	init_disks();		/* initialize disk drives */
	init_memory();		/* initialize memory configuration */
	init_io();		/* initialize I/O devices */
	config();		/* configure the machine */

	f_value = speed;	/* setup speed of the CPU */
	if (f_value)
		tmax = speed * 10000;	/* theoretically */
	else
		tmax = 100000;	/* for periodic CPU accounting updates */

	put_pixel(0x440000);	/* LED green */

	/* run the CPU with whatever is in memory */
#ifdef WANT_ICE
	ice_cust_cmd = picosim_ice_cmd;
	ice_cust_help = picosim_ice_help;
	ice_cmd_loop(0);
#else
	run_cpu();
#endif

	put_pixel(0x000000);	/* LED off */
	exit_io();		/* stop I/O devices */
	exit_disks();		/* stop disk drives */

#if defined(RASPBERRYPI_PICO_W) || defined(RASPBERRYPI_PICO2_W)
	/* de-initialize Pico W WiFi hardware */
	cyw43_arch_disable_sta_mode();
	cyw43_arch_deinit();
#endif

#ifndef WANT_ICE
	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	report_cpu_stats();	/* print some execution statistics */
#endif
	puts("\nPress any key to restart CPU");
	get_cmdline(s, 2);

	/* reset machine */
	watchdog_reboot(0, 0, 0);
	while (true) {
		__nop();
	}
}

/*
 * Read an ICE or config command line of maximum length len - 1
 * from the terminal. For single character requests (len == 2),
 * returns immediately after input is received.
 */
bool get_cmdline(char *buf, int len)
{
	int i = 0;
	char c;

	while (true) {
		c = getchar();
		if ((c == BS) || (c == DEL)) {
			if (i >= 1) {
				putchar(BS);
				putchar(' ');
				putchar(BS);
				i--;
			}
		} else if (c != '\r') {
			if (i < len - 1) {
				buf[i++] = c;
				putchar(c);
				if (len == 2)
					break;
			}
		} else {
			break;
		}
	}
	buf[i] = '\0';
	putchar('\n');
	return true;
}

#ifdef WANT_ICE

/*
 *	This function is the callback for the alarm.
 *	The CPU emulation is stopped here.
 */
static int64_t timeout(alarm_id_t id, void *user_data)
{
	UNUSED(id);
	UNUSED(user_data);

	cpu_state = ST_STOPPED;
	return 0;
}

static void picosim_ice_cmd(char *cmd, WORD *wrk_addr)
{
	char *s;
	BYTE save[3];
	WORD save_PC;
	Tstates_t T0;
	unsigned freq;
#ifdef WANT_HB
	bool save_hb_flag;
#endif

	switch (tolower((unsigned char) *cmd)) {
	case 'c':
		/*
		 *	Calculate the clock frequency of the emulated CPU:
		 *	into memory locations 0000H to 0002H the following
		 *	code will be stored:
		 *		LOOP: JP LOOP
		 *	It uses 10 T states for each execution. A 3 second
		 *	alarm is set and then the CPU started. For every JP
		 *	the T states counter is incremented by 10 and after
		 *	the timer is down and stops the emulation, the clock
		 *	speed of the CPU in MHz is calculated with:
		 *		f = (T - T0) / 3000000
		 */

#ifdef WANT_HB
		save_hb_flag = hb_flag;
		hb_flag = false;
#endif
		save[0] = getmem(0x0000); /* save memory locations */
		save[1] = getmem(0x0001); /* 0000H - 0002H */
		save[2] = getmem(0x0002);
		putmem(0x0000, 0xc3);	/* store opcode JP 0000H at address */
		putmem(0x0001, 0x00);	/* 0000H */
		putmem(0x0002, 0x00);
		save_PC = PC;		/* save PC */
		PC = 0;			/* set PC to this code */
		T0 = T;			/* remember start clock counter */
		add_alarm_in_ms(3000, timeout, /* set 3 second alarm */
				NULL, true);
		run_cpu();		/* start CPU */
		PC = save_PC;		/* restore PC */
		putmem(0x0000, save[0]); /* restore memory locations */
		putmem(0x0001, save[1]); /* 0000H - 0002H */
		putmem(0x0002, save[2]);
#ifdef WANT_HB
		hb_flag = save_hb_flag;
#endif
#ifndef EXCLUDE_Z80
		if (cpu == Z80)
			s = "JP";
#endif
#ifndef EXCLUDE_I8080
		if (cpu == I8080)
			s = "JMP";
#endif
		if (cpu_error == NONE) {
			freq = (unsigned) ((T - T0) / 30000);
			printf("CPU executed %" PRIu64 " %s instructions "
			       "in 3 seconds\n", (T - T0) / 10, s);
			printf("clock frequency = %u.%02u MHz\n",
			       freq / 100, freq % 100);
		} else
			puts("Interrupted by user");
		break;

	case 'r':
		cmd++;
		while (isspace((unsigned char) *cmd))
			cmd++;
		for (s = cmd; *s; s++)
			*s = toupper((unsigned char) *s);
		if (load_file(cmd))
			*wrk_addr = PC = 0;
		break;

	case '!':
		cmd++;
		while (isspace((unsigned char) *cmd))
			cmd++;
		if (strcasecmp(cmd, "ls") == 0)
			list_files("/CODE80", "*.BIN");
		else
			puts("what??");
		break;

	default:
		puts("what??");
		break;
	}
}

static void picosim_ice_help(void)
{
	puts("c                         measure clock frequency");
	puts("r filename                read file (without .BIN) into memory");
	puts("! ls                      list files");
}

#endif
