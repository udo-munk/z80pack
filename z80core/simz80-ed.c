/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 instructions
 *	beginning with the prefix 0xed
 */

INSTR(0x46, op_im0)			/* IM 0 */
{
	int_mode = 0;
	STATES(8);
}

INSTR(0x56, op_im1)			/* IM 1 */
{
	int_mode = 1;
	STATES(8);
}

INSTR(0x5e, op_im2)			/* IM 2 */
{
	int_mode = 2;
	STATES(8);
}

INSTR(0x4d, op_reti)			/* RETI */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(14);
}

INSTR(0x45, op_retn)			/* RETN */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	if (IFF & 2)
		IFF |= 1;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(14);
}

INSTR(0x44, op_neg)			/* NEG */
{
	(A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	(A == 0x80) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(0 - ((signed char) A & 0xf) < 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A = 0 - A;
	F |= N_FLAG;
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[A];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

INSTR(0x78, op_inaic)			/* IN A,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	A = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x40, op_inbic)			/* IN B,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	B = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x48, op_incic)			/* IN C,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	C = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x50, op_indic)			/* IN D,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	D = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x58, op_ineic)			/* IN E,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	E = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x60, op_inhic)			/* IN H,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	H = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x68, op_inlic)			/* IN L,(C) */
{
	extern BYTE io_in(BYTE, BYTE);

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	L = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

INSTR(0x79, op_outca)			/* OUT (C),A */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, A);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x41, op_outcb)			/* OUT (C),B */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, B);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x49, op_outcc)			/* OUT (C),C */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, C);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x51, op_outcd)			/* OUT (C),D */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, D);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x59, op_outce)			/* OUT (C),E */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, E);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x61, op_outch)			/* OUT (C),H */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, H);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x69, op_outcl)			/* OUT (C),L */
{
	extern void io_out(BYTE, BYTE, BYTE);

	io_out(C, B, L);
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0xa2, op_ini)			/* INI */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE data;

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	data = io_in(C, B);
	B--;
	memwrt((H << 8) + L, data);
	L++;
	if (!L)
		H++;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) ((C + 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xb2, op_inir)			/* INIR */
{
	extern BYTE io_in(BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef INSTR_SWTCH
	register int t;
#endif

	addr = (H << 8) + L;
	R -= 2;
	t = -21;
	do {
		data = io_in(C, B);
		B--;
		memwrt(addr++, data);
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) ((C + 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[k & 0x07]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
#ifdef UNDOC_FLAGS
	F &= ~(Y_FLAG | X_FLAG);
	WZ = (0x0100 + C) + 1;
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xb2, op_inir)			/* INIR */
{
#ifndef INSTR_SWTCH
	op_ini();
#else /* INSTR_SWTCH */
	extern BYTE io_in(BYTE, BYTE);
	BYTE data;

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	data = io_in(C, B);
	B--;
	memwrt((H << 8) + L, data);
	L++;
	if (!L)
		H++;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) ((C + 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if (!(F & Z_FLAG)) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		register int i;
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
		if (F & C_FLAG) {
			if (F & N_FLAG) {
				i = parity[(B - 1) & 0x07];
				((B & 0xf) == 0x0) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			} else {
				i = parity[(B + 1) & 0x07];
				((B & 0xf) == 0xf) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			}
		} else
			i = parity[B & 0x07];
		(((F & P_FLAG) == 0) ^ i) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0xaa, op_ind)			/* IND */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE data;

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) - 1;
#endif
	data = io_in(C, B);
	B--;
	memwrt((H << 8) + L, data);
	L--;
	if (L == 0xff)
		H--;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) ((C - 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xba, op_indr)			/* INDR */
{
	extern BYTE io_in(BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef INSTR_SWTCH
	register int t;
#endif

	addr = (H << 8) + L;
	R -= 2;
	t = -21;
	do {
		data = io_in(C, B);
		memwrt(addr--, data);
		B--;
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) ((C - 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[k & 0x07]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
#ifdef UNDOC_FLAGS
	F &= ~(Y_FLAG | X_FLAG);
	WZ = (0x0100 + C) - 1;
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xba, op_indr)			/* INDR */
{
#ifndef INSTR_SWTCH
	op_ind();
#else /* INSTR_SWTCH */
	extern BYTE io_in(BYTE, BYTE);
	BYTE data;

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) - 1;
#endif
	data = io_in(C, B);
	B--;
	memwrt((H << 8) + L, data);
	L--;
	if (L == 0xff)
		H--;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) ((C - 1) & 0xff) + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if (!(F & Z_FLAG)) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		register int i;
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
		if (F & C_FLAG) {
			if (F & N_FLAG) {
				i = parity[(B - 1) & 0x07];
				((B & 0xf) == 0x0) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			} else {
				i = parity[(B + 1) & 0x07];
				((B & 0xf) == 0xf) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			}
		} else
			i = parity[B & 0x07];
		(((F & P_FLAG) == 0) ^ i) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0xa3, op_outi)			/* OUTI */
{
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE data;

	data = memrdr((H << 8) + L);
	B--;
	io_out(C, B, data);
	L++;
	if (!L)
		H++;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = ((B << 8) + C) + 1;
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xb3, op_otir)			/* OTIR */
{
	extern void io_out(BYTE, BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef INSTR_SWTCH
	register int t;
#endif

	addr = (H << 8) + L;
	R -= 2;
	t = -21;
	do {
		B--;
		data = memrdr(addr++);
		io_out(C, B, data);
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[k & 0x07]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
#ifdef UNDOC_FLAGS
	F &= ~(Y_FLAG | X_FLAG);
	WZ = (WORD) C + (WORD) 1;
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xb3, op_otir)			/* OTIR */
{
#ifndef INSTR_SWTCH
	op_outi();
#else /* INSTR_SWTCH */
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE data;

	data = memrdr((H << 8) + L);
	B--;
	io_out(C, B, data);
	L++;
	if (!L)
		H++;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = ((B << 8) + C) + 1;
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if (!(F & Z_FLAG)) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		register int i;
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
		if (F & C_FLAG) {
			if (F & N_FLAG) {
				i = parity[(B - 1) & 0x07];
				((B & 0xf) == 0x0) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			} else {
				i = parity[(B + 1) & 0x07];
				((B & 0xf) == 0xf) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			}
		} else
			i = parity[B & 0x07];
		(((F & P_FLAG) == 0) ^ i) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0xab, op_outd)			/* OUTD */
{
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE data;

	data = memrdr((H << 8) + L);
	B--;
	io_out(C, B, data);
	L--;
	if (L == 0xff)
		H--;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = ((B << 8) + C) - 1;
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xbb, op_otdr)			/* OTDR */
{
	extern void io_out(BYTE, BYTE, BYTE);
	WORD addr;
	BYTE data;
#ifndef INSTR_SWTCH
	register int t;
#endif

	addr = (H << 8) + L;
	R -= 2;
	t = -21;
	do {
		B--;
		data = memrdr(addr--);
		io_out(C, B, data);
		t += 21;
		R += 2;
	} while (B);
	H = addr >> 8;
	L = addr;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[k & 0x07]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	F &= ~S_FLAG;
#endif
	F |= Z_FLAG;
#ifdef UNDOC_FLAGS
	F &= ~(Y_FLAG | X_FLAG);
	WZ = (WORD) C - (WORD) 1;
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xbb, op_otdr)			/* OTDR */
{
#ifndef INSTR_SWTCH
	op_outd();
#else /* INSTR_SWTCH */
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE data;

	data = memrdr((H << 8) + L);
	B--;
	io_out(C, B, data);
	L--;
	if (L == 0xff)
		H--;
#if 0
	F |= N_FLAG; /* As documented in the "Z80 CPU User Manual" */
#else
	/* S,H,P,N,C flags according to "The Undocumented Z80 Documented" */
	WORD k = (WORD) L + (WORD) data;
	(k > 255) ? (F |= (H_FLAG | C_FLAG)) : (F &= ~(H_FLAG | C_FLAG));
	(parity[(k & 0x07) ^ B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(data & 128) ? (F |= N_FLAG) : (F &= ~N_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#endif
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = ((B << 8) + C) - 1;
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if (!(F & Z_FLAG)) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		register int i;
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
		if (F & C_FLAG) {
			if (F & N_FLAG) {
				i = parity[(B - 1) & 0x07];
				((B & 0xf) == 0x0) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			} else {
				i = parity[(B + 1) & 0x07];
				((B & 0xf) == 0xf) ? (F |= H_FLAG)
						   : (F &= ~H_FLAG);
			}
		} else
			i = parity[B & 0x07];
		(((F & P_FLAG) == 0) ^ i) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0x57, op_ldai)			/* LD A,I */
{
	A = I;
	F &= ~(N_FLAG | H_FLAG);
	(IFF & 2) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[A];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(9);
}

INSTR(0x5f, op_ldar)			/* LD A,R */
{
	A = (R_ & 0x80) | (R & 0x7f);
	F &= ~(N_FLAG | H_FLAG);
	(IFF & 2) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[A];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(9);
}

INSTR(0x47, op_ldia)			/* LD I,A */
{
	I = A;
	STATES(9);
}

INSTR(0x4f, op_ldra)			/* LD R,A */
{
	R_ = R = A;
	STATES(9);
}

INSTR(0x4b, op_ldbcinn)			/* LD BC,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	C = memrdr(i++);
	B = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x5b, op_lddeinn)			/* LD DE,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	E = memrdr(i++);
	D = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x6b, op_ldhlinn)			/* LD HL,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	L = memrdr(i++);
	H = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x7b, op_ldspinn)			/* LD SP,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	SP = memrdr(i++);
	SP += memrdr(i) << 8;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x43, op_ldinbc)			/* LD (nn),BC */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, C);
	memwrt(i, B);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x53, op_ldinde)			/* LD (nn),DE */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, E);
	memwrt(i, D);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x63, op_ldinhl2)			/* LD (nn),HL */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, L);
	memwrt(i, H);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(20);
}

INSTR(0x73, op_ldinsp)			/* LD (nn),SP */
{
	register WORD addr;
	WORD i;

	addr = memrdr(PC++);
	addr += memrdr(PC++) << 8;
	i = SP;
	memwrt(addr++, i);
	memwrt(addr, i >> 8);
#ifdef UNDOC_FLAGS
	WZ = addr;
#endif
	STATES(20);
}

INSTR(0x4a, op_adchb)			/* ADC HL,BC */
{
	int carry, i;
	WORD hl, bc;
	SWORD shl, sbc;

	hl = (H << 8) + L;
	bc = (B << 8) + C;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x5a, op_adchd)			/* ADC HL,DE */
{
	int carry, i;
	WORD hl, de;
	SWORD shl, sde;

	hl = (H << 8) + L;
	de = (D << 8) + E;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x6a, op_adchh)			/* ADC HL,HL */
{
	int carry, i;
	WORD hl;
	SWORD shl;

	hl = (H << 8) + L;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x7a, op_adchs)			/* ADC HL,SP */
{
	int carry, i;
	WORD hl, sp;
	SWORD shl, ssp;

	hl = (H << 8) + L;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x42, op_sbchb)			/* SBC HL,BC */
{
	int carry, i;
	WORD hl, bc;
	SWORD shl, sbc;

	hl = (H << 8) + L;
	bc = (B << 8) + C;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x52, op_sbchd)			/* SBC HL,DE */
{
	int carry, i;
	WORD hl, de;
	SWORD shl, sde;

	hl = (H << 8) + L;
	de = (D << 8) + E;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x62, op_sbchh)			/* SBC HL,HL */
{
	int carry, i;
	WORD hl;
	SWORD shl;

	hl = (H << 8) + L;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0x72, op_sbchs)			/* SBC HL,SP */
{
	int carry, i;
	WORD hl, sp;
	SWORD shl, ssp;

	hl = (H << 8) + L;
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
#ifdef UNDOC_FLAGS
	(i & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = hl + 1;
	modF = 1;
#endif
	STATES(15);
}

INSTR(0xa0, op_ldi)			/* LDI */
{
	register BYTE tmp;

	tmp = memrdr((H << 8) + L);
	memwrt((D << 8) + E, tmp);
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
#ifdef UNDOC_FLAGS
	tmp += A;
	(tmp & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xb0, op_ldir)			/* LDIR */
{
	register WORD i;
	register WORD s, d;
	BYTE tmp;
#ifndef INSTR_SWTCH
	register int t;
#endif

	i = (B << 8) + C;
	d = (D << 8) + E;
	s = (H << 8) + L;
	R -= 2;
	t = -21;
#ifdef UNDOC_FLAGS
	if (i == 1)
		WZ = PC - 1;
#endif
	do {
		tmp = memrdr(s++);
		memwrt(d++, tmp);
		t += 21;
		R += 2;
	} while (--i);
	B = C = 0;
	D = d >> 8;
	E = d;
	H = s >> 8;
	L = s;
	F &= ~(N_FLAG | P_FLAG | H_FLAG);
#ifdef UNDOC_FLAGS
	tmp += A;
	(tmp & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xb0, op_ldir)			/* LDIR */
{
#ifndef INSTR_SWTCH
	op_ldi();
#else /* INSTR_SWTCH */
	register BYTE tmp;

	tmp = memrdr((H << 8) + L);
	memwrt((D << 8) + E, tmp);
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
#ifdef UNDOC_FLAGS
	tmp += A;
	(tmp & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
#endif /* INSTR_SWTCH */
	
	if (F & P_FLAG) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0xa8, op_ldd)			/* LDD */
{
	register BYTE tmp;

	tmp = memrdr((H << 8) + L);
	memwrt((D << 8) + E, tmp);
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
#ifdef UNDOC_FLAGS
	tmp += A;
	(tmp & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xb8, op_lddr)			/* LDDR */
{
	register WORD i;
	register WORD s, d;
	BYTE tmp;
#ifndef INSTR_SWTCH
	register int t;
#endif

	i = (B << 8) + C;
	d = (D << 8) + E;
	s = (H << 8) + L;
	R -= 2;
	t = -21;
#ifdef UNDOC_FLAGS
	if (i == 1)
		WZ = PC - 1;
#endif
	do {
		tmp = memrdr(s--);
		memwrt(d--, tmp);
		t += 21;
		R += 2;
	} while (--i);
	B = C = 0;
	D = d >> 8;
	E = d;
	H = s >> 8;
	L = s;
	F &= ~(N_FLAG | P_FLAG | H_FLAG);
#ifdef UNDOC_FLAGS
	tmp += A;
	(tmp & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xb8, op_lddr)			/* LDDR */
{
#ifndef INSTR_SWTCH
	op_ldd();
#else /* INSTR_SWTCH */
	register BYTE tmp;

	tmp = memrdr((H << 8) + L);
	memwrt((D << 8) + E, tmp);
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
#ifdef UNDOC_FLAGS
	tmp += A;
	(tmp & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if (F & P_FLAG) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0xa1, op_cpi)			/* CPI */
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
#ifdef UNDOC_FLAGS
	if (F & H_FLAG)
		i--;
	(i & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ++;
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xb1, op_cpir)			/* CPIR */
{
	register WORD s;
	register BYTE d;
	register WORD i;
	BYTE tmp;
#ifndef INSTR_SWTCH
	register int t;
#endif

	i = (B << 8) + C;
	s = (H << 8) + L;
	R -= 2;
	t = -21;
	do {
		tmp = memrdr(s++);
		((tmp & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		d = A - tmp;
		t += 21;
		R += 2;
#ifdef UNDOC_FLAGS
		if (i == 1 || d == 0)
			WZ++;
		else
			WZ = PC;
#endif
	} while (--i && d);
	F |= N_FLAG;
	B = i >> 8;
	C = i;
	H = s >> 8;
	L = s;
	(i) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(d) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(d & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#ifdef UNDOC_FLAGS
	if (F & H_FLAG)
		d--;
	(d & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(d & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xb1, op_cpir)			/* CPIR */
{
#ifndef INSTR_SWTCH
	op_cpi();
#else /* INSTR_SWTCH */
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
#ifdef UNDOC_FLAGS
	if (F & H_FLAG)
		i--;
	(i & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ++;
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if ((F & (P_FLAG | Z_FLAG)) == P_FLAG) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0xa9, op_cpdop)			/* CPD */
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
#ifdef UNDOC_FLAGS
	if (F & H_FLAG)
		i--;
	(i & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ--;
	modF = 1;
#endif
	STATES(16);
}

#ifdef FAST_BLOCK
INSTR(0xb9, op_cpdr)			/* CPDR */
{
	register WORD s;
	register BYTE d;
	register WORD i;
	BYTE tmp;
#ifndef INSTR_SWTCH
	register int t;
#endif

	i = (B << 8) + C;
	s = (H << 8) + L;
	R -= 2;
	t = -21;
	do {
		tmp = memrdr(s--);
		((tmp & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		d = A - tmp;
		t += 21;
		R += 2;
#ifdef UNDOC_FLAGS
		if (i == 1 || d == 0)
			WZ--;
		else
			WZ = PC - 2;
#endif
	} while (--i && d);
	F |= N_FLAG;
	B = i >> 8;
	C = i;
	H = s >> 8;
	L = s;
	(i) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(d) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(d & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#ifdef UNDOC_FLAGS
	if (F & H_FLAG)
		d--;
	(d & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(d & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(t + 16);
}
#else /* !FAST_BLOCK */
INSTR(0xb9, op_cpdr)			/* CPDR */
{
#ifndef INSTR_SWTCH
	op_cpdop();
#else /* INSTR_SWTCH */
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
#ifdef UNDOC_FLAGS
	if (F & H_FLAG)
		i--;
	(i & 2) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ--;
	modF = 1;
#endif
#endif /* INSTR_SWTCH */

	if ((F & (P_FLAG | Z_FLAG)) == P_FLAG) {
		PC -= 2;
#ifdef UNDOC_FLAGS
		WZ = PC + 1;
		(PC & 0x2000) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
		(PC & 0x0800) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
		STATES(21);
	}
	STATES(16);
}
#endif /* !FAST_BLOCK */

INSTR(0x6f, op_oprld)			/* RLD (HL) */
{
	register BYTE i, j;
	register WORD addr;

	addr = (H << 8) + L;
	i = memrdr(addr);
	j = A & 0x0f;
	A = (A & 0xf0) | (i >> 4);
	i = (i << 4) | j;
	memwrt(addr, i);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	WZ = addr + 1;
	modF = 1;
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
	WZ = addr + 1;
	modF = 1;
#endif
#endif /* FLAG_TABLES */
	STATES(18);
}

INSTR(0x67, op_oprrd)			/* RRD (HL) */
{
	register BYTE i, j;
	register WORD addr;

	addr = (H << 8) + L;
	i = memrdr(addr);
	j = A & 0x0f;
	A = (A & 0xf0) | (i & 0x0f);
	i = (i >> 4) | (j << 4);
	memwrt(addr, i);
	F &= ~(H_FLAG | N_FLAG);
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	WZ = addr + 1;
	modF = 1;
#endif
	STATES(18);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED Z80 INSTRUCTIONS, BEWARE!      **********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

INSTR(0x71, op_undoc_outc0)		/* OUT (C),0 */
{
	extern void io_out(BYTE, BYTE, BYTE);

	if (u_flag) {
		STATES(trap_ed());
	}

	io_out(C, B, 0); /* NMOS, CMOS outputs 0xff */
#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	STATES(12);
}

INSTR(0x70, op_undoc_infic)		/* IN F,(C) */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE tmp;

	if (u_flag) {
		STATES(trap_ed());
	}

#ifdef UNDOC_FLAGS
	WZ = ((B << 8) + C) + 1;
#endif
	tmp = io_in(C, B);
	F &= ~(N_FLAG | H_FLAG);
#ifndef FLAG_TABLES
	(tmp) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(tmp & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[tmp]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#ifdef UNDOC_FLAGS
	(tmp & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(tmp & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZP_FLAGS) | szp_flags[tmp];
#else
	F = (F & ~SZYXP_FLAGS) | szyxp_flags[tmp];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(12);
}

#ifdef UNDOC_IALL

#ifndef INSTR_SWTCH
static int op_undoc_nop(void)		/* NOP */
#else
case 0x00: case 0x01: case 0x02: case 0x03: /* NOP */
case 0x04: case 0x05: case 0x06: case 0x07: /* NOP */
case 0x08: case 0x09: case 0x0a: case 0x0b: /* NOP */
case 0x0c: case 0x0d: case 0x0e: case 0x0f: /* NOP */
case 0x10: case 0x11: case 0x12: case 0x13: /* NOP */
case 0x14: case 0x15: case 0x16: case 0x17: /* NOP */
case 0x18: case 0x19: case 0x1a: case 0x1b: /* NOP */
case 0x1c: case 0x1d: case 0x1e: case 0x1f: /* NOP */
case 0x20: case 0x21: case 0x22: case 0x23: /* NOP */
case 0x24: case 0x25: case 0x26: case 0x27: /* NOP */
case 0x28: case 0x29: case 0x2a: case 0x2b: /* NOP */
case 0x2c: case 0x2d: case 0x2e: case 0x2f: /* NOP */
case 0x30: case 0x31: case 0x32: case 0x33: /* NOP */
case 0x34: case 0x35: case 0x36: case 0x37: /* NOP */
case 0x38: case 0x39: case 0x3a: case 0x3b: /* NOP */
case 0x3c: case 0x3d: case 0x3e: case 0x3f: /* NOP */
				 case 0x77: /* NOP */
				 case 0x7f: /* NOP */
case 0x80: case 0x81: case 0x82: case 0x83: /* NOP */
case 0x84: case 0x85: case 0x86: case 0x87: /* NOP */
case 0x88: case 0x89: case 0x8a: case 0x8b: /* NOP */
case 0x8c: case 0x8d: case 0x8e: case 0x8f: /* NOP */
case 0x90: case 0x91: case 0x92: case 0x93: /* NOP */
case 0x94: case 0x95: case 0x96: case 0x97: /* NOP */
case 0x98: case 0x99: case 0x9a: case 0x9b: /* NOP */
case 0x9c: case 0x9d: case 0x9e: case 0x9f: /* NOP */
case 0xa4: case 0xa5: case 0xa6: case 0xa7: /* NOP */
case 0xac: case 0xad: case 0xae: case 0xaf: /* NOP */
case 0xb4: case 0xb5: case 0xb6: case 0xb7: /* NOP */
case 0xbc: case 0xbd: case 0xbe: case 0xbf: /* NOP */
case 0xc0: case 0xc1: case 0xc2: case 0xc3: /* NOP */
case 0xc4: case 0xc5: case 0xc6: case 0xc7: /* NOP */
case 0xc8: case 0xc9: case 0xca: case 0xcb: /* NOP */
case 0xcc: case 0xcd: case 0xce: case 0xcf: /* NOP */
case 0xd0: case 0xd1: case 0xd2: case 0xd3: /* NOP */
case 0xd4: case 0xd5: case 0xd6: case 0xd7: /* NOP */
case 0xd8: case 0xd9: case 0xda: case 0xdb: /* NOP */
case 0xdc: case 0xdd: case 0xde: case 0xdf: /* NOP */
case 0xe0: case 0xe1: case 0xe2: case 0xe3: /* NOP */
case 0xe4: case 0xe5: case 0xe6: case 0xe7: /* NOP */
case 0xe8: case 0xe9: case 0xea: case 0xeb: /* NOP */
case 0xec: case 0xed: case 0xee: case 0xef: /* NOP */
case 0xf0: case 0xf1: case 0xf2: case 0xf3: /* NOP */
case 0xf4: case 0xf5: case 0xf6: case 0xf7: /* NOP */
case 0xf8: case 0xf9: case 0xfa: case 0xfb: /* NOP */
case 0xfc: case 0xfd: case 0xfe: case 0xff: /* NOP */
#endif
{
	if (u_flag) {
		STATES(trap_ed());
	}

	STATES(8);
}

#ifndef INSTR_SWTCH
static int op_undoc_im0(void)		/* IM 0 */
#else
case 0x4e:				/* IM 0 */
case 0x66:				/* IM 0 */
case 0x6e:				/* IM 0 */
#endif
{
	if (u_flag) {
		STATES(trap_ed());
	}

	int_mode = 0;
	STATES(8);
}

INSTR(0x76, op_undoc_im1)		/* IM 1 */
{
	if (u_flag) {
		STATES(trap_ed());
	}

	int_mode = 1;
	STATES(8);
}

INSTR(0x7e, op_undoc_im2)		/* IM 2 */
{
	if (u_flag) {
		STATES(trap_ed());
	}

	int_mode = 2;
	STATES(8);
}

#ifndef INSTR_SWTCH
static int op_undoc_reti(void)		/* RETI */
#else
case 0x5d:				/* RETI */
case 0x6d:				/* RETI */
case 0x7d:				/* RETI */
#endif
{
	if (u_flag) {
		STATES(trap_ed());
	}

	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(14);
}

#ifndef INSTR_SWTCH
static int op_undoc_retn(void)		/* RETN */
#else
case 0x55:				/* RETN */
case 0x65:				/* RETN */
case 0x75:				/* RETN */
#endif
{
	if (u_flag) {
		STATES(trap_ed());
	}

	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	if (IFF & 2)
		IFF |= 1;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(14);
}

#ifndef INSTR_SWTCH
static int op_undoc_neg(void)		/* NEG */
#else
case 0x4c:				/* NEG */
case 0x54:				/* NEG */
case 0x5c:				/* NEG */
case 0x64:				/* NEG */
case 0x6c:				/* NEG */
case 0x74:				/* NEG */
case 0x7c:				/* NEG */
#endif
{
	if (u_flag) {
		STATES(trap_ed());
	}

	(A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	(A == 0x80) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(0 - ((signed char) A & 0xf) < 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A = 0 - A;
	F |= N_FLAG;
#ifndef FLAG_TABLES
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[A];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[A];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(8);
}

#endif /* UNDOC_IALL */

#endif /* UNDOC_INST */
