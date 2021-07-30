/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2021 by Udo Munk
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
 */

/*
 *	global variable declarations
 */

extern char	*infiles[],
		objfn[],
		lstfn[],
		*srcfn,
		line[],
		tmp[],
		label[],
		opcode[],
		operand[],
		ops[],
		title[];

extern int	list_flag,
		sym_flag,
		ver_flag,
		dump_flag,
		pc,
		pass,
		iflevel,
		gencode,
		errors,
		errnum,
		sd_flag,
		sd_val,
		prg_adr,
		prg_flag,
		out_form,
		symlen,
		symsize,
		no_opcodes,
		no_operands;

extern FILE	*srcfp,
		*objfp,
		*lstfp,
		*errfp;

extern unsigned	c_line,
		s_line,
		p_line,
		ppl,
		page;

extern struct sym *symtab[],
		  **symarray;

extern struct opc opctab[];

extern struct ope opetab[];
