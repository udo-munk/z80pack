/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	Like the function "cpu_z80()" this one emulates multi byte opcodes
 *	starting with 0xdd
 */

#include <stdint.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "memsim.h"

#if !defined(EXCLUDE_Z80) && !defined(ALT_Z80)

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
extern int op_ddcb_handle(void);

#ifdef UNDOC_INST
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
static int op_undoc_cpixl(void), op_undoc_cpixh(void);
static int op_undoc_adaixl(void), op_undoc_adaixh(void);
static int op_undoc_acaixl(void), op_undoc_acaixh(void);
static int op_undoc_suaixl(void), op_undoc_suaixh(void);
static int op_undoc_scaixl(void), op_undoc_scaixh(void);
static int op_undoc_oraixl(void), op_undoc_oraixh(void);
static int op_undoc_andixl(void), op_undoc_andixh(void);
static int op_undoc_xorixl(void), op_undoc_xorixh(void);
static int op_undoc_incixl(void), op_undoc_incixh(void);
static int op_undoc_decixl(void), op_undoc_decixh(void);
#endif

int op_dd_handle(void)
{

#ifdef UNDOC_INST
#define UNDOC(f) f
#else
#define UNDOC(f) trap_dd
#endif

	static int (*op_dd[256])(void) = {
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
		UNDOC(op_undoc_adaixh),		/* 0x84 */
		UNDOC(op_undoc_adaixl),		/* 0x85 */
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
		UNDOC(op_undoc_suaixh),		/* 0x94 */
		UNDOC(op_undoc_suaixl),		/* 0x95 */
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
		UNDOC(op_undoc_xorixh),		/* 0xac */
		UNDOC(op_undoc_xorixl),		/* 0xad */
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
		UNDOC(op_undoc_cpixh),		/* 0xbc */
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
		op_ddcb_handle,			/* 0xcb */
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

#undef UNDOC

	register int t;
#ifdef FRONTPANEL
	extern uint64_t get_clock_us(void);
	uint64_t clk;
#endif

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	if (F_flag) {
		clk = get_clock_us();
		/* update frontpanel */
		fp_clock++;
		fp_sampleLightGroup(0, 0);
		cpu_time -= get_clock_us() - clk;
	}
#endif

	R++;				/* increment refresh register */

	t = (*op_dd[memrdr(PC++)])();	/* execute next opcode */

	return (t);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xdd of a multi byte opcode.
 */
static int trap_dd(void)
{
#ifdef UNDOC_INST
	if (!u_flag) {
		/* Treat 0xdd prefix as NOP on non IX-instructions */
		PC--;
		R--;
		return (4);
	}
#endif
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return (0);
}

static int op_popix(void)		/* POP IX */
{
	IX = memrdr(SP++);
	IX += memrdr(SP++) << 8;
	return (14);
}

static int op_pusix(void)		/* PUSH IX */
{
	memwrt(--SP, IX >> 8);
	memwrt(--SP, IX);
	return (15);
}

static int op_jpix(void)		/* JP (IX) */
{
	PC = IX;
	return (8);
}

static int op_exspx(void)		/* EX (SP),IX */
{
	register WORD i;

	i = memrdr(SP) + (memrdr(SP + 1) << 8);
	memwrt(SP, IX);
	memwrt(SP + 1, IX >> 8);
	IX = i;
	return (23);
}

static int op_ldspx(void)		/* LD SP,IX */
{
	SP = IX;
	return (10);
}

static int op_ldixnn(void)		/* LD IX,nn */
{
	IX = memrdr(PC++);
	IX += memrdr(PC++) << 8;
	return (14);
}

static int op_ldixinn(void)		/* LD IX,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	IX = memrdr(i);
	IX += memrdr(i + 1) << 8;
	return (20);
}

static int op_ldinx(void)		/* LD (nn),IX */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, IX);
	memwrt(i + 1, IX >> 8);
	return (20);
}

static int op_adaxd(void)		/* ADD A,(IX+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IX + (SBYTE) memrdr(PC++));
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (19);
}

static int op_acaxd(void)		/* ADC A,(IX+d) */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(IX + (SBYTE) memrdr(PC++));
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (19);
}

static int op_suaxd(void)		/* SUB A,(IX+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IX + (SBYTE) memrdr(PC++));
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (19);
}

static int op_scaxd(void)		/* SBC A,(IX+d) */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(IX + (SBYTE) memrdr(PC++));
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (19);
}

static int op_andxd(void)		/* AND (IX+d) */
{
	A &= memrdr(IX + (SBYTE) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return (19);
}

static int op_xorxd(void)		/* XOR (IX+d) */
{
	A ^= memrdr(IX + (SBYTE) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return (19);
}

static int op_orxd(void)		/* OR (IX+d) */
{
	A |= memrdr(IX + (SBYTE) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return (19);
}

static int op_cpxd(void)		/* CP (IX+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IX + (SBYTE) memrdr(PC++));
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (19);
}

static int op_incxd(void)		/* INC (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + (SBYTE) memrdr(PC++);
	P = memrdr(addr);
	P++;
	memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (23);
}

static int op_decxd(void)		/* DEC (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + (SBYTE) memrdr(PC++);
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (23);
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
	return (15);
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
	return (15);
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
	return (15);
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
	return (15);
}

static int op_incix(void)		/* INC IX */
{
	IX++;
	return (10);
}

static int op_decix(void)		/* DEC IX */
{
	IX--;
	return (10);
}

static int op_ldaxd(void)		/* LD A,(IX+d) */
{
	A = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_ldbxd(void)		/* LD B,(IX+d) */
{
	B = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_ldcxd(void)		/* LD C,(IX+d) */
{
	C = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_lddxd(void)		/* LD D,(IX+d) */
{
	D = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_ldexd(void)		/* LD E,(IX+d) */
{
	E = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_ldhxd(void)		/* LD H,(IX+d) */
{
	H = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_ldlxd(void)		/* LD L,(IX+d) */
{
	L = memrdr(IX + (SBYTE) memrdr(PC++));
	return (19);
}

static int op_ldxda(void)		/* LD (IX+d),A */
{
	memwrt(IX + (SBYTE) memrdr(PC++), A);
	return (19);
}

static int op_ldxdb(void)		/* LD (IX+d),B */
{
	memwrt(IX + (SBYTE) memrdr(PC++), B);
	return (19);
}

static int op_ldxdc(void)		/* LD (IX+d),C */
{
	memwrt(IX + (SBYTE) memrdr(PC++), C);
	return (19);
}

static int op_ldxdd(void)		/* LD (IX+d),D */
{
	memwrt(IX + (SBYTE) memrdr(PC++), D);
	return (19);
}

static int op_ldxde(void)		/* LD (IX+d),E */
{
	memwrt(IX + (SBYTE) memrdr(PC++), E);
	return (19);
}

static int op_ldxdh(void)		/* LD (IX+d),H */
{
	memwrt(IX + (SBYTE) memrdr(PC++), H);
	return (19);
}

static int op_ldxdl(void)		/* LD (IX+d),L */
{
	memwrt(IX + (SBYTE) memrdr(PC++), L);
	return (19);
}

static int op_ldxdn(void)		/* LD (IX+d),n */
{
	register SBYTE d;

	d = memrdr(PC++);
	memwrt(IX + d, memrdr(PC++));
	return (19);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

static int op_undoc_ldaixl(void)	/* LD A,IXL */
{
	if (u_flag)
		return (trap_dd());

	A = IX & 0xff;
	return (8);
}

static int op_undoc_ldaixh(void)	/* LD A,IXH */
{
	if (u_flag)
		return (trap_dd());

	A = IX >> 8;
	return (8);
}

static int op_undoc_ldbixl(void)	/* LD B,IXL */
{
	if (u_flag)
		return (trap_dd());

	B = IX & 0xff;
	return (8);
}

static int op_undoc_ldbixh(void)	/* LD B,IXH */
{
	if (u_flag)
		return (trap_dd());

	B = IX >> 8;
	return (8);
}

static int op_undoc_ldcixl(void)	/* LD C,IXL */
{
	if (u_flag)
		return (trap_dd());

	C = IX & 0xff;
	return (8);
}

static int op_undoc_ldcixh(void)	/* LD C,IXH */
{
	if (u_flag)
		return (trap_dd());

	C = IX >> 8;
	return (8);
}

static int op_undoc_lddixl(void)	/* LD D,IXL */
{
	if (u_flag)
		return (trap_dd());

	D = IX & 0xff;
	return (8);
}

static int op_undoc_lddixh(void)	/* LD D,IXH */
{
	if (u_flag)
		return (trap_dd());

	D = IX >> 8;
	return (8);
}

static int op_undoc_ldeixl(void)	/* LD E,IXL */
{
	if (u_flag)
		return (trap_dd());

	E = IX & 0xff;
	return (8);
}

static int op_undoc_ldeixh(void)	/* LD E,IXH */
{
	if (u_flag)
		return (trap_dd());

	E = IX >> 8;
	return (8);
}

static int op_undoc_ldixla(void)	/* LD IXL,A */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | A;
	return (8);
}

static int op_undoc_ldixha(void)	/* LD IXH,A */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (A << 8);
	return (8);
}

static int op_undoc_ldixlb(void)	/* LD IXL,B */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | B;
	return (8);
}

static int op_undoc_ldixhb(void)	/* LD IXH,B */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (B << 8);
	return (8);
}

static int op_undoc_ldixlc(void)	/* LD IXL,C */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | C;
	return (8);
}

static int op_undoc_ldixhc(void)	/* LD IXH,C */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (C << 8);
	return (8);
}

static int op_undoc_ldixld(void)	/* LD IXL,D */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | D;
	return (8);
}

static int op_undoc_ldixhd(void)	/* LD IXH,D */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (D << 8);
	return (8);
}

static int op_undoc_ldixle(void)	/* LD IXL,E */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | E;
	return (8);
}

static int op_undoc_ldixhe(void)	/* LD IXH,E */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (E << 8);
	return (8);
}

static int op_undoc_ldixlixh(void)	/* LD IXL,IXH */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | (IX >> 8);
	return (8);
}

static int op_undoc_ldixhixh(void)	/* LD IXH,IXH */
{
	if (u_flag)
		return (trap_dd());

	return (8);
}

static int op_undoc_ldixlixl(void)	/* LD IXL,IXL */
{
	if (u_flag)
		return (trap_dd());

	return (8);
}

static int op_undoc_ldixhixl(void)	/* LD IXH,IXL */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (IX << 8);
	return (8);
}

static int op_undoc_ldixhn(void)	/* LD IXH,n */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0x00ff) | (memrdr(PC++) << 8);
	return (11);
}

static int op_undoc_ldixln(void)	/* LD IXL,n */
{
	if (u_flag)
		return (trap_dd());

	IX = (IX & 0xff00) | memrdr(PC++);
	return (11);
}

static int op_undoc_cpixl(void)		/* CP IXL */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX & 0xff;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_cpixh(void)		/* CP IXH */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX >> 8;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_adaixl(void)	/* ADD A,IXL */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX & 0xff;
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (8);
}

static int op_undoc_adaixh(void)	/* ADD A,IXH */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX >> 8;
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (8);
}

static int op_undoc_acaixl(void)	/* ADC A,IXL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX & 0xff;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (8);
}

static int op_undoc_acaixh(void)	/* ADC A,IXH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX >> 8;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (8);
}

static int op_undoc_suaixl(void)	/* SUB A,IXL */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX & 0xff;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_suaixh(void)	/* SUB A,IXH */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX >> 8;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_scaixl(void)	/* SBC A,IXL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX & 0xff;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_scaixh(void)	/* SBC A,IXH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX >> 8;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_oraixl(void)	/* OR IXL */
{
	if (u_flag)
		return (trap_dd());

	A |= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return (8);
}

static int op_undoc_oraixh(void)	/* OR IXH */
{
	if (u_flag)
		return (trap_dd());

	A |= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return (8);
}

static int op_undoc_xorixl(void)	/* XOR IXL */
{
	if (u_flag)
		return (trap_dd());

	A ^= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return (8);
}

static int op_undoc_xorixh(void)	/* XOR IXH */
{
	if (u_flag)
		return (trap_dd());

	A ^= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return (8);
}

static int op_undoc_andixl(void)	/* AND IXL */
{
	if (u_flag)
		return (trap_dd());

	A &= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return (8);
}

static int op_undoc_andixh(void)	/* AND IXH */
{
	if (u_flag)
		return (trap_dd());

	A &= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return (8);
}

static int op_undoc_incixl(void)	/* INC IXL */
{
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX & 0xff;
	P++;
	IX = (IX & 0xff00) | P;
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (8);
}

static int op_undoc_incixh(void)	/* INC IXH */
{
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX >> 8;
	P++;
	IX = (IX & 0x00ff) | (P << 8);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return (8);
}

static int op_undoc_decixl(void)	/* DEC IXL */
{
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX & 0xff;
	P--;
	IX = (IX & 0xff00) | P;
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

static int op_undoc_decixh(void)	/* DEC IXH */
{
	register BYTE P;

	if (u_flag)
		return (trap_dd());

	P = IX >> 8;
	P--;
	IX = (IX & 0x00ff) | (P << 8);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return (8);
}

#endif /* UNDOC_INST */

#endif /* !EXCLUDE_Z80 && !ALT_Z80 */
