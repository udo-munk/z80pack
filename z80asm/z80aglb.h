/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AGLB_INC
#define Z80AGLB_INC

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

extern BYTE	ops[];

extern WORD	rpc,
		pc,
		hexlen,
		carylen;

extern int	list_flag,
		nodate_flag,
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

#endif /* !Z80AGLB_INC */
