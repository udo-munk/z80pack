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
 *	starting with 0xed
 */

#include <stdio.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

static int trap_ed(void);
static int op_im0(void), op_im1(void), op_im2(void);
static int op_reti(void), op_retn(void);
static int op_neg(void);
static int op_inaic(void), op_inbic(void), op_incic(void);
static int op_indic(void), op_ineic(void);
static int op_inhic(void), op_inlic(void);
static int op_outca(void), op_outcb(void), op_outcc(void);
static int op_outcd(void), op_outce(void);
static int op_outch(void), op_outcl(void);
static int op_ini(void), op_inir(void), op_ind(void), op_indr(void);
static int op_outi(void), op_otir(void), op_outd(void), op_otdr(void);
static int op_ldai(void), op_ldar(void), op_ldia(void), op_ldra(void);
static int op_ldbcinn(void), op_lddeinn(void);
static int op_ldhlinn(void), op_ldspinn(void);
static int op_ldinbc(void), op_ldinde(void), op_ldinhl(void), op_ldinsp(void);
static int op_adchb(void), op_adchd(void), op_adchh(void), op_adchs(void);
static int op_sbchb(void), op_sbchd(void), op_sbchh(void), op_sbchs(void);
static int op_ldi(void), op_ldir(void), op_ldd(void), op_lddr(void);
static int op_cpi(void), op_cpir(void), op_cpdop(void), op_cpdr(void);
static int op_oprld(void), op_oprrd(void);

int op_ed_handel(void)
{
	register int t;

	static int (*op_ed[256]) (void) = {
		trap_ed,			/* 0x00 */
		trap_ed,			/* 0x01 */
		trap_ed,			/* 0x02 */
		trap_ed,			/* 0x03 */
		trap_ed,			/* 0x04 */
		trap_ed,			/* 0x05 */
		trap_ed,			/* 0x06 */
		trap_ed,			/* 0x07 */
		trap_ed,			/* 0x08 */
		trap_ed,			/* 0x09 */
		trap_ed,			/* 0x0a */
		trap_ed,			/* 0x0b */
		trap_ed,			/* 0x0c */
		trap_ed,			/* 0x0d */
		trap_ed,			/* 0x0e */
		trap_ed,			/* 0x0f */
		trap_ed,			/* 0x10 */
		trap_ed,			/* 0x11 */
		trap_ed,			/* 0x12 */
		trap_ed,			/* 0x13 */
		trap_ed,			/* 0x14 */
		trap_ed,			/* 0x15 */
		trap_ed,			/* 0x16 */
		trap_ed,			/* 0x17 */
		trap_ed,			/* 0x18 */
		trap_ed,			/* 0x19 */
		trap_ed,			/* 0x1a */
		trap_ed,			/* 0x1b */
		trap_ed,			/* 0x1c */
		trap_ed,			/* 0x1d */
		trap_ed,			/* 0x1e */
		trap_ed,			/* 0x1f */
		trap_ed,			/* 0x20 */
		trap_ed,			/* 0x21 */
		trap_ed,			/* 0x22 */
		trap_ed,			/* 0x23 */
		trap_ed,			/* 0x24 */
		trap_ed,			/* 0x25 */
		trap_ed,			/* 0x26 */
		trap_ed,			/* 0x27 */
		trap_ed,			/* 0x28 */
		trap_ed,			/* 0x29 */
		trap_ed,			/* 0x2a */
		trap_ed,			/* 0x2b */
		trap_ed,			/* 0x2c */
		trap_ed,			/* 0x2d */
		trap_ed,			/* 0x2e */
		trap_ed,			/* 0x2f */
		trap_ed,			/* 0x30 */
		trap_ed,			/* 0x31 */
		trap_ed,			/* 0x32 */
		trap_ed,			/* 0x33 */
		trap_ed,			/* 0x34 */
		trap_ed,			/* 0x35 */
		trap_ed,			/* 0x36 */
		trap_ed,			/* 0x37 */
		trap_ed,			/* 0x38 */
		trap_ed,			/* 0x39 */
		trap_ed,			/* 0x3a */
		trap_ed,			/* 0x3b */
		trap_ed,			/* 0x3c */
		trap_ed,			/* 0x3d */
		trap_ed,			/* 0x3e */
		trap_ed,			/* 0x3f */
		op_inbic,			/* 0x40 */
		op_outcb,			/* 0x41 */
		op_sbchb,			/* 0x42 */
		op_ldinbc,			/* 0x43 */
		op_neg,				/* 0x44 */
		op_retn,			/* 0x45 */
		op_im0,				/* 0x46 */
		op_ldia,			/* 0x47 */
		op_incic,			/* 0x48 */
		op_outcc,			/* 0x49 */
		op_adchb,			/* 0x4a */
		op_ldbcinn,			/* 0x4b */
		trap_ed,			/* 0x4c */
		op_reti,			/* 0x4d */
		trap_ed,			/* 0x4e */
		op_ldra,			/* 0x4f */
		op_indic,			/* 0x50 */
		op_outcd,			/* 0x51 */
		op_sbchd,			/* 0x52 */
		op_ldinde,			/* 0x53 */
		trap_ed,			/* 0x54 */
		trap_ed,			/* 0x55 */
		op_im1,				/* 0x56 */
		op_ldai,			/* 0x57 */
		op_ineic,			/* 0x58 */
		op_outce,			/* 0x59 */
		op_adchd,			/* 0x5a */
		op_lddeinn,			/* 0x5b */
		trap_ed,			/* 0x5c */
		trap_ed,			/* 0x5d */
		op_im2,				/* 0x5e */
		op_ldar,			/* 0x5f */
		op_inhic,			/* 0x60 */
		op_outch,			/* 0x61 */
		op_sbchh,			/* 0x62 */
		op_ldinhl,			/* 0x63 */
		trap_ed,			/* 0x64 */
		trap_ed,			/* 0x65 */
		trap_ed,			/* 0x66 */
		op_oprrd,			/* 0x67 */
		op_inlic,			/* 0x68 */
		op_outcl,			/* 0x69 */
		op_adchh,			/* 0x6a */
		op_ldhlinn,			/* 0x6b */
		trap_ed,			/* 0x6c */
		trap_ed,			/* 0x6d */
		trap_ed,			/* 0x6e */
		op_oprld,			/* 0x6f */
		trap_ed,			/* 0x70 */
		trap_ed,			/* 0x71 */
		op_sbchs,			/* 0x72 */
		op_ldinsp,			/* 0x73 */
		trap_ed,			/* 0x74 */
		trap_ed,			/* 0x75 */
		trap_ed,			/* 0x76 */
		trap_ed,			/* 0x77 */
		op_inaic,			/* 0x78 */
		op_outca,			/* 0x79 */
		op_adchs,			/* 0x7a */
		op_ldspinn,			/* 0x7b */
		trap_ed,			/* 0x7c */
		trap_ed,			/* 0x7d */
		trap_ed,			/* 0x7e */
		trap_ed,			/* 0x7f */
		trap_ed,			/* 0x80 */
		trap_ed,			/* 0x81 */
		trap_ed,			/* 0x82 */
		trap_ed,			/* 0x83 */
		trap_ed,			/* 0x84 */
		trap_ed,			/* 0x85 */
		trap_ed,			/* 0x86 */
		trap_ed,			/* 0x87 */
		trap_ed,			/* 0x88 */
		trap_ed,			/* 0x89 */
		trap_ed,			/* 0x8a */
		trap_ed,			/* 0x8b */
		trap_ed,			/* 0x8c */
		trap_ed,			/* 0x8d */
		trap_ed,			/* 0x8e */
		trap_ed,			/* 0x8f */
		trap_ed,			/* 0x90 */
		trap_ed,			/* 0x91 */
		trap_ed,			/* 0x92 */
		trap_ed,			/* 0x93 */
		trap_ed,			/* 0x94 */
		trap_ed,			/* 0x95 */
		trap_ed,			/* 0x96 */
		trap_ed,			/* 0x97 */
		trap_ed,			/* 0x98 */
		trap_ed,			/* 0x99 */
		trap_ed,			/* 0x9a */
		trap_ed,			/* 0x9b */
		trap_ed,			/* 0x9c */
		trap_ed,			/* 0x9d */
		trap_ed,			/* 0x9e */
		trap_ed,			/* 0x9f */
		op_ldi,				/* 0xa0 */
		op_cpi,				/* 0xa1 */
		op_ini,				/* 0xa2 */
		op_outi,			/* 0xa3 */
		trap_ed,			/* 0xa4 */
		trap_ed,			/* 0xa5 */
		trap_ed,			/* 0xa6 */
		trap_ed,			/* 0xa7 */
		op_ldd,				/* 0xa8 */
		op_cpdop,			/* 0xa9 */
		op_ind,				/* 0xaa */
		op_outd,			/* 0xab */
		trap_ed,			/* 0xac */
		trap_ed,			/* 0xad */
		trap_ed,			/* 0xae */
		trap_ed,			/* 0xaf */
		op_ldir,			/* 0xb0 */
		op_cpir,			/* 0xb1 */
		op_inir,			/* 0xb2 */
		op_otir,			/* 0xb3 */
		trap_ed,			/* 0xb4 */
		trap_ed,			/* 0xb5 */
		trap_ed,			/* 0xb6 */
		trap_ed,			/* 0xb7 */
		op_lddr,			/* 0xb8 */
		op_cpdr,			/* 0xb9 */
		op_indr,			/* 0xba */
		op_otdr,			/* 0xbb */
		trap_ed,			/* 0xbc */
		trap_ed,			/* 0xbd */
		trap_ed,			/* 0xbe */
		trap_ed,			/* 0xbf */
		trap_ed,			/* 0xc0 */
		trap_ed,			/* 0xc1 */
		trap_ed,			/* 0xc2 */
		trap_ed,			/* 0xc3 */
		trap_ed,			/* 0xc4 */
		trap_ed,			/* 0xc5 */
		trap_ed,			/* 0xc6 */
		trap_ed,			/* 0xc7 */
		trap_ed,			/* 0xc8 */
		trap_ed,			/* 0xc9 */
		trap_ed,			/* 0xca */
		trap_ed,			/* 0xcb */
		trap_ed,			/* 0xcc */
		trap_ed,			/* 0xcd */
		trap_ed,			/* 0xce */
		trap_ed,			/* 0xcf */
		trap_ed,			/* 0xd0 */
		trap_ed,			/* 0xd1 */
		trap_ed,			/* 0xd2 */
		trap_ed,			/* 0xd3 */
		trap_ed,			/* 0xd4 */
		trap_ed,			/* 0xd5 */
		trap_ed,			/* 0xd6 */
		trap_ed,			/* 0xd7 */
		trap_ed,			/* 0xd8 */
		trap_ed,			/* 0xd9 */
		trap_ed,			/* 0xda */
		trap_ed,			/* 0xdb */
		trap_ed,			/* 0xdc */
		trap_ed,			/* 0xdd */
		trap_ed,			/* 0xde */
		trap_ed,			/* 0xdf */
		trap_ed,			/* 0xe0 */
		trap_ed,			/* 0xe1 */
		trap_ed,			/* 0xe2 */
		trap_ed,			/* 0xe3 */
		trap_ed,			/* 0xe4 */
		trap_ed,			/* 0xe5 */
		trap_ed,			/* 0xe6 */
		trap_ed,			/* 0xe7 */
		trap_ed,			/* 0xe8 */
		trap_ed,			/* 0xe9 */
		trap_ed,			/* 0xea */
		trap_ed,			/* 0xeb */
		trap_ed,			/* 0xec */
		trap_ed,			/* 0xed */
		trap_ed,			/* 0xee */
		trap_ed,			/* 0xef */
		trap_ed,			/* 0xf0 */
		trap_ed,			/* 0xf1 */
		trap_ed,			/* 0xf2 */
		trap_ed,			/* 0xf3 */
		trap_ed,			/* 0xf4 */
		trap_ed,			/* 0xf5 */
		trap_ed,			/* 0xf6 */
		trap_ed,			/* 0xf7 */
		trap_ed,			/* 0xf8 */
		trap_ed,			/* 0xf9 */
		trap_ed,			/* 0xfa */
		trap_ed,			/* 0xfb */
		trap_ed,			/* 0xfc */
		trap_ed,			/* 0xfd */
		trap_ed,			/* 0xfe */
		trap_ed				/* 0xff */
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

	t = (*op_ed[memrdr(PC++)]) ();	/* execute next opcode */

	return(t);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xed of a multi byte opcode.
 */
static int trap_ed(void)
{
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return(0);
}

static int op_im0(void)			/* IM 0 */
{
	int_mode = 0;
	return(8);
}

static int op_im1(void)			/* IM 1 */
{
	int_mode = 1;
	return(8);
}

static int op_im2(void)			/* IM 2 */
{
	int_mode = 2;
	return(8);
}

static int op_reti(void)		/* RETI */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	return(14);
}

static int op_retn(void)		/* RETN */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	if (IFF & 2)
		IFF |= 1;
	return(14);
}

static int op_neg(void)			/* NEG */
{
	(A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	(A == 0x80) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(0 - ((signed char) A & 0xf) <	0) ? (F	|= H_FLAG) : (F	&= ~H_FLAG);
	A = 0 - A;
	F |= N_FLAG;
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(8);
}

static int op_inaic(void)		/* IN A,(C) */
{
	BYTE io_in(BYTE, BYTE);

	A = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_inbic(void)		/* IN B,(C) */
{
	BYTE io_in(BYTE, BYTE);

	B = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_incic(void)		/* IN C,(C) */
{
	BYTE io_in(BYTE, BYTE);

	C = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_indic(void)		/* IN D,(C) */
{
	BYTE io_in(BYTE, BYTE);

	D = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_ineic(void)		/* IN E,(C) */
{
	BYTE io_in(BYTE, BYTE);

	E = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_inhic(void)		/* IN H,(C) */
{
	BYTE io_in(BYTE, BYTE);

	H = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_inlic(void)		/* IN L,(C) */
{
	BYTE io_in(BYTE, BYTE);

	L = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(12);
}

static int op_outca(void)		/* OUT (C),A */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, A);
	return(12);
}

static int op_outcb(void)		/* OUT (C),B */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, B);
	return(12);
}

static int op_outcc(void)		/* OUT (C),C */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, C);
	return(12);
}

static int op_outcd(void)		/* OUT (C),D */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, D);
	return(12);
}

static int op_outce(void)		/* OUT (C),E */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, E);
	return(12);
}

static int op_outch(void)		/* OUT (C),H */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, H);
	return(12);
}

static int op_outcl(void)		/* OUT (C),L */
{
	BYTE io_out(BYTE, BYTE, BYTE);

	io_out(C, B, L);
	return(12);
}

static int op_ini(void)			/* INI */
{
	BYTE io_in(BYTE, BYTE);
	BYTE data;

	data = io_in(C, B);
	memwrt((H << 8) + L, data);
	L++;
	if (!L)
		H++;
	B--;
	F |= N_FLAG;
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(16);
}

static int op_inir(void)		/* INIR */
{
	BYTE io_in(BYTE ,BYTE);
	WORD addr;
	BYTE data;
	register int t	= -21;

	addr = (H << 8) + L;
	do {
		data = io_in(C, B);
		memwrt(addr++, data);
		B--;
		t += 21;
	} while	(B);
	H = addr >> 8;
	L = addr;
	F |= N_FLAG | Z_FLAG;
	return(t + 16);
}

static int op_ind(void)			/* IND */
{
	BYTE io_in(BYTE, BYTE);
	BYTE data;

	data = io_in(C, B);
	memwrt((H << 8) + L, data);
	L--;
	if (L == 0xff)
		H--;
	B--;
	F |= N_FLAG;
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(16);
}

static int op_indr(void)		/* INDR */
{
	BYTE io_in(BYTE, BYTE);
	WORD addr;
	BYTE data;
	register int t	= -21;

	addr = (H << 8) + L;
	do {
		data = io_in(C, B);
		memwrt(addr--, data);
		B--;
		t += 21;
	} while	(B);
	H = addr >> 8;
	L = addr;
	F |= N_FLAG | Z_FLAG;
	return(t + 16);
}

static int op_outi(void)		/* OUTI */
{
	BYTE io_out(BYTE, BYTE, BYTE);
	BYTE data;

	data = memrdr((H << 8) + L);
	io_out(C, B, data);
	L++;
	if (!L)
		H++;
	B--;
	F |= N_FLAG;
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(16);
}

static int op_otir(void)		/* OTIR */
{
	BYTE io_out(BYTE, BYTE, BYTE);
	WORD addr;
	BYTE data;
	register int t	= -21;

	addr = (H << 8) + L;
	do {
		data = memrdr(addr++);
		io_out(C, B, data);
		B--;
		t += 21;
	} while	(B);
	H = addr >> 8;
	L = addr;
	F |= N_FLAG | Z_FLAG;
	return(t + 16);
}

static int op_outd(void)		/* OUTD */
{
	BYTE io_out(BYTE, BYTE, BYTE);
	BYTE data;

	data = memrdr((H << 8) + L);
	io_out(C, B, data);
	L--;
	if (L == 0xff)
		H--;
	B--;
	F |= N_FLAG;
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(16);
}

static int op_otdr(void)		/* OTDR */
{
	BYTE io_out(BYTE, BYTE, BYTE);
	WORD addr;
	BYTE data;
	register int t	= -21;

	addr = (H << 8) + L;
	do {
		data = memrdr(addr--);
		io_out(C, B, data);
		B--;
		t += 21;
	} while	(B);
	H = addr >> 8;
	L = addr;
	F |= N_FLAG | Z_FLAG;
	return(t + 16);
}

static int op_ldai(void)		/* LD A,I */
{
	A = I;
	F &= ~(N_FLAG | H_FLAG);
	(IFF & 2) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(9);
}

static int op_ldar(void)		/* LD A,R */
{
	A = (BYTE) R;
	F &= ~(N_FLAG | H_FLAG);
	(IFF & 2) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(9);
}

static int op_ldia(void)		/* LD I,A */
{
	I = A;
	return(9);
}

static int op_ldra(void)		/* LD R,A */
{
	R = A;
	return(9);
}

static int op_ldbcinn(void)		/* LD BC,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	C = memrdr(i++);
	B = memrdr(i);
	return(20);
}

static int op_lddeinn(void)		/* LD DE,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	E = memrdr(i++);
	D = memrdr(i);
	return(20);
}

static int op_ldhlinn(void)		/* LD HL,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	L = memrdr(i++);
	H = memrdr(i);
	return(20);
}

static int op_ldspinn(void)		/* LD SP,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	SP = memrdr(i++);
	SP += memrdr(i) << 8;
	return(20);
}

static int op_ldinbc(void)		/* LD (nn),BC */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, C);
	memwrt(i, B);
	return(20);
}

static int op_ldinde(void)		/* LD (nn),DE */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, E);
	memwrt(i, D);
	return(20);
}

static int op_ldinhl(void)		/* LD (nn),HL */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, L);
	memwrt(i, H);
	return(20);
}

static int op_ldinsp(void)		/* LD (nn),SP */
{
	register WORD addr;
	WORD i;

	addr = memrdr(PC++);
	addr += memrdr(PC++) << 8;
	i = SP;
	memwrt(addr++, i);
	memwrt(addr, i >> 8);
	return(20);
}

static int op_adchb(void)		/* ADC HL,BC */
{
	int carry, i;
	WORD hl, bc;
	SWORD shl, sbc;

	hl = (H	<< 8) + L;
	bc = (B	<< 8) + C;
	shl = hl;
	sbc = bc;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (bc & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + sbc + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + bc + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return(15);
}

static int op_adchd(void)		/* ADC HL,DE */
{
	int carry, i;
	WORD hl, de;
	SWORD shl, sde;

	hl = (H	<< 8) + L;
	de = (D	<< 8) + E;
	shl = hl;
	sde = de;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (de & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + sde + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + de + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return(15);
}

static int op_adchh(void)		/* ADC HL,HL */
{
	int carry, i;
	WORD hl;
	SWORD shl;

	hl = (H	<< 8) + L;
	shl = hl;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (hl & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + shl + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + hl + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return(15);
}

static int op_adchs(void)		/* ADC HL,SP */
{
	int carry, i;
	WORD hl, sp;
	SWORD shl, ssp;

	hl = (H	<< 8) + L;
	sp = SP;
	shl = hl;
	ssp = sp;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + (sp & 0x0fff) + carry) > 0x0fff) ? (F |= H_FLAG)
							   : (F &= ~H_FLAG);
	i = shl + ssp + carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + sp + carry > 0xffff) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F &= ~N_FLAG;
	return(15);
}

static int op_sbchb(void)		/* SBC HL,BC */
{
	int carry, i;
	WORD hl, bc;
	SWORD shl, sbc;

	hl = (H	<< 8) + L;
	bc = (B	<< 8) + C;
	shl = hl;
	sbc = bc;
	carry = (F & C_FLAG) ? 1 : 0;
	(((bc & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - sbc - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(bc + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return(15);
}

static int op_sbchd(void)		/* SBC HL,DE */
{
	int carry, i;
	WORD hl, de;
	SWORD shl, sde;

	hl = (H	<< 8) + L;
	de = (D	<< 8) + E;
	shl = hl;
	sde = de;
	carry = (F & C_FLAG) ? 1 : 0;
	(((de & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - sde - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(de + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return(15);
}

static int op_sbchh(void)		/* SBC HL,HL */
{
	int carry, i;
	WORD hl;
	SWORD shl;

	hl = (H	<< 8) + L;
	shl = hl;
	carry = (F & C_FLAG) ? 1 : 0;
	(((hl & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - shl - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(hl + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return(15);
}

static int op_sbchs(void)		/* SBC HL,SP */
{
	int carry, i;
	WORD hl, sp;
	SWORD shl, ssp;

	hl = (H	<< 8) + L;
	sp = SP;
	shl = hl;
	ssp = sp;
	carry = (F & C_FLAG) ? 1 : 0;
	(((sp & 0x0fff) + carry) > (hl & 0x0fff)) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	i = shl - ssp - carry;
	((i > 32767) || (i < -32768)) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(sp + carry > hl) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i &= 0xffff;
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 0x8000) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	H = i >> 8;
	L = i;
	F |= N_FLAG;
	return(15);
}

static int op_ldi(void)			/* LDI */
{
	memwrt((D << 8) + E, memrdr((H << 8) + L));
	E++;
	if (!E)
		D++;
	L++;
	if (!L)
		H++;
	C--;
	if (C == 0xff)
		B--;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	F &= ~(N_FLAG | H_FLAG);
	return(16);
}

#ifdef WANT_FASTM
static int op_ldir(void)		/* LDIR */
{
	register int t	= -21;
	register WORD i;
	register WORD s, d;

	i = (B << 8) + C;
	d = (D << 8) + E;
	s = (H << 8) + L;
	do {
		memwrt(d++, memrdr(s++));
		t += 21;
	} while	(--i);
	B = C = 0;
	D = d >> 8;
	E = d;
	H = s >> 8;
	L = s;
	F &= ~(N_FLAG | P_FLAG | H_FLAG);
	return(t + 16);
}
#else
static int op_ldir(void)		/* LDIR */
{
	register int t;

	op_ldi();
	if (F & P_FLAG) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return(t);
}
#endif

static int op_ldd(void)			/* LDD */
{
	memwrt((D << 8) + E, memrdr((H << 8) + L));
	E--;
	if (E == 0xff)
		D--;
	L--;
	if (L == 0xff)
		H--;
	C--;
	if (C == 0xff)
		B--;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	F &= ~(N_FLAG | H_FLAG);
	return(16);
}

#ifdef WANT_FASTM
static int op_lddr(void)		/* LDDR */
{
	register int t	= -21;
	register WORD i;
	register WORD s, d;

	i = (B << 8) + C;
	d = (D << 8) + E;
	s = (H << 8) + L;
	do {
		memwrt(d--, memrdr(s--));
		t += 21;
	} while	(--i);
	B = C = 0;
	D = d  >> 8;
	E = d;
	H = s >> 8;
	L = s;
	F &= ~(N_FLAG | P_FLAG | H_FLAG);
	return(t + 16);
}
#else
static int op_lddr(void)		/* LDDR */
{
	register int t;

	op_ldd();
	if (F & P_FLAG) {
		t = 21;
		PC -= 2;
	} else
		t = 16;
	return(t);
}
#endif

static int op_cpi(void)		/* CPI */
{
	register BYTE i;

	i = memrdr(((H << 8) + L));
	((i & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	i = A - i;
	L++;
	if (!L)
		H++;
	C--;
	if (C == 0xff)
		B--;
	F |= N_FLAG;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(16);
}

static int op_cpir(void)	/* CPIR */
{
	register int t	= -21;
	register WORD s;
	register BYTE d;
	register WORD i;
	register BYTE tmp;

	i = (B << 8) + C;
	s = (H << 8) + L;
	do {
		tmp = memrdr(s++);
		((tmp & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		d = A - tmp;
		t += 21;
	} while	(--i && d);
	F |= N_FLAG;
	B = i >> 8;
	C = i;
	H = s >> 8;
	L = s;
	(i) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(d) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(d & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(t + 16);
}

static int op_cpdop(void)	/* CPD */
{
	register BYTE i;

	i = memrdr(((H << 8) + L));
	((i & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	i = A - i;
	L--;
	if (L == 0xff)
		H--;
	C--;
	if (C == 0xff)
		B--;
	F |= N_FLAG;
	(B | C) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(16);
}

static int op_cpdr(void)	/* CPDR */
{
	register int t	= -21;
	register WORD s;
	register BYTE d;
	register WORD i;
	register BYTE tmp;

	i = (B << 8) + C;
	s = (H << 8) + L;
	do {
		tmp = memrdr(s--);
		((tmp & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		d = A - tmp;
		t += 21;
	} while	(--i && d);
	F |= N_FLAG;
	B = i >> 8;
	C = i;
	H = s >> 8;
	L = s;
	(i) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(d) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(d & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	return(t + 16);
}

static int op_oprld(void)	/* RLD (HL) */
{
	register BYTE i, j;

	i = memrdr((H << 8) + L);
	j = A & 0x0f;
	A = (A & 0xf0) | (i >> 4);
	i = (i << 4) | j;
	memwrt((H << 8) + L, i);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(18);
}

static int op_oprrd(void)	/* RRD (HL) */
{
	register BYTE i, j;

	i = memrdr((H << 8) + L);
	j = A & 0x0f;
	A = (A & 0xf0) | (i & 0x0f);
	i = (i >> 4) | (j << 4);
	memwrt((H << 8) + L, i);
	F &= ~(H_FLAG | N_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(18);
}
