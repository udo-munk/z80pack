/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2021 by Udo Munk
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
 * 12-JUL-19 implemented second SIO
 * 17-SEP-19 more consistent SIO naming
 * 07-OCT-19 implemented baud rate for modem device
 * 14-AUG-20 allow building machine without frontpanel
 * 22-JAN-21 added option for config file
 * 14-JUL-21 added all options for SIO 2B
 * 16-JUL-21 added all options for SIO 1B
 * 20-JUL-21 log banked memory
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

extern int num_banks;

extern int exatoi(char *);

extern int sio1a_upper_case;	/* SIO 1 A translate input to upper case */
extern int sio1a_strip_parity;	/* SIO 1 A strip parity from output */
extern int sio1a_drop_nulls;	/* SIO 1 A drop nulls after CR/LF */
extern int sio1a_baud_rate;	/* SIO 1 A simulated baud rate */

extern int sio1b_upper_case;	/* SIO 1 B translate input to upper case */
extern int sio1b_strip_parity;	/* SIO 1 B strip parity from output */
extern int sio1b_drop_nulls;	/* SIO 1 B drop nulls after CR/LF */
extern int sio1b_baud_rate;	/* SIO 1 B simulated baud rate */

extern int sio2a_upper_case;	/* SIO 2 A translate input to upper case */
extern int sio2a_strip_parity;	/* SIO 2 A strip parity from output */
extern int sio2a_drop_nulls;	/* SIO 2 A drop nulls after CR/LF */
extern int sio2a_baud_rate;	/* SIO 2 A simulated baud rate */

extern int sio2b_upper_case;	/* SIO 2 B translate input to upper case */
extern int sio2b_strip_parity;	/* SIO 2 B strip parity from output */
extern int sio2b_drop_nulls;	/* SIO 2 B drop nulls after CR/LF */
extern int sio2b_baud_rate;	/* SIO 2 B simulated baud rate */

extern char bg_color[];		/* VIO background color */
extern char fg_color[];		/* VIO foreground color */
extern int slf;			/* VIO scanlines factor */


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
			t1 = strtok(s, " \t");
			t2 = strtok(NULL, " \t");
			if (!strcmp(t1, "sio1a_upper_case")) {
				switch (*t2) {
				case '0':
					sio1a_upper_case = 0;
					break;
				case '1':
					sio1a_upper_case = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1b_upper_case")) {
				switch (*t2) {
				case '0':
					sio1b_upper_case = 0;
					break;
				case '1':
					sio1b_upper_case = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2a_upper_case")) {
				switch (*t2) {
				case '0':
					sio2a_upper_case = 0;
					break;
				case '1':
					sio2a_upper_case = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2b_upper_case")) {
				switch (*t2) {
				case '0':
					sio2b_upper_case = 0;
					break;
				case '1':
					sio2b_upper_case = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1a_strip_parity")) {
				switch (*t2) {
				case '0':
					sio1a_strip_parity = 0;
					break;
				case '1':
					sio1a_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1b_strip_parity")) {
				switch (*t2) {
				case '0':
					sio1b_strip_parity = 0;
					break;
				case '1':
					sio1b_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2a_strip_parity")) {
				switch (*t2) {
				case '0':
					sio2a_strip_parity = 0;
					break;
				case '1':
					sio2a_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2b_strip_parity")) {
				switch (*t2) {
				case '0':
					sio2b_strip_parity = 0;
					break;
				case '1':
					sio2b_strip_parity = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1a_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio1a_drop_nulls = 0;
					break;
				case '1':
					sio1a_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1b_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio1b_drop_nulls = 0;
					break;
				case '1':
					sio1b_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2a_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio2a_drop_nulls = 0;
					break;
				case '1':
					sio2a_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2b_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio2b_drop_nulls = 0;
					break;
				case '1':
					sio2b_drop_nulls = 1;
					break;
				default:
					LOGW(TAG, "system.conf: invalid value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1a_baud_rate")) {
				sio1a_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio1b_baud_rate")) {
				sio1b_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio2a_baud_rate")) {
				sio2a_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio2b_baud_rate")) {
				sio2b_baud_rate = atoi(t2);
#ifdef FRONTPANEL
			} else if (!strcmp(t1, "fp_fps")) {
				fp_fps = (float) atoi(t2);
			} else if (!strcmp(t1, "fp_size")) {
				fp_size = atoi(t2);
#endif
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
					LOGW(TAG, "Maximal possible RAM size for bank 0 is %d KB", MAX_RAM);
					ram_size = MAX_RAM;
				}
				LOG(TAG, "RAM size bank 0 is %d KB, MMU has %d additional banks a 48 KB\r\n", ram_size, num_banks);
			} else {
				LOGW(TAG, "system.conf unknown command: %s", s);
			}
		}
	}

	LOG(TAG, "SIO 1A running at %d baud\r\n", sio1a_baud_rate);
	LOG(TAG, "SIO 1B running at %d baud\r\n", sio1b_baud_rate);
	LOG(TAG, "SIO 2A running at %d baud\r\n", sio2a_baud_rate);
	LOG(TAG, "SIO 2B running at %d baud\r\n", sio2b_baud_rate);
}
