/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2018 by Udo Munk
 *
 * This module of the simulator contains the I/O simulation
 * for an IMSAI 8080 system
 *
 * History:
 * 20-OCT-08 first version finished
 * 19-JAN-14 unused I/O ports need to return 00 and not FF
 * 02-MAR-14 source cleanup and improvements
 * 23-MAR-14 added 10ms timer interrupt for Kildalls timekeeper PL/M program
 * 16-JUL-14 unused I/O ports need to return FF, see survey.mac
 * 14-OCT-14 support for SIO 2 added, parallel ports problem with ROM avoided
 * 31-JAN-15 took over some improvements made for the Z-1 emulation
 * 09-MAY-15 added Cromemco DAZZLER to the machine
 * 06-DEC-16 implemented status display and stepping for all machine cycles
 * 11-JAN-17 implemented X11 keyboard input for VIO
 * 24-APR-18 cleanup
 * 17-MAY-18 improved hardware control
 * 08-JUN-18 moved hardware initialisation and reset to iosim
 * 12-JUL-18 use logging
 * 14-JUL-18 integrate webfrontend
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "../../iodevices/imsai-sio2.h"
#include "../../iodevices/imsai-fif.h"
#ifdef HAS_DAZZLER
#include "../../iodevices/cromemco-dazzler.h"
#endif //HAS_DAZZLER
#include "../../iodevices/imsai-vio.h"
#include "../../frontpanel/frontpanel.h"
#include "memory.h"
#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif
#include "log.h"

/*
 *	Forward declarations for I/O functions
 */
static BYTE io_trap_in(void), io_no_card_in(void);
static void io_trap_out(BYTE), io_no_card_out(BYTE);
static BYTE fp_in(void);
static void fp_out(BYTE);
static BYTE hwctl_in(void);
static void hwctl_out(BYTE);
static BYTE lpt_in(void);
static void lpt_out(BYTE);
static BYTE io_pport_in(void);
static BYTE imsai_kbd_data_in(void), imsai_kbd_status_in(void);

static const char *TAG = "IO";

static int printer;		/* fd for file "printer.txt" */
BYTE hwctl_lock = 0xff;		/* lock status hardware control port */

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
BYTE (*port_in[256]) (void) = {
	io_trap_in,		/* port 0 */
	io_trap_in,		/* port 1 */
	imsai_sio1_data_in,	/* port 2 */ /* SIO 1 connected to console */
	imsai_sio1_status_in,	/* port 3 */
	imsai_kbd_data_in,	/* port 4 */ /* SIO 2 not connected */
	imsai_kbd_status_in,	/* port 5 */ /* needed for VIO keyboard input */
	io_trap_in,		/* port 6 */
	io_trap_in,		/* port 7 */
	io_no_card_in,		/* port 8 */ /* SIO Control for 1 and 2 */
	io_trap_in,		/* port 9 */
	io_trap_in,		/* port 10 */
	io_trap_in,		/* port 11 */
	io_trap_in,		/* port 12 */
	io_trap_in,		/* port 13 */
#ifdef HAS_DAZZLER
	cromemco_dazzler_flags_in, /* port 14 */
#else
	io_trap_in,		/* port 14 */
#endif //HAS_DAZZLER
	io_trap_in,		/* port 15 */
	io_trap_in,		/* port 16 */
	io_trap_in,		/* port 17 */
	io_no_card_in,		/* port 18 */ /* serial port   */
	io_no_card_in,		/* port 19 */ /*       "       */
	io_pport_in,		/* port 20 */ /* parallel port */
	io_pport_in,		/* port 21 */ /*       "       */
	io_trap_in,		/* port 22 */
	io_trap_in,		/* port 23 */
	io_trap_in,		/* port 24 */
	io_trap_in,		/* port 25 */
	io_trap_in,		/* port 26 */
	io_trap_in,		/* port 27 */
	io_trap_in,		/* port 28 */
	io_trap_in,		/* port 29 */
	io_trap_in,		/* port 30 */
	io_trap_in,		/* port 31 */
	io_trap_in,		/* port 32 */
	io_trap_in,		/* port 33 */
	io_trap_in,		/* port 34 */
	io_trap_in,		/* port 35 */
	io_trap_in,		/* port 36 */
	io_trap_in,		/* port 37 */
	io_trap_in,		/* port 38 */
	io_trap_in,		/* port 39 */
	io_trap_in,		/* port 40 */
	io_trap_in,		/* port 41 */
	io_trap_in,		/* port 42 */
	io_trap_in,		/* port 43 */
	io_trap_in,		/* port 44 */
	io_trap_in,		/* port 45 */
	io_trap_in,		/* port 46 */
	io_trap_in,		/* port 47 */
	io_trap_in,		/* port 48 */
	io_trap_in,		/* port 49 */
	io_trap_in,		/* port 50 */
	io_trap_in,		/* port 51 */
	io_trap_in,		/* port 52 */
	io_trap_in,		/* port 53 */
	io_trap_in,		/* port 54 */
	io_trap_in,		/* port 55 */
	io_trap_in,		/* port 56 */
	io_trap_in,		/* port 57 */
	io_trap_in,		/* port 58 */
	io_trap_in,		/* port 59 */
	io_trap_in,		/* port 60 */
	io_trap_in,		/* port 61 */
	io_trap_in,		/* port 62 */
	io_trap_in,		/* port 63 */
	io_trap_in,		/* port 64 */
	io_trap_in,		/* port 65 */
	io_trap_in,		/* port 66 */
	io_trap_in,		/* port 67 */
	io_trap_in,		/* port 68 */
	io_trap_in,		/* port 69 */
	io_trap_in,		/* port 70 */
	io_trap_in,		/* port 71 */
	io_trap_in,		/* port 72 */
	io_trap_in,		/* port 73 */
	io_trap_in,		/* port 74 */
	io_trap_in,		/* port 75 */
	io_trap_in,		/* port 76 */
	io_trap_in,		/* port 77 */
	io_trap_in,		/* port 78 */
	io_trap_in,		/* port 79 */
	io_trap_in,		/* port 80 */
	io_trap_in,		/* port 81 */
	io_trap_in,		/* port 82 */
	io_trap_in,		/* port 83 */
	io_trap_in,		/* port 84 */
	io_trap_in,		/* port 85 */
	io_trap_in,		/* port 86 */
	io_trap_in,		/* port 87 */
	io_trap_in,		/* port 88 */
	io_trap_in,		/* port 89 */
	io_trap_in,		/* port 90 */
	io_trap_in,		/* port 91 */
	io_trap_in,		/* port 92 */
	io_trap_in,		/* port 93 */
	io_trap_in,		/* port 94 */
	io_trap_in,		/* port 95 */
	io_trap_in,		/* port 96 */
	io_trap_in,		/* port 97 */
	io_trap_in,		/* port 98 */
	io_trap_in,		/* port 99 */
	io_trap_in,		/* port 100 */
	io_trap_in,		/* port 101 */
	io_trap_in,		/* port 102 */
	io_trap_in,		/* port 103 */
	io_trap_in,		/* port 104 */
	io_trap_in,		/* port 105 */
	io_trap_in,		/* port 106 */
	io_trap_in,		/* port 107 */
	io_trap_in,		/* port 108 */
	io_trap_in,		/* port 109 */
	io_trap_in,		/* port 110 */
	io_trap_in,		/* port 111 */
	io_trap_in,		/* port 112 */
	io_trap_in,		/* port 113 */
	io_trap_in,		/* port 114 */
	io_trap_in,		/* port 115 */
	io_trap_in,		/* port 116 */
	io_trap_in,		/* port 117 */
	io_trap_in,		/* port 118 */
	io_trap_in,		/* port 119 */
	io_trap_in,		/* port 120 */
	io_trap_in,		/* port 121 */
	io_trap_in,		/* port 122 */
	io_trap_in,		/* port 123 */
	io_trap_in,		/* port 124 */
	io_trap_in,		/* port 125 */
	io_trap_in,		/* port 126 */
	io_trap_in,		/* port 127 */
	io_trap_in,		/* port 128 */
	io_trap_in,		/* port 129 */
	io_trap_in,		/* port 130 */
	io_trap_in,		/* port 131 */
	io_trap_in,		/* port 132 */
	io_trap_in,		/* port 133 */
	io_trap_in,		/* port 134 */
	io_trap_in,		/* port 135 */
	io_trap_in,		/* port 136 */
	io_trap_in,		/* port 137 */
	io_trap_in,		/* port 138 */
	io_trap_in,		/* port 139 */
	io_trap_in,		/* port 140 */
	io_trap_in,		/* port 141 */
	io_trap_in,		/* port 142 */
	io_trap_in,		/* port 143 */
	io_trap_in,		/* port 144 */
	io_trap_in,		/* port 145 */
	io_trap_in,		/* port 146 */
	io_trap_in,		/* port 147 */
	io_trap_in,		/* port 148 */
	io_trap_in,		/* port 149 */
	io_trap_in,		/* port 150 */
	io_trap_in,		/* port 151 */
	io_trap_in,		/* port 152 */
	io_trap_in,		/* port 153 */
	io_trap_in,		/* port 154 */
	io_trap_in,		/* port 155 */
	io_trap_in,		/* port 156 */
	io_trap_in,		/* port 157 */
	io_trap_in,		/* port 158 */
	io_trap_in,		/* port 159 */
	hwctl_in,		/* port 160 */	/* virtual hardware control */
	io_trap_in,		/* port 161 */
	io_trap_in,		/* port 162 */
	io_trap_in,		/* port 163 */
	io_trap_in,		/* port 164 */
	io_trap_in,		/* port 165 */
	io_trap_in,		/* port 166 */
	io_trap_in,		/* port 167 */
	io_trap_in,		/* port 168 */
	io_trap_in,		/* port 169 */
	io_trap_in,		/* port 170 */
	io_trap_in,		/* port 171 */
	io_trap_in,		/* port 172 */
	io_trap_in,		/* port 173 */
	io_trap_in,		/* port 174 */
	io_trap_in,		/* port 175 */
	io_trap_in,		/* port 176 */
	io_trap_in,		/* port 177 */
	io_trap_in,		/* port 178 */
	io_trap_in,		/* port 179 */
	io_trap_in,		/* port 180 */
	io_trap_in,		/* port 181 */
	io_trap_in,		/* port 182 */
	io_trap_in,		/* port 183 */
	io_trap_in,		/* port 184 */
	io_trap_in,		/* port 185 */
	io_trap_in,		/* port 186 */
	io_trap_in,		/* port 187 */
	io_trap_in,		/* port 188 */
	io_trap_in,		/* port 189 */
	io_trap_in,		/* port 190 */
	io_trap_in,		/* port 191 */
	io_trap_in,		/* port 192 */
	io_trap_in,		/* port 193 */
	io_trap_in,		/* port 194 */
	io_trap_in,		/* port 195 */
	io_trap_in,		/* port 196 */
	io_trap_in,		/* port 197 */
	io_trap_in,		/* port 198 */
	io_trap_in,		/* port 199 */
	io_trap_in,		/* port 200 */
	io_trap_in,		/* port 201 */
	io_trap_in,		/* port 202 */
	io_trap_in,		/* port 203 */
	io_trap_in,		/* port 204 */
	io_trap_in,		/* port 205 */
	io_trap_in,		/* port 206 */
	io_trap_in,		/* port 207 */
	io_trap_in,		/* port 208 */
	io_trap_in,		/* port 209 */
	io_trap_in,		/* port 210 */
	io_trap_in,		/* port 211 */
	io_trap_in,		/* port 212 */
	io_trap_in,		/* port 213 */
	io_trap_in,		/* port 214 */
	io_trap_in,		/* port 215 */
	io_trap_in,		/* port 216 */
	io_trap_in,		/* port 217 */
	io_trap_in,		/* port 218 */
	io_trap_in,		/* port 219 */
	io_trap_in,		/* port 220 */
	io_trap_in,		/* port 221 */
	io_trap_in,		/* port 222 */
	io_trap_in,		/* port 223 */
	io_trap_in,		/* port 224 */
	io_trap_in,		/* port 225 */
	io_trap_in,		/* port 226 */
	io_trap_in,		/* port 227 */
	io_trap_in,		/* port 228 */
	io_trap_in,		/* port 229 */
	io_trap_in,		/* port 230 */
	io_trap_in,		/* port 231 */
	io_trap_in,		/* port 232 */
	io_trap_in,		/* port 233 */
	io_trap_in,		/* port 234 */
	io_trap_in,		/* port 235 */
	io_trap_in,		/* port 236 */
	io_trap_in,		/* port 237 */
	io_trap_in,		/* port 238 */
	io_no_card_in,		/* port 239 */ /* unknown card */
	io_trap_in,		/* port 240 */
	io_trap_in,		/* port 241 */
	io_trap_in,		/* port 242 */
	ctrl_port_in,		/* port 243 */ /* software memory control */
	io_trap_in,		/* port 244 */
	io_trap_in,		/* port 245 */
	lpt_in,			/* port 246 */ /* IMSAI PTR-300 line printer */
	io_no_card_in,		/* port 247 */ /* prio interrupt controller */
	io_trap_in,		/* port 248 */
	io_trap_in,		/* port 249 */
	io_trap_in,		/* port 250 */
	io_trap_in,		/* port 251 */
	io_trap_in,		/* port 252 */
	imsai_fif_in,		/* port 253 */ /* FIF disk controller */
	io_no_card_in,		/* port 254 */ /* memory write protect */
	fp_in			/* port 255 */ /* front panel */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
static void (*port_out[256]) (BYTE) = {
	io_trap_out,		/* port 0 */
	io_trap_out,		/* port 1 */
	imsai_sio1_data_out,	/* port 2 */ /* SIO 1 connected to console */
	imsai_sio1_status_out,	/* port 3 */
	io_no_card_out,		/* port 4 */ /* SIO 2 not connected */
	io_no_card_out,		/* port 5 */
	io_trap_out,		/* port 6 */
	io_trap_out,		/* port 7 */
	io_no_card_out,		/* port 8 */ /* SIO Control for 1 and 2 */
	io_trap_out,		/* port 9 */
	io_trap_out,		/* port 10 */
	io_trap_out,		/* port 11 */
	io_trap_out,		/* port 12 */
	io_trap_out,		/* port 13 */
#ifdef HAS_DAZZLER
	cromemco_dazzler_ctl_out,	/* port 14 */
	cromemco_dazzler_format_out,	/* port 15 */
#else
	io_trap_out,		/* port 14 */
	io_trap_out,		/* port 15 */
#endif //HAS_DAZZLER
	io_trap_out,		/* port 16 */
	io_trap_out,		/* port 17 */
	io_no_card_out,		/* port 18 */ /* serial port   */
	io_no_card_out,		/* port 19 */ /*       "       */
	io_no_card_out,		/* port 20 */ /* parallel port */
	io_no_card_out,		/* port 21 */ /*       "       */
	io_trap_out,		/* port 22 */
	io_trap_out,		/* port 23 */
	io_trap_out,		/* port 24 */
	io_trap_out,		/* port 25 */
	io_trap_out,		/* port 26 */
	io_trap_out,		/* port 27 */
	io_trap_out,		/* port 28 */
	io_trap_out,		/* port 29 */
	io_trap_out,		/* port 30 */
	io_trap_out,		/* port 31 */
	io_trap_out,		/* port 32 */
	io_trap_out,		/* port 33 */
	io_trap_out,		/* port 34 */
	io_trap_out,		/* port 35 */
	io_trap_out,		/* port 36 */
	io_trap_out,		/* port 37 */
	io_trap_out,		/* port 38 */
	io_trap_out,		/* port 39 */
	io_trap_out,		/* port 40 */
	io_trap_out,		/* port 41 */
	io_trap_out,		/* port 42 */
	io_trap_out,		/* port 43 */
	io_trap_out,		/* port 44 */
	io_trap_out,		/* port 45 */
	io_trap_out,		/* port 46 */
	io_trap_out,		/* port 47 */
	io_trap_out,		/* port 48 */
	io_trap_out,		/* port 49 */
	io_trap_out,		/* port 50 */
	io_trap_out,		/* port 51 */
	io_trap_out,		/* port 52 */
	io_trap_out,		/* port 53 */
	io_trap_out,		/* port 54 */
	io_trap_out,		/* port 55 */
	io_trap_out,		/* port 56 */
	io_trap_out,		/* port 57 */
	io_trap_out,		/* port 58 */
	io_trap_out,		/* port 59 */
	io_trap_out,		/* port 60 */
	io_trap_out,		/* port 61 */
	io_trap_out,		/* port 62 */
	io_trap_out,		/* port 63 */
	io_trap_out,		/* port 64 */
	io_trap_out,		/* port 65 */
	io_trap_out,		/* port 66 */
	io_trap_out,		/* port 67 */
	io_trap_out,		/* port 68 */
	io_trap_out,		/* port 69 */
	io_trap_out,		/* port 70 */
	io_trap_out,		/* port 71 */
	io_trap_out,		/* port 72 */
	io_trap_out,		/* port 73 */
	io_trap_out,		/* port 74 */
	io_trap_out,		/* port 75 */
	io_trap_out,		/* port 76 */
	io_trap_out,		/* port 77 */
	io_trap_out,		/* port 78 */
	io_trap_out,		/* port 79 */
	io_trap_out,		/* port 80 */
	io_trap_out,		/* port 81 */
	io_trap_out,		/* port 82 */
	io_trap_out,		/* port 83 */
	io_trap_out,		/* port 84 */
	io_trap_out,		/* port 85 */
	io_trap_out,		/* port 86 */
	io_trap_out,		/* port 87 */
	io_trap_out,		/* port 88 */
	io_trap_out,		/* port 89 */
	io_trap_out,		/* port 90 */
	io_trap_out,		/* port 91 */
	io_trap_out,		/* port 92 */
	io_trap_out,		/* port 93 */
	io_trap_out,		/* port 94 */
	io_trap_out,		/* port 95 */
	io_trap_out,		/* port 96 */
	io_trap_out,		/* port 97 */
	io_trap_out,		/* port 98 */
	io_trap_out,		/* port 99 */
	io_trap_out,		/* port 100 */
	io_trap_out,		/* port 101 */
	io_trap_out,		/* port 102 */
	io_trap_out,		/* port 103 */
	io_trap_out,		/* port 104 */
	io_trap_out,		/* port 105 */
	io_trap_out,		/* port 106 */
	io_trap_out,		/* port 107 */
	io_trap_out,		/* port 108 */
	io_trap_out,		/* port 109 */
	io_trap_out,		/* port 110 */
	io_trap_out,		/* port 111 */
	io_trap_out,		/* port 112 */
	io_trap_out,		/* port 113 */
	io_trap_out,		/* port 114 */
	io_trap_out,		/* port 115 */
	io_trap_out,		/* port 116 */
	io_trap_out,		/* port 117 */
	io_trap_out,		/* port 118 */
	io_trap_out,		/* port 119 */
	io_trap_out,		/* port 120 */
	io_trap_out,		/* port 121 */
	io_trap_out,		/* port 122 */
	io_trap_out,		/* port 123 */
	io_trap_out,		/* port 124 */
	io_trap_out,		/* port 125 */
	io_trap_out,		/* port 126 */
	io_trap_out,		/* port 127 */
	io_trap_out,		/* port 128 */
	io_trap_out,		/* port 129 */
	io_trap_out,		/* port 130 */
	io_trap_out,		/* port 131 */
	io_trap_out,		/* port 132 */
	io_trap_out,		/* port 133 */
	io_trap_out,		/* port 134 */
	io_trap_out,		/* port 135 */
	io_trap_out,		/* port 136 */
	io_trap_out,		/* port 137 */
	io_trap_out,		/* port 138 */
	io_trap_out,		/* port 139 */
	io_trap_out,		/* port 140 */
	io_trap_out,		/* port 141 */
	io_trap_out,		/* port 142 */
	io_trap_out,		/* port 143 */
	io_trap_out,		/* port 144 */
	io_trap_out,		/* port 145 */
	io_trap_out,		/* port 146 */
	io_trap_out,		/* port 147 */
	io_trap_out,		/* port 148 */
	io_trap_out,		/* port 149 */
	io_trap_out,		/* port 150 */
	io_trap_out,		/* port 151 */
	io_trap_out,		/* port 152 */
	io_trap_out,		/* port 153 */
	io_trap_out,		/* port 154 */
	io_trap_out,		/* port 155 */
	io_trap_out,		/* port 156 */
	io_trap_out,		/* port 157 */
	io_trap_out,		/* port 158 */
	io_trap_out,		/* port 159 */
	hwctl_out,		/* port 160 */	/* virtual hardware control */
	io_trap_out,		/* port 161 */
	io_trap_out,		/* port 162 */
	io_trap_out,		/* port 163 */
	io_trap_out,		/* port 164 */
	io_trap_out,		/* port 165 */
	io_trap_out,		/* port 166 */
	io_trap_out,		/* port 167 */
	io_trap_out,		/* port 168 */
	io_trap_out,		/* port 169 */
	io_trap_out,		/* port 170 */
	io_trap_out,		/* port 171 */
	io_trap_out,		/* port 172 */
	io_trap_out,		/* port 173 */
	io_trap_out,		/* port 174 */
	io_trap_out,		/* port 175 */
	io_trap_out,		/* port 176 */
	io_trap_out,		/* port 177 */
	io_trap_out,		/* port 178 */
	io_trap_out,		/* port 179 */
	io_trap_out,		/* port 180 */
	io_trap_out,		/* port 181 */
	io_trap_out,		/* port 182 */
	io_trap_out,		/* port 183 */
	io_trap_out,		/* port 184 */
	io_trap_out,		/* port 185 */
	io_trap_out,		/* port 186 */
	io_trap_out,		/* port 187 */
	io_trap_out,		/* port 188 */
	io_trap_out,		/* port 189 */
	io_trap_out,		/* port 190 */
	io_trap_out,		/* port 191 */
	io_trap_out,		/* port 192 */
	io_trap_out,		/* port 193 */
	io_trap_out,		/* port 194 */
	io_trap_out,		/* port 195 */
	io_trap_out,		/* port 196 */
	io_trap_out,		/* port 197 */
	io_trap_out,		/* port 198 */
	io_trap_out,		/* port 199 */
	io_trap_out,		/* port 200 */
	io_trap_out,		/* port 201 */
	io_trap_out,		/* port 202 */
	io_trap_out,		/* port 203 */
	io_trap_out,		/* port 204 */
	io_trap_out,		/* port 205 */
	io_trap_out,		/* port 206 */
	io_trap_out,		/* port 207 */
	io_trap_out,		/* port 208 */
	io_trap_out,		/* port 209 */
	io_trap_out,		/* port 210 */
	io_trap_out,		/* port 211 */
	io_trap_out,		/* port 212 */
	io_trap_out,		/* port 213 */
	io_trap_out,		/* port 214 */
	io_trap_out,		/* port 215 */
	io_trap_out,		/* port 216 */
	io_trap_out,		/* port 217 */
	io_trap_out,		/* port 218 */
	io_trap_out,		/* port 219 */
	io_trap_out,		/* port 220 */
	io_trap_out,		/* port 221 */
	io_trap_out,		/* port 222 */
	io_trap_out,		/* port 223 */
	io_trap_out,		/* port 224 */
	io_trap_out,		/* port 225 */
	io_trap_out,		/* port 226 */
	io_trap_out,		/* port 227 */
	io_trap_out,		/* port 228 */
	io_trap_out,		/* port 229 */
	io_trap_out,		/* port 230 */
	io_trap_out,		/* port 231 */
	io_trap_out,		/* port 232 */
	io_trap_out,		/* port 233 */
	io_trap_out,		/* port 234 */
	io_trap_out,		/* port 235 */
	io_trap_out,		/* port 236 */
	io_trap_out,		/* port 237 */
	io_trap_out,		/* port 238 */
	io_no_card_out,		/* port 239 */ /* unknown card */
	io_trap_out,		/* port 240 */
	io_trap_out,		/* port 241 */
	io_trap_out,		/* port 242 */
	ctrl_port_out,		/* port 243 */ /* software memory control */
	io_trap_out,		/* port 244 */
	io_trap_out,		/* port 245 */
	lpt_out,		/* port 246 */ /* IMSAI PTR-300 line printer */
	io_no_card_out,		/* port 247 */ /* prio interrupt controller */
	io_trap_out,		/* port 248 */
	io_trap_out,		/* port 249 */
	io_trap_out,		/* port 250 */
	io_trap_out,		/* port 251 */
	io_trap_out,		/* port 252 */
	imsai_fif_out,		/* port 253 */ /* FIF disk controller */
	io_no_card_out,		/* port 254 */ /* memory write protect */
	fp_out			/* port 255 */ /* front panel */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	/* initialise IMSAI VIO if firmware is loaded */
	if (!strncmp((char *) mem_base() + 0xfffd, "VI0", 3)) {
		imsai_vio_init();
	} else {
		/* if no VIO firmware loaded release the ROM and RAM */
		MEM_RELEASE(60);
		MEM_RELEASE(61);
		MEM_RELEASE(62);
		MEM_RELEASE(63);
	}

	imsai_fif_reset();
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
	/* close line printer file */
	if (printer != 0)
		close(printer);

#ifdef HAS_DAZZLER
	/* shutdown DAZZLER */
	cromemco_dazzler_off();
#endif

	/* shutdown VIO */
	imsai_vio_off();
}

/*
 *	This function is to reset the I/O devices. It is
 *	called from the CPU simulation when an External Clear is performed.
 */
void reset_io(void)
{
#ifdef HAS_DAZZLER
	cromemco_dazzler_off();
#endif
	imsai_fif_reset();
	hwctl_lock = 0xff;
}

/*
 *	This is the main handler for all IN op-codes,
 *	called by the simulator. It calls the input
 *	function for port addr.
 */
BYTE io_in(BYTE addrl, BYTE addrh)
{
	int val;

	io_port = addrl;
	io_data = (*port_in[addrl]) ();

	cpu_bus = CPU_WO | CPU_INP;

	fp_clock += 3;
	fp_led_address = (addrh << 8) + addrl;
	fp_led_data = io_data;
	fp_sampleData();
	val = wait_step();

	/* when single stepped INP get last set value of port */
	if (val)
		io_data = (*port_in[io_port]) ();

	return(io_data);
}

/*
 *	This is the main handler for all OUT op-codes,
 *	called by the simulator. It calls the output
 *	function for port addr.
 */
void io_out(BYTE addrl, BYTE addrh, BYTE data)
{
	io_port = addrl;
	io_data = data;
	(*port_out[addrl]) (data);

	cpu_bus = CPU_OUT;

	fp_clock += 6;
	fp_led_address = (addrh << 8) + addrl;
	fp_led_data = io_data;
	fp_sampleData();
	wait_step();
}

/*
 *	I/O input trap function
 *	This function should be added into all unused
 *	entries of the input port array. It can stop the
 *	emulation with an I/O error.
 */
static BYTE io_trap_in(void)
{
	if (i_flag) {
		cpu_error = IOTRAPIN;
		cpu_state = STOPPED;
	}
	return((BYTE) 0xff);
}

/*
 *	Same as above, but don't trap as I/O error.
 *	Used for input ports where I/O cards might be
 *	installed, but haven't.
 */
static BYTE io_no_card_in(void)
{
	return((BYTE) 0xff);
}

/*
 *	I/O output trap function
 *	This function should be added into all unused
 *	entries of the output port array. It can stop the
 *	emulation with an I/O error.
 */
static void io_trap_out(BYTE data)
{
	data = data; /* to avoid compiler warning */

	if (i_flag) {
		cpu_error = IOTRAPOUT;
		cpu_state = STOPPED;
	}
}

/*
 *	Same as above, but don't trap as I/O error.
 *	Used for output ports where I/O cards might be
 *	installed, but haven't.
 */
static void io_no_card_out(BYTE data)
{
	data = data; /* to avoid compiler warning */
}

/*
 *	Read input from front panel switches
 */
static BYTE fp_in(void)
{
	return(address_switch >> 8);
}

/*
 *	Write output to front panel lights
 */
static void fp_out(BYTE data)
{
	fp_led_output = data;
}

/*
 *	timer interrupt causes RST 38 in IM 0 and IM 1
 */
static void int_timer(int sig)
{
	sig = sig;	/* to avoid compiler warning */

	int_int = 1;
	int_data = 0xff;	/* RST 38H */
}

/*
 *	Input from virtual hardware control port
 *	returns lock status of the port
 */
static BYTE hwctl_in(void)
{
	return(hwctl_lock);
}

/*
 *	Port is locked until magic number 0xaa is received!
 *
 *	Virtual hardware control output.
 *	Doesn't exist in the real machine, used to shutdown
 *	and for RST 38H interrupts every 10ms.
 *
 *	bit 0 = 1	start interrupt timer
 *	bit 0 = 0	stop interrupt timer
 *	bit 7 = 1	halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
	static struct itimerval tim;
	static struct sigaction newact;

	/* if port is locked do nothing */
	if (hwctl_lock && (data != 0xaa))
		return;

	/* unlock port ? */
	if (hwctl_lock && (data == 0xaa)) {
		hwctl_lock = 0;
		return;
	}
	
	/* process output to unlocked port */

        if (data & 128) {
                cpu_error = IOHALT;
                cpu_state = STOPPED;
	}

        if (data & 1) {
		newact.sa_handler = int_timer;
		memset((void *) &newact.sa_mask, 0, sizeof(newact.sa_mask));
		newact.sa_flags = 0;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 10000;
		tim.it_interval.tv_sec = 0;
		tim.it_interval.tv_usec = 10000;
		setitimer(ITIMER_REAL, &tim, NULL);
	} else {
		newact.sa_handler = SIG_IGN;
		memset((void *) &newact.sa_mask, 0, sizeof(newact.sa_mask));
		newact.sa_flags = 0;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &tim, NULL);
	}
}

/*
 *	Print into the printer file any data with bit 7 = 0.
 *	Data with bit 7 = 1 are commands which we ignore here.
 */
static void lpt_out(BYTE data)
{
	if ((printer == 0) && !(data & 0x80))
		printer = creat("printer.txt", 0664);

	if ((data != '\r') && !(data & 0x80)) {
again:
		if (write(printer, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				LOGE(TAG, "can't write to printer");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}

#ifdef HAS_NETSERVER
		net_device_send(DEV_LPT, (char *) &data, 1);
#endif
	}
}

/*
 *	I/O handler for line printer in:
 *	The IMSAI line printer returns F4 for ready and F0 for busy.
 *	Our printer file never is busy, so always return ready.
 */
static BYTE lpt_in(void)
{
	return((BYTE) 0xf4);
}

/*
 *	Parallel ports not implemented yet.
 *	Needs to return 0, otherwise the ROM monitor
 *	detects a parallel keyboard and won't use SIO 1
 */
static BYTE io_pport_in(void)
{
	return((BYTE) 0);
}

/*
 *	Return status of the IMSAI VIO keyboard
 */
static BYTE imsai_kbd_status_in(void)
{
	extern int imsai_kbd_status;

	return((BYTE) imsai_kbd_status);
}

/*
 *	Return next data byte from IMSAI VIO keyboard
 */
static BYTE imsai_kbd_data_in(void)
{
	extern int imsai_kbd_data, imsai_kbd_status;
	int data;

	if (imsai_kbd_data == -1)
		return((BYTE) 0);

	/* take over data and reset */
	data = imsai_kbd_data;
	imsai_kbd_data = -1;
	imsai_kbd_status = 0;

	return((BYTE) data);
}
