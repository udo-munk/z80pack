/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk & Thomas Eberhardt
 *
 * This module configures the machine appropriate for the
 * Z80/8080 software we want to run on it.
 *
 * History:
 * 20-APR-2024 dummy, no configuration implemented yet
 * 12-MAY-2024 implemented configuration dialog
 * 27-MAY-2024 implemented load file
 * 28-MAY-2024 implemented mount/unmount of disk images
 * 03-JUN-2024 added directory list for code files and disk images
 * 02-SEP-2024 read date/time from an optional I2C battery backed RTC
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/aon_timer.h"
#include "hardware/i2c.h"

#include "gpio.h"
#include "ff.h"
#include "ds3231.h"
#if LIB_STDIO_MSC_USB
#include "stdio_msc_usb.h"
#endif

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simport.h"
#include "simio.h"
#include "simcfg.h"

#include "disks.h"
#include "picosim.h"

/*
 * prompt for a filename
 */
static void prompt_fn(char *s, const char *ext)
{
	printf("Filename (without .%s): ", ext);
	get_cmdline(s, 9);
	while (*s) {
		*s = toupper((unsigned char) *s);
		s++;
	}
}

/*
 * get an integer with range check
 */
static int get_int(const char *prompt, const char *hint,
		   int min_val, int max_val)
{
	int i;
	char s[6];

	for (;;) {
		printf("Enter %s%s: ", prompt, hint);
		get_cmdline(s, 5);
		if (s[0] == '\0')
			return -1;
		i = atoi(s);
		if (i < min_val || i > max_val) {
			printf("Invalid %s: range %d - %d\n",
			       prompt, min_val, max_val);
		} else
			return i;
	}
}

/*
 * Configuration dialog for the machine
 */
void config(void)
{
	const char *cfg = "/CONF80/" CONF_FILE;
	const char *cpath = "/CODE80";
	const char *cext = "*.BIN";
	const char *dpath = "/DISKS80";
	const char *dext = "*.DSK";
	char s[10];
	unsigned int br;
	int go_flag = 0;
	int i, n, menu;
	struct tm t = { .tm_year = 124, .tm_mon = 0, .tm_mday = 1,
			.tm_wday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0,
			.tm_isdst = -1 };
	static const char *dotw[7] = { "Sun", "Mon", "Tue", "Wed",
				       "Thu", "Fri", "Sat" };
	struct timespec ts;
	struct ds3231_rtc rtc;
	ds3231_datetime_t dt;
	uint8_t buf;
	UNUSED(DS3231_MONTHS);
	UNUSED(DS3231_WDAYS);

	/* try to read config file */
	sd_res = f_open(&sd_file, cfg, FA_READ);
	if (sd_res == FR_OK) {
		f_read(&sd_file, &cpu, sizeof(cpu), &br);
		f_read(&sd_file, &speed, sizeof(speed), &br);
		f_read(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_read(&sd_file, &t, sizeof(t), &br);
		f_read(&sd_file, &disks[0], DISKLEN, &br);
		f_read(&sd_file, &disks[1], DISKLEN, &br);
		f_read(&sd_file, &disks[2], DISKLEN, &br);
		f_read(&sd_file, &disks[3], DISKLEN, &br);
		f_close(&sd_file);
#if defined(EXCLUDE_I8080) || defined(EXCLUDE_Z80)
		cpu = DEF_CPU;
#endif
	}

	/* Create a real-time clock structure and initiate this */
	ds3231_init(DS3231_I2C_PORT, DS3231_I2C_SDA_PIN,
		    DS3231_I2C_SCL_PIN, &rtc);

	/* Try to read the DS3231 RTC status register */
	buf = DS3231_STATUS_REG;
	i2c_write_blocking(rtc.i2c_port, rtc.i2c_addr, &buf, 1, true);
	if (i2c_read_blocking(rtc.i2c_port, rtc.i2c_addr,
			      &buf, 1, false) == 1) {
		/* Read the date and time from the DS3231 RTC */
		ds3231_get_datetime(&dt, &rtc);
		t.tm_year = dt.year - 1900;
		t.tm_mon = dt.month - 1;
		t.tm_mday = dt.day;
		t.tm_hour = dt.hour;
		t.tm_min = dt.minutes;
		t.tm_sec = dt.seconds;
		if (dt.dotw < 7)
			t.tm_wday = dt.dotw;
		else
			t.tm_wday = 0;
	}

	ts.tv_sec = mktime(&t);
	ts.tv_nsec = 0;
	aon_timer_start(&ts);

	menu = 1;

	while (!go_flag) {
		if (menu) {
			aon_timer_get_time(&ts);
			localtime_r(&ts.tv_sec, &t);
			printf("Current time: %s %04d-%02d-%02d "
			       "%02d:%02d:%02d, Chip temperature: %.2f C\n",
			       dotw[t.tm_wday], t.tm_year + 1900, t.tm_mon + 1,
			       t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
			       read_onboard_temp());
			printf("a - set date\n");
			printf("t - set time\n");
#if LIB_STDIO_MSC_USB
			printf("u - enable USB mass storage access\n");
#endif
			printf("c - switch CPU, currently ");
#ifndef EXCLUDE_Z80
			if (cpu == Z80)
				puts("Z80");
#endif
#ifndef EXCLUDE_I8080
			if (cpu == I8080)
				puts("8080");
#endif
			printf("s - CPU speed: ");
			if (speed == 0)
				puts("unlimited");
			else
				printf("%d MHz\n", speed);
			printf("p - Port 255 value: %02XH\n", fp_value);
			printf("f - list files\n");
			printf("r - load file\n");
			printf("d - list disks\n");
			printf("0 - Disk 0: %s\n", disks[0]);
			printf("1 - Disk 1: %s\n", disks[1]);
			printf("2 - Disk 2: %s\n", disks[2]);
			printf("3 - Disk 3: %s\n", disks[3]);
			printf("g - run machine\n\n");
		} else
			menu = 1;
		printf("Command: ");
		get_cmdline(s, 2);
		putchar('\n');

		switch (tolower((unsigned char) s[0])) {
		case 'a':
			n = 0;
			aon_timer_get_time(&ts);
			localtime_r(&ts.tv_sec, &t);
			ds3231_get_datetime(&dt, &rtc);
			if ((i = get_int("weekday", " (0=Sun)", 0, 6)) >= 0) {
				t.tm_wday = i;
				if (i == 0)
					dt.dotw = 7;
				else
					dt.dotw = i;
				n++;
			}
			if ((i = get_int("year", "", 0, 4095)) >= 0) {
				t.tm_year = i - 1900;
				dt.year = i;
				n++;
			}
			if ((i = get_int("month", "", 1, 12)) >= 0) {
				t.tm_mon = i - 1;
				dt.month = i;
				n++;
			}
			if ((i = get_int("day", "", 1, 31)) >= 0) {
				t.tm_mday = i;
				dt.day = i;
				n++;
			}
			if (n > 0) {
				ds3231_set_datetime(&dt, &rtc);
				ts.tv_sec = mktime(&t);
				aon_timer_set_time(&ts);
			}
			putchar('\n');
			break;

		case 't':
			n = 0;
			aon_timer_get_time(&ts);
			localtime_r(&ts.tv_sec, &t);
			ds3231_get_datetime(&dt, &rtc);
			if ((i = get_int("hour", "", 0, 23)) >= 0) {
				t.tm_hour = i;
				dt.hour = i;
				n++;
			}
			if ((i = get_int("minute", "", 0, 59)) >= 0) {
				t.tm_min = i;
				dt.minutes = i;
				n++;
			}
			if ((i = get_int("second", "", 0, 59)) >= 0) {
				t.tm_sec = i;
				dt.seconds = i;
				n++;
			}
			if (n > 0) {
				ds3231_set_datetime(&dt, &rtc);
				ts.tv_sec = mktime(&t);
				aon_timer_set_time(&ts);
			}
			putchar('\n');
			break;

#if LIB_STDIO_MSC_USB
		case 'u':
			exit_disks();
			puts("Waiting for disk to be ejected");
			stdio_msc_usb_do_msc();
			puts("Disk ejected\n");
			init_disks();
			check_disks();
			break;
#endif

		case 'c':
#if defined(EXCLUDE_I8080) || defined(EXCLUDE_Z80)
			puts("Can't switch CPU");
#else
			if (cpu == Z80)
				switch_cpu(I8080);
			else
				switch_cpu(Z80);
#endif
			break;

		case 's':
			i = get_int("speed", " in MHz (0=unlimited)", 0, 40);
			putchar('\n');
			if (i >= 0)
				speed = i;
			break;

		case 'p':
again:
			printf("Enter value in Hex: ");
			get_cmdline(s, 3);
			if (s[0]) {
				if (!isxdigit((unsigned char) s[0]) ||
				    !isxdigit((unsigned char) s[1])) {
					puts("Invalid value: range 00 - FF");
					goto again;
				}
				fp_value = (s[0] <= '9' ? s[0] - '0' :
					    toupper((unsigned char) s[0]) -
					    'A' + 10) << 4;
				fp_value += (s[1] <= '9' ? s[1] - '0' :
					     toupper((unsigned char) s[1]) -
					     'A' + 10);
			}
			putchar('\n');
			break;

		case 'f':
			list_files(cpath, cext);
			putchar('\n');
			menu = 0;
			break;

		case 'r':
			prompt_fn(s, "bin");
			if (s[0] && load_file(s))
				PC = 0;
			putchar('\n');
			menu = 0;
			break;

		case 'd':
			list_files(dpath, dext);
			putchar('\n');
			menu = 0;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
			i = s[0] - '0';
			prompt_fn(s, "dsk");
			if (s[0])
				mount_disk(i, s);
			else {
				disks[i][0] = '\0';
				putchar('\n');
			}
			break;

		case 'g':
			go_flag = 1;
			break;

		default:
			break;
		}
	}

	/* try to save config file */
	sd_res = f_open(&sd_file, cfg, FA_WRITE | FA_CREATE_ALWAYS);
	if (sd_res == FR_OK) {
		f_write(&sd_file, &cpu, sizeof(cpu), &br);
		f_write(&sd_file, &speed, sizeof(speed), &br);
		f_write(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_write(&sd_file, &t, sizeof(t), &br);
		f_write(&sd_file, &disks[0], DISKLEN, &br);
		f_write(&sd_file, &disks[1], DISKLEN, &br);
		f_write(&sd_file, &disks[2], DISKLEN, &br);
		f_write(&sd_file, &disks[3], DISKLEN, &br);
		f_close(&sd_file);
	}
}
