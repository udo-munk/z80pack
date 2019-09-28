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
 * 27-SEP-19 include newline \n as config.txt delimiter
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"

#define BUFSIZE 256	/* max line length of command buffer */

void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2;

	printf("\n");
	if ((fp = fopen("conf/config.txt", "r")) != NULL) {
		s = buf;
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			t1 = strtok(s, "= \t\r\n");
			t2 = strtok(NULL, "= \t\r\n");

			if (0 == strcmp(t1,"bootrom")) {
				printf("\nBoot ROM: %s\n",t2);
				strcpy(xfn,t2);
				x_flag = 1;
			}
			else if (0 == strcmp(t1, "drive0"))
				printf("Drive 0: %s\n",t2);

			else if (0 == strcmp(t1, "drive1")) 
				printf("Drive 1: %s\n",t2);

			else if (0 == strcmp(t1, "drive2"))
				printf("Drive 2: %s\n",t2);

			else if (0 == strcmp(t1, "drive3"))
				printf("Drive 3: %s\n",t2);

			else 
				printf("config.txt: unknown command: %s\n",s);
		}
		fclose(fp);
	}
	else
		printf("Missing conf/config.txt file\n");
	printf("\n");
}
