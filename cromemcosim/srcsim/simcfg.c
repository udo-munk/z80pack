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
 * 20-DEC-2016 dummy, no configuration implemented yet
 * 02-JAN-2017 front panel framerate configurable
 * 27-JAN-2017 initial window size of the front panel configurable
 * 18-JUL-2018 use logging
 * 22-JAN-2021 added option for config file
 * 17-JUN-2021 allow building machine without frontpanel
 * 29-JUL-2021 add boot config for machine without frontpanel
 * 30-AUG-2021 new memory configuration sections
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"
#include "simmem.h"
#include "simcore.h"
#include "simcfg.h"

#define BUFSIZE 256	/* max line length of command buffer */

static const char *TAG = "config";

int  fp_size = 800;		/* default frontpanel size */
BYTE fp_port = 0x10;		/* default fp input port value */
int  ns_port = NS_DEF_PORT;	/* default port to run web server on */

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
			if ((t1 = strtok(s, " \t")) == NULL) {
				LOGW(TAG, "missing command");
				continue;
			}
			if ((t2 = strtok(NULL, " \t,")) == NULL) {
				LOGW(TAG, "missing parameter for %s", t1);
				continue;
			}
			if (!strcmp(t1, "fp_port")) {
				fp_port = (BYTE) exatoi(t2);
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
				LOGD(TAG, "Boot switch address at %04XH", boot_switch[section]);
			} else if (!strcmp(t1, "[MEMORY")) {
				v1 = strtol(t2, &t3, 10);
				if (t3[0] != ']' || v1 < 1 || v1 > MAXMEMSECT) {
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
