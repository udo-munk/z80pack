/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (c) 2022 Thomas Eberhardt
 *
 *	History:
 *	17-SEP-1987 Development under Digital Research CP/M 2.2
 *	28-JUN-1988 Switched to Unix System V.3
 *	21-OCT-2006 changed to ANSI C for modern POSIX OS's
 *	03-FEB-2007 more ANSI C conformance and reduced compiler warnings
 *	18-MAR-2007 use default output file extension dependent on format
 *	04-OCT-2008 fixed comment bug, ';' string argument now working
 *	22-FEB-2014 fixed is...() compiler warnings
 *	13-JAN-2016 fixed buffer overflow, new expression parser from Didier
 *	02-OCT-2017 bug fixes in expression parser from Didier
 *	28-OCT-2017 added variable symbol lenght and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 */

/*
 *	opcode tables
 */

#include <stdio.h>
#include "z80a.h"

extern int op_1b(int, int), op_2b(int, int), op_pupo(int, int);
extern int op_ex(int, int), op_ld(int, int);
extern int op_call(int, int), op_ret(int, int), op_jp(int, int);
extern int op_jr(int, int), op_djnz(int, int), op_rst(int, int);
extern int op_add(int, int), op_adc(int, int), op_sub(int, int);
extern int op_sbc(int, int), op_cp(int, int), op_decinc(int, int);
extern int op_or(int, int), op_xor(int, int), op_and(int, int);
extern int op_rotshf(int, int);
extern int op_out(int, int), op_in(int, int), op_im(int, int);
extern int op_trsbit(int, int);
extern int op_pers(int, int), op_org(int, int), op_dl(int, int);
extern int op_equ(int, int);
extern int op_ds(int, int), op_db(int, int), op_dw(int, int), op_dm(int, int);
extern int op_misc(int, int);
extern int op_cond(int, int);
extern int op_glob(int, int);
extern int op8080_mov(int, int), op8080_alu(int, int), op8080_decinc(int, int);
extern int op8080_reg16(int, int), op8080_regbd(int, int);
extern int op8080_imm(int, int), op8080_rst(int, int), op8080_pupo(int, int);
extern int op8080_addr(int, int), op8080_mvi(int, int), op8080_lxi(int, int);

/*
 *	Z80 opcode table:
 *	includes entries for all opcodes and pseudo ops other than END
 *	must be sorted in ascending order!
 */
struct opc opctab_z80[] = {
	{ ".8080",	op_pers,	PERS8080, 0	},
	{ ".Z80",	op_pers,	PERSZ80, 0	},
	{ "ADC",	op_adc,		0,	0	},
	{ "ADD",	op_add,		0,	0	},
	{ "AND",	op_and,		0,	0	},
	{ "BIT",	op_trsbit,	0x40,	0	},
	{ "CALL",	op_call,	0,	0	},
	{ "CCF",	op_1b,		0x3f,	0	},
	{ "CP",		op_cp,		0,	0	},
	{ "CPD",	op_2b,		0xed,	0xa9	},
	{ "CPDR",	op_2b,		0xed,	0xb9	},
	{ "CPI",	op_2b,		0xed,	0xa1	},
	{ "CPIR",	op_2b,		0xed,	0xb1	},
	{ "CPL",	op_1b,		0x2f,	0	},
	{ "DAA",	op_1b,		0x27,	0	},
	{ "DEC",	op_decinc,	0x05,	0x0b	},
	{ "DEFB",	op_db,		0,	0	},
	{ "DEFL",	op_dl,		0,	0	},
	{ "DEFM",	op_dm,		0,	0	},
	{ "DEFS",	op_ds,		0,	0	},
	{ "DEFW",	op_dw,		0,	0	},
	{ "DI",		op_1b,		0xf3,	0	},
	{ "DJNZ",	op_djnz,	0,	0	},
	{ "EI",		op_1b,		0xfb,	0	},
	{ "EJECT",	op_misc,	1,	0	},
	{ "ELSE",	op_cond,	98,	0	},
	{ "ENDIF",	op_cond,	99,	0	},
	{ "EQU",	op_equ,		0,	0	},
	{ "EX",		op_ex,		0,	0	},
	{ "EXTRN",	op_glob,	1,	0	},
	{ "EXX",	op_1b,		0xd9,	0	},
	{ "HALT",	op_1b,		0x76,	0	},
	{ "IFDEF",	op_cond,	1,	0	},
	{ "IFEQ",	op_cond,	3,	0	},
	{ "IFNDEF",	op_cond,	2,	0	},
	{ "IFNEQ",	op_cond,	4,	0	},
	{ "IM",		op_im,		0,	0	},
	{ "IN",		op_in,		0,	0	},
	{ "INC",	op_decinc,	0x04,	0x03	},
	{ "INCLUDE",	op_misc,	6,	0	},
	{ "IND",	op_2b,		0xed,	0xaa	},
	{ "INDR",	op_2b,		0xed,	0xba	},
	{ "INI",	op_2b,		0xed,	0xa2	},
	{ "INIR",	op_2b,		0xed,	0xb2	},
	{ "JP",		op_jp,		0,	0	},
	{ "JR",		op_jr,		0,	0	},
	{ "LD",		op_ld,		0,	0	},
	{ "LDD",	op_2b,		0xed,	0xa8	},
	{ "LDDR",	op_2b,		0xed,	0xb8	},
	{ "LDI",	op_2b,		0xed,	0xa0	},
	{ "LDIR",	op_2b,		0xed,	0xb0	},
	{ "LIST",	op_misc,	2,	0	},
	{ "NEG",	op_2b,		0xed,	0x44	},
	{ "NOLIST",	op_misc,	3,	0	},
	{ "NOP",	op_1b,		0,	0	},
	{ "OR",		op_or,		0,	0	},
	{ "ORG",	op_org,		0,	0	},
	{ "OTDR",	op_2b,		0xed,	0xbb	},
	{ "OTIR",	op_2b,		0xed,	0xb3	},
	{ "OUT",	op_out,		0,	0	},
	{ "OUTD",	op_2b,		0xed,	0xab	},
	{ "OUTI",	op_2b,		0xed,	0xa3	},
	{ "PAGE",	op_misc,	4,	0	},
	{ "POP",	op_pupo,	0xc1,	0	},
	{ "PRINT",	op_misc,	5,	0	},
	{ "PUBLIC",	op_glob,	2,	0	},
	{ "PUSH",	op_pupo,	0xc5,	0	},
	{ "RES",	op_trsbit,	0x80,	0	},
	{ "RET",	op_ret,		0,	0	},
	{ "RETI",	op_2b,		0xed,	0x4d	},
	{ "RETN",	op_2b,		0xed,	0x45	},
	{ "RL",		op_rotshf,	0x10,	0	},
	{ "RLA",	op_1b,		0x17,	0	},
	{ "RLC",	op_rotshf,	0x00,	0	},
	{ "RLCA",	op_1b,		0x07,	0	},
	{ "RLD",	op_2b,		0xed,	0x6f	},
	{ "RR",		op_rotshf,	0x18,	0	},
	{ "RRA",	op_1b,		0x1f,	0	},
	{ "RRC",	op_rotshf,	0x08,	0	},
	{ "RRCA",	op_1b,		0x0f,	0	},
	{ "RRD",	op_2b,		0xed,	0x67	},
	{ "RST",	op_rst,		0,	0	},
	{ "SBC",	op_sbc,		0,	0	},
	{ "SCF",	op_1b,		0x37,	0	},
	{ "SET",	op_trsbit,	0xc0,	0	},
	{ "SLA",	op_rotshf,	0x20,	0	},
	{ "SLL",	op_rotshf,	0x30,	0	},
	{ "SRA",	op_rotshf,	0x28,	0	},
	{ "SRL",	op_rotshf,	0x38,	0	},
	{ "SUB",	op_sub,		0,	0	},
	{ "TITLE",	op_misc,	7,	0	},
	{ "XOR",	op_xor,		0,	0	}
};

/*
 *	table with reserved Z80 operand words: registers and flags
 *	must be sorted in ascending order!
 */
struct ope opetab_z80[] = {
	{ "(BC)",	REGIBC },
	{ "(DE)",	REGIDE },
	{ "(HL)",	REGIHL },
	{ "(IX)",	REGIIX },
	{ "(IY)",	REGIIY },
	{ "(SP)",	REGISP },
	{ "A",		REGA   },
	{ "AF",		REGAF  },
	{ "B",		REGB   },
	{ "BC",		REGBC  },
	{ "C",		REGC   },
	{ "D",		REGD   },
	{ "DE",		REGDE  },
	{ "E",		REGE   },
	{ "H",		REGH   },
	{ "HL",		REGHL  },
	{ "I",		REGI   },
	{ "IX",		REGIX  },
	{ "IXH",	REGIXH },
	{ "IXL",	REGIXL },
	{ "IY",		REGIY  },
	{ "IYH",	REGIYH },
	{ "IYL",	REGIYL },
	{ "L",		REGL   },
	{ "M",		FLGM   },
	{ "NC",		FLGNC  },
	{ "NZ",		FLGNZ  },
	{ "P",		FLGP   },
	{ "PE",		FLGPE  },
	{ "PO",		FLGPO  },
	{ "R",		REGR   },
	{ "SP",		REGSP  },
	{ "Z",		FLGZ   }
};

/*
 *	8080 opcode table:
 *	includes entries for all opcodes and pseudo ops other than END
 *	must be sorted in ascending order!
 */
struct opc opctab_8080[] = {
	{ ".8080",	op_pers,	PERS8080, 0	},
	{ ".Z80",	op_pers,	PERSZ80, 0	},
	{ "ACI",	op8080_imm,	0xce,	0	},
	{ "ADC",	op8080_alu,	0x88,	0	},
	{ "ADD",	op8080_alu,	0x80,	0	},
	{ "ADI",	op8080_imm,	0xc6,	0	},
	{ "ANA",	op8080_alu,	0xa0,	0	},
	{ "ANI",	op8080_imm,	0xe6,	0	},
	{ "CALL",	op8080_addr,	0xcd,	0	},
	{ "CC",		op8080_addr,	0xdc,	0	},
	{ "CM",		op8080_addr,	0xfc,	0	},
	{ "CMA",	op_1b,		0x2f,	0	},
	{ "CMC",	op_1b,		0x3f,	0	},
	{ "CMP",	op8080_alu,	0xb8,	0	},
	{ "CNC",	op8080_addr,	0xd4,	0	},
	{ "CNZ",	op8080_addr,	0xc4,	0	},
	{ "CP",		op8080_addr,	0xf4,	0	},
	{ "CPE",	op8080_addr,	0xec,	0	},
	{ "CPI",	op8080_imm,	0xfe,	0	},
	{ "CPO",	op8080_addr,	0xe4,	0	},
	{ "CZ",		op8080_addr,	0xcc,	0	},
	{ "DAA",	op_1b,		0x27,	0	},
	{ "DAD",	op8080_reg16,	0x09,	0	},
	{ "DCR",	op8080_decinc,	0x05,	0	},
	{ "DCX",	op8080_reg16,	0x0b,	0	},
	{ "DEFB",	op_db,		0,	0	},
	{ "DEFL",	op_dl,		0,	0	},
	{ "DEFM",	op_dm,		0,	0	},
	{ "DEFS",	op_ds,		0,	0	},
	{ "DEFW",	op_dw,		0,	0	},
	{ "DI",		op_1b,		0xf3,	0	},
	{ "EI",		op_1b,		0xfb,	0	},
	{ "EJECT",	op_misc,	1,	0	},
	{ "ELSE",	op_cond,	98,	0	},
	{ "ENDIF",	op_cond,	99,	0	},
	{ "EQU",	op_equ,		0,	0	},
	{ "EXTRN",	op_glob,	1,	0	},
	{ "HLT",	op_1b,		0x76,	0	},
	{ "IFDEF",	op_cond,	1,	0	},
	{ "IFEQ",	op_cond,	3,	0	},
	{ "IFNDEF",	op_cond,	2,	0	},
	{ "IFNEQ",	op_cond,	4,	0	},
	{ "IN",		op8080_imm,	0xdb,	0	},
	{ "INCLUDE",	op_misc,	6,	0	},
	{ "INR",	op8080_decinc,	0x04,	0	},
	{ "INX",	op8080_reg16,	0x03,	0	},
	{ "JC",		op8080_addr,	0xda,	0	},
	{ "JM",		op8080_addr,	0xfa,	0	},
	{ "JMP",	op8080_addr,	0xc3,	0	},
	{ "JNC",	op8080_addr,	0xd2,	0	},
	{ "JNZ",	op8080_addr,	0xc2,	0	},
	{ "JP",		op8080_addr,	0xf2,	0	},
	{ "JPE",	op8080_addr,	0xea,	0	},
	{ "JPO",	op8080_addr,	0xe2,	0	},
	{ "JZ",		op8080_addr,	0xca,	0	},
	{ "LDA",	op8080_addr,	0x3a,	0	},
	{ "LDAX",	op8080_regbd,	0x0a,	0	},
	{ "LHLD",	op8080_addr,	0x2a,	0	},
	{ "LIST",	op_misc,	2,	0	},
	{ "LXI",	op8080_lxi,	0,	0	},
	{ "MOV",	op8080_mov,	0,	0	},
	{ "MVI",	op8080_mvi,	0,	0	},
	{ "NOLIST",	op_misc,	3,	0	},
	{ "NOP",	op_1b,		0,	0	},
	{ "ORA",	op8080_alu,	0xb0,	0	},
	{ "ORG",	op_org,		0,	0	},
	{ "ORI",	op8080_imm,	0xf6,	0	},
	{ "OUT",	op8080_imm,	0xd3,	0	},
	{ "PAGE",	op_misc,	4,	0	},
	{ "PCHL",	op_1b,		0xe9,	0	},
	{ "POP",	op8080_pupo,	0xc1,	0	},
	{ "PRINT",	op_misc,	5,	0	},
	{ "PUBLIC",	op_glob,	2,	0	},
	{ "PUSH",	op8080_pupo,	0xc5,	0	},
	{ "RAL",	op_1b,		0x17,	0	},
	{ "RAR",	op_1b,		0x1f,	0	},
	{ "RC",		op_1b,		0xd8,	0	},
	{ "RET",	op_1b,		0xc9,	0	},
	{ "RLC",	op_1b,		0x07,	0	},
	{ "RM",		op_1b,		0xf8,	0	},
	{ "RNC",	op_1b,		0xd0,	0	},
	{ "RNZ",	op_1b,		0xc0,	0	},
	{ "RP",		op_1b,		0xf0,	0	},
	{ "RPE",	op_1b,		0xe8,	0	},
	{ "RPO",	op_1b,		0xe0,	0	},
	{ "RRC",	op_1b,		0x0f,	0	},
	{ "RST",	op8080_rst,	0,	0	},
	{ "RZ",		op_1b,		0xc8,	0	},
	{ "SBB",	op8080_alu,	0x98,	0	},
	{ "SBI",	op8080_imm,	0xde,	0	},
	{ "SHLD",	op8080_addr,	0x22,	0	},
	{ "SPHL",	op_1b,		0xf9,	0	},
	{ "STA",	op8080_addr,	0x32,	0	},
	{ "STAX",	op8080_regbd,	0x02,	0	},
	{ "STC",	op_1b,		0x37,	0	},
	{ "SUB",	op8080_alu,	0x90,	0	},
	{ "SUI",	op8080_imm,	0xd6,	0	},
	{ "TITLE",	op_misc,	7,	0	},
	{ "XCHG",	op_1b,		0xeb,	0	},
	{ "XRA",	op8080_alu,	0xa8,	0	},
	{ "XRI",	op8080_imm,	0xee,	0	},
	{ "XTHL",	op_1b,		0xe3,	0	},
};

/*
 *	table with reserved 8080 operand words: registers and flags
 *	must be sorted in ascending order!
 */
struct ope opetab_8080[] = {
	{ "A",		REGA   },
	{ "B",		REGB   },
	{ "C",		REGC   },
	{ "D",		REGD   },
	{ "E",		REGE   },
	{ "H",		REGH   },
	{ "L",		REGL   },
	{ "M",		REGM   },
	{ "PSW",	REGPSW },
	{ "SP",		REGSP  }
};

/*
 *	table of personalities
 */
struct pers perstab[NUMPERS] = {
	/* PERSZ80 */
	{
		sizeof(opctab_z80) / sizeof(struct opc),
		opctab_z80,
		sizeof(opetab_z80) / sizeof(struct ope),
		opetab_z80
	},
	/* PERS8080 */
	{
		sizeof(opctab_8080) / sizeof(struct opc),
		opctab_8080,
		sizeof(opetab_8080) / sizeof(struct ope),
		opetab_8080
	}
};
