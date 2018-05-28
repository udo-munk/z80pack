/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2018 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 */

/*
 *	Like the function "cpu_z80()" this one emulates multi byte opcodes
 *	starting with 0xcb
 */

#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

#ifdef Z80_UNDOC
	#define UNDOC(f) f
#else
	#define UNDOC(f) trap_cb
#endif

static int trap_cb(void);
static int op_srla(void), op_srlb(void), op_srlc(void);
static int op_srld(void), op_srle(void);
static int op_srlh(void), op_srll(void), op_srlhl(void);
static int op_slaa(void), op_slab(void), op_slac(void);
static int op_slad(void), op_slae(void);
static int op_slah(void), op_slal(void), op_slahl(void);
static int op_rlra(void), op_rlb(void), op_rlc(void);
static int op_rld(void), op_rle(void);
static int op_rlh(void), op_rll(void), op_rlhl(void);
static int op_rrra(void), op_rrb(void), op_rrc(void);
static int op_rrd(void), op_rre(void);
static int op_rrh(void), op_rrl(void), op_rrhl(void);
static int op_rrcra(void), op_rrcb(void), op_rrcc(void);
static int op_rrcd(void), op_rrce(void);
static int op_rrch(void), op_rrcl(void), op_rrchl(void);
static int op_rlcra(void), op_rlcb(void), op_rlcc(void);
static int op_rlcd(void), op_rlce(void);
static int op_rlch(void), op_rlcl(void), op_rlchl(void);
static int op_sraa(void), op_srab(void), op_srac(void);
static int op_srad(void), op_srae(void);
static int op_srah(void), op_sral(void), op_srahl(void);
static int op_sb0a(void), op_sb1a(void), op_sb2a(void), op_sb3a(void);
static int op_sb4a(void), op_sb5a(void), op_sb6a(void), op_sb7a(void);
static int op_sb0b(void), op_sb1b(void), op_sb2b(void), op_sb3b(void);
static int op_sb4b(void), op_sb5b(void), op_sb6b(void), op_sb7b(void);
static int op_sb0c(void), op_sb1c(void), op_sb2c(void), op_sb3c(void);
static int op_sb4c(void), op_sb5c(void), op_sb6c(void), op_sb7c(void);
static int op_sb0d(void), op_sb1d(void), op_sb2d(void), op_sb3d(void);
static int op_sb4d(void), op_sb5d(void), op_sb6d(void), op_sb7d(void);
static int op_sb0e(void), op_sb1e(void), op_sb2e(void), op_sb3e(void);
static int op_sb4e(void), op_sb5e(void), op_sb6e(void), op_sb7e(void);
static int op_sb0h(void), op_sb1h(void), op_sb2h(void), op_sb3h(void);
static int op_sb4h(void), op_sb5h(void), op_sb6h(void), op_sb7h(void);
static int op_sb0l(void), op_sb1l(void), op_sb2l(void), op_sb3l(void);
static int op_sb4l(void), op_sb5l(void), op_sb6l(void), op_sb7l(void);
static int op_sb0hl(void), op_sb1hl(void), op_sb2hl(void), op_sb3hl(void);
static int op_sb4hl(void), op_sb5hl(void), op_sb6hl(void), op_sb7hl(void);
static int op_rb0a(void), op_rb1a(void), op_rb2a(void), op_rb3a(void);
static int op_rb4a(void), op_rb5a(void), op_rb6a(void), op_rb7a(void);
static int op_rb0b(void), op_rb1b(void), op_rb2b(void), op_rb3b(void);
static int op_rb4b(void), op_rb5b(void), op_rb6b(void), op_rb7b(void);
static int op_rb0c(void), op_rb1c(void), op_rb2c(void), op_rb3c(void);
static int op_rb4c(void), op_rb5c(void), op_rb6c(void), op_rb7c(void);
static int op_rb0d(void), op_rb1d(void), op_rb2d(void), op_rb3d(void);
static int op_rb4d(void), op_rb5d(void), op_rb6d(void), op_rb7d(void);
static int op_rb0e(void), op_rb1e(void), op_rb2e(void), op_rb3e(void);
static int op_rb4e(void), op_rb5e(void), op_rb6e(void), op_rb7e(void);
static int op_rb0h(void), op_rb1h(void), op_rb2h(void), op_rb3h(void);
static int op_rb4h(void), op_rb5h(void), op_rb6h(void), op_rb7h(void);
static int op_rb0l(void), op_rb1l(void), op_rb2l(void), op_rb3l(void);
static int op_rb4l(void), op_rb5l(void), op_rb6l(void), op_rb7l(void);
static int op_rb0hl(void), op_rb1hl(void), op_rb2hl(void), op_rb3hl(void);
static int op_rb4hl(void), op_rb5hl(void), op_rb6hl(void), op_rb7hl(void);
static int op_tb0a(void), op_tb1a(void), op_tb2a(void), op_tb3a(void);
static int op_tb4a(void), op_tb5a(void), op_tb6a(void), op_tb7a(void);
static int op_tb0b(void), op_tb1b(void), op_tb2b(void), op_tb3b(void);
static int op_tb4b(void), op_tb5b(void), op_tb6b(void), op_tb7b(void);
static int op_tb0c(void), op_tb1c(void), op_tb2c(void), op_tb3c(void);
static int op_tb4c(void), op_tb5c(void), op_tb6c(void), op_tb7c(void);
static int op_tb0d(void), op_tb1d(void), op_tb2d(void), op_tb3d(void);
static int op_tb4d(void), op_tb5d(void), op_tb6d(void), op_tb7d(void);
static int op_tb0e(void), op_tb1e(void), op_tb2e(void), op_tb3e(void);
static int op_tb4e(void), op_tb5e(void), op_tb6e(void), op_tb7e(void);
static int op_tb0h(void), op_tb1h(void), op_tb2h(void), op_tb3h(void);
static int op_tb4h(void), op_tb5h(void), op_tb6h(void), op_tb7h(void);
static int op_tb0l(void), op_tb1l(void), op_tb2l(void), op_tb3l(void);
static int op_tb4l(void), op_tb5l(void), op_tb6l(void), op_tb7l(void);
static int op_tb0hl(void), op_tb1hl(void), op_tb2hl(void), op_tb3hl(void);
static int op_tb4hl(void), op_tb5hl(void), op_tb6hl(void), op_tb7hl(void);

#ifdef Z80_UNDOC
static int op_undoc_slla(void), op_undoc_sllb(void), op_undoc_sllc(void);
static int op_undoc_slld(void), op_undoc_slle(void);
static int op_undoc_sllh(void), op_undoc_slll(void), op_undoc_sllhl(void);
#endif

int op_cb_handel(void)
{
	register int t;

	static int (*op_cb[256]) (void) = {
		op_rlcb,			/* 0x00 */
		op_rlcc,			/* 0x01 */
		op_rlcd,			/* 0x02 */
		op_rlce,			/* 0x03 */
		op_rlch,			/* 0x04 */
		op_rlcl,			/* 0x05 */
		op_rlchl,			/* 0x06 */
		op_rlcra,			/* 0x07 */
		op_rrcb,			/* 0x08 */
		op_rrcc,			/* 0x09 */
		op_rrcd,			/* 0x0a */
		op_rrce,			/* 0x0b */
		op_rrch,			/* 0x0c */
		op_rrcl,			/* 0x0d */
		op_rrchl,			/* 0x0e */
		op_rrcra,			/* 0x0f */
		op_rlb,				/* 0x10 */
		op_rlc,				/* 0x11 */
		op_rld,				/* 0x12 */
		op_rle,				/* 0x13 */
		op_rlh,				/* 0x14 */
		op_rll,				/* 0x15 */
		op_rlhl,			/* 0x16 */
		op_rlra,			/* 0x17 */
		op_rrb,				/* 0x18 */
		op_rrc,				/* 0x19 */
		op_rrd,				/* 0x1a */
		op_rre,				/* 0x1b */
		op_rrh,				/* 0x1c */
		op_rrl,				/* 0x1d */
		op_rrhl,			/* 0x1e */
		op_rrra,			/* 0x1f */
		op_slab,			/* 0x20 */
		op_slac,			/* 0x21 */
		op_slad,			/* 0x22 */
		op_slae,			/* 0x23 */
		op_slah,			/* 0x24 */
		op_slal,			/* 0x25 */
		op_slahl,			/* 0x26 */
		op_slaa,			/* 0x27 */
		op_srab,			/* 0x28 */
		op_srac,			/* 0x29 */
		op_srad,			/* 0x2a */
		op_srae,			/* 0x2b */
		op_srah,			/* 0x2c */
		op_sral,			/* 0x2d */
		op_srahl,			/* 0x2e */
		op_sraa,			/* 0x2f */
		UNDOC(op_undoc_sllb),		/* 0x30 */
		UNDOC(op_undoc_sllc),		/* 0x31 */
		UNDOC(op_undoc_slld),		/* 0x32 */
		UNDOC(op_undoc_slle),		/* 0x33 */
		UNDOC(op_undoc_sllh),		/* 0x34 */
		UNDOC(op_undoc_slll),		/* 0x35 */
		UNDOC(op_undoc_sllhl),		/* 0x36 */
		UNDOC(op_undoc_slla),		/* 0x37 */
		op_srlb,			/* 0x38 */
		op_srlc,			/* 0x39 */
		op_srld,			/* 0x3a */
		op_srle,			/* 0x3b */
		op_srlh,			/* 0x3c */
		op_srll,			/* 0x3d */
		op_srlhl,			/* 0x3e */
		op_srla,			/* 0x3f */
		op_tb0b,			/* 0x40 */
		op_tb0c,			/* 0x41 */
		op_tb0d,			/* 0x42 */
		op_tb0e,			/* 0x43 */
		op_tb0h,			/* 0x44 */
		op_tb0l,			/* 0x45 */
		op_tb0hl,			/* 0x46 */
		op_tb0a,			/* 0x47 */
		op_tb1b,			/* 0x48 */
		op_tb1c,			/* 0x49 */
		op_tb1d,			/* 0x4a */
		op_tb1e,			/* 0x4b */
		op_tb1h,			/* 0x4c */
		op_tb1l,			/* 0x4d */
		op_tb1hl,			/* 0x4e */
		op_tb1a,			/* 0x4f */
		op_tb2b,			/* 0x50 */
		op_tb2c,			/* 0x51 */
		op_tb2d,			/* 0x52 */
		op_tb2e,			/* 0x53 */
		op_tb2h,			/* 0x54 */
		op_tb2l,			/* 0x55 */
		op_tb2hl,			/* 0x56 */
		op_tb2a,			/* 0x57 */
		op_tb3b,			/* 0x58 */
		op_tb3c,			/* 0x59 */
		op_tb3d,			/* 0x5a */
		op_tb3e,			/* 0x5b */
		op_tb3h,			/* 0x5c */
		op_tb3l,			/* 0x5d */
		op_tb3hl,			/* 0x5e */
		op_tb3a,			/* 0x5f */
		op_tb4b,			/* 0x60 */
		op_tb4c,			/* 0x61 */
		op_tb4d,			/* 0x62 */
		op_tb4e,			/* 0x63 */
		op_tb4h,			/* 0x64 */
		op_tb4l,			/* 0x65 */
		op_tb4hl,			/* 0x66 */
		op_tb4a,			/* 0x67 */
		op_tb5b,			/* 0x68 */
		op_tb5c,			/* 0x69 */
		op_tb5d,			/* 0x6a */
		op_tb5e,			/* 0x6b */
		op_tb5h,			/* 0x6c */
		op_tb5l,			/* 0x6d */
		op_tb5hl,			/* 0x6e */
		op_tb5a,			/* 0x6f */
		op_tb6b,			/* 0x70 */
		op_tb6c,			/* 0x71 */
		op_tb6d,			/* 0x72 */
		op_tb6e,			/* 0x73 */
		op_tb6h,			/* 0x74 */
		op_tb6l,			/* 0x75 */
		op_tb6hl,			/* 0x76 */
		op_tb6a,			/* 0x77 */
		op_tb7b,			/* 0x78 */
		op_tb7c,			/* 0x79 */
		op_tb7d,			/* 0x7a */
		op_tb7e,			/* 0x7b */
		op_tb7h,			/* 0x7c */
		op_tb7l,			/* 0x7d */
		op_tb7hl,			/* 0x7e */
		op_tb7a,			/* 0x7f */
		op_rb0b,			/* 0x80 */
		op_rb0c,			/* 0x81 */
		op_rb0d,			/* 0x82 */
		op_rb0e,			/* 0x83 */
		op_rb0h,			/* 0x84 */
		op_rb0l,			/* 0x85 */
		op_rb0hl,			/* 0x86 */
		op_rb0a,			/* 0x87 */
		op_rb1b,			/* 0x88 */
		op_rb1c,			/* 0x89 */
		op_rb1d,			/* 0x8a */
		op_rb1e,			/* 0x8b */
		op_rb1h,			/* 0x8c */
		op_rb1l,			/* 0x8d */
		op_rb1hl,			/* 0x8e */
		op_rb1a,			/* 0x8f */
		op_rb2b,			/* 0x90 */
		op_rb2c,			/* 0x91 */
		op_rb2d,			/* 0x92 */
		op_rb2e,			/* 0x93 */
		op_rb2h,			/* 0x94 */
		op_rb2l,			/* 0x95 */
		op_rb2hl,			/* 0x96 */
		op_rb2a,			/* 0x97 */
		op_rb3b,			/* 0x98 */
		op_rb3c,			/* 0x99 */
		op_rb3d,			/* 0x9a */
		op_rb3e,			/* 0x9b */
		op_rb3h,			/* 0x9c */
		op_rb3l,			/* 0x9d */
		op_rb3hl,			/* 0x9e */
		op_rb3a,			/* 0x9f */
		op_rb4b,			/* 0xa0 */
		op_rb4c,			/* 0xa1 */
		op_rb4d,			/* 0xa2 */
		op_rb4e,			/* 0xa3 */
		op_rb4h,			/* 0xa4 */
		op_rb4l,			/* 0xa5 */
		op_rb4hl,			/* 0xa6 */
		op_rb4a,			/* 0xa7 */
		op_rb5b,			/* 0xa8 */
		op_rb5c,			/* 0xa9 */
		op_rb5d,			/* 0xaa */
		op_rb5e,			/* 0xab */
		op_rb5h,			/* 0xac */
		op_rb5l,			/* 0xad */
		op_rb5hl,			/* 0xae */
		op_rb5a,			/* 0xaf */
		op_rb6b,			/* 0xb0 */
		op_rb6c,			/* 0xb1 */
		op_rb6d,			/* 0xb2 */
		op_rb6e,			/* 0xb3 */
		op_rb6h,			/* 0xb4 */
		op_rb6l,			/* 0xb5 */
		op_rb6hl,			/* 0xb6 */
		op_rb6a,			/* 0xb7 */
		op_rb7b,			/* 0xb8 */
		op_rb7c,			/* 0xb9 */
		op_rb7d,			/* 0xba */
		op_rb7e,			/* 0xbb */
		op_rb7h,			/* 0xbc */
		op_rb7l,			/* 0xbd */
		op_rb7hl,			/* 0xbe */
		op_rb7a,			/* 0xbf */
		op_sb0b,			/* 0xc0 */
		op_sb0c,			/* 0xc1 */
		op_sb0d,			/* 0xc2 */
		op_sb0e,			/* 0xc3 */
		op_sb0h,			/* 0xc4 */
		op_sb0l,			/* 0xc5 */
		op_sb0hl,			/* 0xc6 */
		op_sb0a,			/* 0xc7 */
		op_sb1b,			/* 0xc8 */
		op_sb1c,			/* 0xc9 */
		op_sb1d,			/* 0xca */
		op_sb1e,			/* 0xcb */
		op_sb1h,			/* 0xcc */
		op_sb1l,			/* 0xcd */
		op_sb1hl,			/* 0xce */
		op_sb1a,			/* 0xcf */
		op_sb2b,			/* 0xd0 */
		op_sb2c,			/* 0xd1 */
		op_sb2d,			/* 0xd2 */
		op_sb2e,			/* 0xd3 */
		op_sb2h,			/* 0xd4 */
		op_sb2l,			/* 0xd5 */
		op_sb2hl,			/* 0xd6 */
		op_sb2a,			/* 0xd7 */
		op_sb3b,			/* 0xd8 */
		op_sb3c,			/* 0xd9 */
		op_sb3d,			/* 0xda */
		op_sb3e,			/* 0xdb */
		op_sb3h,			/* 0xdc */
		op_sb3l,			/* 0xdd */
		op_sb3hl,			/* 0xde */
		op_sb3a,			/* 0xdf */
		op_sb4b,			/* 0xe0 */
		op_sb4c,			/* 0xe1 */
		op_sb4d,			/* 0xe2 */
		op_sb4e,			/* 0xe3 */
		op_sb4h,			/* 0xe4 */
		op_sb4l,			/* 0xe5 */
		op_sb4hl,			/* 0xe6 */
		op_sb4a,			/* 0xe7 */
		op_sb5b,			/* 0xe8 */
		op_sb5c,			/* 0xe9 */
		op_sb5d,			/* 0xea */
		op_sb5e,			/* 0xeb */
		op_sb5h,			/* 0xec */
		op_sb5l,			/* 0xed */
		op_sb5hl,			/* 0xee */
		op_sb5a,			/* 0xef */
		op_sb6b,			/* 0xf0 */
		op_sb6c,			/* 0xf1 */
		op_sb6d,			/* 0xf2 */
		op_sb6e,			/* 0xf3 */
		op_sb6h,			/* 0xf4 */
		op_sb6l,			/* 0xf5 */
		op_sb6hl,			/* 0xf6 */
		op_sb6a,			/* 0xf7 */
		op_sb7b,			/* 0xf8 */
		op_sb7c,			/* 0xf9 */
		op_sb7d,			/* 0xfa */
		op_sb7e,			/* 0xfb */
		op_sb7h,			/* 0xfc */
		op_sb7l,			/* 0xfd */
		op_sb7hl,			/* 0xfe */
		op_sb7a				/* 0xff */
	};

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	/* update frontpanel */
	fp_clock++;
	fp_sampleLightGroup(0, 0);
#endif

	t = (*op_cb[memrdr(PC++)]) ();		/* execute next opcode */

	return(t);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xcb of a multi byte opcode.
 */
static int trap_cb(void)
{
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return(0);
}

static int op_srla(void)		/* SRL A */
{
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srlb(void)		/* SRL B */
{
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srlc(void)		/* SRL C */
{
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srld(void)		/* SRL D */
{
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srle(void)		/* SRL E */
{
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) :(F |= P_FLAG);
	return(8);
}

static int op_srlh(void)		/* SRL H */
{
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srll(void)		/* SRL L */
{
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srlhl(void)		/* SRL (HL) */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ?	(F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_slaa(void)		/* SLA A */
{
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slab(void)		/* SLA B */
{
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slac(void)		/* SLA C */
{
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slad(void)		/* SLA D */
{
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slae(void)		/* SLA E */
{
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slah(void)		/* SLA H */
{
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slal(void)		/* SLA L */
{
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_slahl(void)		/* SLA (HL) */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_rlra(void)		/* RL A */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlb(void)			/* RL B */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	if (old_c_flag) B |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlc(void)			/* RL C */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	if (old_c_flag) C |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rld(void)			/* RL D */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	if (old_c_flag) D |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rle(void)			/* RL E */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	if (old_c_flag) E |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlh(void)			/* RL H */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	if (old_c_flag) H |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) :(F |= P_FLAG);
	return(8);
}

static int op_rll(void)			/* RL L */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	if (old_c_flag) L |= 1;
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlhl(void)		/* RL (HL) */
{
	register BYTE P;
	WORD addr;
	int old_c_flag;

	addr = (H << 8) + L;
	P = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	if (old_c_flag) P |= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_rrra(void)		/* RR A */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrb(void)			/* RR B */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	if (old_c_flag) B |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrc(void)			/* RR C */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	if (old_c_flag) C |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrd(void)			/* RR D */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	if (old_c_flag) D |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rre(void)			/* RR E */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	if (old_c_flag) E |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrh(void)			/* RR H */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	if (old_c_flag) H |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrl(void)			/* RR L */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	if (old_c_flag) L |= 128;
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrhl(void)		/* RR (HL) */
{
	register BYTE P;
	WORD addr;
	int old_c_flag;

	addr = (H << 8) + L;
	P = memrdr(addr);
	old_c_flag = F & C_FLAG;
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	if (old_c_flag) P |= 128;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_rrcra(void)		/* RRC A */
{
	register int i;

	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A >>= 1;
	if (i) A |= 128;
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrcb(void)		/* RRC B */
{
	register int i;

	i = B & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	B >>= 1;
	if (i) B |= 128;
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrcc(void)		/* RRC C */
{
	register int i;

	i = C & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	C >>= 1;
	if (i) C |= 128;
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrcd(void)		/* RRC D */
{
	register int i;

	i = D & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	D >>= 1;
	if (i) D |= 128;
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrce(void)		/* RRC E */
{
	register int i;

	i = E & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	E >>= 1;
	if (i) E |= 128;
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrch(void)		/* RRC H */
{
	register int i;

	i = H & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	H >>= 1;
	if (i) H |= 128;
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrcl(void)		/* RRC L */
{
	register int i;

	i = L & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	L >>= 1;
	if (i) L |= 128;
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rrchl(void)		/* RRC (HL) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = (H << 8) + L;
	P = memrdr(addr);
	i = P & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	if (i) P |= 128;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_rlcra(void)		/* RLC A */
{
	register int i;

	i = A & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A <<= 1;
	if (i) A |= 1;
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlcb(void)		/* RLC B */
{
	register int i;

	i = B & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	B <<= 1;
	if (i) B |= 1;
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlcc(void)		/* RLC C */
{
	register int i;

	i = C & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	C <<= 1;
	if (i) C |= 1;
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlcd(void)		/* RLC D */
{
	register int i;

	i = D & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	D <<= 1;
	if (i) D |= 1;
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlce(void)		/* RLC E */
{
	register int i;

	i = E & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	E <<= 1;
	if (i) E |= 1;
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlch(void)		/* RLC H */
{
	register int i;

	i = H & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	H <<= 1;
	if (i) H |= 1;
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlcl(void)		/* RLC L */
{
	register int i;

	i = L & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	L <<= 1;
	if (i) L |= 1;
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_rlchl(void)		/* RLC (HL) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = (H << 8) + L;
	P = memrdr(addr);
	i = P & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	if (i) P |= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_sraa(void)		/* SRA A */
{
	register int i;

	i = A & 128;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	A |= i;
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srab(void)		/* SRA B */
{
	register int i;

	i = B & 128;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	B |= i;
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srac(void)		/* SRA C */
{
	register int i;

	i = C & 128;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	C |= i;
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srad(void)		/* SRA D */
{
	register int i;

	i = D & 128;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	D |= i;
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srae(void)		/* SRA E */
{
	register int i;

	i = E & 128;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	E |= i;
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srah(void)		/* SRA H */
{
	register int i;

	i = H & 128;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	H |= i;
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_sral(void)		/* SRA L */
{
	register int i;

	i = L & 128;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	L |= i;
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_srahl(void)		/* SRA (HL) */
{
	register BYTE P;
	WORD addr;
	int i;

	addr = (H << 8) + L;
	P = memrdr(addr);
	i = P & 128;
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P >> 1) | i;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

static int op_sb0a(void)		/* SET 0,A */
{
	A |= 1;
	return(8);
}

static int op_sb1a(void)		/* SET 1,A */
{
	A |= 2;
	return(8);
}

static int op_sb2a(void)		/* SET 2,A */
{
	A |= 4;
	return(8);
}

static int op_sb3a(void)		/* SET 3,A */
{
	A |= 8;
	return(8);
}

static int op_sb4a(void)		/* SET 4,A */
{
	A |= 16;
	return(8);
}

static int op_sb5a(void)		/* SET 5,A */
{
	A |= 32;
	return(8);
}

static int op_sb6a(void)		/* SET 6,A */
{
	A |= 64;
	return(8);
}

static int op_sb7a(void)		/* SET 7,A */
{
	A |= 128;
	return(8);
}

static int op_sb0b(void)		/* SET 0,B */
{
	B |= 1;
	return(8);
}

static int op_sb1b(void)		/* SET 1,B */
{
	B |= 2;
	return(8);
}

static int op_sb2b(void)		/* SET 2,B */
{
	B |= 4;
	return(8);
}

static int op_sb3b(void)		/* SET 3,B */
{
	B |= 8;
	return(8);
}

static int op_sb4b(void)		/* SET 4,B */
{
	B |= 16;
	return(8);
}

static int op_sb5b(void)		/* SET 5,B */
{
	B |= 32;
	return(8);
}

static int op_sb6b(void)		/* SET 6,B */
{
	B |= 64;
	return(8);
}

static int op_sb7b(void)		/* SET 7,B */
{
	B |= 128;
	return(8);
}

static int op_sb0c(void)		/* SET 0,C */
{
	C |= 1;
	return(8);
}

static int op_sb1c(void)		/* SET 1,C */
{
	C |= 2;
	return(8);
}

static int op_sb2c(void)		/* SET 2,C */
{
	C |= 4;
	return(8);
}

static int op_sb3c(void)		/* SET 3,C */
{
	C |= 8;
	return(8);
}

static int op_sb4c(void)		/* SET 4,C */
{
	C |= 16;
	return(8);
}

static int op_sb5c(void)		/* SET 5,C */
{
	C |= 32;
	return(8);
}

static int op_sb6c(void)		/* SET 6,C */
{
	C |= 64;
	return(8);
}

static int op_sb7c(void)		/* SET 7,C */
{
	C |= 128;
	return(8);
}

static int op_sb0d(void)		/* SET 0,D */
{
	D |= 1;
	return(8);
}

static int op_sb1d(void)		/* SET 1,D */
{
	D |= 2;
	return(8);
}

static int op_sb2d(void)		/* SET 2,D */
{
	D |= 4;
	return(8);
}

static int op_sb3d(void)		/* SET 3,D */
{
	D |= 8;
	return(8);
}

static int op_sb4d(void)		/* SET 4,D */
{
	D |= 16;
	return(8);
}

static int op_sb5d(void)		/* SET 5,D */
{
	D |= 32;
	return(8);
}

static int op_sb6d(void)		/* SET 6,D */
{
	D |= 64;
	return(8);
}

static int op_sb7d(void)		/* SET 7,D */
{
	D |= 128;
	return(8);
}

static int op_sb0e(void)		/* SET 0,E */
{
	E |= 1;
	return(8);
}

static int op_sb1e(void)		/* SET 1,E */
{
	E |= 2;
	return(8);
}

static int op_sb2e(void)		/* SET 2,E */
{
	E |= 4;
	return(8);
}

static int op_sb3e(void)		/* SET 3,E */
{
	E |= 8;
	return(8);
}

static int op_sb4e(void)		/* SET 4,E */
{
	E |= 16;
	return(8);
}

static int op_sb5e(void)		/* SET 5,E */
{
	E |= 32;
	return(8);
}

static int op_sb6e(void)		/* SET 6,E */
{
	E |= 64;
	return(8);
}

static int op_sb7e(void)		/* SET 7,E */
{
	E |= 128;
	return(8);
}

static int op_sb0h(void)		/* SET 0,H */
{
	H |= 1;
	return(8);
}

static int op_sb1h(void)		/* SET 1,H */
{
	H |= 2;
	return(8);
}

static int op_sb2h(void)		/* SET 2,H */
{
	H |= 4;
	return(8);
}

static int op_sb3h(void)		/* SET 3,H */
{
	H |= 8;
	return(8);
}

static int op_sb4h(void)		/* SET 4,H */
{
	H |= 16;
	return(8);
}

static int op_sb5h(void)		/* SET 5,H */
{
	H |= 32;
	return(8);
}

static int op_sb6h(void)		/* SET 6,H */
{
	H |= 64;
	return(8);
}

static int op_sb7h(void)		/* SET 7,H */
{
	H |= 128;
	return(8);
}

static int op_sb0l(void)		/* SET 0,L */
{
	L |= 1;
	return(8);
}

static int op_sb1l(void)		/* SET 1,L */
{
	L |= 2;
	return(8);
}

static int op_sb2l(void)		/* SET 2,L */
{
	L |= 4;
	return(8);
}

static int op_sb3l(void)		/* SET 3,L */
{
	L |= 8;
	return(8);
}

static int op_sb4l(void)		/* SET 4,L */
{
	L |= 16;
	return(8);
}

static int op_sb5l(void)		/* SET 5,L */
{
	L |= 32;
	return(8);
}

static int op_sb6l(void)		/* SET 6,L */
{
	L |= 64;
	return(8);
}

static int op_sb7l(void)		/* SET 7,L */
{
	L |= 128;
	return(8);
}

static int op_sb0hl(void)		/* SET 0,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 1);
	return(15);
}

static int op_sb1hl(void)		/* SET 1,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 2);
	return(15);
}

static int op_sb2hl(void)		/* SET 2,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 4);
	return(15);
}

static int op_sb3hl(void)		/* SET 3,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 8);
	return(15);
}

static int op_sb4hl(void)		/* SET 4,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 16);
	return(15);
}

static int op_sb5hl(void)		/* SET 5,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 32);
	return(15);
}

static int op_sb6hl(void)		/* SET 6,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 64);
	return(15);
}

static int op_sb7hl(void)		/* SET 7,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 128);
	return(15);
}

static int op_rb0a(void)		/* RES 0,A */
{
	A &= ~1;
	return(8);
}

static int op_rb1a(void)		/* RES 1,A */
{
	A &= ~2;
	return(8);
}

static int op_rb2a(void)		/* RES 2,A */
{
	A &= ~4;
	return(8);
}

static int op_rb3a(void)		/* RES 3,A */
{
	A &= ~8;
	return(8);
}

static int op_rb4a(void)		/* RES 4,A */
{
	A &= ~16;
	return(8);
}

static int op_rb5a(void)		/* RES 5,A */
{
	A &= ~32;
	return(8);
}

static int op_rb6a(void)		/* RES 6,A */
{
	A &= ~64;
	return(8);
}

static int op_rb7a(void)		/* RES 7,A */
{
	A &= ~128;
	return(8);
}

static int op_rb0b(void)		/* RES 0,B */
{
	B &= ~1;
	return(8);
}

static int op_rb1b(void)		/* RES 1,B */
{
	B &= ~2;
	return(8);
}

static int op_rb2b(void)		/* RES 2,B */
{
	B &= ~4;
	return(8);
}

static int op_rb3b(void)		/* RES 3,B */
{
	B &= ~8;
	return(8);
}

static int op_rb4b(void)		/* RES 4,B */
{
	B &= ~16;
	return(8);
}

static int op_rb5b(void)		/* RES 5,B */
{
	B &= ~32;
	return(8);
}

static int op_rb6b(void)		/* RES 6,B */
{
	B &= ~64;
	return(8);
}

static int op_rb7b(void)		/* RES 7,B */
{
	B &= ~128;
	return(8);
}

static int op_rb0c(void)		/* RES 0,C */
{
	C &= ~1;
	return(8);
}

static int op_rb1c(void)		/* RES 1,C */
{
	C &= ~2;
	return(8);
}

static int op_rb2c(void)		/* RES 2,C */
{
	C &= ~4;
	return(8);
}

static int op_rb3c(void)		/* RES 3,C */
{
	C &= ~8;
	return(8);
}

static int op_rb4c(void)		/* RES 4,C */
{
	C &= ~16;
	return(8);
}

static int op_rb5c(void)		/* RES 5,C */
{
	C &= ~32;
	return(8);
}

static int op_rb6c(void)		/* RES 6,C */
{
	C &= ~64;
	return(8);
}

static int op_rb7c(void)		/* RES 7,C */
{
	C &= ~128;
	return(8);
}

static int op_rb0d(void)		/* RES 0,D */
{
	D &= ~1;
	return(8);
}

static int op_rb1d(void)		/* RES 1,D */
{
	D &= ~2;
	return(8);
}

static int op_rb2d(void)		/* RES 2,D */
{
	D &= ~4;
	return(8);
}

static int op_rb3d(void)		/* RES 3,D */
{
	D &= ~8;
	return(8);
}

static int op_rb4d(void)		/* RES 4,D */
{
	D &= ~16;
	return(8);
}

static int op_rb5d(void)		/* RES 5,D */
{
	D &= ~32;
	return(8);
}

static int op_rb6d(void)		/* RES 6,D */
{
	D &= ~64;
	return(8);
}

static int op_rb7d(void)		/* RES 7,D */
{
	D &= ~128;
	return(8);
}

static int op_rb0e(void)		/* RES 0,E */
{
	E &= ~1;
	return(8);
}

static int op_rb1e(void)		/* RES 1,E */
{
	E &= ~2;
	return(8);
}

static int op_rb2e(void)		/* RES 2,E */
{
	E &= ~4;
	return(8);
}

static int op_rb3e(void)		/* RES 3,E */
{
	E &= ~8;
	return(8);
}

static int op_rb4e(void)		/* RES 4,E */
{
	E &= ~16;
	return(8);
}

static int op_rb5e(void)		/* RES 5,E */
{
	E &= ~32;
	return(8);
}

static int op_rb6e(void)		/* RES 6,E */
{
	E &= ~64;
	return(8);
}

static int op_rb7e(void)		/* RES 7,E */
{
	E &= ~128;
	return(8);
}

static int op_rb0h(void)		/* RES 0,H */
{
	H &= ~1;
	return(8);
}

static int op_rb1h(void)		/* RES 1,H */
{
	H &= ~2;
	return(8);
}

static int op_rb2h(void)		/* RES 2,H */
{
	H &= ~4;
	return(8);
}

static int op_rb3h(void)		/* RES 3,H */
{
	H &= ~8;
	return(8);
}

static int op_rb4h(void)		/* RES 4,H */
{
	H &= ~16;
	return(8);
}

static int op_rb5h(void)		/* RES 5,H */
{
	H &= ~32;
	return(8);
}

static int op_rb6h(void)		/* RES 6,H */
{
	H &= ~64;
	return(8);
}

static int op_rb7h(void)		/* RES 7,H */
{
	H &= ~128;
	return(8);
}

static int op_rb0l(void)		/* RES 0,L */
{
	L &= ~1;
	return(8);
}

static int op_rb1l(void)		/* RES 1,L */
{
	L &= ~2;
	return(8);
}

static int op_rb2l(void)		/* RES 2,L */
{
	L &= ~4;
	return(8);
}

static int op_rb3l(void)		/* RES 3,L */
{
	L &= ~8;
	return(8);
}

static int op_rb4l(void)		/* RES 4,L */
{
	L &= ~16;
	return(8);
}

static int op_rb5l(void)		/* RES 5,L */
{
	L &= ~32;
	return(8);
}

static int op_rb6l(void)		/* RES 6,L */
{
	L &= ~64;
	return(8);
}

static int op_rb7l(void)		/* RES 7,L */
{
	L &= ~128;
	return(8);
}

static int op_rb0hl(void)		/* RES 0,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~1);
	return(15);
}

static int op_rb1hl(void)		/* RES 1,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~2);
	return(15);
}

static int op_rb2hl(void)		/* RES 2,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~4);
	return(15);
}

static int op_rb3hl(void)		/* RES 3,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~8);
	return(15);
}

static int op_rb4hl(void)		/* RES 4,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~16);
	return(15);
}

static int op_rb5hl(void)		/* RES 5,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~32);
	return(15);
}

static int op_rb6hl(void)		/* RES 6,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~64);
	return(15);
}

static int op_rb7hl(void)		/* RES 7,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~128);
	return(15);
}

static int op_tb0a(void)		/* BIT 0,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1a(void)		/* BIT 1,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2a(void)		/* BIT 2,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3a(void)		/* BIT 3,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4a(void)		/* BIT 4,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5a(void)		/* BIT 5,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6a(void)		/* BIT 6,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7a(void)		/* BIT 7,A */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (A & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0b(void)		/* BIT 0,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1b(void)		/* BIT 1,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2b(void)		/* BIT 2,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3b(void)		/* BIT 3,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4b(void)		/* BIT 4,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5b(void)		/* BIT 5,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6b(void)		/* BIT 6,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7b(void)		/* BIT 7,B */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (B & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0c(void)		/* BIT 0,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1c(void)		/* BIT 1,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2c(void)		/* BIT 2,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3c(void)		/* BIT 3,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4c(void)		/* BIT 4,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5c(void)		/* BIT 5,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6c(void)		/* BIT 6,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7c(void)		/* BIT 7,C */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (C & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0d(void)		/* BIT 0,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1d(void)		/* BIT 1,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2d(void)		/* BIT 2,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3d(void)		/* BIT 3,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4d(void)		/* BIT 4,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5d(void)		/* BIT 5,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6d(void)		/* BIT 6,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7d(void)		/* BIT 7,D */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (D & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0e(void)		/* BIT 0,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1e(void)		/* BIT 1,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2e(void)		/* BIT 2,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3e(void)		/* BIT 3,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4e(void)		/* BIT 4,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5e(void)		/* BIT 5,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6e(void)		/* BIT 6,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7e(void)		/* BIT 7,E */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (E & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0h(void)		/* BIT 0,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1h(void)		/* BIT 1,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2h(void)		/* BIT 2,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3h(void)		/* BIT 3,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4h(void)		/* BIT 4,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5h(void)		/* BIT 5,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6h(void)		/* BIT 6,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7h(void)		/* BIT 7,H */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (H & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0l(void)		/* BIT 0,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb1l(void)		/* BIT 1,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb2l(void)		/* BIT 2,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb3l(void)		/* BIT 3,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb4l(void)		/* BIT 4,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb5l(void)		/* BIT 5,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb6l(void)		/* BIT 6,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
	return(8);
}

static int op_tb7l(void)		/* BIT 7,L */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (L & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(8);
}

static int op_tb0hl(void)		/* BIT 0,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb1hl(void)		/* BIT 1,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb2hl(void)		/* BIT 2,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb3hl(void)		/* BIT 3,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb4hl(void)		/* BIT 4,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
				    : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb5hl(void)		/* BIT 5,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
				    : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb6hl(void)		/* BIT 6,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
				    : (F |= (Z_FLAG | P_FLAG));
	return(12);
}

static int op_tb7hl(void)		/* BIT 7,(HL) */
{
	F &= ~N_FLAG;
	F |= H_FLAG;
	if (memrdr((H << 8) + L) & 128) {
		F &= ~(Z_FLAG | P_FLAG);
		F |= S_FLAG;
	} else {
		F |= (Z_FLAG | P_FLAG);
		F &= ~S_FLAG;
	}
	return(12);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef Z80_UNDOC

static int op_undoc_slla(void)		/* SLL A */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_sllb(void)		/* SLL B */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B = B << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_sllc(void)		/* SLL C */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C = C << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_slld(void)		/* SLL D */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D = D << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_slle(void)		/* SLL E */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E = E << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_sllh(void)		/* SLL H */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H = H << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_slll(void)		/* SLL L */
{
	if (u_flag) {
		trap_cb();
		return(0);
	}

	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L = L << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(8);
}

static int op_undoc_sllhl(void)		/* SLL (HL) */
{
	register BYTE P;
	WORD addr;

	if (u_flag) {
		trap_cb();
		return(0);
	}

	addr = (H << 8) + L;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P << 1) | 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(15);
}

#endif
