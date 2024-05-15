#ifndef FAST_INSTR
#define INSTR(opcode, func)	static int func(void)
#define STATES(states)		return (states);
#else
#define INSTR(opcode, func)	case opcode:
#define STATES(states)		t = states; break;
#endif

INSTR(0x00, op_nop)			/* NOP */
{
	STATES(4);
}

INSTR(0x76, op_hlt)			/* HLT */
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
			/* else wait for INT or user interrupt */
			while ((int_int == 0) && (cpu_state == CONTIN_RUN)) {
				SLEEP_MS(1);
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
			/* INT disabled, wait for frontpanel reset or user interrupt */
			while (!(cpu_state & RESET)) {
				fp_clock++;
				fp_sampleData();
				SLEEP_MS(1);
				if (cpu_error != NONE)
					break;
			}
		} else {
			/* else wait for INT,
			   frontpanel reset or user interrupt */
			while ((int_int == 0) && !(cpu_state & RESET)) {
				fp_clock++;
				fp_sampleData();
				SLEEP_MS(1);
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

	STATES(7);
}

INSTR(0x37, op_stc)			/* STC */
{
	F |= C_FLAG;
	STATES(4);
}

INSTR(0x3f, op_cmc)			/* CMC */
{
	if (F & C_FLAG)
		F &= ~C_FLAG;
	else
		F |= C_FLAG;
	STATES(4);
}

INSTR(0x2f, op_cma)			/* CMA */
{
	A = ~A;
	STATES(4);
}

INSTR(0x27, op_daa)			/* DAA */
{
	register int tmp_a = A;

	if (((A & 0xf) > 9) || (F & H_FLAG)) {
		((A & 0xf) > 9) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		tmp_a += 6;
	}
	if (((tmp_a & 0x1f0) > 0x90) || (F & C_FLAG)) {
		tmp_a += 0x60;
	}
	if (tmp_a & 0x100)
		(F |= C_FLAG);
	A = tmp_a & 0xff;
#ifndef FAST_INSTR
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
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

INSTR(0xdb, op_in)			/* IN n */
{
	extern BYTE io_in(BYTE, BYTE);
	BYTE addr;

	addr = memrdr(PC++);
	A = io_in(addr, addr);
	STATES(10);
}

INSTR(0xd3, op_out)			/* OUT n */
{
	extern void io_out(BYTE, BYTE, BYTE);
	BYTE addr;

	addr = memrdr(PC++);
	io_out(addr, addr, A);
	STATES(10);
}

INSTR(0x3e, op_mvian)			/* MVI A,n */
{
	A = memrdr(PC++);
	STATES(7);
}

INSTR(0x06, op_mvibn)			/* MVI B,n */
{
	B = memrdr(PC++);
	STATES(7);
}

INSTR(0x0e, op_mvicn)			/* MVI C,n */
{
	C = memrdr(PC++);
	STATES(7);
}

INSTR(0x16, op_mvidn)			/* MVI D,n */
{
	D = memrdr(PC++);
	STATES(7);
}

INSTR(0x1e, op_mvien)			/* MVI E,n */
{
	E = memrdr(PC++);
	STATES(7);
}

INSTR(0x26, op_mvihn)			/* MVI H,n */
{
	H = memrdr(PC++);
	STATES(7);
}

INSTR(0x2e, op_mviln)			/* MVI L,n */
{
	L = memrdr(PC++);
	STATES(7);
}

INSTR(0x0a, op_ldaxb)			/* LDAX B */
{
	A = memrdr((B << 8) + C);
	STATES(7);
}

INSTR(0x1a, op_ldaxd)			/* LDAX D */
{
	A = memrdr((D << 8) + E);
	STATES(7);
}

INSTR(0x3a, op_ldann)			/* LDA nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	A = memrdr(i);
	STATES(13);
}

INSTR(0x02, op_staxb)			/* STAX B */
{
	memwrt((B << 8) + C, A);
	STATES(7);
}

INSTR(0x12, op_staxd)			/* STAX D */
{
	memwrt((D << 8) + E, A);
	STATES(7);
}

INSTR(0x32, op_stann)			/* STA nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, A);
	STATES(13);
}

INSTR(0x77, op_movma)			/* MOV M,A */
{
	memwrt((H << 8) + L, A);
	STATES(7);
}

INSTR(0x70, op_movmb)			/* MOV M,B */
{
	memwrt((H << 8) + L, B);
	STATES(7);
}

INSTR(0x71, op_movmc)			/* MOV M,C */
{
	memwrt((H << 8) + L, C);
	STATES(7);
}

INSTR(0x72, op_movmd)			/* MOV M,D */
{
	memwrt((H << 8) + L, D);
	STATES(7);
}

INSTR(0x73, op_movme)			/* MOV M,E */
{
	memwrt((H << 8) + L, E);
	STATES(7);
}

INSTR(0x74, op_movmh)			/* MOV M,H */
{
	memwrt((H << 8) + L, H);
	STATES(7);
}

INSTR(0x75, op_movml)			/* MOV M,L */
{
	memwrt((H << 8) + L, L);
	STATES(7);
}

INSTR(0x36, op_mvimn)			/* MVI M,n */
{
	memwrt((H << 8) + L, memrdr(PC++));
	STATES(10);
}

INSTR(0x7f, op_movaa)			/* MOV A,A */
{
	STATES(5);
}

INSTR(0x78, op_movab)			/* MOV A,B */
{
	A = B;
	STATES(5);
}

INSTR(0x79, op_movac)			/* MOV A,C */
{
	A = C;
	STATES(5);
}

INSTR(0x7a, op_movad)			/* MOV A,D */
{
	A = D;
	STATES(5);
}

INSTR(0x7b, op_movae)			/* MOV A,E */
{
	A = E;
	STATES(5);
}

INSTR(0x7c, op_movah)			/* MOV A,H */
{
	A = H;
	STATES(5);
}

INSTR(0x7d, op_moval)			/* MOV A,L */
{
	A = L;
	STATES(5);
}

INSTR(0x7e, op_movam)			/* MOV A,M */
{
	A = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x47, op_movba)			/* MOV B,A */
{
	B = A;
	STATES(5);
}

INSTR(0x40, op_movbb)			/* MOV B,B */
{
	STATES(5);
}

INSTR(0x41, op_movbc)			/* MOV B,C */
{
	B = C;
	STATES(5);
}

INSTR(0x42, op_movbd)			/* MOV B,D */
{
	B = D;
	STATES(5);
}

INSTR(0x43, op_movbe)			/* MOV B,E */
{
	B = E;
	STATES(5);
}

INSTR(0x44, op_movbh)			/* MOV B,H */
{
	B = H;
	STATES(5);
}

INSTR(0x45, op_movbl)			/* MOV B,L */
{
	B = L;
	STATES(5);
}

INSTR(0x46, op_movbm)			/* MOV B,M */
{
	B = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x4f, op_movca)			/* MOV C,A */
{
	C = A;
	STATES(5);
}

INSTR(0x48, op_movcb)			/* MOV C,B */
{
	C = B;
	STATES(5);
}

INSTR(0x49, op_movcc)			/* MOV C,C */
{
	STATES(5);
}

INSTR(0x4a, op_movcd)			/* MOV C,D */
{
	C = D;
	STATES(5);
}

INSTR(0x4b, op_movce)			/* MOV C,E */
{
	C = E;
	STATES(5);
}

INSTR(0x4c, op_movch)			/* MOV C,H */
{
	C = H;
	STATES(5);
}

INSTR(0x4d, op_movcl)			/* MOV C,L */
{
	C = L;
	STATES(5);
}

INSTR(0x4e, op_movcm)			/* MOV C,M */
{
	C = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x57, op_movda)			/* MOV D,A */
{
	D = A;
	STATES(5);
}

INSTR(0x50, op_movdb)			/* MOV D,B */
{
	D = B;
	STATES(5);
}

INSTR(0x51, op_movdc)			/* MOV D,C */
{
	D = C;
	STATES(5);
}

INSTR(0x52, op_movdd)			/* MOV D,D */
{
	STATES(5);
}

INSTR(0x53, op_movde)			/* MOV D,E */
{
	D = E;
	STATES(5);
}

INSTR(0x54, op_movdh)			/* MOV D,H */
{
	D = H;
	STATES(5);
}

INSTR(0x55, op_movdl)			/* MOV D,L */
{
	D = L;
	STATES(5);
}

INSTR(0x56, op_movdm)			/* MOV D,M */
{
	D = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x5f, op_movea)			/* MOV E,A */
{
	E = A;
	STATES(5);
}

INSTR(0x58, op_moveb)			/* MOV E,B */
{
	E = B;
	STATES(5);
}

INSTR(0x59, op_movec)			/* MOV E,C */
{
	E = C;
	STATES(5);
}

INSTR(0x5a, op_moved)			/* MOV E,D */
{
	E = D;
	STATES(5);
}

INSTR(0x5b, op_movee)			/* MOV E,E */
{
	STATES(5);
}

INSTR(0x5c, op_moveh)			/* MOV E,H */
{
	E = H;
	STATES(5);
}

INSTR(0x5d, op_movel)			/* MOV E,L */
{
	E = L;
	STATES(5);
}

INSTR(0x5e, op_movem)			/* MOV E,M */
{
	E = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x67, op_movha)			/* MOV H,A */
{
	H = A;
	STATES(5);
}

INSTR(0x60, op_movhb)			/* MOV H,B */
{
	H = B;
	STATES(5);
}

INSTR(0x61, op_movhc)			/* MOV H,C */
{
	H = C;
	STATES(5);
}

INSTR(0x62, op_movhd)			/* MOV H,D */
{
	H = D;
	STATES(5);
}

INSTR(0x63, op_movhe)			/* MOV H,E */
{
	H = E;
	STATES(5);
}

INSTR(0x64, op_movhh)			/* MOV H,H */
{
	STATES(5);
}

INSTR(0x65, op_movhl)			/* MOV H,L */
{
	H = L;
	STATES(5);
}

INSTR(0x66, op_movhm)			/* MOV H,M */
{
	H = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x6f, op_movla)			/* MOV L,A */
{
	L = A;
	STATES(5);
}

INSTR(0x68, op_movlb)			/* MOV L,B */
{
	L = B;
	STATES(5);
}

INSTR(0x69, op_movlc)			/* MOV L,C */
{
	L = C;
	STATES(5);
}

INSTR(0x6a, op_movld)			/* MOV L,D */
{
	L = D;
	STATES(5);
}

INSTR(0x6b, op_movle)			/* MOV L,E */
{
	L = E;
	STATES(5);
}

INSTR(0x6c, op_movlh)			/* MOV L,H */
{
	L = H;
	STATES(5);
}

INSTR(0x6d, op_movll)			/* MOV L,L */
{
	STATES(5);
}

INSTR(0x6e, op_movlm)			/* MOV L,M */
{
	L = memrdr((H << 8) + L);
	STATES(7);
}

INSTR(0x01, op_lxibnn)			/* LXI B,nn */
{
	C = memrdr(PC++);
	B = memrdr(PC++);
	STATES(10);
}

INSTR(0x11, op_lxidnn)			/* LXI D,nn */
{
	E = memrdr(PC++);
	D = memrdr(PC++);
	STATES(10);
}

INSTR(0x21, op_lxihnn)			/* LXI H,nn */
{
	L = memrdr(PC++);
	H = memrdr(PC++);
	STATES(10);
}

INSTR(0x31, op_lxispnn)		/* LXI SP,nn */
{
	SP = memrdr(PC++);
	SP += memrdr(PC++) << 8;
	STATES(10);
}

INSTR(0xf9, op_sphl)			/* SPHL */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(H << 8 | L);
#endif
	SP = (H << 8) + L;
	STATES(5);
}

INSTR(0x2a, op_lhldnn)			/* LHLD nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	L = memrdr(i);
	H = memrdr(i + 1);
	STATES(16);
}

INSTR(0x22, op_shldnn)			/* SHLD nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, L);
	memwrt(i + 1, H);
	STATES(16);
}

INSTR(0x03, op_inxb)			/* INX B */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(B << 8 | C);
#endif
	C++;
	if (!C)
		B++;
	STATES(5);
}

INSTR(0x13, op_inxd)			/* INX D */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(D << 8 | E);
#endif
	E++;
	if (!E)
		D++;
	STATES(5);
}

INSTR(0x23, op_inxh)			/* INX H */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(H << 8 | L);
#endif
	L++;
	if (!L)
		H++;
	STATES(5);
}

INSTR(0x33, op_inxsp)			/* INX SP */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(SP);
#endif
	SP++;
	STATES(5);
}

INSTR(0x0b, op_dcxb)			/* DCX B */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(B << 8 | C);
#endif
	C--;
	if (C == 0xff)
		B--;
	STATES(5);
}

INSTR(0x1b, op_dcxd)			/* DCX D */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(D << 8 | E);
#endif
	E--;
	if (E == 0xff)
		D--;
	STATES(5);
}

INSTR(0x2b, op_dcxh)			/* DCX H */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(H << 8 | L);
#endif
	L--;
	if (L == 0xff)
		H--;
	STATES(5);
}

INSTR(0x3b, op_dcxsp)			/* DCX SP */
{
#ifdef FRONTPANEL
	if (F_flag)
		addr_leds(SP);
#endif
	SP--;
	STATES(5);
}

INSTR(0x09, op_dadb)			/* DAD B */
{
	register int carry;

	carry = (L + C > 255) ? 1 : 0;
	L += C;
	(H + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += B + carry;
	STATES(10);
}

INSTR(0x19, op_dadd)			/* DAD D */
{
	register int carry;

	carry = (L + E > 255) ? 1 : 0;
	L += E;
	(H + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += D + carry;
	STATES(10);
}

INSTR(0x29, op_dadh)			/* DAD H */
{
	register int carry;

	carry = (L << 1 > 255) ? 1 : 0;
	L <<= 1;
	(H + H + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += H + carry;
	STATES(10);
}

INSTR(0x39, op_dadsp)			/* DAD SP */
{
	register int carry;

	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;

	carry = (L + spl > 255) ? 1 : 0;
	L += spl;
	(H + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += sph + carry;
	STATES(10);
}

INSTR(0xa7, op_anaa)			/* ANA A */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	(A & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa0, op_anab)			/* ANA B */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | B) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= B;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa1, op_anac)			/* ANA C */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | C) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= C;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa2, op_anad)			/* ANA D */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | D) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= D;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa3, op_anae)			/* ANA E */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | E) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= E;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa4, op_anah)			/* ANA H */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | H) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= H;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa5, op_anal)			/* ANA L */
{
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | L) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= L;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(4);
}

INSTR(0xa6, op_anam)			/* ANA M */
{
	register BYTE P;

	P = memrdr((H << 8) + L);
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | P) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= P;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(7);
}

INSTR(0xe6, op_anin)			/* ANI n */
{
	register BYTE P;

	P = memrdr(PC++);
#ifdef AMD8080
	F &= ~H_FLAG;
#else
	((A | P) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#endif
	A &= P;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~C_FLAG;
	STATES(7);
}

INSTR(0xb7, op_oraa)			/* ORA A */
{
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb0, op_orab)			/* ORA B */
{
	A |= B;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb1, op_orac)			/* ORA C */
{
	A |= C;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb2, op_orad)			/* ORA D */
{
	A |= D;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb3, op_orae)			/* ORA E */
{
	A |= E;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb4, op_orah)			/* ORA H */
{
	A |= H;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb5, op_oral)			/* ORA L */
{
	A |= L;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(4);
}

INSTR(0xb6, op_oram)			/* ORA M */
{
	A |= memrdr((H << 8) + L);
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(7);
}

INSTR(0xf6, op_orin)			/* ORI n */
{
	A |= memrdr(PC++);
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(C_FLAG | H_FLAG);
	STATES(7);
}

INSTR(0xaf, op_xraa)			/* XRA A */
{
	A = 0;
	F &= ~(S_FLAG | H_FLAG | C_FLAG);
	F |= Z_FLAG | P_FLAG;
	STATES(4);
}

INSTR(0xa8, op_xrab)			/* XRA B */
{
	A ^= B;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(4);
}

INSTR(0xa9, op_xrac)			/* XRA C */
{
	A ^= C;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(4);
}

INSTR(0xaa, op_xrad)			/* XRA D */
{
	A ^= D;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(4);
}

INSTR(0xab, op_xrae)			/* XRA E */
{
	A ^= E;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(4);
}

INSTR(0xac, op_xrah)			/* XRA H */
{
	A ^= H;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(4);
}

INSTR(0xad, op_xral)			/* XRA L */
{
	A ^= L;
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(4);
}

INSTR(0xae, op_xram)			/* XRA M */
{
	A ^= memrdr((H << 8) + L);
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(7);
}

INSTR(0xee, op_xrin)			/* XRI n */
{
	A ^= memrdr(PC++);
#ifndef FAST_INSTR
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	F &= ~(H_FLAG | C_FLAG);
	STATES(7);
}

INSTR(0x87, op_adda)			/* ADD A */
{
	((A & 0xf) + (A & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	((A << 1) > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A << 1;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x80, op_addb)			/* ADD B */
{
	((A & 0xf) + (B & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + B > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + B;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x81, op_addc)			/* ADD C */
{
	((A & 0xf) + (C & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + C > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + C;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x82, op_addd)			/* ADD D */
{
	((A & 0xf) + (D & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + D > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + D;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x83, op_adde)			/* ADD E */
{
	((A & 0xf) + (E & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + E > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + E;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x84, op_addh)			/* ADD H */
{
	((A & 0xf) + (H & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + H > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + H;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x85, op_addl)			/* ADD L */
{
	((A & 0xf) + (L & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + L > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + L;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x86, op_addm)			/* ADD M */
{
	register BYTE P;

	P = memrdr((H << 8) + L);
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0xc6, op_adin)			/* ADI n */
{
	register BYTE P;

	P = memrdr(PC++);
	((A & 0xf) + (P & 0xf) > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0x8f, op_adca)			/* ADC A */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (A & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	((A << 1) + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (A << 1) + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x88, op_adcb)			/* ADC B */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + B + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x89, op_adcc)			/* ADC C */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (C & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + C + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + C + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x8a, op_adcd)			/* ADC D */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + D + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x8b, op_adce)			/* ADC E */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (E & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + E + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + E + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x8c, op_adch)			/* ADC H */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (H & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + H + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + H + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x8d, op_adcl)			/* ADC L */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (L & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + L + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + L + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x8e, op_adcm)			/* ADC M */
{
	register int carry;
	register BYTE P;

	P = memrdr((H << 8) + L);
	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0xce, op_acin)			/* ACI n */
{
	register int carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(PC++);
	((A & 0xf) + (P & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P + carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0x97, op_suba)			/* SUB A */
{
	A = 0;
	F &= ~(S_FLAG | C_FLAG);
	F |= Z_FLAG | H_FLAG | P_FLAG;
	STATES(4);
}

INSTR(0x90, op_subb)			/* SUB B */
{
	((B & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(B > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - B;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x91, op_subc)			/* SUB C */
{
	((C & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(C > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - C;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x92, op_subd)			/* SUB D */
{
	((D & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(D > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - D;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x93, op_sube)			/* SUB E */
{
	((E & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(E > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - E;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x94, op_subh)			/* SUB H */
{
	((H & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(H > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - H;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x95, op_subl)			/* SUB L */
{
	((L & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(L > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - L;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x96, op_subm)			/* SUB M */
{
	register BYTE P;

	P = memrdr((H << 8) + L);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0xd6, op_suin)			/* SUI n */
{
	register BYTE P;

	P = memrdr(PC++);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0x9f, op_sbba)			/* SBB A */
{
	if (F & C_FLAG) {
		F |= S_FLAG | C_FLAG | P_FLAG;
		F &= ~(Z_FLAG | H_FLAG);
		A = 255;
	} else {
		F |= Z_FLAG | H_FLAG | P_FLAG;
		F &= ~(S_FLAG | C_FLAG);
		A = 0;
	}
	STATES(4);
}

INSTR(0x98, op_sbbb)			/* SBB B */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((B & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(B + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - B - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x99, op_sbbc)			/* SBB C */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((C & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(C + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - C - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x9a, op_sbbd)			/* SBB D */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((D & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(D + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - D - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x9b, op_sbbe)			/* SBB E */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((E & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(E + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - E - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x9c, op_sbbh)			/* SBB H */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((H & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(H + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - H - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x9d, op_sbbl)			/* SBB L */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((L & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(L + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - L - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(4);
}

INSTR(0x9e, op_sbbm)			/* SBB M */
{
	register int carry;
	register BYTE P;

	P = memrdr((H << 8) + L);
	carry = (F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0xde, op_sbin)			/* SBI n */
{
	register int carry;
	register BYTE P;

	P = memrdr(PC++);
	carry = (F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P - carry;
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(7);
}

INSTR(0xbf, op_cmpa)			/* CMP A */
{
	F &= ~(S_FLAG | C_FLAG);
	F |= Z_FLAG | H_FLAG | P_FLAG;
	STATES(4);
}

INSTR(0xb8, op_cmpb)			/* CMP B */
{
	register BYTE i;

	((B & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(B > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - B;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(4);
}

INSTR(0xb9, op_cmpc)			/* CMP C */
{
	register BYTE i;

	((C & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(C > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - C;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(4);
}

INSTR(0xba, op_cmpd)			/* CMP D */
{
	register BYTE i;

	((D & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(D > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - D;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(4);
}

INSTR(0xbb, op_cmpe)			/* CMP E */
{
	register BYTE i;

	((E & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(E > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - E;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(4);
}

INSTR(0xbc, op_cmph)			/* CMP H */
{
	register BYTE i;

	((H & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(H > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - H;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(4);
}

INSTR(0xbd, op_cmpl)			/* CMP L */
{
	register BYTE i;

	((L & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(L > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - L;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(4);
}

INSTR(0xbe, op_cmpm)			/* CMP M */
{
	register BYTE i;
	register BYTE P;

	P = memrdr((H << 8) + L);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - P;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(7);
}

INSTR(0xfe, op_cpin)			/* CPI n */
{
	register BYTE i;
	register BYTE P;

	P = memrdr(PC++);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - P;
#ifndef FAST_INSTR
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[i];
#endif
	STATES(7);
}

INSTR(0x3c, op_inra)			/* INR A */
{
	A++;
	((A & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(5);
}

INSTR(0x04, op_inrb)			/* INR B */
{
	B++;
	((B & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#endif
	STATES(5);
}

INSTR(0x0c, op_inrc)			/* INR C */
{
	C++;
	((C & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#endif
	STATES(5);
}

INSTR(0x14, op_inrd)			/* INR D */
{
	D++;
	((D & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#endif
	STATES(5);
}

INSTR(0x1c, op_inre)			/* INR E */
{
	E++;
	((E & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#endif
	STATES(5);
}

INSTR(0x24, op_inrh)			/* INR H */
{
	H++;
	((H & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#endif
	STATES(5);
}

INSTR(0x2c, op_inrl)			/* INR L */
{
	L++;
	((L & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#endif
	STATES(5);
}

INSTR(0x34, op_inrm)			/* INR M */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	P++;
	memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
#ifndef FAST_INSTR
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#endif
	STATES(10);
}

INSTR(0x3d, op_dcra)			/* DCR A */
{
	A--;
	((A & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[A];
#endif
	STATES(5);
}

INSTR(0x05, op_dcrb)			/* DCR B */
{
	B--;
	((B & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[B];
#endif
	STATES(5);
}

INSTR(0x0d, op_dcrc)			/* DCR C */
{
	C--;
	((C & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[C];
#endif
	STATES(5);
}

INSTR(0x15, op_dcrd)			/* DCR D */
{
	D--;
	((D & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[D];
#endif
	STATES(5);
}

INSTR(0x1d, op_dcre)			/* DCR E */
{
	E--;
	((E & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[E];
#endif
	STATES(5);
}

INSTR(0x25, op_dcrh)			/* DCR H */
{
	H--;
	((H & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[H];
#endif
	STATES(5);
}

INSTR(0x2d, op_dcrl)			/* DCR L */
{
	L--;
	((L & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[L];
#endif
	STATES(5);
}

INSTR(0x35, op_dcrm)			/* DCR M */
{
	register BYTE P;
	WORD addr;

	addr = (H << 8) + L;
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
#ifndef FAST_INSTR
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
#else
	F = (F & ~SZP_FLAGS) | szp_flags[P];
#endif
	STATES(10);
}

INSTR(0x07, op_rlc)			/* RLC */
{
	register int i;

	i = (A & 128) ? 1 : 0;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	A |= i;
	STATES(4);
}

INSTR(0x0f, op_rrc)			/* RRC */
{
	register int i;

	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (i) A |= 128;
	STATES(4);
}

INSTR(0x17, op_ral)			/* RAL */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
	STATES(4);
}

INSTR(0x1f, op_rar)			/* RAR */
{
	register int i, old_c_flag;

	old_c_flag = F & C_FLAG;
	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
	STATES(4);
}

INSTR(0xeb, op_xchg)			/* XCHG */
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

INSTR(0xe3, op_xthl)			/* XTHL */
{
	register BYTE i;

#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	i = memrdr(SP);
	memwrt(SP, L);
	L = i;
	i = memrdr(SP + 1);
	memwrt(SP + 1, H);
	H = i;
	STATES(18);
}

INSTR(0xf5, op_pushpsw)		/* PUSH PSW */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, A);
	memwrt(--SP, F);
	STATES(11);
}

INSTR(0xc5, op_pushb)			/* PUSH B */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, B);
	memwrt(--SP, C);
	STATES(11);
}

INSTR(0xd5, op_pushd)			/* PUSH D */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, D);
	memwrt(--SP, E);
	STATES(11);
}

INSTR(0xe5, op_pushh)			/* PUSH H */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, H);
	memwrt(--SP, L);
	STATES(11);
}

INSTR(0xf1, op_poppsw)			/* POP PSW */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	F = memrdr(SP++);
	F &= ~(Y_FLAG | X_FLAG);
	F |= N_FLAG;
	A = memrdr(SP++);
	STATES(10);
}

INSTR(0xc1, op_popb)			/* POP B */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	C = memrdr(SP++);
	B = memrdr(SP++);
	STATES(10);
}

INSTR(0xd1, op_popd)			/* POP D */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	E = memrdr(SP++);
	D = memrdr(SP++);
	STATES(10);
}

INSTR(0xe1, op_poph)			/* POP H */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	L = memrdr(SP++);
	H = memrdr(SP++);
	STATES(10);
}

INSTR(0xc3, op_jmp)			/* JMP nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC) << 8;
	PC = i;
	STATES(10);
}

INSTR(0xe9, op_pchl)			/* PCHL */
{
	PC = (H << 8) + L;
	STATES(5);
}

INSTR(0xcd, op_call)			/* CALL nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = i;
	STATES(17);
}

INSTR(0xc9, op_ret)			/* RET */
{
	register WORD i;

#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	STATES(10);
}

INSTR(0xca, op_jz)			/* JZ nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & Z_FLAG)
		PC = i;
	STATES(10);
}

INSTR(0xc2, op_jnz)			/* JNZ nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & Z_FLAG))
		PC = i;
	STATES(10);
}

INSTR(0xda, op_jc)			/* JC nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & C_FLAG)
		PC = i;
	STATES(10);
}

INSTR(0xd2, op_jnc)			/* JNC nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & C_FLAG))
		PC = i;
	STATES(10);
}

INSTR(0xea, op_jpe)			/* JPE nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & P_FLAG)
		PC = i;
	STATES(10);
}

INSTR(0xe2, op_jpo)			/* JPO nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & P_FLAG))
		PC = i;
	STATES(10);
}

INSTR(0xfa, op_jm)			/* JM nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & S_FLAG)
		PC = i;
	STATES(10);
}

INSTR(0xf2, op_jp)			/* JP nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & S_FLAG))
		PC = i;
	STATES(10);
}

INSTR(0xcc, op_cz)			/* CZ nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & Z_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	} else {
		STATES(11);
	}
}

INSTR(0xc4, op_cnz)			/* CNZ nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xdc, op_cc)			/* CC nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & C_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xd4, op_cnc)			/* CNC nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xec, op_cpe)			/* CPE nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & P_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xe4, op_cpo)			/* CPO nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & P_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xfc, op_cm)			/* CM nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (F & S_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xf4, op_cp)			/* CP nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	if (!(F & S_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC);
		PC = i;
		STATES(17);
	}
	STATES(11);
}

INSTR(0xc8, op_rz)			/* RZ */
{
	register WORD i;

	if (F & Z_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xc0, op_rnz)			/* RNZ */
{
	register WORD i;

	if (!(F & Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xd8, op_rc)			/* RC */
{
	register WORD i;

	if (F & C_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xd0, op_rnc)			/* RNC */
{
	register WORD i;

	if (!(F & C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xe8, op_rpe)			/* RPE */
{
	register WORD i;

	if (F & P_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xe0, op_rpo)			/* RPO */
{
	register WORD i;

	if (!(F & P_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xf8, op_rm)			/* RM */
{
	register WORD i;

	if (F & S_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xf0, op_rp)			/* RP */
{
	register WORD i;

	if (!(F & S_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		STATES(11);
	}
	STATES(5);
}

INSTR(0xc7, op_rst0)			/* RST 0 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0;
	STATES(11);
}

INSTR(0xcf, op_rst1)			/* RST 1 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x08;
	STATES(11);
}

INSTR(0xd7, op_rst2)			/* RST 2 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x10;
	STATES(11);
}

INSTR(0xdf, op_rst3)			/* RST 3 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x18;
	STATES(11);
}

INSTR(0xe7, op_rst4)			/* RST 4 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x20;
	STATES(11);
}

INSTR(0xef, op_rst5)			/* RST 5 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x28;
	STATES(11);
}

INSTR(0xf7, op_rst6)			/* RST 6 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x30;
	STATES(11);
}

INSTR(0xff, op_rst7)			/* RST 7 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = 0x38;
	STATES(11);
}

/**********************************************************************/
/**********************************************************************/
/*********       UNDOCUMENTED 8080 INSTRUCTIONS, BEWARE!      *********/
/**********************************************************************/
/**********************************************************************/

#ifdef UNDOC_INST

INSTR(0x08, op_undoc_nop1)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0x10, op_undoc_nop2)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0x18, op_undoc_nop3)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0x20, op_undoc_nop4)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0x28, op_undoc_nop5)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0x30, op_undoc_nop6)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0x38, op_undoc_nop7)		/* NOP* */
{
	if (u_flag) {
		STATES(trap_undoc());
	}

	STATES(4);
}

INSTR(0xcb, op_undoc_jmp)		/* JMP* nn */
{
	register WORD i;

	if (u_flag) {
		STATES(trap_undoc());
	}

	i = memrdr(PC++);
	i += memrdr(PC) << 8;
	PC = i;
	STATES(10);
}

INSTR(0xdd, op_undoc_call1)		/* CALL* nn */
{
	register WORD i;

	if (u_flag) {
		STATES(trap_undoc());
	}

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = i;
	STATES(17);
}

INSTR(0xed, op_undoc_call2)		/* CALL* nn */
{
	register WORD i;

	if (u_flag) {
		STATES(trap_undoc());
	}

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = i;
	STATES(17);
}

INSTR(0xfd, op_undoc_call3)		/* CALL* nn */
{
	register WORD i;

	if (u_flag) {
		STATES(trap_undoc());
	}

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC >> 8);
	memwrt(--SP, PC);
	PC = i;
	STATES(17);
}

INSTR(0xd9, op_undoc_ret)		/* RET */
{
	register WORD i;

	if (u_flag) {
		STATES(trap_undoc());
	}

#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	STATES(10);
}

#endif
