/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-DEC-16 dummy, no configuration implemented yet
 * 02-JAN-17 front panel framerate configurable
 * 27-JAN-17 initial window size of the front panel configurable
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"

#define BUFSIZE 256	/* max line length of command buffer */

int fp_size = 800;

void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2;
	char fn[4095];

	strcpy(&fn[0], &confdir[0]);
	strcat(&fn[0], "/system.conf");
	if ((fp = fopen(&fn[0], "r")) != NULL) {
		s = &buf[0];
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			t1 = strtok(s, " \t");
			t2 = strtok(NULL, " \t");
			if (!strcmp(t1, "fp_fps")) {
				fp_fps = (float) atoi(t2);
			} else if (!strcmp(t1, "fp_size")) {
				fp_size = atoi(t2);
			} else {
				printf("system.conf unknow command: %s\n", s);
			}
		}
	}
}
