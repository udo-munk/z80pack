/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-APR-2024 dummy, no configuration implemented yet
 * 12-MAY-2024 working on configuration dialog
 */

#include <stdio.h>
#include "sim.h"
#include "simglb.h"

extern int get_cmdline(char *, int);
extern unsigned char fp_value;

void config(void)
{
	char buf[10];
	int go_flag = 0;

	while (!go_flag) {
		printf("1 - switch CPU, currently %s\n", (cpu == Z80) ?
							  "Z80" : "8080");
		printf("2 - set port 255 value, currently %02X\n", fp_value);
		printf("3 - run machine\n\n");
		printf("Command: ");
		get_cmdline(buf, 2);
		printf("\n\n");
		if (*buf == '3')
			go_flag = 1;
	}
}
