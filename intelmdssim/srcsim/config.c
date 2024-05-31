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
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"
#include "memsim.h"

#define BUFSIZE 256	/* max line length of command buffer */

extern char *boot_rom_file, *mon_rom_file;
extern int exatoi(char *);

static const char *TAG = "config";

int  fp_size = 800;		/* default frontpanel size */
BYTE fp_port = 0;		/* default fp input port value */

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
