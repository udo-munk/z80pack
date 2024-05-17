/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 instructions
 *	beginning with the prefix 0xcb
 */

INSTR(0x3f, op_srla)			/* SRL A */
{
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x38, op_srlb)			/* SRL B */
{
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x39, op_srlc)			/* SRL C */
{
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x3a, op_srld)			/* SRL D */
{
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x3b, op_srle)			/* SRL E */
{
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x3c, op_srlh)			/* SRL H */
{
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x3d, op_srll)			/* SRL L */
{
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x3e, op_srlhl)			/* SRL (HL) */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	(P & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P >>= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x27, op_slaa)			/* SLA A */
{
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x20, op_slab)			/* SLA B */
{
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x21, op_slac)			/* SLA C */
{
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x22, op_slad)			/* SLA D */
{
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x23, op_slae)			/* SLA E */
{
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x24, op_slah)			/* SLA H */
{
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x25, op_slal)			/* SLA L */
{
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x26, op_slahl)			/* SLA (HL) */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P <<= 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x17, op_rlra)			/* RL A */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x10, op_rlb)			/* RL B */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B <<= 1;
	if (old_c_flag) B |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x11, op_rlc)			/* RL C */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C <<= 1;
	if (old_c_flag) C |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x12, op_rld)			/* RL D */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D <<= 1;
	if (old_c_flag) D |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x13, op_rle)			/* RL E */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E <<= 1;
	if (old_c_flag) E |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x14, op_rlh)			/* RL H */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H <<= 1;
	if (old_c_flag) H |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x15, op_rll)			/* RL L */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L <<= 1;
	if (old_c_flag) L |= 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x16, op_rlhl)			/* RL (HL) */
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
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x1f, op_rrra)			/* RR A */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x18, op_rrb)			/* RR B */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	if (old_c_flag) B |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x19, op_rrc)			/* RR C */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	if (old_c_flag) C |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x1a, op_rrd)			/* RR D */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	if (old_c_flag) D |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x1b, op_rre)			/* RR E */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	if (old_c_flag) E |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x1c, op_rrh)			/* RR H */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	if (old_c_flag) H |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x1d, op_rrl)			/* RR L */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	if (old_c_flag) L |= 128;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x1e, op_rrhl)			/* RR (HL) */
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
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x0f, op_rrcra)			/* RRC A */
{
	register int i;

	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A >>= 1;
	if (i) A |= 128;
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x08, op_rrcb)			/* RRC B */
{
	register int i;

	i = B & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	B >>= 1;
	if (i) B |= 128;
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x09, op_rrcc)			/* RRC C */
{
	register int i;

	i = C & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	C >>= 1;
	if (i) C |= 128;
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x0a, op_rrcd)			/* RRC D */
{
	register int i;

	i = D & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	D >>= 1;
	if (i) D |= 128;
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x0b, op_rrce)			/* RRC E */
{
	register int i;

	i = E & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	E >>= 1;
	if (i) E |= 128;
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x0c, op_rrch)			/* RRC H */
{
	register int i;

	i = H & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	H >>= 1;
	if (i) H |= 128;
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x0d, op_rrcl)			/* RRC L */
{
	register int i;

	i = L & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	L >>= 1;
	if (i) L |= 128;
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x0e, op_rrchl)			/* RRC (HL) */
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
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x07, op_rlcra)			/* RLC A */
{
	register int i;

	i = A & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A <<= 1;
	if (i) A |= 1;
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x00, op_rlcb)			/* RLC B */
{
	register int i;

	i = B & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	B <<= 1;
	if (i) B |= 1;
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x01, op_rlcc)			/* RLC C */
{
	register int i;

	i = C & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	C <<= 1;
	if (i) C |= 1;
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x02, op_rlcd)			/* RLC D */
{
	register int i;

	i = D & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	D <<= 1;
	if (i) D |= 1;
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x03, op_rlce)			/* RLC E */
{
	register int i;

	i = E & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	E <<= 1;
	if (i) E |= 1;
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x04, op_rlch)			/* RLC H */
{
	register int i;

	i = H & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	H <<= 1;
	if (i) H |= 1;
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x05, op_rlcl)			/* RLC L */
{
	register int i;

	i = L & 128;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	L <<= 1;
	if (i) L |= 1;
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x06, op_rlchl)			/* RLC (HL) */
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
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x2f, op_sraa)			/* SRA A */
{
	register int i;

	i = A & 128;
	(A & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	A |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x28, op_srab)			/* SRA B */
{
	register int i;

	i = B & 128;
	(B & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B >>= 1;
	B |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x29, op_srac)			/* SRA C */
{
	register int i;

	i = C & 128;
	(C & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C >>= 1;
	C |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2a, op_srad)			/* SRA D */
{
	register int i;

	i = D & 128;
	(D & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D >>= 1;
	D |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2b, op_srae)			/* SRA E */
{
	register int i;

	i = E & 128;
	(E & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E >>= 1;
	E |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2c, op_srah)			/* SRA H */
{
	register int i;

	i = H & 128;
	(H & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H >>= 1;
	H |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2d, op_sral)			/* SRA L */
{
	register int i;

	i = L & 128;
	(L & 1) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L >>= 1;
	L |= i;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x2e, op_srahl)			/* SRA (HL) */
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
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

INSTR(0xc7, op_sb0a)			/* SET 0,A */
{
	A |= 1;
	STATES(8);
}

INSTR(0xcf, op_sb1a)			/* SET 1,A */
{
	A |= 2;
	STATES(8);
}

INSTR(0xd7, op_sb2a)			/* SET 2,A */
{
	A |= 4;
	STATES(8);
}

INSTR(0xdf, op_sb3a)			/* SET 3,A */
{
	A |= 8;
	STATES(8);
}

INSTR(0xe7, op_sb4a)			/* SET 4,A */
{
	A |= 16;
	STATES(8);
}

INSTR(0xef, op_sb5a)			/* SET 5,A */
{
	A |= 32;
	STATES(8);
}

INSTR(0xf7, op_sb6a)			/* SET 6,A */
{
	A |= 64;
	STATES(8);
}

INSTR(0xff, op_sb7a)			/* SET 7,A */
{
	A |= 128;
	STATES(8);
}

INSTR(0xc0, op_sb0b)			/* SET 0,B */
{
	B |= 1;
	STATES(8);
}

INSTR(0xc8, op_sb1b)			/* SET 1,B */
{
	B |= 2;
	STATES(8);
}

INSTR(0xd0, op_sb2b)			/* SET 2,B */
{
	B |= 4;
	STATES(8);
}

INSTR(0xd8, op_sb3b)			/* SET 3,B */
{
	B |= 8;
	STATES(8);
}

INSTR(0xe0, op_sb4b)			/* SET 4,B */
{
	B |= 16;
	STATES(8);
}

INSTR(0xe8, op_sb5b)			/* SET 5,B */
{
	B |= 32;
	STATES(8);
}

INSTR(0xf0, op_sb6b)			/* SET 6,B */
{
	B |= 64;
	STATES(8);
}

INSTR(0xf8, op_sb7b)			/* SET 7,B */
{
	B |= 128;
	STATES(8);
}

INSTR(0xc1, op_sb0c)			/* SET 0,C */
{
	C |= 1;
	STATES(8);
}

INSTR(0xc9, op_sb1c)			/* SET 1,C */
{
	C |= 2;
	STATES(8);
}

INSTR(0xd1, op_sb2c)			/* SET 2,C */
{
	C |= 4;
	STATES(8);
}

INSTR(0xd9, op_sb3c)			/* SET 3,C */
{
	C |= 8;
	STATES(8);
}

INSTR(0xe1, op_sb4c)			/* SET 4,C */
{
	C |= 16;
	STATES(8);
}

INSTR(0xe9, op_sb5c)			/* SET 5,C */
{
	C |= 32;
	STATES(8);
}

INSTR(0xf1, op_sb6c)			/* SET 6,C */
{
	C |= 64;
	STATES(8);
}

INSTR(0xf9, op_sb7c)			/* SET 7,C */
{
	C |= 128;
	STATES(8);
}

INSTR(0xc2, op_sb0d)			/* SET 0,D */
{
	D |= 1;
	STATES(8);
}

INSTR(0xca, op_sb1d)			/* SET 1,D */
{
	D |= 2;
	STATES(8);
}

INSTR(0xd2, op_sb2d)			/* SET 2,D */
{
	D |= 4;
	STATES(8);
}

INSTR(0xda, op_sb3d)			/* SET 3,D */
{
	D |= 8;
	STATES(8);
}

INSTR(0xe2, op_sb4d)			/* SET 4,D */
{
	D |= 16;
	STATES(8);
}

INSTR(0xea, op_sb5d)			/* SET 5,D */
{
	D |= 32;
	STATES(8);
}

INSTR(0xf2, op_sb6d)			/* SET 6,D */
{
	D |= 64;
	STATES(8);
}

INSTR(0xfa, op_sb7d)			/* SET 7,D */
{
	D |= 128;
	STATES(8);
}

INSTR(0xc3, op_sb0e)			/* SET 0,E */
{
	E |= 1;
	STATES(8);
}

INSTR(0xcb, op_sb1e)			/* SET 1,E */
{
	E |= 2;
	STATES(8);
}

INSTR(0xd3, op_sb2e)			/* SET 2,E */
{
	E |= 4;
	STATES(8);
}

INSTR(0xdb, op_sb3e)			/* SET 3,E */
{
	E |= 8;
	STATES(8);
}

INSTR(0xe3, op_sb4e)			/* SET 4,E */
{
	E |= 16;
	STATES(8);
}

INSTR(0xeb, op_sb5e)			/* SET 5,E */
{
	E |= 32;
	STATES(8);
}

INSTR(0xf3, op_sb6e)			/* SET 6,E */
{
	E |= 64;
	STATES(8);
}

INSTR(0xfb, op_sb7e)			/* SET 7,E */
{
	E |= 128;
	STATES(8);
}

INSTR(0xc4, op_sb0h)			/* SET 0,H */
{
	H |= 1;
	STATES(8);
}

INSTR(0xcc, op_sb1h)			/* SET 1,H */
{
	H |= 2;
	STATES(8);
}

INSTR(0xd4, op_sb2h)			/* SET 2,H */
{
	H |= 4;
	STATES(8);
}

INSTR(0xdc, op_sb3h)			/* SET 3,H */
{
	H |= 8;
	STATES(8);
}

INSTR(0xe4, op_sb4h)			/* SET 4,H */
{
	H |= 16;
	STATES(8);
}

INSTR(0xec, op_sb5h)			/* SET 5,H */
{
	H |= 32;
	STATES(8);
}

INSTR(0xf4, op_sb6h)			/* SET 6,H */
{
	H |= 64;
	STATES(8);
}

INSTR(0xfc, op_sb7h)			/* SET 7,H */
{
	H |= 128;
	STATES(8);
}

INSTR(0xc5, op_sb0l)			/* SET 0,L */
{
	L |= 1;
	STATES(8);
}

INSTR(0xcd, op_sb1l)			/* SET 1,L */
{
	L |= 2;
	STATES(8);
}

INSTR(0xd5, op_sb2l)			/* SET 2,L */
{
	L |= 4;
	STATES(8);
}

INSTR(0xdd, op_sb3l)			/* SET 3,L */
{
	L |= 8;
	STATES(8);
}

INSTR(0xe5, op_sb4l)			/* SET 4,L */
{
	L |= 16;
	STATES(8);
}

INSTR(0xed, op_sb5l)			/* SET 5,L */
{
	L |= 32;
	STATES(8);
}

INSTR(0xf5, op_sb6l)			/* SET 6,L */
{
	L |= 64;
	STATES(8);
}

INSTR(0xfd, op_sb7l)			/* SET 7,L */
{
	L |= 128;
	STATES(8);
}

INSTR(0xc6, op_sb0hl)			/* SET 0,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 1);
	STATES(15);
}

INSTR(0xce, op_sb1hl)			/* SET 1,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 2);
	STATES(15);
}

INSTR(0xd6, op_sb2hl)			/* SET 2,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 4);
	STATES(15);
}

INSTR(0xde, op_sb3hl)			/* SET 3,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 8);
	STATES(15);
}

INSTR(0xe6, op_sb4hl)			/* SET 4,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 16);
	STATES(15);
}

INSTR(0xee, op_sb5hl)			/* SET 5,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 32);
	STATES(15);
}

INSTR(0xf6, op_sb6hl)			/* SET 6,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 64);
	STATES(15);
}

INSTR(0xfe, op_sb7hl)			/* SET 7,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) | 128);
	STATES(15);
}

INSTR(0x87, op_rb0a)			/* RES 0,A */
{
	A &= ~1;
	STATES(8);
}

INSTR(0x8f, op_rb1a)			/* RES 1,A */
{
	A &= ~2;
	STATES(8);
}

INSTR(0x97, op_rb2a)			/* RES 2,A */
{
	A &= ~4;
	STATES(8);
}

INSTR(0x9f, op_rb3a)			/* RES 3,A */
{
	A &= ~8;
	STATES(8);
}

INSTR(0xa7, op_rb4a)			/* RES 4,A */
{
	A &= ~16;
	STATES(8);
}

INSTR(0xaf, op_rb5a)			/* RES 5,A */
{
	A &= ~32;
	STATES(8);
}

INSTR(0xb7, op_rb6a)			/* RES 6,A */
{
	A &= ~64;
	STATES(8);
}

INSTR(0xbf, op_rb7a)			/* RES 7,A */
{
	A &= ~128;
	STATES(8);
}

INSTR(0x80, op_rb0b)			/* RES 0,B */
{
	B &= ~1;
	STATES(8);
}

INSTR(0x88, op_rb1b)			/* RES 1,B */
{
	B &= ~2;
	STATES(8);
}

INSTR(0x90, op_rb2b)			/* RES 2,B */
{
	B &= ~4;
	STATES(8);
}

INSTR(0x98, op_rb3b)			/* RES 3,B */
{
	B &= ~8;
	STATES(8);
}

INSTR(0xa0, op_rb4b)			/* RES 4,B */
{
	B &= ~16;
	STATES(8);
}

INSTR(0xa8, op_rb5b)			/* RES 5,B */
{
	B &= ~32;
	STATES(8);
}

INSTR(0xb0, op_rb6b)			/* RES 6,B */
{
	B &= ~64;
	STATES(8);
}

INSTR(0xb8, op_rb7b)			/* RES 7,B */
{
	B &= ~128;
	STATES(8);
}

INSTR(0x81, op_rb0c)			/* RES 0,C */
{
	C &= ~1;
	STATES(8);
}

INSTR(0x89, op_rb1c)			/* RES 1,C */
{
	C &= ~2;
	STATES(8);
}

INSTR(0x91, op_rb2c)			/* RES 2,C */
{
	C &= ~4;
	STATES(8);
}

INSTR(0x99, op_rb3c)			/* RES 3,C */
{
	C &= ~8;
	STATES(8);
}

INSTR(0xa1, op_rb4c)			/* RES 4,C */
{
	C &= ~16;
	STATES(8);
}

INSTR(0xa9, op_rb5c)			/* RES 5,C */
{
	C &= ~32;
	STATES(8);
}

INSTR(0xb1, op_rb6c)			/* RES 6,C */
{
	C &= ~64;
	STATES(8);
}

INSTR(0xb9, op_rb7c)			/* RES 7,C */
{
	C &= ~128;
	STATES(8);
}

INSTR(0x82, op_rb0d)			/* RES 0,D */
{
	D &= ~1;
	STATES(8);
}

INSTR(0x8a, op_rb1d)			/* RES 1,D */
{
	D &= ~2;
	STATES(8);
}

INSTR(0x92, op_rb2d)			/* RES 2,D */
{
	D &= ~4;
	STATES(8);
}

INSTR(0x9a, op_rb3d)			/* RES 3,D */
{
	D &= ~8;
	STATES(8);
}

INSTR(0xa2, op_rb4d)			/* RES 4,D */
{
	D &= ~16;
	STATES(8);
}

INSTR(0xaa, op_rb5d)			/* RES 5,D */
{
	D &= ~32;
	STATES(8);
}

INSTR(0xb2, op_rb6d)			/* RES 6,D */
{
	D &= ~64;
	STATES(8);
}

INSTR(0xba, op_rb7d)			/* RES 7,D */
{
	D &= ~128;
	STATES(8);
}

INSTR(0x83, op_rb0e)			/* RES 0,E */
{
	E &= ~1;
	STATES(8);
}

INSTR(0x8b, op_rb1e)			/* RES 1,E */
{
	E &= ~2;
	STATES(8);
}

INSTR(0x93, op_rb2e)			/* RES 2,E */
{
	E &= ~4;
	STATES(8);
}

INSTR(0x9b, op_rb3e)			/* RES 3,E */
{
	E &= ~8;
	STATES(8);
}

INSTR(0xa3, op_rb4e)			/* RES 4,E */
{
	E &= ~16;
	STATES(8);
}

INSTR(0xab, op_rb5e)			/* RES 5,E */
{
	E &= ~32;
	STATES(8);
}

INSTR(0xb3, op_rb6e)			/* RES 6,E */
{
	E &= ~64;
	STATES(8);
}

INSTR(0xbb, op_rb7e)			/* RES 7,E */
{
	E &= ~128;
	STATES(8);
}

INSTR(0x84, op_rb0h)			/* RES 0,H */
{
	H &= ~1;
	STATES(8);
}

INSTR(0x8c, op_rb1h)			/* RES 1,H */
{
	H &= ~2;
	STATES(8);
}

INSTR(0x94, op_rb2h)			/* RES 2,H */
{
	H &= ~4;
	STATES(8);
}

INSTR(0x9c, op_rb3h)			/* RES 3,H */
{
	H &= ~8;
	STATES(8);
}

INSTR(0xa4, op_rb4h)			/* RES 4,H */
{
	H &= ~16;
	STATES(8);
}

INSTR(0xac, op_rb5h)			/* RES 5,H */
{
	H &= ~32;
	STATES(8);
}

INSTR(0xb4, op_rb6h)			/* RES 6,H */
{
	H &= ~64;
	STATES(8);
}

INSTR(0xbc, op_rb7h)			/* RES 7,H */
{
	H &= ~128;
	STATES(8);
}

INSTR(0x85, op_rb0l)			/* RES 0,L */
{
	L &= ~1;
	STATES(8);
}

INSTR(0x8d, op_rb1l)			/* RES 1,L */
{
	L &= ~2;
	STATES(8);
}

INSTR(0x95, op_rb2l)			/* RES 2,L */
{
	L &= ~4;
	STATES(8);
}

INSTR(0x9d, op_rb3l)			/* RES 3,L */
{
	L &= ~8;
	STATES(8);
}

INSTR(0xa5, op_rb4l)			/* RES 4,L */
{
	L &= ~16;
	STATES(8);
}

INSTR(0xad, op_rb5l)			/* RES 5,L */
{
	L &= ~32;
	STATES(8);
}

INSTR(0xb5, op_rb6l)			/* RES 6,L */
{
	L &= ~64;
	STATES(8);
}

INSTR(0xbd, op_rb7l)			/* RES 7,L */
{
	L &= ~128;
	STATES(8);
}

INSTR(0x86, op_rb0hl)			/* RES 0,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~1);
	STATES(15);
}

INSTR(0x8e, op_rb1hl)			/* RES 1,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~2);
	STATES(15);
}

INSTR(0x96, op_rb2hl)			/* RES 2,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~4);
	STATES(15);
}

INSTR(0x9e, op_rb3hl)			/* RES 3,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~8);
	STATES(15);
}

INSTR(0xa6, op_rb4hl)			/* RES 4,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~16);
	STATES(15);
}

INSTR(0xae, op_rb5hl)			/* RES 5,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~32);
	STATES(15);
}

INSTR(0xb6, op_rb6hl)			/* RES 6,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~64);
	STATES(15);
}

INSTR(0xbe, op_rb7hl)			/* RES 7,(HL) */
{
	memwrt((H << 8) + L, memrdr((H << 8) + L) & ~128);
	STATES(15);
}

INSTR(0x47, op_tb0a)			/* BIT 0,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x4f, op_tb1a)			/* BIT 1,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x57, op_tb2a)			/* BIT 2,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x5f, op_tb3a)			/* BIT 3,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x67, op_tb4a)			/* BIT 4,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x6f, op_tb5a)			/* BIT 5,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x77, op_tb6a)			/* BIT 6,A */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(A & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x7f, op_tb7a)			/* BIT 7,A */
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
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x40, op_tb0b)			/* BIT 0,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x48, op_tb1b)			/* BIT 1,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x50, op_tb2b)			/* BIT 2,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x58, op_tb3b)			/* BIT 3,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x60, op_tb4b)			/* BIT 4,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x68, op_tb5b)			/* BIT 5,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x70, op_tb6b)			/* BIT 6,B */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(B & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x78, op_tb7b)			/* BIT 7,B */
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
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x41, op_tb0c)			/* BIT 0,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x49, op_tb1c)			/* BIT 1,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x51, op_tb2c)			/* BIT 2,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x59, op_tb3c)			/* BIT 3,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x61, op_tb4c)			/* BIT 4,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x69, op_tb5c)			/* BIT 5,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x71, op_tb6c)			/* BIT 6,C */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(C & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x79, op_tb7c)			/* BIT 7,C */
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
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x42, op_tb0d)			/* BIT 0,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x4a, op_tb1d)			/* BIT 1,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x52, op_tb2d)			/* BIT 2,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x5a, op_tb3d)			/* BIT 3,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x62, op_tb4d)			/* BIT 4,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x6a, op_tb5d)			/* BIT 5,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x72, op_tb6d)			/* BIT 6,D */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(D & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x7a, op_tb7d)			/* BIT 7,D */
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
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x43, op_tb0e)			/* BIT 0,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x4b, op_tb1e)			/* BIT 1,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x53, op_tb2e)			/* BIT 2,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x5b, op_tb3e)			/* BIT 3,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x63, op_tb4e)			/* BIT 4,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x6b, op_tb5e)			/* BIT 5,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x73, op_tb6e)			/* BIT 6,E */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(E & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x7b, op_tb7e)			/* BIT 7,E */
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
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x44, op_tb0h)			/* BIT 0,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x4c, op_tb1h)			/* BIT 1,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x54, op_tb2h)			/* BIT 2,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x5c, op_tb3h)			/* BIT 3,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x64, op_tb4h)			/* BIT 4,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x6c, op_tb5h)			/* BIT 5,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x74, op_tb6h)			/* BIT 6,H */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(H & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x7c, op_tb7h)			/* BIT 7,H */
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
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x45, op_tb0l)			/* BIT 0,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 1) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x4d, op_tb1l)			/* BIT 1,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 2) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x55, op_tb2l)			/* BIT 2,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 4) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x5d, op_tb3l)			/* BIT 3,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 8) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x65, op_tb4l)			/* BIT 4,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 16) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x6d, op_tb5l)			/* BIT 5,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 32) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x75, op_tb6l)			/* BIT 6,L */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(L & 64) ? (F &= ~(Z_FLAG | P_FLAG)) : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x7d, op_tb7l)			/* BIT 7,L */
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
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x46, op_tb0hl)			/* BIT 0,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 1) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x4e, op_tb1hl)			/* BIT 1,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 2) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x56, op_tb2hl)			/* BIT 2,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 4) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x5e, op_tb3hl)			/* BIT 3,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 8) ? (F &= ~(Z_FLAG | P_FLAG))
				   : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x66, op_tb4hl)			/* BIT 4,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 16) ? (F &= ~(Z_FLAG | P_FLAG))
				    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x6e, op_tb5hl)			/* BIT 5,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 32) ? (F &= ~(Z_FLAG | P_FLAG))
				    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x76, op_tb6hl)			/* BIT 6,(HL) */
{
	F &= ~(N_FLAG | S_FLAG);
	F |= H_FLAG;
	(memrdr((H << 8) + L) & 64) ? (F &= ~(Z_FLAG | P_FLAG))
				    : (F |= (Z_FLAG | P_FLAG));
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x7e, op_tb7hl)			/* BIT 7,(HL) */
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
#ifdef UNDOC_FLAGS
	(WZ & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(WZ & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(12);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

/*
 * While the instructions is not documented in the Z80
 * documentation, it is in the Z280 one, including an
 * example to tell Z80 and Z280 apart.
 */
INSTR(0x37, op_undoc_slla)		/* SLL A */
{
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

#ifdef UNDOC_INST

INSTR(0x30, op_undoc_sllb)		/* SLL B */
{
	if (u_flag) {
		STATES(trap_cb());
	}

	(B & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	B = B << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x31, op_undoc_sllc)		/* SLL C */
{
	if (u_flag) {
		STATES(trap_cb());
	}

	(C & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	C = C << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x32, op_undoc_slld)		/* SLL D */
{
	if (u_flag) {
		STATES(trap_cb());
	}

	(D & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	D = D << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x33, op_undoc_slle)		/* SLL E */
{
	if (u_flag) {
		STATES(trap_cb());
	}

	(E & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	E = E << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x34, op_undoc_sllh)		/* SLL H */
{
	if (u_flag) {
		STATES(trap_cb());
	}

	(H & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H = H << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x35, op_undoc_slll)		/* SLL L */
{
	if (u_flag) {
		STATES(trap_cb());
	}

	(L & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	L = L << 1 | 1;
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x36, op_undoc_sllhl)		/* SLL (HL) */
{
	register BYTE P;
	WORD addr;

	if (u_flag) {
		STATES(trap_cb());
	}

	addr = (H << 8) + L;
	P = memrdr(addr);
	(P & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	P = (P << 1) | 1;
	memwrt(addr, P);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FAST_FLAGS
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FAST_FLAGS */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[P];
#endif
#endif /* FAST_FLAGS */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(15);
}

#endif /* UNDOC_INST */
