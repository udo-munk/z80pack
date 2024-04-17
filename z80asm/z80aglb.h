/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	global variable declarations
 */

extern char	**infiles,
		*srcfn,
		*objfn,
		*lstfn,
		line[],
		label[],
		opcode[],
		operand[],
		title[];

extern BYTE	ops[],
		ctype[];

extern WORD	rpc,
		pc,
		a_addr,
		load_addr,
		start_addr,
		hexlen,
		carylen;

extern int	list_flag,
		sym_flag,
		undoc_flag,
		ver_flag,
		nofill_flag,
		upcase_flag,
		mac_list_flag,
		i8080_flag,
		nfiles,
		radix,
		phs_flag,
		pass,
		iflevel,
		act_iflevel,
		act_elselevel,
		gencode,
		nofalselist,
		mac_def_nest,
		mac_exp_nest,
		mac_symmax,
		errors,
		errnum,
		a_mode,
		load_flag,
		obj_fmt,
		symlen,
		symmax,
		p_line,
		ppl,
		page;

extern unsigned long
		c_line;

extern FILE	*srcfp,
		*objfp,
		*lstfp,
		*errfp;
