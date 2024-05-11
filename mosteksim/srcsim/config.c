/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 24-SEP-2019 (Mike Douglas) created for Mostek AID-80F and SYS-80FT
 * 27-SEP-2019 (Mike Douglas) include newline \n as config.txt delimiter
 * 28-SEP-2019 (Udo Munk) use logging
 * 17-OCT-2019 (Udo Munk) fix logging
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"

#define BUFSIZE 256	/* max line length of command buffer */

static const char *TAG = "config";

void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2;
	char fn[MAX_LFN - 1];

	if (c_flag) {
		strcpy(fn, conffn);
	} else {
		strcpy(fn, confdir);
		strcat(fn, "/config.txt");
	}

	if ((fp = fopen(fn, "r")) != NULL) {
		s = buf;
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			if ((t1 = strtok(s, "= \t\r\n")) == NULL) {
				LOGW(TAG, "missing command");
				continue;
			}
			if ((t2 = strtok(NULL, "= \t\r\n")) == NULL) {
				LOGW(TAG, "missing parameter for %s", t1);
				continue;
			}

			if (0 == strcmp(t1, "bootrom")) {
				LOG(TAG, "\r\nBoot ROM: %s\r\n\r\n", t2);
				strcpy(xfn, rompath);
				strcat(xfn, "/");
				strcat(xfn, t2);
				x_flag = 1;
			} else if (0 == strcmp(t1, "drive0")) {
				LOG(TAG, "\r\nDrive 0: %s\r\n", t2);
			} else if (0 == strcmp(t1, "drive1")) {
				LOG(TAG, "Drive 1: %s\r\n", t2);
			} else if (0 == strcmp(t1, "drive2")) {
				LOG(TAG, "Drive 2: %s\r\n", t2);
			} else if (0 == strcmp(t1, "drive3")) {
				LOG(TAG, "Drive 3: %s\r\n", t2);
			} else {
				LOGW(TAG, "unknown command: %s", t1);
			}
		}
		fclose(fp);
	} else {
		LOGW(TAG, "missing conf/config.txt file");
	}
}
