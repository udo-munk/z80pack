/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 24-SEP-19 (Mike Douglas) created for Mostek AID-80F and SYS-80FT
 * 27-SEP-19 (Mike Douglas) include newline \n as config.txt delimiter
 * 28-SEP-19 (Udo Munk) use logging
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

	LOG(TAG, "");

	if ((fp = fopen("conf/config.txt", "r")) != NULL) {
		s = buf;
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			t1 = strtok(s, "= \t\r\n");
			t2 = strtok(NULL, "= \t\r\n");

			if (0 == strcmp(t1, "bootrom")) {
				LOG(TAG, "\nBoot ROM: %s\n\n", t2);
				strcpy(xfn, t2);
				x_flag = 1;
			}
			else if (0 == strcmp(t1, "drive0")) {
				LOG(TAG, "\nDrive 0: %s\n", t2);
			}
			else if (0 == strcmp(t1, "drive1")) {
				LOG(TAG, "Drive 1: %s\n", t2);
			}
			else if (0 == strcmp(t1, "drive2")) {
				LOG(TAG, "Drive 2: %s\n", t2);
			}
			else if (0 == strcmp(t1, "drive3")) {
				LOG(TAG, "Drive 3: %s\n", t2);
			}
			else {
				LOGW(TAG, "unknown command: %s",s);
			}
		}
		fclose(fp);
	}
	else {
		LOGW(TAG, "missing conf/config.txt file");
	}

	LOG(TAG, "");
}
