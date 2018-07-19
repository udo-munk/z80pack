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
 *	starting with 0xdd
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
	#define UNDOC(f) trap_dd
#endif

static int trap_dd(void);
static int op_popix(void), op_pusix(void);
static int op_jpix(void);
static int op_exspx(void);
static int op_ldspx(void);
static int op_ldixnn(void), op_ldixinn(void), op_ldinx(void);
static int op_adaxd(void), op_acaxd(void), op_suaxd(void), op_scaxd(void);
static int op_andxd(void), op_xorxd(void), op_orxd(void), op_cpxd(void);
static int op_decxd(void), op_incxd(void);
static int op_addxb(void), op_addxd(void), op_addxs(void), op_addxx(void);
static int op_incix(void), op_decix(void);
static int op_ldaxd(void), op_ldbxd(void), op_ldcxd(void);
static int op_lddxd(void), op_ldexd(void);
static int op_ldhxd(void), op_ldlxd(void);
static int op_ldxda(void), op_ldxdb(void), op_ldxdc(void);
static int op_ldxdd(void), op_ldxde(void);
static int op_ldxdh(void), op_ldxdl(void), op_ldxdn(void);
extern int op_ddcb_handel(void);

#ifdef Z80_UNDOC
static int op_undoc_ldaixl(void), op_undoc_ldaixh(void);
static int op_undoc_ldbixl(void), op_undoc_ldbixh(void);
static int op_undoc_ldcixl(void), op_undoc_ldcixh(void);
static int op_undoc_lddixl(void), op_undoc_lddixh(void);
static int op_undoc_ldeixl(void), op_undoc_ldeixh(void);
static int op_undoc_ldixha(void), op_undoc_ldixla(void);
static int op_undoc_ldixhb(void), op_undoc_ldixlb(void);
static int op_undoc_ldixhc(void), op_undoc_ldixlc(void);
static int op_undoc_ldixhd(void), op_undoc_ldixld(void);
static int op_undoc_ldixhe(void), op_undoc_ldixle(void);
static int op_undoc_ldixhixh(void), op_undoc_ldixlixh(void);
static int op_undoc_ldixhixl(void), op_undoc_ldixlixl(void);
static int op_undoc_ldixhn(void), op_undoc_ldixln(void);
static int op_undoc_cpixl(void);
static int op_undoc_acaixl(void), op_undoc_acaixh(void);
static int op_undoc_scaixl(void), op_undoc_scaixh(void);
static int op_undoc_oraixl(void), op_undoc_oraixh(void);
static int op_undoc_andixl(void), op_undoc_andixh(void);
static int op_undoc_incixl(void), op_undoc_incixh(void);
static int op_undoc_decixl(void), op_undoc_decixh(void);
#endif

long op_dd_handel(void)
{
	register int t;

	static int (*op_dd[256]) (void) = {
		trap_dd,			/* 0x00 */
		trap_dd,			/* 0x01 */
		trap_dd,			/* 0x02 */
		trap_dd,			/* 0x03 */
		trap_dd,			/* 0x04 */
		trap_dd,			/* 0x05 */
		trap_dd,			/* 0x06 */
		trap_dd,			/* 0x07 */
		trap_dd,			/* 0x08 */
		op_addxb,			/* 0x09 */
		trap_dd,			/* 0x0a */
		trap_dd,			/* 0x0b */
		trap_dd,			/* 0x0c */
		trap_dd,			/* 0x0d */
		trap_dd,			/* 0x0e */
		trap_dd,			/* 0x0f */
		trap_dd,			/* 0x10 */
		trap_dd,			/* 0x11 */
		trap_dd,			/* 0x12 */
		trap_dd,			/* 0x13 */
		trap_dd,			/* 0x14 */
		trap_dd,			/* 0x15 */
		trap_dd,			/* 0x16 */
		trap_dd,			/* 0x17 */
		trap_dd,			/* 0x18 */
		op_addxd,			/* 0x19 */
		trap_dd,			/* 0x1a */
		trap_dd,			/* 0x1b */
		trap_dd,			/* 0x1c */
		trap_dd,			/* 0x1d */
		trap_dd,			/* 0x1e */
		trap_dd,			/* 0x1f */
		trap_dd,			/* 0x20 */
		op_ldixnn,			/* 0x21 */
		op_ldinx,			/* 0x22 */
		op_incix,			/* 0x23 */
		UNDOC(op_undoc_incixh),		/* 0x24 */
		UNDOC(op_undoc_decixh),		/* 0x25 */
		UNDOC(op_undoc_ldixhn),		/* 0x26 */
		trap_dd,			/* 0x27 */
		trap_dd,			/* 0x28 */
		op_addxx,			/* 0x29 */
		op_ldixinn,			/* 0x2a */
		op_decix,			/* 0x2b */
		UNDOC(op_undoc_incixl),		/* 0x2c */
		UNDOC(op_undoc_decixl),		/* 0x2d */
		UNDOC(op_undoc_ldixln),		/* 0x2e */
		trap_dd,			/* 0x2f */
		trap_dd,			/* 0x30 */
		trap_dd,			/* 0x31 */
		trap_dd,			/* 0x32 */
		trap_dd,			/* 0x33 */
		op_incxd,			/* 0x34 */
		op_decxd,			/* 0x35 */
		op_ldxdn,			/* 0x36 */
		trap_dd,			/* 0x37 */
		trap_dd,			/* 0x38 */
		op_addxs,			/* 0x39 */
		trap_dd,			/* 0x3a */
		trap_dd,			/* 0x3b */
		trap_dd,			/* 0x3c */
		trap_dd,			/* 0x3d */
		trap_dd,			/* 0x3e */
		trap_dd,			/* 0x3f */
		trap_dd,			/* 0x40 */
		trap_dd,			/* 0x41 */
		trap_dd,			/* 0x42 */
		trap_dd,			/* 0x43 */
		UNDOC(op_undoc_ldbixh),		/* 0x44 */
		UNDOC(op_undoc_ldbixl),		/* 0x45 */
		op_ldbxd,			/* 0x46 */
		trap_dd,			/* 0x47 */
		trap_dd,			/* 0x48 */
		trap_dd,			/* 0x49 */
		trap_dd,			/* 0x4a */
		trap_dd,			/* 0x4b */
		UNDOC(op_undoc_ldcixh),		/* 0x4c */
		UNDOC(op_undoc_ldcixl),		/* 0x4d */
		op_ldcxd,			/* 0x4e */
		trap_dd,			/* 0x4f */
		trap_dd,			/* 0x50 */
		trap_dd,			/* 0x51 */
		trap_dd,			/* 0x52 */
		trap_dd,			/* 0x53 */
		UNDOC(op_undoc_lddixh),		/* 0x54 */
		UNDOC(op_undoc_lddixl),		/* 0x55 */
		op_lddxd,			/* 0x56 */
		trap_dd,			/* 0x57 */
		trap_dd,			/* 0x58 */
		trap_dd,			/* 0x59 */
		trap_dd,			/* 0x5a */
		trap_dd,			/* 0x5b */
		UNDOC(op_undoc_ldeixh),		/* 0x5c */
		UNDOC(op_undoc_ldeixl),		/* 0x5d */
		op_ldexd,			/* 0x5e */
		trap_dd,			/* 0x5f */
		UNDOC(op_undoc_ldixhb),		/* 0x60 */
		UNDOC(op_undoc_ldixhc),		/* 0x61 */
		UNDOC(op_undoc_ldixhd),		/* 0x62 */
		UNDOC(op_undoc_ldixhe),		/* 0x63 */
		UNDOC(op_undoc_ldixhixh),	/* 0x64 */
		UNDOC(op_undoc_ldixhixl),	/* 0x65 */
		op_ldhxd,			/* 0x66 */
		UNDOC(op_undoc_ldixha),		/* 0x67 */
		UNDOC(op_undoc_ldixlb),		/* 0x68 */
		UNDOC(op_undoc_ldixlc),		/* 0x69 */
		UNDOC(op_undoc_ldixld),		/* 0x6a */
		UNDOC(op_undoc_ldixle),		/* 0x6b */
		UNDOC(op_undoc_ldixlixh),	/* 0x6c */
		UNDOC(op_undoc_ldixlixl),	/* 0x6d */
		op_ldlxd,			/* 0x6e */
		UNDOC(op_undoc_ldixla),		/* 0x6f */
		op_ldxdb,			/* 0x70 */
		op_ldxdc,			/* 0x71 */
		op_ldxdd,			/* 0x72 */
		op_ldxde,			/* 0x73 */
		op_ldxdh,			/* 0x74 */
		op_ldxdl,			/* 0x75 */
		trap_dd,			/* 0x76 */
		op_ldxda,			/* 0x77 */
		trap_dd,			/* 0x78 */
		trap_dd,			/* 0x79 */
		trap_dd,			/* 0x7a */
		trap_dd,			/* 0x7b */
		UNDOC(op_undoc_ldaixh),		/* 0x7c */
		UNDOC(op_undoc_ldaixl),		/* 0x7d */
		op_ldaxd,			/* 0x7e */
		trap_dd,			/* 0x7f */
		trap_dd,			/* 0x80 */
		trap_dd,			/* 0x81 */
		trap_dd,			/* 0x82 */
		trap_dd,			/* 0x83 */
		trap_dd,			/* 0x84 */
		trap_dd,			/* 0x85 */
		op_adaxd,			/* 0x86 */
		trap_dd,			/* 0x87 */
		trap_dd,			/* 0x88 */
		trap_dd,			/* 0x89 */
		trap_dd,			/* 0x8a */
		trap_dd,			/* 0x8b */
		UNDOC(op_undoc_acaixh),		/* 0x8c */
		UNDOC(op_undoc_acaixl),		/* 0x8d */
		op_acaxd,			/* 0x8e */
		trap_dd,			/* 0x8f */
		trap_dd,			/* 0x90 */
		trap_dd,			/* 0x91 */
		trap_dd,			/* 0x92 */
		trap_dd,			/* 0x93 */
		trap_dd,			/* 0x94 */
		trap_dd,			/* 0x95 */
		op_suaxd,			/* 0x96 */
		trap_dd,			/* 0x97 */
		trap_dd,			/* 0x98 */
		trap_dd,			/* 0x99 */
		trap_dd,			/* 0x9a */
		trap_dd,			/* 0x9b */
		UNDOC(op_undoc_scaixh),		/* 0x9c */
		UNDOC(op_undoc_scaixl),		/* 0x9d */
		op_scaxd,			/* 0x9e */
		trap_dd,			/* 0x9f */
		trap_dd,			/* 0xa0 */
		trap_dd,			/* 0xa1 */
		trap_dd,			/* 0xa2 */
		trap_dd,			/* 0xa3 */
		UNDOC(op_undoc_andixh),		/* 0xa4 */
		UNDOC(op_undoc_andixl),		/* 0xa5 */
		op_andxd,			/* 0xa6 */
		trap_dd,			/* 0xa7 */
		trap_dd,			/* 0xa8 */
		trap_dd,			/* 0xa9 */
		trap_dd,			/* 0xaa */
		trap_dd,			/* 0xab */
		trap_dd,			/* 0xac */
		trap_dd,			/* 0xad */
		op_xorxd,			/* 0xae */
		trap_dd,			/* 0xaf */
		trap_dd,			/* 0xb0 */
		trap_dd,			/* 0xb1 */
		trap_dd,			/* 0xb2 */
		trap_dd,			/* 0xb3 */
		UNDOC(op_undoc_oraixh),		/* 0xb4 */
		UNDOC(op_undoc_oraixl),		/* 0xb5 */
		op_orxd,			/* 0xb6 */
		trap_dd,			/* 0xb7 */
		trap_dd,			/* 0xb8 */
		trap_dd,			/* 0xb9 */
		trap_dd,			/* 0xba */
		trap_dd,			/* 0xbb */
		trap_dd,			/* 0xbc */
		UNDOC(op_undoc_cpixl),		/* 0xbd */
		op_cpxd,			/* 0xbe */
		trap_dd,			/* 0xbf */
		trap_dd,			/* 0xc0 */
		trap_dd,			/* 0xc1 */
		trap_dd,			/* 0xc2 */
		trap_dd,			/* 0xc3 */
		trap_dd,			/* 0xc4 */
		trap_dd,			/* 0xc5 */
		trap_dd,			/* 0xc6 */
		trap_dd,			/* 0xc7 */
		trap_dd,			/* 0xc8 */
		trap_dd,			/* 0xc9 */
		trap_dd,			/* 0xca */
		op_ddcb_handel,			/* 0xcb */
		trap_dd,			/* 0xcc */
		trap_dd,			/* 0xcd */
		trap_dd,			/* 0xce */
		trap_dd,			/* 0xcf */
		trap_dd,			/* 0xd0 */
		trap_dd,			/* 0xd1 */
		trap_dd,			/* 0xd2 */
		trap_dd,			/* 0xd3 */
		trap_dd,			/* 0xd4 */
		trap_dd,			/* 0xd5 */
		trap_dd,			/* 0xd6 */
		trap_dd,			/* 0xd7 */
		trap_dd,			/* 0xd8 */
		trap_dd,			/* 0xd9 */
		trap_dd,			/* 0xda */
		trap_dd,			/* 0xdb */
		trap_dd,			/* 0xdc */
		trap_dd,			/* 0xdd */
		trap_dd,			/* 0xde */
		trap_dd,			/* 0xdf */
		trap_dd,			/* 0xe0 */
		op_popix,			/* 0xe1 */
		trap_dd,			/* 0xe2 */
		op_exspx,			/* 0xe3 */
		trap_dd,			/* 0xe4 */
		op_pusix,			/* 0xe5 */
		trap_dd,			/* 0xe6 */
		trap_dd,			/* 0xe7 */
		trap_dd,			/* 0xe8 */
		op_jpix,			/* 0xe9 */
		trap_dd,			/* 0xea */
		trap_dd,			/* 0xeb */
		trap_dd,			/* 0xec */
		trap_dd,			/* 0xed */
		trap_dd,			/* 0xee */
		trap_dd,			/* 0xef */
		trap_dd,			/* 0xf0 */
		trap_dd,			/* 0xf1 */
		trap_dd,			/* 0xf2 */
		trap_dd,			/* 0xf3 */
		trap_dd,			/* 0xf4 */
		trap_dd,			/* 0xf5 */
		trap_dd,			/* 0xf6 */
		trap_dd,			/* 0xf7 */
		trap_dd,			/* 0xf8 */
		op_ldspx,			/* 0xf9 */
		trap_dd,			/* 0xfa */
		trap_dd,			/* 0xfb */
		trap_dd,			/* 0xfc */
		trap_dd,			/* 0xfd */
		trap_dd,			/* 0xfe */
		trap_dd				/* 0xff */
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

	t = (*op_dd[memrdr(PC++)]) ();	/* execute next opcode */

	return(t);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xdd of a multi byte opcode.
 */
static int trap_dd(void)
{
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return(0);
}

static int op_popix(void)		/* POP IX */
{
	IX = memrdr(SP++);
	IX += memrdr(SP++) << 8;
	return(14);
}

static int op_pusix(void)		/* PUSH IX */
{
	memwrt(--SP, IX >> 8);
	memwrt(--SP, IX);
	return(15);
}

static int op_jpix(void)		/* JP (IX) */
{
	PC = IX;
	return(8);
}

static int op_exspx(void)		/* EX (SP),IX */
{
	register WORD i;

	i = memrdr(SP) + (memrdr(SP + 1) << 8);
	memwrt(SP, IX);
	memwrt(SP + 1, IX >> 8);
	IX = i;
	return(23);
}

static int op_ldspx(void)		/* LD SP,IX */
{
	SP = IX;
	return(10);
}

static int op_ldixnn(void)		/* LD IX,nn */
{
	IX = memrdr(PC++);
	IX += memrdr(PC++) << 8;
	return(14);
}

static int op_ldixinn(void)		/* LD IX,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	IX = memrdr(i);
	IX += memrdr(i + 1) << 8;
	return(20);
}

static int op_ldinx(void)		/* LD (nn),IX */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, IX);
	memwrt(i + 1, IX >> 8);
	return(20);
}

static int op_adaxd(void)		/* ADD A,(IX+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IX + (signed char) memrdr(PC++));
	((A & 0xf) + (P	& 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(19);
}

static int op_acaxd(void)		/* ADC A,(IX+d) */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(IX + (signed char) memrdr(PC++));
	((A & 0xf) + (P	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(19);
}

static int op_suaxd(void)		/* SUB A,(IX+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IX + (signed char) memrdr(PC++));
	((P & 0xf) > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(19);
}

static int op_scaxd(void)		/* SBC A,(IX+d) */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(IX + (signed char) memrdr(PC++));
	((P & 0xf) + carry > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(19);
}

static int op_andxd(void)		/* AND (IX+d) */
{
	A &= memrdr(IX + (signed char) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return(19);
}

static int op_xorxd(void)		/* XOR (IX+d) */
{
	A ^= memrdr(IX + (signed char) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return(19);
}

static int op_orxd(void)		/* OR (IX+d) */
{
	A |= memrdr(IX + (signed char) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return(19);
}

static int op_cpxd(void)		/* CP (IX+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IX + (signed char) memrdr(PC++));
	((P & 0xf) > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(19);
}

static int op_incxd(void)		/* INC (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	P++;
	memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F	|= S_FLAG) : (F	&= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(23);
}

static int op_decxd(void)		/* DEC (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F	&= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(23);
}

static int op_addxb(void)		/* ADD IX,BC */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;
	
	carry = (ixl + C > 255) ? 1 : 0;
	ixl += C;
	((ixh & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(ixh + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += B + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
	return(15);
}

static int op_addxd(void)		/* ADD IX,DE */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;
	
	carry = (ixl + E > 255) ? 1 : 0;
	ixl += E;
	((ixh & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(ixh + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += D + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
	return(15);
}

static int op_addxs(void)		/* ADD IX,SP */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;
	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;
	
	carry = (ixl + spl > 255) ? 1 : 0;
	ixl += spl;
	((ixh & 0xf) + (sph & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(ixh + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += sph + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
	return(15);
}

static int op_addxx(void)		/* ADD IX,IX */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;
	
	carry = (ixl << 1 > 255) ? 1 : 0;
	ixl <<= 1;
	((ixh & 0xf) + (ixh & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(ixh + ixh + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += ixh + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
	return(15);
}

static int op_incix(void)		/* INC IX */
{
	IX++;
	return(10);
}

static int op_decix(void)		/* DEC IX */
{
	IX--;
	return(10);
}

static int op_ldaxd(void)		/* LD A,(IX+d) */
{
	A = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_ldbxd(void)		/* LD B,(IX+d) */
{
	B = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_ldcxd(void)		/* LD C,(IX+d) */
{
	C = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_lddxd(void)		/* LD D,(IX+d) */
{
	D = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_ldexd(void)		/* LD E,(IX+d) */
{
	E = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_ldhxd(void)		/* LD H,(IX+d) */
{
	H = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_ldlxd(void)		/* LD L,(IX+d) */
{
	L = memrdr(IX + (signed char) memrdr(PC++));
	return(19);
}

static int op_ldxda(void)		/* LD (IX+d),A */
{
	memwrt(IX + (signed char) memrdr(PC++), A);
	return(19);
}

static int op_ldxdb(void)		/* LD (IX+d),B */
{
	memwrt(IX + (signed char) memrdr(PC++), B);
	return(19);
}

static int op_ldxdc(void)		/* LD (IX+d),C */
{
	memwrt(IX + (signed char) memrdr(PC++), C);
	return(19);
}

static int op_ldxdd(void)		/* LD (IX+d),D */
{
	memwrt(IX + (signed char) memrdr(PC++), D);
	return(19);
}

static int op_ldxde(void)		/* LD (IX+d),E */
{
	memwrt(IX + (signed char) memrdr(PC++), E);
	return(19);
}

static int op_ldxdh(void)		/* LD (IX+d),H */
{
	memwrt(IX + (signed char) memrdr(PC++), H);
	return(19);
}

static int op_ldxdl(void)		/* LD (IX+d),L */
{
	memwrt(IX + (signed char) memrdr(PC++), L);
	return(19);
}

static int op_ldxdn(void)		/* LD (IX+d),n */
{
	register signed char d;

	d = memrdr(PC++);
	memwrt(IX + d, memrdr(PC++));
	return(19);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef Z80_UNDOC

static int op_undoc_ldaixl(void)	/* LD A,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	A = IX & 0xff;
	return(8);
}

static int op_undoc_ldaixh(void)	/* LD A,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	A = IX >> 8;
	return(8);
}

static int op_undoc_ldbixl(void)	/* LD B,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	B = IX & 0xff;
	return(8);
}

static int op_undoc_ldbixh(void)	/* LD B,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	B = IX >> 8;
	return(8);
}

static int op_undoc_ldcixl(void)	/* LD C,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	C = IX & 0xff;
	return(8);
}

static int op_undoc_ldcixh(void)	/* LD C,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	C = IX >> 8;
	return(8);
}

static int op_undoc_lddixl(void)	/* LD D,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	D = IX & 0xff;
	return(8);
}

static int op_undoc_lddixh(void)	/* LD D,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	D = IX >> 8;
	return(8);
}

static int op_undoc_ldeixl(void)	/* LD E,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	E = IX & 0xff;
	return(8);
}

static int op_undoc_ldeixh(void)	/* LD E,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	E = IX >> 8;
	return(8);
}

static int op_undoc_ldixla(void)	/* LD IXL,A */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | A;
	return(8);
}

static int op_undoc_ldixha(void)	/* LD IXH,A */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (A << 8);
	return(8);
}

static int op_undoc_ldixlb(void)	/* LD IXL,B */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | B;
	return(8);
}

static int op_undoc_ldixhb(void)	/* LD IXH,B */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (B << 8);
	return(8);
}

static int op_undoc_ldixlc(void)	/* LD IXL,C */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | C;
	return(8);
}

static int op_undoc_ldixhc(void)	/* LD IXH,C */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (C << 8);
	return(8);
}

static int op_undoc_ldixld(void)	/* LD IXL,D */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | D;
	return(8);
}

static int op_undoc_ldixhd(void)	/* LD IXH,D */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (D << 8);
	return(8);
}

static int op_undoc_ldixle(void)	/* LD IXL,E */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | E;
	return(8);
}

static int op_undoc_ldixhe(void)	/* LD IXH,E */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (E << 8);
	return(8);
}

static int op_undoc_ldixlixh(void)	/* LD IXL,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | (IX >> 8);
	return(8);
}

static int op_undoc_ldixhixh(void)	/* LD IXH,IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	return(8);
}

static int op_undoc_ldixlixl(void)	/* LD IXL,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	return(8);
}

static int op_undoc_ldixhixl(void)	/* LD IXH,IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (IX << 8);
	return(8);
}

static int op_undoc_ldixhn(void)	/* LD IXH,n */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0x00ff) | (memrdr(PC++) << 8);
	return(11);
}

static int op_undoc_ldixln(void)	/* LD IXL,n */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	IX = (IX & 0xff00) | memrdr(PC++);
	return(11);
}

static int op_undoc_cpixl(void)		/* CP IXL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	P = IX & 0xff;
	((P & 0xf) > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(8);
}

static int op_undoc_acaixl(void)	/* ADC A,IXL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX & 0xff;
	((A & 0xf) + (P	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(8);
}

static int op_undoc_acaixh(void)	/* ADC A,IXH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX >> 8;
	((A & 0xf) + (P	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(8);
}

static int op_undoc_scaixl(void)	/* SBC A,IXL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX & 0xff;
	((P & 0xf) + carry > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(8);
}

static int op_undoc_scaixh(void)	/* SBC A,IXH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX >> 8;
	((P & 0xf) + carry > (A	& 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(8);
}

static int op_undoc_oraixl(void)	/* OR IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	A |= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return(8);
}

static int op_undoc_oraixh(void)	/* OR IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	A |= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return(8);
}

static int op_undoc_andixl(void)	/* AND IXL */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	A &= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return(8);
}

static int op_undoc_andixh(void)	/* AND IXH */
{
	if (u_flag) {
		trap_dd();
		return(0);
	}

	A &= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return(8);
}

static int op_undoc_incixl(void)	/* INC IXL */
{
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	P = IX & 0xff;
	P++;
	IX = (IX & 0xff00) | P;
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(8);
}

static int op_undoc_incixh(void)	/* INC IXH */
{
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	P = IX >> 8;
	P++;
	IX = (IX & 0x00ff) | (P << 8);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(8);
}

static int op_undoc_decixl(void)	/* DEC IXL */
{
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	P = IX & 0xff;
	P--;
	IX = (IX & 0xff00) | P;
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(8);
}

static int op_undoc_decixh(void)	/* DEC IXH */
{
	register BYTE P;

	if (u_flag) {
		trap_dd();
		return(0);
	}

	P = IX >> 8;
	P--;
	IX = (IX & 0x00ff) | (P << 8);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(8);
}

#endif
