/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
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
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "f_util.h"
#include "ff.h"
#include "sim.h"
#include "simglb.h"

extern FIL sd_file;
extern FRESULT sd_res;
extern char disks[2][22];
extern int speed;
extern BYTE fp_value;

extern int get_cmdline(char *, int);
extern void switch_cpu(int);
extern void load_file(char *);
extern void mount_disk(int, char *);
extern void my_ls(const char *, const char *);
extern unsigned char fp_value;

/*
 * prompt for a filename
 */
static void prompt_fn(char *s)
{
      printf("Filename: ");
      get_cmdline(s, 9);
}

/*
 * Configuration dialog for the machine
 */
void config(void)
{
	const char *cfg = "/CONF80/CFG.DAT";
	const char *cpath = "/CODE80";
	const char *cext = "*.BIN";
	const char *dpath = "/DISKS80";
	const char *dext = "*.DSK";
	char s[10];
	unsigned int br;
	int go_flag = 0;

	/* try to read config file */
	sd_res = f_open(&sd_file, cfg, FA_READ);
	if (sd_res == FR_OK) {
		f_read(&sd_file, &cpu, sizeof(cpu), &br);
		f_read(&sd_file, &speed, sizeof(speed), &br);
		f_read(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_read(&sd_file, &disks[0], 22, &br);
		f_read(&sd_file, &disks[1], 22, &br);
		f_close(&sd_file);
	}

	while (!go_flag) {
		printf("1 - switch CPU, currently %s\n", (cpu == Z80) ?
							  "Z80" : "8080");
		printf("2 - CPU speed: %d MHz\n", speed);
		printf("3 - Port 255 value: %02XH\n", fp_value);
		printf("4 - list files\n");
		printf("5 - load file\n");
		printf("6 - list disks\n");
		printf("7 - Disk 0: %s\n", disks[0]);
		printf("8 - Disk 1: %s\n", disks[1]);
		printf("9 - run machine\n\n");
		printf("Command: ");
		get_cmdline(s, 2);
		putchar('\n');

		switch (*s) {
		case '1':
			if (cpu == Z80)
				switch_cpu(I8080);
			else
				switch_cpu(Z80);
			break;

		case '2':
			printf("Value in MHz, 0=unlimited: ");
			get_cmdline(s, 2);
			putchar('\n');
			speed = atoi((const char *) &s);
			break;

		case '3':
again:
			printf("Value in Hex: ");
			get_cmdline(s, 3);
			putchar('\n');
			if (!isxdigit(*s) || !isxdigit(*(s + 1))) {
				printf("What?\n");
				goto again;
			}
			fp_value = (*s <= '9' ? *s - '0' : *s - 'A' + 10) << 4;
			fp_value += (*(s + 1) <= '9' ? *(s + 1) - '0' :
				     *(s + 1) - 'A' + 10);
			break;

		case '4':
			my_ls(cpath, cext);
			printf("\n\n");
			break;

		case '5':
			prompt_fn(s);
			load_file(s);
			putchar('\n');
			break;

		case '6':
			my_ls(dpath, dext);
			printf("\n\n");
			break;

		case '7':
			prompt_fn(s);
			if (strlen(s) == 0) {
				disks[0][0] = 0x0;
				putchar('\n');
			} else {
				mount_disk(0, s);
			}
			break;

		case '8':
			prompt_fn(s);
			if (strlen(s) == 0) {
				disks[1][0] = 0x0;
				putchar('\n');
			} else {
				mount_disk(1, s);
			}
			break;

		case '9':
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
		f_write(&sd_file, &disks[0], 22, &br);
		f_write(&sd_file, &disks[1], 22, &br);
		f_close(&sd_file);
	}
}
