/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This module configures the machine appropriate for the
 * Z80/8080 software we want to run on it.
 *
 * History:
 * 20-APR-2024 dummy, no configuration implemented yet
 * 12-MAY-2024 implemented configuration dialog
 * 27-MAY-2024 implemented load file
 */

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "sim.h"
#include "simglb.h"

extern int get_cmdline(char *, int);
extern void switch_cpu(int);
extern void load_file(char *);
extern unsigned char fp_value;

/*
 * prompt for a filename
 */
static void prompt_fn(char *s)
{
      printf("Filename: ");
      get_cmdline(s, 9);
      printf("\n");
}

/*
 * Configuration dialog for the machine
 */
void config(void)
{
	char s[10];
	int go_flag = 0;

	while (!go_flag) {
		printf("1 - switch CPU, currently %s\n", (cpu == Z80) ?
							  "Z80" : "8080");
		printf("2 - set port 255 value, currently %02XH\n", fp_value);
		printf("3 - load file\n");
		printf("4 - run machine\n\n");
		printf("Command: ");
		get_cmdline(s, 2);
		printf("\n\n");

		switch (*s) {
		case '1':
			if (cpu == Z80)
				switch_cpu(I8080);
			else
				switch_cpu(Z80);
			break;
		case '2':
again:
			printf("Value in Hex: ");
			get_cmdline(s, 3);
			printf("\n\n");
			if (!isxdigit(*s) || !isxdigit(*(s + 1))) {
				printf("What?\n");
				goto again;
			}
			fp_value = (*s <= '9' ? *s - '0' : *s - 'A' + 10) << 4;
			fp_value += (*(s + 1) <= '9' ? *(s + 1) - '0' :
				     *(s + 1) - 'A' + 10);
			break;

		case '3':
			prompt_fn(s);
			load_file(s);
			break;

		case '4':
			go_flag = 1;
			break;

		default:
			break;
		}
	}
}
