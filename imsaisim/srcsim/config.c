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
 * 30-DEC-16 made RAM size configurable, memory map > E000 is fixed
 * 04-JAN-17 front panel framerate configurable
 * 12-JAN-17 VIO color configurable
 * 26-JAN-17 initial window size of the front panel configurable
 * 21-FEB-17 VIO monitor scanlines configurable
 * 23-FEB-17 added configuration options for VDM
 * 24-MAR-17 added configuration for SIO 0
 * 18-JUL-18 use logging
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"

#define BUFSIZE 256	/* max line length of command buffer */

static const char *TAG = "config";

int ram_size;
int fp_size = 800;

extern int exatoi(char *);

extern int sio1_upper_case;	/* SIO 1 translate input to upper case */
extern int sio1_strip_parity;	/* SIO 1 strip parity from output */
extern int sio1_drop_nulls;	/* SIO 1 drop nulls after CR/LF */
extern int sio1_baud_rate;	/* SIO 1 simulated baud rate */

extern int sio2_upper_case;	/* SIO 2 translate input to upper case */
extern int sio2_strip_parity;	/* SIO 2 strip parity from output */
extern int sio2_drop_nulls;	/* SIO 2 drop nulls after CR/LF */

extern char bg_color[];		/* VIO background color */
extern char fg_color[];		/* VIO foreground color */
extern int slf;			/* VIO scanlines factor */


void config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2;
	char fn[MAX_LFN - 1];

	strcpy(&fn[0], &confdir[0]);
	strcat(&fn[0], "/system.conf");
	if ((fp = fopen(&fn[0], "r")) != NULL) {
		s = &buf[0];
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			t1 = strtok(s, " \t");
			t2 = strtok(NULL, " \t");
			if (!strcmp(t1, "sio1_upper_case")) {
				switch (*t2) {
				case '0':
					sio1_upper_case = 0;
					break;
				case '1':
					sio1_upper_case = 1;
					break;
				default:
					LOGW(TAG, "system.conf: illegal value for %s: %s\n", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s\n", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s\n", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s\n", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s\n", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_baud_rate")) {
				sio1_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "fp_fps")) {
				fp_fps = (float) atoi(t2);
			} else if (!strcmp(t1, "fp_size")) {
				fp_size = atoi(t2);
			} else if (!strcmp(t1, "vio_bg")) {
				strncpy(&bg_color[1], t2, 6);
			} else if (!strcmp(t1, "vio_fg")) {
				strncpy(&fg_color[1], t2, 6);
			} else if (!strcmp(t1, "vio_scanlines")) {
				if (*t2 != '0')
					slf = 2;
			} else if (!strcmp(t1, "ram")) {
				ram_size = atoi(t2);
				if (ram_size > MAX_RAM) {
					LOGW(TAG, "Maximal possible RAM size is %dKB\n", MAX_RAM);
					ram_size = MAX_RAM;
				}
				LOG(TAG, "RAM size is %d KB\n", ram_size);
			} else {
				LOGW(TAG, "system.conf unknown command: %s\n", s);
			}
		}
	}
}
