/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	Like the function "cpu_z80()" this one emulates multi byte opcodes
 *	starting with 0xfd
 */

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simz80-fd.h"
#include "simz80-fdcb.h"

#ifdef FRONTPANEL
#include <stdint.h>
#include "frontpanel.h"
#include "simport.h"
#endif

#if !defined(EXCLUDE_Z80) && !defined(ALT_Z80)

static int trap_fd(void);
static int op_popiy(void), op_pusiy(void);
static int op_jpiy(void);
static int op_exspy(void);
static int op_ldspy(void);
static int op_ldiynn(void), op_ldiyinn(void), op_ldiny(void);
static int op_adayd(void), op_acayd(void), op_suayd(void), op_scayd(void);
static int op_andyd(void), op_xoryd(void), op_oryd(void), op_cpyd(void);
static int op_decyd(void), op_incyd(void);
static int op_addyb(void), op_addyd(void), op_addys(void), op_addyy(void);
static int op_inciy(void), op_deciy(void);
static int op_ldayd(void), op_ldbyd(void), op_ldcyd(void);
static int op_lddyd(void), op_ldeyd(void);
static int op_ldhyd(void), op_ldlyd(void);
static int op_ldyda(void), op_ldydb(void), op_ldydc(void);
static int op_ldydd(void), op_ldyde(void);
static int op_ldydh(void), op_ldydl(void), op_ldydn(void);

#ifdef UNDOC_INST
static int op_undoc_ldaiyl(void), op_undoc_ldaiyh(void);
static int op_undoc_ldbiyl(void), op_undoc_ldbiyh(void);
static int op_undoc_ldciyl(void), op_undoc_ldciyh(void);
static int op_undoc_lddiyl(void), op_undoc_lddiyh(void);
static int op_undoc_ldeiyl(void), op_undoc_ldeiyh(void);
static int op_undoc_ldiyha(void), op_undoc_ldiyla(void);
static int op_undoc_ldiyhb(void), op_undoc_ldiylb(void);
static int op_undoc_ldiyhc(void), op_undoc_ldiylc(void);
static int op_undoc_ldiyhd(void), op_undoc_ldiyld(void);
static int op_undoc_ldiyhe(void), op_undoc_ldiyle(void);
static int op_undoc_ldiyhiyh(void), op_undoc_ldiyliyh(void);
static int op_undoc_ldiyhiyl(void), op_undoc_ldiyliyl(void);
static int op_undoc_ldiyhn(void), op_undoc_ldiyln(void);
static int op_undoc_cpiyl(void), op_undoc_cpiyh(void);
static int op_undoc_adaiyl(void), op_undoc_adaiyh(void);
static int op_undoc_acaiyl(void), op_undoc_acaiyh(void);
static int op_undoc_suaiyl(void), op_undoc_suaiyh(void);
static int op_undoc_scaiyl(void), op_undoc_scaiyh(void);
static int op_undoc_oraiyl(void), op_undoc_oraiyh(void);
static int op_undoc_andiyl(void), op_undoc_andiyh(void);
static int op_undoc_xoriyl(void), op_undoc_xoriyh(void);
static int op_undoc_inciyl(void), op_undoc_inciyh(void);
static int op_undoc_deciyl(void), op_undoc_deciyh(void);
#endif

int op_fd_handle(void)
{

#ifdef UNDOC_INST
#define UNDOC(f) f
#else
#define UNDOC(f) trap_fd
#endif

	static int (*op_fd[256])(void) = {
		trap_fd,			/* 0x00 */
		trap_fd,			/* 0x01 */
		trap_fd,			/* 0x02 */
		trap_fd,			/* 0x03 */
		trap_fd,			/* 0x04 */
		trap_fd,			/* 0x05 */
		trap_fd,			/* 0x06 */
		trap_fd,			/* 0x07 */
		trap_fd,			/* 0x08 */
		op_addyb,			/* 0x09 */
		trap_fd,			/* 0x0a */
		trap_fd,			/* 0x0b */
		trap_fd,			/* 0x0c */
		trap_fd,			/* 0x0d */
		trap_fd,			/* 0x0e */
		trap_fd,			/* 0x0f */
		trap_fd,			/* 0x10 */
		trap_fd,			/* 0x11 */
		trap_fd,			/* 0x12 */
		trap_fd,			/* 0x13 */
		trap_fd,			/* 0x14 */
		trap_fd,			/* 0x15 */
		trap_fd,			/* 0x16 */
		trap_fd,			/* 0x17 */
		trap_fd,			/* 0x18 */
		op_addyd,			/* 0x19 */
		trap_fd,			/* 0x1a */
		trap_fd,			/* 0x1b */
		trap_fd,			/* 0x1c */
		trap_fd,			/* 0x1d */
		trap_fd,			/* 0x1e */
		trap_fd,			/* 0x1f */
		trap_fd,			/* 0x20 */
		op_ldiynn,			/* 0x21 */
		op_ldiny,			/* 0x22 */
		op_inciy,			/* 0x23 */
		UNDOC(op_undoc_inciyh),		/* 0x24 */
		UNDOC(op_undoc_deciyh),		/* 0x25 */
		UNDOC(op_undoc_ldiyhn),		/* 0x26 */
		trap_fd,			/* 0x27 */
		trap_fd,			/* 0x28 */
		op_addyy,			/* 0x29 */
		op_ldiyinn,			/* 0x2a */
		op_deciy,			/* 0x2b */
		UNDOC(op_undoc_inciyl),		/* 0x2c */
		UNDOC(op_undoc_deciyl),		/* 0x2d */
		UNDOC(op_undoc_ldiyln),		/* 0x2e */
		trap_fd,			/* 0x2f */
		trap_fd,			/* 0x30 */
		trap_fd,			/* 0x31 */
		trap_fd,			/* 0x32 */
		trap_fd,			/* 0x33 */
		op_incyd,			/* 0x34 */
		op_decyd,			/* 0x35 */
		op_ldydn,			/* 0x36 */
		trap_fd,			/* 0x37 */
		trap_fd,			/* 0x38 */
		op_addys,			/* 0x39 */
		trap_fd,			/* 0x3a */
		trap_fd,			/* 0x3b */
		trap_fd,			/* 0x3c */
		trap_fd,			/* 0x3d */
		trap_fd,			/* 0x3e */
		trap_fd,			/* 0x3f */
		trap_fd,			/* 0x40 */
		trap_fd,			/* 0x41 */
		trap_fd,			/* 0x42 */
		trap_fd,			/* 0x43 */
		UNDOC(op_undoc_ldbiyh),		/* 0x44 */
		UNDOC(op_undoc_ldbiyl),		/* 0x45 */
		op_ldbyd,			/* 0x46 */
		trap_fd,			/* 0x47 */
		trap_fd,			/* 0x48 */
		trap_fd,			/* 0x49 */
		trap_fd,			/* 0x4a */
		trap_fd,			/* 0x4b */
		UNDOC(op_undoc_ldciyh),		/* 0x4c */
		UNDOC(op_undoc_ldciyl),		/* 0x4d */
		op_ldcyd,			/* 0x4e */
		trap_fd,			/* 0x4f */
		trap_fd,			/* 0x50 */
		trap_fd,			/* 0x51 */
		trap_fd,			/* 0x52 */
		trap_fd,			/* 0x53 */
		UNDOC(op_undoc_lddiyh),		/* 0x54 */
		UNDOC(op_undoc_lddiyl),		/* 0x55 */
		op_lddyd,			/* 0x56 */
		trap_fd,			/* 0x57 */
		trap_fd,			/* 0x58 */
		trap_fd,			/* 0x59 */
		trap_fd,			/* 0x5a */
		trap_fd,			/* 0x5b */
		UNDOC(op_undoc_ldeiyh),		/* 0x5c */
		UNDOC(op_undoc_ldeiyl),		/* 0x5d */
		op_ldeyd,			/* 0x5e */
		trap_fd,			/* 0x5f */
		UNDOC(op_undoc_ldiyhb),		/* 0x60 */
		UNDOC(op_undoc_ldiyhc),		/* 0x61 */
		UNDOC(op_undoc_ldiyhd),		/* 0x62 */
		UNDOC(op_undoc_ldiyhe),		/* 0x63 */
		UNDOC(op_undoc_ldiyhiyh),	/* 0x64 */
		UNDOC(op_undoc_ldiyhiyl),	/* 0x65 */
		op_ldhyd,			/* 0x66 */
		UNDOC(op_undoc_ldiyha),		/* 0x67 */
		UNDOC(op_undoc_ldiylb),		/* 0x68 */
		UNDOC(op_undoc_ldiylc),		/* 0x69 */
		UNDOC(op_undoc_ldiyld),		/* 0x6a */
		UNDOC(op_undoc_ldiyle),		/* 0x6b */
		UNDOC(op_undoc_ldiyliyh),	/* 0x6c */
		UNDOC(op_undoc_ldiyliyl),	/* 0x6d */
		op_ldlyd,			/* 0x6e */
		UNDOC(op_undoc_ldiyla),		/* 0x6f */
		op_ldydb,			/* 0x70 */
		op_ldydc,			/* 0x71 */
		op_ldydd,			/* 0x72 */
		op_ldyde,			/* 0x73 */
		op_ldydh,			/* 0x74 */
		op_ldydl,			/* 0x75 */
		trap_fd,			/* 0x76 */
		op_ldyda,			/* 0x77 */
		trap_fd,			/* 0x78 */
		trap_fd,			/* 0x79 */
		trap_fd,			/* 0x7a */
		trap_fd,			/* 0x7b */
		UNDOC(op_undoc_ldaiyh),		/* 0x7c */
		UNDOC(op_undoc_ldaiyl),		/* 0x7d */
		op_ldayd,			/* 0x7e */
		trap_fd,			/* 0x7f */
		trap_fd,			/* 0x80 */
		trap_fd,			/* 0x81 */
		trap_fd,			/* 0x82 */
		trap_fd,			/* 0x83 */
		UNDOC(op_undoc_adaiyh),		/* 0x84 */
		UNDOC(op_undoc_adaiyl),		/* 0x85 */
		op_adayd,			/* 0x86 */
		trap_fd,			/* 0x87 */
		trap_fd,			/* 0x88 */
		trap_fd,			/* 0x89 */
		trap_fd,			/* 0x8a */
		trap_fd,			/* 0x8b */
		UNDOC(op_undoc_acaiyh),		/* 0x8c */
		UNDOC(op_undoc_acaiyl),		/* 0x8d */
		op_acayd,			/* 0x8e */
		trap_fd,			/* 0x8f */
		trap_fd,			/* 0x90 */
		trap_fd,			/* 0x91 */
		trap_fd,			/* 0x92 */
		trap_fd,			/* 0x93 */
		UNDOC(op_undoc_suaiyh),		/* 0x94 */
		UNDOC(op_undoc_suaiyl),		/* 0x95 */
		op_suayd,			/* 0x96 */
		trap_fd,			/* 0x97 */
		trap_fd,			/* 0x98 */
		trap_fd,			/* 0x99 */
		trap_fd,			/* 0x9a */
		trap_fd,			/* 0x9b */
		UNDOC(op_undoc_scaiyh),		/* 0x9c */
		UNDOC(op_undoc_scaiyl),		/* 0x9d */
		op_scayd,			/* 0x9e */
		trap_fd,			/* 0x9f */
		trap_fd,			/* 0xa0 */
		trap_fd,			/* 0xa1 */
		trap_fd,			/* 0xa2 */
		trap_fd,			/* 0xa3 */
		UNDOC(op_undoc_andiyh),		/* 0xa4 */
		UNDOC(op_undoc_andiyl),		/* 0xa5 */
		op_andyd,			/* 0xa6 */
		trap_fd,			/* 0xa7 */
		trap_fd,			/* 0xa8 */
		trap_fd,			/* 0xa9 */
		trap_fd,			/* 0xaa */
		trap_fd,			/* 0xab */
		UNDOC(op_undoc_xoriyh),		/* 0xac */
		UNDOC(op_undoc_xoriyl),		/* 0xad */
		op_xoryd,			/* 0xae */
		trap_fd,			/* 0xaf */
		trap_fd,			/* 0xb0 */
		trap_fd,			/* 0xb1 */
		trap_fd,			/* 0xb2 */
		trap_fd,			/* 0xb3 */
		UNDOC(op_undoc_oraiyh),		/* 0xb4 */
		UNDOC(op_undoc_oraiyl),		/* 0xb5 */
		op_oryd,			/* 0xb6 */
		trap_fd,			/* 0xb7 */
		trap_fd,			/* 0xb8 */
		trap_fd,			/* 0xb9 */
		trap_fd,			/* 0xba */
		trap_fd,			/* 0xbb */
		UNDOC(op_undoc_cpiyh),		/* 0xbc */
		UNDOC(op_undoc_cpiyl),		/* 0xbd */
		op_cpyd,			/* 0xbe */
		trap_fd,			/* 0xbf */
		trap_fd,			/* 0xc0 */
		trap_fd,			/* 0xc1 */
		trap_fd,			/* 0xc2 */
		trap_fd,			/* 0xc3 */
		trap_fd,			/* 0xc4 */
		trap_fd,			/* 0xc5 */
		trap_fd,			/* 0xc6 */
		trap_fd,			/* 0xc7 */
		trap_fd,			/* 0xc8 */
		trap_fd,			/* 0xc9 */
		trap_fd,			/* 0xca */
		op_fdcb_handle,			/* 0xcb */
		trap_fd,			/* 0xcc */
		trap_fd,			/* 0xcd */
		trap_fd,			/* 0xce */
		trap_fd,			/* 0xcf */
		trap_fd,			/* 0xd0 */
		trap_fd,			/* 0xd1 */
		trap_fd,			/* 0xd2 */
		trap_fd,			/* 0xd3 */
		trap_fd,			/* 0xd4 */
		trap_fd,			/* 0xd5 */
		trap_fd,			/* 0xd6 */
		trap_fd,			/* 0xd7 */
		trap_fd,			/* 0xd8 */
		trap_fd,			/* 0xd9 */
		trap_fd,			/* 0xda */
		trap_fd,			/* 0xdb */
		trap_fd,			/* 0xdc */
		trap_fd,			/* 0xdd */
		trap_fd,			/* 0xde */
		trap_fd,			/* 0xdf */
		trap_fd,			/* 0xe0 */
		op_popiy,			/* 0xe1 */
		trap_fd,			/* 0xe2 */
		op_exspy,			/* 0xe3 */
		trap_fd,			/* 0xe4 */
		op_pusiy,			/* 0xe5 */
		trap_fd,			/* 0xe6 */
		trap_fd,			/* 0xe7 */
		trap_fd,			/* 0xe8 */
		op_jpiy,			/* 0xe9 */
		trap_fd,			/* 0xea */
		trap_fd,			/* 0xeb */
		trap_fd,			/* 0xec */
		trap_fd,			/* 0xed */
		trap_fd,			/* 0xee */
		trap_fd,			/* 0xef */
		trap_fd,			/* 0xf0 */
		trap_fd,			/* 0xf1 */
		trap_fd,			/* 0xf2 */
		trap_fd,			/* 0xf3 */
		trap_fd,			/* 0xf4 */
		trap_fd,			/* 0xf5 */
		trap_fd,			/* 0xf6 */
		trap_fd,			/* 0xf7 */
		trap_fd,			/* 0xf8 */
		op_ldspy,			/* 0xf9 */
		trap_fd,			/* 0xfa */
		trap_fd,			/* 0xfb */
		trap_fd,			/* 0xfc */
		trap_fd,			/* 0xfd */
		trap_fd,			/* 0xfe */
		trap_fd				/* 0xff */
	};

#undef UNDOC

	register int t;
#ifdef FRONTPANEL
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

	t = (*op_fd[memrdr(PC++)])();	/* execute next opcode */

	return t;
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xfd of a multi byte opcode.
 */
static int trap_fd(void)
{
#ifdef UNDOC_INST
	if (!u_flag) {
		/* Treat 0xfd prefix as NOP on non IY-instructions */
		PC--;
		R--;
		return 4;
	}
#endif
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return 0;
}

static int op_popiy(void)		/* POP IY */
{
	IY = memrdr(SP++);
	IY += memrdr(SP++) << 8;
	return 14;
}

static int op_pusiy(void)		/* PUSH IY */
{
	memwrt(--SP, IY >> 8);
	memwrt(--SP, IY);
	return 15;
}

static int op_jpiy(void)		/* JP (IY) */
{
	PC = IY;
	return 8;
}

static int op_exspy(void)		/* EX (SP),IY */
{
	register WORD i;

	i = memrdr(SP) + (memrdr(SP + 1) << 8);
	memwrt(SP, IY);
	memwrt(SP + 1, IY >> 8);
	IY = i;
	return 23;
}

static int op_ldspy(void)		/* LD SP,IY */
{
	SP = IY;
	return 10;
}

static int op_ldiynn(void)		/* LD IY,nn */
{
	IY = memrdr(PC++);
	IY += memrdr(PC++) << 8;
	return 14;
}

static int op_ldiyinn(void)		/* LD IY,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	IY = memrdr(i);
	IY += memrdr(i + 1) << 8;
	return 20;
}

static int op_ldiny(void)		/* LD (nn),IY */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, IY);
	memwrt(i + 1, IY >> 8);
	return 20;
}

static int op_adayd(void)		/* ADD A,(IY+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IY + (SBYTE) memrdr(PC++));
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 19;
}

static int op_acayd(void)		/* ADC A,(IY+d) */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(IY + (SBYTE) memrdr(PC++));
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 19;
}

static int op_suayd(void)		/* SUB A,(IY+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IY + (SBYTE) memrdr(PC++));
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 19;
}

static int op_scayd(void)		/* SBC A,(IY+d) */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(IY + (SBYTE) memrdr(PC++));
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 19;
}

static int op_andyd(void)		/* AND (IY+d) */
{
	A &= memrdr(IY + (SBYTE) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return 19;
}

static int op_xoryd(void)		/* XOR (IY+d) */
{
	A ^= memrdr(IY + (SBYTE) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return 19;
}

static int op_oryd(void)		/* OR (IY+d) */
{
	A |= memrdr(IY + (SBYTE) memrdr(PC++));
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return 19;
}

static int op_cpyd(void)		/* CP (IY+d) */
{
	register int i;
	register BYTE P;

	P = memrdr(IY + (SBYTE) memrdr(PC++));
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 19;
}

static int op_incyd(void)		/* INC (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + (SBYTE) memrdr(PC++);
	P = memrdr(addr);
	P++;
	memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 23;
}

static int op_decyd(void)		/* DEC (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + (SBYTE) memrdr(PC++);
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 23;
}

static int op_addyb(void)		/* ADD IY,BC */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;

	carry = (iyl + C > 255) ? 1 : 0;
	iyl += C;
	((iyh & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(iyh + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += B + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
	return 15;
}

static int op_addyd(void)		/* ADD IY,DE */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;

	carry = (iyl + E > 255) ? 1 : 0;
	iyl += E;
	((iyh & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(iyh + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += D + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
	return 15;
}

static int op_addys(void)		/* ADD IY,SP */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;
	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;

	carry = (iyl + spl > 255) ? 1 : 0;
	iyl += spl;
	((iyh & 0xf) + (sph & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(iyh + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += sph + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
	return 15;
}

static int op_addyy(void)		/* ADD IY,IY */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;

	carry = (iyl << 1 > 255) ? 1 : 0;
	iyl <<= 1;
	((iyh & 0xf) + (iyh & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(iyh + iyh + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += iyh + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
	return 15;
}

static int op_inciy(void)		/* INC IY */
{
	IY++;
	return 10;
}

static int op_deciy(void)		/* DEC IY */
{
	IY--;
	return 10;
}

static int op_ldayd(void)		/* LD A,(IY+d) */
{
	A = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_ldbyd(void)		/* LD B,(IY+d) */
{
	B = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_ldcyd(void)		/* LD C,(IY+d) */
{
	C = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_lddyd(void)		/* LD D,(IY+d) */
{
	D = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_ldeyd(void)		/* LD E,(IY+d) */
{
	E = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_ldhyd(void)		/* LD H,(IY+d) */
{
	H = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_ldlyd(void)		/* LD L,(IY+d) */
{
	L = memrdr(IY + (SBYTE) memrdr(PC++));
	return 19;
}

static int op_ldyda(void)		/* LD (IY+d),A */
{
	memwrt(IY + (SBYTE) memrdr(PC++), A);
	return 19;
}

static int op_ldydb(void)		/* LD (IY+d),B */
{
	memwrt(IY + (SBYTE) memrdr(PC++), B);
	return 19;
}

static int op_ldydc(void)		/* LD (IY+d),C */
{
	memwrt(IY + (SBYTE) memrdr(PC++), C);
	return 19;
}

static int op_ldydd(void)		/* LD (IY+d),D */
{
	memwrt(IY + (SBYTE) memrdr(PC++), D);
	return 19;
}

static int op_ldyde(void)		/* LD (IY+d),E */
{
	memwrt(IY + (SBYTE) memrdr(PC++), E);
	return 19;
}

static int op_ldydh(void)		/* LD (IY+d),H */
{
	memwrt(IY + (SBYTE) memrdr(PC++), H);
	return 19;
}

static int op_ldydl(void)		/* LD (IY+d),L */
{
	memwrt(IY + (SBYTE) memrdr(PC++), L);
	return 19;
}

static int op_ldydn(void)		/* LD (IY+d),n */
{
	register SBYTE d;

	d = memrdr(PC++);
	memwrt(IY + d, memrdr(PC++));
	return 19;
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

static int op_undoc_ldaiyl(void)	/* LD A,IYL */
{
	if (u_flag)
		return trap_fd();

	A = IY & 0xff;
	return 8;
}

static int op_undoc_ldaiyh(void)	/* LD A,IYH */
{
	if (u_flag)
		return trap_fd();

	A = IY >> 8;
	return 8;
}

static int op_undoc_ldbiyl(void)	/* LD B,IYL */
{
	if (u_flag)
		return trap_fd();

	B = IY & 0xff;
	return 8;
}

static int op_undoc_ldbiyh(void)	/* LD B,IYH */
{
	if (u_flag)
		return trap_fd();

	B = IY >> 8;
	return 8;
}

static int op_undoc_ldciyl(void)	/* LD C,IYL */
{
	if (u_flag)
		return trap_fd();

	C = IY & 0xff;
	return 8;
}

static int op_undoc_ldciyh(void)	/* LD C,IYH */
{
	if (u_flag)
		return trap_fd();

	C = IY >> 8;
	return 8;
}

static int op_undoc_lddiyl(void)	/* LD D,IYL */
{
	if (u_flag)
		return trap_fd();

	D = IY & 0xff;
	return 8;
}

static int op_undoc_lddiyh(void)	/* LD D,IYH */
{
	if (u_flag)
		return trap_fd();

	D = IY >> 8;
	return 8;
}

static int op_undoc_ldeiyl(void)	/* LD E,IYL */
{
	if (u_flag)
		return trap_fd();

	E = IY & 0xff;
	return 8;
}

static int op_undoc_ldeiyh(void)	/* LD E,IYH */
{
	if (u_flag)
		return trap_fd();

	E = IY >> 8;
	return 8;
}

static int op_undoc_ldiyla(void)	/* LD IYL,A */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | A;
	return 8;
}

static int op_undoc_ldiyha(void)	/* LD IYH,A */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (A << 8);
	return 8;
}

static int op_undoc_ldiylb(void)	/* LD IYL,B */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | B;
	return 8;
}

static int op_undoc_ldiyhb(void)	/* LD IYH,B */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (B << 8);
	return 8;
}

static int op_undoc_ldiylc(void)	/* LD IYL,C */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | C;
	return 8;
}

static int op_undoc_ldiyhc(void)	/* LD IYH,C */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (C << 8);
	return 8;
}

static int op_undoc_ldiyld(void)	/* LD IYL,D */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | D;
	return 8;
}

static int op_undoc_ldiyhd(void)	/* LD IYH,D */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (D << 8);
	return 8;
}

static int op_undoc_ldiyle(void)	/* LD IYL,E */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | E;
	return 8;
}

static int op_undoc_ldiyhe(void)	/* LD IYH,E */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (E << 8);
	return 8;
}

static int op_undoc_ldiyliyh(void)	/* LD IYL,IYH */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | (IY >> 8);
	return 8;
}

static int op_undoc_ldiyhiyh(void)	/* LD IYH,IYH */
{
	if (u_flag)
		return trap_fd();

	return 8;
}

static int op_undoc_ldiyliyl(void)	/* LD IYL,IYL */
{
	if (u_flag)
		return trap_fd();

	return 8;
}

static int op_undoc_ldiyhiyl(void)	/* LD IYH,IYL */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (IY << 8);
	return 8;
}

static int op_undoc_ldiyhn(void)	/* LD IYH,n */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0x00ff) | (memrdr(PC++) << 8);
	return 11;
}

static int op_undoc_ldiyln(void)	/* LD IYL,n */
{
	if (u_flag)
		return trap_fd();

	IY = (IY & 0xff00) | memrdr(PC++);
	return 11;
}

static int op_undoc_cpiyl(void)		/* CP IYL */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY & 0xff;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_cpiyh(void)		/* CP IYH */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY >> 8;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_adaiyl(void)	/* ADD A,IYL */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY & 0xff;
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 8;
}

static int op_undoc_adaiyh(void)	/* ADD A,IYH */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY >> 8;
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 8;
}

static int op_undoc_acaiyl(void)	/* ADC A,IYL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY & 0xff;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 8;
}

static int op_undoc_acaiyh(void)	/* ADC A,IYH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY >> 8;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A + (SBYTE) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 8;
}

static int op_undoc_suaiyl(void)	/* SUB A,IYL */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY & 0xff;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_suaiyh(void)	/* SUB A,IYH */
{
	register int i;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY >> 8;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_scaiyl(void)	/* SBC A,IYL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY & 0xff;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_scaiyh(void)	/* SBC A,IYH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag)
		return trap_fd();

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY >> 8;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (SBYTE) A - (SBYTE) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_oraiyl(void)	/* OR IYL */
{
	if (u_flag)
		return trap_fd();

	A |= IY & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return 8;
}

static int op_undoc_oraiyh(void)	/* OR IYH */
{
	if (u_flag)
		return trap_fd();

	A |= IY >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return 8;
}

static int op_undoc_xoriyl(void)	/* XOR IYL */
{
	if (u_flag)
		return trap_fd();

	A ^= IY & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return 8;
}

static int op_undoc_xoriyh(void)	/* XOR IYH */
{
	if (u_flag)
		return trap_fd();

	A ^= IY >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
	return 8;
}

static int op_undoc_andiyl(void)	/* AND IYL */
{
	if (u_flag)
		return trap_fd();

	A &= IY & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return 8;
}

static int op_undoc_andiyh(void)	/* AND IYH */
{
	if (u_flag)
		return trap_fd();

	A &= IY >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
	return 8;
}

static int op_undoc_inciyl(void)	/* INC IYL */
{
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY & 0xff;
	P++;
	IY = (IY & 0xff00) | P;
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 8;
}

static int op_undoc_inciyh(void)	/* INC IYH */
{
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY >> 8;
	P++;
	IY = (IY & 0x00ff) | (P << 8);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return 8;
}

static int op_undoc_deciyl(void)	/* DEC IYL */
{
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY & 0xff;
	P--;
	IY = (IY & 0xff00) | P;
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

static int op_undoc_deciyh(void)	/* DEC IYH */
{
	register BYTE P;

	if (u_flag)
		return trap_fd();

	P = IY >> 8;
	P--;
	IY = (IY & 0x00ff) | (P << 8);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return 8;
}

#endif /* UNDOC_INST */

#endif /* !EXCLUDE_Z80 && !ALT_Z80 */
