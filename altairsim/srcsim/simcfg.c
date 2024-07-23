/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
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
 * 04-JAN-2017 front panel framerate configurable
 * 26-JAN-2017 initial window size of the front panel configurable
 * 23-FEB-2017 added configuration options for VDM
 * 24-MAR-2017 added configuration for SIO 0
 * 14-JUN-2017 added config for Tarbell boot ROM
 * 07-MAY-2018 added memory configuratione needed by apple monitor
 * 03-JUL-2018 added baud rate to terminal 2SIO
 * 04-JUL-2018 added baud rate to terminal SIO
 * 17-JUL-2018 use logging
 * 21-AUG-2018 improved memory configuration
 * 24-NOV-2019 configurable baud rate for second 2SIO channel
 * 22-JAN-2021 added option for config file
 * 31-JUL-2021 allow building machine without frontpanel
 * 29-AUG-2021 new memory configuration sections
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simutil.h"
#include "simcfg.h"

#include "altair-88-sio.h"
#include "altair-88-2sio.h"
#include "proctec-vdm.h"

#include "log.h"
static const char *TAG = "config";

#define BUFSIZE 256	/* max line length of command buffer */

int  fp_size = 800;	/* default frontpanel size */
BYTE fp_port = 0;	/* default fp input port value */

void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2, *t3, *t4;
	int v1, v2;
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
			if (!strcmp(t1, "sio0_upper_case")) {
				switch (*t2) {
				case '0':
					sio0_upper_case = 0;
					break;
				case '1':
					sio0_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_upper_case")) {
				switch (*t2) {
				case '0':
					sio1_upper_case = 0;
					break;
				case '1':
					sio1_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2_upper_case")) {
				switch (*t2) {
				case '0':
					sio2_upper_case = 0;
					break;
				case '1':
					sio2_upper_case = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio0_strip_parity")) {
				switch (*t2) {
				case '0':
					sio0_strip_parity = 0;
					break;
				case '1':
					sio0_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_strip_parity")) {
				switch (*t2) {
				case '0':
					sio1_strip_parity = 0;
					break;
				case '1':
					sio1_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2_strip_parity")) {
				switch (*t2) {
				case '0':
					sio2_strip_parity = 0;
					break;
				case '1':
					sio2_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio0_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio0_drop_nulls = 0;
					break;
				case '1':
					sio0_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio1_drop_nulls = 0;
					break;
				case '1':
					sio1_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio2_drop_nulls = 0;
					break;
				case '1':
					sio2_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio0_revision")) {
				switch (*t2) {
				case '0':
					sio0_revision = 0;
					break;
				case '1':
					sio0_revision = 1;
					break;
				default:
					LOGW(TAG, "invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio0_baud_rate")) {
				sio0_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio1_baud_rate")) {
				sio1_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio2_baud_rate")) {
				sio2_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio3_baud_rate")) {
				sio3_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "fp_port")) {
				fp_port = (BYTE) exatoi(t2);
			} else if (!strcmp(t1, "fp_fps")) {
#ifdef FRONTPANEL
				fp_fps = (float) atoi(t2);
#endif
			} else if (!strcmp(t1, "fp_size")) {
#ifdef FRONTPANEL
				fp_size = atoi(t2);
#endif
			} else if (!strcmp(t1, "vdm_bg")) {
				strncpy(&bg_color[1], t2, 6);
			} else if (!strcmp(t1, "vdm_fg")) {
				strncpy(&fg_color[1], t2, 6);
			} else if (!strcmp(t1, "vdm_scanlines")) {
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
				_boot_switch[section] = strtol(t2, NULL, 0);
				LOGD(TAG, "Boot switch address at %04XH", _boot_switch[section]);
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

	LOG(TAG, "SIO 0 running at %d baud\r\n", sio0_baud_rate);
	LOG(TAG, "SIO 1 running at %d baud\r\n", sio1_baud_rate);
	LOG(TAG, "SIO 2 running at %d baud\r\n", sio2_baud_rate);
	LOG(TAG, "SIO 3 running at %d baud\r\n", sio3_baud_rate);
	LOG(TAG, "\r\n");
}
