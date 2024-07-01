/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 03-JUN-2024 first version
 * 07-JUN-2024 rewrite of the monitor ports and the timing thread
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "simmem.h"
#include "mds-monitor.h"
#include "log.h"

#define BUFSIZE 256	/* max line length of command buffer */

static const char *TAG = "config";

int fp_size = 800;		/* default frontpanel size */

void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2;
	char fn[MAX_LFN - 1];

	if (c_flag) {
		strcpy(&fn[0], &conffn[0]);
	} else {
		strcpy(&fn[0], &confdir[0]);
		strcat(&fn[0], "/system.conf");
	}

	if ((fp = fopen(&fn[0], "r")) != NULL) {
		s = &buf[0];
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			if ((t1 = strtok(s, " \t")) == NULL) {
				LOGW(TAG, "missing command");
				continue;
			}
			if ((t2 = strtok(NULL, " \t,\r\n")) == NULL) {
				LOGW(TAG, "missing parameter for %s", t1);
				continue;
			}
			if (!strcmp(t1, "tty_upper_case")) {
				switch (*t2) {
				case '0':
					tty_upper_case = 0;
					break;
				case '1':
					tty_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "crt_upper_case")) {
				switch (*t2) {
				case '0':
					crt_upper_case = 0;
					break;
				case '1':
					crt_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "tty_strip_parity")) {
				switch (*t2) {
				case '0':
					tty_strip_parity = 0;
					break;
				case '1':
					tty_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "crt_strip_parity")) {
				switch (*t2) {
				case '0':
					crt_strip_parity = 0;
					break;
				case '1':
					crt_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "tty_drop_nulls")) {
				switch (*t2) {
				case '0':
					tty_drop_nulls = 0;
					break;
				case '1':
					tty_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "crt_drop_nulls")) {
				switch (*t2) {
				case '0':
					crt_drop_nulls = 0;
					break;
				case '1':
					crt_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "tty_baud_rate")) {
				tty_clock_div = 38400 / atoi(t2);
				if (tty_clock_div == 0)
					tty_clock_div = 1;
			} else if (!strcmp(t1, "crt_baud_rate")) {
				crt_clock_div = 38400 / atoi(t2);
				if (crt_clock_div == 0)
					crt_clock_div = 1;
			} else if (!strcmp(t1, "pt_baud_rate")) {
				pt_clock_div = 38400 / atoi(t2);
				if (pt_clock_div == 0)
					pt_clock_div = 1;
			} else if (!strcmp(t1, "lpt_baud_rate")) {
				lpt_clock_div = 38400 / atoi(t2);
				if (lpt_clock_div == 0)
					lpt_clock_div = 1;
			} else if (!strcmp(t1, "fp_fps")) {
#ifdef FRONTPANEL
				fp_fps = (float) atoi(t2);
#endif
			} else if (!strcmp(t1, "fp_size")) {
#ifdef FRONTPANEL
				fp_size = atoi(t2);
#endif
			} else if (!strcmp(t1, "boot_rom")) {
				boot_rom_file = strdup(t2);
			} else if (!strcmp(t1, "mon_rom")) {
				mon_rom_file = strdup(t2);
			} else if (!strcmp(t1, "mon_enabled")) {
				mon_enabled = atoi(t2) != 0;
			} else {
				LOGW(TAG, "unknown command: %s", t1);
			}
		}
		fclose(fp);
	}
}
