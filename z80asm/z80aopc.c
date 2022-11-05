/*
 *	Z80 - Macro - Assembler
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
 *	28-OCT-2017 added variable symbol length and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 *	04-OCT-2022 new expression parser (TE)
 *	25-OCT-2022 Intel-like macros (TE)
 */

/*
 *	opcode tables and functions for setup and lookup
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "z80a.h"
#include "z80aglb.h"

int opccmp(const void *, const void *);

/* z80amain.c */
extern void fatal(int, const char *);

/* z80amfun.c */
extern unsigned op_endm(BYTE, BYTE), op_exitm(BYTE, BYTE), op_irp(BYTE, BYTE);
extern unsigned op_local(BYTE, BYTE), op_macro(BYTE, BYTE);
extern unsigned op_mcond(BYTE, BYTE), op_rept(BYTE, BYTE);

/* z80apfun.c */
extern unsigned op_instrset(BYTE, BYTE), op_org(BYTE, BYTE);
extern unsigned op_radix(BYTE, BYTE), op_equ(BYTE, BYTE), op_dl(BYTE, BYTE);
extern unsigned op_ds(BYTE, BYTE), op_db(BYTE, BYTE), op_dw(BYTE, BYTE);
extern unsigned op_misc(BYTE, BYTE), op_cond(BYTE, BYTE), op_glob(BYTE, BYTE);
extern unsigned op_end(BYTE, BYTE);

/* z80arfun.c */
extern unsigned op_1b(BYTE, BYTE), op_2b(BYTE, BYTE), op_im(BYTE, BYTE);
extern unsigned op_pupo(BYTE, BYTE), op_ex(BYTE, BYTE), op_rst(BYTE, BYTE);
extern unsigned op_ret(BYTE, BYTE), op_jpcall(BYTE, BYTE), op_jr(BYTE, BYTE);
extern unsigned op_djnz(BYTE, BYTE), op_ld(BYTE, BYTE), op_add(BYTE, BYTE);
extern unsigned op_sbadc(BYTE, BYTE), op_decinc(BYTE, BYTE);
extern unsigned op_alu(BYTE, BYTE), op_out(BYTE, BYTE), op_in(BYTE, BYTE);
extern unsigned op_cbgrp(BYTE, BYTE), op8080_mov(BYTE, BYTE);
extern unsigned op8080_alu(BYTE, BYTE), op8080_dcrinr(BYTE, BYTE);
extern unsigned op8080_reg16(BYTE, BYTE), op8080_regbd(BYTE, BYTE);
extern unsigned op8080_imm(BYTE, BYTE), op8080_rst(BYTE, BYTE);
extern unsigned op8080_pupo(BYTE, BYTE), op8080_addr(BYTE, BYTE);
extern unsigned op8080_mvi(BYTE, BYTE), op8080_lxi(BYTE, BYTE);

/*
 *	pseudo op table:
 *	includes entries for all common pseudo ops
 */
static struct opc opctab_psd[] = {
	{ ".8080",	op_instrset,	2,	0,	OP_NOLBL | OP_NOOPR },
	{ ".DEPHASE",	op_org,		3,	0,	OP_NOLBL | OP_NOOPR },
	{ ".LALL",	op_misc,	9,	0,	OP_NOLBL | OP_NOOPR },
	{ ".LFCOND",	op_misc,	12,	0,	OP_NOLBL | OP_NOOPR },
	{ ".LIST",	op_misc,	2,	0,	OP_NOLBL | OP_NOOPR },
	{ ".PHASE",	op_org,		2,	0,	OP_NOLBL },
	{ ".PRINTX",	op_misc,	4,	0,	OP_NOLBL | OP_NOPRE },
	{ ".RADIX",	op_radix,	0,	0,	OP_NOLBL },
	{ ".SALL",	op_misc,	10,	0,	OP_NOLBL | OP_NOOPR },
	{ ".SFCOND",	op_misc,	11,	0,	OP_NOLBL | OP_NOOPR },
	{ ".XALL",	op_misc,	8,	0,	OP_NOLBL | OP_NOOPR },
	{ ".XLIST",	op_misc,	3,	0,	OP_NOLBL | OP_NOOPR },
	{ ".Z80",	op_instrset,	1,	0,	OP_NOLBL | OP_NOOPR },
	{ "ABS",	op_glob,	3,	0,	OP_NOLBL | OP_NOOPR },
	{ "ASEG",	op_glob,	3,	0,	OP_NOLBL | OP_NOOPR },
	{ "ASET",	op_dl,		0,	0,	OP_SET	 },
	{ "COND",	op_cond,	5,	0,	OP_COND	 },
	{ "DB",		op_db,		1,	0,	0	 },
	{ "DC",		op_db,		2,	0,	0	 },
	{ "DEFB",	op_db,		1,	0,	0	 },
	{ "DEFC",	op_db,		2,	0,	0	 },
	{ "DEFL",	op_dl,		0,	0,	OP_SET	 },
	{ "DEFM",	op_db,		1,	0,	0	 },
	{ "DEFS",	op_ds,		0,	0,	OP_DS	 },
	{ "DEFW",	op_dw,		0,	0,	0	 },
	{ "DEFZ",	op_db,		3,	0,	0	 },
	{ "DS",		op_ds,		0,	0,	OP_DS	 },
	{ "DW",		op_dw,		0,	0,	0	 },
	{ "EJECT",	op_misc,	1,	0,	OP_NOLBL | OP_NOOPR },
	{ "ELSE",	op_cond,	98,	0,	OP_COND  | OP_NOOPR },
	{ "END",	op_end,		0,	0,	OP_END	 },
	{ "ENDC",	op_cond,	99,	0,	OP_COND  | OP_NOOPR },
	{ "ENDIF",	op_cond,	99,	0,	OP_COND  | OP_NOOPR },
	{ "ENDM",	op_endm,	0,	0,	OP_MEND	 },
	{ "ENT",	op_glob,	2,	0,	OP_NOLBL },
	{ "ENTRY",	op_glob,	2,	0,	OP_NOLBL },
	{ "EQU",	op_equ,		0,	0,	OP_SET	 },
	{ "EXITM",	op_exitm,	0,	0,	0	 },
	{ "EXT",	op_glob,	1,	0,	OP_NOLBL },
	{ "EXTERNAL",	op_glob,	1,	0,	OP_NOLBL },
	{ "EXTRN",	op_glob,	1,	0,	OP_NOLBL },
	{ "GLOBAL",	op_glob,	2,	0,	OP_NOLBL },
	{ "IF",		op_cond,	5,	0,	OP_COND	 },
	{ "IF1",	op_cond,	7,	0,	OP_COND  | OP_NOOPR },
	{ "IF2",	op_cond,	8,	0,	OP_COND  | OP_NOOPR },
	{ "IFB",	op_mcond,	1,	0,	OP_COND  | OP_NOPRE },
	{ "IFDEF",	op_cond,	1,	0,	OP_COND	 },
	{ "IFDIF",	op_mcond,	4,	0,	OP_COND  | OP_NOPRE },
	{ "IFE",	op_cond,	6,	0,	OP_COND	 },
	{ "IFEQ",	op_cond,	3,	0,	OP_COND	 },
	{ "IFF",	op_cond,	6,	0,	OP_COND	 },
	{ "IFIDN",	op_mcond,	3,	0,	OP_COND  | OP_NOPRE },
	{ "IFNB",	op_mcond,	2,	0,	OP_COND  | OP_NOPRE },
	{ "IFNDEF",	op_cond,	2,	0,	OP_COND	 },
	{ "IFNEQ",	op_cond,	4,	0,	OP_COND	 },
	{ "IFT",	op_cond,	5,	0,	OP_COND	 },
	{ "INCLUDE",	op_misc,	6,	0,	OP_INCL  | OP_NOLBL
								 | OP_NOPRE },
	{ "IRP",	op_irp,		1,	0,	OP_MDEF  | OP_NOPRE },
	{ "IRPC",	op_irp,		2,	0,	OP_MDEF  | OP_NOPRE },
	{ "LIST",	op_misc,	2,	0,	OP_NOLBL | OP_NOOPR },
	{ "LOCAL",	op_local,	0,	0,	OP_NOLBL },
	{ "MACLIB",	op_misc,	6,	0,	OP_INCL  | OP_NOLBL
								 | OP_NOPRE },
	{ "MACRO",	op_macro,	0,	0,	OP_MDEF  | OP_SET   },
	{ "NOLIST",	op_misc,	3,	0,	OP_NOLBL | OP_NOOPR },
	{ "ORG",	op_org,		1,	0,	OP_NOLBL },
	{ "PAGE",	op_misc,	1,	0,	OP_NOLBL },
	{ "PRINT",	op_misc,	5,	0,	OP_NOLBL | OP_NOPRE },
	{ "PUBLIC",	op_glob,	2,	0,	OP_NOLBL },
	{ "REPT",	op_rept,	3,	0,	OP_MDEF	 },
	{ "TITLE",	op_misc,	7,	0,	OP_NOLBL | OP_NOPRE }
};
static unsigned no_opc_psd = sizeof(opctab_psd) / sizeof(struct opc);

/*
 *	Z80 opcode table:
 *	includes entries for all Z80 opcodes
 *	must not contain any ops already in opctab_psd!
 */
static struct opc opctab_z80[] = {
	{ "ADC",	op_sbadc,	0x88,	0x4a,	0	 },
	{ "ADD",	op_add,		0x80,	0x09,	0	 },
	{ "AND",	op_alu,		0xa0,	0,	0	 },
	{ "BIT",	op_cbgrp,	0x40,	0,	0	 },
	{ "CALL",	op_jpcall,	0xcd,	0xc4,	0	 },
	{ "CCF",	op_1b,		0x3f,	0,	OP_NOOPR },
	{ "CP",		op_alu,		0xb8,	0,	0	 },
	{ "CPD",	op_2b,		0xed,	0xa9,	OP_NOOPR },
	{ "CPDR",	op_2b,		0xed,	0xb9,	OP_NOOPR },
	{ "CPI",	op_2b,		0xed,	0xa1,	OP_NOOPR },
	{ "CPIR",	op_2b,		0xed,	0xb1,	OP_NOOPR },
	{ "CPL",	op_1b,		0x2f,	0,	OP_NOOPR },
	{ "DAA",	op_1b,		0x27,	0,	OP_NOOPR },
	{ "DEC",	op_decinc,	0x05,	0x0b,	0	 },
	{ "DI",		op_1b,		0xf3,	0,	OP_NOOPR },
	{ "DJNZ",	op_djnz,	0x10,	0,	0	 },
	{ "EI",		op_1b,		0xfb,	0,	OP_NOOPR },
	{ "EX",		op_ex,		0xe3,	0xeb,	0	 },
	{ "EXX",	op_1b,		0xd9,	0,	OP_NOOPR },
	{ "HALT",	op_1b,		0x76,	0,	OP_NOOPR },
	{ "IM",		op_im,		0xed,	0x46,	0	 },
	{ "IN",		op_in,		0xdb,	0x40,	0	 },
	{ "INC",	op_decinc,	0x04,	0x03,	0	 },
	{ "IND",	op_2b,		0xed,	0xaa,	OP_NOOPR },
	{ "INDR",	op_2b,		0xed,	0xba,	OP_NOOPR },
	{ "INI",	op_2b,		0xed,	0xa2,	OP_NOOPR },
	{ "INIR",	op_2b,		0xed,	0xb2,	OP_NOOPR },
	{ "JP",		op_jpcall,	0xc3,	0xc2,	0	 },
	{ "JR",		op_jr,		0x18,	0x20,	0	 },
	{ "LD",		op_ld,		0x40,	0,	0	 },
	{ "LDD",	op_2b,		0xed,	0xa8,	OP_NOOPR },
	{ "LDDR",	op_2b,		0xed,	0xb8,	OP_NOOPR },
	{ "LDI",	op_2b,		0xed,	0xa0,	OP_NOOPR },
	{ "LDIR",	op_2b,		0xed,	0xb0,	OP_NOOPR },
	{ "NEG",	op_2b,		0xed,	0x44,	OP_NOOPR },
	{ "NOP",	op_1b,		0x00,	0,	OP_NOOPR },
	{ "OR",		op_alu,		0xb0,	0,	0	 },
	{ "OTDR",	op_2b,		0xed,	0xbb,	OP_NOOPR },
	{ "OTIR",	op_2b,		0xed,	0xb3,	OP_NOOPR },
	{ "OUT",	op_out,		0xd3,	0x41,	0	 },
	{ "OUTD",	op_2b,		0xed,	0xab,	OP_NOOPR },
	{ "OUTI",	op_2b,		0xed,	0xa3,	OP_NOOPR },
	{ "POP",	op_pupo,	0xc1,	0,	0	 },
	{ "PUSH",	op_pupo,	0xc5,	0,	0	 },
	{ "RES",	op_cbgrp,	0x80,	0,	0	 },
	{ "RET",	op_ret,		0xc9,	0xc0,	0	 },
	{ "RETI",	op_2b,		0xed,	0x4d,	OP_NOOPR },
	{ "RETN",	op_2b,		0xed,	0x45,	OP_NOOPR },
	{ "RL",		op_cbgrp,	0x10,	0,	0	 },
	{ "RLA",	op_1b,		0x17,	0,	OP_NOOPR },
	{ "RLC",	op_cbgrp,	0x00,	0,	0	 },
	{ "RLCA",	op_1b,		0x07,	0,	OP_NOOPR },
	{ "RLD",	op_2b,		0xed,	0x6f,	OP_NOOPR },
	{ "RR",		op_cbgrp,	0x18,	0,	0	 },
	{ "RRA",	op_1b,		0x1f,	0,	OP_NOOPR },
	{ "RRC",	op_cbgrp,	0x08,	0,	0	 },
	{ "RRCA",	op_1b,		0x0f,	0,	OP_NOOPR },
	{ "RRD",	op_2b,		0xed,	0x67,	OP_NOOPR },
	{ "RST",	op_rst,		0xc7,	0,	0	 },
	{ "SBC",	op_sbadc,	0x98,	0x42,	0	 },
	{ "SCF",	op_1b,		0x37,	0,	OP_NOOPR },
	{ "SET",	op_cbgrp,	0xc0,	0,	0	 },
	{ "SLA",	op_cbgrp,	0x20,	0,	0	 },
	{ "SLL",	op_cbgrp,	0x30,	0,	OP_UNDOC },
	{ "SRA",	op_cbgrp,	0x28,	0,	0	 },
	{ "SRL",	op_cbgrp,	0x38,	0,	0	 },
	{ "SUB",	op_alu,		0x90,	0,	0	 },
	{ "XOR",	op_alu,		0xa8,	0,	0	 }
};
static unsigned no_opc_z80 = sizeof(opctab_z80) / sizeof(struct opc);

/*
 *	table with reserved Z80 operand words: registers and flags
 *	must be sorted in ascending order!
 */
static struct ope opetab_z80[] = {
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
static unsigned no_ope_z80 = sizeof(opetab_z80) / sizeof(struct ope);

/*
 *	8080 opcode table:
 *	includes entries for all 8080 opcodes and 8080 specific pseudo ops
 *	must not contain any ops already in opctab_psd!
 */
static struct opc opctab_8080[] = {
	{ "ACI",	op8080_imm,	0xce,	0,	0	 },
	{ "ADC",	op8080_alu,	0x88,	0,	0	 },
	{ "ADD",	op8080_alu,	0x80,	0,	0	 },
	{ "ADI",	op8080_imm,	0xc6,	0,	0	 },
	{ "ANA",	op8080_alu,	0xa0,	0,	0	 },
	{ "ANI",	op8080_imm,	0xe6,	0,	0	 },
	{ "CALL",	op8080_addr,	0xcd,	0,	0	 },
	{ "CC",		op8080_addr,	0xdc,	0,	0	 },
	{ "CM",		op8080_addr,	0xfc,	0,	0	 },
	{ "CMA",	op_1b,		0x2f,	0,	OP_NOOPR },
	{ "CMC",	op_1b,		0x3f,	0,	OP_NOOPR },
	{ "CMP",	op8080_alu,	0xb8,	0,	0	 },
	{ "CNC",	op8080_addr,	0xd4,	0,	0	 },
	{ "CNZ",	op8080_addr,	0xc4,	0,	0	 },
	{ "CP",		op8080_addr,	0xf4,	0,	0	 },
	{ "CPE",	op8080_addr,	0xec,	0,	0	 },
	{ "CPI",	op8080_imm,	0xfe,	0,	0	 },
	{ "CPO",	op8080_addr,	0xe4,	0,	0	 },
	{ "CZ",		op8080_addr,	0xcc,	0,	0	 },
	{ "DAA",	op_1b,		0x27,	0,	OP_NOOPR },
	{ "DAD",	op8080_reg16,	0x09,	0,	0	 },
	{ "DCR",	op8080_dcrinr,	0x05,	0,	0	 },
	{ "DCX",	op8080_reg16,	0x0b,	0,	0	 },
	{ "DI",		op_1b,		0xf3,	0,	OP_NOOPR },
	{ "EI",		op_1b,		0xfb,	0,	OP_NOOPR },
	{ "HLT",	op_1b,		0x76,	0,	OP_NOOPR },
	{ "IN",		op8080_imm,	0xdb,	0,	0	 },
	{ "INR",	op8080_dcrinr,	0x04,	0,	0	 },
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
	{ "LXI",	op8080_lxi,	0x01,	0,	0	 },
	{ "MOV",	op8080_mov,	0x40,	0,	0	 },
	{ "MVI",	op8080_mvi,	0x06,	0,	0	 },
	{ "NOP",	op_1b,		0x00,	0,	OP_NOOPR },
	{ "ORA",	op8080_alu,	0xb0,	0,	0	 },
	{ "ORI",	op8080_imm,	0xf6,	0,	0	 },
	{ "OUT",	op8080_imm,	0xd3,	0,	0	 },
	{ "PCHL",	op_1b,		0xe9,	0,	OP_NOOPR },
	{ "POP",	op8080_pupo,	0xc1,	0,	0	 },
	{ "PUSH",	op8080_pupo,	0xc5,	0,	0	 },
	{ "RAL",	op_1b,		0x17,	0,	OP_NOOPR },
	{ "RAR",	op_1b,		0x1f,	0,	OP_NOOPR },
	{ "RC",		op_1b,		0xd8,	0,	OP_NOOPR },
	{ "RET",	op_1b,		0xc9,	0,	OP_NOOPR },
	{ "RLC",	op_1b,		0x07,	0,	OP_NOOPR },
	{ "RM",		op_1b,		0xf8,	0,	OP_NOOPR },
	{ "RNC",	op_1b,		0xd0,	0,	OP_NOOPR },
	{ "RNZ",	op_1b,		0xc0,	0,	OP_NOOPR },
	{ "RP",		op_1b,		0xf0,	0,	OP_NOOPR },
	{ "RPE",	op_1b,		0xe8,	0,	OP_NOOPR },
	{ "RPO",	op_1b,		0xe0,	0,	OP_NOOPR },
	{ "RRC",	op_1b,		0x0f,	0,	OP_NOOPR },
	{ "RST",	op8080_rst,	0xc7,	0,	0	 },
	{ "RZ",		op_1b,		0xc8,	0,	OP_NOOPR },
	{ "SBB",	op8080_alu,	0x98,	0,	0	 },
	{ "SBI",	op8080_imm,	0xde,	0,	0	 },
	{ "SET",	op_dl,		0,	0,	OP_SET	 },
	{ "SHLD",	op8080_addr,	0x22,	0,	0	 },
	{ "SPHL",	op_1b,		0xf9,	0,	OP_NOOPR },
	{ "STA",	op8080_addr,	0x32,	0,	0	 },
	{ "STAX",	op8080_regbd,	0x02,	0,	0	 },
	{ "STC",	op_1b,		0x37,	0,	OP_NOOPR },
	{ "SUB",	op8080_alu,	0x90,	0,	0	 },
	{ "SUI",	op8080_imm,	0xd6,	0,	0	 },
	{ "XCHG",	op_1b,		0xeb,	0,	OP_NOOPR },
	{ "XRA",	op8080_alu,	0xa8,	0,	0	 },
	{ "XRI",	op8080_imm,	0xee,	0,	0	 },
	{ "XTHL",	op_1b,		0xe3,	0,	OP_NOOPR }
};
static unsigned no_opc_8080 = sizeof(opctab_8080) / sizeof(struct opc);

/*
 *	table with reserved 8080 operand words: registers and flags
 *	must be sorted in ascending order!
 */
static struct ope opetab_8080[] = {
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
static unsigned no_ope_8080 = sizeof(opetab_8080) / sizeof(struct ope);

static int curr_instrset;	/* current instructions set */
static struct opc **opctab;	/* current sorted operations table */
static unsigned no_opcodes;	/* current of operations */
static struct ope *opetab;	/* current sorted register/flags table */
static unsigned no_operands;	/* current number of register/flags */

/*
 *	build sorted table opctab for instruction set os
 */
void instrset(int is)
{
	register struct opc *opc, *p, **q;
	register unsigned i, nopc;

	nopc = 0;		/* silence compiler */
	if (is == curr_instrset)
		return;
	switch (is) {
	case INSTR_Z80:
		opc = opctab_z80;
		nopc = no_opc_z80;
		opetab = opetab_z80;
		no_operands = no_ope_z80;
		break;
	case INSTR_8080:
		opc = opctab_8080;
		nopc = no_opc_8080;
		opetab = opetab_8080;
		no_operands = no_ope_8080;
		break;
	default:
		fatal(F_INTERN, "invalid instr. set for function instrset");
	}
	if (opctab == NULL) {
		i = no_opc_psd;
		i += (no_opc_z80 > no_opc_8080 ? no_opc_z80 : no_opc_8080);
		opctab = (struct opc **) malloc(sizeof(struct opc *) * i);
		if (opctab == NULL)
			fatal(F_OUTMEM, "operations table");
	}
	no_opcodes = no_opc_psd + nopc;
	q = opctab;
	for (i = 0, p = opctab_psd; i < no_opc_psd; i++)
		*q++ = p++;
	for (i = 0, p = opc; i < nopc; i++)
		*q++ = p++;
	qsort(opctab, no_opcodes, sizeof(struct opc *), opccmp);
	curr_instrset = is;
}

/*
 *	compares two opcodes for qsort()
 */
int opccmp(const void *p1, const void *p2)
{
	return(strcmp((*(const struct opc **) p1)->op_name,
		      (*(const struct opc **) p2)->op_name));
}

/*
 *	binary search in sorted table opctab
 *
 *	Input: pointer to string with opcode
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct opc *search_op(char *op_name)
{
	register int cond;
	register struct opc **low, **high, **mid;

	low = opctab;
	high = opctab + no_opcodes - 1;
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(op_name, (*mid)->op_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else if (!undoc_flag && ((*mid)->op_flags & OP_UNDOC))
			return(NULL);
		else
			return(*mid);
	}
	return(NULL);
}

/*
 *	binary search in sorted table opetab
 *
 *	Input: pointer to string with operand
 *
 *	Output: symbol for operand, NOOPERA if empty operand,
 *		NOREG if operand not found
 */
BYTE get_reg(char *s)
{
	register int cond;
	register struct ope *low, *high, *mid;

	if (s == NULL || *s == '\0')
		return(NOOPERA);
	low = opetab;
	high = opetab + no_operands - 1;
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(s, mid->ope_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else if (!undoc_flag && (mid->ope_flags & OPE_UNDOC))
			return(NOREG);
		else
			return(mid->ope_sym);
	}
	return(NOREG);
}
