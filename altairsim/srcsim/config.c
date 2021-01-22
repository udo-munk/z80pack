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
 * 04-JAN-17 front panel framerate configurable
 * 26-JAN-17 initial window size of the front panel configurable
 * 23-FEB-17 added configuration options for VDM
 * 24-MAR-17 added configuration for SIO 0
 * 14-JUN-17 added config for Tarbell boot ROM
 * 07-MAY-18 added memory configuratione needed by apple monitor
 * 03-JUL-18 added baud rate to terminal 2SIO
 * 04-JUL-18 added baud rate to terminal SIO
 * 17-JUL-18 use logging
 * 21-AUG-18 improved memory configuration
 * 24-NOV-19 configurable baud rate for second 2SIO channel
 * 22-JAN-21 added option for config file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "log.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"

#define BUFSIZE 256	/* max line length of command buffer */

extern int exatoi(char *);

static const char *TAG = "config";

struct memmap memconf[MAXSEG];	/* memory map */
static int num_segs;

int boot_switch;		/* boot address for switch */
int fp_size = 800;		/* frontpanel size */

extern int tarbell_rom_enabled;	/* Tarbell bootstrap ROM enable/disable */

extern int sio0_upper_case;	/* SIO 0 translate input to upper case */
extern int sio0_strip_parity;	/* SIO 0 strip parity from output */
extern int sio0_drop_nulls;	/* SIO 0 drop nulls after CR/LF */
extern int sio0_revision;	/* SIO 0 Rev 0 or Rev 1 */
extern int sio0_baud_rate;	/* SIO 0 baud rate */

extern int sio1_upper_case;	/* SIO 1 translate input to upper case */
extern int sio1_strip_parity;	/* SIO 1 strip parity from output */
extern int sio1_drop_nulls;	/* SIO 1 drop nulls after CR/LF */
extern int sio1_baud_rate;	/* SIO 1 baud rate */

extern int sio2_upper_case;	/* SIO 2 translate input to upper case */
extern int sio2_strip_parity;	/* SIO 2 strip parity from output */
extern int sio2_drop_nulls;	/* SIO 2 drop nulls after CR/LF */
extern int sio2_baud_rate;	/* SIO 2 baud rate */

extern int sio3_baud_rate;	/* SIO 3 baud rate */

extern char bg_color[];         /* VDM background color */
extern char fg_color[];         /* VDM foreground color */
extern int slf;                 /* VDM scanlines factor */

void config(void)
{
	int i, v1, v2;
	FILE *fp;
	char *s, *t1, *t2, *t3;
	char buf[BUFSIZE];
	char fn[4095];

	for (i = 0; i < MAXSEG; i++)
		memconf[i].type = -1;

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
			t2 = strtok(NULL, " \t,");
			if (!strcmp(t1, "sio0_upper_case")) {
				switch (*t2) {
				case '0':
					sio0_upper_case = 0;
					break;
				case '1':
					sio0_upper_case = 1;
					break;
				default:
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
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
					LOGW(TAG, "system.conf: illegal value for %s: %s", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio0_baud_rate")) {
				sio0_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio1_baud_rate")) {
				sio1_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio2_baud_rate")) {
				sio2_baud_rate = atoi(t2);
			} else if (!strcmp(t1, "sio3_baud_rate")) {
				sio3_baud_rate = atoi(t2);
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
			} else if (!strcmp(t1, "ram")) {
				if (num_segs >= MAXSEG) {
					LOGW(TAG, "too many rom/ram statements");
					goto next;
				}
				t3 = strtok(NULL, " \t,");
				v1 = atoi(t2);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "illegal ram start address %d", v1);
					goto next;
				}
				v2 = atoi(t3);
				if (v2 < 1 || v1 + v2 > 256) {
					LOGW(TAG, "illegal ram size %d", v2);
					goto next;
				}
				memconf[num_segs].type = MEM_RW;
				memconf[num_segs].spage = v1;
				memconf[num_segs].size = v2;
				LOG(TAG, "RAM %04XH - %04XH\r\n",
				    v1 << 8, (v1 << 8) + (v2 << 8) - 1);
				num_segs++;
			} else if (!strcmp(t1, "rom")) {
				if (num_segs >= MAXSEG) {
					LOGW(TAG, "too many rom/ram statements");
					goto next;
				}
				t3 = strtok(NULL, " \t,");
				v1 = atoi(t2);
				if (v1 < 0 || v1 > 255) {
					LOGW(TAG, "illegal rom start address %d", v1);
					goto next;
				}
				v2 = atoi(t3);
				if (v2 < 1 || v1 + v2 > 256) {
					LOGW(TAG, "illegal rom size %d", v2);
					goto next;
				}
				memconf[num_segs].type = MEM_RO;
				memconf[num_segs].spage = v1;
				memconf[num_segs].size = v2;
				LOG(TAG, "ROM %04XH - %04XH\r\n",
				    v1 << 8, (v1 << 8) + (v2 << 8) - 1);
				num_segs++;
			} else if (!strcmp(t1, "boot")) {
				boot_switch = exatoi(t2);
				LOG(TAG, "Boot switch address at %04XH\r\n", boot_switch);
			} else if (!strcmp(t1, "tarbell_rom_enabled")) {
				tarbell_rom_enabled = atoi(t2);
				LOG(TAG, "Tarbell bootstrap ROM %s\r\n",
				    (tarbell_rom_enabled) ?
				    "enabled" : "disabled");
			} else {
				LOGW(TAG, "system.conf unknown command: %s", s);
			}

			next:
			;

		}
	}

	LOG(TAG, "SIO 0 running at %d baud\r\n", sio0_baud_rate);
	LOG(TAG, "SIO 1 running at %d baud\r\n", sio1_baud_rate);
	LOG(TAG, "SIO 2 running at %d baud\r\n", sio2_baud_rate);
	LOG(TAG, "SIO 3 running at %d baud\r\n", sio3_baud_rate);
	LOG(TAG, "\r\n");
}
