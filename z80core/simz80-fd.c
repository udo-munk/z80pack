/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 instructions
 *	beginning with the prefix 0xfd and a dispatcher for the
 *	prefix 0xfd 0xcb
 */

INSTR(0xe1, op_popiy)			/* POP IY */
{
	IY = memrdr(SP++);
	IY += memrdr(SP++) << 8;
	STATES(14);
}

INSTR(0xe5, op_pusiy)			/* PUSH IY */
{
	memwrt(--SP, IY >> 8);
	memwrt(--SP, IY);
	STATES(15);
}

INSTR(0xe9, op_jpiy)			/* JP (IY) */
{
	PC = IY;
	STATES(8);
}

INSTR(0xe3, op_exspy)			/* EX (SP),IY */
{
	register WORD i;

	i = memrdr(SP) + (memrdr(SP + 1) << 8);
	memwrt(SP, IY);
	memwrt(SP + 1, IY >> 8);
	IY = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(23);
}

INSTR(0xf9, op_ldspy)			/* LD SP,IY */
{
	SP = IY;
	STATES(10);
}

INSTR(0x21, op_ldiynn)			/* LD IY,nn */
{
	IY = memrdr(PC++);
	IY += memrdr(PC++) << 8;
	STATES(14);
}

INSTR(0x2a, op_ldiyinn)			/* LD IY,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	IY = memrdr(i++);
	IY += memrdr(i) << 8;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x22, op_ldiny)			/* LD (nn),IY */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, IY);
	memwrt(i, IY >> 8);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x86, op_adayd)			/* ADD A,(IY+d) */
{
	register int i;
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0x8e, op_acayd)			/* ADC A,(IY+d) */
{
	register int i, carry;
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0x96, op_suayd)			/* SUB A,(IY+d) */
{
	register int i;
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0x9e, op_scayd)			/* SBC A,(IY+d) */
{
	register int i, carry;
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0xa6, op_andyd)			/* AND (IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0xae, op_xoryd)			/* XOR (IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0xb6, op_oryd)			/* OR (IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0xbe, op_cpyd)			/* CP (IY+d) */
{
	register int i;
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0x34, op_incyd)			/* INC (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0x35, op_decyd)			/* DEC (IY+d) */
{
	register BYTE P;
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
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

INSTR(0x09, op_addyb)			/* ADD IY,BC */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;

#ifdef UNDOC_FLAGS
	WZ = IY + 1;
#endif
	carry = (iyl + C > 255) ? 1 : 0;
	iyl += C;
	((iyh & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(iyh + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += B + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(iyh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(iyh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x19, op_addyd)			/* ADD IY,DE */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;

#ifdef UNDOC_FLAGS
	WZ = IY + 1;
#endif
	carry = (iyl + E > 255) ? 1 : 0;
	iyl += E;
	((iyh & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(iyh + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += D + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(iyh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(iyh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x39, op_addys)			/* ADD IY,SP */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;
	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;

#ifdef UNDOC_FLAGS
	WZ = IY + 1;
#endif
	carry = (iyl + spl > 255) ? 1 : 0;
	iyl += spl;
	((iyh & 0xf) + (sph & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(iyh + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += sph + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(iyh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(iyh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x29, op_addyy)			/* ADD IY,IY */
{
	register int carry;
	BYTE iyl = IY & 0xff;
	BYTE iyh = IY >> 8;

#ifdef UNDOC_FLAGS
	WZ = IY + 1;
#endif
	carry = (iyl << 1 > 255) ? 1 : 0;
	iyl <<= 1;
	((iyh & 0xf) + (iyh & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
	(iyh + iyh + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	iyh += iyh + carry;
	IY = (iyh << 8) + iyl;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(iyh & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(iyh & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x23, op_inciy)			/* INC IY */
{
	IY++;
	STATES(10);
}

INSTR(0x2b, op_deciy)			/* DEC IY */
{
	IY--;
	STATES(10);
}

INSTR(0x7e, op_ldayd)			/* LD A,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	A = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x46, op_ldbyd)			/* LD B,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	B = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x4e, op_ldcyd)			/* LD C,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	C = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x56, op_lddyd)			/* LD D,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	D = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x5e, op_ldeyd)			/* LD E,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	E = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x66, op_ldhyd)			/* LD H,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	H = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x6e, op_ldlyd)			/* LD L,(IY+d) */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	L = memrdr(addr);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x77, op_ldyda)			/* LD (IY+d),A */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, A);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x70, op_ldydb)			/* LD (IY+d),B */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, B);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x71, op_ldydc)			/* LD (IY+d),C */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, C);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x72, op_ldydd)			/* LD (IY+d),D */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, D);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x73, op_ldyde)			/* LD (IY+d),E */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, E);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x74, op_ldydh)			/* LD (IY+d),H */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, H);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x75, op_ldydl)			/* LD (IY+d),L */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, L);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0x36, op_ldydn)			/* LD (IY+d),n */
{
	WORD addr;

	addr = IY + (signed char) memrdr(PC++);
	memwrt(addr, memrdr(PC++));
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(19);
}

INSTR(0xcb, op_fdcb_handle)		/* 0xfd 0xcb prefix */
{
#ifndef FAST_INSTR

#ifdef UNDOC_INST
#define UNDOC(f) f
#ifdef UNDOC_IALL
#define UNDOCA(f) f
#else
#define UNDOCA(f) trap_fdcb
#endif
#else
#define UNDOC(f) trap_fdcb
#define UNDOCA(f) trap_fdcb
#endif

	static int (*op_fdcb[256])(int) = {
		UNDOCA(op_undoc_rlciydb),	/* 0x00 */
		UNDOCA(op_undoc_rlciydc),	/* 0x01 */
		UNDOCA(op_undoc_rlciydd),	/* 0x02 */
		UNDOCA(op_undoc_rlciyde),	/* 0x03 */
		UNDOCA(op_undoc_rlciydh),	/* 0x04 */
		UNDOCA(op_undoc_rlciydl),	/* 0x05 */
		op_rlciyd,			/* 0x06 */
		UNDOCA(op_undoc_rlciyda),	/* 0x07 */
		UNDOCA(op_undoc_rrciydb),	/* 0x08 */
		UNDOCA(op_undoc_rrciydc),	/* 0x09 */
		UNDOCA(op_undoc_rrciydd),	/* 0x0a */
		UNDOCA(op_undoc_rrciyde),	/* 0x0b */
		UNDOCA(op_undoc_rrciydh),	/* 0x0c */
		UNDOCA(op_undoc_rrciydl),	/* 0x0d */
		op_rrciyd,			/* 0x0e */
		UNDOCA(op_undoc_rrciyda),	/* 0x0f */
		UNDOCA(op_undoc_rliydb),	/* 0x10 */
		UNDOCA(op_undoc_rliydc),	/* 0x11 */
		UNDOCA(op_undoc_rliydd),	/* 0x12 */
		UNDOCA(op_undoc_rliyde),	/* 0x13 */
		UNDOCA(op_undoc_rliydh),	/* 0x14 */
		UNDOCA(op_undoc_rliydl),	/* 0x15 */
		op_rliyd,			/* 0x16 */
		UNDOCA(op_undoc_rliyda),	/* 0x17 */
		UNDOCA(op_undoc_rriydb),	/* 0x18 */
		UNDOCA(op_undoc_rriydc),	/* 0x19 */
		UNDOCA(op_undoc_rriydd),	/* 0x1a */
		UNDOCA(op_undoc_rriyde),	/* 0x1b */
		UNDOCA(op_undoc_rriydh),	/* 0x1c */
		UNDOCA(op_undoc_rriydl),	/* 0x1d */
		op_rriyd,			/* 0x1e */
		UNDOCA(op_undoc_rriyda),	/* 0x1f */
		UNDOCA(op_undoc_slaiydb),	/* 0x20 */
		UNDOCA(op_undoc_slaiydc),	/* 0x21 */
		UNDOCA(op_undoc_slaiydd),	/* 0x22 */
		UNDOCA(op_undoc_slaiyde),	/* 0x23 */
		UNDOCA(op_undoc_slaiydh),	/* 0x24 */
		UNDOCA(op_undoc_slaiydl),	/* 0x25 */
		op_slaiyd,			/* 0x26 */
		UNDOCA(op_undoc_slaiyda),	/* 0x27 */
		UNDOCA(op_undoc_sraiydb),	/* 0x28 */
		UNDOCA(op_undoc_sraiydc),	/* 0x29 */
		UNDOCA(op_undoc_sraiydd),	/* 0x2a */
		UNDOCA(op_undoc_sraiyde),	/* 0x2b */
		UNDOCA(op_undoc_sraiydh),	/* 0x2c */
		UNDOCA(op_undoc_sraiydl),	/* 0x2d */
		op_sraiyd,			/* 0x2e */
		UNDOCA(op_undoc_sraiyda),	/* 0x2f */
		UNDOCA(op_undoc_slliydb),	/* 0x30 */
		UNDOCA(op_undoc_slliydc),	/* 0x31 */
		UNDOCA(op_undoc_slliydd),	/* 0x32 */
		UNDOCA(op_undoc_slliyde),	/* 0x33 */
		UNDOCA(op_undoc_slliydh),	/* 0x34 */
		UNDOCA(op_undoc_slliydl),	/* 0x35 */
		UNDOC(op_undoc_slliyd),		/* 0x36 */
		UNDOCA(op_undoc_slliyda),	/* 0x37 */
		UNDOCA(op_undoc_srliydb),	/* 0x38 */
		UNDOCA(op_undoc_srliydc),	/* 0x39 */
		UNDOCA(op_undoc_srliydd),	/* 0x3a */
		UNDOCA(op_undoc_srliyde),	/* 0x3b */
		UNDOCA(op_undoc_srliydh),	/* 0x3c */
		UNDOCA(op_undoc_srliydl),	/* 0x3d */
		op_srliyd,			/* 0x3e */
		UNDOCA(op_undoc_srliyda),	/* 0x3f */
		UNDOCA(op_undoc_tb0iyd),	/* 0x40 */
		UNDOCA(op_undoc_tb0iyd),	/* 0x41 */
		UNDOCA(op_undoc_tb0iyd),	/* 0x42 */
		UNDOCA(op_undoc_tb0iyd),	/* 0x43 */
		UNDOCA(op_undoc_tb0iyd),	/* 0x44 */
		UNDOCA(op_undoc_tb0iyd),	/* 0x45 */
		op_tb0iyd,			/* 0x46 */
		UNDOCA(op_undoc_tb0iyd),	/* 0x47 */
		UNDOCA(op_undoc_tb1iyd),	/* 0x48 */
		UNDOCA(op_undoc_tb1iyd),	/* 0x49 */
		UNDOCA(op_undoc_tb1iyd),	/* 0x4a */
		UNDOCA(op_undoc_tb1iyd),	/* 0x4b */
		UNDOCA(op_undoc_tb1iyd),	/* 0x4c */
		UNDOCA(op_undoc_tb1iyd),	/* 0x4d */
		op_tb1iyd,			/* 0x4e */
		UNDOCA(op_undoc_tb1iyd),	/* 0x4f */
		UNDOCA(op_undoc_tb2iyd),	/* 0x50 */
		UNDOCA(op_undoc_tb2iyd),	/* 0x51 */
		UNDOCA(op_undoc_tb2iyd),	/* 0x52 */
		UNDOCA(op_undoc_tb2iyd),	/* 0x53 */
		UNDOCA(op_undoc_tb2iyd),	/* 0x54 */
		UNDOCA(op_undoc_tb2iyd),	/* 0x55 */
		op_tb2iyd,			/* 0x56 */
		UNDOCA(op_undoc_tb2iyd),	/* 0x57 */
		UNDOCA(op_undoc_tb3iyd),	/* 0x58 */
		UNDOCA(op_undoc_tb3iyd),	/* 0x59 */
		UNDOCA(op_undoc_tb3iyd),	/* 0x5a */
		UNDOCA(op_undoc_tb3iyd),	/* 0x5b */
		UNDOCA(op_undoc_tb3iyd),	/* 0x5c */
		UNDOCA(op_undoc_tb3iyd),	/* 0x5d */
		op_tb3iyd,			/* 0x5e */
		UNDOCA(op_undoc_tb3iyd),	/* 0x5f */
		UNDOCA(op_undoc_tb4iyd),	/* 0x60 */
		UNDOCA(op_undoc_tb4iyd),	/* 0x61 */
		UNDOCA(op_undoc_tb4iyd),	/* 0x62 */
		UNDOCA(op_undoc_tb4iyd),	/* 0x63 */
		UNDOCA(op_undoc_tb4iyd),	/* 0x64 */
		UNDOCA(op_undoc_tb4iyd),	/* 0x65 */
		op_tb4iyd,			/* 0x66 */
		UNDOCA(op_undoc_tb4iyd),	/* 0x67 */
		UNDOCA(op_undoc_tb5iyd),	/* 0x68 */
		UNDOCA(op_undoc_tb5iyd),	/* 0x69 */
		UNDOCA(op_undoc_tb5iyd),	/* 0x6a */
		UNDOCA(op_undoc_tb5iyd),	/* 0x6b */
		UNDOCA(op_undoc_tb5iyd),	/* 0x6c */
		UNDOCA(op_undoc_tb5iyd),	/* 0x6d */
		op_tb5iyd,			/* 0x6e */
		UNDOCA(op_undoc_tb5iyd),	/* 0x6f */
		UNDOCA(op_undoc_tb6iyd),	/* 0x70 */
		UNDOCA(op_undoc_tb6iyd),	/* 0x71 */
		UNDOCA(op_undoc_tb6iyd),	/* 0x72 */
		UNDOCA(op_undoc_tb6iyd),	/* 0x73 */
		UNDOCA(op_undoc_tb6iyd),	/* 0x74 */
		UNDOCA(op_undoc_tb6iyd),	/* 0x75 */
		op_tb6iyd,			/* 0x76 */
		UNDOCA(op_undoc_tb6iyd),	/* 0x77 */
		UNDOCA(op_undoc_tb7iyd),	/* 0x78 */
		UNDOCA(op_undoc_tb7iyd),	/* 0x79 */
		UNDOCA(op_undoc_tb7iyd),	/* 0x7a */
		UNDOCA(op_undoc_tb7iyd),	/* 0x7b */
		UNDOCA(op_undoc_tb7iyd),	/* 0x7c */
		UNDOCA(op_undoc_tb7iyd),	/* 0x7d */
		op_tb7iyd,			/* 0x7e */
		UNDOCA(op_undoc_tb7iyd),	/* 0x7f */
		UNDOCA(op_undoc_rb0iydb),	/* 0x80 */
		UNDOCA(op_undoc_rb0iydc),	/* 0x81 */
		UNDOCA(op_undoc_rb0iydd),	/* 0x82 */
		UNDOCA(op_undoc_rb0iyde),	/* 0x83 */
		UNDOCA(op_undoc_rb0iydh),	/* 0x84 */
		UNDOCA(op_undoc_rb0iydl),	/* 0x85 */
		op_rb0iyd,			/* 0x86 */
		UNDOCA(op_undoc_rb0iyda),	/* 0x87 */
		UNDOCA(op_undoc_rb1iydb),	/* 0x88 */
		UNDOCA(op_undoc_rb1iydc),	/* 0x89 */
		UNDOCA(op_undoc_rb1iydd),	/* 0x8a */
		UNDOCA(op_undoc_rb1iyde),	/* 0x8b */
		UNDOCA(op_undoc_rb1iydh),	/* 0x8c */
		UNDOCA(op_undoc_rb1iydl),	/* 0x8d */
		op_rb1iyd,			/* 0x8e */
		UNDOCA(op_undoc_rb1iyda),	/* 0x8f */
		UNDOCA(op_undoc_rb2iydb),	/* 0x90 */
		UNDOCA(op_undoc_rb2iydc),	/* 0x91 */
		UNDOCA(op_undoc_rb2iydd),	/* 0x92 */
		UNDOCA(op_undoc_rb2iyde),	/* 0x93 */
		UNDOCA(op_undoc_rb2iydh),	/* 0x94 */
		UNDOCA(op_undoc_rb2iydl),	/* 0x95 */
		op_rb2iyd,			/* 0x96 */
		UNDOCA(op_undoc_rb2iyda),	/* 0x97 */
		UNDOCA(op_undoc_rb3iydb),	/* 0x98 */
		UNDOCA(op_undoc_rb3iydc),	/* 0x99 */
		UNDOCA(op_undoc_rb3iydd),	/* 0x9a */
		UNDOCA(op_undoc_rb3iyde),	/* 0x9b */
		UNDOCA(op_undoc_rb3iydh),	/* 0x9c */
		UNDOCA(op_undoc_rb3iydl),	/* 0x9d */
		op_rb3iyd,			/* 0x9e */
		UNDOCA(op_undoc_rb3iyda),	/* 0x9f */
		UNDOCA(op_undoc_rb4iydb),	/* 0xa0 */
		UNDOCA(op_undoc_rb4iydc),	/* 0xa1 */
		UNDOCA(op_undoc_rb4iydd),	/* 0xa2 */
		UNDOCA(op_undoc_rb4iyde),	/* 0xa3 */
		UNDOCA(op_undoc_rb4iydh),	/* 0xa4 */
		UNDOCA(op_undoc_rb4iydl),	/* 0xa5 */
		op_rb4iyd,			/* 0xa6 */
		UNDOCA(op_undoc_rb4iyda),	/* 0xa7 */
		UNDOCA(op_undoc_rb5iydb),	/* 0xa8 */
		UNDOCA(op_undoc_rb5iydc),	/* 0xa9 */
		UNDOCA(op_undoc_rb5iydd),	/* 0xaa */
		UNDOCA(op_undoc_rb5iyde),	/* 0xab */
		UNDOCA(op_undoc_rb5iydh),	/* 0xac */
		UNDOCA(op_undoc_rb5iydl),	/* 0xad */
		op_rb5iyd,			/* 0xae */
		UNDOCA(op_undoc_rb5iyda),	/* 0xaf */
		UNDOCA(op_undoc_rb6iydb),	/* 0xb0 */
		UNDOCA(op_undoc_rb6iydc),	/* 0xb1 */
		UNDOCA(op_undoc_rb6iydd),	/* 0xb2 */
		UNDOCA(op_undoc_rb6iyde),	/* 0xb3 */
		UNDOCA(op_undoc_rb6iydh),	/* 0xb4 */
		UNDOCA(op_undoc_rb6iydl),	/* 0xb5 */
		op_rb6iyd,			/* 0xb6 */
		UNDOCA(op_undoc_rb6iyda),	/* 0xb7 */
		UNDOCA(op_undoc_rb7iydb),	/* 0xb8 */
		UNDOCA(op_undoc_rb7iydc),	/* 0xb9 */
		UNDOCA(op_undoc_rb7iydd),	/* 0xba */
		UNDOCA(op_undoc_rb7iyde),	/* 0xbb */
		UNDOCA(op_undoc_rb7iydh),	/* 0xbc */
		UNDOCA(op_undoc_rb7iydl),	/* 0xbd */
		op_rb7iyd,			/* 0xbe */
		UNDOCA(op_undoc_rb7iyda),	/* 0xbf */
		UNDOCA(op_undoc_sb0iydb),	/* 0xc0 */
		UNDOCA(op_undoc_sb0iydc),	/* 0xc1 */
		UNDOCA(op_undoc_sb0iydd),	/* 0xc2 */
		UNDOCA(op_undoc_sb0iyde),	/* 0xc3 */
		UNDOCA(op_undoc_sb0iydh),	/* 0xc4 */
		UNDOCA(op_undoc_sb0iydl),	/* 0xc5 */
		op_sb0iyd,			/* 0xc6 */
		UNDOCA(op_undoc_sb0iyda),	/* 0xc7 */
		UNDOCA(op_undoc_sb1iydb),	/* 0xc8 */
		UNDOCA(op_undoc_sb1iydc),	/* 0xc9 */
		UNDOCA(op_undoc_sb1iydd),	/* 0xca */
		UNDOCA(op_undoc_sb1iyde),	/* 0xcb */
		UNDOCA(op_undoc_sb1iydh),	/* 0xcc */
		UNDOCA(op_undoc_sb1iydl),	/* 0xcd */
		op_sb1iyd,			/* 0xce */
		UNDOCA(op_undoc_sb1iyda),	/* 0xcf */
		UNDOCA(op_undoc_sb2iydb),	/* 0xd0 */
		UNDOCA(op_undoc_sb2iydc),	/* 0xd1 */
		UNDOCA(op_undoc_sb2iydd),	/* 0xd2 */
		UNDOCA(op_undoc_sb2iyde),	/* 0xd3 */
		UNDOCA(op_undoc_sb2iydh),	/* 0xd4 */
		UNDOCA(op_undoc_sb2iydl),	/* 0xd5 */
		op_sb2iyd,			/* 0xd6 */
		UNDOCA(op_undoc_sb2iyda),	/* 0xd7 */
		UNDOCA(op_undoc_sb3iydb),	/* 0xd8 */
		UNDOCA(op_undoc_sb3iydc),	/* 0xd9 */
		UNDOCA(op_undoc_sb3iydd),	/* 0xda */
		UNDOCA(op_undoc_sb3iyde),	/* 0xdb */
		UNDOCA(op_undoc_sb3iydh),	/* 0xdc */
		UNDOCA(op_undoc_sb3iydl),	/* 0xdd */
		op_sb3iyd,			/* 0xde */
		UNDOCA(op_undoc_sb3iyda),	/* 0xdf */
		UNDOCA(op_undoc_sb4iydb),	/* 0xe0 */
		UNDOCA(op_undoc_sb4iydc),	/* 0xe1 */
		UNDOCA(op_undoc_sb4iydd),	/* 0xe2 */
		UNDOCA(op_undoc_sb4iyde),	/* 0xe3 */
		UNDOCA(op_undoc_sb4iydh),	/* 0xe4 */
		UNDOCA(op_undoc_sb4iydl),	/* 0xe5 */
		op_sb4iyd,			/* 0xe6 */
		UNDOCA(op_undoc_sb4iyda),	/* 0xe7 */
		UNDOCA(op_undoc_sb5iydb),	/* 0xe8 */
		UNDOCA(op_undoc_sb5iydc),	/* 0xe9 */
		UNDOCA(op_undoc_sb5iydd),	/* 0xea */
		UNDOCA(op_undoc_sb5iyde),	/* 0xeb */
		UNDOCA(op_undoc_sb5iydh),	/* 0xec */
		UNDOCA(op_undoc_sb5iydl),	/* 0xed */
		op_sb5iyd,			/* 0xee */
		UNDOCA(op_undoc_sb5iyda),	/* 0xef */
		UNDOCA(op_undoc_sb6iydb),	/* 0xf0 */
		UNDOCA(op_undoc_sb6iydc),	/* 0xf1 */
		UNDOCA(op_undoc_sb6iydd),	/* 0xf2 */
		UNDOCA(op_undoc_sb6iyde),	/* 0xf3 */
		UNDOCA(op_undoc_sb6iydh),	/* 0xf4 */
		UNDOCA(op_undoc_sb6iydl),	/* 0xf5 */
		op_sb6iyd,			/* 0xf6 */
		UNDOCA(op_undoc_sb6iyda),	/* 0xf7 */
		UNDOCA(op_undoc_sb7iydb),	/* 0xf8 */
		UNDOCA(op_undoc_sb7iydc),	/* 0xf9 */
		UNDOCA(op_undoc_sb7iydd),	/* 0xfa */
		UNDOCA(op_undoc_sb7iyde),	/* 0xfb */
		UNDOCA(op_undoc_sb7iydh),	/* 0xfc */
		UNDOCA(op_undoc_sb7iydl),	/* 0xfd */
		op_sb7iyd,			/* 0xfe */
		UNDOCA(op_undoc_sb7iyda)	/* 0xff */
	};

#undef UNDOC
#undef UNDOCA

	register int t;

#endif /* !FAST_INSTR */

	register int data;

	data = (signed char) memrdr(PC++);

#ifndef FAST_INSTR
	t = (*op_fdcb[memrdr(PC++)])(data); /* execute next opcode */
#else
	switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-fdcb.c"

	default:
		t = trap_fdcb(data);
		break;
	}
#endif

	STATES(t);
}

#ifndef FAST_INSTR
#include "simz80-fdcb.c"
#endif

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

INSTR(0x7d, op_undoc_ldaiyl)		/* LD A,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A = IY & 0xff;
	STATES(8);
}

INSTR(0x7c, op_undoc_ldaiyh)		/* LD A,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A = IY >> 8;
	STATES(8);
}

INSTR(0x45, op_undoc_ldbiyl)		/* LD B,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	B = IY & 0xff;
	STATES(8);
}

INSTR(0x44, op_undoc_ldbiyh)		/* LD B,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	B = IY >> 8;
	STATES(8);
}

INSTR(0x4d, op_undoc_ldciyl)		/* LD C,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	C = IY & 0xff;
	STATES(8);
}

INSTR(0x4c, op_undoc_ldciyh)		/* LD C,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	C = IY >> 8;
	STATES(8);
}

INSTR(0x55, op_undoc_lddiyl)		/* LD D,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	D = IY & 0xff;
	STATES(8);
}

INSTR(0x54, op_undoc_lddiyh)		/* LD D,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	D = IY >> 8;
	STATES(8);
}

INSTR(0x5d, op_undoc_ldeiyl)		/* LD E,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	E = IY & 0xff;
	STATES(8);
}

INSTR(0x5c, op_undoc_ldeiyh)		/* LD E,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	E = IY >> 8;
	STATES(8);
}

INSTR(0x6f, op_undoc_ldiyla)		/* LD IYL,A */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | A;
	STATES(8);
}

INSTR(0x67, op_undoc_ldiyha)		/* LD IYH,A */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (A << 8);
	STATES(8);
}

INSTR(0x68, op_undoc_ldiylb)		/* LD IYL,B */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | B;
	STATES(8);
}

INSTR(0x60, op_undoc_ldiyhb)		/* LD IYH,B */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (B << 8);
	STATES(8);
}

INSTR(0x69, op_undoc_ldiylc)		/* LD IYL,C */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | C;
	STATES(8);
}

INSTR(0x61, op_undoc_ldiyhc)		/* LD IYH,C */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (C << 8);
	STATES(8);
}

INSTR(0x6a, op_undoc_ldiyld)		/* LD IYL,D */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | D;
	STATES(8);
}

INSTR(0x62, op_undoc_ldiyhd)		/* LD IYH,D */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (D << 8);
	STATES(8);
}

INSTR(0x6b, op_undoc_ldiyle)		/* LD IYL,E */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | E;
	STATES(8);
}

INSTR(0x63, op_undoc_ldiyhe)		/* LD IYH,E */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (E << 8);
	STATES(8);
}

INSTR(0x6c, op_undoc_ldiyliyh)		/* LD IYL,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | (IY >> 8);
	STATES(8);
}

INSTR(0x64, op_undoc_ldiyhiyh)		/* LD IYH,IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	STATES(8);
}

INSTR(0x6d, op_undoc_ldiyliyl)		/* LD IYL,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	STATES(8);
}

INSTR(0x65, op_undoc_ldiyhiyl)		/* LD IYH,IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (IY << 8);
	STATES(8);
}

INSTR(0x26, op_undoc_ldiyhn)		/* LD IYH,n */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0x00ff) | (memrdr(PC++) << 8);
	STATES(11);
}

INSTR(0x2e, op_undoc_ldiyln)		/* LD IYL,n */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	IY = (IY & 0xff00) | memrdr(PC++);
	STATES(11);
}

INSTR(0xbd, op_undoc_cpiyl)		/* CP IYL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY & 0xff;
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

INSTR(0xbc, op_undoc_cpiyh)		/* CP IYH */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY >> 8;
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

INSTR(0x85, op_undoc_adaiyl)		/* ADD A,IYL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY & 0xff;
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

INSTR(0x84, op_undoc_adaiyh)		/* ADD A,IYH */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY >> 8;
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

INSTR(0x8d, op_undoc_acaiyl)		/* ADC A,IYL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY & 0xff;
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

INSTR(0x8c, op_undoc_acaiyh)		/* ADC A,IYH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY >> 8;
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

INSTR(0x95, op_undoc_suaiyl)		/* SUB A,IYL */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY & 0xff;
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

INSTR(0x94, op_undoc_suaiyh)		/* SUB A,IYH */
{
	register int i;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY >> 8;
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

INSTR(0x9d, op_undoc_scaiyl)		/* SBC A,IYL */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY & 0xff;
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

INSTR(0x9c, op_undoc_scaiyh)		/* SBC A,IYH */
{
	register int i, carry;
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	carry = (F & C_FLAG) ? 1 : 0;
	P = IY >> 8;
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

INSTR(0xb5, op_undoc_oraiyl)		/* OR IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A |= IY & 0xff;
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

INSTR(0xb4, op_undoc_oraiyh)		/* OR IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A |= IY >> 8;
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

INSTR(0xad, op_undoc_xoriyl)		/* XOR IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A ^= IY & 0xff;
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

INSTR(0xac, op_undoc_xoriyh)		/* XOR IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A ^= IY >> 8;
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

INSTR(0xa5, op_undoc_andiyl)		/* AND IYL */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A &= IY & 0xff;
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

INSTR(0xa4, op_undoc_andiyh)		/* AND IYH */
{
	if (u_flag) {
		STATES(trap_fd());
	}

	A &= IY >> 8;
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

INSTR(0x2c, op_undoc_inciyl)		/* INC IYL */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY & 0xff;
	P++;
	IY = (IY & 0xff00) | P;
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

INSTR(0x24, op_undoc_inciyh)		/* INC IYH */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY >> 8;
	P++;
	IY = (IY & 0x00ff) | (P << 8);
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

INSTR(0x2d, op_undoc_deciyl)		/* DEC IYL */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY & 0xff;
	P--;
	IY = (IY & 0xff00) | P;
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

INSTR(0x25, op_undoc_deciyh)		/* DEC IYH */
{
	register BYTE P;

	if (u_flag) {
		STATES(trap_fd());
	}

	P = IY >> 8;
	P--;
	IY = (IY & 0x00ff) | (P << 8);
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
