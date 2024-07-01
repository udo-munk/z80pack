/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef ALT8080_INC
#define ALT8080_INC

/*
 *	This module builds the 8080 central processing unit.
 *	The opcode where PC points to is fetched from the memory
 *	and PC incremented by one. The opcode is then dispatched
 *	to execute code, which emulates this 8080 opcode.
 *
 *	This implementation is primarily optimized for code size
 *	and written as a single block of code.
 *
 *	For a description of how the arithmetic flags calculation works see:
 *	http://emulators.com/docs/lazyoverflowdetect_final.pdf
 *
 *	The formula for subtraction carry outs was derived by staring
 *	intently at some truth tables.
 *
 *	cout contains the carry outs for every bit
 */

{

#define H_SHIFT		4	/* H_FLAG shift */
#define C_SHIFT		0	/* C_FLAG shift */

	/* Precomputed table for fast sign, zero and parity flag calculation */
#define _ 0
#define S S_FLAG
#define Z Z_FLAG
#define P P_FLAG
	static const BYTE szp_flags[256] = {
		/*00*/ Z|P,   _,   _,   P,   _,   P,   P,   _,
		/*08*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*10*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*18*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*20*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*28*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*30*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*38*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*40*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*48*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*50*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*58*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*60*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*68*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*70*/   _,   P,   P,   _,   P,   _,   _,   P,
		/*78*/   P,   _,   _,   P,   _,   P,   P,   _,
		/*80*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*88*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*90*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*98*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*a0*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*a8*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*b0*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*b8*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*c0*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*c8*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*d0*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*d8*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*e0*/   S, S|P, S|P,   S, S|P,   S,   S, S|P,
		/*e8*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*f0*/ S|P,   S,   S, S|P,   S, S|P, S|P,   S,
		/*f8*/   S, S|P, S|P,   S, S|P,   S,   S, S|P
	};
#undef _
#undef S
#undef Z
#undef P

	BYTE t, res, cout, P;
	struct cpu_reg w;	/* working register */
	uint64_t clk;

#define W	w.w
#define WH	w.h
#define WL	w.l

	t = 4;				/* minimum clock cycles for M1 */

	switch (memrdr(PC++)) {		/* execute next opcode */

	case 0x00:			/* NOP */
	case 0x08:			/* NOP* */
	case 0x10:			/* NOP* */
	case 0x18:			/* NOP* */
	case 0x20:			/* NOP* */
	case 0x28:			/* NOP* */
	case 0x30:			/* NOP* */
	case 0x38:			/* NOP* */
		break;

	case 0x40:			/* MOV B,B */
	case 0x49:			/* MOV C,C */
	case 0x52:			/* MOV D,D */
	case 0x5b:			/* MOV E,E */
	case 0x64:			/* MOV H,H */
	case 0x6d:			/* MOV L,L */
	case 0x7f:			/* MOV A,A */
		t++;
		break;

	case 0x01:			/* LXI B,nn */
		C = memrdr(PC++);
		B = memrdr(PC++);
		t += 6;
		break;

	case 0x02:			/* STAX B */
		memwrt(BC, A);
		t += 3;
		break;

	case 0x03:			/* INX B */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(BC);
#endif
		BC++;
		t++;
		break;

	case 0x04:			/* INR B */
		P = B;
		res = ++B;
	finish_inr:
		cout = (P & 1) | ((P | 1) & ~res);
		F = ((F & C_FLAG) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		/* C_FLAG unchanged */
		t++;
		break;

	case 0x05:			/* DCR B */
		P = B;
		res = --B;
	finish_dcr:
		cout = (~P & 1) | ((~P | 1) & res);
		F = ((F & C_FLAG) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		F ^= H_FLAG;
		/* C_FLAG unchanged */
		t++;
		break;

	case 0x06:			/* MVI B,n */
		B = memrdr(PC++);
		t += 3;
		break;

	case 0x07:			/* RLC */
		res = ((A & 0x80) >> 7) & 1;
		F = (F & ~C_FLAG) | (res << C_SHIFT);
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		A = (A << 1) | res;
		break;

	case 0x09:			/* DAD B */
		W = HL + BC;
		cout = (H & B) | ((H | B) & ~WH);
	finish_dad:
		F = (F & ~C_FLAG) | (((cout >> 7) & 1) << C_SHIFT);
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		H = WH;
		L = WL;
		t += 6;
		break;

	case 0x0a:			/* LDAX B */
		A = memrdr(BC);
		t += 3;
		break;

	case 0x0b:			/* DCX B */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(BC);
#endif
		BC--;
		t++;
		break;

	case 0x0c:			/* INR C */
		P = C;
		res = ++C;
		goto finish_inr;

	case 0x0d:			/* DCR C */
		P = C;
		res = --C;
		goto finish_dcr;

	case 0x0e:			/* MVI C,n */
		C = memrdr(PC++);
		t += 3;
		break;

	case 0x0f:			/* RRC */
		res = A & 1;
		F = (F & ~C_FLAG) | (res << C_SHIFT);
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		A = (A >> 1) | (res << 7);
		break;

	case 0x11:			/* LXI D,nn */
		E = memrdr(PC++);
		D = memrdr(PC++);
		t += 6;
		break;

	case 0x12:			/* STAX D */
		memwrt(DE, A);
		t += 3;
		break;

	case 0x13:			/* INX D */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(DE);
#endif
		DE++;
		t++;
		break;

	case 0x14:			/* INR D */
		P = D;
		res = ++D;
		goto finish_inr;

	case 0x15:			/* DCR D */
		P = D;
		res = --D;
		goto finish_dcr;

	case 0x16:			/* MVI D,n */
		D = memrdr(PC++);
		t += 3;
		break;

	case 0x17:			/* RAL */
		res = (F >> C_SHIFT) & 1;
		F = (F & ~C_FLAG) | ((((A & 0x80) >> 7) & 1) << C_SHIFT);
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		A = (A << 1) | res;
		break;

	case 0x19:			/* DAD D */
		W = HL + DE;
		cout = (H & D) | ((H | D) & ~WH);
		goto finish_dad;

	case 0x1a:			/* LDAX D */
		A = memrdr(DE);
		t += 3;
		break;

	case 0x1b:			/* DCX D */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(DE);
#endif
		DE--;
		t++;
		break;

	case 0x1c:			/* INR E */
		P = E;
		res = ++E;
		goto finish_inr;

	case 0x1d:			/* DCR E */
		P = E;
		res = --E;
		goto finish_dcr;

	case 0x1e:			/* MVI E,n */
		E = memrdr(PC++);
		t += 3;
		break;

	case 0x1f:			/* RAR */
		res = (F >> C_SHIFT) & 1;
		F = (F & ~C_FLAG) | ((A & 1) << C_SHIFT);
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		A = (A >> 1) | (res << 7);
		break;

	case 0x21:			/* LXI H,nn */
		L = memrdr(PC++);
		H = memrdr(PC++);
		t += 6;
		break;

	case 0x22:			/* SHLD nn */
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		memwrt(W, L);
		memwrt(W + 1, H);
		t += 12;
		break;

	case 0x23:			/* INX H */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(HL);
#endif
		HL++;
		t++;
		break;

	case 0x24:			/* INR H */
		P = H;
		res = ++H;
		goto finish_inr;

	case 0x25:			/* DCR H */
		P = H;
		res = --H;
		goto finish_dcr;

	case 0x26:			/* MVI H,n */
		H = memrdr(PC++);
		t += 3;
		break;

	case 0x27:			/* DAA */
		P = 0;
		if (((A & 0xf) > 9) || (F & H_FLAG))
			P |= 0x06;
		if ((A > 0x99) || (F & C_FLAG)) {
			F |= C_FLAG;
			P |= 0x60;
		}
		res = A + P;
		cout = (A & P) | ((A | P) & ~res);
		F = ((F & C_FLAG) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		A = res;
		break;

	case 0x29:			/* DAD H */
		W = HL << 1;
		cout = H | (H & ~WH);
		goto finish_dad;

	case 0x2a:			/* LHLD nn */
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		L = memrdr(W);
		H = memrdr(W + 1);
		t += 12;
		break;

	case 0x2b:			/* DCX H */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(HL);
#endif
		HL--;
		t++;
		break;

	case 0x2c:			/* INR L */
		P = L;
		res = ++L;
		goto finish_inr;

	case 0x2d:			/* DCR L */
		P = L;
		res = --L;
		goto finish_dcr;

	case 0x2e:			/* MVI L,n */
		L = memrdr(PC++);
		t += 3;
		break;

	case 0x2f:			/* CMA */
		A = ~A;
		break;

	case 0x31:			/* LXI SP,nn */
		SPL = memrdr(PC++);
		SPH = memrdr(PC++);
		t += 6;
		break;

	case 0x32:			/* STA nn */
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		memwrt(W, A);
		t += 9;
		break;

	case 0x33:			/* INX SP */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(SP);
#endif
		SP++;
		t++;
		break;

	case 0x34:			/* INR M */
		P = memrdr(HL);
		res = P + 1;
		memwrt(HL, res);
		t += 5;
		goto finish_inr;

	case 0x35:			/* DCR M */
		P = memrdr(HL);
		res = P - 1;
		memwrt(HL, res);
		t += 5;
		goto finish_dcr;

	case 0x36:			/* MVI M,n */
		memwrt(HL, memrdr(PC++));
		t += 6;
		break;

	case 0x37:			/* STC */
		F |= C_FLAG;
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		break;

	case 0x39:			/* DAD SP */
		W = HL + SP;
		cout = (H & SPH) | ((H | SPH) & ~WH);
		goto finish_dad;

	case 0x3a:			/* LDA nn */
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		A = memrdr(W);
		t += 9;
		break;

	case 0x3b:			/* DCX SP */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(SP);
#endif
		SP--;
		t++;
		break;

	case 0x3c:			/* INR A */
		P = A;
		res = ++A;
		goto finish_inr;

	case 0x3d:			/* DCR A */
		P = A;
		res = --A;
		goto finish_dcr;

	case 0x3e:			/* MVI A,n */
		A = memrdr(PC++);
		t += 3;
		break;

	case 0x3f:			/* CMC */
		F ^= C_FLAG;
		/* S_FLAG, Z_FLAG, H_FLAG, and P_FLAG unchanged */
		break;

	case 0x41:			/* MOV B,C */
		B = C;
		t++;
		break;

	case 0x42:			/* MOV B,D */
		B = D;
		t++;
		break;

	case 0x43:			/* MOV B,E */
		B = E;
		t++;
		break;

	case 0x44:			/* MOV B,H */
		B = H;
		t++;
		break;

	case 0x45:			/* MOV B,L */
		B = L;
		t++;
		break;

	case 0x46:			/* MOV B,M */
		B = memrdr(HL);
		t += 3;
		break;

	case 0x47:			/* MOV B,A */
		B = A;
		t++;
		break;

	case 0x48:			/* MOV C,B */
		C = B;
		t++;
		break;

	case 0x4a:			/* MOV C,D */
		C = D;
		t++;
		break;

	case 0x4b:			/* MOV C,E */
		C = E;
		t++;
		break;

	case 0x4c:			/* MOV C,H */
		C = H;
		t++;
		break;

	case 0x4d:			/* MOV C,L */
		C = L;
		t++;
		break;

	case 0x4e:			/* MOV C,M */
		C = memrdr(HL);
		t += 3;
		break;

	case 0x4f:			/* MOV C,A */
		C = A;
		t++;
		break;

	case 0x50:			/* MOV D,B */
		D = B;
		t++;
		break;

	case 0x51:			/* MOV D,C */
		D = C;
		t++;
		break;

	case 0x53:			/* MOV D,E */
		D = E;
		t++;
		break;

	case 0x54:			/* MOV D,H */
		D = H;
		t++;
		break;

	case 0x55:			/* MOV D,L */
		D = L;
		t++;
		break;

	case 0x56:			/* MOV D,M */
		D = memrdr(HL);
		t += 3;
		break;

	case 0x57:			/* MOV D,A */
		D = A;
		t++;
		break;

	case 0x58:			/* MOV E,B */
		E = B;
		t++;
		break;

	case 0x59:			/* MOV E,C */
		E = C;
		t++;
		break;

	case 0x5a:			/* MOV E,D */
		E = D;
		t++;
		break;

	case 0x5c:			/* MOV E,H */
		E = H;
		t++;
		break;

	case 0x5d:			/* MOV E,L */
		E = L;
		t++;
		break;

	case 0x5e:			/* MOV E,M */
		E = memrdr(HL);
		t += 3;
		break;

	case 0x5f:			/* MOV E,A */
		E = A;
		t++;
		break;

	case 0x60:			/* MOV H,B */
		H = B;
		t++;
		break;

	case 0x61:			/* MOV H,C */
		H = C;
		t++;
		break;

	case 0x62:			/* MOV H,D */
		H = D;
		t++;
		break;

	case 0x63:			/* MOV H,E */
		H = E;
		t++;
		break;

	case 0x65:			/* MOV H,L */
		H = L;
		t++;
		break;

	case 0x66:			/* MOV H,M */
		H = memrdr(HL);
		t += 3;
		break;

	case 0x67:			/* MOV H,A */
		H = A;
		t++;
		break;

	case 0x68:			/* MOV L,B */
		L = B;
		t++;
		break;

	case 0x69:			/* MOV L,C */
		L = C;
		t++;
		break;

	case 0x6a:			/* MOV L,D */
		L = D;
		t++;
		break;

	case 0x6b:			/* MOV L,E */
		L = E;
		t++;
		break;

	case 0x6c:			/* MOV L,H */
		L = H;
		t++;
		break;

	case 0x6e:			/* MOV L,M */
		L = memrdr(HL);
		t += 3;
		break;

	case 0x6f:			/* MOV L,A */
		L = A;
		t++;
		break;

	case 0x70:			/* MOV M,B */
		memwrt(HL, B);
		t += 3;
		break;

	case 0x71:			/* MOV M,C */
		memwrt(HL, C);
		t += 3;
		break;

	case 0x72:			/* MOV M,D */
		memwrt(HL, D);
		t += 3;
		break;

	case 0x73:			/* MOV M,E */
		memwrt(HL, E);
		t += 3;
		break;

	case 0x74:			/* MOV M,H */
		memwrt(HL, H);
		t += 3;
		break;

	case 0x75:			/* MOV M,L */
		memwrt(HL, L);
		t += 3;
		break;

	case 0x76:			/* HLT */
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_HLTA | CPU_MEMR;
#endif

		clk = get_clock_us();
#ifdef FRONTPANEL
		if (!F_flag) {
#endif
			if (IFF == 0) {
				/* without a frontpanel DI + HALT
				   stops the machine */
				cpu_error = OPHALT;
				cpu_state = STOPPED;
			} else {
				/* else wait for INT or user interrupt */
				while ((int_int == 0) &&
				       (cpu_state == CONTIN_RUN)) {
					SLEEP_MS(1);
				}
			}
#ifdef BUS_8080
			if (int_int)
				cpu_bus = CPU_INTA | CPU_WO |
					  CPU_HLTA | CPU_M1;
#endif

			busy_loop_cnt = 0;

#ifdef FRONTPANEL
		} else {
			fp_led_address = 0xffff;
			fp_led_data = 0xff;

			if (IFF == 0) {
				/* INT disabled, wait for
				   frontpanel reset or user interrupt */
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
				while ((int_int == 0) &&
				       !(cpu_state & RESET)) {
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
		cpu_time -= get_clock_us() - clk;
		t += 3;
		break;

	case 0x77:			/* MOV M,A */
		memwrt(HL, A);
		t += 3;
		break;

	case 0x78:			/* MOV A,B */
		A = B;
		t++;
		break;

	case 0x79:			/* MOV A,C */
		A = C;
		t++;
		break;

	case 0x7a:			/* MOV A,D */
		A = D;
		t++;
		break;

	case 0x7b:			/* MOV A,E */
		A = E;
		t++;
		break;

	case 0x7c:			/* MOV A,H */
		A = H;
		t++;
		break;

	case 0x7d:			/* MOV A,L */
		A = L;
		t++;
		break;

	case 0x7e:			/* MOV A,M */
		A = memrdr(HL);
		t += 3;
		break;

	case 0x80:			/* ADD B */
		P = B;
		res = 0;
	finish_add:
		res = A + P + res;
		cout = (A & P) | ((A | P) & ~res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		A = res;
		break;

	case 0x81:			/* ADD C */
		P = C;
		res = 0;
		goto finish_add;

	case 0x82:			/* ADD D */
		P = D;
		res = 0;
		goto finish_add;

	case 0x83:			/* ADD E */
		P = E;
		res = 0;
		goto finish_add;

	case 0x84:			/* ADD H */
		P = H;
		res = 0;
		goto finish_add;

	case 0x85:			/* ADD L */
		P = L;
		res = 0;
		goto finish_add;

	case 0x86:			/* ADD M */
		P = memrdr(HL);
		res = 0;
		t += 3;
		goto finish_add;

	case 0x87:			/* ADD A */
		P = A;
		res = 0;
		goto finish_add;

	case 0x88:			/* ADC B */
		P = B;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x89:			/* ADC C */
		P = C;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x8a:			/* ADC D */
		P = D;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x8b:			/* ADC E */
		P = E;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x8c:			/* ADC H */
		P = H;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x8d:			/* ADC L */
		P = L;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x8e:			/* ADC M */
		P = memrdr(HL);
		res = (F >> C_SHIFT) & 1;
		t += 3;
		goto finish_add;

	case 0x8f:			/* ADC A */
		P = A;
		res = (F >> C_SHIFT) & 1;
		goto finish_add;

	case 0x90:			/* SUB B */
		P = B;
		res = 0;
	finish_sub:
		res = A - P - res;
		cout = (~A & P) | ((~A | P) & res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		F ^= H_FLAG;
		A = res;
		break;

	case 0x91:			/* SUB C */
		P = C;
		res = 0;
		goto finish_sub;

	case 0x92:			/* SUB D */
		P = D;
		res = 0;
		goto finish_sub;

	case 0x93:			/* SUB E */
		P = E;
		res = 0;
		goto finish_sub;

	case 0x94:			/* SUB H */
		P = H;
		res = 0;
		goto finish_sub;

	case 0x95:			/* SUB L */
		P = L;
		res = 0;
		goto finish_sub;

	case 0x96:			/* SUB M */
		P = memrdr(HL);
		res = 0;
		t += 3;
		goto finish_sub;

	case 0x97:			/* SUB A */
		F = Z_FLAG | H_FLAG | P_FLAG;
		/* S_FLAG and C_FLAG cleared */
		A = 0;
		break;

	case 0x98:			/* SBB B */
		P = B;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0x99:			/* SBB C */
		P = C;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0x9a:			/* SBB D */
		P = D;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0x9b:			/* SBB E */
		P = E;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0x9c:			/* SBB H */
		P = H;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0x9d:			/* SBB L */
		P = L;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0x9e:			/* SBB M */
		P = memrdr(HL);
		res = (F >> C_SHIFT) & 1;
		t += 3;
		goto finish_sub;

	case 0x9f:			/* SBB A */
		P = A;
		res = (F >> C_SHIFT) & 1;
		goto finish_sub;

	case 0xa0:			/* ANA B */
		P = B;
	finish_ana:
		res = A & P;
#ifdef AMD8080
		F = szp_flags[res];
		/* H_FLAG and C_FLAG cleared */
#else
		F = (((((A | P) >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		/* C_FLAG cleared */
#endif
		A = res;
		break;

	case 0xa1:			/* ANA C */
		P = C;
		goto finish_ana;

	case 0xa2:			/* ANA D */
		P = D;
		goto finish_ana;

	case 0xa3:			/* ANA E */
		P = E;
		goto finish_ana;

	case 0xa4:			/* ANA H */
		P = H;
		goto finish_ana;

	case 0xa5:			/* ANA L */
		P = L;
		goto finish_ana;

	case 0xa6:			/* ANA M */
		P = memrdr(HL);
		t += 3;
		goto finish_ana;

	case 0xa7:			/* ANA A */
		P = A;
		goto finish_ana;

	case 0xa8:			/* XRA B */
		P = B;
	finish_xra:
		res = A ^ P;
		F = szp_flags[res];
		/* H_FLAG and C_FLAG cleared */
		A = res;
		break;

	case 0xa9:			/* XRA C */
		P = C;
		goto finish_xra;

	case 0xaa:			/* XRA D */
		P = D;
		goto finish_xra;

	case 0xab:			/* XRA E */
		P = E;
		goto finish_xra;

	case 0xac:			/* XRA H */
		P = H;
		goto finish_xra;

	case 0xad:			/* XRA L */
		P = L;
		goto finish_xra;

	case 0xae:			/* XRA M */
		P = memrdr(HL);
		t += 3;
		goto finish_xra;

	case 0xaf:			/* XRA A */
		F = Z_FLAG | P_FLAG;
		/* S_FLAG, H_FLAG, and C_FLAG cleared */
		A = 0;
		break;

	case 0xb0:			/* ORA B */
		P = B;
	finish_ora:
		res = A | P;
		F = szp_flags[res];
		/* H_FLAG and C_FLAG cleared */
		A = res;
		break;

	case 0xb1:			/* ORA C */
		P = C;
		goto finish_ora;

	case 0xb2:			/* ORA D */
		P = D;
		goto finish_ora;

	case 0xb3:			/* ORA E */
		P = E;
		goto finish_ora;

	case 0xb4:			/* ORA H */
		P = H;
		goto finish_ora;

	case 0xb5:			/* ORA L */
		P = L;
		goto finish_ora;

	case 0xb6:			/* ORA M */
		P = memrdr(HL);
		t += 3;
		goto finish_ora;

	case 0xb7:			/* ORA A */
		F = szp_flags[A];
		/* H_FLAG and C_FLAG cleared */
		break;

	case 0xb8:			/* CMP B */
		P = B;
	finish_cmp:
		res = A - P;
		cout = (~A & P) | ((~A | P) & res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		F ^= H_FLAG;
		break;

	case 0xb9:			/* CMP C */
		P = C;
		goto finish_cmp;

	case 0xba:			/* CMP D */
		P = D;
		goto finish_cmp;

	case 0xbb:			/* CMP E */
		P = E;
		goto finish_cmp;

	case 0xbc:			/* CMP H */
		P = H;
		goto finish_cmp;

	case 0xbd:			/* CMP L */
		P = L;
		goto finish_cmp;

	case 0xbe:			/* CMP M */
		P = memrdr(HL);
		t += 3;
		goto finish_cmp;

	case 0xbf:			/* CMP A */
		F = Z_FLAG | H_FLAG | P_FLAG;
		/* S_FLAG and C_FLAG cleared */
		break;

	case 0xc0:			/* RNZ */
		res = !(F & Z_FLAG);
	finish_retc:
		t++;
		if (res)
			goto finish_ret;
		break;

	case 0xc1:			/* POP B */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		C = memrdr(SP++);
		B = memrdr(SP++);
		t += 6;
		break;

	case 0xc2:			/* JNZ nn */
		res = !(F & Z_FLAG);
	finish_jmpc:
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		t += 6;
		if (res)
			PC = W;
		break;

	case 0xc3:			/* JMP nn */
	case 0xcb:			/* JMP* nn */
		WL = memrdr(PC++);
		WH = memrdr(PC);
		t += 6;
		PC = W;
		break;

	case 0xc4:			/* CNZ nn */
		res = !(F & Z_FLAG);
	finish_callc:
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		t += 7;
		if (res)
			goto finish_call;
		break;

	case 0xc5:			/* PUSH B */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, B);
		memwrt(--SP, C);
		t += 7;
		break;

	case 0xc6:			/* ADI n */
		P = memrdr(PC++);
		res = 0;
		t += 3;
		goto finish_add;

	case 0xc7:			/* RST 0 */
		W = 0;
		t++;
		goto finish_call;

	case 0xc8:			/* RZ */
		res = F & Z_FLAG;
		goto finish_retc;

	case 0xc9:			/* RET */
	case 0xd9:			/* RET* */
	finish_ret:
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		WL = memrdr(SP++);
		WH = memrdr(SP++);
		t += 6;
		PC = W;
		break;

	case 0xca:			/* JZ nn */
		res = F & Z_FLAG;
		goto finish_jmpc;

	case 0xcc:			/* CZ nn */
		res = F & Z_FLAG;
		goto finish_callc;

	case 0xcd:			/* CALL nn */
	case 0xdd:			/* CALL* nn */
	case 0xed:			/* CALL* nn */
	case 0xfd:			/* CALL* nn */
		WL = memrdr(PC++);
		WH = memrdr(PC++);
		t += 7;
	finish_call:
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PCH);
		memwrt(--SP, PCL);
		t += 6;
		PC = W;
		break;

	case 0xce:			/* ACI n */
		P = memrdr(PC++);
		res = (F >> C_SHIFT) & 1;
		t += 3;
		goto finish_add;

	case 0xcf:			/* RST 1 */
		W = 0x08;
		t++;
		goto finish_call;

	case 0xd0:			/* RNC */
		res = !(F & C_FLAG);
		goto finish_retc;

	case 0xd1:			/* POP D */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		E = memrdr(SP++);
		D = memrdr(SP++);
		t += 6;
		break;

	case 0xd2:			/* JNC nn */
		res = !(F & C_FLAG);
		goto finish_jmpc;

	case 0xd3:			/* OUT n */
		P = memrdr(PC++);
		io_out(P, P, A);
		t += 6;
		break;

	case 0xd4:			/* CNC nn */
		res = !(F & C_FLAG);
		goto finish_callc;

	case 0xd5:			/* PUSH D */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, D);
		memwrt(--SP, E);
		t += 7;
		break;

	case 0xd6:			/* SUI n */
		P = memrdr(PC++);
		res = 0;
		t += 3;
		goto finish_sub;

	case 0xd7:			/* RST 2 */
		W = 0x10;
		t++;
		goto finish_call;

	case 0xd8:			/* RC */
		res = F & C_FLAG;
		goto finish_retc;

	case 0xda:			/* JC nn */
		res = F & C_FLAG;
		goto finish_jmpc;

	case 0xdb:			/* IN n */
		P = memrdr(PC++);
		A = io_in(P, P);
		t += 6;
		break;

	case 0xdc:			/* CC nn */
		res = F & C_FLAG;
		goto finish_callc;

	case 0xde:			/* SBI n */
		P = memrdr(PC++);
		res = (F >> C_SHIFT) & 1;
		t += 3;
		goto finish_sub;

	case 0xdf:			/* RST 3 */
		W = 0x18;
		t++;
		goto finish_call;

	case 0xe0:			/* RPO */
		res = !(F & P_FLAG);
		goto finish_retc;

	case 0xe1:			/* POP H */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		L = memrdr(SP++);
		H = memrdr(SP++);
		t += 6;
		break;

	case 0xe2:			/* JPO nn */
		res = !(F & P_FLAG);
		goto finish_jmpc;

	case 0xe3:			/* XTHL */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		P = memrdr(SP);
		memwrt(SP, L);
		L = P;
		P = memrdr(SP + 1);
		memwrt(SP + 1, H);
		H = P;
		t += 14;
		break;

	case 0xe4:			/* CPO nn */
		res = !(F & P_FLAG);
		goto finish_callc;

	case 0xe5:			/* PUSH H */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, H);
		memwrt(--SP, L);
		t += 7;
		break;

	case 0xe6:			/* ANI n */
		P = memrdr(PC++);
		t += 3;
		goto finish_ana;

	case 0xe7:			/* RST 4 */
		W = 0x20;
		t++;
		goto finish_call;

	case 0xe8:			/* RPE */
		res = F & P_FLAG;
		goto finish_retc;

	case 0xe9:			/* PCHL */
		PC = HL;
		t++;
		break;

	case 0xea:			/* JPE nn */
		res = F & P_FLAG;
		goto finish_jmpc;

	case 0xeb:			/* XCHG */
		P = D;
		D = H;
		H = P;
		P = E;
		E = L;
		L = P;
		break;

	case 0xec:			/* CPE nn */
		res = F & P_FLAG;
		goto finish_callc;

	case 0xee:			/* XRI n */
		P = memrdr(PC++);
		t += 3;
		goto finish_xra;

	case 0xef:			/* RST 5 */
		W = 0x28;
		t++;
		goto finish_call;

	case 0xf0:			/* RP */
		res = !(F & S_FLAG);
		goto finish_retc;

	case 0xf1:			/* POP PSW */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		F = memrdr(SP++);
		A = memrdr(SP++);
		t += 6;
		break;

	case 0xf2:			/* JP nn */
		res = !(F & S_FLAG);
		goto finish_jmpc;

	case 0xf3:			/* DI */
		IFF = 0;
		break;

	case 0xf4:			/* CP nn */
		res = !(F & S_FLAG);
		goto finish_callc;

	case 0xf5:			/* PUSH PSW */
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, A);
		memwrt(--SP, ((F & ~(Y_FLAG | X_FLAG)) | N_FLAG));
		t += 7;
		break;

	case 0xf6:			/* ORI n */
		P = memrdr(PC++);
		t += 3;
		goto finish_ora;

	case 0xf7:			/* RST 6 */
		W = 0x30;
		t++;
		goto finish_call;

	case 0xf8:			/* RM */
		res = F & S_FLAG;
		goto finish_retc;

	case 0xf9:			/* SPHL */
#ifdef FRONTPANEL
		if (F_flag)
			addr_leds(HL);
#endif
		SP = HL;
		t++;
		break;

	case 0xfa:			/* JM nn */
		res = F & S_FLAG;
		goto finish_jmpc;

	case 0xfb:			/* EI */
		IFF = 3;
		int_protection = 1;	/* protect next instruction */
		break;

	case 0xfc:			/* CM nn */
		res = F & S_FLAG;
		goto finish_callc;

	case 0xfe:			/* CPI n */
		P = memrdr(PC++);
		t += 3;
		goto finish_cmp;

	case 0xff:			/* RST 7 */
		W = 0x38;
		t++;
		goto finish_call;
	}

	T += t;

#undef W
#undef WH
#undef WL

#undef H_SHIFT
#undef C_SHIFT

}

#endif /* !ALT8080_INC */
