/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2025 by Thomas Eberhardt
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-OCT-2008 first version finished
 * 20-MAR-2014 ignore carriage return too, necessary for the Windows port
 * 19-JUN-2014 added config parameter for dropping nulls after CR/LF
 * 09-OCT-2014 modified to support 2 SIO's
 * 09-MAY-2016 added path for config file
 * 29-AUG-2016 ROM and boot switch configuration for Altair emulation added
 * 20-DEC-2016 configuration moved local, will be different for each system
 * 30-DEC-2016 made RAM size configurable, memory map > E000 is fixed
 * 04-JAN-2017 front panel framerate configurable
 * 12-JAN-2017 VIO color configurable
 * 26-JAN-2017 initial window size of the front panel configurable
 * 21-FEB-2017 VIO monitor scanlines configurable
 * 23-FEB-2017 added configuration options for VDM
 * 24-MAR-2017 added configuration for SIO 0
 * 18-JUL-2018 use logging
 * 12-JUL-2019 implemented second SIO
 * 17-SEP-2019 more consistent SIO naming
 * 07-OCT-2019 implemented baud rate for modem device
 * 14-AUG-2020 allow building machine without frontpanel
 * 22-JAN-2021 added option for config file
 * 14-JUL-2021 added all options for SIO 2B
 * 16-JUL-2021 added all options for SIO 1B
 * 20-JUL-2021 log banked memory
 * 05-AUG-2021 add boot config for machine without frontpanel
 * 29-AUG-2021 new memory configuration sections
 * 03-JAN-2025 changed colors configuration to RGB-triple
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simcfg.h"

#include "imsai-sio2.h"
#include "imsai-vio.h"

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "config";

#define BUFSIZE 256		/* max line length of command buffer */

int  fp_size = 800;		/* default frontpanel size */
BYTE fp_port = 0;		/* default fp input port value */
int  ns_port = NS_DEF_PORT;	/* default port to run web server on */

void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2, *t3, *t4;
	int v1, v2, v3;
	char fn[MAX_LFN - 1];

	int num_segs = 0;
	int section = 0;

	if (c_flag) {
		strcpy(fn, conffn);
	} else {
		strcpy(fn, confdir);
		strcat(fn, "/system.conf");
	}

	if ((fp = fopen(fn, "r")) != NULL) {
		s = buf;
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			if ((t1 = strtok(s, " \t")) == NULL) {
				LOGW(TAG, "missing command");
				continue;
			}
			if ((t2 = strtok(NULL, " \t,")) == NULL) {
				LOGW(TAG, "missing parameter for %s", t1);
				continue;
			}
			if (!strcmp(t1, "sio1a_upper_case")) {
				switch (*t2) {
				case '0':
					sio1a_upper_case = 0;
					break;
				case '1':
					sio1a_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1b_upper_case")) {
				switch (*t2) {
				case '0':
					sio1b_upper_case = 0;
					break;
				case '1':
					sio1b_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2a_upper_case")) {
				switch (*t2) {
				case '0':
					sio2a_upper_case = 0;
					break;
				case '1':
					sio2a_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2b_upper_case")) {
				switch (*t2) {
				case '0':
					sio2b_upper_case = 0;
					break;
				case '1':
					sio2b_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1a_strip_parity")) {
				switch (*t2) {
				case '0':
					sio1a_strip_parity = 0;
					break;
				case '1':
					sio1a_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1b_strip_parity")) {
				switch (*t2) {
				case '0':
					sio1b_strip_parity = 0;
					break;
				case '1':
					sio1b_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2a_strip_parity")) {
				switch (*t2) {
				case '0':
					sio2a_strip_parity = 0;
					break;
				case '1':
					sio2a_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2b_strip_parity")) {
				switch (*t2) {
				case '0':
					sio2b_strip_parity = 0;
					break;
				case '1':
					sio2b_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1a_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio1a_drop_nulls = 0;
					break;
				case '1':
					sio1a_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1b_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio1b_drop_nulls = 0;
					break;
				case '1':
					sio1b_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2a_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio2a_drop_nulls = 0;
					break;
				case '1':
					sio2a_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2b_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio2b_drop_nulls = 0;
					break;
				case '1':
					sio2b_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1a_baud_rate")) {
				sio1a_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio1b_baud_rate")) {
				sio1b_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio2a_baud_rate")) {
				sio2a_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio2b_baud_rate")) {
				sio2b_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "fp_port")) {
				fp_port = (BYTE) strtol(t2, NULL, 16);
			} else if (!strcmp(t1, "fp_fps")) {
#ifdef FRONTPANEL
				fp_fps = (float) atoi(t2);
#endif
			} else if (!strcmp(t1, "fp_size")) {
#ifdef FRONTPANEL
				fp_size = atoi(t2);
#endif
			} else if (!strcmp(t1, "ns_port")) {
#ifdef HAS_NETSERVER
				ns_port = atoi(t2);
				if (ns_port < 1024 || ns_port > 65535) {
					LOGW(TAG, "invalid port number %d",
					     ns_port);
					ns_port = NS_DEF_PORT;
				}
#endif
			} else if (!strcmp(t1, "vio_bg")) {
				if ((t3 = strtok(NULL, " \t,")) == NULL ||
				    (t4 = strtok(NULL, " \t,")) == NULL) {
					LOGW(TAG, "missing parameter for %s", t1);
					continue;
				}
				v1 = strtol(t2, NULL, 0);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "invalid red component %d", v1);
					continue;
				}
				v2 = strtol(t3, NULL, 0);
				if (v2 < 0 || v2 > 255) {
					LOGW(TAG, "invalid green component %d", v2);
					continue;
				}
				v3 = strtol(t4, NULL, 0);
				if (v3 < 0 || v3 > 255) {
					LOGW(TAG, "invalid blue component %d", v3);
					continue;
				}
				bg_color[0] = v1;
				bg_color[1] = v2;
				bg_color[2] = v3;
			} else if (!strcmp(t1, "vio_fg")) {
				if ((t3 = strtok(NULL, " \t,")) == NULL ||
				    (t4 = strtok(NULL, " \t,")) == NULL) {
					LOGW(TAG, "missing parameter for %s", t1);
					continue;
				}
				v1 = strtol(t2, NULL, 0);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "invalid red component %d", v1);
					continue;
				}
				v2 = strtol(t3, NULL, 0);
				if (v2 < 0 || v2 > 255) {
					LOGW(TAG, "invalid green component %d", v2);
					continue;
				}
				v3 = strtol(t4, NULL, 0);
				if (v3 < 0 || v3 > 255) {
					LOGW(TAG, "invalid blue component %d", v3);
					continue;
				}
				fg_color[0] = v1;
				fg_color[1] = v2;
				fg_color[2] = v3;
			} else if (!strcmp(t1, "vio_scanlines")) {
				if (*t2 != '0')
					slf = 2;
			} else if (!strcmp(t1, "ram")) {
				if (num_segs >= MAXMEMMAP) {
					LOGW(TAG, "too many rom/ram statements");
					continue;
				}
				if ((t3 = strtok(NULL, " \t,")) == NULL) {
					LOGW(TAG, "missing ram size");
					continue;
				}
				v1 = strtol(t2, NULL, 0);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "invalid ram start address %d", v1);
					continue;
				}
				v2 = strtol(t3, NULL, 0);
				if (v2 < 1 || v1 + v2 > 256) {
					LOGW(TAG, "invalid ram size %d", v2);
					continue;
				}
				memconf[section][num_segs].type = MEM_RW;
				memconf[section][num_segs].spage = v1;
				memconf[section][num_segs].size = v2;
				LOGD(TAG, "RAM %04XH - %04XH",
				     v1 << 8, (v1 << 8) + (v2 << 8) - 1);
				num_segs++;
			} else if (!strcmp(t1, "rom")) {
				if (num_segs >= MAXMEMMAP) {
					LOGW(TAG, "too many rom/ram statements");
					continue;
				}
				if ((t3 = strtok(NULL, " \t,")) == NULL) {
					LOGW(TAG, "missing rom size");
					continue;
				}
				t4 = strtok(NULL, " \t\r\n");
				v1 = strtol(t2, NULL, 0);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "invalid rom start address %d", v1);
					continue;
				}
				v2 = strtol(t3, NULL, 0);
				if (v2 < 1 || v1 + v2 > 256) {
					LOGW(TAG, "invalid rom size %d", v2);
					continue;
				}
				memconf[section][num_segs].type = MEM_RO;
				memconf[section][num_segs].spage = v1;
				memconf[section][num_segs].size = v2;
				if (t4 != NULL) {
					memconf[section][num_segs].rom_file = strdup(t4);
				} else {
					memconf[section][num_segs].rom_file = NULL;
				}
				LOGD(TAG, "ROM %04XH - %04XH %s",
				     v1 << 8, (v1 << 8) + (v2 << 8) - 1,
				     (t4 == NULL ? "" : t4));
				num_segs++;
			} else if (!strcmp(t1, "boot")) {
				boot_switch[section] = strtol(t2, NULL, 0);
				LOGD(TAG, "Power-on-jump address at %04XH", boot_switch[section]);
			} else if (!strcmp(t1, "[MEMORY")) {
				v1 = strtol(t2, &t3, 10);
				if (*t3 != ']' || v1 < 1 || v1 > MAXMEMSECT) {
					LOGW(TAG, "invalid MEMORY section number %d", v1);
					continue;
				}
				LOGD(TAG, "MEMORY CONFIGURATION %d", v1);
				section = v1 - 1;
				num_segs = 0;
			} else {
				LOGW(TAG, "unknown command: %s", t1);
			}
		}
		fclose(fp);
	}

	LOG(TAG, "SIO 1A running at %d baud\r\n", sio1a_baud_rate);
	LOG(TAG, "SIO 1B running at %d baud\r\n", sio1b_baud_rate);
	LOG(TAG, "SIO 2A running at %d baud\r\n", sio2a_baud_rate);
	LOG(TAG, "SIO 2B running at %d baud\r\n", sio2b_baud_rate);

	LOG(TAG, "\r\n");

#ifndef HAS_NETSERVER
	LOG(TAG, "Web server not builtin\r\n");
#else
	if (n_flag) {
		LOG(TAG, "Web server builtin, URL is http://localhost:%d\r\n",
		    ns_port);
	} else {
		LOG(TAG, "Web server builtin, but disabled\r\n");
	}
#endif
}
