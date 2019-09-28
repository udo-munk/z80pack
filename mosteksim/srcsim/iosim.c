/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module of the simulator contains the I/O simulation
 * for a Mostek AID-80F or SYS-80FT
 *
 * History:
 * 15-SEP-19 (Mike Douglas) created from iosim.c for the Altair
 * 28-SEP-19 (Udo Munk) use logging
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
#include "../../iodevices/mostek-cpu.h"
#include "../../iodevices/mostek-fdc.h"
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"

static const char *TAG = "I/O";

/*
 *	Forward declarations for I/O functions
 */
static BYTE io_trap_in(void);
static void io_trap_out(BYTE);
static BYTE io_no_card_in(void);
static void io_no_card_out(BYTE);

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
static BYTE (*port_in[256]) (void) = {
	io_trap_in,			/* port 0 */ 
	io_trap_in,			/* port 1 */
	io_trap_in,			/* port 2 */
	io_trap_in,			/* port 3 */
	io_trap_in,			/* port 4 */
	io_trap_in,			/* port 5 */
	io_trap_in,			/* port 6 */
	io_trap_in,			/* port 7 */
	io_trap_in,			/* port 8 */
	io_trap_in,			/* port 9 */
	io_trap_in,			/* port 10 */
	io_trap_in,			/* port 11 */
	io_trap_in,			/* port 12 */
	io_trap_in,			/* port 13 */
	io_trap_in,	 		/* port 14 */ 
	io_trap_in,			/* port 15 */
	io_trap_in,			/* port 16 */ 
	io_trap_in,			/* port 17 */ 
	io_trap_in,			/* port 18 */ 
	io_trap_in,			/* port 19 */
	io_trap_in,			/* port 20 */
	io_trap_in,			/* port 21 */
	io_trap_in,			/* port 22 */
	io_trap_in,			/* port 23 */
	io_trap_in,			/* port 24 */
	io_trap_in,			/* port 25 */
	io_trap_in,			/* port 26 */
	io_trap_in,			/* port 27 */
	io_trap_in,			/* port 28 */
	io_trap_in,			/* port 29 */
	io_trap_in,			/* port 30 */
	io_trap_in,			/* port 31 */
	io_trap_in,			/* port 32 */
	io_trap_in,			/* port 33 */
	io_trap_in,			/* port 34 */
	io_trap_in,			/* port 35 */
	io_trap_in,			/* port 36 */
	io_trap_in,			/* port 37 */
	io_trap_in,			/* port 38 */
	io_trap_in,			/* port 39 */
	io_trap_in,			/* port 40 */
	io_trap_in,			/* port 41 */
	io_trap_in,			/* port 42 */
	io_trap_in,			/* port 43 */
	io_trap_in,			/* port 44 */
	io_trap_in,			/* port 45 */
	io_trap_in,			/* port 46 */
	io_trap_in,			/* port 47 */
	io_trap_in,			/* port 48 */
	io_trap_in,			/* port 49 */
	io_trap_in,			/* port 50 */
	io_trap_in,			/* port 51 */
	io_trap_in,			/* port 52 */
	io_trap_in,			/* port 53 */
	io_trap_in,			/* port 54 */
	io_trap_in,			/* port 55 */
	io_trap_in,			/* port 56 */
	io_trap_in,			/* port 57 */
	io_trap_in,			/* port 58 */
	io_trap_in,			/* port 59 */
	io_trap_in,			/* port 60 */
	io_trap_in,			/* port 61 */
	io_trap_in,			/* port 62 */
	io_trap_in,			/* port 63 */
	io_trap_in,			/* port 64 */
	io_trap_in,			/* port 65 */
	io_trap_in,			/* port 66 */
	io_trap_in,			/* port 67 */
	io_trap_in,			/* port 68 */
	io_trap_in,			/* port 69 */
	io_trap_in,			/* port 70 */
	io_trap_in,			/* port 71 */
	io_trap_in,			/* port 72 */
	io_trap_in,			/* port 73 */
	io_trap_in,			/* port 74 */
	io_trap_in,			/* port 75 */
	io_trap_in,			/* port 76 */
	io_trap_in,			/* port 77 */
	io_trap_in,			/* port 78 */
	io_trap_in,			/* port 79 */
	io_trap_in,			/* port 80 */
	io_trap_in,			/* port 81 */
	io_trap_in,			/* port 82 */
	io_trap_in,			/* port 83 */
	io_trap_in,			/* port 84 */
	io_trap_in,			/* port 85 */
	io_trap_in,			/* port 86 */
	io_trap_in,			/* port 87 */
	io_trap_in,			/* port 88 */
	io_trap_in,			/* port 89 */
	io_trap_in,			/* port 90 */
	io_trap_in,			/* port 91 */
	io_trap_in,			/* port 92 */
	io_trap_in,			/* port 93 */
	io_trap_in,			/* port 94 */
	io_trap_in,			/* port 95 */
	io_trap_in,			/* port 96 */
	io_trap_in,			/* port 97 */
	io_trap_in,			/* port 98 */
	io_trap_in,			/* port 99 */
	io_trap_in,			/* port 100 */
	io_trap_in,			/* port 101 */
	io_trap_in,			/* port 102 */
	io_trap_in,			/* port 103 */
	io_trap_in,			/* port 104 */
	io_trap_in,			/* port 105 */
	io_trap_in,			/* port 106 */
	io_trap_in,			/* port 107 */
	io_trap_in,			/* port 108 */
	io_trap_in,			/* port 109 */
	io_trap_in,			/* port 110 */
	io_trap_in,			/* port 111 */
	io_trap_in,			/* port 112 */
	io_trap_in,			/* port 113 */
	io_trap_in,			/* port 114 */
	io_trap_in,			/* port 115 */
	io_trap_in,			/* port 116 */
	io_trap_in,			/* port 117 */
	io_trap_in,			/* port 118 */
	io_trap_in,			/* port 119 */
	io_trap_in,			/* port 120 */
	io_trap_in,			/* port 121 */
	io_trap_in,			/* port 122 */
	io_trap_in,			/* port 123 */
	io_trap_in,			/* port 124 */
	io_trap_in,			/* port 125 */
	io_trap_in,			/* port 126 */
	io_trap_in,			/* port 127 */
	io_trap_in,			/* port 128 */
	io_trap_in,			/* port 129 */
	io_trap_in,			/* port 130 */
	io_trap_in,			/* port 131 */
	io_trap_in,			/* port 132 */
	io_trap_in,			/* port 133 */
	io_trap_in,			/* port 134 */
	io_trap_in,			/* port 135 */
	io_trap_in,			/* port 136 */
	io_trap_in,			/* port 137 */
	io_trap_in,			/* port 138 */
	io_trap_in,			/* port 139 */
	io_trap_in,			/* port 140 */
	io_trap_in,			/* port 141 */
	io_trap_in,			/* port 142 */
	io_trap_in,			/* port 143 */
	io_trap_in,			/* port 144 */
	io_trap_in,			/* port 145 */
	io_trap_in,			/* port 146 */
	io_trap_in,			/* port 147 */
	io_trap_in,			/* port 148 */
	io_trap_in,			/* port 149 */
	io_trap_in,			/* port 150 */
	io_trap_in,			/* port 151 */
	io_trap_in,			/* port 152 */
	io_trap_in,			/* port 153 */
	io_trap_in,			/* port 154 */
	io_trap_in,			/* port 155 */
	io_trap_in,			/* port 156 */
	io_trap_in,			/* port 157 */
	io_trap_in,			/* port 158 */
	io_trap_in,			/* port 159 */
	io_trap_in,			/* port 160 */
	io_trap_in,			/* port 161 */
	io_trap_in,			/* port 162 */
	io_trap_in,			/* port 163 */
	io_trap_in,			/* port 164 */
	io_trap_in,			/* port 165 */
	io_trap_in,			/* port 166 */
	io_trap_in,			/* port 167 */
	io_trap_in,			/* port 168 */
	io_trap_in,			/* port 169 */
	io_trap_in,			/* port 170 */
	io_trap_in,			/* port 171 */
	io_trap_in,			/* port 172 */
	io_trap_in,			/* port 173 */
	io_trap_in,			/* port 174 */
	io_trap_in,			/* port 175 */
	io_trap_in,			/* port 176 */
	io_trap_in,			/* port 177 */
	io_trap_in,			/* port 178 */
	io_trap_in,			/* port 179 */
	io_trap_in,			/* port 180 */
	io_trap_in,			/* port 181 */
	io_trap_in,			/* port 182 */
	io_trap_in,			/* port 183 */
	io_trap_in,			/* port 184 */
	io_trap_in,			/* port 185 */
	io_trap_in,			/* port 186 */
	io_trap_in,			/* port 187 */
	io_trap_in,			/* port 188 */
	io_trap_in,			/* port 189 */
	io_trap_in,			/* port 190 */
	io_trap_in,			/* port 191 */
	io_trap_in,			/* port 192 */
	io_trap_in,			/* port 193 */
	io_trap_in,			/* port 194 */
	io_trap_in,			/* port 195 */
	io_trap_in,			/* port 196 */
	io_trap_in,			/* port 197 */
	io_trap_in,			/* port 198 */
	io_trap_in,			/* port 199 */
	io_trap_in,			/* port 200 */
	io_trap_in,			/* port 201 */
	io_trap_in,			/* port 202 */
	io_trap_in,			/* port 203 */
	io_trap_in,			/* port 204 */
	io_trap_in,			/* port 205 */
	io_trap_in,			/* port 206 */
	io_trap_in,			/* port 207 */
	io_no_card_in,		/* port 208 (d0) PIO1 Data A */
	io_no_card_in,		/* port 209 (d1) PIO1 Control A */
	io_no_card_in,		/* port 210 (d2) PIO1 Data B */
	io_no_card_in,		/* port 211 (d3) PIO1 Control B */
	io_no_card_in,		/* port 212 (d4) PIO2 Data A */
	io_no_card_in,		/* port 213 (d5) PIO2 Control A */
	io_no_card_in,		/* port 214 (d6) PIO2 Data B */
	io_no_card_in,		/* port 215 (d7) PIO2 Control B */
	io_no_card_in,		/* port 216 (d8) CTC 0 (baud rate) */
	io_no_card_in,		/* port 217 (d9) CTC 1 */
	io_no_card_in,		/* port 218 (da) CTC 2 */
	io_no_card_in,		/* port 219 (db) CTC 3 */
	sio_data_in,		/* port 220 (dc) SIO Data */
	sio_status_in,		/* port 221 (dd) SIO Status */
	sio_handshake_in,	/* port 222 (de) Sys Control (handshake lines in) */
	io_no_card_in,		/* port 223 (df) Debug Control */
	io_trap_in,			/* port 224 */
	io_trap_in,			/* port 225 */
	fdcBoard_stat_in,	/* port 226 (e2) FDC board status */
	fdcBoard_ctl_in,	/* port 227 (e3) FDC board control */
	fdc1771_stat_in,	/* port 228 (e4) WD1771 Command/Status */
	fdc1771_track_in,	/* port 229 (e5) WD1771 Track */
	fdc1771_sec_in,		/* port 230 (e6) WD1771 Sector */
	fdc1771_data_in,	/* port 231 (e7) WD1771 Data */
	io_trap_in,			/* port 232 */
	io_trap_in,			/* port 233 */
	io_trap_in,			/* port 234 */
	io_trap_in,			/* port 235 */
	io_trap_in,			/* port 236 */
	io_trap_in,			/* port 237 */
	io_trap_in,			/* port 238 */
	io_trap_in,			/* port 239 */
	io_trap_in,			/* port 240 */
	io_trap_in,			/* port 241 */
	io_trap_in,			/* port 242 */
	io_trap_in,			/* port 243 */
	io_trap_in,			/* port 244 */
	io_trap_in,			/* port 245 */
	io_trap_in,			/* port 246 */
	io_trap_in,			/* port 247 */
	io_trap_in,			/* port 248 */
	io_trap_in,			/* port 249 */
	io_trap_in,			/* port 250 */
	io_trap_in,			/* port 251 */
	io_trap_in,			/* port 252 */
	io_trap_in,			/* port 253 */
	io_trap_in,			/* port 254 */
	io_trap_in			/* port 255 */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
static void (*port_out[256]) (BYTE) = {
	io_trap_out,		/* port 0 */
	io_trap_out,		/* port 1 */
	io_trap_out,		/* port 2 */
	io_trap_out,		/* port 3 */
	io_trap_out,		/* port 4 */
	io_trap_out,		/* port 5 */
	io_trap_out,		/* port 6 */
	io_trap_out,		/* port 7 */ 
	io_trap_out,		/* port 8 */
	io_trap_out,		/* port 9 */
	io_trap_out,		/* port 10 */
	io_trap_out,		/* port 11 */
	io_trap_out,		/* port 12 */
	io_trap_out,		/* port 13 */
	io_trap_out,		/* port 14 */
	io_trap_out,		/* port 15 */
	io_trap_out,		/* port 16 */
	io_trap_out,		/* port 17 */
	io_trap_out,		/* port 18 */
	io_trap_out,		/* port 19 */
	io_trap_out,		/* port 20 */
	io_trap_out,		/* port 21 */
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
	io_trap_out,		/* port 160 */
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
	io_no_card_out,		/* port 208 (d0) PIO1 Data A */
	io_no_card_out,		/* port 209 (d1) PIO1 Control A */
	io_no_card_out,		/* port 210 (d2) PIO1 Data B */
	io_no_card_out,		/* port 211 (d3) PIO1 Control B */
	io_no_card_out,		/* port 212 (d4) PIO2 Data A */
	io_no_card_out,		/* port 213 (d5) PIO2 Control A */
	io_no_card_out,		/* port 214 (d6) PIO2 Data B */
	io_no_card_out,		/* port 215 (d7) PIO2 Control B */
	io_no_card_out,		/* port 216 (d8) CTC 0 (baud rate) */
	io_no_card_out,		/* port 217 (d9) CTC 1 */
	io_no_card_out,		/* port 218 (da) CTC 2 */
	io_no_card_out,		/* port 219 (db) CTC 3 */
	sio_data_out,		/* port 220 (dc) SIO Data */
	sio_control_out,	/* port 221 (dd) SIO Control */
	sio_handshake_out,	/* port 222 (de) Sys Control (handshake lines out) */
	io_no_card_out,		/* port 223 (df) Debug Control */
	io_trap_out,		/* port 224 */
	io_trap_out,		/* port 225 */
	io_no_card_out,		/* port 226 (e2) FDC board status */
	fdcBoard_ctl_out,	/* port 227 (e3) FDC board control */
	fdc1771_cmd_out,	/* port 228 (e4) WD1771 Command/Status */
	fdc1771_track_out,	/* port 229 (e5) WD1771 Track */
	fdc1771_sec_out,	/* port 230 (e6) WD1771 Sector */
	fdc1771_data_out,	/* port 231 (e7) WD1771 Data */
	io_trap_out,		/* port 232 */
	io_trap_out,		/* port 233 */
	io_trap_out,		/* port 234 */
	io_trap_out,		/* port 235 */
	io_trap_out,		/* port 236 */
	io_trap_out,		/* port 237 */
	io_trap_out,		/* port 238 */
	io_trap_out,		/* port 239 */
	io_trap_out,		/* port 240 */
	io_trap_out,		/* port 241 */
	io_trap_out,		/* port 242 */
	io_trap_out,		/* port 243 */
	io_trap_out,		/* port 244 */
	io_trap_out,		/* port 245 */
	io_trap_out,		/* port 246 */
	io_trap_out,		/* port 247 */
	io_trap_out,		/* port 248 */
	io_trap_out,		/* port 249 */
	io_trap_out,		/* port 250 */
	io_trap_out,		/* port 251 */
	io_trap_out,		/* port 252 */ 
	io_trap_out,		/* port 253 */
	io_trap_out,		/* port 254 */
	io_trap_out		/* port 255 */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
}

/*
 *	This is the main handler for all IN op-codes,
 *	called by the simulator. It calls the input
 *	function for port addr.
 */
BYTE io_in(BYTE addrl, BYTE addrh)
{
	addrh = addrh;		/* to avoid compiler warning */
	io_port = addrl;
	io_data = (*port_in[addrl]) ();
	LOGD(TAG, "input %02x from port %0sx", io_data, io_port);
	return(io_data);
}

/*
 *	This is the main handler for all OUT op-codes,
 *	called by the simulator. It calls the output
 *	function for port addr.
 */
void io_out(BYTE addrl, BYTE addrh, BYTE data)
{
	addrh = addrh;		/* to avoid compiler warning */
	io_port = addrl;
	io_data = data;
	(*port_out[addrl]) (data);
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
 *      I/O output trap function
 *      This function should be added into all unused
 *      entries of the output port array. It can stop the
 *      emulation with an I/O error.
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

