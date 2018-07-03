/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2018 by Udo Munk
 *
 * This module reads the system configuration file and sets
 * global variables, so that the system can be configured.
 *
 * History:
 * 20-OCT-08 first version finished
 * 20-MAR-14 ignore carriage return too, necessary for the Windows port
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 09-OCT-14 modified to support 2 SIO's
 * 09-MAY-16 added path for config file
 * 29-AUG-16 ROM and boot switch configuration for Altair emulation added
 * 20-DEC-16 configuration moved local, will be different for each system
 * 04-JAN-17 front panel framerate configurable
 * 26-JAN-17 initial window size of the front panel configurable
 * 23-FEB-17 added configuration options for VDM
 * 24-MAR-17 added configuration for SIO 0
 * 14-JUN-17 added config for Tarbell boot ROM
 * 07-MAY-18 added memory configuratione needed by apple monitor
 * 03-JUL-18 added baud rate to terminal 2SIO
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"

#define BUFSIZE 256	/* max line length of command buffer */

int ram_size;
int rom_size;
int rom_start;
int boot_switch;
int fp_size = 800;

extern int exatoi(char *);

extern int tarbell_rom_enabled;	/* Tarbell bootstrap ROM enable/disable */

extern int sio0_upper_case;	/* SIO 0 translate input to upper case */
extern int sio0_strip_parity;	/* SIO 0 strip parity from output */
extern int sio0_drop_nulls;	/* SIO 0 drop nulls after CR/LF */
extern int sio0_revision;	/* SIO 0 Rev 0 or Rev 1 */

extern int sio1_upper_case;	/* SIO 1 translate input to upper case */
extern int sio1_strip_parity;	/* SIO 1 strip parity from output */
extern int sio1_drop_nulls;	/* SIO 1 drop nulls after CR/LF */
extern int sio1_baud_rate;	/* SIO 1 baud rate */

extern int sio2_upper_case;	/* SIO 2 translate input to upper case */
extern int sio2_strip_parity;	/* SIO 2 strip parity from output */
extern int sio2_drop_nulls;	/* SIO 2 drop nulls after CR/LF */

extern char bg_color[];         /* VDM background color */
extern char fg_color[];         /* VDM foreground color */
extern int slf;                 /* VDM scanlines factor */

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
			if (!strcmp(t1, "sio0_upper_case")) {
				switch (*t2) {
				case '0':
					sio0_upper_case = 0;
					break;
				case '1':
					sio0_upper_case = 1;
					break;
				default:
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
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
					printf("system.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_baud_rate")) {
				sio1_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "fp_fps")) {
				fp_fps = (float) atoi(t2);
			} else if (!strcmp(t1, "fp_size")) {
				fp_size = atoi(t2);
			} else if (!strcmp(t1, "vdm_bg")) {
				strncpy(&bg_color[1], t2, 6);
			} else if (!strcmp(t1, "vdm_fg")) {
				strncpy(&fg_color[1], t2, 6);
			} else if (!strcmp(t1, "vdm_scanlines")) {
				if (*t2 != '0')
					slf = 2;
#ifndef MONITORMEM
			} else if (!strcmp(t1, "ram")) {
				ram_size = atoi(t2);
				printf("RAM size %4d pages, 0000H - %04xH\n",
				       ram_size, (ram_size << 8) - 1);
			} else if (!strcmp(t1, "rom")) {
				rom_size = atoi(t2);
				rom_start = (256 - rom_size) << 8;
				printf("ROM size %4d pages, %04xH - ffffH\n",
				       rom_size, rom_start);
#else
			} else if (!strcmp(t1, "ram")) {
				;
			} else if (!strcmp(t1, "rom")) {
				;
#endif
			} else if (!strcmp(t1, "boot")) {
				boot_switch = exatoi(t2);
				printf("Boot switch address at %04xH\n", boot_switch);
			} else if (!strcmp(t1, "tarbell_rom_enabled")) {
				tarbell_rom_enabled = atoi(t2);
				printf("Tarbell bootstrap ROM %s\n",
				       (tarbell_rom_enabled) ?
				       "enabled" : "disabled");
			} else {
				printf("system.conf unknown command: %s\n", s);
			}
		}
	}

#ifdef MONITORMEM
	puts("RAM 0000H - efffH");
	puts("ROM f000H - f7ffH");
	puts("RAM f800H - ffffH");
#endif

	printf("\n");

}
