/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022 by Thomas Eberhardt
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
 *	04-OCT-2022 new expression parser (TE)
 */

/*
 *	opcode tables
 */

#include <stdio.h>
#include "z80a.h"

/* z80apfun.c */
extern int op_opset(int, int), op_org(int, int), op_phase(int, int);
extern int op_dephase(int, int), op_radix(int, int), op_equ(int, int);
extern int op_dl(int, int), op_ds(int, int), op_db(int, int), op_dm(int, int);
extern int op_dw(int, int), op_misc(int, int), op_cond(int, int);
extern int op_glob(int, int), op_end(int, int);

/* z80arfun.c */
extern int op_1b(int, int), op_2b(int, int), op_im(int, int);
extern int op_pupo(int, int), op_ex(int, int), op_rst(int, int);
extern int op_ret(int, int), op_jpcall(int, int), op_jr(int, int);
extern int op_djnz(int, int), op_ld(int, int), op_add(int, int);
extern int op_sbadc(int, int), op_decinc(int, int), op_alu(int, int);
extern int op_out(int, int), op_in(int, int), op_cbgrp(int, int);
extern int op8080_mov(int, int), op8080_alu(int, int), op8080_decinc(int, int);
extern int op8080_reg16(int, int), op8080_regbd(int, int);
extern int op8080_imm(int, int), op8080_rst(int, int), op8080_pupo(int, int);
extern int op8080_addr(int, int), op8080_mvi(int, int), op8080_lxi(int, int);

/*
 *	pseudo op table:
 *	includes entries for all common pseudo ops
 *	must be sorted in ascending order!
 */
struct opc opctab_psd[] = {
	{ ".8080",	op_opset,	1,	0,	0	 },
	{ ".DEPHASE",	op_dephase,	0,	0,	0	 },
	{ ".PHASE",	op_phase,	0,	0,	0	 },
	{ ".RADIX",	op_radix,	0,	0,	0	 },
	{ ".Z80",	op_opset,	2,	0,	0	 },
	{ "ASET",	op_dl,		0,	0,	OP_SET	 },
	{ "COND",	op_cond,	5,	0,	OP_COND	 },
	{ "DB",		op_db,		0,	0,	0	 },
	{ "DC",		op_dm,		2,	0,	0	 },
	{ "DEFB",	op_db,		0,	0,	0	 },
	{ "DEFC",	op_dm,		2,	0,	0	 },
	{ "DEFL",	op_dl,		0,	0,	OP_SET	 },
	{ "DEFM",	op_dm,		1,	0,	0	 },
	{ "DEFS",	op_ds,		0,	0,	0	 },
	{ "DEFW",	op_dw,		0,	0,	0	 },
	{ "DEFZ",	op_dm,		3,	0,	0	 },
	{ "DS",		op_ds,		0,	0,	0	 },
	{ "DW",		op_dw,		0,	0,	0	 },
	{ "EJECT",	op_misc,	1,	0,	0	 },
	{ "ELSE",	op_cond,	98,	0,	OP_COND	 },
	{ "END",	op_end,		0,	0,	OP_END	 },
	{ "ENDC",	op_cond,	99,	0,	OP_COND	 },
	{ "ENDIF",	op_cond,	99,	0,	OP_COND	 },
	{ "ENT",	op_glob,	2,	0,	0	 },
	{ "ENTRY",	op_glob,	2,	0,	0	 },
	{ "EQU",	op_equ,		0,	0,	OP_SET	 },
	{ "EXT",	op_glob,	1,	0,	0	 },
	{ "EXTERNAL",	op_glob,	1,	0,	0	 },
	{ "EXTRN",	op_glob,	1,	0,	0	 },
	{ "GLOBAL",	op_glob,	2,	0,	0	 },
	{ "IF",		op_cond,	5,	0,	OP_COND	 },
	{ "IFDEF",	op_cond,	1,	0,	OP_COND	 },
	{ "IFE",	op_cond,	6,	0,	OP_COND	 },
	{ "IFEQ",	op_cond,	3,	0,	OP_COND	 },
	{ "IFF",	op_cond,	6,	0,	OP_COND	 },
	{ "IFNDEF",	op_cond,	2,	0,	OP_COND	 },
	{ "IFNEQ",	op_cond,	4,	0,	OP_COND	 },
	{ "IFT",	op_cond,	5,	0,	OP_COND	 },
	{ "INCLUDE",	op_misc,	6,	0,	0	 },
	{ "LIST",	op_misc,	2,	0,	0	 },
	{ "NOLIST",	op_misc,	3,	0,	0	 },
	{ "ORG",	op_org,		0,	0,	0	 },
	{ "PAGE",	op_misc,	4,	0,	0	 },
	{ "PRINT",	op_misc,	5,	0,	0	 },
	{ "PUBLIC",	op_glob,	2,	0,	0	 },
	{ "TITLE",	op_misc,	7,	0,	0	 }
};

/*
 *	Z80 opcode table:
 *	includes entries for all Z80 opcodes
 *	must be sorted in ascending order!
 */
struct opc opctab_z80[] = {
	{ "ADC",	op_sbadc,	0x88,	0x4a,	0	 },
	{ "ADD",	op_add,		0,	0,	0	 },
	{ "AND",	op_alu,		0xa0,	0,	0	 },
	{ "BIT",	op_cbgrp,	0x40,	0,	0	 },
	{ "CALL",	op_jpcall,	0xc4,	0xcd,	0	 },
	{ "CCF",	op_1b,		0x3f,	0,	0	 },
	{ "CP",		op_alu,		0xb8,	0,	0	 },
	{ "CPD",	op_2b,		0xed,	0xa9,	0	 },
	{ "CPDR",	op_2b,		0xed,	0xb9,	0	 },
	{ "CPI",	op_2b,		0xed,	0xa1,	0	 },
	{ "CPIR",	op_2b,		0xed,	0xb1,	0	 },
	{ "CPL",	op_1b,		0x2f,	0,	0	 },
	{ "DAA",	op_1b,		0x27,	0,	0	 },
	{ "DEC",	op_decinc,	0x05,	0x0b,	0	 },
	{ "DI",		op_1b,		0xf3,	0,	0	 },
	{ "DJNZ",	op_djnz,	0,	0,	0	 },
	{ "EI",		op_1b,		0xfb,	0,	0	 },
	{ "EX",		op_ex,		0,	0,	0	 },
	{ "EXX",	op_1b,		0xd9,	0,	0	 },
	{ "HALT",	op_1b,		0x76,	0,	0	 },
	{ "IM",		op_im,		0,	0,	0	 },
	{ "IN",		op_in,		0,	0,	0	 },
	{ "INC",	op_decinc,	0x04,	0x03,	0	 },
	{ "IND",	op_2b,		0xed,	0xaa,	0	 },
	{ "INDR",	op_2b,		0xed,	0xba,	0	 },
	{ "INI",	op_2b,		0xed,	0xa2,	0	 },
	{ "INIR",	op_2b,		0xed,	0xb2,	0	 },
	{ "JP",		op_jpcall,	0xc2,	0xc3,	0	 },
	{ "JR",		op_jr,		0,	0,	0	 },
	{ "LD",		op_ld,		0,	0,	0	 },
	{ "LDD",	op_2b,		0xed,	0xa8,	0	 },
	{ "LDDR",	op_2b,		0xed,	0xb8,	0	 },
	{ "LDI",	op_2b,		0xed,	0xa0,	0	 },
	{ "LDIR",	op_2b,		0xed,	0xb0,	0	 },
	{ "NEG",	op_2b,		0xed,	0x44,	0	 },
	{ "NOP",	op_1b,		0,	0,	0	 },
	{ "OR",		op_alu,		0xb0,	0,	0	 },
	{ "OTDR",	op_2b,		0xed,	0xbb,	0	 },
	{ "OTIR",	op_2b,		0xed,	0xb3,	0	 },
	{ "OUT",	op_out,		0,	0,	0	 },
	{ "OUTD",	op_2b,		0xed,	0xab,	0	 },
	{ "OUTI",	op_2b,		0xed,	0xa3,	0	 },
	{ "POP",	op_pupo,	0xc1,	0,	0	 },
	{ "PUSH",	op_pupo,	0xc5,	0,	0	 },
	{ "RES",	op_cbgrp,	0x80,	0,	0	 },
	{ "RET",	op_ret,		0,	0,	0	 },
	{ "RETI",	op_2b,		0xed,	0x4d,	0	 },
	{ "RETN",	op_2b,		0xed,	0x45,	0	 },
	{ "RL",		op_cbgrp,	0x10,	0,	0	 },
	{ "RLA",	op_1b,		0x17,	0,	0	 },
	{ "RLC",	op_cbgrp,	0x00,	0,	0	 },
	{ "RLCA",	op_1b,		0x07,	0,	0	 },
	{ "RLD",	op_2b,		0xed,	0x6f,	0	 },
	{ "RR",		op_cbgrp,	0x18,	0,	0	 },
	{ "RRA",	op_1b,		0x1f,	0,	0	 },
	{ "RRC",	op_cbgrp,	0x08,	0,	0	 },
	{ "RRCA",	op_1b,		0x0f,	0,	0	 },
	{ "RRD",	op_2b,		0xed,	0x67,	0	 },
	{ "RST",	op_rst,		0,	0,	0	 },
	{ "SBC",	op_sbadc,	0x98,	0x42,	0	 },
	{ "SCF",	op_1b,		0x37,	0,	0	 },
	{ "SET",	op_cbgrp,	0xc0,	0,	0	 },
	{ "SLA",	op_cbgrp,	0x20,	0,	0	 },
	{ "SLL",	op_cbgrp,	0x30,	0,	OP_UNDOC },
	{ "SRA",	op_cbgrp,	0x28,	0,	0	 },
	{ "SRL",	op_cbgrp,	0x38,	0,	0	 },
	{ "SUB",	op_alu,		0x90,	0,	0	 },
	{ "XOR",	op_alu,		0xa8,	0,	0	 }
};

/*
 *	table with reserved Z80 operand words: registers and flags
 *	must be sorted in ascending order!
 */
struct ope opetab_z80[] = {
	{ "(BC)",	REGIBC,	0	  },
	{ "(DE)",	REGIDE,	0	  },
	{ "(HL)",	REGIHL,	0	  },
	{ "(IX)",	REGIIX,	0	  },
	{ "(IY)",	REGIIY,	0	  },
	{ "(SP)",	REGISP,	0	  },
	{ "A",		REGA,	0	  },
	{ "AF",		REGAF,	0	  },
	{ "B",		REGB,	0	  },
	{ "BC",		REGBC,	0	  },
	{ "C",		REGC,	0	  },
	{ "D",		REGD,	0	  },
	{ "DE",		REGDE,	0	  },
	{ "E",		REGE,	0	  },
	{ "H",		REGH,	0	  },
	{ "HL",		REGHL,	0	  },
	{ "I",		REGI,	0	  },
	{ "IX",		REGIX,	0	  },
	{ "IXH",	REGIXH,	OPE_UNDOC },
	{ "IXL",	REGIXL,	OPE_UNDOC },
	{ "IY",		REGIY,	0	  },
	{ "IYH",	REGIYH,	OPE_UNDOC },
	{ "IYL",	REGIYL,	OPE_UNDOC },
	{ "L",		REGL,	0	  },
	{ "M",		FLGM,	0	  },
	{ "NC",		FLGNC,	0	  },
	{ "NZ",		FLGNZ,	0	  },
	{ "P",		FLGP,	0	  },
	{ "PE",		FLGPE,	0	  },
	{ "PO",		FLGPO,	0	  },
	{ "R",		REGR,	0	  },
	{ "SP",		REGSP,	0	  },
	{ "Z",		FLGZ,	0	  }
};

/*
 *	8080 opcode table:
 *	includes entries for all 8080 opcodes and 8080 specific pseudo ops
 *	must be sorted in ascending order!
 */
struct opc opctab_8080[] = {
	{ "ACI",	op8080_imm,	0xce,	0,	0	 },
	{ "ADC",	op8080_alu,	0x88,	0,	0	 },
	{ "ADD",	op8080_alu,	0x80,	0,	0	 },
	{ "ADI",	op8080_imm,	0xc6,	0,	0	 },
	{ "ANA",	op8080_alu,	0xa0,	0,	0	 },
	{ "ANI",	op8080_imm,	0xe6,	0,	0	 },
	{ "CALL",	op8080_addr,	0xcd,	0,	0	 },
	{ "CC",		op8080_addr,	0xdc,	0,	0	 },
	{ "CM",		op8080_addr,	0xfc,	0,	0	 },
	{ "CMA",	op_1b,		0x2f,	0,	0	 },
	{ "CMC",	op_1b,		0x3f,	0,	0	 },
	{ "CMP",	op8080_alu,	0xb8,	0,	0	 },
	{ "CNC",	op8080_addr,	0xd4,	0,	0	 },
	{ "CNZ",	op8080_addr,	0xc4,	0,	0	 },
	{ "CP",		op8080_addr,	0xf4,	0,	0	 },
	{ "CPE",	op8080_addr,	0xec,	0,	0	 },
	{ "CPI",	op8080_imm,	0xfe,	0,	0	 },
	{ "CPO",	op8080_addr,	0xe4,	0,	0	 },
	{ "CZ",		op8080_addr,	0xcc,	0,	0	 },
	{ "DAA",	op_1b,		0x27,	0,	0	 },
	{ "DAD",	op8080_reg16,	0x09,	0,	0	 },
	{ "DCR",	op8080_decinc,	0x05,	0,	0	 },
	{ "DCX",	op8080_reg16,	0x0b,	0,	0	 },
	{ "DI",		op_1b,		0xf3,	0,	0	 },
	{ "EI",		op_1b,		0xfb,	0,	0	 },
	{ "HLT",	op_1b,		0x76,	0,	0	 },
	{ "IN",		op8080_imm,	0xdb,	0,	0	 },
	{ "INR",	op8080_decinc,	0x04,	0,	0	 },
	{ "INX",	op8080_reg16,	0x03,	0,	0	 },
	{ "JC",		op8080_addr,	0xda,	0,	0	 },
	{ "JM",		op8080_addr,	0xfa,	0,	0	 },
	{ "JMP",	op8080_addr,	0xc3,	0,	0	 },
	{ "JNC",	op8080_addr,	0xd2,	0,	0	 },
	{ "JNZ",	op8080_addr,	0xc2,	0,	0	 },
	{ "JP",		op8080_addr,	0xf2,	0,	0	 },
	{ "JPE",	op8080_addr,	0xea,	0,	0	 },
	{ "JPO",	op8080_addr,	0xe2,	0,	0	 },
	{ "JZ",		op8080_addr,	0xca,	0,	0	 },
	{ "LDA",	op8080_addr,	0x3a,	0,	0	 },
	{ "LDAX",	op8080_regbd,	0x0a,	0,	0	 },
	{ "LHLD",	op8080_addr,	0x2a,	0,	0	 },
	{ "LXI",	op8080_lxi,	0,	0,	0	 },
	{ "MOV",	op8080_mov,	0,	0,	0	 },
	{ "MVI",	op8080_mvi,	0,	0,	0	 },
	{ "NOP",	op_1b,		0,	0,	0	 },
	{ "ORA",	op8080_alu,	0xb0,	0,	0	 },
	{ "ORI",	op8080_imm,	0xf6,	0,	0	 },
	{ "OUT",	op8080_imm,	0xd3,	0,	0	 },
	{ "PCHL",	op_1b,		0xe9,	0,	0	 },
	{ "POP",	op8080_pupo,	0xc1,	0,	0	 },
	{ "PUSH",	op8080_pupo,	0xc5,	0,	0	 },
	{ "RAL",	op_1b,		0x17,	0,	0	 },
	{ "RAR",	op_1b,		0x1f,	0,	0	 },
	{ "RC",		op_1b,		0xd8,	0,	0	 },
	{ "RET",	op_1b,		0xc9,	0,	0	 },
	{ "RLC",	op_1b,		0x07,	0,	0	 },
	{ "RM",		op_1b,		0xf8,	0,	0	 },
	{ "RNC",	op_1b,		0xd0,	0,	0	 },
	{ "RNZ",	op_1b,		0xc0,	0,	0	 },
	{ "RP",		op_1b,		0xf0,	0,	0	 },
	{ "RPE",	op_1b,		0xe8,	0,	0	 },
	{ "RPO",	op_1b,		0xe0,	0,	0	 },
	{ "RRC",	op_1b,		0x0f,	0,	0	 },
	{ "RST",	op8080_rst,	0,	0,	0	 },
	{ "RZ",		op_1b,		0xc8,	0,	0	 },
	{ "SBB",	op8080_alu,	0x98,	0,	0	 },
	{ "SBI",	op8080_imm,	0xde,	0,	0	 },
	{ "SET",	op_dl,		0,	0,	OP_SET	 },
	{ "SHLD",	op8080_addr,	0x22,	0,	0	 },
	{ "SPHL",	op_1b,		0xf9,	0,	0	 },
	{ "STA",	op8080_addr,	0x32,	0,	0	 },
	{ "STAX",	op8080_regbd,	0x02,	0,	0	 },
	{ "STC",	op_1b,		0x37,	0,	0	 },
	{ "SUB",	op8080_alu,	0x90,	0,	0	 },
	{ "SUI",	op8080_imm,	0xd6,	0,	0	 },
	{ "XCHG",	op_1b,		0xeb,	0,	0	 },
	{ "XRA",	op8080_alu,	0xa8,	0,	0	 },
	{ "XRI",	op8080_imm,	0xee,	0,	0	 },
	{ "XTHL",	op_1b,		0xe3,	0,	0	 }
};

/*
 *	table with reserved 8080 operand words: registers and flags
 *	must be sorted in ascending order!
 */
struct ope opetab_8080[] = {
	{ "A",		REGA,	0 },
	{ "B",		REGB,	0 },
	{ "C",		REGC,	0 },
	{ "D",		REGD,	0 },
	{ "E",		REGE,	0 },
	{ "H",		REGH,	0 },
	{ "L",		REGL,	0 },
	{ "M",		REGM,	0 },
	{ "PSW",	REGPSW,	0 },
	{ "SP",		REGSP,	0 }
};

/*
 *	table of operations sets
 */
struct opset opsettab[] = {
	/* OPSET_PSD */
	{
		sizeof(opctab_psd) / sizeof(struct opc),
		opctab_psd,
		0,
		NULL
	},
	/* OPSET_Z80 */
	{
		sizeof(opctab_z80) / sizeof(struct opc),
		opctab_z80,
		sizeof(opetab_z80) / sizeof(struct ope),
		opetab_z80
	},
	/* OPSET_8080 */
	{
		sizeof(opctab_8080) / sizeof(struct opc),
		opctab_8080,
		sizeof(opetab_8080) / sizeof(struct ope),
		opetab_8080
	}
};
