/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef ALTZ80_INC
#define ALTZ80_INC

/*
 *	This module builds the Z80 central processing unit.
 *	The opcode where PC points to is fetched from the memory
 *	and PC incremented by one. The opcode is then dispatched
 *	to execute code, which emulates this Z80 opcode.
 *
 *	This implementation is primarily optimized for code size
 *	and written as a single block of code.
 *
 *	For a description of how the arithmetic flags calculation works see:
 *	http://emulators.com/docs/lazyoverflowdetect_final.pdf
 *
 *	The formula for subtraction carry outs was determined by staring
 *	intently at some truth tables.
 *
 *	cout contains the carry outs for every bit
 */

{
	extern BYTE io_in(BYTE, BYTE);
	extern void io_out(BYTE, BYTE, BYTE);

#define S_SHIFT		7	/* S_FLAG shift */
#define Z_SHIFT		6	/* Z_FLAG shift */
#define H_SHIFT		4	/* H_FLAG shift */
#define P_SHIFT		2	/* P_FLAG shift */
#define N_SHIFT		1	/* N_FLAG shift */
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

	BYTE t, res, cout, P, op, n, curr_ir;
	WORD W;
	WORD IR;	/* current index register(HL, IX, IY) */
#ifdef FAST_BLOCK
	WORD s, d;
	long tl;	/* loops can run for 65535 * 21 + 16 cycles */
#endif

#define IR_HL	0	/* values for curr_ir */
#define IR_IX	1
#define IR_IY	2

	t = 0;
	curr_ir = IR_HL;
	IR = (H << 8) | L;

next_opcode:

	t += 4;

	switch (memrdr(PC++)) {	/* execute next opcode */

	case 0x00:			/* NOP */
	case 0x40:			/* LD B,B */
	case 0x49:			/* LD C,C */
	case 0x52:			/* LD D,D */
	case 0x5b:			/* LD E,E */
	case 0x64:			/* LD irh,irh */
	case 0x6d:			/* LD irl,irl */
	case 0x7f:			/* LD A,A */
		break;

	case 0x01:			/* LD BC,nn */
		C = memrdr(PC++);
		B = memrdr(PC++);
		t += 6;
		break;

	case 0x02:			/* LD (BC),A */
		memwrt((B << 8) | C, A);
		t += 3;
		break;

	case 0x03:			/* INC BC */
		W = ((B << 8) | C) + 1;
		B = W >> 8;
		C = W & 0xff;
		t += 2;
		break;

	case 0x04:			/* INC B */
		P = B;
		res = ++B;
	finish_inc:
		cout = (P & 1) | ((P | 1) & ~res);
		F = ((F & C_FLAG) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     (szp_flags[res] & ~P_FLAG));
		/* N_FLAG cleared, C_FLAG unchanged */
		break;

	case 0x05:			/* DEC B */
		P = B;
		res = --B;
	finish_dec:
		cout = (~P & 1) | ((~P | 1) & res);
		F = ((F & C_FLAG) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     N_FLAG |
		     (szp_flags[res] & ~P_FLAG));
		/* C_FLAG unchanged */
		break;

	case 0x06:			/* LD B,n */
		B = memrdr(PC++);
		t += 3;
		break;

	case 0x07:			/* RLCA */
		res = ((A & 0x80) >> 7) & 1;
		F = (F & ~(H_FLAG | N_FLAG | C_FLAG)) | (res << C_SHIFT);
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		A = (A << 1) | res;
		break;

	case 0x08:			/* EX AF,AF' */
		P = A;
		A = A_;
		A_ = P;
		P = F;
		F = F_;
		F_ = P;
		break;

	case 0x09:			/* ADD ir,BC */
		W = IR + ((B << 8) | C);
		cout = ((IR >> 8) & B) | (((IR >> 8) | B) & ~(W >> 8));
	finish_addhl:
		F = ((F & ~(H_FLAG | N_FLAG | C_FLAG)) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     (((cout >> 7) & 1) << C_SHIFT));
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		IR = W;
		t += 7;
		break;

	case 0x0a:			/* LD A,(BC) */
		A = memrdr((B << 8) | C);
		t += 3;
		break;

	case 0x0b:			/* DEC BC */
		W = ((B << 8) | C) - 1;
		B = W >> 8;
		C = W & 0xff;
		t += 2;
		break;

	case 0x0c:			/* INC C */
		P = C;
		res = ++C;
		goto finish_inc;

	case 0x0d:			/* DEC C */
		P = C;
		res = --C;
		goto finish_dec;

	case 0x0e:			/* LD C,n */
		C = memrdr(PC++);
		t += 3;
		break;

	case 0x0f:			/* RRCA */
		res = A & 1;
		F = (F & ~(H_FLAG | N_FLAG | C_FLAG)) | (res << C_SHIFT);
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		A = (A >> 1) | (res << 7);
		break;

	case 0x10:			/* DJNZ n */
		P = memrdr(PC++);
		t++;
		if (--B) {
			PC += (signed char) P;
			t += 8;
		}
		break;

	case 0x11:			/* LD DE,nn */
		E = memrdr(PC++);
		D = memrdr(PC++);
		t += 6;
		break;

	case 0x12:			/* LD (DE),A */
		memwrt((D << 8) | E, A);
		t += 3;
		break;

	case 0x13:			/* INC DE */
		W = ((D << 8) | E) + 1;
		D = W >> 8;
		E = W & 0xff;
		t += 2;
		break;

	case 0x14:			/* INC D */
		P = D;
		res = ++D;
		goto finish_inc;

	case 0x15:			/* DEC D */
		P = D;
		res = --D;
		goto finish_dec;

	case 0x16:			/* LD D,n */
		D = memrdr(PC++);
		t += 3;
		break;

	case 0x17:			/* RLA */
		res = (F >> C_SHIFT) & 1;
		F = ((F & ~(H_FLAG | N_FLAG | C_FLAG)) |
		     ((((A & 0x80) >> 7) & 1) << C_SHIFT));
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		A = (A << 1) | res;
		break;

	case 0x18:			/* JR n */
		P = memrdr(PC++);
		PC += (signed char) P;
		t += 8;
		break;

	case 0x19:			/* ADD ir,DE */
		W = IR + ((D << 8) | E);
		cout = ((IR >> 8) & D) | (((IR >> 8) | D) & ~(W >> 8));
		goto finish_addhl;

	case 0x1a:			/* LD A,(DE) */
		A = memrdr((D << 8) | E);
		t += 3;
		break;

	case 0x1b:			/* DEC DE */
		W = ((D << 8) | E) - 1;
		D = W >> 8;
		E = W & 0xff;
		t += 2;
		break;

	case 0x1c:			/* INC E */
		P = E;
		res = ++E;
		goto finish_inc;

	case 0x1d:			/* DEC E */
		P = E;
		res = --E;
		goto finish_dec;

	case 0x1e:			/* LD E,n */
		E = memrdr(PC++);
		t += 3;
		break;

	case 0x1f:			/* RRA */
		res = (F >> C_SHIFT) & 1;
		F = (F & ~(H_FLAG | N_FLAG | C_FLAG)) | ((A & 1) << C_SHIFT);
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		A = (A >> 1) | (res << 7);
		break;

	case 0x20:			/* JR NZ,n */
		res = !(F & Z_FLAG);
	finish_jrc:
		P = memrdr(PC++);
		t += 3;
		if (res) {
			PC += (signed char) P;
			t += 5;
		}
		break;

	case 0x21:			/* LD ir,nn */
		IR = memrdr(PC++);
		IR |= memrdr(PC++) << 8;
		t += 6;
		break;

	case 0x22:			/* LD (nn),ir */
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		memwrt(W, IR & 0xff);
		memwrt(W + 1, IR >> 8);
		t += 12;
		break;

	case 0x23:			/* INC ir */
		IR++;
		t += 2;
		break;

	case 0x24:			/* INC irh */
		P = IR >> 8;
		res = P + 1;
		IR = (IR & 0xff) | (res << 8);
		goto finish_inc;

	case 0x25:			/* DEC irh */
		P = IR >> 8;
		res = P - 1;
		IR = (IR & 0xff) | (res << 8);
		goto finish_dec;

	case 0x26:			/* LD irh,n */
		IR = (IR & 0xff) | (memrdr(PC++) << 8);
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
		if (F & N_FLAG) {
			res = A - P;
			cout = (~A & P) | ((~A | P) & res);
		} else  {
			res = A + P;
			cout = (A & P) | ((A | P) & ~res);
		}
		F = ((F & (N_FLAG | C_FLAG)) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     szp_flags[res]);
		/* N_FLAG unchanged */
		A = res;
		break;

	case 0x28:			/* JR Z,n */
		res = F & Z_FLAG;
		goto finish_jrc;

	case 0x29:			/* ADD ir,ir */
		W = IR << 1;
		cout = (IR >> 8) | ((IR >> 8) & ~(W >> 8));
		goto finish_addhl;

	case 0x2a:			/* LD ir,(nn) */
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		IR = memrdr(W);
		IR |= memrdr(W + 1) << 8;
		t += 12;
		break;

	case 0x2b:			/* DEC ir */
		IR--;
		t += 2;
		break;

	case 0x2c:			/* INC irl */
		P = IR & 0xff;
		res = P + 1;
		IR = (IR & ~0xff) | res;
		goto finish_inc;

	case 0x2d:			/* DEC irl */
		P = IR & 0xff;
		res = P - 1;
		IR = (IR & ~0xff) | res;
		goto finish_dec;

	case 0x2e:			/* LD irl,n */
		IR = (IR & ~0xff) | memrdr(PC++);
		t += 3;
		break;

	case 0x2f:			/* CPL */
		A = ~A;
		F |= H_FLAG | N_FLAG;
		/* S_FLAG, Z_FLAG, P_FLAG, and C_FLAG unchanged */
		break;

	case 0x30:			/* JR NC,n */
		res = !(F & C_FLAG);
		goto finish_jrc;

	case 0x31:			/* LD SP,nn */
		SP = memrdr(PC++);
		SP |= memrdr(PC++) << 8;
		t += 6;
		break;

	case 0x32:			/* LD (nn),A */
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		memwrt(W, A);
		t += 9;
		break;

	case 0x33:			/* INC SP */
		SP++;
		t += 2;
		break;

	case 0x34:			/* INC (ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		res = P + 1;
		memwrt(W, res);
		t += 7;
		goto finish_inc;

	case 0x35:			/* DEC (ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		res = P - 1;
		memwrt(W, res);
		t += 7;
		goto finish_dec;

	case 0x36:			/* LD (ir),n */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 5;
		}
		memwrt(W, memrdr(PC++));
		t += 6;
		break;

	case 0x37:			/* SCF */
		F |= C_FLAG;
		F &= ~(N_FLAG | H_FLAG);
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		break;

	case 0x38:			/* JR C,n */
		res = F & C_FLAG;
		goto finish_jrc;

	case 0x39:			/* ADD ir,SP */
		W = IR + SP;
		cout = (((IR >> 8) & (SP >> 8)) |
			(((IR >> 8) | (SP >> 8)) & ~(W >> 8)));
		goto finish_addhl;

	case 0x3a:			/* LD A,(nn) */
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		A = memrdr(W);
		t += 9;
		break;

	case 0x3b:			/* DEC SP */
		SP--;
		t += 2;
		break;

	case 0x3c:			/* INC A */
		P = A;
		res = ++A;
		goto finish_inc;

	case 0x3d:			/* DEC A */
		P = A;
		res = --A;
		goto finish_dec;

	case 0x3e:			/* LD A,n */
		A = memrdr(PC++);
		t += 3;
		break;

	case 0x3f:			/* CCF */
		if (F & C_FLAG) {
			F |= H_FLAG;
			F &= ~C_FLAG;
		} else {
			F &= ~H_FLAG;
			F |= C_FLAG;
		}
		F &= ~N_FLAG;
		/* S_FLAG, Z_FLAG, and P_FLAG unchanged */
		break;

	case 0x41:			/* LD B,C */
		B = C;
		break;

	case 0x42:			/* LD B,D */
		B = D;
		break;

	case 0x43:			/* LD B,E */
		B = E;
		break;

	case 0x44:			/* LD B,irh */
		B = IR >> 8;
		break;

	case 0x45:			/* LD B,irl */
		B = IR & 0xff;
		break;

	case 0x46:			/* LD B,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		B = memrdr(W);
		t += 3;
		break;

	case 0x47:			/* LD B,A */
		B = A;
		break;

	case 0x48:			/* LD C,B */
		C = B;
		break;

	case 0x4a:			/* LD C,D */
		C = D;
		break;

	case 0x4b:			/* LD C,E */
		C = E;
		break;

	case 0x4c:			/* LD C,irh */
		C = IR >> 8;
		break;

	case 0x4d:			/* LD C,irl */
		C = IR & 0xff;
		break;

	case 0x4e:			/* LD C,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		C = memrdr(W);
		t += 3;
		break;

	case 0x4f:			/* LD C,A */
		C = A;
		break;

	case 0x50:			/* LD D,B */
		D = B;
		break;

	case 0x51:			/* LD D,C */
		D = C;
		break;

	case 0x53:			/* LD D,E */
		D = E;
		break;

	case 0x54:			/* LD D,irh */
		D = IR >> 8;
		break;

	case 0x55:			/* LD D,irl */
		D = IR & 0xff;
		break;

	case 0x56:			/* LD D,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		D = memrdr(W);
		t += 3;
		break;

	case 0x57:			/* LD D,A */
		D = A;
		break;

	case 0x58:			/* LD E,B */
		E = B;
		break;

	case 0x59:			/* LD E,C */
		E = C;
		break;

	case 0x5a:			/* LD E,D */
		E = D;
		break;

	case 0x5c:			/* LD E,irh */
		E = IR >> 8;
		break;

	case 0x5d:			/* LD E,irl */
		E = IR & 0xff;
		break;

	case 0x5e:			/* LD E,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		E = memrdr(W);
		t += 3;
		break;

	case 0x5f:			/* LD E,A */
		E = A;
		break;

	case 0x60:			/* LD irh,B */
		IR = (IR & 0xff) | (B << 8);
		break;

	case 0x61:			/* LD irh,C */
		IR = (IR & 0xff) | (C << 8);
		break;

	case 0x62:			/* LD irh,D */
		IR = (IR & 0xff) | (D << 8);
		break;

	case 0x63:			/* LD irh,E */
		IR = (IR & 0xff) | (E << 8);
		break;

	case 0x65:			/* LD irh,irl */
		IR = (IR & 0xff) | ((IR & 0xff) << 8);
		break;

	case 0x66:			/* LD H,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
			H = memrdr(W);
		} else
		IR = (IR & 0xff) | (memrdr(W) << 8);
		t += 3;
		break;

	case 0x67:			/* LD irh,A */
		IR = (IR & 0xff) | (A << 8);
		break;

	case 0x68:			/* LD irl,B */
		IR = (IR & ~0xff) | B;
		break;

	case 0x69:			/* LD irl,C */
		IR = (IR & ~0xff) | C;
		break;

	case 0x6a:			/* LD irl,D */
		IR = (IR & ~0xff) | D;
		break;

	case 0x6b:			/* LD irl,E */
		IR = (IR & ~0xff) | E;
		break;

	case 0x6c:			/* LD irl,irh */
		IR = (IR & ~0xff) | (IR >> 8);
		break;

	case 0x6e:			/* LD L,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
			L = memrdr(W);
		} else
			IR = (IR & ~0xff) | memrdr(W);
		t += 3;
		break;

	case 0x6f:			/* LD irl,A */
		IR = (IR & ~0xff) | A;
		break;

	case 0x70:			/* LD (ir),B */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		memwrt(W, B);
		t += 3;
		break;

	case 0x71:			/* LD (ir),C */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		memwrt(W, C);
		t += 3;
		break;

	case 0x72:			/* LD (ir),D */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		memwrt(W, D);
		t += 3;
		break;

	case 0x73:			/* LD (ir),E */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		memwrt(W, E);
		t += 3;
		break;

	case 0x74:			/* LD (ir),H */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
			memwrt(W, H);
		} else
			memwrt(W, IR >> 8);
		t += 3;
		break;

	case 0x75:			/* LD (ir),L */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
			memwrt(W, L);
		} else
			memwrt(W, IR & 0xff);
		t += 3;
		break;

	case 0x76:			/* HALT */
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_HLTA | CPU_MEMR;
#endif

#ifdef FRONTPANEL
		if (!F_flag) {
#endif
			if (IFF == 0) {
				/* without a frontpanel DI + HALT
				   stops the machine */
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
				cpu_bus = CPU_INTA | CPU_WO |
					  CPU_HLTA | CPU_M1;
#endif

			busy_loop_cnt = 0;

#ifdef FRONTPANEL
		} else {
			fp_led_address = 0xffff;
			fp_led_data = 0xff;

			if (IFF == 0) {
				/* INT disabled, wait for NMI,
				   frontpanel reset or user interrupt */
				while ((int_nmi == 0) &&
				       !(cpu_state & RESET)) {
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
		break;

	case 0x77:			/* LD (ir),A */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		memwrt(W, A);
		t += 3;
		break;

	case 0x78:			/* LD A,B */
		A = B;
		break;

	case 0x79:			/* LD A,C */
		A = C;
		break;

	case 0x7a:			/* LD A,D */
		A = D;
		break;

	case 0x7b:			/* LD A,E */
		A = E;
		break;

	case 0x7c:			/* LD A,irh */
		A = IR >> 8;
		break;

	case 0x7d:			/* LD A,irl */
		A = IR & 0xff;
		break;

	case 0x7e:			/* LD A,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		A = memrdr(W);
		t += 3;
		break;

	case 0x80:			/* ADD A,B */
		P = B;
	finish_add:
		res = A + P;
		cout = (A & P) | ((A | P) & ~res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     (szp_flags[res] & ~P_FLAG));
		/* N_FLAG cleared */
		A = res;
		break;

	case 0x81:			/* ADD A,C */
		P = C;
		goto finish_add;

	case 0x82:			/* ADD A,D */
		P = D;
		goto finish_add;

	case 0x83:			/* ADD A,E */
		P = E;
		goto finish_add;

	case 0x84:			/* ADD A,irh */
		P = IR >> 8;
		goto finish_add;

	case 0x85:			/* ADD A,irl */
		P = IR & 0xff;
		goto finish_add;

	case 0x86:			/* ADD A,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_add;

	case 0x87:			/* ADD A,A */
		P = A;
		goto finish_add;

	case 0x88:			/* ADC A,B */
		P = B;
	finish_adc:
		res = A + P + ((F >> C_SHIFT) & 1);
		cout = (A & P) | ((A | P) & ~res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     (szp_flags[res] & ~P_FLAG));
		/* N_FLAG cleared */
		A = res;
		break;

	case 0x89:			/* ADC A,C */
		P = C;
		goto finish_adc;

	case 0x8a:			/* ADC A,D */
		P = D;
		goto finish_adc;

	case 0x8b:			/* ADC A,E */
		P = E;
		goto finish_adc;

	case 0x8c:			/* ADC A,irh */
		P = IR >> 8;
		goto finish_adc;

	case 0x8d:			/* ADC A,irl */
		P = IR & 0xff;
		goto finish_adc;

	case 0x8e:			/* ADC A,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_adc;

	case 0x8f:			/* ADC A,A */
		P = A;
		goto finish_adc;

	case 0x90:			/* SUB A,B */
		P = B;
	finish_sub:
		res = A - P;
		cout = (~A & P) | ((~A | P) & res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     N_FLAG |
		     (szp_flags[res] & ~P_FLAG));
		A = res;
		break;

	case 0x91:			/* SUB A,C */
		P = C;
		goto finish_sub;

	case 0x92:			/* SUB A,D */
		P = D;
		goto finish_sub;

	case 0x93:			/* SUB A,E */
		P = E;
		goto finish_sub;

	case 0x94:			/* SUB A,irh */
		P = IR >> 8;
		goto finish_sub;

	case 0x95:			/* SUB A,irl */
		P = IR & 0xff;
		goto finish_sub;

	case 0x96:			/* SUB A,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_sub;

	case 0x97:			/* SUB A,A */
		F = Z_FLAG | N_FLAG;
		/* S_FLAG, H_FLAG, P_FLAG, and C_FLAG cleared */
		A = 0;
		break;

	case 0x98:			/* SBC A,B */
		P = B;
	finish_sbc:
		res = A - P - ((F >> C_SHIFT) & 1);
		cout = (~A & P) | ((~A | P) & res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     N_FLAG |
		     (szp_flags[res] & ~P_FLAG));
		A = res;
		break;

	case 0x99:			/* SBC A,C */
		P = C;
		goto finish_sbc;

	case 0x9a:			/* SBC A,D */
		P = D;
		goto finish_sbc;

	case 0x9b:			/* SBC A,E */
		P = E;
		goto finish_sbc;

	case 0x9c:			/* SBC A,irh */
		P = IR >> 8;
		goto finish_sbc;

	case 0x9d:			/* SBC A,irl */
		P = IR & 0xff;
		goto finish_sbc;

	case 0x9e:			/* SBC A,(ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_sbc;

	case 0x9f:			/* SBC A,A */
		P = A;
		goto finish_sbc;

	case 0xa0:			/* AND B */
		P = B;
	finish_and:
		res = A & P;
		F = H_FLAG | szp_flags[res];
		/* N_FLAG and C_FLAG cleared */
		A = res;
		break;

	case 0xa1:			/* AND C */
		P = C;
		goto finish_and;

	case 0xa2:			/* AND D */
		P = D;
		goto finish_and;

	case 0xa3:			/* AND E */
		P = E;
		goto finish_and;

	case 0xa4:			/* AND irh */
		P = IR >> 8;
		goto finish_and;

	case 0xa5:			/* AND irl */
		P = IR & 0xff;
		goto finish_and;

	case 0xa6:			/* AND (ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_and;

	case 0xa7:			/* AND A */
		P = A;
		goto finish_and;

	case 0xa8:			/* XOR B */
		P = B;
	finish_xor:
		res = A ^ P;
		F = szp_flags[res];
		/* H_FLAG, N_FLAG, and C_FLAG cleared */
		A = res;
		break;

	case 0xa9:			/* XOR C */
		P = C;
		goto finish_xor;

	case 0xaa:			/* XOR D */
		P = D;
		goto finish_xor;

	case 0xab:			/* XOR E */
		P = E;
		goto finish_xor;

	case 0xac:			/* XOR irh */
		P = IR >> 8;
		goto finish_xor;

	case 0xad:			/* XOR irl */
		P = IR & 0xff;
		goto finish_xor;

	case 0xae:			/* XOR (ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_xor;

	case 0xaf:			/* XOR A */
		F = Z_FLAG | P_FLAG;
		/* S_FLAG, H_FLAG, N_FLAG, and C_FLAG cleared */
		A = 0;
		break;

	case 0xb0:			/* OR B */
		P = B;
	finish_or:
		res = A | P;
		F = szp_flags[res];
		/* H_FLAG, N_FLAG, and C_FLAG cleared */
		A = res;
		break;

	case 0xb1:			/* OR C */
		P = C;
		goto finish_or;

	case 0xb2:			/* OR D */
		P = D;
		goto finish_or;

	case 0xb3:			/* OR E */
		P = E;
		goto finish_or;

	case 0xb4:			/* OR irh */
		P = IR >> 8;
		goto finish_or;

	case 0xb5:			/* OR irl */
		P = IR & 0xff;
		goto finish_or;

	case 0xb6:			/* OR (ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_or;

	case 0xb7:			/* OR A */
		F = szp_flags[A];
		/* H_FLAG, N_FLAG, and C_FLAG cleared */
		break;

	case 0xb8:			/* CP B */
		P = B;
	finish_cp:
		res = A - P;
		cout = (~A & P) | ((~A | P) & res);
		F = ((((cout >> 7) & 1) << C_SHIFT) |
		     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
		     (((cout >> 3) & 1) << H_SHIFT) |
		     N_FLAG |
		     (szp_flags[res] & ~P_FLAG));
		break;

	case 0xb9:			/* CP C */
		P = C;
		goto finish_cp;

	case 0xba:			/* CP D */
		P = D;
		goto finish_cp;

	case 0xbb:			/* CP E */
		P = E;
		goto finish_cp;

	case 0xbc:			/* CP irh */
		P = IR >> 8;
		goto finish_cp;

	case 0xbd:			/* CP irl */
		P = IR & 0xff;
		goto finish_cp;

	case 0xbe:			/* CP (ir) */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		}
		P = memrdr(W);
		t += 3;
		goto finish_cp;

	case 0xbf:			/* CP A */
		F = Z_FLAG | N_FLAG;
		/* S_FLAG, H_FLAG, P_FLAG, and C_FLAG cleared */
		break;

	case 0xc0:			/* RET NZ */
		res = !(F & Z_FLAG);
	finish_retc:
		t++;
		if (res)
			goto finish_ret;
		break;

	case 0xc1:			/* POP BC */
		C = memrdr(SP++);
		B = memrdr(SP++);
		t += 6;
		break;

	case 0xc2:			/* JP NZ,nn */
		res = !(F & Z_FLAG);
	finish_jpc:
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		t += 6;
		if (res)
			PC = W;
		break;

	case 0xc3:			/* JP nn */
		W = memrdr(PC++);
		W |= memrdr(PC) << 8;
		t += 6;
		PC = W;
		break;

	case 0xc4:			/* CALL NZ,nn */
		res = !(F & Z_FLAG);
	finish_callc:
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		t += 6;
		if (res)
			goto finish_call;
		break;

	case 0xc5:			/* PUSH BC */
		memwrt(--SP, B);
		memwrt(--SP, C);
		t += 7;
		break;

	case 0xc6:			/* ADD A,n */
		P = memrdr(PC++);
		t += 3;
		goto finish_add;

	case 0xc7:			/* RST 00 */
		W = 0;
		goto finish_call;

	case 0xc8:			/* RET Z */
		res = F & Z_FLAG;
		goto finish_retc;

	case 0xc9:			/* RET */
	finish_ret:
		W = memrdr(SP++);
		W |= memrdr(SP++) << 8;
		t += 6;
		PC = W;
		break;

	case 0xca:			/* JP Z,nn */
		res = F & Z_FLAG;
		goto finish_jpc;

	case 0xcb:			/* 0xcb prefix */
		W = IR;
		if (curr_ir != IR_HL) {
			W += (signed char) memrdr(PC++);
			t += 8;
		} else {
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

			R++;		/* increment refresh register */
		}

		t += 4;

		res = 0;		/* silence compiler */

		op = memrdr(PC++);
		n = (op >> 3) & 7;
		if (curr_ir != IR_HL)
			P = memrdr(W);
		else {
			switch (op & 7) {
			case 0:
				P = B;
				break;
			case 1:
				P = C;
				break;
			case 2:
				P = D;
				break;
			case 3:
				P = E;
				break;
			case 4:
				P = IR >> 8;
				break;
			case 5:
				P = IR & 0xff;
				break;
			case 6:
				P = memrdr(W);
				t += 4;
				break;
			case 7:
				P = A;
				break;
			}
		}
		switch (op & 0xc0) {
		case 0x00:
			switch (n) {
			case 0:		/* RLC */
				res = (P << 1) | (P >> 7);
				F = (res & 1) << C_SHIFT;
				break;
			case 1:		/* RRC */
				res = (P >> 1) | (P << 7);
				F = ((res & 0x80) >> 7) << C_SHIFT;
				break;
			case 2:		/* RL */
				res = (P << 1) | ((F & C_FLAG) >> C_SHIFT);
				F = ((P & 0x80) >> 7) << C_SHIFT;
				break;
			case 3:		/* RR */
				res = ((P >> 1) |
				       (((F & C_FLAG) >> C_SHIFT) << 7));
				F = (P & 1) << C_SHIFT;
				break;
			case 4:		/* SLA */
				res = P << 1;
				F = ((P & 0x80) >> 7) << C_SHIFT;
				break;
			case 5:		/* SRA */
				res = (P >> 1) | (P & 0x80);
				F = (P & 1) << C_SHIFT;
				break;
			case 6:		/* SLL */
				res = (P << 1) | 1;
				F = ((P & 0x80) >> 7) << C_SHIFT;
				break;
			case 7:		/* SRL */
				res = P >> 1;
				F = (P & 1) << C_SHIFT;
				break;
			}
			F = (F & C_FLAG) | szp_flags[res];
			/* H_FLAG and N_FLAG cleared */
			break;
		case 0x40:		/* BIT n */
			res = P & (1 << n);
			F = (F & C_FLAG) | H_FLAG | szp_flags[res];
			/* N_FLAG cleared, C_FLAG unchanged */
			goto end_cb;
		case 0x80:		/* RES n */
			res = P & ~(1 << n);
			break;
		case 0xc0:		/* SET n */
			res = P | (1 << n);
			break;
		}
		if (curr_ir != IR_HL)
			memwrt(W, res);
		switch (op & 7) {
		case 0:
			B = res;
			break;
		case 1:
			C = res;
			break;
		case 2:
			D = res;
			break;
		case 3:
			E = res;
			break;
		case 4:
			if (curr_ir != IR_HL)
				H = res;
			else
				IR = (IR & 0xff) | (res << 8);
			break;
		case 5:
			if (curr_ir != IR_HL)
				L = res;
			else
				IR = (IR & ~0xff) | res;
			break;
		case 6:
			if (curr_ir == IR_HL)
				memwrt(W, res);
			t += 3;
			break;
		case 7:
			A = res;
			break;
		}
	end_cb:
		break;

	case 0xcc:			/* CALL Z,nn */
		res = F & Z_FLAG;
		goto finish_callc;

	case 0xcd:			/* CALL nn */
		W = memrdr(PC++);
		W |= memrdr(PC++) << 8;
		t += 6;
	finish_call:
		memwrt(--SP, PC >> 8);
		memwrt(--SP, PC & 0xff);
		t += 7;
		PC = W;
		break;

	case 0xce:			/* ADC A,n */
		P = memrdr(PC++);
		t += 3;
		goto finish_adc;

	case 0xcf:			/* RST 08 */
		W = 0x08;
		goto finish_call;

	case 0xd0:			/* RET NC */
		res = !(F & C_FLAG);
		goto finish_retc;

	case 0xd1:			/* POP DE */
		E = memrdr(SP++);
		D = memrdr(SP++);
		t += 6;
		break;

	case 0xd2:			/* JP NC,nn */
		res = !(F & C_FLAG);
		goto finish_jpc;

	case 0xd3:			/* OUT (n),A */
		P = memrdr(PC++);
		io_out(P, A, A);
		t += 7;
		break;

	case 0xd4:			/* CALL NC,nn */
		res = !(F & C_FLAG);
		goto finish_callc;

	case 0xd5:			/* PUSH DE */
		memwrt(--SP, D);
		memwrt(--SP, E);
		t += 7;
		break;

	case 0xd6:			/* SUB A,n */
		P = memrdr(PC++);
		t += 3;
		goto finish_sub;

	case 0xd7:			/* RST 10 */
		W = 0x10;
		goto finish_call;

	case 0xd8:			/* RET C */
		res = F & C_FLAG;
		goto finish_retc;

	case 0xd9:			/* EXX */
		P = B;
		B = B_;
		B_ = P;
		P = C;
		C = C_;
		C_ = P;
		P = D;
		D = D_;
		D_ = P;
		P = E;
		E = E_;
		E_ = P;
		P = H;
		H = H_;
		H_ = P;
		P = L;
		L = L_;
		L_ = P;
		curr_ir = IR_HL;
		IR = (H << 8) | L;
		break;

	case 0xda:			/* JP C,nn */
		res = F & C_FLAG;
		goto finish_jpc;

	case 0xdb:			/* IN A,(n) */
		P = memrdr(PC++);
		A = io_in(P, A);
		t += 7;
		break;

	case 0xdc:			/* CALL C,nn */
		res = F & C_FLAG;
		goto finish_callc;

	case 0xdd:			/* 0xdd prefix */
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

		R++;			/* increment refresh register */

		curr_ir = IR_IX;
		IR = IX;
		goto next_opcode;

	case 0xde:			/* SBC A,n */
		P = memrdr(PC++);
		t += 3;
		goto finish_sbc;

	case 0xdf:			/* RST 18 */
		W = 0x18;
		goto finish_call;

	case 0xe0:			/* RET PO */
		res = !(F & P_FLAG);
		goto finish_retc;

	case 0xe1:			/* POP ir */
		IR = memrdr(SP++);
		IR |= memrdr(SP++) << 8;
		t += 6;
		break;

	case 0xe2:			/* JP PO,nn */
		res = !(F & P_FLAG);
		goto finish_jpc;

	case 0xe3:			/* EX (SP),ir */
		W = memrdr(SP);
		W |= memrdr(SP + 1) << 8;
		memwrt(SP, IR & 0xff);
		memwrt(SP + 1, IR >> 8);
		IR = W;
		t += 15;
		break;

	case 0xe4:			/* CALL PO,nn */
		res = !(F & P_FLAG);
		goto finish_callc;

	case 0xe5:			/* PUSH ir */
		memwrt(--SP, IR >> 8);
		memwrt(--SP, IR & 0xff);
		t += 7;
		break;

	case 0xe6:			/* AND n */
		P = memrdr(PC++);
		t += 3;
		goto finish_and;

	case 0xe7:			/* RST 20 */
		W = 0x20;
		goto finish_call;

	case 0xe8:			/* RET PE */
		res = F & P_FLAG;
		goto finish_retc;

	case 0xe9:			/* JP (ir) */
		PC = IR;
		break;

	case 0xea:			/* JP PE,nn */
		res = F & P_FLAG;
		goto finish_jpc;

	case 0xeb:			/* EX DE,HL */
		P = D;
		D = H;
		H = P;
		P = E;
		E = L;
		L = P;
	        curr_ir = IR_HL;
		IR = (H << 8) | L;
		break;

	case 0xec:			/* CALL PE,nn */
		res = F & P_FLAG;
		goto finish_callc;

	case 0xed:			/* 0xed prefix */
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

		R++;			/* increment refresh register */

		t += 4;

		switch (memrdr(PC++)) {
		case 0x40:		/* IN B,(C) */
			B = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[B];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x41:		/* OUT (C),B */
			io_out(C, B, B);
			t += 4;
			break;

		case 0x42:		/* SBC HL,BC */
			W = (((H << 8) | L) - ((B << 8) | C)
			     - ((F >> C_SHIFT) & 1));
			cout = (~H & B) | ((~H | B) & (W >> 8));
		finish_sbchl:
			F = ((((cout >> 7) & 1) << C_SHIFT) |
			     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
			     (((cout >> 3) & 1) << H_SHIFT) |
			     N_FLAG |
			     ((W == 0) << Z_SHIFT) |
			     ((((W >> 8) & 0x80) >> 7) << S_SHIFT));
			H = W >> 8;
			L = W & 0xff;
			t += 7;
			break;

		case 0x43:		/* LD (nn),BC */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			memwrt(W, C);
			memwrt(W + 1, B);
			t += 12;
			break;

		case 0x44:		/* NEG */
		case 0x4c:		/* NEG* */
		case 0x54:		/* NEG* */
		case 0x5c:		/* NEG* */
		case 0x64:		/* NEG* */
		case 0x6c:		/* NEG* */
		case 0x74:		/* NEG* */
		case 0x7c:		/* NEG* */
			res = 0 - A;
			cout = A | res;
			F = ((((cout >> 7) & 1) << C_SHIFT) |
			     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
			     (((cout >> 3) & 1) << H_SHIFT) |
			     N_FLAG |
			     (szp_flags[res] & ~P_FLAG));
			A = res;
			break;

		case 0x45:		/* RETN */
		case 0x55:		/* RETN* */
		case 0x65:		/* RETN* */
		case 0x75:		/* RETN* */
			W = memrdr(SP++);
			W |= memrdr(SP++) << 8;
			t += 6;
			PC = W;
			if (IFF & 2)
				IFF |= 1;
			break;

		case 0x46:		/* IM 0 */
		case 0x4e:		/* IM 0* */
		case 0x66:		/* IM 0* */
		case 0x6e:		/* IM 0* */
			int_mode = 0;
			break;

		case 0x47:		/* LD I,A */
			I = A;
			t++;
			break;

		case 0x48:		/* IN C,(C) */
			C = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[C];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x49:		/* OUT (C),C */
			io_out(C, B, C);
			t += 4;
			break;

		case 0x4a:		/* ADC HL,BC */
			W = (((H << 8) | L) + ((B << 8) | C) +
			     ((F >> C_SHIFT) & 1));
			cout = (H & B) | ((H | B) & ~(W >> 8));
		finish_adchl:
			F = ((((cout >> 7) & 1) << C_SHIFT) |
			     ((((cout + 64) >> 7) & 1) << P_SHIFT) |
			     (((cout >> 3) & 1) << H_SHIFT) |
			     ((W == 0) << Z_SHIFT) |
			     ((((W >> 8) & 0x80) >> 7) << S_SHIFT));
			/* N_FLAG cleared */
			H = W >> 8;
			L = W & 0xff;
			t += 7;
			break;

		case 0x4b:		/* LD BC,(nn) */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			C = memrdr(W);
			B = memrdr(W + 1);
			t += 12;
			break;

		case 0x4d:		/* RETI */
		case 0x5d:		/* RETI* */
		case 0x6d:		/* RETI* */
		case 0x7d:		/* RETI* */
			W = memrdr(SP++);
			W |= memrdr(SP++) << 8;
			t += 6;
			PC = W;
			break;

		case 0x4f:		/* LD R,A */
			R_ = R = A;
			t++;
			break;

		case 0x50:		/* IN D,(C) */
			D = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[D];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x51:		/* OUT (C),D */
			io_out(C, B, D);
			t += 4;
			break;

		case 0x52:		/* SBC HL,DE */
			W = (((H << 8) | L) - ((D << 8) | E) -
			     ((F >> C_SHIFT) & 1));
			cout = (~H & D) | ((~H | D) & (W >> 8));
			goto finish_sbchl;

		case 0x53:		/* LD (nn),DE */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			memwrt(W, E);
			memwrt(W + 1, D);
			t += 12;
			break;

		case 0x56:		/* IM 1 */
		case 0x76:		/* IM 1* */
			int_mode = 1;
			break;

		case 0x57:		/* LD A,I */
			A = I;
			F = ((F & C_FLAG) |
			     (((IFF >> 1) & 1) << P_SHIFT) |
			     (szp_flags[A] & ~P_FLAG));
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t++;
			break;

		case 0x58:		/* IN E,(C) */
			E = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[E];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x59:		/* OUT (C),E */
			io_out(C, B, E);
			t += 4;
			break;

		case 0x5a:		/* ADC HL,DE */
			W = (((H << 8) | L) + ((D << 8) | E) +
			     ((F >> C_SHIFT) & 1));
			cout = (H & D) | ((H | D) & ~(W >> 8));
			goto finish_adchl;

		case 0x5b:		/* LD DE,(nn) */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			E = memrdr(W);
			D = memrdr(W + 1);
			t += 12;
			break;

		case 0x5e:		/* IM 2 */
		case 0x7e:		/* IM 2* */
			int_mode = 2;
			break;

		case 0x5f:		/* LD A,R */
			A = (R_ & 0x80) | (R & 0x7f);
			F = ((F & C_FLAG) |
			     (((IFF >> 1) & 1) << P_SHIFT) |
			     (szp_flags[A] & ~P_FLAG));
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t++;
			break;

		case 0x60:		/* IN H,(C) */
			H = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[H];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x61:		/* OUT (C),H */
			io_out(C, B, H);
			t += 4;
			break;

		case 0x62:		/* SBC HL,HL */
			W = -((F >> C_SHIFT) & 1);
			cout = W >> 8;
			goto finish_sbchl;

		case 0x63:		/* LD (nn),HL */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			memwrt(W, L);
			memwrt(W + 1, H);
			t += 12;
			break;

		case 0x67:		/* RRD (HL) */
			P = memrdr((H << 8) | L);
			res = A & 0x0f;
			A = (A & 0xf0) | (P & 0x0f);
			memwrt((H << 8) | L, ((P >> 4) | (res << 4)));
			F = (F & C_FLAG) | szp_flags[A];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 10;
			break;

		case 0x68:		/* IN L,(C) */
			L = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[L];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x69:		/* OUT (C),L */
			io_out(C, B, L);
			t += 4;
			break;

		case 0x6a:		/* ADC HL,HL */
			W = (((H << 8) | L) << 1) + ((F >> C_SHIFT) & 1);
			cout = H | (H & ~(W >> 8));
			goto finish_adchl;

		case 0x6b:		/* LD HL,(nn) */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			L = memrdr(W);
			H = memrdr(W + 1);
			t += 12;
			break;

		case 0x6f:		/* RLD (HL) */
			P = memrdr((H << 8) | L);
			res = A & 0x0f;
			A = (A & 0xf0) | (P >> 4);
			memwrt((H << 8) | L, (P << 4) | res);
			F = (F & C_FLAG) | szp_flags[A];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 10;
			break;

		case 0x70:		/* IN F,(C) */
			res = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[res];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x71:		/* OUT (C),0 */
			io_out(C, B, 0); /* NMOS, CMOS outputs 0xff */
			t += 4;
			break;

		case 0x72:		/* SBC HL,SP */
			W = ((H << 8) | L) - SP - ((F >> C_SHIFT) & 1);
			cout = ((~H & (SP >> 8)) |
				((~H | (SP >> 8)) & (W >> 8)));
			goto finish_sbchl;

		case 0x73:		/* LD (nn),SP */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			memwrt(W, SP & 0xff);
			memwrt(W + 1, SP >> 8);
			t += 12;
			break;

		case 0x78:		/* IN A,(C) */
			A = io_in(C, B);
			F = (F & C_FLAG) | szp_flags[A];
			/* H_FLAG and N_FLAG cleared, C_FLAG unchanged */
			t += 4;
			break;

		case 0x79:		/* OUT (C),A */
			io_out(C, B, A);
			t += 4;
			break;

		case 0x7a:		/* ADC HL,SP */
			W = ((H << 8) | L) + SP + ((F >> C_SHIFT) & 1);
			cout = (H & (SP >> 8)) | ((H | (SP >> 8)) & ~(W >> 8));
			goto finish_adchl;

		case 0x7b:		/* LD SP,(nn) */
			W = memrdr(PC++);
			W |= memrdr(PC++) << 8;
			SP = memrdr(W);
			SP |= memrdr(W + 1) << 8;
			t += 12;
			break;

		case 0xa0:		/* LDI */
			memwrt((D << 8) | E, memrdr((H << 8) | L));
			W = ((D << 8) | E) + 1;
			D = W >> 8;
			E = W & 0xff;
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
		finish_ldid:
			W = ((B << 8) | C) - 1;
			B = W >> 8;
			C = W & 0xff;
			F = ((F & ~(H_FLAG | N_FLAG | P_FLAG)) |
			     (((B | C) != 0) << P_SHIFT));
			/* S_FLAG, Z_FLAG, and C_FLAG unchanged */
			t += 8;
			break;

		case 0xa1:		/* CPI */
			P = memrdr((H << 8) | L);
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
		finish_cpid:
			W = ((B << 8) | C) - 1;
			B = W >> 8;
			C = W & 0xff;
			res = A - P;
			cout = (~A & P) | ((~A | P) & res);
			F = ((F & C_FLAG) |
			     (((cout >> 3) & 1) << H_SHIFT) |
			     N_FLAG |
			     (((B | C) != 0) << P_SHIFT) |
			     (szp_flags[res] & ~P_FLAG));
			/* C_FLAG unchanged */
			t += 8;
			break;

		case 0xa2:		/* INI */
			res = io_in(C, B--);
			memwrt((H << 8) | L, res);
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
			W = (C + 1) & 0xff;
		finish_ioid:
			W += res;
			F = (((W >> 8) << H_SHIFT) | ((W >> 8) << C_SHIFT) |
			     ((((res & 0x80) >> 7) & 1) << N_SHIFT) |
			     (szp_flags[(W & 7) ^ B] & P_FLAG) |
			     (szp_flags[B] & ~P_FLAG));
			t += 8;
			break;

		case 0xa3:		/* OUTI */
			res = memrdr((H << 8) | L);
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
			io_out(C, --B, res);
			W = L;
			goto finish_ioid;

		case 0xa8:		/* LDD */
			memwrt((D << 8) | E, memrdr((H << 8) | L));
			W = ((D << 8) | E) - 1;
			D = W >> 8;
			E = W & 0xff;
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			goto finish_ldid;

		case 0xa9:		/* CPD */
			P = memrdr((H << 8) | L);
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			goto finish_cpid;

		case 0xaa:		/* IND */
			res = io_in(C, B--);
			memwrt((H << 8) | L, res);
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			W = (C - 1) & 0xff;
			goto finish_ioid;

		case 0xab:		/* OUTD */
			res = memrdr((H << 8) | L);
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			io_out(C, --B, res);
			W = L;
			goto finish_ioid;

#ifdef FAST_BLOCK
		case 0xb0:		/* LDIR */
			W = (B << 8) | C;
			d = (D << 8) | E;
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				memwrt(d++, memrdr(s++));
				tl += 21L;
				R += 2;
			} while (--W);
		finish_ldidr:
			B = C = 0;
			D = d >> 8;
			E = d & 0xff;
			H = s >> 8;
			L = s & 0xff;
			F &= ~(H_FLAG | N_FLAG | P_FLAG);
			/* S_FLAG, Z_FLAG, and C_FLAG unchanged */
			T += tl;
			break;

		case 0xb1:		/* CPIR */
			W = (B << 8) | C;
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				P = memrdr(s++);
				res = A - P;
				tl += 21L;
				R += 2;
			} while (--W && res);
		finish_cpidr:
			B = W >> 8;
			C = W & 0xff;
			H = s >> 8;
			L = s & 0xff;
			cout = (~A & P) | ((~A | P) & res);
			F = ((F & C_FLAG) |
			     (((cout >> 3) & 1) << H_SHIFT) |
			     N_FLAG |
			     ((W != 0) << P_SHIFT) |
			     (szp_flags[res] & ~P_FLAG));
			/* C_FLAG unchanged */
			T += tl;
			break;

		case 0xb2:		/* INIR */
			s = (H << 8) | L;
			R -= 2;
			tl = -13L;
			do {
				res = io_in(C, B--);
				memwrt(s++, res);
				tl += 21L;
				R += 2;
			} while (B);
			W = (C + 1) & 0xff;
		finish_ioidr:
			H = s >> 8;
			L = s & 0xff;
			W += res;
			F = (((W >> 8) << H_SHIFT) | ((W >> 8) << C_SHIFT) |
			     ((((res & 0x80) >> 7) & 1) << N_SHIFT) |
			     (szp_flags[W & 7] & P_FLAG) |
			     Z_FLAG);
			/* S_FLAG cleared */
			T += tl;
			break;

		case 0xb3:		/* OTIR */
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				res = memrdr(s++);
				io_out(C, --B, res);
				tl += 21L;
				R += 2;
			} while (B);
			W = s & 0xff;
			goto finish_ioidr;

		case 0xb8:		/* LDDR */
			W = (B << 8) | C;
			d = (D << 8) | E;
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				memwrt(d--, memrdr(s--));
				tl += 21L;
				R += 2;
			} while (--W);
			goto finish_ldidr;

		case 0xb9:		/* CPDR */
			W = (B << 8) | C;
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				P = memrdr(s--);
				res = A - P;
				tl += 21L;
				R += 2;
			} while (--W && res);
			goto finish_cpidr;

		case 0xba:		/* INDR */
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				res = io_in(C, B--);
				memwrt(s--, res);
				tl += 21L;
				R += 2;
			} while (B);
			W = (C - 1) & 0xff;
			goto finish_ioidr;

		case 0xbb:		/* OTDR */
			s = (H << 8) | L;
			tl = -13L;
			R -= 2;
			do {
				res = memrdr(s--);
				io_out(C, --B, res);
				tl += 21L;
				R += 2;
			} while (B);
			W = s & 0xff;
			goto finish_ioidr;
#else /* !FAST_BLOCK */
		case 0xb0:		/* LDIR */
			memwrt((D << 8) | E, memrdr((H << 8) | L));
			W = ((D << 8) | E) + 1;
			D = W >> 8;
			E = W & 0xff;
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
		finish_ldidr:
			W = ((B << 8) | C) - 1;
			B = W >> 8;
			C = W & 0xff;
			F = ((F & ~(H_FLAG | N_FLAG | P_FLAG)) |
			     (((B | C) != 0) << P_SHIFT));
			/* S_FLAG, Z_FLAG, and C_FLAG unchanged */
			t += 8;
			if (F & P_FLAG) {
				t += 5;
				PC -= 2;
			}
			break;

		case 0xb1:		/* CPIR */
			P = memrdr((H << 8) | L);
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
		finish_cpidr:
			W = ((B << 8) | C) - 1;
			B = W >> 8;
			C = W & 0xff;
			res = A - P;
			cout = (~A & P) | ((~A | P) & res);
			F = ((F & C_FLAG) |
			     (((cout >> 3) & 1) << H_SHIFT) |
			     N_FLAG |
			     (((B | C) != 0) << P_SHIFT) |
			     (szp_flags[res] & ~P_FLAG));
			/* C_FLAG unchanged */
			t += 8;
			if ((F & (P_FLAG | Z_FLAG)) == P_FLAG) {
				t += 5;
				PC -= 2;
			}
			break;

		case 0xb2:		/* INIR */
			res = io_in(C, B--);
			memwrt((H << 8) | L, res);
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
			W = (C + 1) & 0xff;
		finish_ioidr:
			W += res;
			F = (((W >> 8) << H_SHIFT) | ((W >> 8) << C_SHIFT) |
			     ((((res & 0x80) >> 7) & 1) << N_SHIFT) |
			     (szp_flags[(W & 7) ^ B] & P_FLAG) |
			     (szp_flags[B] & ~P_FLAG));
			t += 8;
			if (!(F & Z_FLAG)) {
				t += 5;
				PC -= 2;
			}
			break;

		case 0xb3:		/* OTIR */
			res = memrdr((H << 8) | L);
			W = ((H << 8) | L) + 1;
			H = W >> 8;
			L = W & 0xff;
			io_out(C, --B, res);
			W = L;
			goto finish_ioidr;

		case 0xb8:		/* LDDR */
			memwrt((D << 8) | E, memrdr((H << 8) | L));
			W = ((D << 8) | E) - 1;
			D = W >> 8;
			E = W & 0xff;
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			goto finish_ldidr;

		case 0xb9:		/* CPDR */
			P = memrdr((H << 8) | L);
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			goto finish_cpidr;

		case 0xba:		/* INDR */
			res = io_in(C, B--);
			memwrt((H << 8) | L, res);
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			W = (C - 1) & 0xff;
			goto finish_ioidr;

		case 0xbb:		/* OTDR */
			res = memrdr((H << 8) | L);
			W = ((H << 8) | L) - 1;
			H = W >> 8;
			L = W & 0xff;
			io_out(C, --B, res);
			W = L;
			goto finish_ioidr;
#endif /* !FAST_BLOCK */

		default:		/* NOP* */
			break;
		}
		curr_ir = IR_HL;
		IR = (H << 8) | L;
		break;

	case 0xee:			/* XOR n */
		P = memrdr(PC++);
		t += 3;
		goto finish_xor;

	case 0xef:			/* RST 28 */
		W = 0x28;
		goto finish_call;

	case 0xf0:			/* RET P */
		res = !(F & S_FLAG);
		goto finish_retc;

	case 0xf1:			/* POP AF */
		F = memrdr(SP++);
		A = memrdr(SP++);
		t += 6;
		break;

	case 0xf2:			/* JP P,nn */
		res = !(F & S_FLAG);
		goto finish_jpc;

	case 0xf3:			/* DI */
		IFF = 0;
		break;

	case 0xf4:			/* CALL P,nn */
		res = !(F & S_FLAG);
		goto finish_callc;

	case 0xf5:			/* PUSH AF */
		memwrt(--SP, A);
		memwrt(--SP, F);
		t += 7;
		break;

	case 0xf6:			/* OR n */
		P = memrdr(PC++);
		t += 3;
		goto finish_or;

	case 0xf7:			/* RST 30 */
		W = 0x30;
		goto finish_call;

	case 0xf8:			/* RET M */
		res = F & S_FLAG;
		goto finish_retc;

	case 0xf9:			/* LD SP,ir */
		SP = IR;
		t += 2;
		break;

	case 0xfa:			/* JP M,nn */
		res = F & S_FLAG;
		goto finish_jpc;

	case 0xfb:			/* EI */
		IFF = 3;
		int_protection = 1;	/* protect next instruction */
		break;

	case 0xfc:			/* CALL M,nn */
		res = F & S_FLAG;
		goto finish_callc;

	case 0xfd:			/* 0xfd prefix */
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

		R++;			/* increment refresh register */

		curr_ir = IR_IY;
		IR = IY;
		goto next_opcode;

	case 0xfe:			/* CP n */
		P = memrdr(PC++);
		t += 3;
		goto finish_cp;

	case 0xff:			/* RST 38 */
		W = 0x38;
		goto finish_call;
	}

	if (curr_ir == IR_HL) {
		H = IR >> 8;
		L = IR & 0xff;
	} else if (curr_ir == IR_IX)
		IX = IR;
	else
		IY = IR;

	T += t;

#undef IR_HL
#undef IR_IX
#undef IR_IY

#undef S_SHIFT
#undef Z_SHIFT
#undef H_SHIFT
#undef P_SHIFT
#undef N_SHIFT
#undef C_SHIFT
}

#endif /* !ALTZ80_INC */
