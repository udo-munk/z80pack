/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 instructions
 *	beginning with the prefix 0xdd and a dispatcher for the
 *	prefix 0xdd 0xcb
 */

INSTR(0xe1, op_popix)			/* POP IX */
{
	IX = memrdr(SP++);
	IX += memrdr(SP++) << 8;
	STATES(14);
}

INSTR(0xe5, op_pusix)			/* PUSH IX */
{
	memwrt(--SP, IX >> 8);
	memwrt(--SP, IX);
	STATES(15);
}

INSTR(0xe9, op_jpix)			/* JP (IX) */
{
	PC = IX;
	STATES(8);
}

INSTR(0xe3, op_exspx)			/* EX (SP),IX */
{
	register WORD i;

	i = memrdr(SP) + (memrdr(SP + 1) << 8);
	memwrt(SP, IX);
	memwrt(SP + 1, IX >> 8);
	IX = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(23);
}

INSTR(0xf9, op_ldspx)			/* LD SP,IX */
{
	SP = IX;
	STATES(10);
}

INSTR(0x21, op_ldixnn)			/* LD IX,nn */
{
	IX = memrdr(PC++);
	IX += memrdr(PC++) << 8;
	STATES(14);
}

INSTR(0x2a, op_ldixinn)			/* LD IX,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	IX = memrdr(i++);
	IX += memrdr(i) << 8;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x22, op_ldinx)			/* LD (nn),IX */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, IX);
	memwrt(i, IX >> 8);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x86, op_adaxd)			/* ADD A,(IX+d) */
{
	register int i;
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0x8e, op_acaxd)			/* ADC A,(IX+d) */
{
	register int i, carry;
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(addr);
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0x96, op_suaxd)			/* SUB A,(IX+d) */
{
	register int i;
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0x9e, op_scaxd)			/* SBC A,(IX+d) */
{
	register int i, carry;
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(addr);
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0xa6, op_andxd)			/* AND (IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	A &= memrdr(addr);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0xae, op_xorxd)			/* XOR (IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	A ^= memrdr(addr);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0xb6, op_orxd)			/* OR (IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	A |= memrdr(addr);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0xbe, op_cpxd)			/* CP (IX+d) */
{
	register int i;
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(19);
}

INSTR(0x34, op_incxd)			/* INC (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	P++;
	memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTR(0x35, op_decxd)			/* DEC (IX+d) */
{
	register BYTE P;
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr;
	modF = 1;
#endif
	STATES(23);
}

INSTR(0x09, op_addxb)			/* ADD IX,BC */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;

#ifdef UNDOC_FLAGS
	WZ = IX + 1;
#endif
	carry = (ixl + C > 255) ? 1 : 0;
	ixl += C;
	((ixh & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(ixh + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += B + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(ixh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(ixh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x19, op_addxd)			/* ADD IX,DE */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;

#ifdef UNDOC_FLAGS
	WZ = IX + 1;
#endif
	carry = (ixl + E > 255) ? 1 : 0;
	ixl += E;
	((ixh & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(ixh + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += D + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(ixh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(ixh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x39, op_addxs)			/* ADD IX,SP */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;
	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;

#ifdef UNDOC_FLAGS
	WZ = IX + 1;
#endif
	carry = (ixl + spl > 255) ? 1 : 0;
	ixl += spl;
	((ixh & 0xf) + (sph & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(ixh + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += sph + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(ixh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(ixh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x29, op_addxx)			/* ADD IX,IX */
{
	register int carry;
	BYTE ixl = IX & 0xff;
	BYTE ixh = IX >> 8;

#ifdef UNDOC_FLAGS
	WZ = IX + 1;
#endif
	carry = (ixl << 1 > 255) ? 1 : 0;
	ixl <<= 1;
	((ixh & 0xf) + (ixh & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(ixh + ixh + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	ixh += ixh + carry;
	IX = (ixh << 8) + ixl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(ixh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(ixh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x23, op_incix)			/* INC IX */
{
	IX++;
	STATES(10);
}

INSTR(0x2b, op_decix)			/* DEC IX */
{
	IX--;
	STATES(10);
}

INSTR(0x7e, op_ldaxd)			/* LD A,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	A = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x46, op_ldbxd)			/* LD B,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	B = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x4e, op_ldcxd)			/* LD C,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	C = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x56, op_lddxd)			/* LD D,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	D = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x5e, op_ldexd)			/* LD E,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	E = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x66, op_ldhxd)			/* LD H,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	H = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x6e, op_ldlxd)			/* LD L,(IX+d) */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	L = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x77, op_ldxda)			/* LD (IX+d),A */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x70, op_ldxdb)			/* LD (IX+d),B */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x71, op_ldxdc)			/* LD (IX+d),C */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x72, op_ldxdd)			/* LD (IX+d),D */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x73, op_ldxde)			/* LD (IX+d),E */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x74, op_ldxdh)			/* LD (IX+d),H */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x75, op_ldxdl)			/* LD (IX+d),L */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x36, op_ldxdn)			/* LD (IX+d),n */
{
	WORD addr;

	addr = IX + (signed char) memrdr(PC++);
	memwrt(addr, memrdr(PC++));
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0xcb, op_ddcb_handle)		/* 0xdd 0xcb prefix */
{
#ifndef FAST_INSTR

#ifdef UNDOC_INST
#define UNDOC(f) f
#ifdef UNDOC_IALL
#define UNDOCA(f) f
#else
#define UNDOCA(f) trap_ddcb
#endif
#else
#define UNDOC(f) trap_ddcb
#define UNDOCA(f) trap_ddcb
#endif

	static int (*op_ddcb[256])(int) = {
		UNDOCA(op_undoc_rlcixdb),	/* 0x00 */
		UNDOCA(op_undoc_rlcixdc),	/* 0x01 */
		UNDOCA(op_undoc_rlcixdd),	/* 0x02 */
		UNDOCA(op_undoc_rlcixde),	/* 0x03 */
		UNDOCA(op_undoc_rlcixdh),	/* 0x04 */
		UNDOCA(op_undoc_rlcixdl),	/* 0x05 */
		op_rlcixd,			/* 0x06 */
		UNDOCA(op_undoc_rlcixda),	/* 0x07 */
		UNDOCA(op_undoc_rrcixdb),	/* 0x08 */
		UNDOCA(op_undoc_rrcixdc),	/* 0x09 */
		UNDOCA(op_undoc_rrcixdd),	/* 0x0a */
		UNDOCA(op_undoc_rrcixde),	/* 0x0b */
		UNDOCA(op_undoc_rrcixdh),	/* 0x0c */
		UNDOCA(op_undoc_rrcixdl),	/* 0x0d */
		op_rrcixd,			/* 0x0e */
		UNDOCA(op_undoc_rrcixda),	/* 0x0f */
		UNDOCA(op_undoc_rlixdb),	/* 0x10 */
		UNDOCA(op_undoc_rlixdc),	/* 0x11 */
		UNDOCA(op_undoc_rlixdd),	/* 0x12 */
		UNDOCA(op_undoc_rlixde),	/* 0x13 */
		UNDOCA(op_undoc_rlixdh),	/* 0x14 */
		UNDOCA(op_undoc_rlixdl),	/* 0x15 */
		op_rlixd,			/* 0x16 */
		UNDOCA(op_undoc_rlixda),	/* 0x17 */
		UNDOCA(op_undoc_rrixdb),	/* 0x18 */
		UNDOCA(op_undoc_rrixdc),	/* 0x19 */
		UNDOCA(op_undoc_rrixdd),	/* 0x1a */
		UNDOCA(op_undoc_rrixde),	/* 0x1b */
		UNDOCA(op_undoc_rrixdh),	/* 0x1c */
		UNDOCA(op_undoc_rrixdl),	/* 0x1d */
		op_rrixd,			/* 0x1e */
		UNDOCA(op_undoc_rrixda),	/* 0x1f */
		UNDOCA(op_undoc_slaixdb),	/* 0x20 */
		UNDOCA(op_undoc_slaixdc),	/* 0x21 */
		UNDOCA(op_undoc_slaixdd),	/* 0x22 */
		UNDOCA(op_undoc_slaixde),	/* 0x23 */
		UNDOCA(op_undoc_slaixdh),	/* 0x24 */
		UNDOCA(op_undoc_slaixdl),	/* 0x25 */
		op_slaixd,			/* 0x26 */
		UNDOCA(op_undoc_slaixda),	/* 0x27 */
		UNDOCA(op_undoc_sraixdb),	/* 0x28 */
		UNDOCA(op_undoc_sraixdc),	/* 0x29 */
		UNDOCA(op_undoc_sraixdd),	/* 0x2a */
		UNDOCA(op_undoc_sraixde),	/* 0x2b */
		UNDOCA(op_undoc_sraixdh),	/* 0x2c */
		UNDOCA(op_undoc_sraixdl),	/* 0x2d */
		op_sraixd,			/* 0x2e */
		UNDOCA(op_undoc_sraixda),	/* 0x2f */
		UNDOCA(op_undoc_sllixdb),	/* 0x30 */
		UNDOCA(op_undoc_sllixdc),	/* 0x31 */
		UNDOCA(op_undoc_sllixdd),	/* 0x32 */
		UNDOCA(op_undoc_sllixde),	/* 0x33 */
		UNDOCA(op_undoc_sllixdh),	/* 0x34 */
		UNDOCA(op_undoc_sllixdl),	/* 0x35 */
		UNDOC(op_undoc_sllixd),		/* 0x36 */
		UNDOCA(op_undoc_sllixda),	/* 0x37 */
		UNDOCA(op_undoc_srlixdb),	/* 0x38 */
		UNDOCA(op_undoc_srlixdc),	/* 0x39 */
		UNDOCA(op_undoc_srlixdd),	/* 0x3a */
		UNDOCA(op_undoc_srlixde),	/* 0x3b */
		UNDOCA(op_undoc_srlixdh),	/* 0x3c */
		UNDOCA(op_undoc_srlixdl),	/* 0x3d */
		op_srlixd,			/* 0x3e */
		UNDOCA(op_undoc_srlixda),	/* 0x3f */
		UNDOCA(op_undoc_tb0ixd),	/* 0x40 */
		UNDOCA(op_undoc_tb0ixd),	/* 0x41 */
		UNDOCA(op_undoc_tb0ixd),	/* 0x42 */
		UNDOCA(op_undoc_tb0ixd),	/* 0x43 */
		UNDOCA(op_undoc_tb0ixd),	/* 0x44 */
		UNDOCA(op_undoc_tb0ixd),	/* 0x45 */
		op_tb0ixd,			/* 0x46 */
		UNDOCA(op_undoc_tb0ixd),	/* 0x47 */
		UNDOCA(op_undoc_tb1ixd),	/* 0x48 */
		UNDOCA(op_undoc_tb1ixd),	/* 0x49 */
		UNDOCA(op_undoc_tb1ixd),	/* 0x4a */
		UNDOCA(op_undoc_tb1ixd),	/* 0x4b */
		UNDOCA(op_undoc_tb1ixd),	/* 0x4c */
		UNDOCA(op_undoc_tb1ixd),	/* 0x4d */
		op_tb1ixd,			/* 0x4e */
		UNDOCA(op_undoc_tb1ixd),	/* 0x4f */
		UNDOCA(op_undoc_tb2ixd),	/* 0x50 */
		UNDOCA(op_undoc_tb2ixd),	/* 0x51 */
		UNDOCA(op_undoc_tb2ixd),	/* 0x52 */
		UNDOCA(op_undoc_tb2ixd),	/* 0x53 */
		UNDOCA(op_undoc_tb2ixd),	/* 0x54 */
		UNDOCA(op_undoc_tb2ixd),	/* 0x55 */
		op_tb2ixd,			/* 0x56 */
		UNDOCA(op_undoc_tb2ixd),	/* 0x57 */
		UNDOCA(op_undoc_tb3ixd),	/* 0x58 */
		UNDOCA(op_undoc_tb3ixd),	/* 0x59 */
		UNDOCA(op_undoc_tb3ixd),	/* 0x5a */
		UNDOCA(op_undoc_tb3ixd),	/* 0x5b */
		UNDOCA(op_undoc_tb3ixd),	/* 0x5c */
		UNDOCA(op_undoc_tb3ixd),	/* 0x5d */
		op_tb3ixd,			/* 0x5e */
		UNDOCA(op_undoc_tb3ixd),	/* 0x5f */
		UNDOCA(op_undoc_tb4ixd),	/* 0x60 */
		UNDOCA(op_undoc_tb4ixd),	/* 0x61 */
		UNDOCA(op_undoc_tb4ixd),	/* 0x62 */
		UNDOCA(op_undoc_tb4ixd),	/* 0x63 */
		UNDOCA(op_undoc_tb4ixd),	/* 0x64 */
		UNDOCA(op_undoc_tb4ixd),	/* 0x65 */
		op_tb4ixd,			/* 0x66 */
		UNDOCA(op_undoc_tb4ixd),	/* 0x67 */
		UNDOCA(op_undoc_tb5ixd),	/* 0x68 */
		UNDOCA(op_undoc_tb5ixd),	/* 0x69 */
		UNDOCA(op_undoc_tb5ixd),	/* 0x6a */
		UNDOCA(op_undoc_tb5ixd),	/* 0x6b */
		UNDOCA(op_undoc_tb5ixd),	/* 0x6c */
		UNDOCA(op_undoc_tb5ixd),	/* 0x6d */
		op_tb5ixd,			/* 0x6e */
		UNDOCA(op_undoc_tb5ixd),	/* 0x6f */
		UNDOCA(op_undoc_tb6ixd),	/* 0x70 */
		UNDOCA(op_undoc_tb6ixd),	/* 0x71 */
		UNDOCA(op_undoc_tb6ixd),	/* 0x72 */
		UNDOCA(op_undoc_tb6ixd),	/* 0x73 */
		UNDOCA(op_undoc_tb6ixd),	/* 0x74 */
		UNDOCA(op_undoc_tb6ixd),	/* 0x75 */
		op_tb6ixd,			/* 0x76 */
		UNDOCA(op_undoc_tb6ixd),	/* 0x77 */
		UNDOCA(op_undoc_tb7ixd),	/* 0x78 */
		UNDOCA(op_undoc_tb7ixd),	/* 0x79 */
		UNDOCA(op_undoc_tb7ixd),	/* 0x7a */
		UNDOCA(op_undoc_tb7ixd),	/* 0x7b */
		UNDOCA(op_undoc_tb7ixd),	/* 0x7c */
		UNDOCA(op_undoc_tb7ixd),	/* 0x7d */
		op_tb7ixd,			/* 0x7e */
		UNDOCA(op_undoc_tb7ixd),	/* 0x7f */
		UNDOCA(op_undoc_rb0ixdb),	/* 0x80 */
		UNDOCA(op_undoc_rb0ixdc),	/* 0x81 */
		UNDOCA(op_undoc_rb0ixdd),	/* 0x82 */
		UNDOCA(op_undoc_rb0ixde),	/* 0x83 */
		UNDOCA(op_undoc_rb0ixdh),	/* 0x84 */
		UNDOCA(op_undoc_rb0ixdl),	/* 0x85 */
		op_rb0ixd,			/* 0x86 */
		UNDOCA(op_undoc_rb0ixda),	/* 0x87 */
		UNDOCA(op_undoc_rb1ixdb),	/* 0x88 */
		UNDOCA(op_undoc_rb1ixdc),	/* 0x89 */
		UNDOCA(op_undoc_rb1ixdd),	/* 0x8a */
		UNDOCA(op_undoc_rb1ixde),	/* 0x8b */
		UNDOCA(op_undoc_rb1ixdh),	/* 0x8c */
		UNDOCA(op_undoc_rb1ixdl),	/* 0x8d */
		op_rb1ixd,			/* 0x8e */
		UNDOCA(op_undoc_rb1ixda),	/* 0x8f */
		UNDOCA(op_undoc_rb2ixdb),	/* 0x90 */
		UNDOCA(op_undoc_rb2ixdc),	/* 0x91 */
		UNDOCA(op_undoc_rb2ixdd),	/* 0x92 */
		UNDOCA(op_undoc_rb2ixde),	/* 0x93 */
		UNDOCA(op_undoc_rb2ixdh),	/* 0x94 */
		UNDOCA(op_undoc_rb2ixdl),	/* 0x95 */
		op_rb2ixd,			/* 0x96 */
		UNDOCA(op_undoc_rb2ixda),	/* 0x97 */
		UNDOCA(op_undoc_rb3ixdb),	/* 0x98 */
		UNDOCA(op_undoc_rb3ixdc),	/* 0x99 */
		UNDOCA(op_undoc_rb3ixdd),	/* 0x9a */
		UNDOCA(op_undoc_rb3ixde),	/* 0x9b */
		UNDOCA(op_undoc_rb3ixdh),	/* 0x9c */
		UNDOCA(op_undoc_rb3ixdl),	/* 0x9d */
		op_rb3ixd,			/* 0x9e */
		UNDOCA(op_undoc_rb3ixda),	/* 0x9f */
		UNDOCA(op_undoc_rb4ixdb),	/* 0xa0 */
		UNDOCA(op_undoc_rb4ixdc),	/* 0xa1 */
		UNDOCA(op_undoc_rb4ixdd),	/* 0xa2 */
		UNDOCA(op_undoc_rb4ixde),	/* 0xa3 */
		UNDOCA(op_undoc_rb4ixdh),	/* 0xa4 */
		UNDOCA(op_undoc_rb4ixdl),	/* 0xa5 */
		op_rb4ixd,			/* 0xa6 */
		UNDOCA(op_undoc_rb4ixda),	/* 0xa7 */
		UNDOCA(op_undoc_rb5ixdb),	/* 0xa8 */
		UNDOCA(op_undoc_rb5ixdc),	/* 0xa9 */
		UNDOCA(op_undoc_rb5ixdd),	/* 0xaa */
		UNDOCA(op_undoc_rb5ixde),	/* 0xab */
		UNDOCA(op_undoc_rb5ixdh),	/* 0xac */
		UNDOCA(op_undoc_rb5ixdl),	/* 0xad */
		op_rb5ixd,			/* 0xae */
		UNDOCA(op_undoc_rb5ixda),	/* 0xaf */
		UNDOCA(op_undoc_rb6ixdb),	/* 0xb0 */
		UNDOCA(op_undoc_rb6ixdc),	/* 0xb1 */
		UNDOCA(op_undoc_rb6ixdd),	/* 0xb2 */
		UNDOCA(op_undoc_rb6ixde),	/* 0xb3 */
		UNDOCA(op_undoc_rb6ixdh),	/* 0xb4 */
		UNDOCA(op_undoc_rb6ixdl),	/* 0xb5 */
		op_rb6ixd,			/* 0xb6 */
		UNDOCA(op_undoc_rb6ixda),	/* 0xb7 */
		UNDOCA(op_undoc_rb7ixdb),	/* 0xb8 */
		UNDOCA(op_undoc_rb7ixdc),	/* 0xb9 */
		UNDOCA(op_undoc_rb7ixdd),	/* 0xba */
		UNDOCA(op_undoc_rb7ixde),	/* 0xbb */
		UNDOCA(op_undoc_rb7ixdh),	/* 0xbc */
		UNDOCA(op_undoc_rb7ixdl),	/* 0xbd */
		op_rb7ixd,			/* 0xbe */
		UNDOCA(op_undoc_rb7ixda),	/* 0xbf */
		UNDOCA(op_undoc_sb0ixdb),	/* 0xc0 */
		UNDOCA(op_undoc_sb0ixdc),	/* 0xc1 */
		UNDOCA(op_undoc_sb0ixdd),	/* 0xc2 */
		UNDOCA(op_undoc_sb0ixde),	/* 0xc3 */
		UNDOCA(op_undoc_sb0ixdh),	/* 0xc4 */
		UNDOCA(op_undoc_sb0ixdl),	/* 0xc5 */
		op_sb0ixd,			/* 0xc6 */
		UNDOCA(op_undoc_sb0ixda),	/* 0xc7 */
		UNDOCA(op_undoc_sb1ixdb),	/* 0xc8 */
		UNDOCA(op_undoc_sb1ixdc),	/* 0xc9 */
		UNDOCA(op_undoc_sb1ixdd),	/* 0xca */
		UNDOCA(op_undoc_sb1ixde),	/* 0xcb */
		UNDOCA(op_undoc_sb1ixdh),	/* 0xcc */
		UNDOCA(op_undoc_sb1ixdl),	/* 0xcd */
		op_sb1ixd,			/* 0xce */
		UNDOCA(op_undoc_sb1ixda),	/* 0xcf */
		UNDOCA(op_undoc_sb2ixdb),	/* 0xd0 */
		UNDOCA(op_undoc_sb2ixdc),	/* 0xd1 */
		UNDOCA(op_undoc_sb2ixdd),	/* 0xd2 */
		UNDOCA(op_undoc_sb2ixde),	/* 0xd3 */
		UNDOCA(op_undoc_sb2ixdh),	/* 0xd4 */
		UNDOCA(op_undoc_sb2ixdl),	/* 0xd5 */
		op_sb2ixd,			/* 0xd6 */
		UNDOCA(op_undoc_sb2ixda),	/* 0xd7 */
		UNDOCA(op_undoc_sb3ixdb),	/* 0xd8 */
		UNDOCA(op_undoc_sb3ixdc),	/* 0xd9 */
		UNDOCA(op_undoc_sb3ixdd),	/* 0xda */
		UNDOCA(op_undoc_sb3ixde),	/* 0xdb */
		UNDOCA(op_undoc_sb3ixdh),	/* 0xdc */
		UNDOCA(op_undoc_sb3ixdl),	/* 0xdd */
		op_sb3ixd,			/* 0xde */
		UNDOCA(op_undoc_sb3ixda),	/* 0xdf */
		UNDOCA(op_undoc_sb4ixdb),	/* 0xe0 */
		UNDOCA(op_undoc_sb4ixdc),	/* 0xe1 */
		UNDOCA(op_undoc_sb4ixdd),	/* 0xe2 */
		UNDOCA(op_undoc_sb4ixde),	/* 0xe3 */
		UNDOCA(op_undoc_sb4ixdh),	/* 0xe4 */
		UNDOCA(op_undoc_sb4ixdl),	/* 0xe5 */
		op_sb4ixd,			/* 0xe6 */
		UNDOCA(op_undoc_sb4ixda),	/* 0xe7 */
		UNDOCA(op_undoc_sb5ixdb),	/* 0xe8 */
		UNDOCA(op_undoc_sb5ixdc),	/* 0xe9 */
		UNDOCA(op_undoc_sb5ixdd),	/* 0xea */
		UNDOCA(op_undoc_sb5ixde),	/* 0xeb */
		UNDOCA(op_undoc_sb5ixdh),	/* 0xec */
		UNDOCA(op_undoc_sb5ixdl),	/* 0xed */
		op_sb5ixd,			/* 0xee */
		UNDOCA(op_undoc_sb5ixda),	/* 0xef */
		UNDOCA(op_undoc_sb6ixdb),	/* 0xf0 */
		UNDOCA(op_undoc_sb6ixdc),	/* 0xf1 */
		UNDOCA(op_undoc_sb6ixdd),	/* 0xf2 */
		UNDOCA(op_undoc_sb6ixde),	/* 0xf3 */
		UNDOCA(op_undoc_sb6ixdh),	/* 0xf4 */
		UNDOCA(op_undoc_sb6ixdl),	/* 0xf5 */
		op_sb6ixd,			/* 0xf6 */
		UNDOCA(op_undoc_sb6ixda),	/* 0xf7 */
		UNDOCA(op_undoc_sb7ixdb),	/* 0xf8 */
		UNDOCA(op_undoc_sb7ixdc),	/* 0xf9 */
		UNDOCA(op_undoc_sb7ixdd),	/* 0xfa */
		UNDOCA(op_undoc_sb7ixde),	/* 0xfb */
		UNDOCA(op_undoc_sb7ixdh),	/* 0xfc */
		UNDOCA(op_undoc_sb7ixdl),	/* 0xfd */
		op_sb7ixd,			/* 0xfe */
		UNDOCA(op_undoc_sb7ixda)	/* 0xff */
	};

#undef UNDOC
#undef UNDOCA

	register int t;

#endif /* !FAST_INSTR */

	register int data;

	data = (signed char) memrdr(PC++);

#ifndef FAST_INSTR
	t = (*op_ddcb[memrdr(PC++)])(data); /* execute next opcode */
#else
	switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-ddcb.c"

	default:
		t = trap_ddcb(data);
		break;
	}
#endif

	STATES(t);
}

#ifndef FAST_INSTR
#include "simz80-ddcb.c"
#endif

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

INSTR(0x7d, op_undoc_ldaixl)		/* LD A,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A = IX & 0xff;
	STATES(8);
}

INSTR(0x7c, op_undoc_ldaixh)		/* LD A,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A = IX >> 8;
	STATES(8);
}

INSTR(0x45, op_undoc_ldbixl)		/* LD B,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	B = IX & 0xff;
	STATES(8);
}

INSTR(0x44, op_undoc_ldbixh)		/* LD B,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	B = IX >> 8;
	STATES(8);
}

INSTR(0x4d, op_undoc_ldcixl)		/* LD C,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	C = IX & 0xff;
	STATES(8);
}

INSTR(0x4c, op_undoc_ldcixh)		/* LD C,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	C = IX >> 8;
	STATES(8);
}

INSTR(0x55, op_undoc_lddixl)		/* LD D,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	D = IX & 0xff;
	STATES(8);
}

INSTR(0x54, op_undoc_lddixh)		/* LD D,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	D = IX >> 8;
	STATES(8);
}

INSTR(0x5d, op_undoc_ldeixl)		/* LD E,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	E = IX & 0xff;
	STATES(8);
}

INSTR(0x5c, op_undoc_ldeixh)		/* LD E,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	E = IX >> 8;
	STATES(8);
}

INSTR(0x6f, op_undoc_ldixla)		/* LD IXL,A */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | A;
	STATES(8);
}

INSTR(0x67, op_undoc_ldixha)		/* LD IXH,A */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (A << 8);
	STATES(8);
}

INSTR(0x68, op_undoc_ldixlb)		/* LD IXL,B */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | B;
	STATES(8);
}

INSTR(0x60, op_undoc_ldixhb)		/* LD IXH,B */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (B << 8);
	STATES(8);
}

INSTR(0x69, op_undoc_ldixlc)		/* LD IXL,C */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | C;
	STATES(8);
}

INSTR(0x61, op_undoc_ldixhc)		/* LD IXH,C */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (C << 8);
	STATES(8);
}

INSTR(0x6a, op_undoc_ldixld)		/* LD IXL,D */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | D;
	STATES(8);
}

INSTR(0x62, op_undoc_ldixhd)		/* LD IXH,D */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (D << 8);
	STATES(8);
}

INSTR(0x6b, op_undoc_ldixle)		/* LD IXL,E */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | E;
	STATES(8);
}

INSTR(0x63, op_undoc_ldixhe)		/* LD IXH,E */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (E << 8);
	STATES(8);
}

INSTR(0x6c, op_undoc_ldixlixh)		/* LD IXL,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | (IX >> 8);
	STATES(8);
}

INSTR(0x64, op_undoc_ldixhixh)		/* LD IXH,IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	STATES(8);
}

INSTR(0x6d, op_undoc_ldixlixl)		/* LD IXL,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	STATES(8);
}

INSTR(0x65, op_undoc_ldixhixl)		/* LD IXH,IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (IX << 8);
	STATES(8);
}

INSTR(0x26, op_undoc_ldixhn)		/* LD IXH,n */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0x00ff) | (memrdr(PC++) << 8);
	STATES(11);
}

INSTR(0x2e, op_undoc_ldixln)		/* LD IXL,n */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	IX = (IX & 0xff00) | memrdr(PC++);
	STATES(11);
}

INSTR(0xbd, op_undoc_cpixl)		/* CP IXL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX & 0xff;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xbc, op_undoc_cpixh)		/* CP IXH */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX >> 8;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x85, op_undoc_adaixl)		/* ADD A,IXL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX & 0xff;
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x84, op_undoc_adaixh)		/* ADD A,IXH */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX >> 8;
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x8d, op_undoc_acaixl)		/* ADC A,IXL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX & 0xff;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x8c, op_undoc_acaixh)		/* ADC A,IXH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX >> 8;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x95, op_undoc_suaixl)		/* SUB A,IXL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX & 0xff;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x94, op_undoc_suaixh)		/* SUB A,IXH */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX >> 8;
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x9d, op_undoc_scaixl)		/* SBC A,IXL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX & 0xff;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x9c, op_undoc_scaixh)		/* SBC A,IXH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IX >> 8;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xb5, op_undoc_oraixl)		/* OR IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A |= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xb4, op_undoc_oraixh)		/* OR IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A |= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xad, op_undoc_xorixl)		/* XOR IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A ^= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xac, op_undoc_xorixh)		/* XOR IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A ^= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xa5, op_undoc_andixl)		/* AND IXL */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A &= IX & 0xff;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0xa4, op_undoc_andixh)		/* AND IXH */
{
	if (u_flag) {
		STATES(trap_dd());
	}

	A &= IX >> 8;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(N_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2c, op_undoc_incixl)		/* INC IXL */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX & 0xff;
	P++;
	IX = (IX & 0xff00) | P;
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x24, op_undoc_incixh)		/* INC IXH */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX >> 8;
	P++;
	IX = (IX & 0x00ff) | (P << 8);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2d, op_undoc_decixl)		/* DEC IXL */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX & 0xff;
	P--;
	IX = (IX & 0xff00) | P;
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x25, op_undoc_decixh)		/* DEC IXH */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_dd());
	}

	P = IX >> 8;
	P--;
	IX = (IX & 0x00ff) | (P << 8);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

#endif /* UNDOC_INST */
