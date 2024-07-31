/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2024 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ALST_INC
#define Z80ALST_INC

#include <stdio.h>

#include "z80asm.h"

/*
 *	definition of address output modes for pseudo ops
 */
#define A_STD		0	/* address */
#define A_EQU		1	/* address, '=' */
#define A_SET		2	/* address, '#' */
#define A_DS		3	/* address, no data */
#define A_NONE		4	/* no address */

/*
 *	definition of symbol table sorting options
 */
#define SYM_NONE	0	/* no symbol table */
#define SYM_UNSORT	1	/* unsorted symbol table */
#define SYM_SORTN	2	/* symbol table sorted by name */
#define SYM_SORTA	3	/* symbol table sorted by address */

extern void lst_set_options(int pagelen, int nodate, int sym_sort);
extern FILE *lst_open_file(const char *fn);
extern void lst_close_file(void);
extern void lst_set_ppl(int n);
extern void lst_set_title(const char *s);
extern void lst_set_srcfn(const char *fn);
extern void lst_eject(int before);
extern void lst_line(const char *line, int list_active, int expn_flag,
		     int a_mode, WORD addr, const BYTE *ops, WORD op_cnt,
		     unsigned long c_line, const char *errmsg);
extern void lst_mac(void);
extern void lst_sym(void);

#endif /* !Z80ALST_INC */
