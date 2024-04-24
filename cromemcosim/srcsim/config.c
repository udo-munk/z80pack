/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2021 Udo Munk
 * Copyright (C) 2021 David McNaughton
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-DEC-16 dummy, no configuration implemented yet
 * 02-JAN-17 front panel framerate configurable
 * 27-JAN-17 initial window size of the front panel configurable
 * 18-JUL-18 use logging
 * 22-JAN-21 added option for config file
 * 17-JUN-21 allow building machine without frontpanel
 * 29-JUL-21 add boot config for machine without frontpanel
 * 30-AUG-21 new memory configuration sections
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"

#define BUFSIZE 256	/* max line length of command buffer */

static const char *TAG = "config";

int  fp_size = 800;	/* default frontpanel size */
BYTE fp_port = 0x10;	/* default fp input port value */

extern int exatoi(char *);

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
			t1 = strtok(s, " \t");
			t2 = strtok(NULL, " \t,");
			if (!strcmp(t1, "fp_port")) {
				fp_port = (BYTE) exatoi(t2);
			} else if (!strcmp(t1, "fp_fps")) {
#ifdef FRONTPANEL
				fp_fps = (float) atoi(t2);
#else
				;
#endif
			} else if (!strcmp(t1, "fp_size")) {
#ifdef FRONTPANEL
				fp_size = atoi(t2);
#else
				;
#endif
			} else if (!strcmp(t1, "ram")) {
				if (num_segs >= MAXMEMMAP) {
					LOGW(TAG, "too many rom/ram statements");
					goto next;
				}
				t3 = strtok(NULL, " \t,");
				v1 = strtol(t2, NULL, 0);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "invalid ram start address %d", v1);
					goto next;
				}
				v2 = strtol(t3, NULL, 0);
				if (v2 < 1 || v1 + v2 > 256) {
					LOGW(TAG, "invalid ram size %d", v2);
					goto next;
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
					goto next;
				}
				t3 = strtok(NULL, " \t,");
				t4 = strtok(NULL, " \t\r\n");
				v1 = strtol(t2, NULL, 0);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "invalid rom start address %d", v1);
					goto next;
				}
				v2 = strtol(t3, NULL, 0);
				if (v2 < 1 || v1 + v2 > 256) {
					LOGW(TAG, "invalid rom size %d", v2);
					goto next;
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
				LOGD(TAG, "Boot switch address at %04XH", boot_switch[section]);
			} else if (!strcmp(t1, "[MEMORY")) {
				v1 = strtol(t2, &t3, 10);
				if (t3[0] != ']' || v1 < 1 || v1 > MAXMEMSECT) {
					LOGW(TAG, "invalid MEMORY section number %d", v1);
					goto next;
				}
				LOGD(TAG, "MEMORY CONFIGURATION %d", v1);
				section = v1 - 1;
				num_segs = 0;
			} else {
				LOGW(TAG, "system.conf unknown command: %s", s);
			}

next:
			;

		}
		fclose(fp);
	}

	LOG(TAG, "\r\n");

#ifndef HAS_NETSERVER
	LOG(TAG, "Web server not builtin\r\n");
#else
	LOG(TAG, "Web server builtin, URL is http://localhost:8080\r\n");
#endif
}
