/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	This module is a Z80 and 8080 disassembler for the Z80-CPU simulator
 */

#include <stdio.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simmem.h"
#include "simdis.h"

#ifdef WANT_ICE

/*
 *	Forward declarations
 */
static char *btoh(BYTE b, char *p);
static char *wtoa(WORD w, char *p);

#ifndef EXCLUDE_Z80

static const char *const optab_01[64] = {
	/*00*/	"NOP",		"LD\tBC,w",	"LD\t(BC),A",	"INC\tBC",
	/*04*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"RLCA",
	/*08*/	"EX\tAF,AF'",	"ADD\ti,BC",	"LD\tA,(BC)",	"DEC\tBC",
	/*0C*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"RRCA",
	/*10*/	"DJNZ\tj",	"LD\tDE,w",	"LD\t(DE),A",	"INC\tDE",
	/*14*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"RLA",
	/*18*/	"JR\tj",	"ADD\ti,DE",	"LD\tA,(DE)",	"DEC\tDE",
	/*1C*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"RRA",
	/*20*/	"JR\tNZ,j",	"LD\ti,w",	"LD\t(w),i",	"INC\ti",
	/*24*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"DAA",
	/*28*/	"JR\tZ,j",	"ADD\ti,i",	"LD\ti,(w)",	"DEC\ti",
	/*2C*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"CPL",
	/*30*/	"JR\tNC,j",	"LD\tSP,w",	"LD\t(w),A",	"INC\tSP",
	/*34*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"SCF",
	/*38*/	"JR\tC,j",	"ADD\ti,SP",	"LD\tA,(w)",	"DEC\tSP",
	/*3C*/	"INC\tr",	"DEC\tr",	"LD\tr,b",	"CCF"
};

static const char *const optab_45[8] = {
	/*80*/	"ADD\tA,r",	"ADC\tA,r",	"SUB\tr",	"SBC\tA,r",
	/*A0*/	"AND\tr",	"XOR\tr",	"OR\tr",	"CP\tr"
};

static const char *const optab_67[64] = {
	/*C0*/	"RET\tNZ",	"POP\tBC",	"JP\tNZ,w",	"JP\tw",
	/*C4*/	"CALL\tNZ,w",	"PUSH\tBC",	"ADD\tA,b",	"RST\t0H",
	/*C8*/	"RET\tZ",	"RET",		"JP\tZ,w",	"",
	/*CC*/	"CALL\tZ,w",	"CALL\tw",	"ADC\tA,b",	"RST\t8H",
	/*D0*/	"RET\tNC",	"POP\tDE",	"JP\tNC,w",	"OUT\t(b),A",
	/*D4*/	"CALL\tNC,w",	"PUSH\tDE",	"SUB\tb",	"RST\t10H",
	/*D8*/	"RET\tC",	"EXX",		"JP\tC,w",	"IN\tA,(b)",
	/*DC*/	"CALL\tC,w",	"",		"SBC\tA,b",	"RST\t18H",
	/*E0*/	"RET\tPO",	"POP\ti",	"JP\tPO,w",	"EX\t(SP),i",
	/*E4*/	"CALL\tPO,w",	"PUSH\ti",	"AND\tb",	"RST\t20H",
	/*E8*/	"RET\tPE",	"JP\t(i)",	"JP\tPE,w",	"EX\tDE,HL",
	/*EC*/	"CALL\tPE,w",	"",		"XOR\tb",	"RST\t28H",
	/*F0*/	"RET\tP",	"POP\tAF",	"JP\tP,w",	"DI",
	/*F4*/	"CALL\tP,w",	"PUSH\tAF",	"OR\tb",	"RST\t30H",
	/*F8*/	"RET\tM",	"LD\tSP,i",	"JP\tM,w",	"EI",
	/*FC*/	"CALL\tM,w",	"",		"CP\tb",	"RST\t38H"
};

static const char *const optab_ed_23[64] = {
	/*40*/	"IN\tB,(C)",	"OUT\t(C),B",	"SBC\tHL,BC",	"LD\t(w),BC",
	/*44*/	"NEG",		"RETN",		"IM\t0",	"LD\tI,A",
	/*48*/	"IN\tC,(C)",	"OUT\t(C),C",	"ADC\tHL,BC",	"LD\tBC,(w)",
	/*4C*/	"NEG*",		"RETI",		"IM*\t0",	"LD\tR,A",
	/*50*/	"IN\tD,(C)",	"OUT\t(C),D",	"SBC\tHL,DE",	"LD\t(w),DE",
	/*54*/	"NEG*",		"RETN*",	"IM\t1",	"LD\tA,I",
	/*58*/	"IN\tE,(C)",	"OUT\t(C),E",	"ADC\tHL,DE",	"LD\tDE,(w)",
	/*5C*/	"NEG*",		"RETI*",	"IM\t2",	"LD\tA,R",
	/*60*/	"IN\tH,(C)",	"OUT\t(C),H",	"SBC\tHL,HL",	"LD*\t(w),HL",
	/*64*/	"NEG*",		"RETN*",	"IM*\t0",	"RRD",
	/*68*/	"IN\tL,(C)",	"OUT\t(C),L",	"ADC\tHL,HL",	"LD*\tHL,(w)",
	/*6C*/	"NEG*",		"RETI*",	"IM*\t0",	"RLD",
	/*70*/	"IN*\tF,(C)",	"OUT*\t(C),0",	"SBC\tHL,SP",	"LD\t(w),SP",
	/*74*/	"NEG*",		"RETN*",	"IM*\t1",	"NOP*",
	/*78*/	"IN\tA,(C)",	"OUT\t(C),A",	"ADC\tHL,SP",	"LD\tSP,(w)",
	/*7C*/	"NEG*",		"RETI*",	"IM*\t2",	"NOP*"
};

static const char *const optab_ed_5[32] = {
	/*A0*/	"LDI",		"CPI",		"INI",		"OUTI",
	/*A4*/	"NOP*",		"NOP*",		"NOP*",		"NOP*",
	/*A8*/	"LDD",		"CPD",		"IND",		"OUTD",
	/*AC*/	"NOP*",		"NOP*",		"NOP*",		"NOP*",
	/*B0*/	"LDIR",		"CPIR",		"INIR",		"OTIR",
	/*B4*/	"NOP*",		"NOP*",		"NOP*",		"NOP*",
	/*B8*/	"LDDR",		"CPDR",		"INDR",		"OTDR",
	/*BC*/	"NOP*",		"NOP*",		"NOP*",		"NOP*"
};

static const char *const optab_cb_rs[8] = {
	/*00*/	"RLC\tr",	"RRC\tr",	"RL\tr",	"RR\tr",
	/*20*/	"SLA\tr",	"SRA\tr",	"SLL*\tr",	"SRL\tr"
};

static const char *const optab_cb_bit[4] = {
	/*00*/	"",		"BIT\tn,r",	"RES\tn,r",	"SET\tn,r"
};

static const char *const optab_ddfdcb_rs_undoc[8] = {
	/*00*/	"RLC*\tr,r",	"RRC*\tr,r",	"RL*\tr,r",	"RR*\tr,r",
	/*20*/	"SLA*\tr,r",	"SRA*\tr,r",	"SLL*\tr,r",	"SRL*\tr,r"
};

static const char *const optab_ddfdcb_bit_undoc[4] = {
	/*00*/	"",		"BIT*\tn,r",	"RES*\tn,r,r",	"SET*\tn,r,r"
};

static const BYTE undoc_ddfd[32] = {
	/*00*/	0xff, 0xfd, 0xff, 0xfd, 0xf1, 0xf1, 0x8f, 0xfd,
	/*40*/	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0x40, 0xbf,
	/*80*/	0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf,
	/*C0*/	0xff, 0xf7, 0xff, 0xff, 0xd5, 0xfd, 0xff, 0xfd
};

#endif /* !EXCLUDE_Z80 */

#ifndef EXCLUDE_I8080

static const char *const optab_8080_01[64] = {
	/*00*/	"NOP",		"LXI\tB,w",	"STAX\tB",	"INX\tB",
	/*04*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"RLC",
	/*08*/	"NOP*",		"DAD\tB",	"LDAX\tB",	"DCX\tB",
	/*0C*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"RRC",
	/*10*/	"NOP*",		"LXI\tD,w",	"STAX\tD",	"INX\tD",
	/*14*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"RAL",
	/*18*/	"NOP*",		"DAD\tD",	"LDAX\tD",	"DCX\tD",
	/*1C*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"RAR",
	/*20*/	"NOP*",		"LXI\tH,w",	"SHLD\tw",	"INX\tH",
	/*24*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"DAA",
	/*28*/	"NOP*",		"DAD\tH",	"LHLD\tw",	"DCX\tH",
	/*2C*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"CMA",
	/*30*/	"NOP*",		"LXI\tSP,w",	"STA\tw",	"INX\tSP",
	/*34*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"STC",
	/*38*/	"NOP*",		"DAD\tSP",	"LDA\tw",	"DCX\tSP",
	/*3C*/	"INR\tr",	"DCR\tr",	"MVI\tr,b",	"CMC"
};

static const char *const optab_8080_45[8] = {
	/*80*/	"ADD\tr",	"ADC\tr",	"SUB\tr",	"SBB\tr",
	/*A0*/	"ANA\tr",	"XRA\tr",	"ORA\tr",	"CMP\tr"
};

static const char *const optab_8080_67[64] = {
	/*C0*/	"RNZ",		"POP\tB",	"JNZ\tw",	"JMP\tw",
	/*C4*/	"CNZ\tw",	"PUSH\tB",	"ADI\tb",	"RST\t0",
	/*C8*/	"RZ",		"RET",		"JZ\tw",	"JMP*\tw",
	/*CC*/	"CZ\tw",	"CALL\tw",	"ACI\tb",	"RST\t1",
	/*D0*/	"RNC",		"POP\tD",	"JNC\tw",	"OUT\tb",
	/*D4*/	"CNC\tw",	"PUSH\tD",	"SUI\tb",	"RST\t2",
	/*D8*/	"RC",		"RET*",		"JC\tw",	"IN\tb",
	/*DC*/	"CC\tw",	"CALL*\tw",	"SBI\tb",	"RST\t3",
	/*E0*/	"RPO",		"POP\tH",	"JPO\tw",	"XTHL",
	/*E4*/	"CPO\tw",	"PUSH\tH",	"ANI\tb",	"RST\t4",
	/*E8*/	"RPE",		"PCHL",		"JPE\tw",	"XCHG",
	/*EC*/	"CPE\tw",	"CALL*\tw",	"XRI\tb",	"RST\t5",
	/*F0*/	"RP",		"POP\tPSW",	"JP\tw",	"DI",
	/*F4*/	"CP\tw",	"PUSH\tPSW",	"ORI\tb",	"RST\t6",
	/*F8*/	"RM",		"SPHL",		"JM\tw",	"EI",
	/*FC*/	"CM\tw",	"CALL*\tw",	"CPI\tb",	"RST\t7"
};

#endif /* !EXCLUDE_I8080 */

/* globals for passing disassembled code to anyone else who's interested */

char Disass_Str[64];
char Opcode_Str[64];

/*
 *	The function disass() is the only global function of
 *	this module. The argument is the address of the op-code
 *	to disassemble. The output of the disassembly goes to
 *	stdout, terminated by a newline. After the disassembly
 *	the length in bytes of the disassembled instruction
 *	is returned.
 *
 *	At most four bytes will be read from memory using
 *	getmem().
 *
 */
int disass(WORD addr)
{
	register BYTE op;
	register const char *tmpl;
	register char *p;
	BYTE b1, b2;
	WORD a = addr;
	int reg1 = 0, reg2 = 0, i, len;
#ifndef EXCLUDE_Z80
	BYTE displ = 0;
	WORD w;
	char ireg = 0;
	int undoc_ireg = 0, bit = 0;
#endif

	/*
	 * select instruction template tmpl, decode operands, and
	 * flag undocumented Z80 DD/FD main block instructions
	 */
	switch (cpu) {
#ifndef EXCLUDE_Z80
	case Z80:
		op = getmem(a++);
		/* index register prefix? */
		if ((op & 0xdf) == 0xdd) {
			ireg = 'X' + ((op >> 5) & 1);
			op = getmem(a++);
			undoc_ireg = !!(undoc_ddfd[op >> 3] & (1 << (op & 7)));
		}
		reg1 = (op >> 3) & 7;
		reg2 = op & 7;
		if (op < 0x40) {
			tmpl = optab_01[op];
			if (ireg && reg1 == 6 && (4 <= reg2 && reg2 <= 6))
				displ = getmem(a++);
		} else if (op < 0x80) {
			if (op == 0x76)
				tmpl = "HALT";
			else {
				tmpl = "LD\tr,r";
				if (ireg && (reg1 == 6 || reg2 == 6))
					displ = getmem(a++);
			}
		} else if (op < 0xc0) {
			tmpl = optab_45[reg1];
			reg1 = reg2;
			if (ireg && reg1 == 6)
				displ = getmem(a++);
		} else if (op == 0xcb) {
			if (ireg)
				displ = getmem(a++);
			op = getmem(a++);
			bit = (op >> 3) & 7;
			reg1 = op & 7;
			if (ireg && reg1 != 6) {
				/* DD/FD undocumented CB RS/BIT */
				reg2 = reg1;
				reg1 = 6;
				if (op < 0x40)
					tmpl = optab_ddfdcb_rs_undoc[op >> 3];
				else
					tmpl = optab_ddfdcb_bit_undoc[op >> 6];
			} else {
				if (op < 0x40)
					tmpl = optab_cb_rs[op >> 3];
				else
					tmpl = optab_cb_bit[op >> 6];
			}
		} else if (op == 0xed) {
			if (ireg) {
				/* DD/FD followed by ED is an undoc'd NOP */
				tmpl = "NOP";
				a--;
			} else {
				op = getmem(a++);
				if (0x40 <= op && op < 0x80)
					tmpl = optab_ed_23[op & 0x3f];
				else if (0xa0 <= op && op < 0xc0)
					tmpl = optab_ed_5[op & 0x1f];
				else
					tmpl = "NOP*";
			}
		} else if (ireg && (op & 0xdf) == 0xdd) {
			/* DD/FD followed by DD/FD is an undocumented NOP */
			tmpl = "NOP";
			a--;
		} else
			tmpl = optab_67[op & 0x3f];
		break;
#endif
#ifndef EXCLUDE_I8080
	case I8080:
		op = getmem(a++);
		reg1 = (op >> 3) & 7;
		reg2 = op & 7;
		if (op < 0x40) {
			tmpl = optab_8080_01[op];
		} else if (op < 0x80) {
			if (op == 0x76)
				tmpl = "HLT";
			else
				tmpl = "MOV\tr,r";
		} else if (op < 0xc0) {
			tmpl = optab_8080_45[reg1];
			reg1 = reg2;
		} else
			tmpl = optab_8080_67[op & 0x3f];
		break;
#endif
	default:
		tmpl = "";
		break;
	}

	/*
	 * expand instruction template tmpl into disassembly string
	 * uses ireg, reg1, reg2, bit, displ, and undoc_ireg, which
	 * where setup in the previous step
	 */
	for (p = Disass_Str; *tmpl; tmpl++) {
		switch (*tmpl) {
		case 'b':	/* byte */
			b1 = getmem(a++);
			p = wtoa(b1, p);
			break;
		case 'w':	/* word */
			b1 = getmem(a++);
			b2 = getmem(a++);
			p = wtoa((b2 << 8) | b1, p);
			break;
		case 'r':	/* register */
			switch (cpu) {
#ifndef EXCLUDE_Z80
			case Z80:
				switch (reg1) {
				case 4:	/* H */
				case 5:	/* L */
					if (undoc_ireg) {
						*p++ = 'I';
						*p++ = ireg;
					}
					/* fall through */
				case 0:	/* B */
				case 1:	/* C */
				case 2:	/* D */
				case 3:	/* E */
				case 7:	/* A */
					*p++ = "BCDEHLMA"[reg1];
					break;
				case 6:	/* (HL) */
					*p++ = '(';
					if (ireg) {
						*p++ = 'I';
						*p++ = ireg;
						if (displ) {
							if (displ < 128)
								*p++ = '+';
							else {
								*p++ = '-';
								displ = -displ;
							}
							p = wtoa(displ, p);
						}
					} else {
						*p++ = 'H';
						*p++ = 'L';
					}
					*p++ = ')';
					break;
				default:
					break;
				}
				break;
#endif
#ifndef EXCLUDE_I8080
			case I8080:
				*p++ = "BCDEHLMA"[reg1];
				break;
#endif
			default:
				break;
			}
			reg1 = reg2;
			break;
#ifndef EXCLUDE_Z80
		case 'i':	/* index register */
			if (ireg) {
				*p++ = 'I';
				*p++ = ireg;
			} else {
				*p++ = 'H';
				*p++ = 'L';
			}
			break;
		case 'j':	/* relative jump address */
			b1 = getmem(a++);
			w = a + (SBYTE) b1;
			p = wtoa(w, p);
			break;
		case 'n':	/* bit number */
			*p++ = '0' + bit;
			break;
		case '\t':
			if (undoc_ireg) {
				undoc_ireg++;
				*p++ = '*';
			}
#endif
			/* fall through */ /* should really be inside #if */
		default:
			*p++ = *tmpl;
			break;
		}
	}
#ifndef EXCLUDE_Z80
	if (undoc_ireg == 1)
		*p++ = '*';
#endif
	*p++ = '\n';
	*p = '\0';
	len = a - addr;

	/* fill opcodes string */
	p = Opcode_Str;
	for (i = 0; i < 4; i++) {
		if (i)
			*p++ = ' ';
		if (i < len) {
			b1 = getmem(addr + i);
			p = btoh(b1, p);
		} else {
			*p++ = ' ';
			*p++ = ' ';
		}
	}
	*p = '\0';

#ifndef WANT_GUI
	fputs(Opcode_Str, stdout);
	putchar('\t');
	fputs(Disass_Str, stdout);
#endif

	return len;
}

/*
 *	convert BYTE into ASCII hex and copy to string at p
 *	returns p increased by 2
 */
static char *btoh(BYTE b, char *p)
{
	register char c;

	c = b >> 4;
	*p++ = c + (c < 10 ? '0' : '7');
	c = b & 0xf;
	*p++ = c + (c < 10 ? '0' : '7');
	return p;
}

/*
 *	convert WORD into assembler hex and copy to string at p
 *	returns p increased by number of characters produced
 */
static char *wtoa(WORD w, char *p)
{
	register char c;
	register int onlyz, shift;

	onlyz = 1;
	for (shift = 12; shift >= 0; shift -= 4) {
		c = (w >> shift) & 0xf;
		if (onlyz && c > 9)
			*p++ = '0';
		if (!onlyz || c) {
			*p++ = c + (c < 10 ? '0' : '7');
			onlyz = 0;
		}
	}
	if (onlyz)
		*p++ = '0';
	*p++ = 'H';
	return p;
}

#endif /* WANT_ICE */
