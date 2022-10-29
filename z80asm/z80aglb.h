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
 *	global variable declarations
 */

extern char	*infiles[],
		objfn[],
		lstfn[],
		*srcfn,
		line[],
		label[],
		opcode[],
		operand[],
		title[];

extern unsigned char
		ops[];

extern int	list_flag,
		sym_flag,
		undoc_flag,
		ver_flag,
		nofill_flag,
		mac_list_flag,
		radix,
		opset,
		rpc,
		pc,
		phs_flag,
		pass,
		iflevel,
		condnest[],
		gencode,
		mac_def_nest,
		mac_exp_nest,
		mac_symmax,
		errors,
		errnum,
		a_mode,
		a_addr,
		load_addr,
		load_flag,
		start_addr,
		out_form,
		symlen,
		symmax,
		symsize,
		hexlen;

extern FILE	*srcfp,
		*objfp,
		*lstfp,
		*errfp;

extern unsigned	c_line,
		p_line,
		ppl,
		page;

extern struct sym *symtab[],
		  **symarray;

extern struct opset opsettab[];
