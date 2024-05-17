/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	This module contains the implementation of all Z80 single byte
 *	instructions and dispatchers for the prefixes 0xcb, 0xdd, 0xed,
 *	and 0xfd.
 */

INSTR(0x00, op_nop)			/* NOP */
{
	STATES(4);
}

INSTR(0x76, op_halt)			/* HALT */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_HLTA | CPU_MEMR;
#endif

#ifdef FRONTPANEL
	if (!F_flag) {
#endif
		if (IFF == 0) {
			/* without a frontpanel DI + HALT stops the machine */
			cpu_error = OPHALT;
			cpu_state = STOPPED;
		} else {
			/* else wait for INT, NMI or user interrupt */
			while ((int_int == 0) && (int_nmi == 0) &&
			       (cpu_state == CONTIN_RUN)) {
				SLEEP_MS(1);
				R += 99;
			}
		}
#ifdef BUS_8080
		if (int_int)
			cpu_bus = CPU_INTA | CPU_WO | CPU_HLTA | CPU_M1;
#endif

		busy_loop_cnt = 0;

#ifdef FRONTPANEL
	} else {
		fp_led_address = 0xffff;
		fp_led_data = 0xff;

		if (IFF == 0) {
			/* INT disabled, wait for NMI,
			   frontpanel reset or user interrupt */
			while ((int_nmi == 0) && !(cpu_state & RESET)) {
				fp_clock++;
				fp_sampleData();
				SLEEP_MS(1);
				R += 99;
				if (cpu_error != NONE)
					break;
			}
		} else {
			/* else wait for INT, NMI,
			   frontpanel reset or user interrupt */
			while ((int_int == 0) && (int_nmi == 0) &&
			       !(cpu_state & RESET)) {
				fp_clock++;
				fp_sampleData();
				SLEEP_MS(1);
				R += 99;
				if (cpu_error != NONE)
					break;
			}
			if (int_int) {
				cpu_bus = CPU_INTA | CPU_WO |
					  CPU_HLTA | CPU_M1;
				fp_clock++;
				fp_sampleLightGroup(0, 0);
			}
		}
	}
#endif

	STATES(4);
}

INSTR(0x37, op_scf)			/* SCF */
{
	F |= C_FLAG;
	F &= ~(N_FLAG | H_FLAG);
#ifdef UNDOC_FLAGS
	/* This is Zilog/Mostek NMOS Z80 behaviour */
	(A & 32) ? (F |= Y_FLAG) : (pmodF ? (F &= ~Y_FLAG) : 0);
	(A & 8) ? (F |= X_FLAG) : (pmodF ? (F &= ~X_FLAG) : 0);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x3f, op_ccf)			/* CCF */
{
	if (F & C_FLAG) {
		F |= H_FLAG;
		F &= ~C_FLAG;
	} else {
		F &= ~H_FLAG;
		F |= C_FLAG;
	}
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	/* This is Zilog/Mostek NMOS Z80 behaviour */
	(A & 32) ? (F |= Y_FLAG) : (pmodF ? (F &= ~Y_FLAG) : 0);
	(A & 8) ? (F |= X_FLAG) : (pmodF ? (F &= ~X_FLAG) : 0);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x2f, op_cpl)			/* CPL */
{
	A = ~A;
	F |= H_FLAG | N_FLAG;
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

/*
 * This is my original implementation of the DAA instruction.
 * It implements the instruction as described in Z80 data sheets
 * and books, but it won't pass the ex.com instruction exerciser.
 * Below is a contributed implementation active, that also passes
 * the tests done by ex.com.
 */
#if 0
INSTR(0x27, op_daa)			/* DAA */
{
	if (F & N_FLAG) {		/* subtractions */
		if (((A & 0x0f) > 9) || (F & H_FLAG)) {
			(((A & 0x0f) - 6) < 0) ? (F |= H_FLAG)
					       : (F &= ~H_FLAG);
			A -= 6;
		}
		if (((A & 0xf0) > 0x90) || (F & C_FLAG)) {
			if (((A & 0xf0) - 0x60) < 0)
				F |= C_FLAG;
			A -= 0x60;
		}
	} else {			/* additions */
		if (((A & 0x0f) > 9) || (F & H_FLAG)) {
			(((A & 0x0f) + 6) > 0x0f) ? (F |= H_FLAG)
						  : (F &= ~H_FLAG);
			A += 6;
		}
		if (((A & 0xf0) > 0x90) || (F & C_FLAG)) {
			if (((A & 0xf0) + 0x60) > 0xf0)
				F |= C_FLAG;
			A += 0x60;
		}
	}
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
#endif /*FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}
#endif

/*
 * This implementation was contributed by Mark Garlanger,
 * see http://heathkit.garlanger.com/
 * It passes the instruction exerciser test from ex.com
 * and is correct.
 */
INSTR(0x27, op_daa)			/* DAA */
{
	int tmp_a = A;
	int low_nibble = A & 0x0f;
	int carry = (F & C_FLAG);

	if (F & N_FLAG) {		/* subtraction */
		int adjustment = (carry || (tmp_a > 0x99)) ? 0x160 : 0x00;

		if ((F & H_FLAG) || (low_nibble > 9)) {
			if (low_nibble > 5) {
				F &= ~H_FLAG;
			}
			tmp_a = (tmp_a - 6) & 0xff;
		}
		tmp_a -= adjustment;
	} else {			/* addition */
		if ((low_nibble > 9) || (F & H_FLAG)) {
			(low_nibble > 9) ? (F |= H_FLAG) : (F &= ~H_FLAG);
			tmp_a += 6;
		}
		if (((tmp_a & 0x1f0) > 0x90) || carry) {
			tmp_a += 0x60;
		}
	}

	(carry || (tmp_a & 0x100)) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (tmp_a & 0xff);
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
	STATES(4);
}

INSTR(0xfb, op_ei)			/* EI */
{
	IFF = 3;
	int_protection = 1;		/* protect next instruction */
	STATES(4);
}

INSTR(0xf3, op_di)			/* DI */
{
	IFF = 0;
	STATES(4);
}

INSTR(0xdb, op_in)			/* IN A,(n) */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE addr;

	addr = memrdr(PC++);
#ifdef UNDOC_FLAGS
	WZ = (A << 8) + ((addr + 1) & 0xff);
#endif
	A = io_in(addr, A);
	STATES(11);
}

INSTR(0xd3, op_out)			/* OUT (n),A */
{
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE addr;

	addr = memrdr(PC++);
	io_out(addr, A, A);
#ifdef UNDOC_FLAGS
	WZ = (A << 8) + ((addr + 1) & 0xff);
#endif
	STATES(11);
}

INSTR(0x3e, op_ldan)			/* LD A,n */
{
	A = memrdr(PC++);
	STATES(7);
}

INSTR(0x06, op_ldbn)			/* LD B,n */
{
	B = memrdr(PC++);
	STATES(7);
}

INSTR(0x0e, op_ldcn)			/* LD C,n */
{
	C = memrdr(PC++);
	STATES(7);
}

INSTR(0x16, op_lddn)			/* LD D,n */
{
	D = memrdr(PC++);
	STATES(7);
}

INSTR(0x1e, op_lden)			/* LD E,n */
{
	E = memrdr(PC++);
	STATES(7);
}

INSTR(0x26, op_ldhn)			/* LD H,n */
{
	H = memrdr(PC++);
	STATES(7);
}

INSTR(0x2e, op_ldln)			/* LD L,n */
{
	L = memrdr(PC++);
	STATES(7);
}

INSTR(0x0a, op_ldabc)			/* LD A,(BC) */
{
	register WORD i;

	i = (B << 8) + C;
	A = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i + 1;
#endif
	STATES(7);
}

INSTR(0x1a, op_ldade)			/* LD A,(DE) */
{
	register WORD i;

	i = (D << 8) + E;
	A = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i + 1;
#endif
	STATES(7);
}

INSTR(0x3a, op_ldann)			/* LD A,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	A = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i + 1;
#endif
	STATES(13);
}

INSTR(0x02, op_ldbca)			/* LD (BC),A */
{
	memwrt((B << 8) + C, A);
#ifdef UNDOC_FLAGS
	WZ = (A << 8) + ((C + 1) & 0xff);
#endif
	STATES(7);
}

INSTR(0x12, op_lddea)			/* LD (DE),A */
{
	memwrt((D << 8) + E, A);
#ifdef UNDOC_FLAGS
	WZ = (A << 8) + ((E + 1) & 0xff);
#endif
	STATES(7);
}

INSTR(0x32, op_ldnna)			/* LD (nn),A */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, A);
#ifdef UNDOC_FLAGS
	WZ = (A << 8) + ((i + 1) & 0xff);
#endif
	STATES(13);
}

INSTR(0x77, op_ldhla)			/* LD (HL),A */
{
	memwrt((H << 8) + L, A);
	STATES(7);
}

INSTR(0x70, op_ldhlb)			/* LD (HL),B */
{
	memwrt((H << 8) + L, B);
	STATES(7);
}

INSTR(0x71, op_ldhlc)			/* LD (HL),C */
{
	memwrt((H << 8) + L, C);
	STATES(7);
}

INSTR(0x72, op_ldhld)			/* LD (HL),D */
{
	memwrt((H << 8) + L, D);
	STATES(7);
}

INSTR(0x73, op_ldhle)			/* LD (HL),E */
{
	memwrt((H << 8) + L, E);
	STATES(7);
}

INSTR(0x74, op_ldhlh)			/* LD (HL),H */
{
	memwrt((H << 8) + L, H);
	STATES(7);
}

INSTR(0x75, op_ldhll)			/* LD (HL),L */
{
	memwrt((H << 8) + L, L);
	STATES(7);
}

INSTR(0x36, op_ldhl1)			/* LD (HL),n */
{
	memwrt((H << 8) + L, memrdr(PC++));
	STATES(10);
}

INSTR(0x7f, op_ldaa)			/* LD A,A */
{
	STATES(4);
}

INSTR(0x78, op_ldab)			/* LD A,B */
{
	A = B;
	STATES(4);
}

INSTR(0x79, op_ldac)			/* LD A,C */
{
	A = C;
	STATES(4);
}

INSTR(0x7a, op_ldad)			/* LD A,D */
{
	A = D;
	STATES(4);
}

INSTR(0x7b, op_ldae)			/* LD A,E */
{
	A = E;
	STATES(4);
}

INSTR(0x7c, op_ldah)			/* LD A,H */
{
	A = H;
	STATES(4);
}

INSTR(0x7d, op_ldal)			/* LD A,L */
{
	A = L;
	STATES(4);
}

INSTR(0x7e, op_ldahl)			/* LD A,(HL) */
{
	A = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x47, op_ldba)			/* LD B,A */
{
	B = A;
	STATES(4);
}

INSTR(0x40, op_ldbb)			/* LD B,B */
{
	STATES(4);
}

INSTR(0x41, op_ldbc)			/* LD B,C */
{
	B = C;
	STATES(4);
}

INSTR(0x42, op_ldbd)			/* LD B,D */
{
	B = D;
	STATES(4);
}

INSTR(0x43, op_ldbe)			/* LD B,E */
{
	B = E;
	STATES(4);
}

INSTR(0x44, op_ldbh)			/* LD B,H */
{
	B = H;
	STATES(4);
}

INSTR(0x45, op_ldbl)			/* LD B,L */
{
	B = L;
	STATES(4);
}

INSTR(0x46, op_ldbhl)			/* LD B,(HL) */
{
	B = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x4f, op_ldca)			/* LD C,A */
{
	C = A;
	STATES(4);
}

INSTR(0x48, op_ldcb)			/* LD C,B */
{
	C = B;
	STATES(4);
}

INSTR(0x49, op_ldcc)			/* LD C,C */
{
	STATES(4);
}

INSTR(0x4a, op_ldcd)			/* LD C,D */
{
	C = D;
	STATES(4);
}

INSTR(0x4b, op_ldce)			/* LD C,E */
{
	C = E;
	STATES(4);
}

INSTR(0x4c, op_ldch)			/* LD C,H */
{
	C = H;
	STATES(4);
}

INSTR(0x4d, op_ldcl)			/* LD C,L */
{
	C = L;
	STATES(4);
}

INSTR(0x4e, op_ldchl)			/* LD C,(HL) */
{
	C = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x57, op_ldda)			/* LD D,A */
{
	D = A;
	STATES(4);
}

INSTR(0x50, op_lddb)			/* LD D,B */
{
	D = B;
	STATES(4);
}

INSTR(0x51, op_lddc)			/* LD D,C */
{
	D = C;
	STATES(4);
}

INSTR(0x52, op_lddd)			/* LD D,D */
{
	STATES(4);
}

INSTR(0x53, op_ldde)			/* LD D,E */
{
	D = E;
	STATES(4);
}

INSTR(0x54, op_lddh)			/* LD D,H */
{
	D = H;
	STATES(4);
}

INSTR(0x55, op_lddl)			/* LD D,L */
{
	D = L;
	STATES(4);
}

INSTR(0x56, op_lddhl)			/* LD D,(HL) */
{
	D = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x5f, op_ldea)			/* LD E,A */
{
	E = A;
	STATES(4);
}

INSTR(0x58, op_ldeb)			/* LD E,B */
{
	E = B;
	STATES(4);
}

INSTR(0x59, op_ldec)			/* LD E,C */
{
	E = C;
	STATES(4);
}

INSTR(0x5a, op_lded)			/* LD E,D */
{
	E = D;
	STATES(4);
}

INSTR(0x5b, op_ldee)			/* LD E,E */
{
	STATES(4);
}

INSTR(0x5c, op_ldeh)			/* LD E,H */
{
	E = H;
	STATES(4);
}

INSTR(0x5d, op_ldel)			/* LD E,L */
{
	E = L;
	STATES(4);
}

INSTR(0x5e, op_ldehl)			/* LD E,(HL) */
{
	E = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x67, op_ldha)			/* LD H,A */
{
	H = A;
	STATES(4);
}

INSTR(0x60, op_ldhb)			/* LD H,B */
{
	H = B;
	STATES(4);
}

INSTR(0x61, op_ldhc)			/* LD H,C */
{
	H = C;
	STATES(4);
}

INSTR(0x62, op_ldhd)			/* LD H,D */
{
	H = D;
	STATES(4);
}

INSTR(0x63, op_ldhe)			/* LD H,E */
{
	H = E;
	STATES(4);
}

INSTR(0x64, op_ldhh)			/* LD H,H */
{
	STATES(4);
}

INSTR(0x65, op_ldhl)			/* LD H,L */
{
	H = L;
	STATES(4);
}

INSTR(0x66, op_ldhhl)			/* LD H,(HL) */
{
	H = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x6f, op_ldla)			/* LD L,A */
{
	L = A;
	STATES(4);
}

INSTR(0x68, op_ldlb)			/* LD L,B */
{
	L = B;
	STATES(4);
}

INSTR(0x69, op_ldlc)			/* LD L,C */
{
	L = C;
	STATES(4);
}

INSTR(0x6a, op_ldld)			/* LD L,D */
{
	L = D;
	STATES(4);
}

INSTR(0x6b, op_ldle)			/* LD L,E */
{
	L = E;
	STATES(4);
}

INSTR(0x6c, op_ldlh)			/* LD L,H */
{
	L = H;
	STATES(4);
}

INSTR(0x6d, op_ldll)			/* LD L,L */
{
	STATES(4);
}

INSTR(0x6e, op_ldlhl)			/* LD L,(HL) */
{
	L = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x01, op_ldbcnn)			/* LD BC,nn */
{
	C = memrdr(PC++);
	B = memrdr(PC++);
	STATES(10);
}

INSTR(0x11, op_lddenn)			/* LD DE,nn */
{
	E = memrdr(PC++);
	D = memrdr(PC++);
	STATES(10);
}

INSTR(0x21, op_ldhlnn)			/* LD HL,nn */
{
	L = memrdr(PC++);
	H = memrdr(PC++);
	STATES(10);
}

INSTR(0x31, op_ldspnn)			/* LD SP,nn */
{
	SP = memrdr(PC++);
	SP += memrdr(PC++) << 8;
	STATES(10);
}

INSTR(0xf9, op_ldsphl)			/* LD SP,HL */
{
	SP = (H << 8) + L;
	STATES(6);
}

INSTR(0x2a, op_ldhlin)			/* LD HL,(nn) */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	L = memrdr(i++);
	H = memrdr(i);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(16);
}

INSTR(0x22, op_ldinhl)			/* LD (nn),HL */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i++, L);
	memwrt(i, H);
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(16);
}

INSTR(0x03, op_incbc)			/* INC BC */
{
	C++;
	if (!C)
		B++;
	STATES(6);
}

INSTR(0x13, op_incde)			/* INC DE */
{
	E++;
	if (!E)
		D++;
	STATES(6);
}

INSTR(0x23, op_inchl)			/* INC HL */
{
	L++;
	if (!L)
		H++;
	STATES(6);
}

INSTR(0x33, op_incsp)			/* INC SP */
{
	SP++;
	STATES(6);
}

INSTR(0x0b, op_decbc)			/* DEC BC */
{
	C--;
	if (C == 0xff)
		B--;
	STATES(6);
}

INSTR(0x1b, op_decde)			/* DEC DE */
{
	E--;
	if (E == 0xff)
		D--;
	STATES(6);
}

INSTR(0x2b, op_dechl)			/* DEC HL */
{
	L--;
	if (L == 0xff)
		H--;
	STATES(6);
}

INSTR(0x3b, op_decsp)			/* DEC SP */
{
	SP--;
	STATES(6);
}

INSTR(0x09, op_adhlbc)			/* ADD HL,BC */
{
	register int carry;

#ifdef UNDOC_FLAGS
	WZ = ((H << 8) + L) + 1;
#endif
	carry = (L + C > 255) ? 1 : 0;
	L += C;
	((H & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += B + carry;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(11);
}

INSTR(0x19, op_adhlde)			/* ADD HL,DE */
{
	register int carry;

#ifdef UNDOC_FLAGS
	WZ = ((H << 8) + L) + 1;
#endif
	carry = (L + E > 255) ? 1 : 0;
	L += E;
	((H & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += D + carry;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(11);
}

INSTR(0x29, op_adhlhl)			/* ADD HL,HL */
{
	register int carry;

#ifdef UNDOC_FLAGS
	WZ = ((H << 8) + L) + 1;
#endif
	carry = (L << 1 > 255) ? 1 : 0;
	L <<= 1;
	((H & 0xf) + (H & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + H + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += H + carry;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(11);
}

INSTR(0x39, op_adhlsp)			/* ADD HL,SP */
{
	register int carry;

	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;

#ifdef UNDOC_FLAGS
	WZ = ((H << 8) + L) + 1;
#endif
	carry = (L + spl > 255) ? 1 : 0;
	L += spl;
	((H & 0xf) + (sph & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(H + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += sph + carry;
	F &= ~N_FLAG;
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(11);
}

INSTR(0xa7, op_anda)			/* AND A */
{
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa0, op_andb)			/* AND B */
{
	A &= B;
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa1, op_andc)			/* AND C */
{
	A &= C;
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa2, op_andd)			/* AND D */
{
	A &= D;
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa3, op_ande)			/* AND E */
{
	A &= E;
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa4, op_andh)			/* AND H */
{
	A &= H;
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa5, op_andl)			/* AND L */
{
	A &= L;
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa6, op_andhl)			/* AND (HL) */
{
	A &= memrdr((H << 8) + L);
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(7);
}

INSTR(0xe6, op_andn)			/* AND n */
{
	A &= memrdr(PC++);
	F |= H_FLAG;
	F &= ~(N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(7);
}

INSTR(0xb7, op_ora)			/* OR A */
{
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb0, op_orb)			/* OR B */
{
	A |= B;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb1, op_orc)			/* OR C */
{
	A |= C;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb2, op_ord)			/* OR D */
{
	A |= D;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb3, op_ore)			/* OR E */
{
	A |= E;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb4, op_orh)			/* OR H */
{
	A |= H;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb5, op_orl)			/* OR L */
{
	A |= L;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xb6, op_orhl)			/* OR (HL) */
{
	A |= memrdr((H << 8) + L);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(7);
}

INSTR(0xf6, op_orn)			/* OR n */
{
	A |= memrdr(PC++);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(7);
}

INSTR(0xaf, op_xora)			/* XOR A */
{
	A = 0;
	F &= ~(S_FLAG | H_FLAG | N_FLAG | C_FLAG);
	F |= Z_FLAG | P_FLAG;
#ifdef UNDOC_FLAGS
	F &= ~(Y_FLAG | X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xa8, op_xorb)			/* XOR B */
{
	A ^= B;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xa9, op_xorc)			/* XOR C */
{
	A ^= C;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xaa, op_xord)			/* XOR D */
{
	A ^= D;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xab, op_xore)			/* XOR E */
{
	A ^= E;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xac, op_xorh)			/* XOR H */
{
	A ^= H;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xad, op_xorl)			/* XOR L */
{
	A ^= L;
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0xae, op_xorhl)			/* XOR (HL) */
{
	A ^= memrdr((H << 8) + L);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(7);
}

INSTR(0xee, op_xorn)			/* XOR n */
{
	A ^= memrdr(PC++);
	F &= ~(H_FLAG | N_FLAG | C_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(7);
}

INSTR(0x87, op_adda)			/* ADD A,A */
{
	register int i;

	((A & 0xf) + (A & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	((A << 1) > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) A;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x80, op_addb)			/* ADD A,B */
{
	register int i;

	((A & 0xf) + (B & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + B > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) B;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x81, op_addc)			/* ADD A,C */
{
	register int i;

	((A & 0xf) + (C & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + C > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) C;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x82, op_addd)			/* ADD A,D */
{
	register int i;

	((A & 0xf) + (D & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + D > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) D;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x83, op_adde)			/* ADD A,E */
{
	register int i;

	((A & 0xf) + (E & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + E > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) E;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x84, op_addh)			/* ADD A,H */
{
	register int i;

	((A & 0xf) + (H & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + H > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) H;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x85, op_addl)			/* ADD A,L */
{
	register int i;

	((A & 0xf) + (L & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + L > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) L;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x86, op_addhl)			/* ADD A,(HL) */
{
	register int i;
	register BYTE P;

	P = memrdr((H << 8) + L);
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0xc6, op_addn)			/* ADD A,n */
{
	register int i;
	register BYTE P;

	P = memrdr(PC++);
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0x8f, op_adca)			/* ADC A,A */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (A & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	((A << 1) + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) A + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x88, op_adcb)			/* ADC A,B */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) B + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x89, op_adcc)			/* ADC A,C */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (C & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + C + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) C + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x8a, op_adcd)			/* ADC A,D */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) D + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x8b, op_adce)			/* ADC A,E */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (E & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + E + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) E + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x8c, op_adch)			/* ADC A,H */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (H & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + H + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) H + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x8d, op_adcl)			/* ADC A,L */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (L & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + L + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) L + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x8e, op_adchl)			/* ADC A,(HL) */
{
	register int i, carry;
	register BYTE P;

	P = memrdr((H << 8) + L);
	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0xce, op_adcn)			/* ADC A,n */
{
	register int i, carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(PC++);
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A + (signed char) P + carry;
	F &= ~N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0x97, op_suba)			/* SUB A,A */
{
	A = 0;
	F &= ~(S_FLAG | H_FLAG | P_FLAG | C_FLAG);
	F |= Z_FLAG | N_FLAG;
#ifdef UNDOC_FLAGS
	F &= ~(Y_FLAG | X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x90, op_subb)			/* SUB A,B */
{
	register int i;

	((B & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(B > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) B;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x91, op_subc)			/* SUB A,C */
{
	register int i;

	((C & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(C > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) C;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x92, op_subd)			/* SUB A,D */
{
	register int i;

	((D & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(D > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) D;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x93, op_sube)			/* SUB A,E */
{
	register int i;

	((E & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(E > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) E;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x94, op_subh)			/* SUB A,H */
{
	register int i;

	((H & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) H;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x95, op_subl)			/* SUB A,L */
{
	register int i;

	((L & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(L > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) L;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x96, op_subhl)			/* SUB A,(HL) */
{
	register int i;
	register BYTE P;

	P = memrdr((H << 8) + L);
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0xd6, op_subn)			/* SUB A,n */
{
	register int i;
	register BYTE P;

	P = memrdr(PC++);
	((P & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0x9f, op_sbca)			/* SBC A,A */
{
	if (F & C_FLAG) {
		A = 255;
		F |= S_FLAG | H_FLAG | N_FLAG | C_FLAG;
		F &= ~(Z_FLAG | P_FLAG);
#ifdef UNDOC_FLAGS
		F |= Y_FLAG | X_FLAG;
#endif
	} else {
		A = 0;
		F |= Z_FLAG | N_FLAG;
		F &= ~(S_FLAG | H_FLAG | P_FLAG | C_FLAG);
#ifdef UNDOC_FLAGS
		F &= ~(Y_FLAG | X_FLAG);
#endif
	}
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x98, op_sbcb)			/* SBC A,B */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((B & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(B + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) B - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x99, op_sbcc)			/* SBC A,C */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((C & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(C + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) C - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x9a, op_sbcd)			/* SBC A,D */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((D & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(D + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) D - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x9b, op_sbce)			/* SBC A,E */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((E & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(E + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) E - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x9c, op_sbch)			/* SBC A,H */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((H & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) H - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x9d, op_sbcl)			/* SBC A,L */
{
	register int i, carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((L & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(L + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) L - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(4);
}

INSTR(0x9e, op_sbchl)			/* SBC A,(HL) */
{
	register int i, carry;
	register BYTE P;

	P = memrdr((H << 8) + L);
	carry = (F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0xde, op_sbcn)			/* SBC A,n */
{
	register int i, carry;
	register BYTE P;

	P = memrdr(PC++);
	carry = (F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i = (signed char) A - (signed char) P - carry;
	F |= N_FLAG;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(i & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(i & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
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
	STATES(7);
}

INSTR(0xbf, op_cpa)			/* CP A */
{
	F &= ~(S_FLAG | H_FLAG | P_FLAG | C_FLAG);
	F |= Z_FLAG | N_FLAG;
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xb8, op_cpb)			/* CP B */
{
	register int i;

	((B & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(B > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) B;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xb9, op_cpc)			/* CP C */
{
	register int i;

	((C & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(C > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) C;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xba, op_cpd)			/* CP D */
{
	register int i;

	((D & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(D > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) D;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xbb, op_cpe)			/* CP E */
{
	register int i;

	((E & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(E > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) E;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xbc, op_cph)			/* CP H */
{
	register int i;

	((H & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) H;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xbd, op_cplr)			/* CP L */
{
	register int i;

	((L & 0xf) > (A & 0xf)) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(L > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = (signed char) A - (signed char) L;
	(i < -128 || i > 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xbe, op_cphl)			/* CP (HL) */
{
	register int i;
	register BYTE P;

	P = memrdr((H << 8) + L);
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
	STATES(7);
}

INSTR(0xfe, op_cpn)			/* CP n */
{
	register int i;
	register BYTE P;

	P = memrdr(PC++);
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
	STATES(7);
}

INSTR(0x3c, op_inca)			/* INC A */
{
	A++;
	((A & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(A == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0x04, op_incb)			/* INC B */
{
	B++;
	((B & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(B == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[B];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x0c, op_incc)			/* INC C */
{
	C++;
	((C & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(C == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[C];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x14, op_incd)			/* INC D */
{
	D++;
	((D & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(D == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[D];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x1c, op_ince)			/* INC E */
{
	E++;
	((E & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(E == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[E];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x24, op_inch)			/* INC H */
{
	H++;
	((H & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(H == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[H];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x2c, op_incl)			/* INC L */
{
	L++;
	((L & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(L == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[L];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x34, op_incihl)			/* INC (HL) */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	P++;
	memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F &= ~N_FLAG;
	(P == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[P];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(11);
}

INSTR(0x3d, op_deca)			/* DEC A */
{
	A--;
	((A & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(A == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
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
	STATES(4);
}

INSTR(0x05, op_decb)			/* DEC B */
{
	B--;
	((B & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(B == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(B & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(B & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[B];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[B];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x0d, op_decc)			/* DEC C */
{
	C--;
	((C & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(C == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(C & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(C & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[C];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[C];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x15, op_decd)			/* DEC D */
{
	D--;
	((D & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(D == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(D & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(D & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[D];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[D];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x1d, op_dece)			/* DEC E */
{
	E--;
	((E & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(E == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(E & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(E & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[E];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[E];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x25, op_dech)			/* DEC H */
{
	H--;
	((H & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(H == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(H & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(H & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[H];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[H];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x2d, op_decl)			/* DEC L */
{
	L--;
	((L & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(L == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(L & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(L & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[L];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[L];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x35, op_decihl)			/* DEC (HL) */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	F |= N_FLAG;
	(P == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
#ifndef FLAG_TABLES
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#ifdef UNDOC_FLAGS
	(P & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(P & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
#endif
#else /* FLAG_TABLES */
#ifndef UNDOC_FLAGS
	F = (F & ~SZ_FLAGS) | sz_flags[P];
#else
	F = (F & ~SZYX_FLAGS) | szyx_flags[P];
#endif
#endif /* FLAG_TABLES */
#ifdef UNDOC_FLAGS
	modF = 1;
#endif
	STATES(11);
}

INSTR(0x07, op_rlca)			/* RLCA */
{
	register int i;

	i = (A & 128) ? 1 : 0;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A <<= 1;
	A |= i;
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x0f, op_rrca)			/* RRCA */
{
	register int i;

	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A >>= 1;
	if (i) A |= 128;
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x17, op_rla)			/* RLA */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0x1f, op_rra)			/* RRA */
{
	register int i, old_c_flag;

	old_c_flag = F & C_FLAG;
	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG | N_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
#ifdef UNDOC_FLAGS
	(A & 32) ? (F |= Y_FLAG) : (F &= ~Y_FLAG);
	(A & 8) ? (F |= X_FLAG) : (F &= ~X_FLAG);
	modF = 1;
#endif
	STATES(4);
}

INSTR(0xeb, op_exdehl)			/* EX DE,HL */
{
	register BYTE i;

	i = D;
	D = H;
	H = i;
	i = E;
	E = L;
	L = i;
	STATES(4);
}

INSTR(0x08, op_exafaf)			/* EX AF,AF' */
{
	register BYTE i;

	i = A;
	A = A_;
	A_ = i;
	i = F;
	F = F_;
	F_ = i;
	STATES(4);
}

INSTR(0xd9, op_exx)			/* EXX */
{
	register BYTE i;

	i = B;
	B = B_;
	B_ = i;
	i = C;
	C = C_;
	C_ = i;
	i = D;
	D = D_;
	D_ = i;
	i = E;
	E = E_;
	E_ = i;
	i = H;
	H = H_;
	H_ = i;
	i = L;
	L = L_;
	L_ = i;
	STATES(4);
}

INSTR(0xe3, op_exsphl)			/* EX (SP),HL */
{
	register BYTE i;

	i = memrdr(SP);
	memwrt(SP, L);
	L = i;
	i = memrdr(SP + 1);
	memwrt(SP + 1, H);
	H = i;
#ifdef UNDOC_FLAGS
	WZ = (H << 8) + L;
#endif
	STATES(19);
}

INSTR(0xf5, op_pushaf)			/* PUSH AF */
{
	memwrt(--SP, A);
	memwrt(--SP, F);
	STATES(11);
}

INSTR(0xc5, op_pushbc)			/* PUSH BC */
{
	memwrt(--SP, B);
	memwrt(--SP, C);
	STATES(11);
}

INSTR(0xd5, op_pushde)			/* PUSH DE */
{
	memwrt(--SP, D);
	memwrt(--SP, E);
	STATES(11);
}

INSTR(0xe5, op_pushhl)			/* PUSH HL */
{
	memwrt(--SP, H);
	memwrt(--SP, L);
	STATES(11);
}

INSTR(0xf1, op_popaf)			/* POP AF */
{
	F = memrdr(SP++);
	A = memrdr(SP++);
	STATES(10);
}

INSTR(0xc1, op_popbc)			/* POP BC */
{
	C = memrdr(SP++);
	B = memrdr(SP++);
	STATES(10);
}

INSTR(0xd1, op_popde)			/* POP DE */
{
	E = memrdr(SP++);
	D = memrdr(SP++);
	STATES(10);
}

INSTR(0xe1, op_pophl)			/* POP HL */
{
	L = memrdr(SP++);
	H = memrdr(SP++);
	STATES(10);
}

INSTR(0xc3, op_jp)			/* JP nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC) << 8;
	PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xe9, op_jphl)			/* JP (HL) */
{
	PC = (H << 8) + L;
	STATES(4);
}

INSTR(0x18, op_jr)			/* JR n */
{
	register int d;

	d = (signed char) memrdr(PC++);
	PC += d;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(12);
}

INSTR(0x10, op_djnz)			/* DJNZ n */
{
	register int d;

	d = (signed char) memrdr(PC++);
	if (--B) {
		PC += d;
#ifdef UNDOC_FLAGS
		WZ = PC;
#endif
		STATES(13);
	}
	STATES(5);
}

INSTR(0xcd, op_call)			/* CALL nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(17);
}

INSTR(0xc9, op_ret)			/* RET */
{
	register WORD i;

	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xca, op_jpz)			/* JP Z,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & Z_FLAG)
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xc2, op_jpnz)			/* JP NZ,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & Z_FLAG))
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xda, op_jpc)			/* JP C,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & C_FLAG)
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xd2, op_jpnc)			/* JP NC,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & C_FLAG))
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xea, op_jppe)			/* JP PE,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & P_FLAG)
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xe2, op_jppo)			/* JP PO,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & P_FLAG))
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xfa, op_jpm)			/* JP M,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & S_FLAG)
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xf2, op_jpp)			/* JP P,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & S_FLAG))
		PC = i;
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xcc, op_calz)			/* CALL Z,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & Z_FLAG) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xc4, op_calnz)			/* CALL NZ,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & Z_FLAG)) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xdc, op_calc)			/* CALL C,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & C_FLAG) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xd4, op_calnc)			/* CALL NC,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & C_FLAG)) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xec, op_calpe)			/* CALL PE,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & P_FLAG) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xe4, op_calpo)			/* CALL PO,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & P_FLAG)) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xfc, op_calm)			/* CALL M,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & S_FLAG) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xf4, op_calp)			/* CALL P,nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & S_FLAG)) {
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
#ifdef UNDOC_FLAGS
	WZ = i;
#endif
	STATES(10);
}

INSTR(0xc8, op_retz)			/* RET Z */
{
	register WORD i;

	if (F & Z_FLAG) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0xc0, op_retnz)			/* RET NZ */
{
	register WORD i;

	if (!(F & Z_FLAG)) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0xd8, op_retc)			/* RET C */
{
	register WORD i;

	if (F & C_FLAG) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
		STATES(11);
#endif
	}
	STATES(5);
}

INSTR(0xd0, op_retnc)			/* RET NC */
{
	register WORD i;

	if (!(F & C_FLAG)) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0xe8, op_retpe)			/* RET PE */
{
	register WORD i;

	if (F & P_FLAG) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0xe0, op_retpo)			/* RET PO */
{
	register WORD i;

	if (!(F & P_FLAG)) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0xf8, op_retm)			/* RET M */
{
	register WORD i;

	if (F & S_FLAG) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0xf0, op_retp)			/* RET P */
{
	register WORD i;

	if (!(F & S_FLAG)) {
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
#ifdef UNDOC_FLAGS
		WZ = i;
#endif
		STATES(11);
	}
	STATES(5);
}

INSTR(0x28, op_jrz)			/* JR Z,n */
{
	register int d;

	d = (signed char) memrdr(PC++);
	if (F & Z_FLAG) {
		PC += d;
#ifdef UNDOC_FLAGS
		WZ = PC;
#endif
		STATES(12);
	}
	STATES(7);
}

INSTR(0x20, op_jrnz)			/* JR NZ,n */
{
	register int d;

	d = (signed char) memrdr(PC++);
	if (!(F & Z_FLAG)) {
		PC += d;
#ifdef UNDOC_FLAGS
		WZ = PC;
#endif
		STATES(12);
	}
	STATES(7);
}

INSTR(0x38, op_jrc)			/* JR C,n */
{
	register int d;

	d = (signed char) memrdr(PC++);
	if (F & C_FLAG) {
		PC += d;
#ifdef UNDOC_FLAGS
		WZ = PC;
#endif
		STATES(12);
	}
	STATES(7);
}

INSTR(0x30, op_jrnc)			/* JR NC,n */
{
	register int d;

	d = (signed char) memrdr(PC++);
	if (!(F & C_FLAG)) {
		PC += d;
#ifdef UNDOC_FLAGS
		WZ = PC;
#endif
		STATES(12);
	}
	STATES(7);
}

INSTR(0xc7, op_rst00)			/* RST 00 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xcf, op_rst08)			/* RST 08 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x08;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xd7, op_rst10)			/* RST 10 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x10;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xdf, op_rst18)			/* RST 18 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x18;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xe7, op_rst20)			/* RST 20 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x20;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xef, op_rst28)			/* RST 28 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x28;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xf7, op_rst30)			/* RST 30 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x30;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xff, op_rst38)			/* RST 38 */
{
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x38;
#ifdef UNDOC_FLAGS
	WZ = PC;
#endif
	STATES(11);
}

INSTR(0xcb, op_cb_handle)		/* 0xcb prefix */
{
#ifndef INSTR_SWTCH

#ifdef UNDOC_INST
#define UNDOC(f) f
#else
#define UNDOC(f) trap_cb
#endif

	static int (*op_cb[256])(void) = {
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
		op_undoc_slla,			/* 0x37 */
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

#undef UNDOC

	register int t;

#endif /* !INSTR_SWTCH */

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	if (F_flag) {
		/* update frontpanel */
		fp_clock++;
		fp_sampleLightGroup(0, 0);
	}
#endif

	R++;				/* increment refresh register */

#ifndef INSTR_SWTCH
	t = (*op_cb[memrdr(PC++)])();	/* execute next opcode */
#else
	switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-cb.c"

	default:
		t = trap_cb();
		break;
	}
#endif

	STATES(t);
}

#ifndef INSTR_SWTCH
#include "simz80-cb.c"
#endif

INSTR(0xdd, op_dd_handle)		/* 0xdd prefix */
{
#ifndef INSTR_SWTCH

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

#endif /* !INSTR_SWTCH */

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	if (F_flag) {
		/* update frontpanel */
		fp_clock++;
		fp_sampleLightGroup(0, 0);
	}
#endif

	R++;				/* increment refresh register */

#ifndef INSTR_SWTCH
	t = (*op_dd[memrdr(PC++)])();	/* execute next opcode */
#else
	switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-dd.c"

	default:
		t = trap_dd();
		break;
	}
#endif

	STATES(t);
}

#ifndef INSTR_SWTCH
#include "simz80-dd.c"
#endif

INSTR(0xed, op_ed_handle)		/* 0xed prefix */
{
#ifndef INSTR_SWTCH

#ifdef UNDOC_INST
#define UNDOC(f) f
#ifdef UNDOC_IALL
#define UNDOCA(f) f
#else
#define UNDOCA(f) trap_ed
#endif
#else
#define UNDOC(f) trap_ed
#define UNDOCA(f) trap_ed
#endif

	static int (*op_ed[256])(void) = {
		UNDOCA(op_undoc_nop),		/* 0x00 */
		UNDOCA(op_undoc_nop),		/* 0x01 */
		UNDOCA(op_undoc_nop),		/* 0x02 */
		UNDOCA(op_undoc_nop),		/* 0x03 */
		UNDOCA(op_undoc_nop),		/* 0x04 */
		UNDOCA(op_undoc_nop),		/* 0x05 */
		UNDOCA(op_undoc_nop),		/* 0x06 */
		UNDOCA(op_undoc_nop),		/* 0x07 */
		UNDOCA(op_undoc_nop),		/* 0x08 */
		UNDOCA(op_undoc_nop),		/* 0x09 */
		UNDOCA(op_undoc_nop),		/* 0x0a */
		UNDOCA(op_undoc_nop),		/* 0x0b */
		UNDOCA(op_undoc_nop),		/* 0x0c */
		UNDOCA(op_undoc_nop),		/* 0x0d */
		UNDOCA(op_undoc_nop),		/* 0x0e */
		UNDOCA(op_undoc_nop),		/* 0x0f */
		UNDOCA(op_undoc_nop),		/* 0x10 */
		UNDOCA(op_undoc_nop),		/* 0x11 */
		UNDOCA(op_undoc_nop),		/* 0x12 */
		UNDOCA(op_undoc_nop),		/* 0x13 */
		UNDOCA(op_undoc_nop),		/* 0x14 */
		UNDOCA(op_undoc_nop),		/* 0x15 */
		UNDOCA(op_undoc_nop),		/* 0x16 */
		UNDOCA(op_undoc_nop),		/* 0x17 */
		UNDOCA(op_undoc_nop),		/* 0x18 */
		UNDOCA(op_undoc_nop),		/* 0x19 */
		UNDOCA(op_undoc_nop),		/* 0x1a */
		UNDOCA(op_undoc_nop),		/* 0x1b */
		UNDOCA(op_undoc_nop),		/* 0x1c */
		UNDOCA(op_undoc_nop),		/* 0x1d */
		UNDOCA(op_undoc_nop),		/* 0x1e */
		UNDOCA(op_undoc_nop),		/* 0x1f */
		UNDOCA(op_undoc_nop),		/* 0x20 */
		UNDOCA(op_undoc_nop),		/* 0x21 */
		UNDOCA(op_undoc_nop),		/* 0x22 */
		UNDOCA(op_undoc_nop),		/* 0x23 */
		UNDOCA(op_undoc_nop),		/* 0x24 */
		UNDOCA(op_undoc_nop),		/* 0x25 */
		UNDOCA(op_undoc_nop),		/* 0x26 */
		UNDOCA(op_undoc_nop),		/* 0x27 */
		UNDOCA(op_undoc_nop),		/* 0x28 */
		UNDOCA(op_undoc_nop),		/* 0x29 */
		UNDOCA(op_undoc_nop),		/* 0x2a */
		UNDOCA(op_undoc_nop),		/* 0x2b */
		UNDOCA(op_undoc_nop),		/* 0x2c */
		UNDOCA(op_undoc_nop),		/* 0x2d */
		UNDOCA(op_undoc_nop),		/* 0x2e */
		UNDOCA(op_undoc_nop),		/* 0x2f */
		UNDOCA(op_undoc_nop),		/* 0x30 */
		UNDOCA(op_undoc_nop),		/* 0x31 */
		UNDOCA(op_undoc_nop),		/* 0x32 */
		UNDOCA(op_undoc_nop),		/* 0x33 */
		UNDOCA(op_undoc_nop),		/* 0x34 */
		UNDOCA(op_undoc_nop),		/* 0x35 */
		UNDOCA(op_undoc_nop),		/* 0x36 */
		UNDOCA(op_undoc_nop),		/* 0x37 */
		UNDOCA(op_undoc_nop),		/* 0x38 */
		UNDOCA(op_undoc_nop),		/* 0x39 */
		UNDOCA(op_undoc_nop),		/* 0x3a */
		UNDOCA(op_undoc_nop),		/* 0x3b */
		UNDOCA(op_undoc_nop),		/* 0x3c */
		UNDOCA(op_undoc_nop),		/* 0x3d */
		UNDOCA(op_undoc_nop),		/* 0x3e */
		UNDOCA(op_undoc_nop),		/* 0x3f */
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
		UNDOCA(op_undoc_neg),		/* 0x4c */
		op_reti,			/* 0x4d */
		UNDOCA(op_undoc_im0),		/* 0x4e */
		op_ldra,			/* 0x4f */
		op_indic,			/* 0x50 */
		op_outcd,			/* 0x51 */
		op_sbchd,			/* 0x52 */
		op_ldinde,			/* 0x53 */
		UNDOCA(op_undoc_neg),		/* 0x54 */
		UNDOCA(op_undoc_retn),		/* 0x55 */
		op_im1,				/* 0x56 */
		op_ldai,			/* 0x57 */
		op_ineic,			/* 0x58 */
		op_outce,			/* 0x59 */
		op_adchd,			/* 0x5a */
		op_lddeinn,			/* 0x5b */
		UNDOCA(op_undoc_neg),		/* 0x5c */
		UNDOCA(op_undoc_reti),		/* 0x5d */
		op_im2,				/* 0x5e */
		op_ldar,			/* 0x5f */
		op_inhic,			/* 0x60 */
		op_outch,			/* 0x61 */
		op_sbchh,			/* 0x62 */
		op_ldinhl2,			/* 0x63 */
		UNDOCA(op_undoc_neg),		/* 0x64 */
		UNDOCA(op_undoc_retn),		/* 0x65 */
		UNDOCA(op_undoc_im0),		/* 0x66 */
		op_oprrd,			/* 0x67 */
		op_inlic,			/* 0x68 */
		op_outcl,			/* 0x69 */
		op_adchh,			/* 0x6a */
		op_ldhlinn,			/* 0x6b */
		UNDOCA(op_undoc_neg),		/* 0x6c */
		UNDOCA(op_undoc_reti),		/* 0x6d */
		UNDOCA(op_undoc_im0),		/* 0x6e */
		op_oprld,			/* 0x6f */
		UNDOC(op_undoc_infic),		/* 0x70 */
		UNDOC(op_undoc_outc0),		/* 0x71 */
		op_sbchs,			/* 0x72 */
		op_ldinsp,			/* 0x73 */
		UNDOCA(op_undoc_neg),		/* 0x74 */
		UNDOCA(op_undoc_retn),		/* 0x75 */
		UNDOCA(op_undoc_im1),		/* 0x76 */
		UNDOCA(op_undoc_nop),		/* 0x77 */
		op_inaic,			/* 0x78 */
		op_outca,			/* 0x79 */
		op_adchs,			/* 0x7a */
		op_ldspinn,			/* 0x7b */
		UNDOCA(op_undoc_neg),		/* 0x7c */
		UNDOCA(op_undoc_reti),		/* 0x7d */
		UNDOCA(op_undoc_im2),		/* 0x7e */
		UNDOCA(op_undoc_nop),		/* 0x7f */
		UNDOCA(op_undoc_nop),		/* 0x80 */
		UNDOCA(op_undoc_nop),		/* 0x81 */
		UNDOCA(op_undoc_nop),		/* 0x82 */
		UNDOCA(op_undoc_nop),		/* 0x83 */
		UNDOCA(op_undoc_nop),		/* 0x84 */
		UNDOCA(op_undoc_nop),		/* 0x85 */
		UNDOCA(op_undoc_nop),		/* 0x86 */
		UNDOCA(op_undoc_nop),		/* 0x87 */
		UNDOCA(op_undoc_nop),		/* 0x88 */
		UNDOCA(op_undoc_nop),		/* 0x89 */
		UNDOCA(op_undoc_nop),		/* 0x8a */
		UNDOCA(op_undoc_nop),		/* 0x8b */
		UNDOCA(op_undoc_nop),		/* 0x8c */
		UNDOCA(op_undoc_nop),		/* 0x8d */
		UNDOCA(op_undoc_nop),		/* 0x8e */
		UNDOCA(op_undoc_nop),		/* 0x8f */
		UNDOCA(op_undoc_nop),		/* 0x90 */
		UNDOCA(op_undoc_nop),		/* 0x91 */
		UNDOCA(op_undoc_nop),		/* 0x92 */
		UNDOCA(op_undoc_nop),		/* 0x93 */
		UNDOCA(op_undoc_nop),		/* 0x94 */
		UNDOCA(op_undoc_nop),		/* 0x95 */
		UNDOCA(op_undoc_nop),		/* 0x96 */
		UNDOCA(op_undoc_nop),		/* 0x97 */
		UNDOCA(op_undoc_nop),		/* 0x98 */
		UNDOCA(op_undoc_nop),		/* 0x99 */
		UNDOCA(op_undoc_nop),		/* 0x9a */
		UNDOCA(op_undoc_nop),		/* 0x9b */
		UNDOCA(op_undoc_nop),		/* 0x9c */
		UNDOCA(op_undoc_nop),		/* 0x9d */
		UNDOCA(op_undoc_nop),		/* 0x9e */
		UNDOCA(op_undoc_nop),		/* 0x9f */
		op_ldi,				/* 0xa0 */
		op_cpi,				/* 0xa1 */
		op_ini,				/* 0xa2 */
		op_outi,			/* 0xa3 */
		UNDOCA(op_undoc_nop),		/* 0xa4 */
		UNDOCA(op_undoc_nop),		/* 0xa5 */
		UNDOCA(op_undoc_nop),		/* 0xa6 */
		UNDOCA(op_undoc_nop),		/* 0xa7 */
		op_ldd,				/* 0xa8 */
		op_cpdop,			/* 0xa9 */
		op_ind,				/* 0xaa */
		op_outd,			/* 0xab */
		UNDOCA(op_undoc_nop),		/* 0xac */
		UNDOCA(op_undoc_nop),		/* 0xad */
		UNDOCA(op_undoc_nop),		/* 0xae */
		UNDOCA(op_undoc_nop),		/* 0xaf */
		op_ldir,			/* 0xb0 */
		op_cpir,			/* 0xb1 */
		op_inir,			/* 0xb2 */
		op_otir,			/* 0xb3 */
		UNDOCA(op_undoc_nop),		/* 0xb4 */
		UNDOCA(op_undoc_nop),		/* 0xb5 */
		UNDOCA(op_undoc_nop),		/* 0xb6 */
		UNDOCA(op_undoc_nop),		/* 0xb7 */
		op_lddr,			/* 0xb8 */
		op_cpdr,			/* 0xb9 */
		op_indr,			/* 0xba */
		op_otdr,			/* 0xbb */
		UNDOCA(op_undoc_nop),		/* 0xbc */
		UNDOCA(op_undoc_nop),		/* 0xbd */
		UNDOCA(op_undoc_nop),		/* 0xbe */
		UNDOCA(op_undoc_nop),		/* 0xbf */
		UNDOCA(op_undoc_nop),		/* 0xc0 */
		UNDOCA(op_undoc_nop),		/* 0xc1 */
		UNDOCA(op_undoc_nop),		/* 0xc2 */
		UNDOCA(op_undoc_nop),		/* 0xc3 */
		UNDOCA(op_undoc_nop),		/* 0xc4 */
		UNDOCA(op_undoc_nop),		/* 0xc5 */
		UNDOCA(op_undoc_nop),		/* 0xc6 */
		UNDOCA(op_undoc_nop),		/* 0xc7 */
		UNDOCA(op_undoc_nop),		/* 0xc8 */
		UNDOCA(op_undoc_nop),		/* 0xc9 */
		UNDOCA(op_undoc_nop),		/* 0xca */
		UNDOCA(op_undoc_nop),		/* 0xcb */
		UNDOCA(op_undoc_nop),		/* 0xcc */
		UNDOCA(op_undoc_nop),		/* 0xcd */
		UNDOCA(op_undoc_nop),		/* 0xce */
		UNDOCA(op_undoc_nop),		/* 0xcf */
		UNDOCA(op_undoc_nop),		/* 0xd0 */
		UNDOCA(op_undoc_nop),		/* 0xd1 */
		UNDOCA(op_undoc_nop),		/* 0xd2 */
		UNDOCA(op_undoc_nop),		/* 0xd3 */
		UNDOCA(op_undoc_nop),		/* 0xd4 */
		UNDOCA(op_undoc_nop),		/* 0xd5 */
		UNDOCA(op_undoc_nop),		/* 0xd6 */
		UNDOCA(op_undoc_nop),		/* 0xd7 */
		UNDOCA(op_undoc_nop),		/* 0xd8 */
		UNDOCA(op_undoc_nop),		/* 0xd9 */
		UNDOCA(op_undoc_nop),		/* 0xda */
		UNDOCA(op_undoc_nop),		/* 0xdb */
		UNDOCA(op_undoc_nop),		/* 0xdc */
		UNDOCA(op_undoc_nop),		/* 0xdd */
		UNDOCA(op_undoc_nop),		/* 0xde */
		UNDOCA(op_undoc_nop),		/* 0xdf */
		UNDOCA(op_undoc_nop),		/* 0xe0 */
		UNDOCA(op_undoc_nop),		/* 0xe1 */
		UNDOCA(op_undoc_nop),		/* 0xe2 */
		UNDOCA(op_undoc_nop),		/* 0xe3 */
		UNDOCA(op_undoc_nop),		/* 0xe4 */
		UNDOCA(op_undoc_nop),		/* 0xe5 */
		UNDOCA(op_undoc_nop),		/* 0xe6 */
		UNDOCA(op_undoc_nop),		/* 0xe7 */
		UNDOCA(op_undoc_nop),		/* 0xe8 */
		UNDOCA(op_undoc_nop),		/* 0xe9 */
		UNDOCA(op_undoc_nop),		/* 0xea */
		UNDOCA(op_undoc_nop),		/* 0xeb */
		UNDOCA(op_undoc_nop),		/* 0xec */
		UNDOCA(op_undoc_nop),		/* 0xed */
		UNDOCA(op_undoc_nop),		/* 0xee */
		UNDOCA(op_undoc_nop),		/* 0xef */
		UNDOCA(op_undoc_nop),		/* 0xf0 */
		UNDOCA(op_undoc_nop),		/* 0xf1 */
		UNDOCA(op_undoc_nop),		/* 0xf2 */
		UNDOCA(op_undoc_nop),		/* 0xf3 */
		UNDOCA(op_undoc_nop),		/* 0xf4 */
		UNDOCA(op_undoc_nop),		/* 0xf5 */
		UNDOCA(op_undoc_nop),		/* 0xf6 */
		UNDOCA(op_undoc_nop),		/* 0xf7 */
		UNDOCA(op_undoc_nop),		/* 0xf8 */
		UNDOCA(op_undoc_nop),		/* 0xf9 */
		UNDOCA(op_undoc_nop),		/* 0xfa */
		UNDOCA(op_undoc_nop),		/* 0xfb */
		UNDOCA(op_undoc_nop),		/* 0xfc */
		UNDOCA(op_undoc_nop),		/* 0xfd */
		UNDOCA(op_undoc_nop),		/* 0xfe */
		UNDOCA(op_undoc_nop)		/* 0xff */
	};

#undef UNDOC
#undef UNDOCA

	register int t;

#endif /* !INSTR_SWTCH */

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	if (F_flag) {
		/* update frontpanel */
		fp_clock++;
		fp_sampleLightGroup(0, 0);
	}
#endif

	R++;				/* increment refresh register */

#ifndef INSTR_SWTCH
	t = (*op_ed[memrdr(PC++)])();	/* execute next opcode */
#else
	switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-ed.c"

	default:
		t = trap_ed();
		break;
	}
#endif

	STATES(t);
}

#ifndef INSTR_SWTCH
#include "simz80-ed.c"
#endif

INSTR(0xfd, op_fd_handle)		/* 0xfd prefix */
{
#ifndef INSTR_SWTCH

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

#endif /* !INSTR_SWTCH */

#ifdef BUS_8080
	/* M1 opcode fetch */
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
	m1_step = 1;
#endif
#ifdef FRONTPANEL
	if (F_flag) {
		/* update frontpanel */
		fp_clock++;
		fp_sampleLightGroup(0, 0);
	}
#endif

	R++;				/* increment refresh register */

#ifndef INSTR_SWTCH
	t = (*op_fd[memrdr(PC++)])();	/* execute next opcode */
#else
	switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-fd.c"

	default:
		t = trap_fd();
		break;
	}
#endif

	STATES(t);
}

#ifndef INSTR_SWTCH
#include "simz80-fd.c"
#endif
